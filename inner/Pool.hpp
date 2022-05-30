#pragma once
#include "Allocation.hpp"

namespace Langulus::Anyness
{
	struct Allocation;
}

namespace Langulus::Anyness::Inner
{

	NOD() constexpr Size FastLog2(Size x) noexcept;
	NOD() constexpr Size LSB(const Size& n) noexcept;


	///																								
	///	A MEMORY POOL 																			
	///																								
	class Pool final {
	public:
		Pool() = delete;
		Pool(const Pool&) = delete;
		Pool(Pool&&) = delete;
		Pool(const Size&, void*) SAFETY_NOEXCEPT();

		// Default pool allocation is 1 MB											
		static constexpr Size DefaultPoolSize = 1024 * 1024;
		static constexpr Offset InvalidIndex = ::std::numeric_limits<Offset>::max();

	public:
		NOD() static constexpr Size GetSize() noexcept;
		NOD() static constexpr Size GetNewAllocationSize(const Size&) noexcept;

		NOD() Byte* GetBlockStart() noexcept;
		NOD() const Byte* GetBlockStart() const noexcept;

		NOD() constexpr Size AllocatedByBackend() const noexcept;
		NOD() constexpr Size AllocatedByFrontend() const noexcept;
		NOD() constexpr bool IsInUse() const noexcept;
		NOD() constexpr bool CanRecycle() const noexcept;
		NOD() constexpr bool CanContain(const Size&) const noexcept;

		void CheckCollision(const Allocation*) const;
		void RemoveEntry(Allocation*);
		bool ResizeEntry(Allocation*, Size);
		void FreePoolChain();

		template<bool REUSE = true, bool SAFE = false>
		NOD() Allocation* CreateEntry(Size) noexcept(!SAFE);

		void Init(void*, Size);
		void Recycle(Offset) noexcept;
		void Null();

		NOD() Size ThresholdFromIndex(const Size&) const noexcept;
		NOD() Allocation* AllocationFromIndex(const Offset&) noexcept;
		NOD() const Allocation* AllocationFromIndex(const Offset&) const noexcept;
		NOD() Allocation* ValidateAddress(const void*) noexcept;
		NOD() const void* ValidateAddress(const void*) const noexcept;

		NOD() Offset IndexFromAddress(const void*) const noexcept;
		NOD() Offset ValidateIndex(Offset) const noexcept;
		NOD() Offset UpIndex(Offset) const noexcept;
		NOD() const Allocation* UpperAllocation(const Allocation*) const noexcept;
		NOD() Allocation* UpperAllocation(const Allocation*) noexcept;

		NOD() Size GetBiggest(Count&) const noexcept;
		NOD() bool Contains(const void*) const noexcept;

		void AddBytes(Size) SAFETY_NOEXCEPT();
		void RemoveBytes(Size) SAFETY_NOEXCEPT();

		Allocation* AllocationFromAddress(const void*) noexcept;
		const Allocation* AllocationFromAddress(const void*) const noexcept;

	public:
		// Bytes allocated by the backend											
		Size mAllocatedByBackend = 0;
		// Bytes allocated by the frontend											
		Size mAllocatedByFrontend = 0;

		// Number of entries																
		Count mEntries = 0;
		// Number of valid entries														
		Count mValidEntries = 0;
		// For keeping track of the biggest size									
		Size mBiggestSize = 0;
		// Count of entries with the mBiggestSize									
		Count mBigSet = 0;
		// Reminder of the last entry freed											
		Offset mLastFreed = 0;
		// Current threshold, that is, max size of a new entry				
		Size mThreshold = 0;
		// Max entries possible in this pool										
		Count mEntriesMax = 0;
		// Pointer to start of indexed memory										
		Allocation* mMemory = nullptr;
		// Handle for the pool allocation, for use with AlignedFree			
		void* mHandle = nullptr;

		// Next pool in the pool chain												
		Pool* mNext = nullptr;
	};

} // namespace Langulus::Anyness::Inner

#include "Pool.inl"