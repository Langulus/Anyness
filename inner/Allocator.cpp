#include "Allocator.hpp"
#include "Exceptions.hpp"
#include "Reflection.hpp"

namespace Langulus::Anyness
{

	/// Credit for malloc wrappers goes to:												
	/// http://stackoverflow.com/questions/1919183										
	/// MSVC will likely never support std::aligned_alloc, so we use				
	/// a custom portable routine that's almost the same								
	/// https://stackoverflow.com/questions/62962839									
	///	@param align - the number of bytes to align to								
	///	@param size - the number of bytes to allocate								
	///	@return a newly allocated memory that is correctly aligned				
	///	@attention you are responsible for deallocating via AlignedFree 		
	inline Byte* AlignedAllocate(const Stride& align, const Stride& size) {
		const auto padding = align + sizeof(void*);
		auto mem = malloc(size + padding);
		if (!mem)
			throw Except::Allocate(Logger::Error() << "Out of memory");

		auto ptr = (reinterpret_cast<Stride>(mem) + padding) & ~(align - 1);
		reinterpret_cast<void**>(ptr)[-1] = mem;
		return *reinterpret_cast<Byte**>(ptr);
	}

	/// Free aligned memory that has been allocated via AlignedMalloc				
	///	@param ptr - the aligned pointer to free										
	///					 must've been prior allocated via AlignedMalloc				
	inline void AlignedFree(Byte* ptr) noexcept {
		free(reinterpret_cast<void**>(ptr)[-1]);
	}

	/// Reallocate aligned memory that has been allocated via AlignedMalloc		
	///	@param ptr - the aligned pointer to reallocate								
	///					 must've been prior allocated via AlignedMalloc				
	///	@param align - the number of bytes to align to								
	///	@param size - the number of bytes to allocate								
	///	@return a newly allocated memory that is correctly aligned				
	///	@attention you are responsible for deallocating via AlignedFree 		
	inline Byte* AlignedReallocate(Byte* old, const Stride& align, const Stride& size) {
		const auto padding = align + sizeof(void*);
		auto mem = realloc(reinterpret_cast<void**>(old)[-1], size + padding);
		if (!mem)
			throw Except::Allocate(Logger::Error() << "Out of memory");

		auto ptr = (reinterpret_cast<Stride>(mem) + padding) & ~(align - 1);
		reinterpret_cast<void**>(ptr)[-1] = mem;
		return *reinterpret_cast<Byte**>(ptr);
	}

	/// Allocate a memory entry																
	///	@param meta - the type of data to allocate									
	///	@param count - number of instances of the data type to allocate		
	///	@return the allocated memory entry												
	Entry* Allocator::Allocate(DMeta meta, Count count) {
		return reinterpret_cast<Entry*>(AlignedAllocate(
			LANGULUS_ALIGN(),
			count * meta->mSize
		));
	}

	/// Reallocate a memory entry																
	///	@attention memory entry might move												
	///	@param meta - the type of data to allocate									
	///	@param count - number of instances of the data type to allocate		
	///	@param previous - the previous memory entry									
	///	@return the reallocated memory entry											
	Entry* Allocator::Reallocate(DMeta meta, Count count, Entry* previous) {
		if (!previous)
			return Allocate(meta, count);

		return reinterpret_cast<Entry*>(AlignedReallocate(
			reinterpret_cast<Byte*>(previous), 
			LANGULUS_ALIGN(), 
			count * meta->mSize
		));
	}

	/// Find a memory entry from pointer													
	///	@param meta - the type of data to allocate									
	///	@param memory - memory pointer													
	///	@return the reallocated memory entry											
	Entry* Allocator::Find(DMeta meta, const void* memory) {
		return nullptr;
	}

	/// Deallocate a memory allocation														
	///	@param meta - the type of data to deallocate									
	///	@param previous - the previous memory entry									
	void Allocator::Deallocate(DMeta meta, Entry* entry) {
		entry->Deallocate();
		AlignedFree(reinterpret_cast<Byte*>(entry));
	}
	
	/// Deallocate an entry, removing it from its owning pool						
	void Entry::Deallocate() {
		
	}

} // namespace Langulus::Anyness
