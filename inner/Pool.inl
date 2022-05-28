#pragma once
#include "Pool.hpp"
#include "Allocator.hpp"

namespace Langulus::Anyness::Inner
{

	/// Initialize a pool																		
	///	@attention this constructor relies that instance is placed in the		
	///		beginning of a heap allocation of size Pool::NewAllocationSize()	
	///	@param size - bytes of the usable block to initialize with				
	///	@param memory - handle for use with std::free()								
	inline Pool::Pool(const Size& size, void* memory) SAFETY_NOEXCEPT() {
		// Index the memory																
		SAFETY(if (!IsPowerOfTwo(size))
			Throw<Except::Allocation>("Bad indexed memory initialization"));

		mEntries = mValidEntries = 0;
		mAllocatedByFrontend = 0;
		mAllocatedByBackend = size;
		mBiggestSize = mBigSet = mLastFreed = 0;
		mThreshold = size;
		//mMinAlloc = T::Size() * 2;
		mEntriesMax = size / Alignment;
		mMemory = reinterpret_cast<Allocation*>(
			reinterpret_cast<Byte*>(this) + GetSize());
		mHandle = memory;


		// Do a unit test																	
		/*SAFETY(
			const pcptr origin = pcP2N(Block());
			const pcptr half = mMemory.mAllocatedByBackend / 2;
			const pcptr quarter = mMemory.mAllocatedByBackend / 4;

			if (origin != pcP2N(mMemory.AddressFromIndex(0)))
				throw Except::BadAllocation("Bad");
			if (origin + half != pcP2N(mMemory.AddressFromIndex(1)))
				throw Except::BadAllocation("Bad");
			if (origin + quarter != pcP2N(mMemory.AddressFromIndex(2)))
				throw Except::BadAllocation("Bad");
			if (origin + quarter + half != pcP2N(mMemory.AddressFromIndex(3)))
				throw Except::BadAllocation("Bad");
		)*/

		//PCMEMORY.GetStatistics().mBytesAllocatedByFrontend += AllocatedByFrontend();
	}

	/// Free the whole pool chain																
	///	@attention make sure this is called for the first pool in the chain	
	void Pool::FreePoolChain() {
		/*auto& stats = PCMEMORY.GetStatistics();

		SAFETY(if (stats.mBytesAllocatedByFrontend < AllocatedByFrontend())
			throw Except::BadDestruction("Removing unregistered mBytesAllocatedByFrontend")
		);
		stats.mBytesAllocatedByFrontend -= AllocatedByFrontend();

		SAFETY(if (stats.mBytesAllocatedByBackend < AllocatedByBackend())
			throw Except::BadDestruction("Removing unregistered mBytesAllocatedByBackend")
		);
		stats.mBytesAllocatedByBackend -= AllocatedByBackend();

		SAFETY(if (stats.mEntryCount < mMemory.mValidEntries)
			throw Except::BadDestruction("Removing unregistered mEntryCount")
		);
		stats.mEntryCount -= mMemory.mValidEntries;*/

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
	inline Byte* Pool::GetBlockStart() noexcept {
		const auto entryStart = reinterpret_cast<Byte*>(this);
		return entryStart + Allocation::GetSize();
	}

	/// Get the start of the usable memory for the pool (const)						
	///	@return the start of the memory													
	inline const Byte* Pool::GetBlockStart() const noexcept {
		return const_cast<Pool*>(this)->GetBlockStart();
	}

	/// Get the true allocation size, as bytes requested from OS					
	///	@return bytes allocated for the pool, including alignment/overhead	
	constexpr Size Pool::AllocatedByBackend() const noexcept {
		return mAllocatedByBackend;
	}

	/// Get the allocation size, as bytes requested from client						
	///	@return bytes allocated by the client											
	constexpr Size Pool::AllocatedByFrontend() const noexcept {
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

			const auto e1 = pcP2N(part);
			const auto e2 = pcP2N(entry);
			if (e1 < e2 && e2 - e1 < part->GetTotalSize())
				Throw<Except::MemoryCollision>("Memory collision");
			else if (e2 < e1 && e1 - e2 < entry->GetTotalSize())
				Throw<Except::MemoryCollision>("Memory collision");
			++valids;
		}
	}

