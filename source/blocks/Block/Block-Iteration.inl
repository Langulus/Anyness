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
   ///   @tparam MUTABLE - are we executing in a mutable, or immutable blocks 
   ///   @tparam F - the function signature (deducible)                       
   ///   @param call - function to execute for each element block             
   ///   @return the number of executions                                     
   template<bool REVERSE, bool MUTABLE, class F>
   Count Block::ForEachElement(F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block binary-compatible type");
      static_assert(CT::Constant<A> or MUTABLE,
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

   /// Iterate each immutable element block and execute F for it              
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam F - the function signature                                   
   ///   @param call - function to execute for each constant element block    
   ///   @return the number of executions                                     
   template<bool REVERSE, class F>
   LANGULUS(INLINED)
   Count Block::ForEachElement(F&& call) const {
      return const_cast<Block&>(*this).template
         ForEachElement<REVERSE, false>(call);
   }

   /// Execute functions for each element inside container                    
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count Block::ForEach(F&&... calls) {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (
         0 != (result = ForEachInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, MUTABLE>(
            Forward<F>(calls))
         )
      ));
      return result;
   }

   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count Block::ForEach(F&&... calls) const {
      return const_cast<Block&>(*this).template
         ForEach<REVERSE, false>(Forward<F>(calls)...);
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
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE, bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count Block::ForEachDeep(F&&... calls) {
      Count result = 0;
      ((result += ForEachDeepInner<ReturnOf<F>, ArgumentOf<F>, REVERSE, SKIP, MUTABLE>(
         Forward<F>(calls))
      ), ...);
      return result;
   }

   template<bool REVERSE, bool SKIP, class... F>
   LANGULUS(INLINED)
   Count Block::ForEachDeep(F&&... calls) const {
      return const_cast<Block&>(*this).template
         ForEachDeep<REVERSE, SKIP, false>(Forward<F>(calls)...);
   }

   template<bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count Block::ForEachElementRev(F&&... f) {
      return ForEachElement<true, MUTABLE, F...>(Forward<F>(f)...);
   }

   template<class... F>
   LANGULUS(INLINED)
   Count Block::ForEachElementRev(F&&... f) const {
      return ForEachElement<true, F...>(Forward<F>(f)...);
   }

   template<bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count Block::ForEachRev(F&&... f) {
      return ForEach<true, MUTABLE, F...>(Forward<F>(f)...);
   }

   template<class... F>
   LANGULUS(INLINED)
   Count Block::ForEachRev(F&&... f) const {
      return ForEach<true, F...>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count Block::ForEachDeepRev(F&&... f) {
      return ForEachDeep<true, SKIP, MUTABLE, F...>(Forward<F>(f)...);
   }

   template<bool SKIP, class... F>
   LANGULUS(INLINED)
   Count Block::ForEachDeepRev(F&&... f) const {
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
   ///	@tparam F - function signature for f() (deducible)                   
   ///   @param f - the function to execute for each element of type A        
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE, class F>
   LANGULUS(INLINED)
   Count Block::ForEachInner(F&& f) noexcept(NoexceptIterator<decltype(f)>) {
      constexpr auto NOE = NoexceptIterator<decltype(f)>;
      if (CT::Block<Decay<A>> != IsDeep() or not CastsTo<A>())
         return 0;

      Count index = 0;
      if (mType->mIsSparse) {
         // Iterate using pointers of A                                 
         using DA = Conditional<MUTABLE, Decay<A>*, const Decay<A>*>;
         IterateInner<R, DA, REVERSE, MUTABLE>(
            [&index, &f](DA element) noexcept(NOE) -> R {
               ++index;
               if constexpr (CT::Sparse<A>)  return f(element);
               else                          return f(*element);
            }
         );
      }
      else {
         // Iterate using references of A                               
         using DA = Conditional<MUTABLE, Decay<A>&, const Decay<A>&>;
         IterateInner<R, DA, REVERSE, MUTABLE>(
            [&index, &f](DA element) noexcept(NOE) -> R {
               ++index;
               if constexpr (CT::Sparse<A>)  return f(&element);
               else                          return f(element);
            }
         );
      }

      return index;
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
   template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE, class F>
   Count Block::ForEachDeepInner(F&& call) {
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
                        Forward<F>(call));
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
                        Forward<F>(call));
               }
            );
         }
         else {
            // Equivalent to non-deep iteration                         
            counter += ForEachInner<R, A, REVERSE, MUTABLE>(Forward<F>(call));
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
   ///   @tparam MUTABLE - whether or not block's allowed to change during    
   ///                     iteration (iteration is slower if true)            
   ///   @param call - the constexpr noexcept function to call on each item   
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE, class F>
   LANGULUS(INLINED)
   void Block::IterateInner(F&& f) noexcept(NoexceptIterator<decltype(f)>) {
      static_assert(CT::Complete<Decay<A>> or CT::Sparse<A>,
         "Can't iterate with incomplete type, use pointer instead");
      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant memory is not allowed");

      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Block is empty");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Container is not typed");
      LANGULUS_ASSUME(DevAssumes, IsSparse() == CT::Sparse<A>,
         "Sparseness mismatch");

      if constexpr (CT::Dense<A>) {
         LANGULUS_ASSUME(DevAssumes, (CastsTo<A, true>()),
            "Iteration type is binary incompatible");
      }

      // These are used as detectors for block change while iterating   
      // Should be optimized-out when !MUTABLE                          
      using DA = Deref<A>;
      UNUSED() DA* initialData;
      UNUSED() Count initialCount;
      if constexpr (MUTABLE) {
         initialData = GetRawAs<DA>();
         initialCount = mCount;
      }

      // Prepare for the loop                                           
      constexpr bool HasBreaker = CT::Bool<R>;
      auto data = GetRawAs<DA>();
      if constexpr (REVERSE)
         data += mCount - 1;

      auto dataEnd = REVERSE ? GetRawAs<DA>() - 1 : GetRawAs<DA>() + mCount;
      while (data != dataEnd) {
         // Execute function                                            
         if constexpr (HasBreaker) {
            if (not f(*data)) {
               // Early return, if function returns a false bool        
               return;
            }
         }
         else f(*data);

         if constexpr (MUTABLE) {
            // The block might change while iterating - make sure we    
            // consider this. It is always assumed, that the change     
            // happened in the last call at '*data'                     
            if (GetRawAs<DA>() != initialData) {
               // Memory moved, so we have to recalculate iterators     
               // based on the new memory (can happen independently)    
               data = GetRawAs<DA>() + (data - initialData);
               if constexpr (REVERSE)
                  dataEnd = GetRawAs<DA>() - 1;
               else
                  dataEnd = GetRawAs<DA>() + mCount;

               initialData = GetRawAs<DA>();
            }

            if (mCount > initialCount) {
               // Something was inserted at that position, so make sure 
               // we skip the addition and extend the 'dataEnd'         
               const auto addition = mCount - initialCount;
               if constexpr (REVERSE)
                  data -= addition;
               else {
                  data += addition;
                  dataEnd += addition;
               }

               initialCount = mCount;
            }
            else if (mCount < initialCount) {
               // Something was removed at current position, so make    
               // sure we don't advance the iterator - it's already on  
               // the next relevant element. There is no danger for     
               // memory moving in this case                            
               const auto removed = initialCount - mCount;
               if constexpr (!REVERSE)
                  dataEnd -= removed;

               initialCount = mCount;

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

} // namespace Langulus::Anyness