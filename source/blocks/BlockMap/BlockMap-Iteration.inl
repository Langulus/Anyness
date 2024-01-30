///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"


namespace Langulus::Anyness
{

   /// Execute a call for each pair inside the map                            
   ///   @tparam REVERSE - whether or not to iterate in reverse               
   ///   @param call - the function to execute for each pair                  
   ///   @return the number of successfull executions                         
   template<bool REVERSE, CT::Map THIS>
   Count BlockMap::ForEach(auto&& call) const {
      if (IsEmpty())
         return 0;

      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Pair<A>, "F's argument must be a pair type");
      static_assert(CT::Dense<A>, "F's argument must be a dense pair");

      if constexpr (CT::TypedPair<A>) {
         // If the pair is statically typed, we check contained types   
         // against it prior to iterating                               
         using K = typename A::Key;
         using V = typename A::Value;
         if (not IsKey<THIS, K>() or not IsValue<THIS, V>()) {
            // Key/Value mismatch, no need to iterate at all            
            return 0;
         }
      }

      // Prepare for the loop                                           
      auto key = mKeys.GetElement(REVERSE ? -1 : 0);
      auto val = mValues.GetElement(REVERSE ? -1 : 0);
      auto inf = REVERSE ? mInfo + GetReserved() - 1 : mInfo;
      const auto infEnd = REVERSE ? mInfo - 1 : mInfo + GetReserved();
      const auto next = [&inf,&key,&val] {
         if constexpr (REVERSE) {
            --inf;
            --key;
            --val;
         }
         else {
            ++inf;
            ++key;
            ++val;
         }
      };

      Count executions = 0;
      while (inf != infEnd) {
         if (not *inf) {
            next();
            continue;
         }
         
         // Execute function for each valid pair                        
         ++executions;

         if constexpr (CT::Void<R>) {
            if constexpr (CT::TypedPair<A>) {
               // The pair is statically typed, so we need to access    
               // the elements by the provided types                    
               using K = typename A::Key;
               using V = typename A::Value;
               A pair {key.template Get<K>(), val.template Get<V>()};   
               call(pair);
            }
            else {
               // The pair is dynamically typed, so we directly         
               // forward the element blocks                            
               A pair {key, val};
               call(pair);
            }

            next();
         }
         else {
            R loop;
            if constexpr (CT::TypedPair<A>) {
               // The pair is statically typed, so we need to access    
               // the elements by the provided types                    
               using K = typename A::Key;
               using V = typename A::Value;
               A pair {key.template Get<K>(), val.template Get<V>()};
               loop = call(pair);
            }
            else {
               // The pair is dynamically typed, so we directly         
               // forward the element blocks                            
               A pair {key, val};
               loop = call(pair);
            }

            if constexpr (CT::Bool<R>) {
               if (not loop)
                  return executions;
               next();
            }
            else if constexpr (CT::Exact<R, LoopControl>) {
               switch (loop) {
               case Loop::Break:
                  return executions;
               case Loop::Continue:
                  next();
                  break;
               case Loop::Discard:
                  if constexpr (CT::Mutable<THIS>) {
                     // Discard is allowed only if THIS is mutable      
                     const_cast<BlockMap*>(this)
                        ->template RemoveInner<THIS>(inf - mInfo);
                     if constexpr (REVERSE)
                        next();
                  }
                  else {
                     // ...otherwise it acts like a Loop::Continue      
                     next();
                  }
                  break;
               }
            }
            else LANGULUS_ERROR("Unknown loop control format");
         }
      }

