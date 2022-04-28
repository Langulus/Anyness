#include "../Any.hpp"
#define VERBOSE(a) //a

namespace Langulus::Anyness
{

	/// Invoke the shallow copy operators of all elements inside this block		
	///	@param result - the resulting block (must be preinitialized)			
	///	@param allocate - whether or not to allocate elements in result		
	///							(will be performed only if result is empty)			
	///	@return the number of copied elements											
	Count Block::Copy(Block& result, bool allocate) const {
		// Check if there's anything to copy at all								
		if (IsEmpty()) {
			throw Except::Copy(VERBOSE(Logger::Error()
				<< "Nothing to copy"));
		}

		// Check if resulting container is allocated and initialized		
		if (!allocate && result.IsEmpty()) {
			throw Except::Copy(VERBOSE(Logger::Error()
				<< "Trying to copy " << GetToken()
				<< " to an uninitialized memory block " << result.GetToken()));
		}

		// Check if types are compatible												
		if (!mType->Is(result.mType)) {
			Block decayedResult = result.ReinterpretAs(*this);
			if (!decayedResult.IsEmpty() && decayedResult.GetCount() <= GetCount()) {
				// Attempt copy inside decayed type									
				return Copy(decayedResult, false);
			}
			else {
				// Data is incompatible for copying									
				throw Except::Copy(VERBOSE(Logger::Error()
					<< "Can't copy " << GetToken()
					<< " to incompatible block of type " << result.GetToken()));
			}
		}

		// This is reached only if types are exactly the same					
		// Check if counts match														
		if (mCount != result.mCount) {
			if (!allocate || !result.IsEmpty()) {
				throw Except::Copy(VERBOSE(Logger::Error()
					<< "Trying to copy " << GetToken()
					<< " differently sized memory block " << result.GetToken()));
			}
			else result.Allocate<true>(mCount);
		}

		// Check if memory is the same after checking size						
		// After all, there is no point in copying over the copy				
		if (mRaw == result.mRaw) {
			VERBOSE(Logger::Verbose()
				<< "Data is already copied (pointers are the same)" 
				<< ccCyan << " (optimal)"
			);
			return mCount;
		}

		// Check if resulting container is constant								
		if (result.IsConstant()) {
			throw Except::Copy(VERBOSE(Logger::Error()
				<< "Trying to copy " << GetToken()
				<< " to constant block " << result.GetToken()));
		}

		// Start copying																	
		VERBOSE(ScopedTab tab; Logger::Verbose()
			<< "Copying " << mCount
			<< " elements of " << GetToken() << " (" << GetStride()
			<< " bytes each) to " << result.GetToken() << tab);

		if (IsSparse() && result.IsSparse()) {
			// Won't copy anything but pointers										
			VERBOSE(Logger::Verbose()
				<< "Sparse -> Sparse referencing copy");

			CopyMemory(mRaw, result.mRaw, mCount * sizeof(void*));

			// Cycle all pointers and reference their memories					
			auto from_ptrarray = GetRawSparse();
			for (Count i = 0; i < mCount; ++i)
				Allocator::Keep(mType, from_ptrarray[i], 1);

			VERBOSE(Logger::Verbose()
				<< "Copied " << mCount << " pointers"
				<< ccGreen << " (fast)");
			return mCount;
		}
		else if (IsSparse() && !result.IsSparse()) {
			// Copy sparse items to a dense container								
			VERBOSE(Logger::Verbose() << "Sparse -> Dense shallow copy");

			if (result.mType->Is<Block>()) {
				// Blocks don't have keep/free in their reflected copy		
				// operators, so we must compensate for that here				
				for (Count i = 0; i < mCount; ++i) {
					// Resolve left side and call the copy operator				
					const auto from = GetElementResolved(i);
					Block& to = result.Get<Block>(i);

					// Type may not be compatible after resolve					
					if (!from.mType->Is(to.mType)) {
						throw Except::Copy(Logger::Error()
							<< "Trying to copy uncompatible types after resolving source: "
							<< from.GetToken() << " -> " << to.GetToken());
					}

					// Call copy operator												
					to.Free();
					to = from.Get<Block>(i);
					to.Keep();
				}

				VERBOSE(Logger::Verbose()
					<< "Copied " << mCount << " blocks"
					<< ccRed << " (slow)");
			}
			else {
				// Check if a copy operation is available							
				if (!result.mType->mCopier) {
					throw Except::Copy(Logger::Error()
						<< "Trying to copy uncopiable " << result.GetToken());
				}

				for (Count i = 0; i < mCount; ++i) {
					// Resolve left side and call the copy operator				
					const auto from = GetElementResolved(i);
					auto to = result.GetElement(i);

					// Type may not be compatible after resolve					
					if (!from.mType->Is(to.mType)) {
						throw Except::Copy(Logger::Error()
							<< "Trying to copy uncompatible types after resolving source: "
							<< from.GetToken() << " -> " << to.GetToken());
					}

					// Call copy operator												
					result.mType->mCopier(from.mRaw, to.mRaw);
				}

				VERBOSE(Logger::Verbose()
					<< "Copied " << mCount << " elements"
					<< ccRed << " (slow)");
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
				auto to = result.GetElement(i);
				to.GetRawSparse()[0] = from.mRaw;

				// Reference each copied pointer!									
				Allocator::Keep(from.mType, from.mRaw, 1);
			}

			VERBOSE(Logger::Verbose()
				<< "Copied " << mCount << " pointers"
				<< ccGreen << " (fast)");
			return mCount;
		}

		// If this is reached, both source and destination are dense		
		if (result.mType->mIsPOD) {
			// If data is not complex just do a memcpy and we're done		
			CopyMemory(mRaw, result.mRaw, GetSize());
			VERBOSE(Logger::Verbose()
				<< "Copied " << GetSize() << " bytes via memcpy" 
				<< ccGreen << " (fast copy)");
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

			VERBOSE(Logger::Verbose()
				<< "Copied " << mCount << " blocks"
				<< ccRed << " (slow)");
		}
		else {
			// Check if a copy operation is available								
			if (!mType->mCopier) {
				throw Except::Copy(VERBOSE(Logger::Error()
					<< "Trying to copy uncopiable " << result.GetToken()));
			}

			// Iterate each instance in memory										
			for (Count i = 0; i < mCount; ++i) {
				const auto from = GetElement(i);
				auto to = result.GetElement(i);

				// And call the copy operator											
				result.mType->mCopier(from.mRaw, to.mRaw);
			}

			VERBOSE(Logger::Verbose()
				<< "Copied " << mCount << " elements" 
				<< ccRed << " (slow)");
		}

		return mCount;
	}

} // namespace Langulus::Anyness
