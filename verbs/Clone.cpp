#include "Block.hpp"
#include "Any.hpp"

#define VERBOSE_TAB(a) //ScopedTab tab; pcLogFuncVerbose << a << tab
#define VERBOSE(a) //pcLogFuncVerbose << a

namespace Langulus::Anyness
{

	/// Clone any data using RTTI																
	/// Nested for each memory subregion, including sparse links as long			
	/// as depth allows it. It will allocate and call constructors, as well as	
	/// invoke Clone() routines if available												
	///	@param result - the resulting container										
	Count Block::Clone(Block& result) const {
		// Always clone the state, but make it unconstrained					
		result.SetType<false>(mType);
		result.mState += GetUnconstrainedState();

		if (!IsAllocated()) {
			// Nothing to actually clone, except the state						
			return 1;
		}

		VERBOSE_TAB("Cloning " << mCount
			<< " elements of type " << GetToken() 
			<< " (" << GetStride() << " bytes each)");

		if (IsSparse()) {
			// Data is sparse - no escape from this scope						
			if (!result.IsAllocated()) {
				// Allocate the array of pointers if not allocated yet		
				result.mState -= DataState::Static;
				result.mState -= DataState::Constant;
				result.Allocate(mCount, false, true);
			}

			// Clone data behind each valid pointer								
			for (Offset i = 0; i < mCount; ++i) {
				auto ptrFrom = GetRawSparse()[i];
				auto& ptrTo = result.GetRawSparse()[i];
				if (!ptrFrom) {
					// Pointer points nowhere, so just set to nullptr			
					ptrTo = nullptr;
					continue;
				}

				// Pointer is pointing somewhere, so clone the data			
				const auto from = GetElementResolved(i);
				Block to;
				from.Clone(to);
				ptrTo = to.mRaw;
			}

			VERBOSE("Cloned sparse data " << ccRed << "(slowest)");
			return mCount;
		}

		// If this is reached, then data is dense									
		// Iterate each instance in memory											
		if (!mType->mResolver) {
			// Type is not resolvable, so preallocate safely					
			if (mType->mCloneInUninitilizedMemory) {
				// Cloning by placement is available								
				if (result.IsEmpty())
					result.Allocate(mCount, false, true);

				for (Offset index = 0; index < mCount; ++index) {
					const auto from = GetElement(index);
					auto to = result.GetElement(index);
					result.mType->mCloneInUninitilizedMemory(from.mRaw, to.mRaw);
				}

				VERBOSE("Cloned non-resolvable dense by move-placement new " 
					<< ccDarkYellow << "(slow)");
			}
			else if (mType->mCloneInInitializedMemory) {
				// Cloning by copy is available										
				if (result.IsEmpty())
					result.Allocate(mCount, true);

				for (Offset index = 0; index < mCount; ++index) {
					const auto from = GetElement(index);
					auto to = result.GetElement(index);
					result.mType->mCloneInInitializedMemory(from.mRaw, to.mRaw);
				}

				VERBOSE("Cloned non-resolvable dense by shallow copy " 
					<< ccRed << "(slowest)");
			}
			else if (mType->mPOD) {
				// Just memcpy simple POD data										
				if (result.IsEmpty())
					result.Allocate(mCount, false, true);
				
				CopyMemory(mRaw, result.mRaw, GetSize());

				VERBOSE("Cloned non-resolvable dense POD by memcpy " 
					<< ccGreen << "(fast)");
			}
			else {
				throw Except::Copy(Logger::Error()
					<< "Trying to clone unclonable complex type: " << GetToken());
			}
		}
		else {
			const bool preallocatedResult = !result.IsEmpty();
			for (Offset index = 0; index < mCount; ++index) {
				// Type is resolvable, so allocate and clone each resolved	
				const auto from = GetElementResolved(index);
				auto to = Any::From(from.GetType());

				// Check if a clone operation is available for element		
				if (mType->mCloneInUninitilizedMemory) {
					// Placement-clone													
					to.Allocate(1, false, true);
					from.mType->mCloneInUninitilizedMemory(from.mRaw, to.mRaw);
					VERBOSE("Cloned resolved dense by move-placement new" 
						<< ccRed << "(slowest)");
				}
				else if (mType->mCloneInInitializedMemory) {
					// Clone and copy inside initialized elements				
					to.Allocate(1, true);
					from.mType->mCloneInInitializedMemory(from.mRaw, to.mRaw);
					VERBOSE("Cloned resolved dense by shallow copy " 
						<< ccRed << "(slowest)");
				}
				else if (from.mType->mPOD) {
					// Just memcpy simple POD data									
					to.Allocate(1, false, true);
					CopyMemory(from.mRaw, to.mRaw, from.GetSize());
					VERBOSE("Cloned resolved dense POD by memcpy " 
						<< ccDarkYellow << "(slow)");
				}
				else {
					throw Except::Copy(Logger::Error()
						<< "Trying to clone unclonable complex type (resolved): " << from.GetToken());
				}

				// Commit the cloned, by shallowly copying it to result		
				if (preallocatedResult) {
					auto element = result.GetElementResolved(index);
					to.Copy(element); //TODO move instead?
				}
				else result.InsertBlock(Move(to));
			}
		}

		return mCount;
	}

} // namespace Langulus::Anyness
