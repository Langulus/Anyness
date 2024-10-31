///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../BlockMap.hpp"


namespace Langulus::Anyness
{

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Reserve(const Count count) {
      AllocateInner<THIS>(
         Roof2(count < MinimalAllocation ? MinimalAllocation : count)
      );
   }
   
   /// Allocate a fresh set keys and values (for internal use only)           
   ///   @attention doesn't initialize anything, but the memory state         
   ///   @attention doesn't modify count, doesn't set info sentinel           
   ///   @attention assumes count is a power-of-two                           
   ///   @param count - the new number of pairs                               
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::AllocateFresh(const Count count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      const auto keyAndInfoSize = RequestKeyAndInfoSize<THIS>(count, infoOffset);
      mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      const auto valueByteSize = RequestValuesSize(count);
      mValues.mEntry = Allocator::Allocate(mValues.mType, valueByteSize);

      if (not mValues.mEntry) {
         Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate, "Out of memory");
      }

      mValues.mRaw = const_cast<Byte*>(mValues.mEntry->GetBlockStart());
      mKeys.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = const_cast<Byte*>(mKeys.mEntry->GetBlockStart());
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
   }

   /// Allocate or reallocate key, value, and info array                      
   ///   @attention assumes count is a power-of-two                           
   ///   @attention assumes key and value types have been set prior           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   template<CT::Map THIS, bool REUSE>
   void BlockMap::AllocateData(const Count count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, mKeys.mType and mValues.mType,
         "Key and value types haven't been set");

      if constexpr (REUSE) {
         LANGULUS_ASSUME(DevAssumes,
            mKeys.GetUses() == 1 and mValues.GetUses() == 1,
            "Can't reuse memory of a map used from multiple places, "
            "BranchOut should've been called prior to AllocateData"
         );
      }

      Offset infoOffset;
      BlockMap old = *this;

      // Allocate new keys                                              
      const auto keyAndInfoSize = RequestKeyAndInfoSize<THIS>(count, infoOffset);
      if constexpr (REUSE)
         mKeys.mEntry = Allocator::Reallocate(
            keyAndInfoSize, const_cast<Allocation*>(mKeys.mEntry));
      else
         mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);

      LANGULUS_ASSERT(mKeys.mEntry, Allocate,
         "Out of memory on allocating/reallocating keys");

      // Allocate new values                                            
      const auto valueByteSize = RequestValuesSize(count);
      if constexpr (REUSE)
         mValues.mEntry = Allocator::Reallocate(
            valueByteSize, const_cast<Allocation*>(mValues.mEntry));
      else
         mValues.mEntry = Allocator::Allocate(mValues.mType, valueByteSize);

      if (not mValues.mEntry) {
         Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate,
            "Out of memory on allocating/reallocating values");
      }

      mValues.mRaw = const_cast<Byte*>(mValues.mEntry->GetBlockStart());
      mKeys.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = const_cast<Byte*>(mKeys.mEntry->GetBlockStart());
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
      // Set the sentinel                                               
      mInfo[count] = 1;

      // Zero or move the info array                                    
      if constexpr (REUSE) {
         // Check if any data was reused                                
         if (mKeys.mEntry == old.mKeys.mEntry
         or mValues.mEntry == old.mValues.mEntry) {
            // No escape from this scope                                
            // Check if keys were reused                                
            if (mKeys.mEntry == old.mKeys.mEntry) {
               if (mValues.mEntry == old.mValues.mEntry) {
                  // Both keys and values come from 'this'              
                  // Reusing keys means reusing info, but it shifts     
                  // Moving memory to account for potential overlap     
                  //TODO is overlap really possible, if map always doubles??
                  MoveMemory(mInfo, old.mInfo, old.GetReserved());
                  // Make sure new info data is zeroed                  
                  ZeroMemory(mInfo + old.GetReserved(), count - old.GetReserved());

                  RehashBoth<THIS>(old.GetReserved());
               }
               else {
                  // Keys come from 'this', values come from 'old'      
                  RehashKeys<THIS>(old);
               }
            }
            else {
               // Keys come from 'old', values come from 'this'         
               RehashVals<THIS>(old);
            }

            return;
         }
      }

      // If reached, then both keys and values are newly allocated      
      ZeroMemory(mInfo, count);
      if (old.IsEmpty()) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      mKeys.mCount = 0;

      UNUSED() auto& me = reinterpret_cast<const THIS&>(*this);
      auto key = old.GetKeyHandle<THIS>(0);
      auto val = old.GetValHandle<THIS>(0);
      const auto hashmask = GetReserved() - 1;
      const auto infoend = old.GetInfoEnd();

      while (old.mInfo != infoend) {
         if (*old.mInfo) {
            if constexpr (CT::TypedMap<THIS>) {
               InsertInner<THIS, false>(
                  GetBucket(hashmask, key.Get()),
                  Abandon(key), Abandon(val)
               );
               key.FreeInner();
               val.FreeInner();
            }
            else {
               InsertBlockInner<THIS, false>(
                  GetBucketUnknown(hashmask, key),
                  Abandon(key), Abandon(val)
               );

               if (key)
                  key.FreeInner();
               else
                  key.mCount = 1;

               if (val)
                  val.FreeInner();
               else
                  val.mCount = 1;
            }
         }

         ++key;
         ++val;
         ++old.mInfo;
      }

      // Free the old allocations                                       
      if constexpr (REUSE) {
         // When reusing, keys and values can potentially remain same   
         // Avoid deallocating them if that's the case                  
         if (old.mValues.mEntry != mValues.mEntry)
            Allocator::Deallocate(const_cast<Allocation*>(old.mValues.mEntry));

         if (old.mKeys.mEntry != mKeys.mEntry)
            Allocator::Deallocate(const_cast<Allocation*>(old.mKeys.mEntry));
      }
      else {
         // Not reusing, so either deallocate, or dereference           
         if (old.mKeys.mEntry) {
            if (old.mKeys.mEntry->GetUses() > 1)
               const_cast<Allocation*>(old.mKeys.mEntry)->Free();
            else
               Allocator::Deallocate(const_cast<Allocation*>(old.mKeys.mEntry));
         }

         if (old.mValues.mEntry) {
            if (old.mValues.mEntry->GetUses() > 1)
               const_cast<Allocation*>(old.mValues.mEntry)->Free();
            else
               Allocator::Deallocate(const_cast<Allocation*>(old.mValues.mEntry));
         }
      }
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - number of pairs to allocate                           
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::AllocateInner(Count count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() and mKeys.GetUses() == 1 and mValues.GetUses() == 1)
         AllocateData<THIS, true>(count);
      else
         AllocateData<THIS, false>(count);
   }
   
   /// Reference memory block once                                            
   template<CT::Map THIS, bool DEEP> LANGULUS(INLINED)
   void BlockMap::Keep() const noexcept {
      if (mKeys.mEntry) {
         const_cast<Allocation*>(mKeys.mEntry)->Keep(1);

         if constexpr (DEEP)
            GetKeys<THIS>().KeepInner(mInfo);
      }

      if (mValues.mEntry) {
         const_cast<Allocation*>(mValues.mEntry)->Keep(1);

         if constexpr (DEEP)
            GetVals<THIS>().KeepInner(mInfo);
      }
   }

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @attention this doesn't modify any immediate map state               
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Free() {
      // Always destroy values first, because keys also contain mInfo   
      if (mValues.mEntry) {
         LANGULUS_ASSUME(DevAssumes, mValues.mEntry->GetUses() >= 1,
            "Bad value memory dereferencing");

         if (mValues.mEntry->GetUses() == 1) {
            if (not IsEmpty())
               GetVals<THIS>().FreeInner(mInfo);

            // Deallocate values                                        
            Allocator::Deallocate(const_cast<Allocation*>(mValues.mEntry));
         }
         else {
            // Dereference values                                       
            if (not IsEmpty())
               GetVals<THIS>().template FreeInner<false>(mInfo);

            const_cast<Allocation*>(mValues.mEntry)->Free();
         }

         mValues.mEntry = nullptr;
      }

      if (mKeys.mEntry) {
         LANGULUS_ASSUME(DevAssumes, mKeys.mEntry->GetUses() >= 1,
            "Bad key memory dereferencing");

         if (mKeys.mEntry->GetUses() == 1) {
            if (not IsEmpty())
               GetKeys<THIS>().FreeInner(mInfo);

            // Deallocate keys                                          
            Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
         }
         else {
            // Dereference keys                                         
            if (not IsEmpty())
               GetKeys<THIS>().template FreeInner<false>(mInfo);

            const_cast<Allocation*>(mKeys.mEntry)->Free();
         }

         mKeys.mEntry = nullptr;
      }
   }

} // namespace Langulus::Anyness
