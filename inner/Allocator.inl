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


	///																								
	///	POOL																						
	///																								

#define POOL_TEMPLATE() template <class T, Count MIN_ALLOCS, Count MAX_ALLOCS>
#define POOL() BulkPoolAllocator<T,MIN_ALLOCS,MAX_ALLOCS>

	/// Does not copy anything, just creates a new allocator							
	POOL_TEMPLATE()
	POOL()::BulkPoolAllocator(const BulkPoolAllocator& other) noexcept
		: mHead(other.mHead)
		, mListForFree(other.mListForFree) {}

	POOL_TEMPLATE()
	POOL()::BulkPoolAllocator(BulkPoolAllocator&& other) noexcept
		: mHead(other.mHead)
		, mListForFree(other.mListForFree) {
		other.mListForFree = nullptr;
		other.mHead = nullptr;
	}

	POOL_TEMPLATE()
	POOL()& POOL()::operator = (BulkPoolAllocator&& other) noexcept {
		reset();
		mHead = other.mHead;
		mListForFree = other.mListForFree;
		other.mListForFree = nullptr;
		other.mHead = nullptr;
		return *this;
	}

	POOL_TEMPLATE()
	POOL()& POOL()::operator = (const BulkPoolAllocator&) noexcept {
		// Does not do anything														
		return *this;
	}

	POOL_TEMPLATE()
	POOL()::~BulkPoolAllocator() noexcept {
		reset();
	}

	/// Deallocates all allocated memory													
	POOL_TEMPLATE()
	void POOL()::reset() noexcept {
		while (mListForFree) {
			T* tmp = *mListForFree;
			std::free(mListForFree);
			mListForFree = reinterpret_cast_no_cast_align_warning<T**>(tmp);
		}
		mHead = nullptr;
	}

	/// Allocates, but does NOT initialize. Use in-place new constructor,		
	/// e.g. T* obj = pool.allocate(); ::new (static_cast<void*>(obj)) T();		
	POOL_TEMPLATE()
	T* POOL()::allocate() {
		auto tmp = mHead ? mHead : AllocateInner();
		mHead = *reinterpret_cast_no_cast_align_warning<T**>(tmp);
		return tmp;
	}

	/// Does not actually deallocate but puts it in store. Make sure you have	
	/// already called the destructor!														
	/// e.g. with obj->~T(); pool.deallocate(obj);										
	POOL_TEMPLATE()
	void POOL()::deallocate(T* obj) noexcept {
		*reinterpret_cast_no_cast_align_warning<T**>(obj) = mHead;
		mHead = obj;
	}

	/// Adds an already allocated block of memory to the allocator. This			
	/// allocator is from now on responsible for freeing the data					
	/// (with free()). If the provided data is not large enough to make use		
	/// of, it is immediately freed. Otherwise it is reused and freed in the	
	/// destructor.																				
	/*POOL_TEMPLATE()
	void POOL()::AddOrFree(Entry* entry) noexcept {
		// Calculate number of available elements in ptr						
		if (numBytes < Alignment + AlignedSize) {
			// Not enough data for at least one element. Free and return.	
			std::free(ptr);
		}
		else 
		AddPool(entry);
	}*/

	/// Swap pool allocators																	
	///	@param other - the pool to swap with											
	POOL_TEMPLATE()
	void POOL()::swap(BulkPoolAllocator& other) noexcept {
		::std::swap(mHead, other.mHead);
		::std::swap(mListForFree, other.mListForFree);
	}

	/// Iterates the list of allocated memory to calculate how many to alloc	
	/// next. Recalculating this each time saves us a size_t member. This		
	/// ignores the fact that memory blocks might have been added manually		
	/// with addOrFree. In practice, this should not matter much.					
	POOL_TEMPLATE()
	size_t POOL()::calcNumElementsToAlloc() const noexcept {
		auto tmp = mListForFree;
		Count numAllocs = MinAllocations;
		while (numAllocs * 2 <= MaxAllocations && tmp) {
			auto x = reinterpret_cast<T***>(tmp);
			tmp = *x;
			numAllocs *= 2;
		}

		return numAllocs;
	}

	/// WARNING: Underflow if numBytes < ALIGNMENT! Guarded in addOrFree()		
	///TODO don't guard it, because Allocator::Allocate never does that! - removed addOrFree as a whole
	POOL_TEMPLATE()
	void POOL()::AddPool(Entry* entry) noexcept {
		const Count numElements = (entry->mAllocatedBytes - Alignment) / AlignedSize;
		const auto ptr = entry->GetBlockStart();
		auto data = reinterpret_cast<T**>(ptr);

		// Link free list																	
		auto x = reinterpret_cast<T***>(data);
		*x = mListForFree;
		mListForFree = data;

		// Create linked list for newly allocated data							
		auto* const headT = reinterpret_cast_no_cast_align_warning<T*>(ptr + Alignment);
		auto* const head = reinterpret_cast<Byte*>(headT);

		// Visual Studio compiler automatically unrolls this loop, which	
		// is pretty cool																	
		for (size_t i = 0; i < numElements; ++i) {
			*reinterpret_cast<Byte**>(head + i * AlignedSize) = head + (i + 1) * AlignedSize;
		}

		// Last one points to 0															
		*reinterpret_cast_no_cast_align_warning<T**>(head + (numElements - 1) * AlignedSize) = mHead;
		mHead = headT;
	}

	/// Called when no memory is available (mHead == 0)								
	/// Don't inline this slow path															
	/// Allocates new memory: [prev |T, T, ... T]										
	POOL_TEMPLATE()
	LANGULUS(NOINLINE) T* POOL()::AllocateInner() {
		const Size bytes = Alignment + AlignedSize * calcNumElementsToAlloc();
		AddPool(Allocator::Allocate(bytes));
		return mHead;
	}

#undef POOL_TEMPLATE
#undef POOL

} // namespace Langulus::Anyness
