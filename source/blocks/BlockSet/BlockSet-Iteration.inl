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
   
   /// Iterate and execute call for each flat element, counting each          
   /// successfull execution                                                  
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is typed                                    
   ///   @tparam R - the function return type                                 
   ///               if R is boolean, loop will cease on f() returning false  
   ///   @tparam A - the function argument type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param f - the function to execute for each element of type A        
   ///   @return the number of executions that occured                        
   template<CT::Set THIS, class R, CT::Data A, bool REVERSE> LANGULUS(INLINED)
   Count BlockSet::ForEachInner(auto&& f) const
   noexcept(NoexceptIterator<decltype(f)>) {
      constexpr auto NOE = NoexceptIterator<decltype(f)>;

      if ((CT::Deep<Decay<A>> and mKeys.IsDeep())
      or (not CT::Deep<Decay<A>> and mKeys.CastsTo<A>())) {
         Count index = 0;
         Count executions = 0;
         if (mKeys.mType->mIsSparse) {
            // Iterate using pointers of A                              
            using DA = Conditional<CT::Mutable<THIS>, Decay<A>*, const Decay<A>*>;
            mKeys.IterateInner<THIS, R, DA, REVERSE>(
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
            using DA = Conditional<CT::Mutable<THIS>, Decay<A>&, const Decay<A>&>;
            mKeys.IterateInner<THIS, R, DA, REVERSE>(
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
         
   /// Iterate all keys inside the set, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, CT::Set THIS>
   Count BlockSet::ForEachElement(auto&& call) const {
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
   
   /// Iterate and execute call for each deep element                         
   ///   @tparam R - the function return type (deduced)                       
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type (deduced)                     
   ///               A must be a CT::Block type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<CT::Set THIS, class R, CT::Data A, bool REVERSE, bool SKIP>
   Count BlockSet::ForEachDeepInner(auto&& call) const {
      constexpr bool HasBreaker = CT::Bool<R>;
      constexpr bool Mutable = CT::Mutable<THIS>;
      using SubBlock = Conditional<Mutable, Block&, const Block&>;
      Count counter = 0;

      if constexpr (CT::Deep<Decay<A>>) {
         if (not SKIP or not IsDeep<THIS>()) {
            // Always execute for intermediate/non-deep *this           
            ++counter;

            if constexpr (CT::Dense<A>) {
               if constexpr (HasBreaker) {
                  if (not call(mKeys))
                     return counter;
               }
               else call(mKeys);
            }
            else {
               if constexpr (HasBreaker) {
                  if (not call(&mKeys))
                     return counter;
               }
               else call(&mKeys);
            }
         }

         if (IsDeep<THIS>()) {
            // Iterate using a block type                               
            ForEachInner<THIS, void, SubBlock, REVERSE>(
               [&counter, &call](SubBlock group) {
                  counter += DenseCast(group).template
                     ForEachDeepInner<SubBlock, R, A, REVERSE, SKIP>(
                        ::std::move(call));
               }
            );
         }
      }
      else {
         if (IsDeep<THIS>()) {
            // Iterate deep keys/values using non-block type            
            ForEachInner<THIS, void, SubBlock, REVERSE>(
               [&counter, &call](SubBlock group) {
                  counter += DenseCast(group).template
                     ForEachDeepInner<SubBlock, R, A, REVERSE, SKIP>(
                        ::std::move(call));
               }
            );
         }
         else {
            // Equivalent to non-deep iteration                         
            counter += ForEachInner<THIS, R, A, REVERSE>(
               ::std::move(call));
         }
      }

      return counter;
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, CT::Set THIS, class...F> LANGULUS(INLINED)
   Count BlockSet::ForEach(F&&...f) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (0 != (result = 
         ForEachInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE>(
            Forward<F>(f)))
      ));
      return result;
   }

   /// Iterate each subblock of keys inside the set, and perform a set of     
   /// functions on them                                                      
   ///   @param calls - the functions to call for each key block              
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool SKIP, CT::Set THIS, class...F>
   LANGULUS(INLINED) Count BlockSet::ForEachDeep(F&&...calls) const {
      Count result = 0;
      ((result += ForEachDeepInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP>(
         Forward<F>(calls))
      ), ...);
      return result;
   }

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   template<CT::Set SET> LANGULUS(INLINED)
   typename BlockSet::Iterator<SET> BlockSet::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info)
         ++info;

      const auto offset = info - GetInfo();
      return {info, GetInfoEnd(), GetRaw<SET>(offset)};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   template<CT::Set SET> LANGULUS(INLINED)
   typename BlockSet::Iterator<SET> BlockSet::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info)
         ;

      const auto offset = info - GetInfo();
      return {info, GetInfoEnd(), GetRaw<SET>(offset)};
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   template<CT::Set SET> LANGULUS(INLINED)
   typename BlockSet::Iterator<const SET> BlockSet::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info)
         ++info;

      const auto offset = info - GetInfo();
      return {info, GetInfoEnd(), GetRaw<SET>(offset)};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   template<CT::Set SET> LANGULUS(INLINED)
   typename BlockSet::Iterator<const SET> BlockSet::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info)
         ;

      const auto offset = info - GetInfo();
      return {info, GetInfoEnd(), GetRaw<SET>(offset)};
   }
   
   
   ///                                                                        
   ///   Set iterator                                                         
   ///                                                                        

   /// Construct a set iterator                                               
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer/block to the key element                        
   template<class SET> LANGULUS(INLINED)
   constexpr BlockSet::Iterator<SET>::Iterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const InnerT&   key
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
      , mKey {key} {}

   /// Construct from end point                                               
   template<class SET> LANGULUS(INLINED)
   constexpr BlockSet::Iterator<SET>::Iterator(const A::IteratorEnd&) noexcept
      : mInfo {}
      , mSentinel {}
      , mKey {} {}

   /// Prefix increment operator                                              
   /// Moves pointers to the right, unless end has been reached               
   ///   @return the modified iterator                                        
   template<class SET> LANGULUS(INLINED)
   constexpr BlockSet::Iterator<SET>& BlockSet::Iterator<SET>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (not *++mInfo)
         ;

      const auto offset = mInfo - previous;
      if constexpr (CT::Typed<SET>) {
         const_cast<InnerT&>(mKey) += offset;
      }
      else {
         const_cast<InnerT&>(mKey).mRaw += offset * mKey.GetStride();
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
   template<class SET> LANGULUS(INLINED)
   constexpr BlockSet::Iterator<SET> BlockSet::Iterator<SET>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare iterators                                                      
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<class SET> LANGULUS(INLINED)
   constexpr bool BlockSet::Iterator<SET>::operator == (const Iterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Check if iterator has reached the end                                  
   ///   @return true if entries match                                        
   template<class SET> LANGULUS(INLINED)
   constexpr bool BlockSet::Iterator<SET>::operator == (const A::IteratorEnd&) const noexcept {
      return mInfo >= mSentinel;
   }

   /// Iterator access operator                                               
   /// It is required for ranged-for expressions                              
   /// In our case, it just generates a temporary pair of references          
   ///   @return the pair at the current iterator position                    
   template<class SET> LANGULUS(INLINED)
   constexpr decltype(auto) BlockSet::Iterator<SET>::operator * () const {
      if (mInfo >= mSentinel)
         LANGULUS_OOPS(Access, "Trying to access end of iteration");
      return DenseCast(mKey);
   }

   /// Explicit bool operator, to check if iterator is valid                  
   template<class SET> LANGULUS(INLINED)
   constexpr BlockSet::Iterator<SET>::operator bool() const noexcept {
      return mInfo < mSentinel;
   }

   /// Implicitly convert to a constant iterator                              
   template<class SET> LANGULUS(INLINED)
   constexpr BlockSet::Iterator<SET>::operator Iterator<const SET>() const noexcept requires Mutable {
      return {mInfo, mSentinel, mKey};
   }

} // namespace Langulus::Anyness
