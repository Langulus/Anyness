///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet.hpp"

namespace Langulus::Anyness
{
   
   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   LANGULUS(INLINED)
   typename BlockSet::Iterator BlockSet::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (!*info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetValue(offset)
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
      while (info >= GetInfo() && !*--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetValue(offset)
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
      while (!*info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         GetValue(offset)
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
      while (info >= GetInfo() && !*--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetValue(offset)
      };
   }

   /// Execute functions for each element inside container                    
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function types (deducible)                           
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool MUTABLE, bool REVERSE, class F>
   Count BlockSet::ForEachSplitter(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      return ForEachInner<R, A, REVERSE, MUTABLE>(part, Forward<F>(call));
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
   Count BlockSet::ForEachDeepSplitter(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      if constexpr (CT::Deep<A>) {
         // If argument type is deep                                    
         return ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(
            part, Forward<F>(call));
      }
      else if constexpr (CT::Constant<A>) {
         // Any other type is wrapped inside another ForEachDeep call   
         return ForEachDeep<REVERSE, SKIP, MUTABLE>(part,
            [&call](const Block& block) {
               block.template ForEach<REVERSE>(Forward<F>(call));
            }
         );
      }
      else {
         // Any other type is wrapped inside another ForEachDeep call   
         return ForEachDeep<REVERSE, SKIP, MUTABLE>(part,
            [&call](Block& block) {
               block.template ForEach<REVERSE>(Forward<F>(call));
            }
         );
      }
   }

   /// Iterate and execute call for each element                              
   ///   @param call - the function to execute for each element of type T     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE, class F>
   Count BlockSet::ForEachInner(Block& part, F&& call) {
      if (IsEmpty() || !part.mType->template CastsTo<A, true>())
         return 0;
       
      constexpr bool HasBreaker = CT::Bool<R>;
      Count done {};
      Count index {};

      while (index < mKeys.mReserved) {
         if (!mInfo[index]) {
            ++index;
            continue;
         }

         if constexpr (REVERSE) {
            if constexpr (HasBreaker) {
               if (!call(part.template Get<A>(mKeys.mReserved - index - 1)))
                  return ++done;
            }
            else call(part.template Get<A>(mKeys.mReserved - index - 1));
         }
         else {
            if constexpr (HasBreaker) {
               if (!call(part.template Get<A>(index)))
                  return ++done;
            }
            else call(part.template Get<A>(index));
         }

         ++index;
         ++done;
      }

      return done;
   }
   
   /// Iterate and execute call for each element                              
   ///   @param call - the function to execute for each element of type T     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE, class F>
   Count BlockSet::ForEachDeepInner(Block& part, F&& call) {
      constexpr bool HasBreaker = CT::Bool<R>;
      auto count {part.GetCountDeep()};
      Count index {};
      while (index < count) {
         auto block = ReinterpretCast<A>(part.GetBlockDeep(index));//TODO custom checked getblockdeep here, write tests and you'll see
         if constexpr (SKIP) {
            // Skip deep/empty sub blocks                               
            if (block->IsDeep() || block->IsEmpty()) {
               ++index;
               continue;
            }
         }

         if constexpr (HasBreaker) {
            if (!call(*block))
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
   Count BlockSet::ForEachElement(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block type");
      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      Count index {};
      while (index < GetReserved()) {
         if (!mInfo[index]) {
            ++index;
            continue;
         }

         A block = part.GetElement(index);
         if constexpr (CT::Bool<R>) {
            if (!call(block))
               return ++index;
         }
         else call(block);

         ++index;
      }

      return index;
   }

   /// Iterate all elements inside the map, and perform f() on them           
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each element block               
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachElement(F&& f) {
      return ForEachElement<REVERSE, MUTABLE>(mKeys, Forward<F>(f));
   }

   template<bool REVERSE, class F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachElement(F&& f) const {
      return const_cast<BlockSet&>(*this).template
         ForEachElement<REVERSE, false>(Forward<F>(f));
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool REVERSE, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEach(F&&... f) {
      Count result {};
      (void) (... || (0 != (result = ForEachSplitter<MUTABLE, REVERSE>(mKeys, Forward<F>(f)))));
      return result;
   }

   template<bool REVERSE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEach(F&&... f) const {
      return const_cast<BlockSet&>(*this).template
         ForEach<REVERSE, false>(Forward<F>(f)...);
   }

   template<bool REVERSE, bool SKIP, bool MUTABLE, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachDeep(F&&... f) {
      Count result {};
      (void) (... || (0 != (result = ForEachDeepSplitter<SKIP, MUTABLE, REVERSE>(mKeys, Forward<F>(f)))));
      return result;
   }

   template<bool REVERSE, bool SKIP, class... F>
   LANGULUS(INLINED)
   Count BlockSet::ForEachDeep(F&&... f) const {
      return const_cast<BlockSet&>(*this).template
         ForEachDeep<REVERSE, SKIP, false>(Forward<F>(f)...);
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
      while (!*++mInfo);
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

} // namespace Langulus::Anyness
