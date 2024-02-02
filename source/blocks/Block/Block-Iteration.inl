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

      Count index = REVERSE ? mCount - 1 : 0;
      const auto next = [&index] {
         if constexpr (REVERSE)  --index;
         else                    ++index;
      };

      while (index < mCount) {
         A block = GetElement(index);

         if constexpr (CT::Bool<R>) {
            // If F returns bool, you can decide when to break the loop 
            // by simply returning false                                
            if (not call(block))
               return REVERSE ? mCount - index : index + 1;
            next();
         }
         else if constexpr (CT::Exact<R, LoopControl>) {
            // Do things depending on the F's return                    
            const auto loop = call(block);
            switch (loop) {
            case Loop::Break:
               return REVERSE ? mCount - index : index + 1;
            case Loop::Continue:
               next();
               break;
            case Loop::Discard:
               if constexpr (CT::Mutable<THIS>) {
                  // Discard is allowed only if THIS is mutable         
                  const_cast<Block*>(this)
                     ->template RemoveIndex<THIS>(index);
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
            call(block);
            next();
         }
      }

      return mCount;
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
      (void) (... or (Loop::NextLoop !=
         ForEachInner<THIS, REVERSE>(Forward<F>(calls), result)
      ));
      return result;
   }

   /// Execute functions in each sub-block, inclusively                       
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. None of the provided     
   /// functions are ignored, unless Loop::Break is called.                   
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE, bool SKIP, CT::Block THIS, class...F>
   LANGULUS(INLINED) Count Block::ForEachDeep(F&&...calls) const {
      Count result = 0;
      (void)(... or (Loop::Break !=
         ForEachDeepInner<THIS, REVERSE, SKIP>(Forward<F>(calls), result)
      ));
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
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param f - the function to execute for each element of type A        
   ///   @return the last 'f' result                                          
   template<CT::Block THIS, bool REVERSE> LANGULUS(INLINED)
   LoopControl Block::ForEachInner(auto&& f, Count& index) const
   noexcept(NoexceptIterator<decltype(f)>) {
      using F = Deref<decltype(f)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      constexpr auto NOE = NoexceptIterator<decltype(f)>;

      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;

         if constexpr (CT::Deep<Decay<A>, Decay<T>> or CT::Same<T, A>) {
            using IT = Conditional<CT::Mutable<THIS>, T&, const T&>;
            return IterateInner<THIS, REVERSE>(mCount,
               [&index, &f](IT element) noexcept(NOE) -> R {
                  ++index;

                  //TODO this does only one dereference if needed, but it should actually
                  // check the difference of sparseness between A and T, and dereference as
                  // many times as needed. that way we can iterate int*** for example,
                  // even if int***** is contained
                  // it can be done on compile time without any cost whatsoever
                  if constexpr (CT::Dense<A, T> or CT::Sparse<A, T>)
                     return f(element);
                  else if constexpr (CT::Dense<A>)
                     return f(*element);
                  else
                     return f(&element);
               }
            );
         }
         else return Loop::NextLoop;
      }
      else if (   (CT::Deep<Decay<A>> and IsDeep<THIS>())
           or (not CT::Deep<Decay<A>> and CastsTo<A>())
      ) {
         if (mType->mIsSparse) {
            // Iterate sparse container                                 
            using DA = Conditional<CT::Mutable<THIS>, void*&, const void* const&>;
            return IterateInner<THIS, REVERSE>(mCount,
               [&index, &f](DA element) noexcept(NOE) -> R {
                  ++index;
                  if constexpr (CT::Dense<A>) {
                     if constexpr (CT::Mutable<THIS>)
                        return f(*static_cast<Deref<A>*>(element));
                     else
                        return f(*static_cast<const Deref<A>*>(element));
                  }
                  else return f( static_cast<A>(element));
               }
            );
         }
         else {
            // Iterate dense container                                  
            using DA = Conditional<CT::Mutable<THIS>, Decay<A>&, const Decay<A>&>;
            return IterateInner<THIS, REVERSE>(mCount,
               [&index, &f](DA element) noexcept(NOE) -> R {
                  ++index;
                  if constexpr (CT::Dense<A>)
                     return f(element);
                  else
                     return f(&element);
               }
            );
         }
      }
      else return Loop::NextLoop;
   }
   
   /// Iterate and execute call for each deep element                         
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - whether to execute call for intermediate blocks       
   ///   @param call - the function to execute for each element of type A     
   ///   @return the number of executions that occured                        
   template<CT::Block THIS, bool REVERSE, bool SKIP>
   LoopControl Block::ForEachDeepInner(auto&& call, Count& counter) const {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      using SubBlock = Conditional<CT::Mutable<THIS>, Block&, const Block&>;
      static_assert(CT::Dense<A>, "Iterator must be dense value/reference");

      if constexpr (CT::Deep<A>) {
         if (not SKIP or not IsDeep<THIS>()) {
            // Always execute for intermediate/non-deep *this           
            ++counter;

            auto b = reinterpret_cast<Deref<A>*>(const_cast<Block*>(this));
            if constexpr (CT::Bool<R>) {
               if (not call(*b))
                  return Loop::Break;
            }
            else if constexpr (CT::Exact<R, LoopControl>) {
               // Do things depending on the F's return                 
               const auto loop = call(*b);
               switch (loop) {
               case Loop::Break:
                  return Loop::Break;
               case Loop::Continue:
                  break;
               case Loop::Discard:
                  if constexpr (CT::Mutable<THIS>) {
                     // Discard is allowed only if THIS is mutable      
                     // You can't fully discard the topmost block,      
                     // only reset it                                   
                     const_cast<Block*>(this)->template Reset<THIS>();
                     return Loop::Discard;
                  }
                  else {
                     // ...otherwise it acts like a Loop::Continue      
                     break;
                  }
               }
            }
            else call(*b);
         }
      }

      if (IsDeep<THIS>()) {
         // Iterate subblocks                                           
         Count intermediateCounterSink = 0;
         return ForEachInner<THIS, REVERSE>(
            [&counter, &call](SubBlock group) {
               return DenseCast(group).template
                  ForEachDeepInner<SubBlock, REVERSE, SKIP>(
                     ::std::move(call), counter
                  );
            },
            intermediateCounterSink
         );
      }
      else if constexpr (not CT::Deep<A>) {
         // Equivalent to non-deep iteration                            
         return ForEachInner<THIS, REVERSE>(::std::move(call), counter);
      }

      return Loop::Continue;
   }

   /// Execute a function for each element inside container                   
   /// Lowest-level element iteration function (for internal use only)        
   ///   @attention assumes A is binary compatible with the contained type    
   ///   @attention assumes block is not empty                                
   ///   @attention assumes sparseness matches                                
   ///   @tparam REVERSE - direction we're iterating in                       
   ///   @param call - the constexpr noexcept function to call on each item   
   template<CT::Block THIS, bool REVERSE> LANGULUS(INLINED)
   LoopControl Block::IterateInner(const Count count, auto&& f) const
   noexcept(NoexceptIterator<decltype(f)>) {
      using F = Deref<decltype(f)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

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
      const auto raw = const_cast<Block*>(this)->GetRawAs<DA, THIS>();

      // Prepare for the loop                                           
      auto data = raw;
      if constexpr (REVERSE)
         data += count - 1;
      const auto next = [&data] {
         if constexpr (REVERSE)  --data;
         else                    ++data;
      };

      auto dataEnd = REVERSE ? raw - 1 : raw + count;
      while (data != dataEnd) {
         // Execute function                                            
         if constexpr (CT::Bool<R>) {
            if (not f(*data))
               return Loop::Break;
            next();
         }
         else if constexpr (CT::Exact<R, LoopControl>) {
            // Do things depending on the F's return                    
            const auto loop = f(*data);
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
                  const_cast<Block*>(this)
                     ->template RemoveIndex<THIS>(raw - data);
                  if constexpr (REVERSE)
                     next();
                  else
                     --dataEnd;
               }
               else {
                  // ...otherwise it acts like a Loop::Continue         
                  next();
               }
               break;
            }
         }
         else {
            f(*data);
            next();
         }
      }

      return Loop::Continue;
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
   
   /// Prefix increment - get next element by incrementing data pointer       
   LANGULUS(INLINED)
   Block& Block::operator ++ () IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mRaw + mType->mSize <= mEntry->GetBlockEnd(),
         "Block limit breached");

      mRaw += mType->mSize;
      return *this;
   }

   /// Prefix decrement - get previous element by decrementing data pointer   
   LANGULUS(INLINED)
   Block& Block::operator -- () IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mRaw - mType->mSize >= mEntry->GetBlockStart(),
         "Block limit breached");

      mRaw -= mType->mSize;
      return *this;
   }

   /// Suffic increment - get next element by incrementing data pointer       
   LANGULUS(INLINED)
   Block Block::operator ++ (int) const IF_UNSAFE(noexcept) {
      Block copy {*this};
      return ++copy;
   }

   /// Suffic decrement - get previous element by decrementing data pointer   
   LANGULUS(INLINED)
   Block Block::operator -- (int) const IF_UNSAFE(noexcept) {
      Block copy {*this};
      return --copy;
   }
   
   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   LANGULUS(INLINED)
   Block Block::operator + (Offset offset) const IF_UNSAFE(noexcept) {
      auto copy {*this};
      return copy += offset;
   }

   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   LANGULUS(INLINED)
   Block Block::operator - (Offset offset) const IF_UNSAFE(noexcept) {
      auto copy {*this};
      return copy -= offset;
   }
   
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   LANGULUS(INLINED)
   Block& Block::operator += (Offset offset) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mRaw + offset * mType->mSize <= mEntry->GetBlockEnd(),
         "Block limit breached");

      mRaw += offset * mType->mSize;
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   LANGULUS(INLINED)
   Block& Block::operator -= (Offset offset) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mRaw - offset * mType->mSize >= mEntry->GetBlockStart(),
         "Block limit breached");

      mRaw -= offset * mType->mSize;
      return *this;
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
         return *mValue;
      else
         return &mValue;
   }

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<class T> LANGULUS(INLINED)
   constexpr Block::Iterator<T>& Block::Iterator<T>::operator ++ () noexcept {
      ++mValue;
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