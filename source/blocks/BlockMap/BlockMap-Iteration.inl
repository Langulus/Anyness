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

   /// Iterate and execute call for each element                              
   ///   @attention assumes map is not empty, and part is typed               
   ///   @param call - the function to execute for each element of type T     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE, class F>
   Count BlockMap::ForEachInner(Block& part, F&& call) {
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(), "Map is empty");
      if (not part.mType->CastsTo<A, true>())
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
   template<bool REVERSE, bool MUTABLE>
   Count BlockMap::ForEachElement(Block& part, auto&& call) {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block binary-compatible type");
      static_assert(CT::Constant<A> or MUTABLE,
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
   ///   @tparam R - the function return type (deduced)                       
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type (deduced)                     
   ///               A must be a CT::Block type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating (iteration is slower if true)      
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
   Count BlockMap::ForEachDeepInner(Block& part, auto&& call) {
      constexpr bool HasBreaker = CT::Bool<R>;
      using DA = Decay<A>;

      Count counter = 0;
      if constexpr (CT::Deep<DA>) {
         using BlockType = Conditional<MUTABLE, DA*, const DA*>;

         if (not SKIP or not part.IsDeep()) {
            // Always execute for intermediate/non-deep *this           
            ++counter;
            if constexpr (CT::Dense<A>) {
               if constexpr (HasBreaker) {
                  if (not call(*reinterpret_cast<BlockType>(&part)))
                     return counter;
               }
               else call(*reinterpret_cast<BlockType>(&part));
            }
            else {
               if constexpr (HasBreaker) {
                  if (not call(reinterpret_cast<BlockType>(&part)))
                     return counter;
               }
               else call(reinterpret_cast<BlockType>(&part));
            }
         }

         if (part.IsDeep()) {
            // Iterate using a block type                               
            ForEachInner<void, BlockType, REVERSE, MUTABLE>(part,
               [&counter, &call](BlockType group) {
                  counter += const_cast<DA*>(group)->
                     template ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(
                        ::std::move(call));
               }
            );
         }
      }
      else {
         if (part.IsDeep()) {
            // Iterate deep keys/values using non-block type            
            using BlockType = Conditional<MUTABLE, Block&, const Block&>;
            ForEachInner<void, BlockType, REVERSE, MUTABLE>(part,
               [&counter, &call](BlockType group) {
                  counter += const_cast<Block&>(group).
                     template ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(
                        ::std::move(call));
               }
            );
         }
         else {
            // Equivalent to non-deep iteration                         
            counter += ForEachInner<R, A, REVERSE, MUTABLE>(part, ::std::move(call));
         }
      }

      return counter;
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
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (
         0 != (result = ForEachInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, MUTABLE>(
            mKeys, Forward<F>(f))
         )
      ));
      return result;
   }

   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKey(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachKey<REVERSE, false>(Forward<F>(f)...);
   }

   /// Iterate values inside the map, and perform a set of functions on them  
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each value block                
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValue(F&&... f) {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (
         0 != (result = ForEachInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, MUTABLE>(
            mValues, Forward<F>(f))
         )
      ));
      return result;
   }

   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValue(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachValue<REVERSE, false>(Forward<F>(f)...);
   }

   /// Iterate each subblock of keys inside the map, and perform a set of     
   /// functions on them                                                      
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKeyDeep(F&&... f) {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (
         0 != (result = ForEachDeepInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP, MUTABLE>(
            mKeys, Forward<F>(f))
         )
      ));
      return result;
   }

   template<bool REVERSE, bool SKIP, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachKeyDeep(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachKeyDeep<REVERSE, SKIP, false>(Forward<F>(f)...);
   }

   /// Iterate each subblock of values inside the map, and perform a set of   
   /// functions on them                                                      
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValueDeep(F&&... f) {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (
         0 != (result = ForEachDeepInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP, MUTABLE>(
            mValues, Forward<F>(f))
         )
      ));
      return result;
   }

   template<bool REVERSE, bool SKIP, class... F>
   LANGULUS(INLINED)
   Count BlockMap::ForEachValueDeep(F&&... f) const {
      return const_cast<BlockMap&>(*this).template
         ForEachValueDeep<REVERSE, SKIP, false>(Forward<F>(f)...);
   }

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
   LANGULUS(INLINED)
   typename BlockMap::ConstIterator BlockMap::begin() const noexcept {
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
   LANGULUS(INLINED)
   typename BlockMap::ConstIterator BlockMap::last() const noexcept {
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

   /// Explicit bool operator, to check if iterator is valid                  
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr BlockMap::TIterator<MUTABLE>::operator bool() const noexcept {
      return mInfo != mSentinel;
   }

} // namespace Langulus::Anyness