      return executions;
   }

   /// Iterate and execute call for each element in mKeys/mValues             
   ///   @attention assumes map is not empty, and part is properly typed      
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the function to execute for each element               
   ///   @return the number of executions that occured                        
   template<CT::Map THIS, bool REVERSE>
   LoopControl BlockMap::ForEachInner(
      const CT::Block auto& part, auto&& call, Count& counter
   ) const {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Map is empty");
      LANGULUS_ASSUME(DevAssumes, (part.template CastsTo<A, true>()),
         "Map is not typed properly");
       
      auto inf = REVERSE ? mInfo + GetReserved() - 1 : mInfo;
      const auto infEnd = REVERSE ? mInfo - 1 : mInfo + GetReserved();
      const auto next = [&inf] {
         if constexpr (REVERSE)  --inf;
         else                    ++inf;
      };

      while (inf != infEnd) {
         if (not *inf) {
            next();
            continue;
         }

         ++counter;

         if constexpr (CT::Bool<R>) {
            if (not call(part.template Get<A>(inf - mInfo)))
               return Loop::Break;
            next();
         }
         else if constexpr (CT::Exact<R, LoopControl>) {
            const auto loop = call(part.template Get<A>(inf - mInfo));
            switch (loop.mControl) {
            case LoopControl::Break:
            case LoopControl::NextLoop:
               return loop;
            case LoopControl::Continue:
               next();
               break;
            case LoopControl::Discard:
               if constexpr (CT::Mutable<THIS>) {
                  // Discard is allowed only if THIS is mutable         
                  const_cast<BlockMap*>(this)
                     ->template RemoveInner<THIS>(inf - mInfo);
                  if constexpr (REVERSE)
                     next();
               }
               else {
                  // ...otherwise it acts like a Loop::Continue         
                  next();
               }
               break;
            }
         }
         else {
            call(part.template Get<A>(inf - mInfo));
            next();
         }
      }

      return Loop::Continue;
   }
   
   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param part - the block to iterate                                   
   ///   @param call - the function to call for each key block                
   ///   @return the number of successful executions                          
   template<CT::Map THIS, bool REVERSE>
   LoopControl BlockMap::ForEachElementInner(
      const CT::Block auto& part, auto&& call, Count& counter
   ) const {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block binary-compatible type");
      static_assert(CT::Constant<A> or CT::Mutable<THIS>,
         "Non constant iterator for constant memory block");

      auto info = REVERSE ? mInfo + GetReserved() - 1 : mInfo;
      const auto infoEnd = REVERSE ? mInfo - 1 : mInfo + GetReserved();
      const auto next = [&info] {
         if constexpr (REVERSE)  --info;
         else                    ++info;
      };

      while (info != infoEnd) {
         if (not *info) {
            next();
            continue;
         }

         ++counter;

         if constexpr (CT::Bool<R>) {
            if (not call(part.GetElement(info - mInfo)))
               return Loop::Break;
            next();
         }
         else if constexpr (CT::Exact<R, LoopControl>) {
            const auto loop = call(part.GetElement(info - mInfo));
            switch (loop) {
            case LoopControl::Break:
            case LoopControl::NextLoop:
               return loop;
            case Loop::Continue:
               next();
               break;
            case Loop::Discard:
               if constexpr (CT::Mutable<THIS>) {
                  // Discard is allowed only if THIS is mutable         
                  const_cast<BlockMap*>(this)
                     ->template RemoveInner<THIS>(info - mInfo);
                  if constexpr (REVERSE)
                     next();
               }
               else {
                  // ...otherwise it acts like a Loop::Continue         
                  next();
               }
               break;
            }
         }
         else {
            call(part.GetElement(info - mInfo));
            next();
         }
      }

      return Loop::Continue;
   }
   
   /// Iterate and execute call for each deep element                         
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - true to avoid executing call in intermediate blocks   
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<CT::Map THIS, bool REVERSE, bool SKIP>
   LoopControl BlockMap::ForEachDeepInner(
      const CT::Block auto& part, auto&& call, Count& counter
   ) const {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      //using R = ReturnOf<F>;
      using SubBlock = Conditional<CT::Mutable<THIS>, Block&, const Block&>;

      /*if constexpr (CT::Deep<A>) {
         if (not SKIP or not part.IsDeep()) {
            // Always execute for intermediate/non-deep *this           
            ++counter;

            if constexpr (CT::Bool<R>) {
               if (not call(part))
                  return Loop::Break;
            }
            else if constexpr (CT::Exact<R, LoopControl>) {
               // Do things depending on the F's return                 
               const auto loop = call(part);
               switch (loop) {
               case Loop::Break:
                  return Loop::Break;
               case Loop::Continue:
                  break;
               case Loop::Discard:
                  if constexpr (CT::Mutable<THIS>) {
                     // Discard is allowed only if THIS is mutable      
                     const_cast<Block&>(part).template Reset<THIS>();
                     return Loop::Discard;
                  }
                  else {
                     // ...otherwise it acts like a Loop::Continue      
                     break;
                  }
               }
            }
            else call(part);
         }
      }*/

      if (part.IsDeep()) {
         // Iterate deep keys/values using non-block type               
         return ForEachInner<THIS, REVERSE>(part,
            [&counter, &call](SubBlock group) -> LoopControl {
               return DenseCast(group).template
                  ForEachDeepInner<SubBlock, REVERSE, SKIP>(
                     ::std::move(call), counter);
            }, counter
         );
      }
      else if constexpr (not CT::Deep<A>) {
         // Equivalent to non-deep iteration                            
         return ForEachInner<THIS, REVERSE>(
            part, ::std::move(call), counter);
      }

      return Loop::Continue;
   }

   /// Iterate all keys inside the map, and perform call() on each            
   /// You can break the loop, by returning false inside call()               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the function to call for each key block                
   ///   @return the number of successful executions                          
   template<bool REVERSE, CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::ForEachKeyElement(auto&& call) const {
      using F = Deref<decltype(call)>;
      return ForEachElement<THIS, REVERSE>(
         GetKeys<THIS>(), Forward<F>(call));
   }

   /// Iterate all values inside the map, and perform call() on them          
   /// You can break the loop, by returning false inside call()               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the function to call for each key block                
   ///   @return the number of successful call() executions                   
   template<bool REVERSE, CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::ForEachValueElement(auto&& call) const {
      using F = Deref<decltype(call)>;
      return ForEachElement<THIS, REVERSE>(
         GetVals<THIS>(), Forward<F>(call));
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside call()               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the functions to call for each key block               
   ///   @return the number of successful call() executions                   
   template<bool REVERSE, CT::Map THIS, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachKey(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (Loop::NextLoop !=  
         ForEachInner<THIS, REVERSE>(GetKeys<THIS>(), Forward<F>(call), result)
      ));
      return result;
   }

   /// Iterate values inside the map, and perform a set of functions on them  
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the functions to call for each value block             
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, CT::Map THIS, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachValue(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (Loop::NextLoop != 
         ForEachInner<THIS, REVERSE>(GetVals<THIS>(), Forward<F>(call), result)
      ));
      return result;
   }

   /// Iterate each subblock of keys inside the map, and perform a set of     
   /// functions on them                                                      
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the functions to call for each key block               
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool SKIP, CT::Map THIS, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachKeyDeep(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (Loop::Break != 
         ForEachDeepInner<THIS, REVERSE, SKIP>(
            GetKeys<THIS>(), Forward<F>(call), result)
      ));
      return result;
   }

   /// Iterate each subblock of values inside the map, and perform a set of   
   /// functions on them                                                      
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - whether to execute calls for intermediate containers  
   ///   @param call - the functions to call for each value block             
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool SKIP, CT::Map THIS, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachValueDeep(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (Loop::Break != 
         ForEachDeepInner<THIS, REVERSE, SKIP>(
            GetVals<THIS>(), Forward<F>(call), result)
      ));
      return result;
   }

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::Iterator<THIS> BlockMap::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info)
         ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetRawKey<THIS>(offset),
         GetRawVal<THIS>(offset)
      };
   }

   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::Iterator<const THIS> BlockMap::begin() const noexcept {
      return const_cast<BlockMap*>(this)->begin<THIS>();
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::Iterator<THIS> BlockMap::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info)
         ;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetRawKey<THIS>(offset),
         GetRawVal<THIS>(offset)
      };
   }

   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::Iterator<const THIS> BlockMap::last() const noexcept {
      return const_cast<BlockMap*>(this)->last<THIS>();
   }


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        

   /// Construct a map iterator                                               
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer/block to the key element                        
   ///   @param value - pointer/block to the value element                    
   template<class T> LANGULUS(INLINED)
   constexpr BlockMap::Iterator<T>::Iterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const KA& key, const VA& value
   ) noexcept
      : mKey {key}
      , mValue {value}
      , mInfo {info}
      , mSentinel {sentinel} {}

   /// Construct from end point                                               
   template<class T> LANGULUS(INLINED)
   constexpr BlockMap::Iterator<T>::Iterator(const A::IteratorEnd&) noexcept
      : mKey {}
      , mValue {} 
      , mInfo {}
      , mSentinel {} {}

   /// Prefix increment operator                                              
   /// Moves pointers to the right, unless end has been reached               
   ///   @return the modified iterator                                        
   template<class T> LANGULUS(INLINED)
   constexpr BlockMap::Iterator<T>& BlockMap::Iterator<T>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (not *++mInfo)
         ;

      const auto offset = mInfo - previous;
      if constexpr (CT::Typed<T>) {
         const_cast<KA&>(mKey)   += offset;
         const_cast<VA&>(mValue) += offset;
      }
      else {
         const_cast<KA&>(mKey).mRaw   += offset * mKey.GetStride();
         const_cast<VA&>(mValue).mRaw += offset * mValue.GetStride();
         // Notice we don't affect count for properly accessing entries 
         // Any attempt at transferring these blocks will UB            
         // Iterators are not intended for use as mediators of any      
         // transfer of ownership. We just stick to them,               
         // as a form of indexing, and nothing more. Otherwise,         
         // accounting for avoiding potentially hazardous access will   
         // cost twice as much per iteration                            
      }
      return *this;
   }

   /// Suffix increment operator                                              
   /// Moves pointers to the right, unless end has been reached               
   ///   @return the previous value of the iterator                           
   template<class T> LANGULUS(INLINED)
   constexpr BlockMap::Iterator<T> BlockMap::Iterator<T>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare iterators                                                      
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<class T> LANGULUS(INLINED)
   constexpr bool BlockMap::Iterator<T>::operator == (const Iterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Check if iterator has reached the end                                  
   ///   @return true if entries match                                        
   template<class T> LANGULUS(INLINED)
   constexpr bool BlockMap::Iterator<T>::operator == (const A::IteratorEnd&) const noexcept {
      return mInfo >= mSentinel;
   }

   /// Iterator access operator                                               
   /// It is required for ranged-for expressions                              
   /// In our case, it just generates a temporary pair of references          
   ///   @return the pair at the current iterator position                    
   template<class T> LANGULUS(INLINED)
   constexpr auto BlockMap::Iterator<T>::operator * () const {
      if (mInfo >= mSentinel)
         LANGULUS_OOPS(Access, "Trying to access end of iteration");

      const auto me = const_cast<Iterator<T>*>(this);
      using B = Conditional<CT::Mutable<T>, Block&, const Block&>;
      if constexpr (CT::TypeErased<Key>) {
         if constexpr (CT::TypeErased<Value>)
            return TPair<const Block&, B> {mKey, me->mValue};
         else
            return TPair<const Block&, Value&> {mKey, *me->mValue};
      }
      else {
         if constexpr (CT::TypeErased<Value>)
            return TPair<Key&, B> {*mKey, me->mValue};
         else
            return TPair<Key&, Value&> {*mKey, *me->mValue};
      }
   }

   /// Explicit bool operator, to check if iterator is valid                  
   template<class T> LANGULUS(INLINED)
   constexpr BlockMap::Iterator<T>::operator bool() const noexcept {
      return mInfo < mSentinel;
   }

   /// Implicitly convert to a constant iterator                              
   template<class T> LANGULUS(INLINED)
   constexpr BlockMap::Iterator<T>::operator Iterator<const T>() const noexcept requires Mutable {
      return {mInfo, mSentinel, mKey, mValue};
   }

} // namespace Langulus::Anyness
