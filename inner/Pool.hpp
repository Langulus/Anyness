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
		Pool(const Size&, void*) noexcept;

		// Default pool allocation is 1 MB											
		static constexpr Size DefaultPoolSize = 1024 * 1024;
		static constexpr Size DefaultMinAllocation = Roof2(Allocation::GetSize() + Alignment);
		static constexpr Offset InvalidIndex = ::std::numeric_limits<Offset>::max();

	public:
		NOD() static constexpr Size GetSize() noexcept;
		NOD() static constexpr Size GetNewAllocationSize(const Size&) noexcept;

		template<class T = Allocation>
		NOD() T* GetPoolStart() noexcept;

		template<class T = Allocation>
		NOD() const T* GetPoolStart() const noexcept;

		NOD() constexpr Size GetMinAllocation() const noexcept;
		NOD() constexpr Count GetMaxEntries() const noexcept;
		NOD() constexpr Size GetAllocatedByBackend() const noexcept;
		NOD() constexpr Size GetAllocatedByFrontend() const noexcept;
		NOD() constexpr bool IsInUse() const noexcept;
		NOD() constexpr bool CanContain(const Size&) const noexcept;

		void RemoveEntry(Allocation*) SAFETY_NOEXCEPT();
		bool ResizeEntry(Allocation*, Size) SAFETY_NOEXCEPT();
		void FreePoolChain();

		NOD() Allocation* CreateEntry(Size) SAFETY_NOEXCEPT();

		void Init(void*, Size);
		void Null();

		NOD() Size ThresholdFromIndex(const Offset&) const noexcept;
		NOD() Allocation* AllocationFromIndex(const Offset&) noexcept;
		NOD() const Allocation* AllocationFromIndex(const Offset&) const noexcept;

		NOD() Offset IndexFromAddress(const void*) const SAFETY_NOEXCEPT();
		NOD() Offset ValidateIndex(Offset) const noexcept;
		NOD() Offset UpIndex(Offset) const noexcept;

		NOD() bool Contains(const void*) const noexcept;

		Allocation* AllocationFromAddress(const void*) SAFETY_NOEXCEPT();
		const Allocation* AllocationFromAddress(const void*) const noexcept;

	public:
		// Bytes allocated by the backend											
		const Size mAllocatedByBackend {};
		const Offset mAllocatedByBackendLog2 {};

		// Bytes allocated by the frontend											
		Size mAllocatedByFrontend {};
		// Number of valid entries														
		Count mEntries {};
		// A chain of freed entries in the range [0-mEntries)					
		Allocation* mLastFreed {};
		// Current threshold, that is, max size of a new entry				
		Size mThreshold {};
		// Smallest allocation possible for the pool								
		Size mThresholdMin{};
		// Pointer to start of usable memory										
		Byte* mMemory {};
		// Handle for the pool allocation, for use with ::std::free			
		void* mHandle {};

		// Next pool in the pool chain												
		Pool* mNext {};
	};

} // namespace Langulus::Anyness::Inner

#include "Pool.inl"