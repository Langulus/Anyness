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
      mKeys.mReserved = mValues.mReserved = count;

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
      mKeys.mReserved = mValues.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = const_cast<Byte*>(mKeys.mEntry->GetBlockStart());
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
      // Set the sentinel                                               
      mInfo[count] = 1;

      // Zero or move the info array                                    
      if constexpr (REUSE) {
         // Check if keys were reused                                   
         if (mKeys.mEntry == old.mKeys.mEntry) {
            // Data was reused, but info always moves (null the rest)   
            MoveMemory(mInfo, old.mInfo, old.GetReserved());
            ZeroMemory(mInfo + old.GetReserved(), count - old.GetReserved());

            // Data was reused, but entries always move if sparse keys  
            if (IsKeySparse<THIS>()) {
               MoveMemory(
                  mKeys.mRawSparse + count,
                  mKeys.mRawSparse + old.GetReserved(),
                  old.GetReserved()
               );
            };

            if (mValues.mEntry == old.mValues.mEntry) {
               // Both keys and values remain in the same place         
               // Data was reused, but entries always move if sparse val
               if (IsValueSparse<THIS>()) {
                  MoveMemory(
                     mValues.mRawSparse + count,
                     mValues.mRawSparse + old.GetReserved(),
                     old.GetReserved()
                  );
               };

               Rehash<THIS>(old.GetReserved());
            }
            else {
               // Only values moved, reinsert them, rehash the rest     
               RehashKeys<THIS>(old);
               Allocator::Deallocate(
                  const_cast<Allocation*>(old.mValues.mEntry));
            }
            return;
         }
         else if (mValues.mEntry == old.mValues.mEntry) {
            // Only keys moved, reinsert them, rehash the rest          
            RehashVals<THIS>(old);
            Allocator::Deallocate(
               const_cast<Allocation*>(old.mKeys.mEntry));
            return;
         }
      }

      // If reached, then both keys and values are newly allocated      
      ZeroMemory(mInfo, count);
      if (not old.mValues) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      mValues.mCount = 0;

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
               key.Destroy();
               val.Destroy();
            }
            else {
               InsertBlockInner<THIS, false>(
                  GetBucketUnknown(hashmask, key),
                  Abandon(key), Abandon(val)
               );

               if (key)
                  key.Destroy();
               else
                  key.mCount = 1;

               if (val)
                  val.Destroy();
               else
                  val.mCount = 1;
            }
         }

         if constexpr (CT::TypedMap<THIS>) {
            ++key;
            ++val;
         }
         else {
            key.Next();
            val.Next();
         }

         ++old.mInfo;
      }

      // Free the old allocations                                       
      if constexpr (REUSE) {
         // When reusing, keys and values can potentially remain same   
         // Avoid deallocating them if that's the case                  
         if (old.mValues.mEntry != mValues.mEntry) {
            LANGULUS_ASSUME(DevAssumes, old.mValues.mEntry->GetUses() == 1,
               "Bad assumption");
            Allocator::Deallocate(const_cast<Allocation*>(old.mValues.mEntry));
         }

         if (old.mKeys.mEntry != mKeys.mEntry) {
            LANGULUS_ASSUME(DevAssumes, old.mKeys.mEntry->GetUses() == 1,
               "Bad assumption");
            Allocator::Deallocate(const_cast<Allocation*>(old.mKeys.mEntry));
         }
      }
      else if (old.mValues.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         // (keys are always present, if values are present)            
         if (old.mValues.mEntry->GetUses() > 1) {
            const_cast<Allocation*>(old.mValues.mEntry)->Free();
            LANGULUS_ASSUME(DevAssumes, old.mKeys.mEntry->GetUses() == 1,
               "Bad assumption");
         }
         else {
            LANGULUS_ASSUME(DevAssumes, old.mKeys.mEntry->GetUses() == 1,
               "Bad assumption");
            Allocator::Deallocate(const_cast<Allocation*>(old.mValues.mEntry));
            Allocator::Deallocate(const_cast<Allocation*>(old.mKeys.mEntry));
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
      if (IsAllocated() and GetUses() == 1)
         AllocateData<THIS, true>(count);
      else
         AllocateData<THIS, false>(count);
   }
   
   /// Reference memory block if we own it                                    
   ///   @param times - number of references to add                           
   LANGULUS(INLINED)
   void BlockMap::Reference(Count times) const noexcept {
      mValues.Reference(times);
   }
   
   /// Reference memory block once                                            
   LANGULUS(INLINED)
   void BlockMap::Keep() const noexcept {
      Reference(1);
   }

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @attention this doesn't modify any immediate map state               
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Free() {
      if (not mValues.mEntry)
         return;

      LANGULUS_ASSUME(DevAssumes,
         mValues.mEntry->GetUses() >= 1, "Bad memory dereferencing");

      if (mValues.mEntry->GetUses() == 1) {
         if (not IsEmpty())
            ClearInner<THIS>();

         // Deallocate stuff                                            
         LANGULUS_ASSUME(DevAssumes, mKeys.mEntry->GetUses() == 1,
            "Bad assumption");
         Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
         Allocator::Deallocate(const_cast<Allocation*>(mValues.mEntry));
      }
      else {
         // Data is used from multiple locations, just deref values     
         // Notice how we don't dereference keys, since we use only the 
         // values' references to save on some redundancy               
         const_cast<Allocation*>(mValues.mEntry)->Free();
      }

      mValues.mEntry = nullptr;
   }

} // namespace Langulus::Anyness
