///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Allocator.hpp"
#include <RTTI/Assume.hpp>

namespace Langulus::Anyness
{
   
   /// MSVC will likely never support std::aligned_alloc, so we use           
   /// a custom portable routine that's almost the same                       
   /// https://stackoverflow.com/questions/62962839                           
   ///                                                                        
   /// Each allocation has the following prefixed bytes:                      
   /// [padding][T::GetSize()][client bytes...]                               
   ///                                                                        
   ///   @param size - the number of client bytes to allocate                 
   ///   @return a newly allocated memory that is correctly aligned           
   template<AllocationPrimitive T>
   T* AlignedAllocate(DMeta hint, const Size& size) SAFETY_NOEXCEPT() {
      const auto finalSize = T::GetNewAllocationSize(size) + Alignment;
      const auto base = ::std::malloc(finalSize);
      if (!base) UNLIKELY()
         return nullptr;

      // Align pointer to the alignment LANGULUS was built with         
      auto ptr = reinterpret_cast<T*>(
         (reinterpret_cast<Size>(base) + Alignment)
         & ~(Alignment - Size {1})
      );

      // Place the entry there                                          
      new (ptr) T {hint, size, base};
      return ptr;
   }

   /// Global allocator interface                                             
   Allocator Fractalloc {};

   /// Allocate a memory entry                                                
   ///   @attention doesn't call any constructors                             
   ///   @attention doesn't throw - check if return is nullptr                
   ///   @attention assumes size is not zero                                  
   ///   @param hint - optional meta data to associate pool with              
   ///   @param size - the number of bytes to allocate                        
   ///   @return the allocation, or nullptr if out of memory                  
   Allocation* Allocator::Allocate(RTTI::DMeta hint, const Size& size) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, size, "Zero allocation is not allowed");

      // Decide pool chain, based on hint                               
      Pool* pool {};
      if (hint) {
         switch (hint->mPoolTactic) {
         case RTTI::PoolTactic::Size:
            pool = mSizePoolChain[Inner::FastLog2(hint->mSize)];
            break;
         case RTTI::PoolTactic::Type:
            pool = static_cast<Pool*>(const_cast<MetaData*>(hint)->mPool);
            break;
         case RTTI::PoolTactic::Default:
            pool = mDefaultPoolChain;
            break;
         }
      }
      else pool = mDefaultPoolChain;

      //	Attempt to place allocation in the default chain               
      Allocation* memory {};
      while (pool) {
         memory = pool->Allocate(size);
         if (memory) {
            #if LANGULUS_FEATURE(MEMORY_STATISTICS)
               mStatistics.mEntries += 1;
               mStatistics.mBytesAllocatedByFrontend += memory->GetTotalSize();
            #endif
            return memory;
         }

         pool = pool->mNext;
      }

      if (memory)
         return memory;

      // If reached, pool chain can't contain the memory                
      // Allocate a new pool and add it at the front of hinted chain    
      pool = AllocatePool(nullptr, Allocation::GetNewAllocationSize(size));
      if (!pool)
         return nullptr;

      memory = pool->Allocate(size);

      if (hint) {
         switch (hint->mPoolTactic) {
         case RTTI::PoolTactic::Size: {
            auto& sizeChain = mSizePoolChain[Inner::FastLog2(hint->mSize)];
            pool->mNext = sizeChain;
            sizeChain = pool;
            break;
         }
         case RTTI::PoolTactic::Type:
            pool->mNext = static_cast<Pool*>(hint->mPool);
            const_cast<MetaData*>(hint)->mPool = pool;
            mInstantiatedTypes.insert(hint);
            break;
         case RTTI::PoolTactic::Default:
            pool->mNext = mDefaultPoolChain;
            mDefaultPoolChain = pool;
            break;
         }
      }
      else {
         pool->mNext = mDefaultPoolChain;
         mDefaultPoolChain = pool;
      }

      #if LANGULUS_FEATURE(MEMORY_STATISTICS)
         mStatistics.AddPool(pool);
      #endif
      return memory;
   }

   /// Reallocate a memory entry                                              
   ///   @attention never calls any constructors                              
   ///   @attention never copies any data                                     
   ///   @attention never deallocates previous entry                          
   ///   @attention returned entry might be different from the previous       
   ///   @attention doesn't throw - check if return is nullptr                
   ///   @param size - the number of bytes to allocate                        
   ///   @param previous - the previous memory entry                          
   ///   @return the reallocated memory entry, or nullptr if out of memory    
   Allocation* Allocator::Reallocate(const Size& size, Allocation* previous) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, previous,
         "Reallocating nullptr");
      LANGULUS_ASSUME(DevAssumes, size != previous->GetAllocatedSize(),
         "Reallocation suboptimal - size is same as previous");
      LANGULUS_ASSUME(DevAssumes, size,
         "Zero reallocation is not allowed");
      LANGULUS_ASSUME(DevAssumes, previous->mReferences,
         "Deallocating an unused allocation");

      #if LANGULUS_FEATURE(MEMORY_STATISTICS)
         const auto oldSize = previous->GetTotalSize();
      #endif

      // New size is bigger, precautions must be taken                  
      if (previous->mPool->Reallocate(previous, size)) {
         #if LANGULUS_FEATURE(MEMORY_STATISTICS)
            mStatistics.mBytesAllocatedByFrontend -= oldSize;
            mStatistics.mBytesAllocatedByFrontend += previous->GetTotalSize();
         #endif
         return previous;
      }

      // If this is reached, we have a collision, so new entry is made  
      return Allocate(previous->mPool->mMeta, size);
   }
   
   /// Deallocate a memory allocation                                         
   ///   @attention assumes entry is a valid entry under jurisdiction         
   ///   @attention doesn't call any destructors                              
   ///   @param entry - the memory entry to deallocate                        
   void Allocator::Deallocate(Allocation* entry) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, entry,
         "Deallocating nullptr");
      LANGULUS_ASSUME(DevAssumes, entry->GetAllocatedSize(),
         "Deallocating an empty allocation");
      LANGULUS_ASSUME(DevAssumes, entry->mReferences,
         "Deallocating an unused allocation");
      LANGULUS_ASSUME(DevAssumes, entry->mReferences == 1,
         "Deallocating an allocation used from multiple places");

      #if LANGULUS_FEATURE(MEMORY_STATISTICS)
         mStatistics.mBytesAllocatedByFrontend -= entry->GetTotalSize();
         mStatistics.mEntries -= 1;
      #endif

      entry->mPool->Deallocate(entry);
   }

   /// Allocate a pool                                                        
   ///   @attention the pool must be deallocated with DeallocatePool          
   ///   @param hint - optional meta data to associate pool with              
   ///   @param size - size of the pool (in bytes)                            
   ///   @return a pointer to the new pool                                    
   Pool* Allocator::AllocatePool(DMeta hint, const Size& size) SAFETY_NOEXCEPT() {
      const auto poolSize = ::std::max(Pool::DefaultPoolSize, Roof2(size));
      return AlignedAllocate<Pool>(hint, poolSize);
   }

   /// Deallocate a pool                                                      
   ///   @attention doesn't call any destructors                              
   ///   @attention pool or any entry inside is no longer valid after this    
   ///   @attention assumes pool is a valid pointer                           
   ///   @param pool - the pool to deallocate                                 
   void Allocator::DeallocatePool(Pool* pool) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, pool, "Nullptr provided");
      ::std::free(pool->mHandle);
   }

   /// Deallocates all unused pools in a chain                                
   ///   @param chainStart - [in/out] the start of the chain                  
   void Allocator::CollectGarbageChain(Pool*& chainStart) {
      while (chainStart) {
         if (chainStart->IsInUse())
            break;

         #if LANGULUS_FEATURE(MEMORY_STATISTICS)
            mStatistics.DelPool(chainStart);
         #endif

         auto next = chainStart->mNext;
         DeallocatePool(chainStart);
         chainStart = next;
      }

      if (!chainStart)
         return;

      auto prev = chainStart;
      auto pool = chainStart->mNext;
      while (pool) {
         if (pool->IsInUse()) {
            prev = pool;
            pool = pool->mNext;
            continue;
         }

         #if LANGULUS_FEATURE(MEMORY_STATISTICS)
            mStatistics.DelPool(pool);
         #endif

         const auto next = pool->mNext;
         DeallocatePool(pool);
         prev->mNext = next;
         pool = next;
      }
   }
   
   /// Deallocates all unused pools                                           
   void Allocator::CollectGarbage() {
      mLastFoundPool = nullptr;

      // Cleanup the default chain                                      
      CollectGarbageChain(mDefaultPoolChain);

      // Cleanup all size chains                                        
      for (auto& sizeChain : mSizePoolChain)
         CollectGarbageChain(sizeChain);

      // Cleanup all type chains                                        
      for (auto typeChain =  mInstantiatedTypes.begin(); 
                typeChain != mInstantiatedTypes.end();
      ) {
         CollectGarbageChain(reinterpret_cast<Pool*&>(
            const_cast<MetaData*>(*typeChain)->mPool));

         // Also discard the type if no pools remain                    
         if ((*typeChain)->mPool == nullptr)
            typeChain = mInstantiatedTypes.erase(typeChain);
         else
            ++typeChain;
      }
   }

   /// Check RTTI boundary for allocated pools                                
   /// Useful to decide when shared library is no longer used and is ready    
   /// to be unloaded. Use it after a call to CollectGarbage                  
   ///   @param boundary - the boundary name                                  
   ///   @return the number of pools                                          
   Count Allocator::CheckBoundary(const Token& boundary) const noexcept {
      Count count {};
      for (auto type : mInstantiatedTypes) {
         if (type->mLibraryName == boundary) {
            auto pool = static_cast<const Pool*>(type->mPool);
            while (pool) {
               ++count;
               pool = pool->mNext;
            }
         }
      }
      return count;
   }

   /// Search in a pool chain                                                 
   ///   @param memory - memory pointer                                       
   ///   @param pool - start of the pool chain                                
   ///   @return the memory entry that contains the memory pointer, or        
   ///           nullptr if memory is not ours, its entry is no longer used   
   Allocation* Allocator::FindInChain(const void* memory, Pool* pool) const SAFETY_NOEXCEPT() {
      while (pool) {
         const auto found = pool->Find(memory);
         if (found) {
            mLastFoundPool = pool;
            return found;
         }

         // Continue inside the poolchain                               
         pool = pool->mNext;
      }

      return nullptr;
   }
   
   /// Search if memory is contained inside a pool chain                      
   ///   @param memory - memory pointer                                       
   ///   @param pool - start of the pool chain                                
   ///   @return true if we have authority over the memory                    
   bool Allocator::ContainedInChain(const void* memory, Pool* pool) const SAFETY_NOEXCEPT() {
      while (pool) {
         if (pool->Contains(memory))
            return true;

         // Continue inside the poolchain                               
         pool = pool->mNext;
      }

      return false;
   }

   /// Find a memory entry from pointer                                       
   /// Allows us to safely interface unknown memory, possibly reusing it      
   /// Optimized for consecutive searches in near memory                      
   ///   @param hint - the type of data to search for (optional)              
   ///                 always provide hint for optimal performance            
   ///   @param memory - memory pointer                                       
   ///   @return the memory entry that contains the memory pointer, or        
   ///           nullptr if memory is not ours, its entry is no longer used   
   Allocation* Allocator::Find(DMeta hint, const void* memory) const SAFETY_NOEXCEPT() {
      // Scan the last pool that found something (hot region)           
      //TODO consider a whole stack of those?
      if (mLastFoundPool) {
         const auto found = mLastFoundPool->Find(memory);
         if (found)
            return found;
      }

      // Decide pool chains, based on hint                              
      Allocation* result;
      if (hint) {
         switch (hint->mPoolTactic) {
         case RTTI::PoolTactic::Size: {
            // Hint is sized, so check in size pool chain first         
            const auto sizebucket = Inner::FastLog2(hint->mSize);
            result = FindInChain(memory, mSizePoolChain[sizebucket]);
            if (result)
               return result;

            // Then check default pool chain                            
            // (pointer could be a member of default-pooled type)       
            result = FindInChain(memory, mDefaultPoolChain);
            if (result)
               return result;

            // Check all typed pool chains                              
            // (pointer could be a member of type-pooled type)          
            for (auto& type : mInstantiatedTypes) {
               result = FindInChain(memory, static_cast<Pool*>(type->mPool));
               if (result)
                  return result;
            }

            // Finally, check all other size pool chains                
            // (pointer could be a member of differently sized type)    
            for (Size i = 0; i < sizebucket; ++i) {
               result = FindInChain(memory, mSizePoolChain[i]);
               if (result)
                  return result;
            }
            for (Size i = sizebucket + 1; i < SizeBuckets; ++i) {
               result = FindInChain(memory, mSizePoolChain[i]);
               if (result)
                  return result;
            }
         } return nullptr;
         case RTTI::PoolTactic::Type:
            // Hint is typed, so check in its typed pool chain first    
            result = FindInChain(memory, static_cast<Pool*>(hint->mPool));
            if (result)
               return result;

            // Then check default pool chain                            
            // (pointer could be a member of default-pooled type)       
            result = FindInChain(memory, mDefaultPoolChain);
            if (result)
               return result;

            // Check all size pool chains                               
            // (pointer could be a member of a size-pooled type)        
            for (auto& sizepool : mSizePoolChain) {
               result = FindInChain(memory, sizepool);
               if (result)
                  return result;
            }

            // Finally, check all type pool chains                      
            // (pointer could be a member of a type-pooled type)        
            for (auto& typepool : mInstantiatedTypes) {
               if (typepool == hint)
                  continue;

               result = FindInChain(memory, static_cast<Pool*>(typepool->mPool));
               if (result)
                  return result;
            }

            return nullptr;
         case RTTI::PoolTactic::Default:
            break;
         }
      }

      // If reached, either no hint is provided, or PoolTactic::Default 
      //  Check default pool chain                                      
      result = FindInChain(memory, mDefaultPoolChain);
      if (result)
         return result;

      // Check all size pool chains                                     
      // (pointer could be a member of a size-pooled type)              
      for (auto& sizepool : mSizePoolChain) {
         result = FindInChain(memory, sizepool);
         if (result)
            return result;
      }

      // Finally, check all type pool chains                            
      // (pointer could be a member of a type-pooled type)              
      for (auto& typepool : mInstantiatedTypes) {
         result = FindInChain(memory, static_cast<Pool*>(typepool->mPool));
         if (result)
            return result;
      }

      // If reahced, then memory is guaranteed to not be ours           
      return nullptr;
   }

   /// Check if memory is owned by the memory manager                         
   /// Unlike Allocator::Find, this doesn't check if memory is currently used 
   /// but returns true, as long as the required pool is still available      
   ///   @attention assumes memory is a valid pointer                         
   ///   @param hint - the type of data to search for (optional)              
   ///   @param memory - memory pointer                                       
   ///   @return true if we own the memory                                    
   bool Allocator::CheckAuthority(DMeta hint, const void* memory) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, memory, "Nullptr provided");

      // Scan the last pool that found something (hot region)           
      //TODO consider a whole stack of those?
      if (mLastFoundPool) {
         const auto found = mLastFoundPool->Find(memory);
         if (found)
            return found;
      }

      // Decide pool chains, based on hint                              
      if (hint) {
         switch (hint->mPoolTactic) {
         case RTTI::PoolTactic::Size: {
            // Hint is sized, so check in size pool chain first         
            const auto sizebucket = Inner::FastLog2(hint->mSize);
            if (ContainedInChain(memory, mSizePoolChain[sizebucket]))
               return true;

            // Then check default pool chain                            
            // (pointer could be a member of default-pooled type)       
            if (ContainedInChain(memory, mDefaultPoolChain))
               return true;

            // Check all typed pool chains                              
            // (pointer could be a member of type-pooled type)          
            for (auto& type : mInstantiatedTypes) {
               if (ContainedInChain(memory, static_cast<Pool*>(type->mPool)))
                  return true;
            }

            // Finally, check all other size pool chains                
            // (pointer could be a member of differently sized type)    
            for (Size i = 0; i < sizebucket; ++i) {
               if (ContainedInChain(memory, mSizePoolChain[i]))
                  return true;
            }
            for (Size i = sizebucket + 1; i < SizeBuckets; ++i) {
               if (ContainedInChain(memory, mSizePoolChain[i]))
                  return true;
            }
         } return false;

         case RTTI::PoolTactic::Type:
            // Hint is typed, so check in its typed pool chain first    
            if (ContainedInChain(memory, static_cast<Pool*>(hint->mPool)))
               return true;

            // Then check default pool chain                            
            // (pointer could be a member of default-pooled type)       
            if (ContainedInChain(memory, mDefaultPoolChain))
               return true;

            // Check all size pool chains                               
            // (pointer could be a member of a size-pooled type)        
            for (auto& sizepool : mSizePoolChain) {
               if (ContainedInChain(memory, sizepool))
                  return true;
            }

            // Finally, check all type pool chains                      
            // (pointer could be a member of a type-pooled type)        
            for (auto& typepool : mInstantiatedTypes) {
               if (typepool == hint)
                  continue;

               if (ContainedInChain(memory, static_cast<Pool*>(typepool->mPool)))
                  return true;
            }
            return false;

         case RTTI::PoolTactic::Default:
            break;
         }
      }

      // If reached, either no hint is provided, or PoolTactic::Default 
      //  Check default pool chain                                      
      if (ContainedInChain(memory, mDefaultPoolChain))
         return true;

      // Check all size pool chains                                     
      // (pointer could be a member of a size-pooled type)              
      for (auto& sizepool : mSizePoolChain) {
         if (ContainedInChain(memory, sizepool))
            return true;
      }

      // Finally, check all type pool chains                            
      // (pointer could be a member of a type-pooled type)              
      for (auto& typepool : mInstantiatedTypes) {
         if (ContainedInChain(memory, static_cast<Pool*>(typepool->mPool)))
            return true;
      }

      return false;
   }
   
