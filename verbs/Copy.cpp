///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "../Any.hpp"

#define VERBOSE(...) //Logger::Verbose(__VA_ARGS__)
#define VERBOSE_TAB(...) //auto tab = Logger::Section(__VA_ARGS__)

namespace Langulus::Anyness
{

	/// Invokes reflected copy-assignments of all elements inside this block	
	///	@attention assumes result has been preallocated and initialized		
	///	@attention assumes this block is not empty									
	///	@attention assumes result is not constant										
	///	@param result - the resulting block (must be preinitialized)			
	///	@param allocate - whether or not to allocate elements in result		
	///							(will be performed only if result is empty)			
	///	@return the number of copied elements											
	Count Block::Copy(Block& result) const {
		// Check if types are compatible												
		if (!mType->Is(result.mType)) {
			Block decayedResult = result.ReinterpretAs(*this);
			if (decayedResult.IsEmpty()) {
				Throw<Except::Copy>(
					"Can't copy elements - incompatible types",
					LANGULUS_LOCATION());
			}

			// Attempt copy inside decayed type										
			return Copy(decayedResult);
		}

		// If counts differ, no copy can occur										
		if (mCount != result.mCount) {
			Throw<Except::Copy>(
				"Can't copy elements - incompatible count",
				LANGULUS_LOCATION());
		}

		// Check if memory is the same after checking size						
		// After all, there is no point in copying over the copy				
		if (mRaw == result.mRaw) {
			VERBOSE("Data is already copied (pointers are the same)", 
				Logger::Cyan, " (optimal)");
			return mCount;
		}

		// Start copying																	
		VERBOSE_TAB("Copying ", mCount, " elements of ", GetToken(), " (", 
			GetStride(), " bytes each) to ", result.GetToken());

		if (IsSparse() && result.IsSparse()) {
			// Won't copy anything but pointers										
			VERBOSE("Sparse -> Sparse referencing copy");

			CopyMemory(mRaw, result.mRaw, mCount * sizeof(void*));

			// Cycle all pointers and reference their allocations				
			auto p = GetRawSparse();
			const auto pEnd = GetRawSparse() + mCount;
			while (p != pEnd) {
				if (p->mEntry)
					p->mEntry->Keep();
				++p;
			}

			VERBOSE("Copied ", mCount, " pointers", Logger::Green, " (fast)");
			return mCount;
		}
		else if (IsSparse() && !result.IsSparse()) {
			// Copy sparse items to a dense container								
			VERBOSE("Sparse -> Dense shallow copy");

			if (result.mType->Is<Block>()) {
				// Blocks don't have keep/free in their reflected copy		
				// operators, so we must compensate for that here				
				for (Count i = 0; i < mCount; ++i) {
					// Resolve left side and call the copy operator				
					const auto from = GetElementResolved(i);
					Block& to = result.Get<Block>(i);

					// Type may not be compatible after resolve					
					if (!from.mType->Is(to.mType)) {
						Throw<Except::Copy>(
							"Unable to copy-assign incompatible types after resolving element",
							LANGULUS_LOCATION());
					}

					// Call copy operator												
					to.Free();
					to = from.Get<Block>(i);
					to.Keep();
				}

				VERBOSE("Copied ", mCount, " blocks", Logger::Red, " (slow)");
			}
			else {
				// Check if a copy operation is available							
				if (!result.mType->mCopier) {
					Throw<Except::Copy>(
						"Unable to copy-assign - no assignment reflected",
						LANGULUS_LOCATION());
				}

				for (Count i = 0; i < mCount; ++i) {
					// Resolve left side and call the copy operator				
					const auto from = GetElementResolved(i);
					auto to = result.GetElement(i);

					// Type may not be compatible after resolve					
					if (!from.mType->Is(to.mType)) {
						Throw<Except::Copy>(
							"Unable to copy-assign incompatible types after resolving element",
							LANGULUS_LOCATION());
					}

					// Call copy operator												
					result.mType->mCopier(from.mRaw, to.mRaw);
				}

				VERBOSE("Copied ", mCount, " elements", Logger::Red, " (slow)");
			}

			return mCount;
		}
		else if (!IsSparse() && result.IsSparse()) {
			// Copy dense items to a sparse container								
			VERBOSE(Logger::Verbose() << "Dense -> Sparse referencing copy");

			// This is dangerous, because original memory might move if		
			// it's not static															
			if (!IsStatic()) {
				Logger::Warning()
					<< "Instantiating dense elements in a sparse container "
					<< "You're seeing this because source memory was not static, "
					<< "and undefined behavior awaits if original memory moves even a bit";
			}

			// Copy pointers																
			for (Count i = 0; i < mCount; ++i) {
				const auto from = GetElement(i);
				auto to = result.GetElement(i).GetRawSparse();
				to->mPointer = from.mRaw;
				to->mEntry = from.mEntry;

				// Reference each copied pointer!									
				if (to->mEntry)
					to->mEntry->Keep();
			}

			VERBOSE("Copied ", mCount, " pointers", Logger::Green, " (fast)");
			return mCount;
		}

		// If this is reached, both source and destination are dense		
		if (result.mType->mIsPOD) {
			// If data is not complex just do a memcpy and we're done		
			CopyMemory(mRaw, result.mRaw, GetByteSize());
			VERBOSE("Copied ", GetByteSize(), " bytes via memcpy", Logger::Green, " (fast copy)");
			return mCount;
		}

		if (result.mType->Is<Block>()) {
			// Blocks don't have keep/free in their reflected copy			
			// operators, so we must compensate for that here					
			for (Count i = 0; i < mCount; ++i) {
				// Resolve left side and call the copy operator					
				const Block& from = Get<Block>(i);
				Block& to = result.Get<Block>(i);

				// Call copy operator													
				to.Free();
				to = from;
				to.Keep();
			}

			VERBOSE("Copied ", mCount, " blocks", Logger::Red, " (slow)");
		}
		else {
			// Check if a copy operation is available								
			if (!mType->mCopier) {
				Throw<Except::Copy>(
					"Unable to copy-assign - no assignment reflected",
					LANGULUS_LOCATION());
			}

			// Iterate each instance in memory										
			for (Count i = 0; i < mCount; ++i) {
				const auto from = GetElement(i);
				auto to = result.GetElement(i);

				// And call the copy operator											
				result.mType->mCopier(from.mRaw, to.mRaw);
			}

			VERBOSE("Copied ", mCount, " elements", Logger::Red, " (slow)");
		}

		return mCount;
	}

} // namespace Langulus::Anyness
