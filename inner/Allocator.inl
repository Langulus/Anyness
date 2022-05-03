#pragma once
#include "Allocator.hpp"

namespace Langulus::Anyness
{
   
   /// Define a new entry in use                                              
   ///   @param allocatedBytes - the number of allocated bytes                
   ///   @param owner - the owner pool of the entry                           
   constexpr Entry::Entry(const Size& allocatedBytes, Pool* owner) noexcept
      : mAllocatedBytes {allocatedBytes}
      , mReferences {1}
      , mOwner {owner} {}

	/// Entry memory is accessed even after entry destruction						
	/// Make sure the memory is marked as unused											
	inline Entry::~Entry() noexcept {
		mReferences = 0;
	}
	
	/// Get the size of the Entry structure, rounded up for alignment				
	///	@return the byte size of the entry, including alignment					
	constexpr Size Entry::GetSize() noexcept {
		return sizeof(Entry) + (sizeof(Entry) % LANGULUS_ALIGN());
	}

	/// Check if the memory of the entry is in use										
	///	@return true if entry has any references										
	constexpr bool Entry::IsInUse() const noexcept {
		return mReferences > 0;
	}

	/// Return the aligned start of usable block memory (const)						
	///	@return pointer to the entry's memory											
	inline const Byte* Entry::GetBlockStart() const noexcept {
		const auto entryStart = reinterpret_cast<const Byte*>(this);
		return entryStart + Entry::GetSize();
	}

	/// Return the aligned start of usable block memory								
	///	@return pointer to the entry's publicly usable memory						
	inline Byte* Entry::GetBlockStart() noexcept {
		const auto entryStart = reinterpret_cast<Byte*>(this);
		return entryStart + Entry::GetSize();
	}

	/// Get the total of the entry, and its allocated data, in bytes				
	///	@return the byte size of the entry plus the usable region after it	
	constexpr Size Entry::GetTotalSize() const noexcept {
		return Entry::GetSize() + mAllocatedBytes;
	}

	/// Get the number of allocated bytes in this entry								
	///	@return the byte size of usable memory region								
	constexpr const Size& Entry::Allocated() const noexcept {
		return mAllocatedBytes;
	}

	/// Check if memory address is inside this entry									
	///	@param address - address to check if inside this entry					
	///	@return true if address is inside												
	inline bool Entry::Contains(const Byte* address) const noexcept {
		const auto blockStart = GetBlockStart();
		return address >= blockStart && address < blockStart + mAllocatedBytes;
	}

	/// Test if one entry overlaps another													
	///	@param other - entry to check for collision									
	///	@return true if memories dont intersect										
	inline bool Entry::CollisionFree(const Entry& other) const noexcept {
		const auto blockStart1 = GetBlockStart();
		const auto blockStart2 = other.GetBlockStart();
		return 
			(blockStart2 - blockStart1) > ::std::ptrdiff_t(mAllocatedBytes) &&
			(blockStart1 - blockStart2) > ::std::ptrdiff_t(other.mAllocatedBytes);
	}
   
} // namespace Langulus::Anyness
