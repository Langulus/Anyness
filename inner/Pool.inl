#pragma once
#include "Pool.hpp"
#include "Allocator.hpp"

namespace Langulus::Anyness::Inner
{

	/// Fast log2																					
	/// https://stackoverflow.com/questions/11376288									
	///	@param u - number																		
	///	@return the log2																		
	constexpr Size FastLog2(Size x) noexcept {
		if (x < 2)
			return 0;
		return Size {8} * sizeof(Size) - CountLeadingZeroes(x) - Size {1};
	}

	/// Get least significant bit																
	/// https://stackoverflow.com/questions/757059										
	///	@param n - number																		
	///	@return the least significant bit												
	constexpr Size LSB(const Size& n) noexcept {
		#if LANGULUS(BITNESS) == 32
			constexpr Size DeBruijnBitPosition[32] = {
				0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
				31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
			};
			constexpr Size f = 0x077CB531u;
			return DeBruijnBitPosition[(Size {n & (0 - n)} * f) >> Size {27}];
		#elif LANGULUS(BITNESS) == 64
			constexpr Size DeBruijnBitPosition[64] = {
				0,   1,  2, 53,  3,  7, 54, 27,  4, 38, 41,  8, 34, 55, 48, 28,
				62,  5, 39, 46, 44, 42, 22,  9, 24, 35, 59, 56, 49, 18, 29, 11,
				63, 52,  6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
				51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12
			};
			constexpr Size f = 0x022fdd63cc95386dul;
			return DeBruijnBitPosition[(Size {n & (0 - n)} * f) >> Size {58}];
		#else
			#error Implement for your architecture
		#endif
	}


	/// Initialize a pool																		
	///	@attention relies that size is a power-of-two								
	///	@attention this constructor relies that instance is placed in the		
	///		beginning of a heap allocation of size Pool::NewAllocationSize()	
	///	@param size - bytes of the usable block to initialize with				
	///	@param memory - handle for use with std::free()								
	inline Pool::Pool(const Size& size, void* memory) noexcept {
		mEntries = mValidEntries = 0;
		mAllocatedByFrontend = 0;
		mAllocatedByBackend = size;
		mAllocatedByBackendLog2 = FastLog2(size);
		mLastFreed = 0;
		mThreshold = size;
		mEntriesMax = size / GetMinAllocation();
		mMemory = GetPoolStart<Byte>();
		mHandle = memory;
	}

	/// Get the minimum allocation for an entry inside this pool					
	///	@return the size in bytes, always a power-of-two							
	constexpr Size Pool::GetMinAllocation() noexcept {
		return Roof2(Allocation::GetSize() + Alignment);
	}

	/// Free the whole pool chain																
	///	@attention make sure this is called for the first pool in the chain	
	inline void Pool::FreePoolChain() {
		if (mNext)
			mNext->FreePoolChain();

		::std::free(mHandle);
	}

	/// Get the size of the Pool structure, rounded up for alignment				
	///	@return the byte size of the pool, including alignment					
	constexpr Size Pool::GetSize() noexcept {
		return sizeof(Pool) + Alignment - (sizeof(Pool) % Alignment);
	}

	/// Get the size for a new pool allocation, with alignment/additional		
	/// memory requirements																		
	///	@param size - the number of bytes to request									
	///	@return the number of bytes to allocate, to add entry and pool, too	
	constexpr Size Pool::GetNewAllocationSize(const Size& size) noexcept {
		constexpr Size minimum {Pool::DefaultPoolSize + Pool::GetSize()};
		return ::std::max(Roof2(Allocation::GetNewAllocationSize(size)) + Pool::GetSize(), minimum);
	}

	/// Get the start of the usable memory for the pool								
	///	@return the start of the memory													
	template<class T>
	inline T* Pool::GetPoolStart() noexcept {
		const auto poolStart = reinterpret_cast<Byte*>(this);
		return reinterpret_cast<T*>(poolStart + Pool::GetSize());
	}

	/// Get the start of the usable memory for the pool (const)						
	///	@return the start of the memory													
	template<class T>
	inline const T* Pool::GetPoolStart() const noexcept {
		return const_cast<Pool*>(this)->template GetPoolStart<T>();
	}

