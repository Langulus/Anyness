///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"

namespace Langulus::Anyness
{

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   LANGULUS(INLINED)
   void BlockMap::Reserve(const Count& count) {
      AllocateInner(
         Roof2(count < MinimalAllocation ? MinimalAllocation : count)
      );
   }
   
   /// Allocate a fresh set keys and values (for internal use only)           
   ///   @attention doesn't initialize anything, but the memory state         
   ///   @attention doesn't modify count, doesn't set info sentinel           
   ///   @attention assumes count is a power-of-two                           
   ///   @param count - the new number of pairs                               
   inline void BlockMap::AllocateFresh(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      const auto valueByteSize = RequestValuesSize(count);
      mValues.mEntry = Allocator::Allocate(mValues.mType, valueByteSize);

      if (not mValues.mEntry) {
         Allocator::Deallocate(mKeys.mEntry);
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate, "Out of memory");
      }

      mValues.mRaw = mValues.mEntry->GetBlockStart();
      mKeys.mReserved = mValues.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = mKeys.mEntry->GetBlockStart();
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
   }

   /// Allocate or reallocate key, value, and info array                      
   ///   @attention assumes count is a power-of-two                           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   template<bool REUSE>
   void BlockMap::AllocateData(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      auto oldInfo = mInfo;
      const auto oldCount = GetReserved();
      const auto oldInfoEnd = oldInfo + oldCount;

      // Allocate new keys                                              
      Block oldKeys {mKeys};
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      if constexpr (REUSE)
         mKeys.mEntry = Allocator::Reallocate(keyAndInfoSize, mKeys.mEntry);
      else
         mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);

      LANGULUS_ASSERT(mKeys.mEntry, Allocate,
         "Out of memory on allocating/reallocating keys");

      // Allocate new values                                            
      Block oldValues {mValues};
      const auto valueByteSize = RequestValuesSize(count);
      if constexpr (REUSE)
         mValues.mEntry = Allocator::Reallocate(valueByteSize, mValues.mEntry);
      else
         mValues.mEntry = Allocator::Allocate(mValues.mType, valueByteSize);

      if (not mValues.mEntry) {
         Allocator::Deallocate(mKeys.mEntry);
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate,
            "Out of memory on allocating/reallocating values");
      }

      mValues.mRaw = mValues.mEntry->GetBlockStart();
      mKeys.mReserved = mValues.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = mKeys.mEntry->GetBlockStart();
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
      // Set the sentinel                                               
      mInfo[count] = 1;

      // Zero or move the info array                                    
      if constexpr (REUSE) {
         // Check if keys were reused                                   
         if (mKeys.mEntry == oldKeys.mEntry) {
            // Data was reused, but info always moves (null the rest)   
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

            if (mValues.mEntry == oldValues.mEntry) {
               // Both keys and values remain in the same place         
               // Data was reused, but entries always move if sparse val
               if (mValues.IsSparse()) {
                  MoveMemory(
                     mValues.mRawSparse + count,
                     mValues.mRawSparse + oldCount,
                     oldCount
                  );
               };

               Rehash(oldCount);
            }
            else {
               // Only values moved, reinsert them, rehash the rest     
               RehashKeys(oldCount, oldValues);
               Allocator::Deallocate(oldValues.mEntry);
            }
            return;
         }
         else if (mValues.mEntry == oldValues.mEntry) {
            // Only keys moved, reinsert them, rehash the rest          
            RehashValues(oldCount, oldKeys);
            Allocator::Deallocate(oldKeys.mEntry);
            return;
         }
      }

      // If reached, then both keys and values are newly allocated      
      ZeroMemory(mInfo, count);
      if (oldValues.IsEmpty())
         return;

      // Reinsert everything                                            
      ZeroMemory(mInfo, count);
      mValues.mCount = 0;
      SAFETY(oldKeys.mCount = oldCount);
      SAFETY(oldValues.mCount = oldCount);
      auto key = oldKeys.GetElement();
      auto val = oldValues.GetElement();
      const auto hashmask = count - 1;
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            InsertInnerUnknown<false>(
               GetBucketUnknown(hashmask, key),
               Abandon(key),
               Abandon(val)
            );

            if (not key.IsEmpty())
               key.CallUnknownDestructors();
            else
               key.mCount = 1;

            if (not val.IsEmpty())
               val.CallUnknownDestructors();
            else
               val.mCount = 1;
         }

         ++oldInfo;
         key.Next();
         val.Next();
      }
      
      if (oldValues.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         // (keys are always present, if values are present)            
         if (oldValues.mEntry->GetUses() > 1)
            oldValues.mEntry->Free();
         else {
            Allocator::Deallocate(oldValues.mEntry);
            Allocator::Deallocate(oldKeys.mEntry);
         }
      }
   }
   
   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - number of pairs to allocate                           
   LANGULUS(INLINED)
   void BlockMap::AllocateInner(const Count& count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() and GetUses() == 1)
         AllocateData<true>(count);
      else
         AllocateData<false>(count);
   }
   
   /// Reference memory block if we own it                                    
   ///   @param times - number of references to add                           
   LANGULUS(INLINED)
   void BlockMap::Reference(const Count& times) const noexcept {
      mValues.Reference(times);
   }
   
   /// Reference memory block once                                            
   LANGULUS(INLINED)
   void BlockMap::Keep() const noexcept {
      Reference(1);
   }
         
   /// Dereference memory block                                               
   ///   @attention this never modifies any state, except mValues.mEntry      
   ///   @tparam DESTROY - whether to call destructors on full dereference    
   ///   @param times - number of references to subtract                      
   template<bool DESTROY>
   void BlockMap::Dereference(const Count& times) {
      if (not mValues.mEntry)
         return;

      LANGULUS_ASSUME(DevAssumes,
         mValues.mEntry->GetUses() >= times, "Bad memory dereferencing");

      if (mValues.mEntry->GetUses() == 1) {
         if constexpr (DESTROY) {
            if (not IsEmpty()) {
               // Destroy all keys and values                           
               ClearInner();
            }
         }

         // Deallocate stuff                                            
         Allocator::Deallocate(mKeys.mEntry);
         Allocator::Deallocate(mValues.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref values     
         // Notice how we don't dereference keys, since we use only the 
         // values' references to save on some redundancy               
         mValues.mEntry->Free();
      }
   }

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @attention this never modifies any state, except mValues.mEntry      
   LANGULUS(INLINED)
   void BlockMap::Free() {
      return Dereference<true>(1);
   }

} // namespace Langulus::Anyness
