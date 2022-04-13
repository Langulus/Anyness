#include "Allocator.hpp"
#include "Exceptions.hpp"
#include "Reflection.hpp"

namespace Langulus::Anyness
{

	/// Credit for malloc wrappers goes to:												
	/// http://stackoverflow.com/questions/1919183										
	/// MSVC will likely never support std::aligned_alloc, so we use				
	/// a custom routine that's almost the same											
	/// https://stackoverflow.com/questions/62962839									
	///	@param align - the number of bytes to align to								
	///	@param size - the number of byte to align										
	///	@return a newly allocated memory that is correctly aligned, and		
	///			  you are responsible for deallocating it via AlignedFree 		
	inline Byte* AlignedMalloc(const Stride& align, const Stride& size) {
		const auto padding = align + sizeof(void*);
		auto mem = malloc(size + padding);
		if (!mem)
			throw Except::Allocate();

		auto ptr = (reinterpret_cast<Stride>(mem) + padding) & ~(align - 1);
		reinterpret_cast<void**>(ptr)[-1] = mem;
		return reinterpret_cast<Byte**>(ptr)[0];
	}

	/// Free aligned memory that has been allocated via AlignedMalloc				
	///	@param ptr - the aligned pointer to free										
	///					 must've been prior allocated via AlignedMalloc				
	inline void AlignedFree(Byte* ptr) noexcept {
		free(reinterpret_cast<void**>(ptr)[-1]);
	}

	/// Allocate a memory entry																
	///	@param meta - the type of data to allocate									
	///	@param count - number of instances of the data type to allocate		
	///	@return the allocated memory entry												
	Entry* Allocator::Allocate(DMeta meta, Count count) {

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

	}

	/// Find a memory entry from pointer													
	///	@param meta - the type of data to allocate									
	///	@param memory - memory pointer													
	///	@return the reallocated memory entry											
	Entry* Allocator::Find(DMeta meta, const void* memory) {

	}

	void Allocator::Deallocate(DMeta, Entry*) {

	}

	void Allocator::Reference(Entry*, Count) {

	}

	bool Allocator::Dereference(Entry*, Count) {

	}
	
	/// Deallocate an entry, removing it from its owning pool						
	void Entry::Deallocate() {
		
	}

} // namespace Langulus::Anyness
