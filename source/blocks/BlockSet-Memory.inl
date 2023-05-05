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

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   LANGULUS(INLINED)
   void BlockSet::Reserve(const Count& count) {
      AllocateInner(
         Roof2(count < MinimalAllocation ? MinimalAllocation : count)
      );
   }
   
   /// Allocate or reallocate key and info array                              
   ///   @attention assumes count is a power-of-two                           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   template<bool REUSE>
   void BlockSet::Allocate(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      auto oldInfo = mInfo;
      const auto oldCount = GetReserved();
      const auto oldInfoEnd = oldInfo + oldCount;

      // Allocate new keys                                              
      const Block oldKeys {mKeys};
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      if constexpr (REUSE)
         mKeys.mEntry = Fractalloc.Reallocate(keyAndInfoSize, mKeys.mEntry);
      else
         mKeys.mEntry = Fractalloc.Allocate(mKeys.mType, keyAndInfoSize);

      LANGULUS_ASSERT(mKeys.mEntry, Allocate,
         "Out of memory on allocating/reallocating keys");

      mKeys.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = mKeys.mEntry->GetBlockStart();
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
      // Set the sentinel                                               
      mInfo[count] = 1;

      // Zero or move the info array                                    
      if constexpr (REUSE) {
         // Check if keys were reused                                   
         if (mKeys.mEntry == oldKeys.mEntry) {
            // Keys were reused, but info always moves (null the rest)  
            MoveMemory(mInfo, oldInfo, oldCount);
            ZeroMemory(mInfo + oldCount, count - oldCount);

            // Data was reused, but entries always move if sparse keys  
            if (mKeys.IsSparse()) {
               MoveMemory(
                  mKeys.mRawSparse + count,
                  mKeys.mRawSparse + oldCount,
                  oldCount
               );
            };

            Rehash(oldCount);
            return;
         }
         else ZeroMemory(mInfo, count);
      }
      else ZeroMemory(mInfo, count);

      if (oldKeys.IsEmpty()) {
         // There are no old values, the previous set was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys moved - reinsert all pairs to rehash     
      mKeys.mCount = 0;
      auto key = oldKeys.GetElement();
      const auto hashmask = count - 1;
      while (oldInfo != oldInfoEnd) {
         if (!*(oldInfo++)) {
            key.Next();
            continue;
         }

         InsertInnerUnknown<false>(
            GetBucketUnknown(hashmask, key), 
            Abandon(key)
         );

         if (!key.IsEmpty())
            key.CallUnknownDestructors();
         else
            key.mCount = 1;

         key.Next();
      }

      // Free the old allocations                                       
      if constexpr (REUSE) {
         // When reusing, keys and values can potentially remain same   
         // Avoid deallocating them if that's the case                  
         if (oldKeys.mEntry != mKeys.mEntry)
            Fractalloc.Deallocate(oldKeys.mEntry);
      }
      else if (oldKeys.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         if (oldKeys.mEntry->GetUses() > 1)
            oldKeys.mEntry->Free();
         else
            Fractalloc.Deallocate(oldKeys.mEntry);
      }
   }
   
   /// Allocate a fresh set of keys (for internal use only)                   
   ///   @attention doesn't initialize anything, but the memory state         
   ///   @attention doesn't modify count, doesn't set info sentinel           
   ///   @attention assumes count is a power-of-two                           
   ///   @param count - the new number of elements                            
   inline void BlockSet::AllocateFresh(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      mKeys.mEntry = Fractalloc.Allocate(mKeys.mType, keyAndInfoSize);
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      mKeys.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = mKeys.mEntry->GetBlockStart();
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - number of pairs to allocate                           
   LANGULUS(INLINED)
   void BlockSet::AllocateInner(const Count& count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() && GetUses() == 1)
         Allocate<true>(count);
      else
         Allocate<false>(count);
   }

   /// Reference memory block if we own it                                    
   ///   @param times - number of references to add                           
   LANGULUS(INLINED)
   void BlockSet::Reference(const Count& times) const noexcept {
      mKeys.Reference(times);
   }
   
   /// Reference memory block once                                            
   LANGULUS(INLINED)
   void BlockSet::Keep() const noexcept {
      Reference(1);
   }
         
   /// Dereference memory block                                               
   ///   @attention this never modifies any state, except mKeys.mEntry        
   ///   @tparam DESTROY - whether to call destructors on full dereference    
   ///   @param times - number of references to subtract                      
   template<bool DESTROY>
   void BlockSet::Dereference(const Count& times) {
      if (!mKeys.mEntry)
         return;

      LANGULUS_ASSUME(DevAssumes,
         mKeys.mEntry->GetUses() >= times, "Bad memory dereferencing");

      if (mKeys.mEntry->GetUses() == 1) {
         if constexpr (DESTROY) {
            if (!IsEmpty()) {
               // Destroy all valid entries                             
               ClearInner();
            }
         }

         // Deallocate stuff                                            
         Fractalloc.Deallocate(mKeys.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref            
         mKeys.mEntry->Free();
      }
   }

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @attention this never modifies any state, except mKeys.mEntry        
   LANGULUS(INLINED)
   void BlockSet::Free() {
      return Dereference<true>(1);
   }

} // namespace Langulus::Anyness