	/// Get the true allocation size, as bytes requested from OS					
	///	@return bytes allocated for the pool, including alignment/overhead	
	constexpr Size Pool::GetAllocatedByBackend() const noexcept {
		return mAllocatedByBackend;
	}

	/// Get the allocation size, as bytes requested from client						
	///	@return bytes allocated by the client											
	constexpr Size Pool::GetAllocatedByFrontend() const noexcept {
		return mAllocatedByFrontend;
	}

	/// Check if an allocation collides with any of this pool's entries			
	///	@param entry - the allocation to check for collision						
	///	@attention throws if a collision occurs										
	inline void Pool::CheckCollision(const Allocation* entry) const {
		for (Offset i = 0, valids = 0; i < mEntries && valids < mValidEntries; ++i) {
			auto part = AllocationFromIndex(i);
			if (part == entry) {
				++valids;
				continue;
			}
			else if (0 == part->GetUses())
				continue;

			const auto e1 = reinterpret_cast<const Byte*>(part);
			const auto e2 = reinterpret_cast<const Byte*>(entry);
			if (e1 < e2 && e2 - e1 < part->GetTotalSize())
				Throw<Except::MemoryCollision>("Memory collision");
			else if (e2 < e1 && e1 - e2 < entry->GetTotalSize())
				Throw<Except::MemoryCollision>("Memory collision");
			++valids;
		}
	}

	/// Allocate an entry inside the pool - returned pointer is aligned			
	///	@param bytes - number of bytes to allocate									
	///	@return the new allocation, or nullptr if pool is full					
	inline Allocation* Pool::CreateEntry(const Size bytes) noexcept {
		constexpr Offset one {1};

		// Check if we can add a new entry											
		const auto bytesWithPadding = Allocation::GetNewAllocationSize(bytes);
		if (!CanContain(bytesWithPadding))
			return nullptr;

		if (mValidEntries == mEntries) {
			// The entire pool is full, skip search for free spot				
			// Add a new allocation directly											
			auto newEntry = AllocationFromIndex(mEntries);
			new (newEntry) Allocation {bytes, this};

			const Size base = mEntries - (one << FastLog2(mEntries));
			const Size level = (base << one) + one;
			++mEntryDistribution[level];
			++mEntries;
			++mValidEntries;
			mAllocatedByFrontend += bytesWithPadding;
			return newEntry;
		}

		// Check the last freed entry													
		if (mLastFreed && 0 == mLastFreed->GetUses()) {
			// Found a free spot															
			const auto backup = mLastFreed;
			mLastFreed = nullptr;
			new (backup) Allocation {bytes, this};

			const auto index = IndexFromAddress(backup);
			const Size base = index - (one << FastLog2(index));
			const Size level = (base << one) + one;
			++mEntryDistribution[level];
			++mValidEntries;
			mAllocatedByFrontend += bytesWithPadding;
			return backup;
		}

		// Check all subdivision levels for empty spots							
		auto bytesPerBase = mAllocatedByBackend >> one;
		auto bytesPerEntry = bytesPerBase >> one;
		auto inherited = 1;
		auto subdivisions = 2;

		for (Offset level = 1; level < mAllocatedByBackendLog2; ++level) {
			const auto capacity = subdivisions - inherited;
			if (mEntryDistribution[level] == capacity) {
				// Level is full, move on												
				bytesPerBase = bytesPerEntry;
				bytesPerEntry >>= one;
				inherited = subdivisions;
				subdivisions <<= one;
				continue;
			}

			// There are free spots, start searching for them					
			auto baseMemory = mMemory;
			const auto baseMemoryEnd = mMemory + mAllocatedByBackend;
			while (baseMemory < baseMemoryEnd) {
				auto entryMemory = baseMemory + bytesPerEntry;
				const auto entryMemoryEnd = baseMemory + bytesPerBase;
				while (entryMemory < entryMemoryEnd) {
					auto entry = reinterpret_cast<Allocation*>(entryMemory);
					if (0 == entry->GetUses()) {
						// Free entry found												
						new (entry) Allocation {bytes, this};
						++mValidEntries;
						++mEntryDistribution[level];
						mAllocatedByFrontend += bytesWithPadding;
						return entry;
					}
					entryMemory += bytesPerEntry;
				}
				baseMemory += bytesPerBase;
			}

			// Reaching this is an error and shouldn't happen					
		}

		// Reaching this is an error and shouldn't happen						
		return nullptr;
	}

