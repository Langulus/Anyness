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
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param count - number of pairs to allocate                           
   template<class MAP> LANGULUS(INLINED)
   void BlockMap::Reserve(const Count& count) {
      AllocateInner<MAP>(
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
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   template<bool REUSE, class MAP>
   void BlockMap::AllocateData(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, mKeys.mType and mValues.mType,
         "Key and value types haven't been set");

      Offset infoOffset;
      auto oldInfo = mInfo;
      const auto oldCount = GetReserved();
      const auto oldInfoEnd = oldInfo + oldCount;

      // Allocate new keys                                              
      Block oldKeys {mKeys};
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      if constexpr (REUSE)
         mKeys.mEntry = Allocator::Reallocate(
            keyAndInfoSize, const_cast<Allocation*>(mKeys.mEntry));
      else
         mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);

      LANGULUS_ASSERT(mKeys.mEntry, Allocate,
         "Out of memory on allocating/reallocating keys");

      // Allocate new values                                            
      Block oldVals {mValues};
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

            if (mValues.mEntry == oldVals.mEntry) {
               // Both keys and values remain in the same place         
               // Data was reused, but entries always move if sparse val
               if (mValues.IsSparse()) {
                  MoveMemory(
                     mValues.mRawSparse + count,
                     mValues.mRawSparse + oldCount,
                     oldCount
                  );
               };

               Rehash<MAP>(oldCount);
            }
            else {
               // Only values moved, reinsert them, rehash the rest     
               RehashKeys<MAP>(oldCount, oldVals);
               Allocator::Deallocate(const_cast<Allocation*>(oldVals.mEntry));
            }
            return;
         }
         else if (mValues.mEntry == oldVals.mEntry) {
            // Only keys moved, reinsert them, rehash the rest          
            RehashValues<MAP>(oldCount, oldKeys);
            Allocator::Deallocate(const_cast<Allocation*>(oldKeys.mEntry));
            return;
         }
      }

      // If reached, then both keys and values are newly allocated      
      ZeroMemory(mInfo, count);
      if (not oldVals) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      static_assert(CT::Map<MAP>, "MAP must be a map type");
      UNUSED() auto& THIS = reinterpret_cast<const MAP&>(*this); //TODO
      mValues.mCount = 0;
      IF_SAFE(oldKeys.mCount = oldCount);
      IF_SAFE(oldVals.mCount = oldCount);

      auto key = [&oldKeys]{
         if constexpr (CT::TypedMap<MAP>)
            return oldKeys.template GetHandle<typename MAP::Key>(0);
         else
            return oldKeys.GetElement();
      }();
      auto val = [&oldVals]{
         if constexpr (CT::TypedMap<MAP>)
            return oldVals.template GetHandle<typename MAP::Value>(0);
         else
            return oldVals.GetElement();
      }();

      const auto hashmask = GetReserved() - 1;
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            if constexpr (CT::TypedMap<MAP>) {
               InsertInner<false, MAP::Ordered>(
                  GetBucket(hashmask, key.Get()),
                  Abandon(key), Abandon(val)
               );
               key.Destroy();
               val.Destroy();
            }
            else {
               InsertInnerUnknown<false, MAP::Ordered>(
                  GetBucketUnknown(hashmask, key),
                  Abandon(key), Abandon(val)
               );

               if (key)
                  key.CallUnknownDestructors();
               else
                  key.mCount = 1;

               if (val)
                  val.CallUnknownDestructors();
               else
                  val.mCount = 1;
            }
         }

         if constexpr (CT::TypedMap<MAP>) {
            ++key;
            ++val;
         }
         else {
            key.Next();
            val.Next();
         }

         ++oldInfo;
      }

      // Free the old allocations                                       
      if constexpr (REUSE) {
         // When reusing, keys and values can potentially remain same   
         // Avoid deallocating them if that's the case                  
         if (oldVals.mEntry != mValues.mEntry) {
            LANGULUS_ASSUME(DevAssumes, oldVals.mEntry->GetUses() == 1,
               "Bad assumption");
            Allocator::Deallocate(const_cast<Allocation*>(oldVals.mEntry));
         }

         if (oldKeys.mEntry != mKeys.mEntry) {
            LANGULUS_ASSUME(DevAssumes, oldKeys.mEntry->GetUses() == 1,
               "Bad assumption");
            Allocator::Deallocate(const_cast<Allocation*>(oldKeys.mEntry));
         }
      }
      else if (oldVals.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         // (keys are always present, if values are present)            
         if (oldVals.mEntry->GetUses() > 1) {
            const_cast<Allocation*>(oldVals.mEntry)->Free();
            LANGULUS_ASSUME(DevAssumes, oldKeys.mEntry->GetUses() == 1,
               "Bad assumption");
         }
         else {
            LANGULUS_ASSUME(DevAssumes, oldKeys.mEntry->GetUses() == 1,
               "Bad assumption");
            Allocator::Deallocate(const_cast<Allocation*>(oldVals.mEntry));
            Allocator::Deallocate(const_cast<Allocation*>(oldKeys.mEntry));
         }
      }
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param count - number of pairs to allocate                           
   template<class MAP> LANGULUS(INLINED)
   void BlockMap::AllocateInner(const Count& count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() and GetUses() == 1)
         AllocateData<true,  MAP>(count);
      else
         AllocateData<false, MAP>(count);
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

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @attention this doesn't modify any immediate map state               
   template<class MAP> LANGULUS(INLINED)
   void BlockMap::Free() {
      if (not mValues.mEntry)
         return;

      LANGULUS_ASSUME(DevAssumes,
         mValues.mEntry->GetUses() >= 1, "Bad memory dereferencing");

      if (mValues.mEntry->GetUses() == 1) {
         if (not IsEmpty())
            ClearInner<MAP>();

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
   }

} // namespace Langulus::Anyness
