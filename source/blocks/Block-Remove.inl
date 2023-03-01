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
   
   /// Remove the first occurence of a given item                             
   ///   @tparam REVERSE - whether to search from the back                    
   ///   @tparam T - the type to search for (deducible)                       
   ///   @param item - the item type to search for and remove                 
   ///   @return 1 if the element was found and removed, 0 otherwise          
   template<bool REVERSE, CT::Data T>
   LANGULUS(ALWAYSINLINE)
   Count Block::Remove(const T& item) {
      const auto found = FindKnown<REVERSE>(item);
      if (found)
         return RemoveIndex(found.GetOffset(), 1);
      return 0;
   }
   
   /// Remove sequential indices                                              
   ///   @tparam INDEX - the type of indexing to use                          
   ///   @param index - index to start removing from                          
   ///   @param count - number of items to remove                             
   ///   @return the number of removed elements                               
   template<CT::Index INDEX>
   Count Block::RemoveIndex(const INDEX index, const Count count) {
      if constexpr (CT::Same<INDEX, Index>) {
         // By special indices                                          
         if (index == IndexAll) {
            const auto oldCount = mCount;
            Free();
            ResetMemory();
            ResetState();
            return oldCount;
         }

         const auto idx = Constrain(index);
         if (idx.IsSpecial())
            return 0;

         return RemoveIndex(idx.GetOffset(), count);
      }
      else {
         Offset idx;
         if constexpr (CT::Signed<INDEX>) {
            if (index < 0)
               idx = mCount - static_cast<Offset>(-index);
            else
               idx = static_cast<Offset>(index);
         }
         else idx = index;

         // By simple index (signed or not)                             
         LANGULUS_ASSUME(DevAssumes, idx + count <= mCount, "Out of range");

         if (IsConstant() || IsStatic()) {
            if (mType->mIsPOD && idx + count >= mCount) {
               // If data is POD and elements are on the back, we can   
               // get around constantness and staticness, by simply     
               // truncating the count without any reprecussions        
               const auto removed = mCount - idx;
               mCount = idx;
               return removed;
            }
            else {
               LANGULUS_ASSERT(!IsConstant(), Access,
                  "Removing from constant container");
               LANGULUS_ASSERT(!IsStatic(), Access,
                  "Removing from static container");
               return 0;
            }
         }

         // First call the destructors on the correct region            
         const auto ender = idx + count;
         const auto removed = ender - idx;
         CropInner(idx, removed).CallUnknownDestructors();

         if (ender < mCount) {
            // Fill gap by invoking abandon-constructors                
            // We're moving to the left, so no reverse is required      
            LANGULUS_ASSERT(GetUses() == 1, Move, "Moving elements in use");
            const auto tail = mCount - ender;
            CropInner(idx, 0)
               .CallUnknownSemanticConstructors(
                  tail, Abandon(CropInner(ender, tail))
               );
         }

         // Change count                                                
         mCount -= removed;
         return removed;
      }
   }

   /// Remove a deep index corresponding to a whole sub-block                 
   ///   @tparam INDEX - the type of indexing to use                          
   ///   @param index - index to remove                                       
   ///   @return 1 if block at that index was removed, 0 otherwise            
   template<CT::Index INDEX>
   Count Block::RemoveIndexDeep(INDEX index) {
      if constexpr (!CT::Same<INDEX, Index>) {
         if (!IsDeep())
            return 0;

         --index;

         for (Count i = 0; i != mCount; i += 1) {
            if (index == 0)
               return RemoveIndex(i);

            auto ith = As<Block*>(i);
            const auto count = ith->GetCountDeep();
            if (index <= count && ith->RemoveIndexDeep(index))
               return 1;

            index -= count;
         }

         return 0;
      }
      else TODO();
   }

   /// Remove elements on the back                                            
   ///   @param count - the new count                                         
   inline void Block::Trim(const Count count) {
      if (count >= mCount)
         return;

      if (IsConstant() || IsStatic()) {
         if (mType->mIsPOD) {
            // If data is POD and elements are on the back, we can      
            // get around constantness and staticness, by simply        
            // truncating the count without any reprecussions           
            mCount = count;
         }
         else {
            LANGULUS_ASSERT(!IsConstant(), Access,
               "Removing from constant container");
            LANGULUS_ASSERT(!IsStatic(), Access,
               "Removing from static container");
         }

         return;
      }

      // Call destructors and change count                              
      CropInner(count, mCount - count).CallUnknownDestructors();
      mCount = count;
   }

   /// Flattens unnecessarily deep containers and combines their states       
   /// when possible                                                          
   /// Discards ORness if container has only one element                      
   inline void Block::Optimize() {
      if (IsOr() && GetCount() == 1)
         MakeAnd();

      while (GetCount() == 1 && IsDeep()) {
         auto& subPack = As<Block>();
         if (!CanFitState(subPack)) {
            subPack.Optimize();
            if (subPack.IsEmpty())
               Reset();
            return;
         }

         Block temporary {subPack};
         subPack.ResetMemory();
         Free();
         *this = temporary;
      }

      if (GetCount() > 1 && IsDeep()) {
         for (Count i = 0; i < mCount; ++i) {
            auto& subBlock = As<Block>(i);
            subBlock.Optimize();
            if (subBlock.IsEmpty()) {
               RemoveIndex(i);
               --i;
            }
         }
      }
   }

   /// Destroy all elements, but don't deallocate memory if possible          
   LANGULUS(ALWAYSINLINE)
   void Block::Clear() {
      if (!mEntry) {
         // Data is either static or unallocated                        
         // Don't call destructors, just clear it up                    
         mRaw = nullptr;
         mCount = mReserved = 0;
         return;
      }

      if (mEntry->GetUses() == 1) {
         // Entry is used only in this block, so it's safe to           
         // destroy all elements. We can reuse the entry                
         CallUnknownDestructors();
         mCount = 0;
      }
      else {
         // If reached, then data is referenced from multiple places    
         // Don't call destructors, just clear it up and dereference    
         mEntry->Free();
         mRaw = nullptr;
         mEntry = nullptr;
         mCount = mReserved = 0;
      }
   }

   /// Destroy all elements, deallocate block and reset state                 
   LANGULUS(ALWAYSINLINE)
   void Block::Reset() {
      Free();
      ResetMemory();
      ResetState();
   }
   
   /// Reset the block's state                                                
   /// Type constraints shall remain, if any                                  
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::ResetState() noexcept {
      mState = mState.mState & DataState::Typed;
      ResetType();
   }
   
   /// Call destructors of all initialized items                              
   ///   @attention never modifies any block state                            
   ///   @attention assumes there's at least one valid element                
   inline void Block::CallUnknownDestructors() const {
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Container is empty");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Container has no type");

      const bool destroy = !mType->mIsPOD && mType->mDestructor;
      if (mType->mIsSparse && !mType->mDeptr->mIsSparse) {
         // We dereference each pointer - destructors will be called    
         // if data behind these pointers is fully dereferenced, too    
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            const auto mthis = const_cast<Block*>(this);
            auto data = mthis->GetRawSparse();
            auto dataEntry = mthis->GetEntries();
            const auto dataEnd = data + mCount;
            if (destroy) {
               while (data != dataEnd) {
                  auto entry = *dataEntry;
                  if (entry) {
                     if (entry->GetUses() == 1) {
                        mType->mDeptr->mDestructor(*data);
                        Inner::Allocator::Deallocate(entry);
                     }
                     else entry->Free();
                  }

                  ++data;
                  ++dataEntry;
               }
            }
            else {
               while (data != dataEnd) {
                  auto entry = *dataEntry;
                  if (entry) {
                     if (entry->GetUses() == 1)
                        Inner::Allocator::Deallocate(entry);
                     else
                        entry->Free();
                  }

                  ++data;
                  ++dataEntry;
               }
            }
         #endif
      }
      else if (mType->mIsSparse) {
         // Destroy each indirection layer                              
         TODO();
      }
      else if (destroy) {
         // Destroy every dense element, one by one, using the          
         // reflected destructors (if any)                              
         auto data = mRaw;
         const auto dataStride = mType->mSize;
         const auto dataEnd = data + mCount * dataStride;
         while (data != dataEnd) {
            mType->mDestructor(data);
            data += dataStride;
         }
      }

      // Always nullify upon destruction only if we're paranoid         
      PARANOIA(ZeroMemory(mRaw, GetByteSize()));
   }

   /// Call destructors of all initialized items                              
   ///   @attention never modifies any block state                            
   ///   @attention assumes block is of type T, or is at least virtual base   
   ///   @tparam T - the type to destroy                                      
   template<CT::Data T>
   void Block::CallKnownDestructors() const {
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Container is empty");
      LANGULUS_ASSUME(DevAssumes, 
         IsExact<T>() || mType->template HasDerivation<T>(),
         "T isn't related to contained type");

      using DT = Decay<T>;
      const auto mthis = const_cast<Block*>(this);
      constexpr bool destroy = !CT::POD<T> && CT::Destroyable<T>;
      if constexpr (CT::Sparse<T> && CT::Dense<Deptr<T>>) {
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            // We dereference each pointer - destructors will be called 
            // if data behind these pointers is fully dereferenced, too 
            auto data = mthis->GetRawSparse();
            auto dataEntry = mthis->GetEntries();
            const auto dataEnd = data + mCount;
            while (data != dataEnd) {
               auto entry = *dataEntry;
               if (entry) {
                  if (entry->GetUses() == 1) {
                     if (destroy)
                        reinterpret_cast<T>(*data)->~DT();
                     Inner::Allocator::Deallocate(entry);
                  }
                  else entry->Free();
               }

               ++data;
               ++dataEntry;
            }
         #endif
      }
      else if constexpr (CT::Sparse<T>) {
         // Destroy each indirection layer                              
         TODO();
      }
      else if constexpr (destroy) {
         // Destroy every dense element                                 
         auto data = mthis->template GetRawAs<T>();
         const auto dataEnd = data + mCount;
         while (data != dataEnd)
            (data++)->~DT();
      }

      // Always nullify upon destruction only if we're paranoid         
      PARANOIA(ZeroMemory(mRaw, GetByteSize()));
   }

   /// Clear the block, only zeroing its size                                 
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::ClearInner() noexcept {
      mCount = 0;
   }

   /// Reset the memory inside the block                                      
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::ResetMemory() noexcept {
      mRaw = nullptr;
      mEntry = nullptr;
      mCount = mReserved = 0;
   }

} // namespace Langulus::Anyness