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
   template<CT::Map, bool REVERSE>
   Count BlockMap::ForEach(auto&& call) const {
      if (IsEmpty())
         return 0;

      using F = Deref<decltype(call)>;
      using A = Decay<ArgumentOf<F>>;
      using R = ReturnOf<F>;

      static_assert(CT::Pair<A>, "F's argument must be a pair type");
      static_assert(CT::Dense<A>, "F's argument must be a dense pair");

      if constexpr (CT::TypedPair<A>) {
         // If the pair is statically typed, we check contained types   
         // against it prior to iterating                               
         using K = typename A::Key;
         using V = typename A::Value;
         if (not KeyIs<K>() or not ValueIs<V>()) {
            // Key/Value mismatch, no need to iterate at all            
            return 0;
         }
      }

      // Prepare for the loop                                           
      constexpr bool HasBreaker = CT::Bool<R>;
      auto key = mKeys.GetElement(REVERSE ? -1 : 0);
      auto val = mValues.GetElement(REVERSE ? -1 : 0);
      auto inf = REVERSE ? mInfo + GetReserved() - 1 : mInfo;
      const auto infEnd = REVERSE ? mInfo - 1 : mInfo + GetReserved();
      Count executions {};

      while (inf != infEnd) {
         if (*inf) {
            ++executions;

            // Execute function for each valid pair                     
            if constexpr (HasBreaker) {
               if constexpr (CT::TypedPair<A>) {
                  // The pair is statically typed, so we need to access 
                  // the elements by the provided types                 
                  using K = typename A::Key;
                  using V = typename A::Value;
                  A pair {key.template Get<K>(), val.template Get<V>()};
                  if (not call(pair)) {
                     // Early return, if function returns a false bool  
                     return executions;
                  }
               }
               else {
                  // The pair is dynamically typed, so we directly      
                  // forward the element blocks                         
                  A pair {key, val};
                  if (not call(pair)) {
                     // Early return, if function returns a false bool  
                     return executions;
                  }
               }
            }
            else {
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
            }
         }

         // Next element                                                
         if constexpr (REVERSE) {
            --inf;
            key.Prev();
            val.Prev();
         }
         else {
            ++inf;
            key.Next();
            val.Next();
         }
      }

      return executions;
   }

   /// Iterate and execute call for each element in mKeys/mValues             
   ///   @attention assumes map is not empty, and part is properly typed      
   ///   @tparam R - the function return type                                 
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type                               
   ///               A must be a CT::Block type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the function to execute for each element               
   ///   @return the number of executions that occured                        
   template<CT::Map, class R, CT::Data A, bool REVERSE>
   Count BlockMap::ForEachInner(const CT::Block auto& part, auto&& call) const {
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Map is empty");
      LANGULUS_ASSUME(DevAssumes, (part.template CastsTo<A, true>()),
         "Map is not typed properly");
       
      constexpr bool HasBreaker = CT::Bool<R>;
      Count done = 0;
      Count index = 0;

      while (index < mValues.mReserved) {
         if (not mInfo[index]) {
            ++index;
            continue;
         }

         const auto dind = REVERSE ? mValues.mReserved - index - 1 : index;
         if constexpr (HasBreaker) {
            if (not call(part.template Get<A>(dind)))
               return ++done;
         }
         else call(part.template Get<A>(dind));

         ++index;
         ++done;
      }

      return done;
   }
   
   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param part - the block to iterate                                   
   ///   @param call - the function to call for each key block                
   ///   @return the number of successful executions                          
   template<CT::Map THIS, bool REVERSE>
   Count BlockMap::ForEachElement(const CT::Block auto& part, auto&& call) const {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block binary-compatible type");
      static_assert(CT::Constant<A> or CT::Mutable<THIS>,
         "Non constant iterator for constant memory block");

      auto info = mInfo;
      const auto infoEnd = mInfo + GetReserved();
      while (info != infoEnd) {
         if (not *info) {
            ++info;
            continue;
         }

         const Offset index = info - mInfo;
         A block = part.GetElement(index);
         if constexpr (CT::Bool<R>) {
            if (not call(block))
               return index + 1;
         }
         else call(block);

         ++info;
      }

      return info - mInfo;
   }
   
   /// Iterate and execute call for each deep element                         
   ///   @tparam R - the function return type                                 
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type                               
   ///               A must be a CT::Block type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - true to avoid executing call in intermediate blocks   
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<CT::Map THIS, class R, CT::Data A, bool REVERSE, bool SKIP>
   Count BlockMap::ForEachDeepInner(const CT::Block auto& part, auto&& call) const {
      constexpr bool HasBreaker = CT::Bool<R>;
      constexpr bool Mutable = CT::Mutable<THIS>;
      using SubBlock = Conditional<Mutable, Block&, const Block&>;
      Count counter = 0;

      if constexpr (CT::Deep<Decay<A>>) {
         if (not SKIP or not part.IsDeep()) {
            // Always execute for intermediate/non-deep *this           
            ++counter;

            if constexpr (CT::Dense<A>) {
               if constexpr (HasBreaker) {
                  if (not call(part))
                     return counter;
               }
               else call(part);
            }
            else {
               if constexpr (HasBreaker) {
                  if (not call(&part))
                     return counter;
               }
               else call(&part);
            }
         }

         if (part.IsDeep()) {
            // Iterate using a block type                               
            ForEachInner<THIS, void, SubBlock, REVERSE>(part,
               [&counter, &call](SubBlock group) {
                  counter += DenseCast(group).template
                     ForEachDeepInner<SubBlock, R, A, REVERSE, SKIP>(
                        ::std::move(call));
               }
            );
         }
      }
      else {
         if (part.IsDeep()) {
            // Iterate deep keys/values using non-block type            
            ForEachInner<THIS, void, SubBlock, REVERSE>(part,
               [&counter, &call](SubBlock group) {
                  counter += DenseCast(group).template
                     ForEachDeepInner<SubBlock, R, A, REVERSE, SKIP>(
                        ::std::move(call));
               }
            );
         }
         else {
            // Equivalent to non-deep iteration                         
            counter += ForEachInner<THIS, R, A, REVERSE>(part,
               ::std::move(call));
         }
      }

      return counter;
   }

   /// Iterate all keys inside the map, and perform call() on each            
   /// You can break the loop, by returning false inside call()               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the function to call for each key block                
   ///   @return the number of successful executions                          
   template<CT::Map THIS, bool REVERSE> LANGULUS(INLINED)
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
   template<CT::Map THIS, bool REVERSE> LANGULUS(INLINED)
   Count BlockMap::ForEachValueElement(auto&& call) const {
      using F = Deref<decltype(call)>;
      return ForEachElement<THIS, REVERSE>(
         GetValues<THIS>(), Forward<F>(call));
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside call()               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the functions to call for each key block               
   ///   @return the number of successful call() executions                   
   template<CT::Map THIS, bool REVERSE, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachKey(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (0 != (result = 
         ForEachInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE>(
            GetKeys<THIS>(), Forward<F>(call)))
      ));
      return result;
   }

   /// Iterate values inside the map, and perform a set of functions on them  
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the functions to call for each value block             
   ///   @return the number of successful f() executions                      
   template<CT::Map THIS, bool REVERSE, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachValue(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (0 != (result = 
         ForEachInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE>(
            GetValues<THIS>(), Forward<F>(call)))
      ));
      return result;
   }

   /// Iterate each subblock of keys inside the map, and perform a set of     
   /// functions on them                                                      
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the functions to call for each key block               
   ///   @return the number of successful f() executions                      
   template<CT::Map THIS, bool REVERSE, bool SKIP, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachKeyDeep(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (0 != (result = 
         ForEachDeepInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP>(
            GetKeys<THIS>(), Forward<F>(call)))
      ));
      return result;
   }

   /// Iterate each subblock of values inside the map, and perform a set of   
   /// functions on them                                                      
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - whether to execute calls for intermediate containers  
   ///   @param call - the functions to call for each value block             
   ///   @return the number of successful f() executions                      
   template<CT::Map THIS, bool REVERSE, bool SKIP, class...F> LANGULUS(INLINED)
   Count BlockMap::ForEachValueDeep(F&&...call) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (0 != (result = 
         ForEachDeepInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP>(
            GetValues<THIS>(), Forward<F>(call)))
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
      if constexpr (CT::Typed<THIS>) {
         return {
            info, GetInfoEnd(),
            &GetRawKey<THIS>(offset),
            &GetRawValue<THIS>(offset)
         };
      }
      else {
         return {
            info, GetInfoEnd(),
            GetRawKey<THIS>(offset),
            GetRawValue<THIS>(offset)
         };
      }
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
         GetRawValue<THIS>(offset)
      };
   }

   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::Iterator<const THIS> BlockMap::last() const noexcept {
      return const_cast<BlockMap*>(this)->last<THIS>();
   }


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   /*template<class MAP> LANGULUS(INLINED)
   BlockMap::Iterator<MAP>::Iterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      K&& key, 
      V&& value
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
      , mKey {key}
      , mValue {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<class MAP> LANGULUS(INLINED)
   BlockMap::Iterator<MAP>& BlockMap::Iterator<MAP>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (not *++mInfo)
         ;
      const auto offset = mInfo - previous;
      mKey.mRaw += offset * mKey.GetStride();
      mValue.mRaw += offset * mValue.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<class MAP> LANGULUS(INLINED)
   BlockMap::Iterator<MAP> BlockMap::Iterator<MAP>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<class MAP> LANGULUS(INLINED)
   bool BlockMap::Iterator<MAP>::operator == (const Iterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<class MAP> LANGULUS(INLINED)
   BlockMap::Iterator<MAP>::Pair BlockMap::Iterator<MAP>::operator * () const noexcept {
      return {Disown(mKey), Disown(mValue)};
   }

   /// Explicit bool operator, to check if iterator is valid                  
   template<class MAP> LANGULUS(INLINED)
   constexpr BlockMap::Iterator<MAP>::operator bool() const noexcept {
      return mInfo != mSentinel;
   }*/

} // namespace Langulus::Anyness
