///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Allocator.hpp"

namespace Langulus::Anyness
{
   
   /// Define a new entry in use                                              
   ///   @param count - the number of allocated elements								
   ///   @param bytes - the number of allocated bytes									
   ///   @param pool - the pool/handle of the entry                           
	constexpr Allocation::Allocation(const Size& bytes, Pool* pool) noexcept
		: mAllocatedBytes {bytes}
      , mReferences {1}
		, mPool {pool} {}

	/// Get the size of the Allocation structure, rounded up for alignment		
	///	@return the byte size of the entry, including alignment					
	constexpr Size Allocation::GetSize() noexcept {
		return sizeof(Allocation) + (sizeof(Allocation) % Size {LANGULUS(ALIGN)});
	}

	/// Check if the memory of the entry is in use										
	///	@return true if entry has any references										
	constexpr const Count& Allocation::GetUses() const noexcept {
		return mReferences;
	}

	/// Return the aligned start of usable block memory (const)						
	///	@return pointer to the entry's memory											
	inline const Byte* Allocation::GetBlockStart() const noexcept {
		const auto entryStart = reinterpret_cast<const Byte*>(this);
		return entryStart + Allocation::GetSize();
	}

	/// Return the end of usable block memory (always const)							
	///	@return pointer to the entry's memory end										
	inline const Byte* Allocation::GetBlockEnd() const noexcept {
		return GetBlockStart() + mAllocatedBytes;
	}

	/// Return the aligned start of usable block memory								
	///	@return pointer to the entry's publicly usable memory						
	inline Byte* Allocation::GetBlockStart() noexcept {
		const auto entryStart = reinterpret_cast<Byte*>(this);
		return entryStart + Allocation::GetSize();
	}

	/// Get the total of the entry, and its allocated data, in bytes				
	///	@return the byte size of the entry plus the usable region after it	
	constexpr Size Allocation::GetTotalSize() const noexcept {
		return Allocation::GetSize() + mAllocatedBytes;
	}

	/// Get the number of allocated bytes in this entry								
	///	@return the byte size of usable memory region								
	constexpr const Size& Allocation::GetAllocatedSize() const noexcept {
		return mAllocatedBytes;
	}

	/// Check if memory address is inside this entry									
	///	@param address - address to check if inside this entry					
	///	@return true if address is inside												
	inline bool Allocation::Contains(const void* address) const noexcept {
		const auto a = reinterpret_cast<const Byte*>(address);
		const auto blockStart = GetBlockStart();
		return a >= blockStart && a < blockStart + mAllocatedBytes;
	}

	/// Test if one entry overlaps another													
	///	@param other - entry to check for collision									
	///	@return true if memories dont intersect										
	inline bool Allocation::CollisionFree(const Allocation& other) const noexcept {
		const auto blockStart1 = GetBlockStart();
		const auto blockStart2 = other.GetBlockStart();
		return 
			(blockStart2 - blockStart1) > ::std::ptrdiff_t(mAllocatedBytes) &&
			(blockStart1 - blockStart2) > ::std::ptrdiff_t(other.mAllocatedBytes);
	}

	/// Get the start of the entry as a given type										
	///	@return a pointer to the first element											
	template<class T>
	NOD() T* Allocation::As() const noexcept {
		return reinterpret_cast<T*>(const_cast<Allocation*>(this)->GetBlockStart());
	}

	/// Reference the entry once																
	constexpr void Allocation::Keep() noexcept {
		++mReferences;
	}

	/// Reference the entry once																
	constexpr void Allocation::Keep(const Count& c) noexcept {
		mReferences += c;
	}

	/// Dereference the entry once, and deallocate it if not in use after that	
	template<bool DEALLOCATE>
	void Allocation::Free() noexcept {
		--mReferences;
		if constexpr (DEALLOCATE) {
			if (0 == mReferences)
				Allocator::Deallocate(this);
		}
	}

	/// Dereference the entry once, and deallocate it if not in use after that	
	template<bool DEALLOCATE>
	void Allocation::Free(const Count& c) SAFETY_NOEXCEPT() {
		SAFETY(if (c > mReferences)
			Throw<Except::Reference>("Invalid dereferencing count"));
		mReferences -= c;
		if constexpr (DEALLOCATE) {
			if (0 == mReferences)
				Allocator::Deallocate(this);
		}
	}

} // namespace Langulus::Anyness
