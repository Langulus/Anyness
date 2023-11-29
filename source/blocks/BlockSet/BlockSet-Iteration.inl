///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockSet.hpp"


namespace Langulus::Anyness
{
      
   /// Iterate all keys inside the set, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE>
   Count BlockSet::ForEachElement(auto&& call) {
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
         A block = mKeys.GetElement(index);
         if constexpr (CT::Bool<R>) {
            if (not call(block))
               return index + 1;
         }
         else call(block);

         ++info;
      }

      return info - mInfo;
   }

   /// Iterate each immutable element block and execute F for it              
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - function to execute for each constant element block    
   ///   @return the number of executions                                     
   template<bool REVERSE>
   LANGULUS(INLINED)
   Count BlockSet::ForEachElement(auto&& call) const {
      return const_cast<BlockSet&>(*this).template
         ForEachElement<REVERSE, false>(call);
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEach(F&&... f) {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (
         0 != (result = ForEachInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, MUTABLE>(
            Forward<F>(f))
         )
      ));
      return result;
   }

   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEach(F&&... f) const {
      return const_cast<BlockSet&>(*this).template
         ForEach<REVERSE, false>(Forward<F>(f)...);
   }

   /// Iterate each subblock of keys inside the set, and perform a set of     
   /// functions on them                                                      
   ///   @param calls - the functions to call for each key block              
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachDeep(F&&... calls) {
      Count result = 0;
      ((result += ForEachDeepInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP, MUTABLE>(
         Forward<F>(calls))
      ), ...);
      return result;
   }

   template<bool REVERSE, bool SKIP, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachDeep(F&&... f) const {
      return const_cast<BlockSet&>(*this).template
         ForEachDeep<REVERSE, SKIP, false>(Forward<F>(f)...);
   }
   
   template<bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachElementRev(F&&... f) {
      return ForEachElement<true, MUTABLE, F...>(Forward<F>(f)...);
   }

   template<class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachElementRev(F&&... f) const {
      return ForEachElement<true, F...>(Forward<F>(f)...);
   }

   template<bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachRev(F&&... f) {
      return ForEach<true, MUTABLE, F...>(Forward<F>(f)...);
   }

   template<class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachRev(F&&... f) const {
      return ForEach<true, F...>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachDeepRev(F&&... f) {
      return ForEachDeep<true, SKIP, MUTABLE, F...>(Forward<F>(f)...);
   }

   template<bool SKIP, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachDeepRev(F&&... f) const {
      return ForEachDeep<true, SKIP, F...>(Forward<F>(f)...);
   }
   
   /// Iterate and execute call for each flat element, counting each          
   /// successfull execution                                                  
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is typed                                    
   ///   @tparam R - the function return type                                 
   ///               if R is boolean, loop will cease on f() returning false  
   ///   @tparam A - the function argument type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///	@tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating (iteration is slower if true)      
   ///   @param f - the function to execute for each element of type A        
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
   LANGULUS(INLINED)
   Count BlockSet::ForEachInner(auto&& f) noexcept(NoexceptIterator<decltype(f)>) {
      constexpr auto NOE = NoexceptIterator<decltype(f)>;
      if ((CT::Deep<Decay<A>> and mKeys.IsDeep())
      or (not CT::Deep<Decay<A>> and mKeys.CastsTo<A>())) {
         Count index = 0;
         Count executions = 0;
         if (mKeys.mType->mIsSparse) {
            // Iterate using pointers of A                              
            using DA = Conditional<MUTABLE, Decay<A>*, const Decay<A>*>;
            mKeys.IterateInner<R, DA, REVERSE, MUTABLE>(
               mKeys.mReserved,
               [&](DA element) noexcept(NOE) -> R {
                  if (not mInfo[index++]) {
                     if constexpr (CT::Bool<R>)
                        return true;
                     else
                        return;
                  }

                  ++executions;
                  if constexpr (CT::Sparse<A>)  return f(element);
                  else                          return f(*element);
               }
            );
         }
         else {
            // Iterate using references of A                            
            using DA = Conditional<MUTABLE, Decay<A>&, const Decay<A>&>;
            mKeys.IterateInner<R, DA, REVERSE, MUTABLE>(
               mKeys.mReserved,
               [&](DA element) noexcept(NOE) -> R {
                  if (not mInfo[index++]) {
                     if constexpr (CT::Bool<R>)
                        return true;
                     else
                        return;
                  }

                  ++executions;
                  if constexpr (CT::Sparse<A>)  return f(&element);
                  else                          return f(element);
               }
            );
         }

         return executions;
      }
      else return 0;
   }
   
   /// Iterate and execute call for each deep element                         
   ///   @tparam R - the function return type (deduced)                       
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type (deduced)                     
   ///               A must be a CT::Block type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///	@tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating (iteration is slower if true)      
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
   Count BlockSet::ForEachDeepInner(auto&& call) {
      constexpr bool HasBreaker = CT::Bool<R>;
      using DA = Decay<A>;

      Count counter = 0;
      if constexpr (CT::Deep<DA>) {
         using BlockType = Conditional<MUTABLE, DA*, const DA*>;

         if (not SKIP or not IsDeep()) {
            // Always execute for intermediate/non-deep *this           
            ++counter;
            if constexpr (CT::Dense<A>) {
               if constexpr (HasBreaker) {
                  if (not call(*reinterpret_cast<BlockType>(this)))
                     return counter;
               }
               else call(*reinterpret_cast<BlockType>(this));
            }
            else {
               if constexpr (HasBreaker) {
                  if (not call(reinterpret_cast<BlockType>(this)))
                     return counter;
               }
               else call(reinterpret_cast<BlockType>(this));
            }
         }

         if (IsDeep()) {
            // Iterate using a block type                               
            ForEachInner<void, BlockType, REVERSE, MUTABLE>(
               [&counter, &call](BlockType group) {
                  counter += const_cast<DA*>(group)->
                     template ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(
                        ::std::move(call));
               }
            );
         }
      }
      else {
         if (IsDeep()) {
            // Iterate deep using non-block type                        
            using BlockType = Conditional<MUTABLE, Block&, const Block&>;
            ForEachInner<void, BlockType, REVERSE, MUTABLE>(
               [&counter, &call](BlockType group) {
                  counter += const_cast<Block&>(group).
                     template ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(
                        ::std::move(call));
               }
            );
         }
         else {
            // Equivalent to non-deep iteration                         
            counter += ForEachInner<R, A, REVERSE, MUTABLE>(::std::move(call));
         }
      }

      return counter;
   }
   
   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   LANGULUS(INLINED)
   typename BlockSet::Iterator BlockSet::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetInner(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   LANGULUS(INLINED)
   typename BlockSet::Iterator BlockSet::end() noexcept {
      return {GetInfoEnd(), GetInfoEnd(), {}};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   LANGULUS(INLINED)
   typename BlockSet::Iterator BlockSet::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetInner(offset)
      };
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   LANGULUS(INLINED)
   typename BlockSet::ConstIterator BlockSet::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         GetInner(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   LANGULUS(INLINED)
   typename BlockSet::ConstIterator BlockSet::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), {}};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   LANGULUS(INLINED)
   typename BlockSet::ConstIterator BlockSet::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetInner(offset)
      };
   }
   

   ///                                                                        
   ///   Set iterator                                                         
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   template<bool MUTABLE>
   LANGULUS(INLINED)
   BlockSet::TIterator<MUTABLE>::TIterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const Block& value
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
      , mKey {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename BlockSet::TIterator<MUTABLE>& BlockSet::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (not *++mInfo);
      const auto offset = mInfo - previous;
      mKey.mRaw += offset * mKey.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename BlockSet::TIterator<MUTABLE> BlockSet::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE>
   LANGULUS(INLINED)
   bool BlockSet::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE>
   LANGULUS(INLINED)
   Any BlockSet::TIterator<MUTABLE>::operator * () const noexcept {
      return {Disown(mKey)};
   }

   /// Explicit bool operator, to check if iterator is valid                  
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr BlockSet::TIterator<MUTABLE>::operator bool() const noexcept {
      return mInfo != mSentinel;
   }

} // namespace Langulus::Anyness
