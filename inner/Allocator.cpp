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
	inline Byte* AlignedAllocate(const Size& align, const Size& size) {
		const auto padding = align + sizeof(void*);
		auto mem = malloc(size + padding);
		if (!mem)
			throw Except::Allocate(Logger::Error() << "Out of memory");

		auto ptr = (reinterpret_cast<Size>(mem) + padding) & ~(align - 1);
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
	inline Byte* AlignedReallocate(Byte* old, const Size& align, const Size& size) {
		const auto padding = align + sizeof(void*);
		auto mem = realloc(reinterpret_cast<void**>(old)[-1], size + padding);
		if (!mem)
			throw Except::Allocate(Logger::Error() << "Out of memory");

		auto ptr = (reinterpret_cast<Size>(mem) + padding) & ~(align - 1);
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
	/// If LANGULUS_FEATURE(MANAGED_MEMORY) is enabled, this function will		
	/// attempt to find memory entry from the memory manager							
	/// This allows us to safely interface unknown memory, possible reusing it	
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@return the reallocated memory entry											
	Entry* Allocator::Find(DMeta meta, const void* memory) {
		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			return nullptr;
		#else
			return nullptr;
		#endif
	}

	/// Deallocate a memory allocation														
	///	@param meta - the type of data to deallocate (optional)					
	///	@param entry - the memory entry to deallocate								
	void Allocator::Deallocate(DMeta meta, Entry* entry) {
		AlignedFree(reinterpret_cast<Byte*>(entry));
	}
	
	/// Reference some memory, which we do not know if owned or not				
	/// If LANGULUS_FEATURE(MANAGED_MEMORY) is enabled, this function will		
	/// attempt to find memory entry from the memory manager and reference it	
	///	@attention this function does nothing if										
	///              LANGULUS_FEATURE(MANAGED_MEMORY) is disabled. This has		
	///				  dire consequences on sparse containers, since one can not	
	///				  determine if a pointer is owned or not without it!			
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@param count - the number of references to add								
	void Allocator::Keep(DMeta meta, const void* memory, Count count) {
		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			auto found = Find(meta, memory);
			if (found)
				found->mReferences += count;
		#endif
	}

	/// Dereference some memory, which we do not know if owned or not				
	/// If LANGULUS_FEATURE(MANAGED_MEMORY) is enabled, this function will		
	/// attempt to find memory entry from the memory manager and dereference	
	///	@attention this function does nothing if										
	///              LANGULUS_FEATURE(MANAGED_MEMORY) is disabled. This has		
	///				  dire consequences on sparse containers, since one can not	
	///				  determine if a pointer is owned or not without it!			
	///	@attention this will deallocate memory if fully dereferenced			
	///				  which is troublesome if you need constructors being called
	///				  Won't deallocate if LANGULUS_FEATURE(MANAGED_MEMORY) is	
	///				  disabled																	
	///	@param meta - the type of data to search for (optional)					
	///	@param memory - memory pointer													
	///	@param count - the number of references to add								
	///	@return true if the memory has been fully dereferenced					
	bool Allocator::Free(DMeta meta, const void* memory, Count count) {
		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			auto found = Find(meta, memory);
			if (!found)
				// Data is either static or unallocated - don't touch it		
				return false;

			if (found->mReferences <= times) {
				// Deallocate the entry													
				Deallocate(meta, found);
				return true;
			}

			found->mReferences -= count;
			return false;
		#else
			return false;
		#endif
	}

} // namespace Langulus::Anyness
