///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Allocator.hpp"
#include "Logger.hpp"

#if LANGULUS_FEATURE(MANAGED_MEMORY)
	#include "Pool.hpp"
#endif

namespace Langulus::Anyness::Inner
{

	/// Setup the default pool																	
	Pool* Allocator::mDefaultPool = nullptr;
	Pool* Allocator::mLastFoundPool = nullptr;
	
	/// Allocate a memory entry																
	///	@attention doesn't call any constructors										
	///	@attention doesn't throw - check if return is nullptr						
	///	@param size - the number of bytes to allocate								
	///	@return the allocation, or nullptr if out of memory						
	Allocation* Allocator::Allocate(const Size& size) SAFETY_NOEXCEPT() {
		SAFETY(if (0 == size)
			Throw<Except::Allocate>("Zero allocation is not allowed"));

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			//	Attempt to directly allocate in available pools					
			auto pool = mDefaultPool;
			while (pool) {
				auto memory = pool->Allocate(size);
				if (memory) {
					#if LANGULUS_FEATURE(MEMORY_STATISTICS)
						mStatistics.mEntries += 1;
						mStatistics.mBytesAllocatedByFrontend += memory->GetTotalSize();
					#endif
					return memory;
				}

				// Continue inside the poolchain if not able to allocate		
				pool = pool->mNext;
			}

			// If reached, available pools can't contain the memory			
			// Allocate a new pool and add it at the beginning of chain		
			const auto poolSize = ::std::max(
				Pool::DefaultPoolSize, 
				Roof2(Allocation::GetNewAllocationSize(size))
			);

			pool = AllocatePool(poolSize);
			if (!pool)
				return nullptr;

			auto memory = pool->Allocate(size);
			pool->mNext = mDefaultPool;
			mDefaultPool = pool;
			#if LANGULUS_FEATURE(MEMORY_STATISTICS)
				mStatistics.mBytesAllocatedByBackend += pool->GetTotalSize();
				mStatistics.mBytesAllocatedByFrontend += pool->GetAllocatedByFrontend();
				mStatistics.mPools += 1;
				mStatistics.mEntries += 1;
			#endif
			return memory;
		#else
			const auto result = Inner::AlignedAllocate<Allocation>(size);
			#if LANGULUS_FEATURE(MEMORY_STATISTICS)
				mStatistics.mBytesAllocatedByBackend += result->GetTotalSize();
				mStatistics.mBytesAllocatedByFrontend += result->GetAllocatedSize();
				mStatistics.mEntries += 1;
			#endif
			return result;
		#endif
	}

	/// Reallocate a memory entry																
	/// This actually works only when MANAGED_MEMORY feature is enabled			
	///	@attention never calls any constructors										
	///	@attention never copies any data													
	///	@attention never deallocates previous entry									
	///	@attention returned entry might be different from the previous			
	///	@attention doesn't throw - check if return is nullptr						
	///	@param size - the number of bytes to allocate								
	///	@param previous - the previous memory entry									
	///	@return the reallocated memory entry, or nullptr if out of memory		
	Allocation* Allocator::Reallocate(const Size& size, Allocation* previous) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (previous == nullptr)
				Throw<Except::Allocate>("Reallocating nullptr");
			if (size == previous->GetAllocatedSize())
				Throw<Except::Allocate>("Reallocation suboptimal - size is same as previous");
			if (size == 0)
				Throw<Except::Allocate>("Zero reallocation is not allowed");
			if (previous->mReferences == 0)
				Throw<Except::Allocate>("Deallocating an unused allocation");
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			// New size is bigger, precautions must be taken					
			const auto oldSize = previous->GetTotalSize();
			if (previous->mPool->Reallocate(previous, size)) {
				#if LANGULUS_FEATURE(MEMORY_STATISTICS)
					mStatistics.mBytesAllocatedByFrontend -= oldSize;
					mStatistics.mBytesAllocatedByFrontend += previous->GetTotalSize();
				#endif
				return previous;
			}

