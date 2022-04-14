#pragma once
#include "Integration.hpp"
#include "Reflection.hpp"

namespace Langulus::Anyness
{

	class Pool;
	
	template<Unsigned T>
	NOD() constexpr T Roof2(const T& x) noexcept;

	///																								
	///	Memory entry																			
	///																								
	/// This is a single allocation record inside a memory pool						
	///																								
	class Entry {
	public:
		// Allocated bytes for this chunk											
		Stride mAllocatedBytes;
		// The number of references to this memory								
		Count mReferences;
		// The pool that owns the memory entry										
		Pool* mOwner;

	public:
		constexpr Entry() = delete;
		constexpr Entry(const Stride&, Pool*) noexcept;
		~Entry() noexcept;

	public:
		static constexpr Stride GetSize() noexcept;
		constexpr bool IsInUse() const noexcept;
		const Byte* GetBlockStart() const noexcept;
		Byte* GetBlockStart() noexcept;
		constexpr Stride GetTotalSize() const noexcept;
		constexpr const Stride& Allocated() const noexcept;
		bool Contains(const Byte* address) const noexcept;
		bool CollisionFree(const Entry& other) const noexcept;
		
		void Deallocate();
	};


	///																								
	///	Memory allocator																		
	///																								
	/// This is a single allocation record inside a memory pool						
	///																								
	class Allocator {
	public:
		NOD() static Entry* Allocate(DMeta, Count);
		NOD() static Entry* Reallocate(DMeta, Count, Entry*);
		NOD() static Entry* Find(DMeta, const void*);
		static void Deallocate(DMeta, Entry*);
		static void Reference(Entry*, Count);
		static void Reference(DMeta, const void*, Count);
		NOD() static bool Dereference(Entry*, Count);
		NOD() static bool Dereference(DMeta, const void*, Count);
	};

} // namespace Langulus::Anyness

#include "Allocator.inl"
