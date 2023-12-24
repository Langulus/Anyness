///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockSet.hpp"


namespace Langulus::Anyness
{

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param count - number of pairs to allocate                           
   template<class SET> LANGULUS(INLINED)
   void BlockSet::Reserve(const Count& count) {
      AllocateInner<SET>(
         Roof2(count < MinimalAllocation ? MinimalAllocation : count)
      );
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
      mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      mKeys.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = const_cast<Byte*>(mKeys.mEntry->GetBlockStart());
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
   }

   /// Allocate or reallocate key and info array                              
   ///   @attention assumes count is a power-of-two                           
   ///   @attention assumes key type have been set prior                      
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param count - the new number of pairs                               
   template<bool REUSE, class SET>
   void BlockSet::AllocateData(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, mKeys.mType,
         "Key type haven't been set");
      static_assert(CT::Set<SET>, "SET must be a set type");
      UNUSED() auto& THIS = reinterpret_cast<const SET&>(*this); //TODO

      Offset infoOffset;
      auto oldInfo = mInfo;
      const auto oldCount = GetReserved();
      const auto oldInfoEnd = oldInfo + oldCount;

      // Allocate new keys                                              
      Block oldKeys {mKeys};
      const auto keyAndInfoSize = THIS.RequestKeyAndInfoSize(count, infoOffset);
      if constexpr (REUSE)
         mKeys.mEntry = Allocator::Reallocate(
            keyAndInfoSize, const_cast<Allocation*>(mKeys.mEntry));
      else
         mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);

      LANGULUS_ASSERT(mKeys.mEntry, Allocate,
         "Out of memory on allocating/reallocating keys");

      mKeys.mReserved = count;

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
            if (THIS.IsSparse()) {
               MoveMemory(
                  mKeys.mRawSparse + count,
                  mKeys.mRawSparse + oldCount,
                  oldCount
               );
            };

            Rehash<SET>(oldCount);
            return;
         }
      }

      // If reached, then keys are newly allocated                      
      ZeroMemory(mInfo, count);
      if (not oldKeys) {
         // There are no old values, the previous set was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys moved - reinsert all keys to rehash     
      mKeys.mCount = 0;
      IF_SAFE(oldKeys.mCount = oldCount);
      
      auto key = [&oldKeys]{
         if constexpr (CT::TypedSet<SET>)
            return oldKeys.template GetHandle<TypeOf<SET>>(0);
         else
            return oldKeys.GetElement();
      }();

      const auto hashmask = GetReserved() - 1;
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            if constexpr (CT::TypedSet<SET>) {
               InsertInner<false, SET::Ordered>(
                  GetBucket(hashmask, key.Get()),
                  Abandon(key)
               );
               key.Destroy();
            }
            else {
               InsertInnerUnknown<false, SET::Ordered>(
                  GetBucketUnknown(hashmask, key),
                  Abandon(key)
               );

               if (key)
                  key.CallUnknownDestructors();
               else
                  key.mCount = 1;
            }
         }

         if constexpr (CT::TypedSet<SET>) {
            ++key;
         }
         else {
            key.Next();
         }

         ++oldInfo;
      }

      // Free the old allocations                                       
      if (oldKeys.mEntry and oldKeys.mEntry != mKeys.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         if (oldKeys.mEntry->GetUses() > 1)
            const_cast<Allocation*>(oldKeys.mEntry)->Free();
         else
            Allocator::Deallocate(const_cast<Allocation*>(oldKeys.mEntry));
      }
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param count - number of pairs to allocate                           
   template<class SET> LANGULUS(INLINED)
   void BlockSet::AllocateInner(const Count& count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() and GetUses() == 1)
         AllocateData<true,  SET>(count);
      else
         AllocateData<false, SET>(count);
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
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param times - number of references to subtract                      
   template<bool DESTROY, class SET>
   void BlockSet::Dereference(const Count& times) {
      if (not mKeys.mEntry)
         return;

      LANGULUS_ASSUME(DevAssumes,
         mKeys.mEntry->GetUses() >= times, "Bad memory dereferencing");

      if (mKeys.mEntry->GetUses() == 1) {
         if constexpr (DESTROY) {
            if (not IsEmpty())
               ClearInner<SET>();
         }

         // Deallocate stuff                                            
         Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
      }
      else {
         // Data is used from multiple locations, just deref            
         const_cast<Allocation*>(mKeys.mEntry)->Free();
      }
   }

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @attention this never modifies any state, except mKeys.mEntry        
   template<class SET> LANGULUS(INLINED)
   void BlockSet::Free() {
      return Dereference<true, SET>(1);
   }

} // namespace Langulus::Anyness
