///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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
      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
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

      Count result {};
      (void) (... or (0 != (result = ForEachSplitter<MUTABLE, REVERSE>(Forward<F>(calls)))));
      return result;
   }

   /// Execute functions for each element inside container (const)            
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count Block::ForEach(F&&... calls) const {
      return const_cast<Block&>(*this).template
         ForEach<REVERSE, false>(Forward<F>(calls)...);
   }

   /// Execute functions in each sub-block                                    
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
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
      Count result {};
      (void) (... or (0 != (result = ForEachDeepSplitter<SKIP, MUTABLE, REVERSE>(Forward<F>(calls)))));
      return result;
   }

   /// Execute functions in each sub-block (const)                            
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @tparam F - the function signatures (deducible)                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
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

   /// Execute single function from a sequence of functions for each element  
   /// inside container, if F's argument is compatible with contained type    
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is typed                                    
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam F - the function signature (deducible)                       
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool MUTABLE, bool REVERSE, class F>
   LANGULUS(INLINED)
   Count Block::ForEachSplitter(F&& call) {
      LANGULUS_ASSUME(DevAssumes, !IsEmpty(),
         "Container is empty");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Container is not typed");

      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant memory block");

      if constexpr (CT::Complete<Decay<A>>) {
         // If the decayed argument is complete, we can try more things 
         if (mType->mIsDeep == CT::Deep<Decay<A>> and mType->template CastsTo<A, true>()) {
            // Container is binary compatible, and of same deepness     
            return ForEachInner<R, A, REVERSE, MUTABLE>(Forward<F>(call));
         }
         else if (mType->mIsSparse and mType->mResolver) {
            // Not binary compatible, but pointers are resolvable       
            Count counter {};
            Iterate<MUTABLE, REVERSE>([&](const void*& element) -> R {
               if constexpr (CT::Bool<R>) {
                  if (not element)
                     return Flow::Continue;
               }
               else if (not element)
                  return;

               auto resolved = mType->mResolver(element);
               if (resolved.template Is<A>()) {
                  ++counter;
                  return call(resolved.template Get<A>());
               }
               else {
                  if constexpr (CT::Bool<R>)
                     return Flow::Continue;
                  else
                     return;
               }
            });

            return counter;
         }
      }
      else if constexpr (CT::Sparse<A>) {
         // The argument is not complete, so stick to using pointer     
         return ForEachInner<R, A, REVERSE, MUTABLE>(Forward<F>(call));
      }
      else LANGULUS_ERROR("Can't iterate with incomplete type, unless it's a pointer");
      
      return 0;
   }

   /// Execute single function from a sequence of functions for each element  
   /// inside each sub-block in container                                     
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is typed                                    
   ///   @tparam SKIP - set to false, to execute F for deep elements, too     
   ///                  set to true, to execute only for non-deep elements    
   ///	@tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam F - the function signature (deducible)                       
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
   LANGULUS(INLINED)
   Count Block::ForEachDeepSplitter(F&& call) {
      // Notice, that no assumptions required here                      
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant memory block");

      if constexpr (CT::Deep<Decay<A>>) {
         // If argument type is deep                                    
         return ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(Forward<F>(call));
      }
      else {
         // Any other type is wrapped inside another ForEachDeep call   
         Count it = 0;
         using DA = Conditional<CT::Constant<A>, const Block&, Block&>;
         ForEachDeep<SKIP, MUTABLE>([&call, &it](DA block) {
            it += block.ForEach(Forward<F>(call));
         });
         return it;
      }
   }

   /// Iterate and execute call for each flat element, counting each          
   /// successfull execution                                                  
   ///   @attention assumes A is binary compatible to the contained type      
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is typed                                    
   ///   @tparam R - the function return type (deduced)                       
   ///               if R is boolean, loop will cease on returning false      
   ///   @tparam A - the function argument type (deduced)                     
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///	@tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating (iteration is slower if true)      
   ///   @param f - the function to execute for each element of type A        
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE, class F>
   LANGULUS(INLINED)
   Count Block::ForEachInner(F&& f) noexcept(NoexceptIterator<decltype(f)>) {
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Container is empty");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Container is not typed");

      constexpr auto NOE = NoexceptIterator<decltype(f)>;
      Count index = 0;

      if constexpr (CT::Complete<Decay<A>>) {
         // Argument type is complete, and we can allow to be more      
         // flexible with our iterations                                
         LANGULUS_ASSUME(DevAssumes, (CastsTo<A, true>()),
            "Incompatible type");

         if (mType->mIsSparse) {
            // Iterate pointers of A                                    
            using DA = Decay<A>*;
            IterateInner<R, DA, REVERSE, MUTABLE>(
               [&index, &f](DA element) noexcept(NOE) -> R {
                  ++index;
                  if constexpr (CT::Sparse<A>)  return f(element);
                  else                          return f(*element);
               }
            );
         }
         else {
            // Iterate references of A                                  
            using DA = Decay<A>&;
            IterateInner<R, DA, REVERSE, MUTABLE>(
               [&index, &f](DA element) noexcept(NOE) -> R {
                  ++index;
                  if constexpr (CT::Sparse<A>)  return f(&element);
                  else                          return f(element);
               }
            );
         }
      }
      else {
         // Argument type is incomplete, and we allow only iteration    
         // by pointers                                                 
         LANGULUS_ASSUME(DevAssumes, CT::Sparse<A> and mType->mIsSparse,
            "Iterating with dense incomplete type");

         using DA = Decay<A>*;
         IterateInner<R, DA, REVERSE, MUTABLE>(
            [&index, &f](DA element) noexcept(NOE) -> R {
               ++index;
               return f(element);
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
      using B = Decay<A>;
      static_assert(CT::Block<B>, "A must be a Block type");
      constexpr bool HasBreaker = CT::Bool<R>;
      UNUSED() bool atLeastOneChange = false;
      auto count = GetCountDeep();
      Count index = 0;
      Count skipped = 0;
      while (index < count) {
         auto block = ReinterpretCast<B>(GetBlockDeep(index));
         if constexpr (MUTABLE) {
            if (not block)
               break;
         }

         if constexpr (SKIP) {
            // Skip deep/empty sub blocks                               
            if (block->IsDeep() or block->IsEmpty()) {
               ++index;
               ++skipped;
               continue;
            }
         }

         UNUSED() const auto initialBlockCount = block->GetCount();
         if constexpr (HasBreaker) {
            if (not call(*block))
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
                  while (block and block->IsEmpty() and not block->GetUnconstrainedState()) {
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
   
   /// Execute a function for each element inside container                   
   /// Lowest-level element iteration function (for internal use only)        
   ///   @attention assumes F's argument is binary compatible with the        
   ///              contained type                                            
   ///   @attention assumes block is not empty                                
   ///   @attention assumes sparseness matches                                
   ///   @tparam MUTABLE - whether or not block's allowed to change during    
   ///                     iteration (iteration is slower if true)            
   ///   @tparam REVERSE - direction we're iterating in                       
   ///   @tparam F - the function signature (deducible)                       
   ///   @param call - to function to execute                                 
   template<bool MUTABLE, bool REVERSE, class F>
   LANGULUS(INLINED)
   void Block::Iterate(F&& call) noexcept(NoexceptIterator<F>) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant memory block");

      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Block is empty");
      LANGULUS_ASSUME(DevAssumes, IsSparse() == CT::Sparse<A>,
         "Sparseness mismatch");

      if constexpr (CT::Dense<A>) {
         LANGULUS_ASSUME(DevAssumes, (CastsTo<A, true>()),
            "Iteration type is binary incompatible");
      }

      IterateInner<R, A, REVERSE, MUTABLE>(Forward<F>(call));
   }
   
   /// Execute a function for each element inside container (const            
   /// Lowest-level element iteration function (for internal use only)        
   ///   @attention assumes F's argument is binary compatible with the        
   ///              contained type                                            
   ///   @attention assumes block is not empty                                
   ///   @attention assumes sparseness matches                                
   ///   @tparam REVERSE - direction we're iterating in                       
   ///   @tparam F - the function signature (deducible)                       
   ///   @param call - to function to execute                                 
   template<bool REVERSE, class F>
   LANGULUS(INLINED)
   void Block::Iterate(F&& call) const noexcept(NoexceptIterator<F>) {
      const_cast<Block*>(this)->template
         Iterate<false, REVERSE>(Forward<F>(call));
   }

   /// Execute a function for each element inside container                   
   /// Lowest-level element iteration function (for internal use only)        
   ///   @attention assumes AS is binary compatible with the contained type   
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
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Block is empty");
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