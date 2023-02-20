///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block.hpp"

namespace Langulus::Anyness
{
   
   /// Iterate each element block and execute F for it                        
   ///   @tparam MUTABLE - are we executing in a mutable, or immutable blocks 
   ///   @tparam F - the function signature (deducible)                       
   ///   @param call - function to execute for each element block             
   ///   @return the number of executions                                     
   template<bool MUTABLE, class F>
   Count Block::ForEachElement(F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block binary-compatible type");
      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      Count index {};
      while (index < mCount) {
         A block = GetElement(index);
         if constexpr (CT::Bool<R>) {
            // If F returns bool, you can decide when to break the loop 
            // by simply returning false                                
            if (!call(block))
               return index + 1;
         }
         else call(block);

         ++index;
      }

      return index;
   }

   /// Iterate each immutable element block and execute F for it              
   ///   @tparam F - the function signature                                   
   ///   @param call - function to execute for each constant element block    
   ///   @return the number of executions                                     
   template<class F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachElement(F&& call) const {
      return const_cast<Block&>(*this)
         .template ForEachElement<false>(call);
   }

   /// Execute functions for each element inside container                    
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEach(F&&... calls) {
      return (... || ForEachSplitter<MUTABLE, false>(Forward<F>(calls)));
   }

   /// Execute functions for each element inside container (const)            
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEach(F&&... calls) const {
      return const_cast<Block&>(*this)
         .template ForEach<false>(Forward<F>(calls)...);
   }

   /// Execute functions for each element inside container (reverse)          
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachRev(F&&... calls) {
      return (... || ForEachSplitter<MUTABLE, true>(Forward<F>(calls)));
   }

   /// Execute functions for each element inside container (reverse, const)   
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachRev(F&&... calls) const {
      return const_cast<Block&>(*this)
         .template ForEachRev<false>(Forward<F>(calls)...);
   }