			// If this is reached, we have a collision, so memory moves		
			return Allocator::Allocate(size);
		#else
			return Allocator::Allocate(size);
		#endif
	}
	
	/// Deallocate a memory allocation														
	///	@attention assumes entry is a valid entry under jurisdiction			
	///	@attention doesn't call any destructors										
	///	@param entry - the memory entry to deallocate								
	void Allocator::Deallocate(Allocation* entry) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (entry == nullptr)
				Throw<Except::Allocate>("Deallocating nullptr");
			if (entry->GetAllocatedSize() == 0)
				Throw<Except::Allocate>("Deallocating an empty allocation");
			if (entry->mReferences == 0)
				Throw<Except::Allocate>("Deallocating an unused allocation");
		#endif

		#if LANGULUS_FEATURE(MEMORY_STATISTICS)
			mStatistics.mBytesAllocatedByFrontend -= entry->GetTotalSize();
			mStatistics.mEntries -= 1;
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			entry->mPool->Deallocate(entry);
		#else
			::std::free(entry->mPool);
		#endif
	}

#if LANGULUS_FEATURE(MANAGED_MEMORY)
	/// Allocate a pool																			
	///	@attention size must be power-of-two											
	///	@attention the pool must be deallocated with DeallocatePool				
	///	@param size size of the pool (in bytes)										
	///	@return a pointer to the new pool												
	Pool* Allocator::AllocatePool(const Size& size) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (!IsPowerOfTwo(size))
				Throw<Except::Allocate>("Pool size is not a power-of-two");
		#endif

		return Inner::AlignedAllocate<Pool>(size);
	}

	/// Deallocate a pool																		
	///	@attention doesn't call any destructors										
	///	@attention pool or any entry inside is no longer valid after this		
	///	@param pool - the pool to deallocate											
	void Allocator::DeallocatePool(Pool* pool) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (!pool)
				Throw<Except::Deallocate>("Trying to deallocate nullptr pool");
		#endif

		::std::free(pool->mHandle);
	}

	/// Deallocates all unused pools															
	void Allocator::CollectGarbage() {
		mLastFoundPool = nullptr;

		while (mDefaultPool) {
			if (mDefaultPool->IsInUse())
				break;

			#if LANGULUS_FEATURE(MEMORY_STATISTICS)
				mStatistics.mBytesAllocatedByBackend -= mDefaultPool->GetTotalSize();
				mStatistics.mPools -= 1;
			#endif

			auto next = mDefaultPool->mNext;
			DeallocatePool(mDefaultPool);
			mDefaultPool = next;
		}

		if (!mDefaultPool)
			return;

		auto prev = mDefaultPool;
		auto pool = mDefaultPool->mNext;
		while (pool) {
			if (pool->IsInUse()) {
				prev = pool;
				pool = pool->mNext;
				continue;
			}

			#if LANGULUS_FEATURE(MEMORY_STATISTICS)
				mStatistics.mBytesAllocatedByBackend -= pool->GetTotalSize();
				mStatistics.mPools -= 1;
			#endif

			const auto next = pool->mNext;
			DeallocatePool(pool);
			prev->mNext = next;
			pool = next;
		}
	}
