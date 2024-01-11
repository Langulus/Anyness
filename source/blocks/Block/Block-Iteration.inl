///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Block.hpp"


namespace Langulus::Anyness
{
   
   /// Iterate each element block and execute F for it                        
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - function to execute for each element block             
   ///   @return the number of executions                                     
   template<bool REVERSE, CT::Block THIS>
   Count Block::ForEachElement(auto&& call) const {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block binary-compatible type");
      static_assert(CT::Constant<A> or CT::Mutable<THIS>,
         "Non constant iterator for constant memory block");

      Count index {};
      while (index < mCount) {
         A block = GetElement(index);
         if constexpr (CT::Bool<R>) {
            // If F returns bool, you can decide when to break the loop 
            // by simply returning false                                
            if (not call(block))
               return index + 1;
         }
         else call(block);

         ++index;
      }

      return index;
   }

   /// Execute functions for each element inside container                    
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE, CT::Block THIS, class...F> LANGULUS(INLINED)
   Count Block::ForEach(F&&...calls) const {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (0 != (
         result = ForEachInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE>(
            Forward<F>(calls)))
      ));
      return result;
   }

   /// Execute functions in each sub-block, inclusively                       
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. None of the provided     
   /// functions are ignored.                                                 
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE, bool SKIP, CT::Block THIS, class...F>
   LANGULUS(INLINED) Count Block::ForEachDeep(F&&...calls) const {
      Count result = 0;
      ((result += ForEachDeepInner<THIS, ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP>(
         Forward<F>(calls))
      ), ...);
      return result;
   }

   /// Same as ForEachElement, but in reverse                                 
   template<CT::Block THIS, class...F> LANGULUS(INLINED)
   Count Block::ForEachElementRev(F&&...f) const {
      return ForEachElement<true, THIS>(Forward<F>(f)...);
   }

   /// Same as ForEach, but in reverse                                        
   template<CT::Block THIS, class...F> LANGULUS(INLINED)
   Count Block::ForEachRev(F&&...f) const {
      return ForEach<true, THIS>(Forward<F>(f)...);
   }

   /// Same as ForEachDeep, but in reverse                                    
   template<bool SKIP, CT::Block THIS, class...F> LANGULUS(INLINED)
   Count Block::ForEachDeepRev(F&&...f) const {
      return ForEachDeep<true, SKIP, THIS>(Forward<F>(f)...);
   }

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
   template<CT::Block THIS, class R, CT::Data A, bool REVERSE> LANGULUS(INLINED)
   Count Block::ForEachInner(auto&& f) const
   noexcept(NoexceptIterator<decltype(f)>) {
      constexpr auto NOE = NoexceptIterator<decltype(f)>;

      if ((CT::Deep<Decay<A>> and IsDeep<THIS>())
      or (not CT::Deep<Decay<A>> and CastsTo<A>())) {
         Count index = 0;

         if (mType->mIsSparse) {
            // Iterate using pointers of A                              
            using DA = Conditional<CT::Mutable<THIS>, Decay<A>*, const Decay<A>*>;
            IterateInner<THIS, R, DA, REVERSE>(
               mCount, [&index, &f](DA element) noexcept(NOE) -> R {
                  ++index;
                  if constexpr (CT::Sparse<A>)  return f(element);
                  else                          return f(*element);
               }
            );
         }
         else {
            // Iterate using references of A                            
            using DA = Conditional<CT::Mutable<THIS>, Decay<A>&, const Decay<A>&>;
            IterateInner<THIS, R, DA, REVERSE>(
               mCount, [&index, &f](DA element) noexcept(NOE) -> R {
                  ++index;
                  if constexpr (CT::Sparse<A>)  return f(&element);
                  else                          return f(element);
               }
            );
         }

         return index;
      }
      else return 0;
   }
   
   /// Iterate and execute call for each deep element                         
   ///   @tparam R - the function return type (deduced)                       
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type (deduced)                     
   ///               A must be a CT::Block type                               
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - whether to execute call for intermediate blocks       
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<CT::Block THIS, class R, CT::Data A, bool REVERSE, bool SKIP>
   Count Block::ForEachDeepInner(auto&& call) const {
      constexpr bool HasBreaker = CT::Bool<R>;
      constexpr bool Mutable = CT::Mutable<THIS>;
      using SubBlock = Conditional<Mutable, Block&, const Block&>;
      Count counter = 0;

      if constexpr (CT::Deep<Decay<A>>) {
         if (not SKIP or not IsDeep<THIS>()) {
            // Always execute for intermediate/non-deep *this           
            ++counter;

            auto b = reinterpret_cast<Decay<A>*>(const_cast<Block*>(this));
            if constexpr (CT::Dense<A>) {
               if constexpr (HasBreaker) {
                  if (not call(*b))
                     return counter;
               }
               else call(*b);
            }
            else {
               if constexpr (HasBreaker) {
                  if (not call(b))
                     return counter;
               }
               else call(b);
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
            // Iterate deep using non-block type                        
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

   /// Execute a function for each element inside container                   
   /// Lowest-level element iteration function (for internal use only)        
   ///   @attention assumes A is binary compatible with the contained type    
   ///   @attention assumes block is not empty                                
   ///   @attention assumes sparseness matches                                
   ///   @tparam R - optional call return (deducible)                         
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - iterator type (deducible)                                
   ///   @tparam REVERSE - direction we're iterating in                       
   ///   @param call - the constexpr noexcept function to call on each item   
   template<CT::Block THIS, class R, CT::Data A, bool REVERSE> LANGULUS(INLINED)
   void Block::IterateInner(const Count count, auto&& f) const
   noexcept(NoexceptIterator<decltype(f)>) {
      static_assert(CT::Complete<Decay<A>> or CT::Sparse<A>,
         "Can't iterate with incomplete type, use pointer instead");
      static_assert(CT::Constant<Deptr<A>> or CT::Mutable<THIS>,
         "Non-constant iterator for constant memory is not allowed");

      LANGULUS_ASSUME(DevAssumes, IsTyped<THIS>(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Block is empty", " (of type `", mType, "`)");
      LANGULUS_ASSUME(DevAssumes, IsSparse<THIS>() == CT::Sparse<A>,
         "Sparseness mismatch", " (`", mType, 
         "` compared against `", MetaDataOf<A>(), "`)"
      );

      if constexpr (CT::Dense<A>) {
         LANGULUS_ASSUME(DevAssumes, (CastsTo<A, true>()),
            "Incompatible iterator type", " `", MetaDataOf<A>(),
            "` (iterating block of type `", mType, "`)"
         );
      }

      // These are used as detectors for block change while iterating   
      // Should be optimized-out when !MUTABLE                          
      using DA = Deref<A>;
      UNUSED() DA* initialData;
      UNUSED() Count initialCount;
      if constexpr (CT::Mutable<THIS>) {
         initialData = const_cast<Block*>(this)->GetRawAs<DA>();
         initialCount = count;
      }

      // Prepare for the loop                                           
      constexpr bool HasBreaker = CT::Bool<R>;
      auto data = const_cast<Block*>(this)->GetRawAs<DA>();
      if constexpr (REVERSE)
         data += count - 1;

      auto dataEnd = REVERSE ? GetRawAs<DA>() - 1 : GetRawAs<DA>() + count;
      while (data != dataEnd) {
         // Execute function                                            
         if constexpr (HasBreaker) {
            if (not f(*data)) {
               // Early return, if function returns a false bool        
               return;
            }
         }
         else f(*data);

         if constexpr (CT::Mutable<THIS>) {
            // The block might change while iterating - make sure we    
            // consider this. It is always assumed, that the change     
            // happened in the last call at '*data'                     
            if (GetRawAs<DA>() != initialData) {
               // Memory moved, so we have to recalculate iterators     
               // based on the new memory (can happen independently)    
               data = const_cast<Block*>(this)->GetRawAs<DA>() + (data - initialData);
               if constexpr (REVERSE)
                  dataEnd = GetRawAs<DA>() - 1;
               else
                  dataEnd = GetRawAs<DA>() + count;

               initialData = const_cast<Block*>(this)->GetRawAs<DA>();
            }

            if (count > initialCount) {
               // Something was inserted at that position, so make sure 
               // we skip the addition and extend the 'dataEnd'         
               const auto addition = count - initialCount;
               if constexpr (REVERSE)
                  data -= addition;
               else {
                  data += addition;
                  dataEnd += addition;
               }

               initialCount = count;
            }
            else if (count < initialCount) {
               // Something was removed at current position, so make    
               // sure we don't advance the iterator - it's already on  
               // the next relevant element. There is no danger for     
               // memory moving in this case                            
               const auto removed = initialCount - count;
               if constexpr (not REVERSE)
                  dataEnd -= removed;

               initialCount = count;

               // Skip incrementing/decrementing                        
               continue;
            }
         }

         // Next element                                                
         if constexpr (REVERSE)
            --data;
         else
            ++data;
      }
   }
   
   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   template<CT::Block THIS> LANGULUS(INLINED)
   constexpr Block::Iterator<THIS> Block::begin() noexcept {
      if (IsEmpty())
         return end();

      if constexpr (CT::Typed<THIS>)
         return {GetRaw<THIS>(), GetRawEndAs<Byte, THIS>()};
      else
         return {GetElement(), GetRawEndAs<Byte, THIS>()};
   }

   template<CT::Block THIS> LANGULUS(INLINED)
   constexpr Block::Iterator<const THIS> Block::begin() const noexcept {
      return const_cast<Block*>(this)->begin<THIS>();
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   template<CT::Block THIS> LANGULUS(INLINED)
   constexpr Block::Iterator<THIS> Block::last() noexcept {
      if (IsEmpty())
         return end();

      if constexpr (CT::Typed<THIS>) {
         const auto ptr = GetRaw<THIS>();
         return {ptr + (mCount - 1), reinterpret_cast<const Byte*>(ptr + mCount)};
      }
      else return {GetElement(), GetRawEndAs<Byte, THIS>()};
   }

   template<CT::Block THIS> LANGULUS(INLINED)
   constexpr Block::Iterator<const THIS> Block::last() const noexcept {
      return const_cast<Block*>(this)->last<THIS>();
   }




   /// Construct an iterator                                                  
   ///   @param start - the current iterator position                         
   ///   @param end - the ending marker                                       
   template<class T> LANGULUS(INLINED)
   constexpr Block::Iterator<T>::Iterator(TypeInner start, Byte const* end) noexcept
      : mValue {start}
      , mEnd {end} {}

   /// Construct an end iterator                                              
   template<class T> LANGULUS(INLINED)
   constexpr Block::Iterator<T>::Iterator(const A::IteratorEnd&) noexcept
      : mValue {nullptr}
      , mEnd {nullptr} {}

   /// Compare two iterators                                                  
   ///   @param rhs - the other iterator                                      
   ///   @return true if iterators point to the same element                  
   template<class T> LANGULUS(INLINED)
   constexpr bool Block::Iterator<T>::operator == (const Iterator& rhs) const noexcept {
      if constexpr (CT::Typed<T>)
         return mValue == reinterpret_cast<const Type*>(rhs.mValue);
      else
         return mValue.mRaw == rhs.mValue.mRaw;
   }

   /// Compare iterator with an end marker                                    
   ///   @param rhs - the end iterator                                        
   ///   @return true element is at or beyond the end marker                  
   template<class T> LANGULUS(INLINED)
   constexpr bool Block::Iterator<T>::operator == (const A::IteratorEnd&) const noexcept {
      if constexpr (CT::Typed<T>)
         return mValue >= reinterpret_cast<const Type*>(mEnd);
      else
         return mValue.mRaw >= mEnd;
   }
   
   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   template<class T> LANGULUS(INLINED)
   constexpr decltype(auto) Block::Iterator<T>::operator * () const noexcept {
      if constexpr (CT::Typed<T>)
         return *mValue;
      else
         return (mValue);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   template<class T> LANGULUS(INLINED)
   constexpr decltype(auto) Block::Iterator<T>::operator -> () const noexcept {
      if constexpr (CT::Typed<T>)
         return mValue;
      else
         return &mValue;
   }

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<class T> LANGULUS(INLINED)
   constexpr Block::Iterator<T>& Block::Iterator<T>::operator ++ () noexcept {
      if constexpr (CT::Typed<T>)
         ++mValue;
      else
         mValue.Next();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<class T> LANGULUS(INLINED)
   constexpr Block::Iterator<T> Block::Iterator<T>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Check if iterator is valid                                             
   template<class T> LANGULUS(INLINED)
   constexpr Block::Iterator<T>::operator bool() const noexcept {
      return *this != A::IteratorEnd {};
   }

   /// Implicitly convert to a constant iterator                              
   template<class T> LANGULUS(INLINED)
   constexpr Block::Iterator<T>::operator Iterator<const T>() const noexcept requires Mutable {
      return {mValue, mEnd};
   }

} // namespace Langulus::Anyness