	/// Allocate an entry inside the pool - returned pointer is aligned			
	///	@tparam REUSE - enable to attempt reusal of unused entries				
	///	@tparam SAFE - enable to check for collision (mainly for testing)		
	///	@param bytes - number of bytes to allocate									
	///	@return pointer to the new entry inside this pool							
	template<bool REUSE, bool SAFE>
	Allocation* Pool::CreateEntry(const Size bytes) noexcept(!SAFE) {
		const auto bytesWithPadding = Allocation::GetNewAllocationSize();

		// Always allocate a bit more for aligning								
		//auto& stats = PCMEMORY.GetStatistics();
		if constexpr (REUSE) {
			if (mEntries > mValidEntries) {
				// If there are recyclable entries									
				const bool recycleFits = bytesWithPadding <= mThreshold;
				for (auto i = mLastFreed; i < mEntries && recycleFits; ++i) {
					auto entry = AllocationFromIndex(i);
					if (0 != entry->GetUses()) {
						++mLastFreed;
						continue;
					}

					// Recycle the entry													
					if constexpr (SAFE)
						CheckCollision(entry);

					new (entry) Allocation {bytes, this};
					AddBytes(entry->GetTotalSize());
					Recycle(i);
					//stats.mBytesAllocatedByFrontend += entry->GetTotalSize();
					//stats.mEntryCount += 1;
					return entry;
				}
			}
		}

		// Otherwise check if we can add a new entry								
		if (!CanContain(bytesWithPadding))
			return nullptr;

		// Add item																			
		auto entry = AddItem(Entry(bytesWithPadding));
		if constexpr (SAFE)
			CheckCollision(entry);

		//stats.mBytesAllocatedByFrontend += entry->Total();
		//stats.mEntryCount += 1;
		return entry;
	}

	/// Remove an entry																			
	///	@param entry - entry to remove													
	void Pool::RemoveEntry(Allocation* entry) {
		/*auto& stats = PCMEMORY.GetStatistics();
		SAFETY(if (stats.mEntryCount < 1)
			throw Except::BadDestruction("Removing unregistered mEntryCount")
		);
		stats.mEntryCount -= 1;

		SAFETY(if (stats.mBytesAllocatedByFrontend < entry->Total())
			throw Except::BadDestruction("Removing unregistered mBytesAllocatedByFrontend")
		);
		stats.mBytesAllocatedByFrontend -= entry->Total();*/

		--mValidEntries;
		RemoveBytes(entry->GetTotalSize());
		const auto index = IndexFromAddress(entry);
		if (index < mLastFreed)
			mLastFreed = index;
	}

	/// Resize an entry																			
	///	@param entry - entry to resize													
	///	@param bytes - new number of bytes												
	///	@return true if entry was enlarged without conflict						
	bool Pool::ResizeEntry(Allocation* entry, const Size bytes) {
		//auto& stats = PCMEMORY.GetStatistics();
		const auto total = Allocation::GetNewAllocationSize(bytes);
		if (total > mThreshold) {
			// The entry can't be resized in place, it has to move			
			return false;
		}

		RemoveBytes(entry->GetTotalSize());
		/*SAFETY(if (stats.mBytesAllocatedByFrontend < entry->Total())
			throw Except::BadDestruction("Removing unregistered mBytesAllocatedByFrontend")
		);
		stats.mBytesAllocatedByFrontend -= entry->Total();*/

		entry->mAllocatedBytes = bytes;

		AddBytes(entry->GetTotalSize());
		//stats.mBytesAllocatedByFrontend += entry->Total();
		return true;
	}
	
	/// Get the allocation that corresponds to a given index							
	/// Guaranteed to be valid for indices between [0, mEntries)					
	///	@param idx - the entry index														
	///	@return a pointer to the element													
	/*inline Allocation* Pool::GetItem(const Offset idx) noexcept {
		return static_cast<Allocation*>(ValidateAddress(AddressFromIndex(idx)));
	}

	/// Get the entry that corresponds to a given index								
	/// Guaranteed to be valid for indices between [0, mEntries)					
	///	@param idx - the entry index														
	///	@return a pointer to the element													
	inline const Allocation* Pool::GetItem(const Offset idx) const noexcept {
		return const_cast<Pool*>(this)->GetItem(idx);
	}

	/// Get the entry that corresponds to a given pointer.							
	/// Guaranteed to be valid for pointers in range									
	/// [mMemory, mMemory + mMemorySize)													
	///	@param ptr - the pointer to get the element index of						
	///	@return pointer to the element													
	inline T* Pool::GetItem(const void* ptr) noexcept {
		return static_cast<T*>(AddressFromIndex(ValidateIndex(IndexFromAddress(ptr))));
	}

	/// Get the entry that corresponds to a given pointer.							
	/// Guaranteed to be valid for pointers in range									
	/// [mMemory, mMemory + mMemorySize)													
	///	@param ptr - the pointer to get the element index of						
	///	@return pointer to the element													
	inline const T* Pool::GetItem(const void* ptr) const noexcept {
		return const_cast<TIndexedMemory<T>*>(this)->GetItem(ptr);
	}*/

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