	/// Remove an entry																			
	///	@attention assumes entry is valid												
	///	@param entry - entry to remove													
	inline void Pool::RemoveEntry(Allocation* entry) {
		mAllocatedByFrontend -= entry->GetTotalSize();
		entry->mReferences = 0;
		mLastFreed = entry;

		constexpr Offset one {1};
		const auto index = IndexFromAddress(entry);
		const Size base = index - (one << FastLog2(index));
		const Size level = (base << one) + one;
		--mEntryDistribution[level];
		--mValidEntries;
	}

	/// Resize an entry																			
	///	@param entry - entry to resize													
	///	@param bytes - new number of bytes												
	///	@return true if entry was enlarged without conflict						
	inline bool Pool::ResizeEntry(Allocation* entry, const Size bytes) {
		const auto total = Allocation::GetNewAllocationSize(bytes);
		if (total > mThreshold) {
			// The entry can't be resized in this pool, it must move out	
			return false;
		}

		if (bytes > entry->mAllocatedBytes)
			mAllocatedByFrontend += bytes - entry->mAllocatedBytes;
		else
			mAllocatedByFrontend -= entry->mAllocatedBytes - bytes;

		entry->mAllocatedBytes = bytes;
		return true;
	}

	/// Get the entry that corresponds to a given pointer.							
	/// Guaranteed to be valid for pointers in pool's range							
	///	@param ptr - the pointer to get the element index of						
	///	@return pointer to the element													
	inline Allocation* Pool::AllocationFromAddress(const void* ptr) noexcept {
		return AllocationFromIndex(ValidateIndex(IndexFromAddress(ptr)));
	}

	/// Get the entry that corresponds to a given pointer.							
	/// Guaranteed to be valid for pointers in range									
	/// [mMemory, mMemory + mMemorySize)													
	///	@param ptr - the pointer to get the element index of						
	///	@return pointer to the element													
	inline const Allocation* Pool::AllocationFromAddress(const void* ptr) const noexcept {
		return const_cast<Pool*>(this)->AllocationFromAddress(ptr);
	}

	/// Check if there is any used memory													
	///	@return true on at least one valid entry										
	constexpr bool Pool::IsInUse() const noexcept {
		return mValidEntries > 0;
	}

	/// Check if there is the possibility for reusing an old entry					
	///	@return true if valid entries are less than the registered				
	constexpr bool Pool::CanRecycle() const noexcept {
		return mValidEntries < mEntries;
	}

	/// Check if memory can contain a number of bytes									
	///	@param bytes - number of bytes to check										
	///	@return true if bytes can be contained in a new element					
	constexpr bool Pool::CanContain(const Size& bytes) const noexcept {
		return mEntriesMax > mValidEntries && mThreshold >= bytes;
	}

	/// Null the memory																			
	inline void Pool::Null() {
		memset(mMemory, 0, mAllocatedByBackend);
	}

	/// Get threshold associated with an index											
	///	@param index - the index															
	///	@return the threshold																
	inline Size Pool::ThresholdFromIndex(const Offset& index) const noexcept {
		auto roof = Roof2(index);
		if (roof == index)
			roof *= 2;
		return index > 0 ? mAllocatedByBackend / roof : mAllocatedByBackend;
	}

	/// Get allocation from index																
	///	@param index - the index															
	///	@return the allocation (not validated and constrained)					
	inline Allocation* Pool::AllocationFromIndex(const Offset& index) noexcept {
		// Credit goes to Vladislav Penchev (G2)									
		if (index == 0)
			return reinterpret_cast<Allocation*>(mMemory);
		if (index >= mEntriesMax)
			return nullptr;

		constexpr Size one {1};
		const Size basePower = FastLog2(index);
		const Size baselessIndex = index - (one << basePower);
		const Size levelIndex = (baselessIndex << one) + one;
		const Size memorySize = mAllocatedByBackend >> one;
		const Size lsb = LSB(memorySize);
		const Size levelSize = (one << (lsb - basePower));
		return reinterpret_cast<Allocation*>(mMemory + levelIndex * levelSize);
	}

