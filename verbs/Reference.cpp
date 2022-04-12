#include "inner/Block.hpp"
#include "../Any.hpp"

namespace Langulus::Anyness::Inner
{

	/// Reference memory block (internal use)												
	/// This is called upon shallow container copies and destructions				
	/// Upon destruction, element destructors are called only if block is		
	/// fully dereferenced. It is your responsibility to clear your container	
	///	@param times - number of references to add or subtract					
	///	@return the remaining references for the memory block						
	RefCount Block::ReferenceBlock(RefCount times) {
		if (!IsAllocated())
			return 1;

		Inner::Pool* pool = nullptr;
		Inner::Entry* entry = nullptr;
		if (!PCMEMORY.Find(mType, mRaw, &pool, &entry)) {
			// Never touch memory without jurisdiction							
			return 1;
		}

		// Anticipate if we're fully dereferencing something					
		if (entry->mReferences + times <= 0)
			CallDestructors();

		// Do the actual (de)referencing	for the block							
		// We do it at the end so that we don't fuck the memory prior		
		return pool->Reference(entry, times);
	}
	
} // namespace Langulus::Anyness::Inner