	/// Check if memory can contain a new entry of some given bytes				
	///	@param bytes - number of bytes to check										
	///	@return true if bytes can be contained in a new element					
	constexpr bool Pool::CanContain(const Size& bytes) const noexcept {
		const auto total = Allocation::GetNewAllocationSize(bytes);
		return (mEntriesMax > mEntries) && (mAllocatedByFrontend + total <= mAllocatedByBackend) && (mThreshold >= total);
	}

	/// Null the memory																			
	void Pool::Null() {
		memset(mMemory, 0, mAllocatedByBackend);
	}

	/// Get threshold associated with an index											
	///	@param index - the index															
	///	@return the threshold																
	Size Pool::ThresholdFromIndex(const Offset& index) const noexcept {
		auto roof = Roof2(index);
		if (roof == index)
			roof *= 2;
		return index > 0 ? mAllocatedByBackend / roof : mAllocatedByBackend;
	}

	/// Get allocation from index																
	///	@param index - the index															
	///	@return the allocation (not validated and constrained)					
	Allocation* Pool::AllocationFromIndex(const Offset& index) noexcept {
		// Credit goes to Vladislav Penchev (G2)									
		if (index == 0)
			return mMemory;
		if (index >= mEntriesMax)
			return nullptr;

		constexpr Size one {1};
		const Size basePower = pcFastLog2(index);
		const Size baselessIndex = index - (one << basePower);
		const Size levelIndex = (baselessIndex << one) + one;
		const Size memorySize = mAllocatedByBackend >> one;
		const Size lsb = pcGetLSB(memorySize);
		const Size levelSize = (one << (lsb - basePower));
		return const_cast<void*>(pcN2P(pcP2N(mMemory) + levelIndex * levelSize));
	}

	/// Get allocation from index (const)													
	///	@param index - the index															
	///	@return the allocation (not validated and constrained)					
	const Allocation* Pool::AllocationFromIndex(const Offset& index) const noexcept {
		return const_cast<Pool*>(this)->AllocationFromIndex(index);
	}

	/// Validate an address, returning an allocation, which is valid				
	///	@param address - address to validate.											
	///	@returns the address, or nullptr if invalid									
	Allocation* Pool::ValidateAddress(const void* address) noexcept {
		// Check if address is inside pool bounds									
		if (!Contains(address))
			return nullptr;

		// Snap the address to the current threshold								
		const auto offset = 
			reinterpret_cast<const Byte*>(address) - 
			reinterpret_cast<Byte*>(mMemory);

		auto entry = reinterpret_cast<Allocation*>(
			reinterpret_cast<Byte*>(mMemory) + offset - (offset % mThreshold));

		// If address is not in use, step up										
		while (nullptr != entry && 0 == entry->GetUses())
			entry = UpperAllocation(entry);
		return entry;
	}

	/// Validate an address, returning an entry, which is valid						
	///	@param address - address to validate.											
	///	@returns the address, or nullptr if invalid									
	const void* Pool::ValidateAddress(const void* address) const noexcept {
		return const_cast<Pool*>(this)->ValidateAddress(address);
	}