	/// Get allocation from index (const)													
	///	@param index - the index															
	///	@return the allocation (not validated and constrained)					
	inline const Allocation* Pool::AllocationFromIndex(const Offset& index) const noexcept {
		return const_cast<Pool*>(this)->AllocationFromIndex(index);
	}

	/// Validate an address, returning an allocation, which is valid				
	///	@param address - address to validate.											
	///	@returns the address, or nullptr if invalid									
	inline Allocation* Pool::ValidateAddress(const void* address) noexcept {
		// Check if address is inside pool bounds									
		if (!Contains(address))
			return nullptr;

		// Snap the address to the current threshold								
		const auto offset = static_cast<const Byte*>(address) - mMemory;
		auto entry = reinterpret_cast<Allocation*>(
			mMemory + (offset & ~(mThreshold - 1)));

		// If address is not in use, step up										
		while (entry && 0 == entry->GetUses())
			entry = UpperAllocation(entry);
		return entry;
	}

	/// Validate an address, returning an entry, which is valid						
	///	@param address - address to validate.											
	///	@returns the address, or nullptr if invalid									
	inline const void* Pool::ValidateAddress(const void* address) const noexcept {
		return const_cast<Pool*>(this)->ValidateAddress(address);
	}

	/// Get index from address																	
	///	@param ptr - the address															
	///	@return the index																		
	inline Offset Pool::IndexFromAddress(const void* ptr) const noexcept {
		// Credit goes to Yasen Vidolov (G1)										
		const auto i = static_cast<const Byte*>(ptr) - mMemory;
		if (i == 0)
			return 0;

		// We got the index, but it is not constrained to the pool			
		constexpr Size one {1};
		Size index = ((mAllocatedByBackend + i) / (i & ~(i - one)) - one) >> one;
		while (index >= mEntriesMax)
			index = UpIndex(index);
		return index;
	}

	/// Validate an index																		
	///	@param index - index to validate													
	///	@returns the address, or InvalidIndex if invalid							
	inline Offset Pool::ValidateIndex(Offset index) const noexcept {
		// Pool is empty, so search is pointless									
		if (mValidEntries == 0)
			return InvalidIndex;

		// Step up until a valid entry inside bounds is hit					
		const Allocation* entry;
		while (index != 0 && (index >= mEntries || !(entry = AllocationFromIndex(index)) || 0 == entry->GetUses()))
			index = UpIndex(index);

		// Check if we reached root of pool and it is unused					
		if (index == 0 && 0 == reinterpret_cast<const Allocation*>(mMemory)->GetUses())
			return InvalidIndex;
		return index;
	}

	/// Get index above another index														
	///	@param index																			
	///	@return index above the given one												
	inline Offset Pool::UpIndex(const Offset index) const noexcept {
		// Credit goes to Vladislav Penchev											
		return index >> (LSB(index) + 1u);
	}

	/// Get allocation above another allocation											
	///	@param address - the address of the allocation								
	///	@return allocation above the given one, or nullptr if master alloc	
	inline Allocation* Pool::UpperAllocation(const void* address) noexcept {
		if (address == mMemory)
			return nullptr;
		return AllocationFromIndex(UpIndex(IndexFromAddress(address)));
	}

	/// Get allocation above another allocation (const)								
	///	@param address - the address of the allocation								
	///	@return allocation above the given one, or nullptr if master alloc	
	inline const Allocation* Pool::UpperAllocation(const void* address) const noexcept {
		return const_cast<Pool*>(this)->UpperAllocation(address);
	}

	///  Check if a memory address resigns inside pool's range						
	///	@param address - address to check												
	///	@return true if address belongs to this pool									
	inline bool Pool::Contains(const void* address) const noexcept {
		return address >= mMemory && address < mMemory + mAllocatedByBackend;
	}

} // namespace Langulus::Anyness::Inner
