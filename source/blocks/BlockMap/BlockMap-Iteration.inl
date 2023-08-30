///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"

namespace Langulus::Anyness
{

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   LANGULUS(INLINED)
   typename BlockMap::Iterator BlockMap::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetKeyInner(offset),
         GetValueInner(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   LANGULUS(INLINED)
   typename BlockMap::Iterator BlockMap::end() noexcept {
      return {GetInfoEnd(), GetInfoEnd(), {}, {}};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   LANGULUS(INLINED)
   typename BlockMap::Iterator BlockMap::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetKeyInner(offset),
         GetValueInner(offset)
      };
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   inline typename BlockMap::ConstIterator BlockMap::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         GetKeyInner(offset),
         GetValueInner(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   LANGULUS(INLINED)
   typename BlockMap::ConstIterator BlockMap::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), {}, {}};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   inline typename BlockMap::ConstIterator BlockMap::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetKeyInner(offset),
         GetValueInner(offset)
      };
   }

   /// Execute a call for each type-erased pair inside the map                
   ///   @tparam REVERSE - whether or not to iterate in reverse               
   ///   @tparam F - the call to execute for each pair (deducible)            
   ///   @param call - the function to execute for each pair                  
   ///   @return the number of successfull executions                         
   template<bool REVERSE, class F>
   Count BlockMap::ForEach(F&& call) const {
      if (IsEmpty())
         return 0;

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

   /// Execute functions for each element inside container                    
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function types (deducible)                           
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool MUTABLE, bool REVERSE, class F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachSplitter(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant memory block");

      return ForEachInner<R, A, REVERSE, MUTABLE>(part, Forward<F>(call));
   }

   /// Iterate and execute call for each element                              
   ///   @param call - the function to execute for each element of type T     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE, class F>
   Count BlockMap::ForEachInner(Block& part, F&& call) {
      if (IsEmpty() or not part.mType->CastsTo<A, true>())
         return 0;
       
      constexpr bool HasBreaker = CT::Bool<R>;
      Count done {};
      Count index {};

      while (index < mValues.mReserved) {
         if (not mInfo[index]) {
            ++index;
            continue;
         }

         if constexpr (REVERSE) {
            if constexpr (HasBreaker) {
               if (not call(part.Get<A>(mValues.mReserved - index - 1)))
                  return ++done;
            }
            else call(part.Get<A>(mValues.mReserved - index - 1));
         }
         else {
            if constexpr (HasBreaker) {
               if (not call(part.Get<A>(index)))
                  return ++done;
            }
            else call(part.Get<A>(index));
         }

         ++index;
         ++done;
      }

      return done;
   }
   
   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class F>
   Count BlockMap::ForEachElement(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block type");
      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant memory block");

      Count index {};
      while (index < GetReserved()) {
         if (not mInfo[index]) {
            ++index;
            continue;
         }

         A block = part.GetElement(index);
         if constexpr (CT::Bool<R>) {
            if (not call(block))
               return ++index;
         }
         else call(block);

         ++index;
      }

      return index;
   }
   
   /// Execute functions for each element inside container, nested for any    
   /// contained deep containers                                              
   ///   @tparam SKIP - set to false, to execute F for containers, too        
   ///                  set to true, to execute only for non-deep elements    
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function type (deducible                             
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachDeepSplitter(const Count count, Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant memory block");

      if constexpr (CT::Deep<A>) {
         // If argument type is deep                                    
         return ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(
            count, part, Forward<F>(call)
         );
      }
      else if constexpr (CT::Constant<A>) {
         // Any other type is wrapped inside another ForEachDeep call   
         return ForEachDeep<SKIP, MUTABLE>(
            count, part, 
            [&call](const Block& block) {
               block.ForEach(Forward<F>(call));
            }
         );
      }
      else {
         // Any other type is wrapped inside another ForEachDeep call   
         return ForEachDeep<SKIP, MUTABLE>(
            count, part,
            [&call](Block& block) {
               block.ForEach(Forward<F>(call));
            }
         );
      }
   }

   /// Iterate and execute call for each element                              
   ///   @param call - the function to execute for each element of type T     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE, class F>
   Count BlockMap::ForEachDeepInner(const Count count, Block& part, F&& call) {
      constexpr bool HasBreaker = CT::Bool<R>;
      Offset index = 0;
      while (index < count) {
         auto block = ReinterpretCast<A>(part.GetBlockDeep(index));//TODO custom checked getblockdeep here, write tests and you'll see
         if constexpr (SKIP) {
            // Skip deep/empty sub blocks                               
            if (block->IsDeep() or block->IsEmpty()) {
               ++index;
               continue;
            }
         }

         if constexpr (HasBreaker) {
            if (not call(*block))
               return ++index;
         }
         else call(*block);

         ++index;
      }

      return index;
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKeyElement(F&& f) {
      return ForEachElement<REVERSE, MUTABLE>(mKeys, Forward<F>(f));
   }

   template<bool REVERSE, class F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKeyElement(F&& f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachKeyElement<REVERSE, false>(Forward<F>(f));
   }

   /// Iterate all values inside the map, and perform f() on them             
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValueElement(F&& f) {
      return ForEachElement<REVERSE, MUTABLE>(mValues, Forward<F>(f));
   }

   template<bool REVERSE, class F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValueElement(F&& f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachValueElement<REVERSE, false>(Forward<F>(f));
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKey(F&&... f) {
      Count result {};
      (void) (... or (0 != (result = ForEachSplitter<MUTABLE, REVERSE>(
         mKeys, Forward<F>(f)))));
      return result;
   }

   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKey(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachKey<REVERSE, false>(Forward<F>(f)...);
   }

   template<bool REVERSE, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValue(F&&... f) {
      Count result {};
      (void) (... or (0 != (result = ForEachSplitter<MUTABLE, REVERSE>(
         mValues, Forward<F>(f)))));
      return result;
   }

   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValue(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachValue<REVERSE, false>(Forward<F>(f)...);
   }

   template<bool REVERSE, bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKeyDeep(F&&... f) {
      Count result {};
      (void) (... or (0 != (result = ForEachDeepSplitter<SKIP, MUTABLE, REVERSE>(
         GetKeyCountDeep(), mKeys, Forward<F>(f)))));
      return result;
   }

   template<bool REVERSE, bool SKIP, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKeyDeep(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachKeyDeep<REVERSE, SKIP, false>(Forward<F>(f)...);
   }

   template<bool REVERSE, bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValueDeep(F&&... f) {
      Count result {};
      (void) (... or (0 != (result = ForEachDeepSplitter<SKIP, MUTABLE, REVERSE>(
         GetValueCountDeep(), mValues, Forward<F>(f)))));
      return result;
   }

   template<bool REVERSE, bool SKIP, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValueDeep(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachValueDeep<REVERSE, SKIP, false>(Forward<F>(f)...);
   }


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   template<bool MUTABLE>
   LANGULUS(INLINED)
   BlockMap::TIterator<MUTABLE>::TIterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const Block& key, 
      const Block& value
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
      , mKey {key}
      , mValue {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename BlockMap::TIterator<MUTABLE>& BlockMap::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (not *++mInfo);
      const auto offset = mInfo - previous;
      mKey.mRaw += offset * mKey.GetStride();
      mValue.mRaw += offset * mValue.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename BlockMap::TIterator<MUTABLE> BlockMap::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE>
   LANGULUS(INLINED)
   bool BlockMap::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE>
   LANGULUS(INLINED)
   Pair BlockMap::TIterator<MUTABLE>::operator * () const noexcept {
      return {Disown(mKey), Disown(mValue)};
   }

} // namespace Langulus::Anyness