	/// Get index from address																	
	///	@param ptr - the address															
	///	@return the index																		
	Offset Pool::IndexFromAddress(const void* ptr) const noexcept {
		// Credit goes to Yasen Vidolov (G1)										
		const auto i = 
			reinterpret_cast<const Byte*>(ptr) - 
			reinterpret_cast<const Byte*>(mMemory);

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
	Offset Pool::ValidateIndex(Offset index) const noexcept {
		// Pool is empty, so search is pointless									
		if (mValidEntries == 0)
			return InvalidIndex;

		// Step up until a valid entry inside bounds is hit					
		const Allocation* entry;
		while (index != 0 && (index >= mEntries || !(entry = AllocationFromIndex(index)) || 0 == entry->GetUses()))
			index = UpIndex(index);

		// Check if we reached root of pool and it is unused					
		if (index == 0 && 0 == mMemory->GetUses())
			return InvalidIndex;
		return index;
	}

	/// Get index above another index														
	///	@param index																			
	///	@return index above the given one												
	Offset Pool::UpIndex(const Offset index) const noexcept {
		// Credit goes to Vladislav Penchev											
		return index >> (pcGetLSB(index) + 1u);
	}

	/// Get allocation above another allocation											
	///	@param address - the address of the allocation								
	///	@return allocation above the given one, or nullptr if master alloc	
	Allocation* Pool::UpperAllocation(const Allocation* address) noexcept {
		if (address == mMemory)
			return nullptr;
		return AllocationFromIndex(UpIndex(IndexFromAddress(address)));
	}

	/// Get allocation above another allocation (const)								
	///	@param address - the address of the allocation								
	///	@return allocation above the given one, or nullptr if master alloc	
	const Allocation* Pool::UpperAllocation(const Allocation* address) const noexcept {
		return const_cast<Pool*>(this)->UpperAllocation(address);
	}

	/// Set the biggest value and recalculate max entries and threshold			
	///	@param size - the new size															
	void Pool::CheckBiggest(int size) {
		if (size < 0) {
			// We're removing bytes														
			if (!mValidEntries) {
				// There's no valid entry, so reset all values					
				mBigSet = mBiggestSize = mEntries = 0;
				mEntriesMax = mAllocatedByBackend / Alignment;
			}
			else if (pcptr(-size) == mBiggestSize) {
				// Removed bytes match the registered bigset size				
				SAFETY(if (mBigSet == 0)
					Throw<Except::Allocation>(
						"Error while removing bytes - biggest size matches, but no big set specified"));

				--mBigSet;
				if (mBigSet == 0) {
					mBiggestSize = GetBiggest(mBigSet);
					SAFETY(if (mBiggestSize > mAllocatedByBackend)
						Throw<Except::Allocation>(
							"Memory size is bigger than the biggest registered size. This is impossible"));

					// Recalculate threshold and limits								
					const auto r2 = Roof2<true>(mBiggestSize);
					mEntriesMax = mAllocatedByBackend / r2;
				}
			}
		}
		else {
			// We're adding bytes														
			if (pcptr(size) > mBiggestSize) {
				// New biggest size														
				mBiggestSize = pcptr(size);
				mBigSet = 1;

				// Recalculate threshold and limits									
				mEntriesMax = mAllocatedByBackend / Roof2(mBiggestSize);
			}
			else if (pcptr(size) == mBiggestSize)
				++mBigSet;
		}

		// Recalculate threshold														
		mThreshold = mAllocatedByBackend / Roof2(mEntries + 1);
	}

	/// Recycle an item																			
	///	@param start - recycled index to start from									
	void Pool::Recycle(Offset start) noexcept {
		// Add a fictional entry														
		++mValidEntries;

		if (CanRecycle()) {
			++start;

			// Start from the fist valid up until the end						
			Offset valids {};
			for (; start < mEntries && valids < mValidEntries; ++start) {
				auto item = AllocationFromIndex(start);
				if (0 != item->GetUses()) {
					++valids;
					continue;
				}

				mLastFreed = start;
				return;
			}
		}

		// No free entry if this is reached											
		mLastFreed = mEntries;
	}

	/// Get the biggest used entry and the number of such								
	///	@param count - [out] number of matching biggest sizes						
	///	@return the maximum size															
	Size Pool::GetBiggest(Count& count) const noexcept {
		Size result {};
		count = 0;
		for (Offset i = 0, valids = 0; i < mEntries && valids < mValidEntries; ++i) {
			auto item = AllocationFromIndex(i);
			if (0 == item->GetUses())
				continue;

			const auto bytes = item->GetTotalSize();
			if (bytes > result) {
				result = bytes;
				count = 1;
			}
			else if (bytes == result)
				++count;
			++valids;
		}
		return result;
	}

	///  Check if a memory address resigns inside pool's range						
	///	@param address - address to check												
	///	@return true if address belongs to this pool									
	bool Pool::Contains(const void* address) const noexcept {
		const auto a = reinterpret_cast<const Byte*>(address);
		const auto blockStart = GetBlockStart();
		return a >= blockStart && a < blockStart + mAllocatedByFrontend;
	}

	/// Add bytes to the indexed memory														
	///	@param bytes - number of bytes to add											
	void Pool::AddBytes(const Size bytes) SAFETY_NOEXCEPT() {
		SAFETY(if (mAllocatedByFrontend + bytes > mAllocatedByBackend)
			Throw<Except::Allocation>(
				"Error while allocating bytes - allocated bytes are more that the memory size")
		);

		mAllocatedByFrontend += bytes;
		CheckBiggest(int(bytes));
	}

	/// Remove bytes from indexed memory													
	///	@param bytes - number of bytes to remove										
	void Pool::RemoveBytes(const Size bytes) SAFETY_NOEXCEPT() {
		SAFETY(if (mAllocatedByFrontend < bytes)
			Throw<Except::Deallocation>(
				"Error while removing bytes - allocated bytes caused underflow")
		);

		mAllocatedByFrontend -= bytes;
		CheckBiggest(-int(bytes));
	}

	/// Add item to the indexed memory														
	///	@param copythis - copy this item													
	///	@return the item																		
	/*T* Pool::AddItem(const T& copythis) noexcept {
		auto item = AllocationFromIndex(mEntries);
		new (item) T(copythis);
		++mEntries;
		++mValidEntries;
		AddBytes(item->Total());
		return item;
	}*/

} // namespace PCFW::Memory::Inner
