///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Reflection.hpp"

#if LANGULUS_FEATURE(MANAGED_MEMORY)
	LANGULUS_EXCEPTION(MemoryCollision);
#endif

namespace Langulus::Anyness::Inner
{

	constexpr Size Alignment {LANGULUS(ALIGN)};

	#if !LANGULUS_FEATURE(MANAGED_MEMORY)
		using Pool = void;
	#else
		class Pool;
	#endif

	template<class T>
	concept AllocationPrimitive = requires(T a) { 
		{T::GetNewAllocationSize(Size {})} -> CT::Same<Size>;
	};
				
	template<AllocationPrimitive T>
	T* AlignedAllocate(const Size& size);

	
	///																								
	///	Memory allocation																		
	///																								
	/// This is a single allocation record													
	///																								
	struct Allocation final {
	#if LANGULUS_FEATURE(MANAGED_MEMORY)
		friend class Pool;
	#endif
	friend class Allocator;
	protected:
		// Allocated bytes for this chunk											
		Size mAllocatedBytes;
		// The number of references to this memory								
		Count mReferences;
		// The pool that owns the allocation, or handle for std::free()	
		Pool* mPool;

	public:
		Allocation() = delete;
		Allocation(const Allocation&) = delete;
		Allocation(Allocation&&) = delete;
		constexpr Allocation(const Size&, Pool*) noexcept;

		NOD() static constexpr Size GetSize() noexcept;
		NOD() static constexpr Size GetNewAllocationSize(const Size&) noexcept;

		constexpr const Count& GetUses() const noexcept;
		const Byte* GetBlockStart() const noexcept;
		const Byte* GetBlockEnd() const noexcept;
		Byte* GetBlockStart() noexcept;
		constexpr Size GetTotalSize() const noexcept;
		constexpr const Size& GetAllocatedSize() const noexcept;
		bool Contains(const void*) const noexcept;
		bool CollisionFree(const Allocation&) const noexcept;
		template<class T>
		NOD() T* As() const noexcept;
		constexpr void Keep() noexcept;
		constexpr void Keep(const Count&) noexcept;
		constexpr void Free() noexcept;
		constexpr void Free(const Count&) noexcept;
	};

} // namespace Langulus::Anyness::Inner

#include "Allocation.inl"