#if LANGULUS_FEATURE(MEMORY_STATISTICS)

   /// Check for memory leaks, by retrieving the new memory manager state     
   /// and comparing it against this one                                      
   ///   @return true if no functional difference between the states          
   bool Allocator::State::Assert() {
      Fractalloc.CollectGarbage();

      if (mAvailable) {
         if (mState != Fractalloc.GetStatistics()) {
            // Assertion failure                                        
            Fractalloc.DumpPools();
            mState = Fractalloc.GetStatistics();
            return false;
         }
      }

      // All is fine                                                    
      mState = Fractalloc.GetStatistics();
      mAvailable = true;
      return true;
   }
   
   /// Get allocator statistics                                               
   ///   @return a reference to the statistics structure                      
   const Allocator::Statistics& Allocator::GetStatistics() const noexcept {
      return mStatistics;
   }

   /// Dump a single pool                                                     
   ///   @param id - pool id                                                  
   ///   @param pool - the pool to dump                                       
   void Allocator::DumpPool(Offset id, const Pool* pool) const noexcept {
      const auto scope = Logger::Info(Logger::Cyan, "Pool #", id, " at ",
         fmt::format("{:x}", reinterpret_cast<Pointer>(pool)), Logger::Tabs {});

      Logger::Info("Bytes in use/reserved: ", 
         Logger::Push, Logger::Green, pool->mAllocatedByFrontend, Logger::Pop,
         '/',
         Logger::Push, Logger::Red, pool->mAllocatedByBackend, Logger::Pop,
         " bytes"
      );
      Logger::Info("Min/Current/Max threshold: ", 
         Logger::Push, Logger::Green, pool->mThresholdMin, Logger::Pop,
         '/',
         Logger::Push, Logger::Yellow, pool->mThreshold, Logger::Pop,
         '/',
         Logger::Push, Logger::Red, pool->mAllocatedByBackend, Logger::Pop,
         " bytes"
      );

      if (pool->mMeta) {
         Logger::Info("Associated type: `",
            pool->mMeta->mCppName, "`, of size ", pool->mMeta->mSize, " bytes"
         );
      }

      if (pool->mEntries) {
         const auto escope = Logger::Info("Active entries: ",
            Logger::Push, Logger::Green, pool->mEntries, Logger::Pop, Logger::Tabs {}
         );

         Count ecounter {};
         do {
            const auto entry = pool->AllocationFromIndex(ecounter);
            if (entry->mReferences) {
               Logger::Info(ecounter, "] ", Logger::Green, entry->mAllocatedBytes, " bytes, ");
               Logger::Append(entry->mReferences, " references: `");
               auto raw = entry->GetBlockStart();
               for (Size i = 0; i < ::std::min(Size {32}, entry->mAllocatedBytes); ++i) {
                  if (::isprint(raw[i].mValue))
                     Logger::Append(static_cast<char>(raw[i].mValue));
                  else
                     Logger::Append('?');
               }

               if (entry->mAllocatedBytes > 32)
                  Logger::Append("...`");
               else
                  Logger::Append('`');
            }
            else Logger::Info(ecounter, "] ", Logger::Red, "unused entry");
         }
         while (++ecounter < pool->mEntries);
      }
   }

   /// Dump all currently allocated pools and entries, useful to locate leaks 
   void Allocator::DumpPools() const noexcept {
      Logger::Info("------------------ MANAGED MEMORY POOL DUMP START ------------------");

      // Dump default pool chain                                        
      {
         if (mDefaultPoolChain) {
            const auto scope =
               Logger::Info(Logger::Purple, "DEFAULT POOL CHAIN: ", Logger::Tabs {});

            Count counter {};
            auto pool = mDefaultPoolChain;
            while (pool) {
               DumpPool(counter, pool);
               pool = pool->mNext;
               ++counter;
            }
         }
      }

      // Dump every size pool chain                                     
      for (Size size = 0; size < sizeof(Size) * 8; ++size) {
         if (!mSizePoolChain[size])
            continue;

         const auto scope = 
            Logger::Info(Logger::Purple, "SIZE POOL CHAIN FOR ", (1 << size) ,": ", Logger::Tabs {});

         Count counter {};
         auto pool = mSizePoolChain[size];
         while (pool) {
            DumpPool(counter, pool);
            pool = pool->mNext;
            ++counter;
         }
      }
      
      // Dump every type pool chain                                     
      for (auto type : mInstantiatedTypes) {
         if (!type->mPool)
            continue;

         const auto scope = 
            Logger::Info(Logger::Purple, "TYPE POOL CHAIN FOR ", type->mCppName, 
               " (BOUNDARY: ", Logger::Push, Logger::Underline, type->mLibraryName, Logger::Pop,
               "): ", Logger::Tabs {});

         Count counter {};
         auto pool = static_cast<Pool*>(type->mPool);
         while (pool) {
            DumpPool(counter, pool);
            pool = pool->mNext;
            ++counter;
         }
      }

      Logger::Info("------------------  MANAGED MEMORY POOL DUMP END  ------------------");
   }

   /// Account for a newly allocated pool                                     
   ///   @param pool - the pool to account for                                
   void Allocator::Statistics::AddPool(const Pool* pool) noexcept {
      mBytesAllocatedByBackend += pool->GetTotalSize();
      mBytesAllocatedByFrontend += pool->GetAllocatedByFrontend();
      ++mPools;
      ++mEntries;
   }
   
   /// Account for a removed pool                                             
   ///   @param pool - the pool to account for                                
   void Allocator::Statistics::DelPool(const Pool* pool) noexcept {
      mBytesAllocatedByBackend -= pool->GetTotalSize();
      --mPools;
   }

#endif

} // namespace Langulus::Anyness::Inner
