#pragma once
#include "Integration.hpp"
#include "Reflection.hpp"

namespace Langulus::Anyness
{

	/// Round to the upper power-of-two														
	///	@param x - the unsigned integer to round up									
	///	@return the closest upper round-of-two to x									
	template<Unsigned T>
	NOD() constexpr T Roof2(const T& x) noexcept {
		T n = x;
		--n;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		if constexpr (sizeof(T) > 1)
			n |= n >> 8;
		if constexpr (sizeof(T) > 2)
			n |= n >> 16;
		if constexpr (sizeof(T) > 4)
			n |= n >> 32;
		if constexpr (sizeof(T) > 8)
			TODO();
		++n;
		return n;
	}

	///																								
	///	Memory entry																			
	///																								
	/// This is a single allocation record inside a memory pool						
	///																								
	class Entry {
	public:
		// Allocated bytes for this chunk											
		Stride mAllocatedBytes {};
		// The number of references to this memory								
		Count mReferences {};

	public:
		constexpr Entry() noexcept = default;

		/// Define a new entry in use															
		/// @param bytes - the number of allocated bytes								
		constexpr Entry(const Stride& bytes) noexcept
			: mAllocatedBytes {bytes}
			, mReferences {1} {}

		/// Entries are accessed even after their destruction							
		~Entry() noexcept {
			mReferences = 0;
		}

	public:
		//static constexpr Stride Invalid = std::numeric_limits<Stride>::max();

		/// Get the size of the Entry structure, rounded up for alignment			
		static constexpr Stride GetSize() noexcept {
			return Roof2(::std::max(sizeof(Entry), LANGULUS_ALIGN()));
		}

		/// Check if the memory of the entry is in use									
		inline bool IsInUse() const noexcept {
			return mReferences > 0;
		}

		/// Return the aligned start of usable block memory (const)					
		///	@return pointer to the entry's memory										
		inline const Byte* GetBlockStart() const {
			const auto entryStart = reinterpret_cast<const Byte*>(this);
			return entryStart + Entry::GetSize();
		}

		/// Return the aligned start of usable block memory							
		///	@return pointer to the entry's memory										
		inline Byte* GetBlockStart() {
			const auto entryStart = reinterpret_cast<Byte*>(this);
			return entryStart + Entry::GetSize();
		}

		/// Get the total of the entry, and its allocated data, in bytes			
		constexpr Stride GetTotalSize() const noexcept {
			return Entry::GetSize() + mAllocatedBytes;
		}

		/// Get the number of allocated bytes in this entry							
		constexpr const Stride& Allocated() const noexcept {
			return mAllocatedBytes;
		}

		/// Check if memory address is inside this entry								
		///	@param memory - address to check if inside this entry					
		///	@return true if address is inside											
		inline bool Contains(const Byte* address) const noexcept {
			const auto blockStart = GetBlockStart();
			return address >= blockStart && address < blockStart + mAllocatedBytes;
		}

		/// Test if one entry overlaps another												
		///	@param start - memory to check for collision								
		///	@param size - size of the memory	in bytes									
		///	@return true if memories dont intersect									
		inline bool CollisionFree(const Entry& other) const {
			const auto blockStart1 = GetBlockStart();
			const auto blockStart2 = other.GetBlockStart();
			return 
				(blockStart2 - blockStart1) > mAllocatedBytes &&
				(blockStart1 - blockStart2) > other.mAllocatedBytes;
		}
	};


	///																								
	///	Memory allocator																		
	///																								
	/// This is a single allocation record inside a memory pool						
	///																								
	class Allocator {
	public:
		static Entry* Allocate(DMeta, Count);
		static Entry* Reallocate(DMeta, Count, Entry*);
		static Entry* Find(DMeta, const void*);
		static void Deallocate(DMeta, Entry*);
		static void Reference(Entry*, Count);
		static bool Dereference(Entry*, Count);
	};

} // namespace Langulus::Anyness