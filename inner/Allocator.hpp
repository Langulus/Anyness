#pragma once
#include "Integration.hpp"
#include "Reflection.hpp"

namespace Langulus::Anyness
{

	class Pool;
	
	///																								
	///	Memory entry																			
	///																								
	/// This is a single allocation record inside a memory pool						
	///																								
	class Entry {
	public:
		// Allocated bytes for this chunk											
		Size mAllocatedBytes;
		// The number of references to this memory								
		Count mReferences;
		// The pool that owns the memory entry										
		Pool* mOwner;

	public:
		constexpr Entry() = delete;
		constexpr Entry(const Size&, Pool*) noexcept;
		~Entry() noexcept;

	public:
		static constexpr Size GetSize() noexcept;
		constexpr bool IsInUse() const noexcept;
		const Byte* GetBlockStart() const noexcept;
		Byte* GetBlockStart() noexcept;
		constexpr Size GetTotalSize() const noexcept;
		constexpr const Size& Allocated() const noexcept;
		bool Contains(const Byte*) const noexcept;
		bool CollisionFree(const Entry&) const noexcept;
	};


	///																								
	///	Memory allocator																		
	///																								
	/// The primary memory management interface											
	///																								
	class Allocator {
	private:
		struct Statistics {
			// The real allocated bytes, provided by malloc in backend		
			Size mBytesAllocatedByBackend {};
			// The bytes allocated by the frontend									
			Size mBytesAllocatedByFrontend {};
			// Number of registered pools												
			Count mPools {};
			// Number of registered entries											
			Count mEntries {};
			// Number of registered meta datas										
			Count mDataDefinitions {};
			// Number of registered meta traits										
			Count mTraitDefinitions {};
			// Number of registered meta verbs										
			Count mVerbDefinitions {};

			bool operator == (const Statistics&) const noexcept = default;
			bool operator != (const Statistics&) const noexcept = default;
		};
		
		static Statistics mStatistics;

	public:
		//																						
		// Standard functionality														
		//																						
		NOD() static Entry* Allocate(Size);
		NOD() static Entry* Reallocate(Size, Entry*);
		static void Deallocate(DMeta, Entry*);

		//																						
		// More functionality, when feature MANAGED_MEMORY is enabled		
		//																						
		NOD() static Entry* Find(DMeta, const void*);
		NOD() static bool CheckAuthority(DMeta, const void*);
		NOD() static Count GetReferences(DMeta, const void*);
		static void Keep(DMeta, const void*, Count);
		NOD() static bool Free(DMeta, const void*, Count);
		NOD() static const Statistics& GetStatistics() noexcept;
	};


	/// This cast gets rid of warnings like "cast from 'uint8_t*'					
	/// {aka 'unsigned char*'} to 'uint64_t*' {aka 'long unsigned int*'}			
	/// increases required alignment of target type". Use with care!				
	template <typename T>
	inline T reinterpret_cast_no_cast_align_warning(void* ptr) noexcept {
		return reinterpret_cast<T>(ptr);
	}

	template <typename T>
	inline T reinterpret_cast_no_cast_align_warning(void const* ptr) noexcept {
		return reinterpret_cast<T>(ptr);
	}

	/// Make sure this is not inlined as it is slow and dramatically enlarges	
	/// code, thus making other inlinings more difficult								
	/// Throws are also generally the slow path											
	template <typename E, typename... Args>
	[[noreturn]] LANGULUS(NOINLINE) void doThrow(Args&&... args) {
		throw E {Forward<Args>(args)...};
	}

	template <typename E, typename T, typename... Args>
	T* assertNotNull(T* t, Args&&... args) {
		if (LANGULUS_UNLIKELY(nullptr == t))
			doThrow<E>(Forward<Args>(args)...);
		return t;
	}

	enum class AllocationMethod {
		Stack, Heap
	};


	/// Allocates bulks of memory for objects of type T. This deallocates the	
	/// memory in the destructor, and keeps a linked list of the allocated		
	/// memory around. Overhead per allocation is the size of a pointer			
	template <class T, Count MIN_ALLOCS = 4, Count MAX_ALLOCS = 256>
	class BulkPoolAllocator {
	private:
		// Enforce byte alignment of the T's										
		static constexpr Count MinAllocations = MIN_ALLOCS;
		static constexpr Count MaxAllocations = MAX_ALLOCS;
		static constexpr Size Alignment = (::std::max) (alignof(T), alignof(T*));
		static constexpr Size AlignedSize = ((sizeof(T) - 1) / Alignment + 1) * Alignment;

		static_assert(MinAllocations >= 1, "MinAllocations can't be zero");
		static_assert(MaxAllocations >= MinAllocations, "MaxAllocations must be greater or equal to MinAllocations");
		static_assert(AlignedSize >= sizeof(T*), "AlignedSize is invalid for some reason");
		static_assert(0 == (AlignedSize % sizeof(T*)), "AlignedSize mod is invalid for some reason");
		static_assert(Alignment >= sizeof(T*), "Alignment is invalid for some reason");

		T* mHead {};
		T** mListForFree {};

	public:
		BulkPoolAllocator() noexcept = default;
		BulkPoolAllocator(const BulkPoolAllocator&) noexcept;
		BulkPoolAllocator(BulkPoolAllocator&&) noexcept;
		~BulkPoolAllocator() noexcept;

		BulkPoolAllocator& operator = (BulkPoolAllocator&&) noexcept;
		BulkPoolAllocator& operator = (const BulkPoolAllocator&) noexcept;

		void reset() noexcept;
		T* allocate();
		void deallocate(T*) noexcept;
		void addOrFree(void*, const size_t) noexcept;
		void swap(BulkPoolAllocator&) noexcept;
		NOD() bool IsAllocated() const noexcept { return mHead != nullptr; }

	private:
		NOD() size_t calcNumElementsToAlloc() const noexcept;
		void add(void*, const size_t) noexcept;
		LANGULUS(NOINLINE) T* performAllocation();
	};


	template <class, Count, Count, AllocationMethod>
	struct NodeAllocator;

	/// Dummy allocator that does nothing and takes no space							
	template <class T, Count MinSize, Count MaxSize>
	struct NodeAllocator<T, MinSize, MaxSize, AllocationMethod::Stack> {
		// We are not using the data, so just free it							
		void addOrFree(void* ptr, size_t) noexcept {
			::std::free(ptr);
		}
	};

	/// Heap allocator																			
	template <class T, Count MinSize, Count MaxSize>
	struct NodeAllocator<T, MinSize, MaxSize, AllocationMethod::Heap>
		: public BulkPoolAllocator<T, MinSize, MaxSize> {};

} // namespace Langulus::Anyness

#include "Allocator.inl"
