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


	/// Allocates bulks of memory for objects of type T. This deallocates the	
	/// memory in the destructor, and keeps a linked list of the allocated		
	/// memory around. Overhead per allocation is the size of a pointer			
	template <class T, Count MinNumAllocs = 4, Count MaxNumAllocs = 256>
	class BulkPoolAllocator {
	public:
		BulkPoolAllocator() noexcept = default;

		// Does not copy anything, just creates a new allocator				
		BulkPoolAllocator(const BulkPoolAllocator&) noexcept
			: mHead(nullptr)
			, mListForFree(nullptr) {}

		BulkPoolAllocator(BulkPoolAllocator&& o) noexcept
			: mHead(o.mHead)
			, mListForFree(o.mListForFree) {
			o.mListForFree = nullptr;
			o.mHead = nullptr;
		}

		BulkPoolAllocator& operator=(BulkPoolAllocator&& o) noexcept {
			reset();
			mHead = o.mHead;
			mListForFree = o.mListForFree;
			o.mListForFree = nullptr;
			o.mHead = nullptr;
			return *this;
		}

		BulkPoolAllocator& operator=(const BulkPoolAllocator&) noexcept {
			// Does not do anything														
			return *this;
		}

		~BulkPoolAllocator() noexcept {
			reset();
		}

		// Deallocates all allocated memory.
		void reset() noexcept {
			while (mListForFree) {
				T* tmp = *mListForFree;
				std::free(mListForFree);
				mListForFree = reinterpret_cast_no_cast_align_warning<T**>(tmp);
			}
			mHead = nullptr;
		}

		// allocates, but does NOT initialize. Use in-place new constructor, e.g.
		//   T* obj = pool.allocate();
		//   ::new (static_cast<void*>(obj)) T();
		T* allocate() {
			T* tmp = mHead;
			if (!tmp)
				tmp = performAllocation();

			mHead = *reinterpret_cast_no_cast_align_warning<T**>(tmp);
			return tmp;
		}

		// does not actually deallocate but puts it in store.
		// make sure you have already called the destructor! e.g. with
		//  obj->~T();
		//  pool.deallocate(obj);
		void deallocate(T* obj) noexcept {
			*reinterpret_cast_no_cast_align_warning<T**>(obj) = mHead;
			mHead = obj;
		}

		// Adds an already allocated block of memory to the allocator. This allocator is from now on
		// responsible for freeing the data (with free()). If the provided data is not large enough to
		// make use of, it is immediately freed. Otherwise it is reused and freed in the destructor.
		void addOrFree(void* ptr, const size_t numBytes) noexcept {
			// calculate number of available elements in ptr
			if (numBytes < ALIGNMENT + ALIGNED_SIZE) {
				// not enough data for at least one element. Free and return.
				std::free(ptr);
			}
			else add(ptr, numBytes);
		}

		void swap(BulkPoolAllocator<T, MinNumAllocs, MaxNumAllocs>& other) noexcept {
			using std::swap;
			swap(mHead, other.mHead);
			swap(mListForFree, other.mListForFree);
		}

	private:
		// iterates the list of allocated memory to calculate how many to alloc next.
		// Recalculating this each time saves us a size_t member.
		// This ignores the fact that memory blocks might have been added manually with addOrFree. In
		// practice, this should not matter much.
		NOD() size_t calcNumElementsToAlloc() const noexcept {
			auto tmp = mListForFree;
			size_t numAllocs = MinNumAllocs;

			while (numAllocs * 2 <= MaxNumAllocs && tmp) {
				auto x = reinterpret_cast<T***>(tmp);
				tmp = *x;
				numAllocs *= 2;
			}

			return numAllocs;
		}

		// WARNING: Underflow if numBytes < ALIGNMENT! This is guarded in addOrFree().
		void add(void* ptr, const size_t numBytes) noexcept {
			const size_t numElements = (numBytes - ALIGNMENT) / ALIGNED_SIZE;
			auto data = reinterpret_cast<T**>(ptr);

			// link free list
			auto x = reinterpret_cast<T***>(data);
			*x = mListForFree;
			mListForFree = data;

			// create linked list for newly allocated data
			auto* const headT = reinterpret_cast_no_cast_align_warning<T*>(reinterpret_cast<char*>(ptr) + ALIGNMENT);
			auto* const head = reinterpret_cast<char*>(headT);

			// Visual Studio compiler automatically unrolls this loop, which is pretty cool
			for (size_t i = 0; i < numElements; ++i) {
				*reinterpret_cast_no_cast_align_warning<char**>(head + i * ALIGNED_SIZE) = head + (i + 1) * ALIGNED_SIZE;
			}

			// last one points to 0
			*reinterpret_cast_no_cast_align_warning<T**>(head + (numElements - 1) * ALIGNED_SIZE) = mHead;
			mHead = headT;
		}

		// Called when no memory is available (mHead == 0).
		// Don't inline this slow path.
		LANGULUS(NOINLINE) T* performAllocation() {
			size_t const numElementsToAlloc = calcNumElementsToAlloc();

			// alloc new memory: [prev |T, T, ... T]
			size_t const bytes = ALIGNMENT + ALIGNED_SIZE * numElementsToAlloc;
			add(assertNotNull<std::bad_alloc>(std::malloc(bytes)), bytes);
			return mHead;
		}

		// enforce byte alignment of the T's
		static constexpr size_t ALIGNMENT = (std::max) (std::alignment_of<T>::value, std::alignment_of<T*>::value);
		static constexpr size_t ALIGNED_SIZE = ((sizeof(T) - 1) / ALIGNMENT + 1) * ALIGNMENT;

		static_assert(MinNumAllocs >= 1, "MinNumAllocs");
		static_assert(MaxNumAllocs >= MinNumAllocs, "MaxNumAllocs");
		static_assert(ALIGNED_SIZE >= sizeof(T*), "ALIGNED_SIZE");
		static_assert(0 == (ALIGNED_SIZE % sizeof(T*)), "ALIGNED_SIZE mod");
		static_assert(ALIGNMENT >= sizeof(T*), "ALIGNMENT");

		T* mHead{ nullptr };
		T** mListForFree{ nullptr };
	};


	template <typename T, size_t MinSize, size_t MaxSize, bool IsFlat>
	struct NodeAllocator;

	/// Dummy allocator that does nothing													
	template <typename T, size_t MinSize, size_t MaxSize>
	struct NodeAllocator<T, MinSize, MaxSize, true> {
		// We are not using the data, so just free it							
		void addOrFree(void* ptr, size_t) noexcept {
			std::free(ptr);
		}
	};

	template <typename T, size_t MinSize, size_t MaxSize>
	struct NodeAllocator<T, MinSize, MaxSize, false> : public BulkPoolAllocator<T, MinSize, MaxSize> {};

	///																								
	template <typename T>
	T unaligned_load(void const* ptr) noexcept {
		// Using memcpy so we don't get into unaligned load problems		
		// Compiler should optimize this very well anyways						
		T t;
		std::memcpy(&t, ptr, sizeof(T));
		return t;
	}

} // namespace Langulus::Anyness

#include "Allocator.inl"