   /// Execute functions in each sub-block                                    
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool SKIP, bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachDeep(F&&... calls) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, false>(Forward<F>(calls)));
   }

   /// Execute functions in each sub-block (const)                            
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool SKIP, class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachDeep(F&&... calls) const {
      return const_cast<Block&>(*this)
         .template ForEachDeep<SKIP, false>(Forward<F>(calls)...);
   }

   /// Execute functions in each sub-block (reverse)                          
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool SKIP, bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachDeepRev(F&&... calls) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, true>(Forward<F>(calls)));
   }

   /// Execute functions in each sub-block (reverse, const)                   
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool SKIP, class... F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachDeepRev(F&&... calls) const {
      return const_cast<Block&>(*this)
         .template ForEachDeepRev<SKIP, false>(Forward<F>(calls)...);
   }

   /// Execute single function from a sequence of functions for each element  
   /// inside container                                                       
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam F - the function types (deducible)                           
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool MUTABLE, bool REVERSE, class F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachSplitter(F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      return ForEachInner<R, A, REVERSE, MUTABLE>(Forward<F>(call));
   }

   /// Execute single function from a sequence of functions for each element  
   /// inside each sub-block in container                                     
   ///   @tparam SKIP - set to false, to execute F for deep elements, too     
   ///                  set to true, to execute only for non-deep elements    
   ///	@tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam F - the function type (deducible)                            
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachDeepSplitter(F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      if constexpr (CT::Deep<Decay<A>>) {
         // If argument type is deep                                    
         return ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(Forward<F>(call));
      }
      else {
         // Any other type is wrapped inside another ForEachDeep call   
         Count it = 0;

         if constexpr (CT::Constant<A>) {
            ForEachDeep<SKIP, MUTABLE>([&call, &it](const Block& block) {
               it += block.ForEach(Forward<F>(call));
            });
         }
         else {
            ForEachDeep<SKIP, MUTABLE>([&call, &it](Block& block) {
               it += block.ForEach(Forward<F>(call));
            });
         }

         return it;
      }
   }

   /// Iterate and execute call for each flat element                         
   ///   @tparam R - the function return type (deduced)                       
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type (deduced)                     
   ///               if A is incompatible with container, function wont ever  
   ///               be called, and ForEachInner will return 0                
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///	@tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   Count Block::ForEachInner(TFunctor<R(A)>&& call) {
      if (IsEmpty() || !mType->template CastsTo<A, true>())
         return 0;
       
      UNUSED() auto initialCount = mCount;
      constexpr bool HasBreaker = CT::Bool<R>;
      Count index {};

      while (index < mCount) {
         if constexpr (REVERSE) {
            if constexpr (HasBreaker) {
               if (!call(Get<A>(mCount - index - 1)))
                  return ++index;
            }
            else call(Get<A>(mCount - index - 1));
         }
         else {
            if constexpr (HasBreaker) {
               if (!call(Get<A>(index)))
                  return ++index;
            }
            else call(Get<A>(index));
         }

         if constexpr (MUTABLE) {
            // This block might change while iterating - make sure      
            // index compensates for changes                            
            if (mCount < initialCount) {
               initialCount = mCount;
               continue;
            }
         }
         
         ++index;
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
   ///                     while iterating                                    
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
   Count Block::ForEachDeepInner(TFunctor<R(A)>&& call) {
      using B = Decay<A>;
      static_assert(CT::Block<B>, "A must be a Block type");
      constexpr bool HasBreaker = CT::Bool<R>;
      UNUSED() bool atLeastOneChange = false;
      auto count {GetCountDeep()};
      Count index = 0;
      Count skipped = 0;
      while (index < count) {
         auto block = ReinterpretCast<B>(GetBlockDeep(index));
         if constexpr (MUTABLE) {
            if (!block)
               break;
         }

         if constexpr (SKIP) {
            // Skip deep/empty sub blocks                               
            if (block->IsDeep() || block->IsEmpty()) {
               ++index;
               ++skipped;
               continue;
            }
         }

         UNUSED() const auto initialBlockCount = block->GetCount();
         if constexpr (HasBreaker) {
            if (!call(*block))
               return ++index;
         }
         else if constexpr (CT::Sparse<A>)
            call(block);
         else
            call(*block);

         if constexpr (MUTABLE) {
            // Iterator might be invalid at this point!                 
            if (block->GetCount() != initialBlockCount) {
               // Something changes, so do a recalculation              
               if (block->GetCount() < initialBlockCount) {
                  // Something was removed, so propagate removal upwards
                  // until all empty stateless blocks are removed       
                  while (block && block->IsEmpty() && !block->GetUnconstrainedState()) {
                     index -= RemoveIndexDeep(index);
                     block = ReinterpretCast<B>(GetBlockDeep(index - 1));
                  }
               }

               count = GetCountDeep();
               atLeastOneChange = true;
            }
         }

         ++index;
      }

      if constexpr (MUTABLE) {
         if (atLeastOneChange)
            Optimize();
      }

      if constexpr (SKIP)
         return index - skipped;
      else
         return index;
   }
   
   /// Low-level element iteration function (for internal use only)           
   ///   @tparam AS - type of the element                                     
   ///   @tparam REVERSE - direction we're iterating in                       
   ///   @param call - the constexpr noexcept function to call on each item   
   template<class AS, bool REVERSE>
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::Iterate(TFunctor<void(const AS&)>&& call) const noexcept {
      if (!IsExact<AS>())
         return;

      if constexpr (REVERSE) {
         auto data = GetRawAs<AS>() + mCount - 1;
         const auto dataEnd = data - mCount;
         while (data != dataEnd)
            call(*(data--));
      }
      else {
         auto data = GetRawAs<AS>();
         const auto dataEnd = data + mCount;
         while (data != dataEnd)
            call(*(data++));
      }
   }

} // namespace Langulus::Anyness