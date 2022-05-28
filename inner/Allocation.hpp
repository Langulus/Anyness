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
				

	/// MSVC will likely never support std::aligned_alloc, so we use				
	/// a custom portable routine that's almost the same								
	/// https://stackoverflow.com/questions/62962839									
	///																								
	/// Each allocation has the following prefixed bytes:								
	/// [padding][T::GetSize()][client bytes...]											
	///																								
	///	@param size - the number of client bytes to allocate						
	///	@return a newly allocated memory that is correctly aligned				
	///	@attention if LANGULUS_FEATURE(MANAGED_MEMORY) is enabled, this		
	///		is used to allocate Pool. If not, then it's used to allocate		
	///		Allocation. For internal use only!											
	template<AllocationPrimitive T>
	T* AlignedAllocate(const Size& size) {
		auto base = malloc(T::GetNewAllocationSize(size));
		if (!base)
			Throw<Except::Allocate>("Out of memory");

		// Align pointer to LANGULUS_ALIGN()										
		auto ptr = reinterpret_cast<T*>(
			(reinterpret_cast<Pointer>(base) + Pointer {Alignment})
			& ~Pointer {Alignment - 1}
		);
		
		// Place the entry there														
		new (ptr) T {size, base};
		return ptr;
	}

	
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
		template<bool DEALLOCATE>
		void Free() noexcept;
		template<bool DEALLOCATE>
		void Free(const Count&) SAFETY_NOEXCEPT();
	};

} // namespace Langulus::Anyness::Inner

#include "Allocation.inl"
