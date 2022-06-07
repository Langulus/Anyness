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
	inline Pool::Pool(const Size& size, void* memory) noexcept
		: mAllocatedByBackend {size}
		, mAllocatedByBackendLog2 {FastLog2(size)}
		, mThresholdMin {Pool::GetMinAllocation()}
		, mValidEntries {}
		, mAllocatedByFrontend {}
		, mLastFreed {}
		, mThreshold {size}
		, mHandle {memory} {
		mMemory = GetPoolStart<Byte>();
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

	/// Allocate an entry inside the pool - returned pointer is aligned			
	///	@param bytes - number of bytes to allocate									
	///	@return the new allocation, or nullptr if pool is full					
	inline Allocation* Pool::CreateEntry(const Size bytes) SAFETY_NOEXCEPT() {
		constexpr Offset one {1};

		// Check if we can add a new entry											
		const auto bytesWithPadding = Allocation::GetNewAllocationSize(bytes);
		if (!CanContain(bytesWithPadding))
			return nullptr;

		if (!mLastFreed) {
			// The entire pool is full, skip search for free spot				
			// Add a new allocation directly											
			auto newEntry = AllocationFromIndex(mValidEntries);
			new (newEntry) Allocation {bytes, this};

			++mValidEntries;
			mAllocatedByFrontend += bytesWithPadding;
			mThreshold = ::std::max(
				Roof2(bytesWithPadding), 
				ThresholdFromIndex(mValidEntries)
			);
			return newEntry;
		}

		// Recycle entries																
		const auto newEntry = mLastFreed;
		mLastFreed = mLastFreed->mNextFreeEntry;
		new (newEntry) Allocation {bytes, this};

		mAllocatedByFrontend += bytesWithPadding;
		return newEntry;
	}

	/// Remove an entry																			
	///	@attention assumes entry is valid												
	///	@param entry - entry to remove													
	inline void Pool::RemoveEntry(Allocation* entry) SAFETY_NOEXCEPT() {
		constexpr Offset one {1};

		#if LANGULUS(SAFE)
			if (entry->mReferences == 0)
				Throw<Except::Deallocation>("Removing an invalid entry");
			if (mValidEntries == 0)
				Throw<Except::Deallocation>("Bad valid entry count");
			if (mAllocatedByFrontend < entry->GetTotalSize())
				Throw<Except::Deallocation>("Bad frontend allocation size");
		#endif

		if (1 == mValidEntries) {
			// The freed entry was the last used entry							
			// Reset the entire pool													
			mThreshold = mAllocatedByBackend;
			mValidEntries = 0;
			mAllocatedByFrontend = 0;
			mLastFreed = nullptr;
			return;
		}

		mAllocatedByFrontend -= entry->GetTotalSize();
		entry->mNextFreeEntry = mLastFreed;
		entry->mReferences = 0;
		mLastFreed = entry;
	}

	/// Resize an entry																			
	///	@param entry - entry to resize													
	///	@param bytes - new number of bytes												
	///	@return true if entry was enlarged without conflict						
	inline bool Pool::ResizeEntry(Allocation* entry, const Size bytes) SAFETY_NOEXCEPT() {
		#if LANGULUS(SAFE)
			if (!Contains(entry) || entry->GetUses() == 0 || !bytes)
				Throw<Except::Reallocation>("Invalid reallocation");
		#endif

		if (bytes > entry->mAllocatedBytes) {
			// We're enlarging the entry												
			// Make sure we don't violate threshold								
			const auto addition = bytes - entry->mAllocatedBytes;
			if (entry->GetTotalSize() + addition > mThreshold)
				return false;

			mAllocatedByFrontend += addition;
		}
		else {
			// We're shrinking the entry												
			// No checks required														
			const auto removal = entry->mAllocatedBytes - bytes;
			#if LANGULUS(SAFE)
				if (mAllocatedByFrontend < removal)
					Throw<Except::Reallocation>("Bad frontend allocation size");
			#endif
			mAllocatedByFrontend -= removal;
		}

		entry->mAllocatedBytes = bytes;
		return true;
	}

	/// Get the entry that corresponds to a given pointer.							
	/// Guaranteed to be valid for pointers in pool's range							
	///	@param ptr - the pointer to get the element index of						
	///	@return pointer to the element													
	inline Allocation* Pool::AllocationFromAddress(const void* ptr) SAFETY_NOEXCEPT() {
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

	/// Check if memory can contain a number of bytes									
	///	@attention assumes that bytes include any padding and overhead			
	///	@param bytes - number of bytes to check										
	///	@return true if bytes can be contained in a new/recycled element		
	constexpr bool Pool::CanContain(const Size& bytes) const noexcept {
		return mThreshold >= mThresholdMin && bytes <= mThreshold;
	}

	/// Null the memory																			
	inline void Pool::Null() {
		memset(mMemory, 0, mAllocatedByBackend);
	}

	/// Get threshold associated with an index											
	///	@param index - the index															
	///	@return the threshold																
	inline Size Pool::ThresholdFromIndex(const Offset& index) const noexcept {
		if (0 == index)
			return mAllocatedByBackend;
		constexpr Size one {1};
		const Size basePower = FastLog2(index);
		const Size lsb = LSB(mAllocatedByBackend >> one);
		return one << (lsb - basePower);
	}

	/// Get level associated with an index													
	///	@param index - the index															
	///	@return the level																		
	inline Offset Pool::LevelFromIndex(const Offset& index) const noexcept {
		return FastLog2(index);
	}

	/// Get allocation from index																
	///	@param index - the index															
	///	@return the allocation (not validated and constrained)					
	inline Allocation* Pool::AllocationFromIndex(const Offset& index) noexcept {
		// Credit goes to Vladislav Penchev (G2)									
		if (index == 0)
			return GetPoolStart();
		//if (index >= mEntriesMax)
		//	return nullptr;

		constexpr Size one {1};
		const Size basePower = FastLog2(index);
		const Size baselessIndex = index - (one << basePower);
		const Size levelIndex = (baselessIndex << one) + one;
		const Size lsb = LSB(mAllocatedByBackend >> one);
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
	///	@attention assumes pointer is inside the pool								
	///	@param ptr - the address															
	///	@return the index																		
	inline Offset Pool::IndexFromAddress(const void* ptr) const SAFETY_NOEXCEPT() {
		SAFETY(if (!Contains(ptr))
			Throw<Except::OutOfRange>("Entry is outside pool"));

		// Credit goes to Yasen Vidolov (G1)										
		const Offset i = static_cast<const Byte*>(ptr) - mMemory;
		if (i < mThreshold || 0 == mValidEntries)
			return 0;

		// We got the index, but it is not constrained to the pool			
		constexpr Offset one {1};
		Offset index = ((mAllocatedByBackend + i) / (i & ~(i - one)) - one) >> one;
		while (index >= mValidEntries)
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
		while (index != 0 && (index >= mValidEntries || !(entry = AllocationFromIndex(index)) || 0 == entry->GetUses()))
			index = UpIndex(index);

		// Check if we reached root of pool and it is unused					
		if (index == 0 && 0 == GetPoolStart()->GetUses())
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
	inline Allocation* Pool::UpperAllocation(const void* address) SAFETY_NOEXCEPT() {
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