#endif

	/// Find a memory entry from pointer													
	/// If LANGULUS_FEATURE(MANAGED_MEMORY) is enabled, this function will		
	/// attempt to find memory entry from the memory manager							
	/// Allows us to safely interface unknown memory, possibly reusing it		
	///	@attention assumes memory is a valid pointer									
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@return the memory entry that manages the memory pointer, or			
	///		nullptr if memory is not ours, or is no longer used					
	Allocation* Allocator::Find(DMeta meta, const void* memory) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (memory == nullptr)
				Throw<Except::Allocate>("Searching for nullptr");
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			// Scan the last last pool that found something (hot region)	
			if (mLastFoundPool) {
				const auto found = mLastFoundPool->Find(memory);
				if (found)
					return found;
			}

			// Scan all pools, and find one that contains the memory			
			auto pool = mDefaultPool;
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
		#else
			(void) (meta); (void) (memory);
			return nullptr;
		#endif
	}

	/// Check if memory is owned by the memory manager									
	/// Unlike Allocator::Find, this doesn't check if memory is currently used	
	/// and returns true, as long as the required pool is still available		
	///	@attention this function does nothing if										
	///              LANGULUS_FEATURE(MANAGED_MEMORY) is disabled					
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@return true if we own the memory												
	bool Allocator::CheckAuthority(DMeta meta, const void* memory) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (memory == nullptr)
				Throw<Except::Allocate>("Searching for nullptr");
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			// Scan the last last pool that found something (hot region)	
			if (mLastFoundPool) {
				const auto found = mLastFoundPool->Find(memory);
				if (found)
					return found;
			}

			// Scan all pools, and find one that contains the memory			
			auto pool = mDefaultPool;
			while (pool) {
				if (pool->Contains(memory))
					return true;

				// Continue inside the poolchain										
				pool = pool->mNext;
			}

			return false;
		#else
			(void) (meta); (void) (memory);
			return false;
		#endif
	}
	
	/// Get the number of uses an unknown memory pointer has							
	///	@attention this function does nothing if										
	///              LANGULUS_FEATURE(MANAGED_MEMORY) is disabled					
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@return the number of references, or 0 if memory is not ours			
	Count Allocator::GetReferences(DMeta meta, const void* memory) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (memory == nullptr)
				Throw<Except::Allocate>("Searching for nullptr");
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			auto found = Find(meta, memory);
			if (found)
				return found->mReferences;
			return 0;
		#else
			(void) (meta); (void) (memory);
			return 0;
		#endif
	}
	
	/// Reference some memory, which we do not know if owned or not				
	/// If LANGULUS_FEATURE(MANAGED_MEMORY) is enabled, this function will		
	/// attempt to find memory entry from the memory manager and reference it	
	///	@attention this function does nothing if										
	///              LANGULUS_FEATURE(MANAGED_MEMORY) is disabled					
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@param count - the number of references to add								
	void Allocator::Keep(DMeta meta, const void* memory, Count count) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (memory == nullptr)
				Throw<Except::Allocate>("Searching for nullptr");
			if (count == 0)
				Throw<Except::Allocate>("Zero references added");
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			auto found = Find(meta, memory);
			if (found)
				found->mReferences += count;
		#else
			(void) (meta); (void) (memory); (void) (count);
		#endif
	}

	/// Dereference some memory, which we do not know if owned or not				
	/// If LANGULUS_FEATURE(MANAGED_MEMORY) is enabled, this function will		
	/// attempt to find memory entry from the memory manager and dereference	
	///	@attention this function does nothing if										
	///              LANGULUS_FEATURE(MANAGED_MEMORY) is disabled					
	///	@attention this will deallocate memory if fully dereferenced			
	///				  which is troublesome if you need to call destructors		
	///				  Won't deallocate if LANGULUS_FEATURE(MANAGED_MEMORY) is	
	///				  disabled																	
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@param count - the number of references to add								
	///	@return true if the memory has been fully dereferenced					
	bool Allocator::Free(DMeta meta, const void* memory, Count count) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (memory == nullptr)
				Throw<Except::Allocate>("Searching for nullptr");
			if (count == 0)
				Throw<Except::Allocate>("Zero references removed");
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			auto found = Find(meta, memory);
			if (!found)
				// Data is either static or unallocated - don't touch it		
				return false;

			if (found->mReferences <= count) {
				// Deallocate the entry													
				Deallocate(found);
				return true;
			}

			found->mReferences -= count;
			return false;
		#else
			(void) (meta); (void) (memory); (void) (count);
			return false;
		#endif
	}
	
	#if LANGULUS_FEATURE(MEMORY_STATISTICS)
		Allocator::Statistics Allocator::mStatistics {};
	
		/// Get allocator statistics															
		///	@return a reference to the statistics structure							
		const Allocator::Statistics& Allocator::GetStatistics() noexcept {
			return mStatistics;
		}
	#endif

} // namespace Langulus::Anyness::Inner
