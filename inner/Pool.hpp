#pragma once
#include "Allocation.hpp"

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

		template<class T = Allocation>
		NOD() T* GetPoolStart() noexcept;

		template<class T = Allocation>
		NOD() const T* GetPoolStart() const noexcept;

		NOD() static constexpr Size GetMinAllocation() noexcept;
		NOD() constexpr Size GetAllocatedByBackend() const noexcept;
		NOD() constexpr Size GetAllocatedByFrontend() const noexcept;
		NOD() constexpr bool IsInUse() const noexcept;
		NOD() constexpr bool CanRecycle() const noexcept;
		NOD() constexpr bool CanContain(const Size&) const noexcept;

		void CheckCollision(const Allocation*) const;
		void RemoveEntry(Allocation*);
		bool ResizeEntry(Allocation*, Size);
		void FreePoolChain();

		NOD() Allocation* CreateEntry(Size) noexcept;

		void Init(void*, Size);
		void Null();

		NOD() Size ThresholdFromIndex(const Size&) const noexcept;
		NOD() Allocation* AllocationFromIndex(const Offset&) noexcept;
		NOD() const Allocation* AllocationFromIndex(const Offset&) const noexcept;
		NOD() Allocation* ValidateAddress(const void*) noexcept;
		NOD() const void* ValidateAddress(const void*) const noexcept;

		NOD() Offset IndexFromAddress(const void*) const noexcept;
		NOD() Offset ValidateIndex(Offset) const noexcept;
		NOD() Offset UpIndex(Offset) const noexcept;
		NOD() const Allocation* UpperAllocation(const void*) const noexcept;
		NOD() Allocation* UpperAllocation(const void*) noexcept;

		NOD() bool Contains(const void*) const noexcept;

		Allocation* AllocationFromAddress(const void*) noexcept;
		const Allocation* AllocationFromAddress(const void*) const noexcept;

	public:
		// Bytes allocated by the backend											
		Size mAllocatedByBackend {};
		Size mAllocatedByBackendLog2 {};
		// Bytes allocated by the frontend											
		Size mAllocatedByFrontend {};

		// Number of entries																
		Count mEntries {};
		// Number of valid entries														
		Count mValidEntries {};
		// Number of entries on each subdivision									
		Count mEntryDistribution[sizeof(Size) * 8] {};
		// Reminder of the last entry freed											
		Allocation* mLastFreed {};
		// Current threshold, that is, max size of a new entry				
		Size mThreshold {};
		// Max entries possible in this pool										
		Count mEntriesMax {};
		// Pointer to start of indexed memory										
		Byte* mMemory {};
		// Handle for the pool allocation, for use with AlignedFree			
		void* mHandle {};

		// Next pool in the pool chain												
		Pool* mNext {};
	};

} // namespace Langulus::Anyness::Inner

#include "Pool.inl"