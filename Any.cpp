#include "../include/PCFW.Memory.hpp"

namespace Langulus::Anyness
{

	/// Copy construction - does a shallow copy and references content			
	///	@param other - the container to shallow-copy									
	Any::Any(const Any& other) 
		: Block {other} {
		Block::Keep();
	}

	/// Copy construction - does a shallow copy and references content			
	///	@param other - the container to shallow-copy									
	Any::Any(const Block& other) 
		: Block {other} {
		Block::Keep();
	}

	/// Move construction - moves block and references content						
	/// Since we are not aware if that block is referenced or not, we				
	/// reference it just in case, and we also do not reset the other block		
	/// to avoid memory leaks - it will be freed if it has been kept				
	///	@param other - the block to move													
	Any::Any(Block&& other) 
		: Block {static_cast<const Block&>(other)} {
		Block::Keep();
	}

	/// Destruction																				
	Any::~Any() {
		Block::Free();
	}

	/// Assign by referencing another Any													
	/// Dereferences old contents, and does a type-constraint before copy		
	///	@param other - the container to shallow copy									
	///	@return a reference to this container											
	Any& Any::operator = (const Any& other) {
		Block::Free();

		if (IsTypeConstrained() && other.mType != mType) {
			// This Any is type-constrained											
			if (!other.mType || mType->InterpretsAs(other.mType)) {
				// Overwrite everything except the type-constraint				
				mRaw = other.mRaw;
				mCount = other.mCount;
				mReserved = other.mReserved;
				mState = other.mState + DState::Typed;
				Block::Keep();
				return *this;
			}
			else {
				throw Except::BadCopy(pcLogFuncError
					<< "Bad memory assignment for type-constrained any: from "
					<< GetToken() << " to " << other.GetToken());
			}
		}

		Block::operator = (other);
		Block::Keep();
		return *this;
	}

	/// Assign by moving another Any															
	///	@param other - the container to move											
	///	@return a reference to this container											
	Any& Any::operator = (Any&& other) {
		SAFETY(const Block otherProbe = other);
		Block::Free();
		SAFETY(if (otherProbe != other || (other.CheckJurisdiction() && !other.CheckUsage()))
			throw Except::BadMove(pcLogFuncError
				<< "You've hit a really nasty corner case, where trying to move a container destroys it, "
				<< "due to a circular referencing. Try to move a shallow-copy, instead of a reference to "
				<< "the original. Data may be incorrect at this point, but the moved container was: " << other.GetToken());
		);

		if (IsTypeConstrained() && other.mType != mType) {
			// This Any is type-constrained											
			if (!other.mType || mType->InterpretsAs(other.mType)) {
				// Overwrite everything except the type-constraint				
				mRaw = other.mRaw;
				mCount = other.mCount;
				mReserved = other.mReserved;
				mState = other.mState + DState::Typed;
				other.ResetInner();
				return *this;
			}
			else {
				throw Except::BadMove(pcLogFuncError
					<< "Bad memory assignment for type-constrained any: from "
					<< GetToken() << " to " << other.GetToken());
			}
		}

		Block::operator = (pcForward<Block>(other));
		return *this;
	}

	/// Assign by referencing a memory block												
	///	@param other - the memory block to reference									
	///	@return a reference to this container											
	Any& Any::operator = (const Block& other) {
		return Any::operator = (Any{ other });
	}

	/// Assign by moving a memory block														
	///	@param other - the memory block to reference									
	///	@return a reference to this container											
	Any& Any::operator = (Block&& other) {
		return Any::operator = (Any{ pcForward<Block>(other) });
	}

	/// Clone anyness																				
	///	@return the cloned container														
	Any Any::Clone() const {
		Any clone;
		Block::Clone(clone);
		return clone;
	}

	/// Destroy all elements, but retain allocated memory if possible				
	void Any::Clear() {
		if (mCount) {
			if (GetBlockReferences() == 1) {
				// Only one use - just destroy elements and reset count,		
				// reusing the allocation for later									
				Block::CallDestructors();
				Block::ClearInner();
			}
			else {
				// We're forced to reset the memory, because it's in use		
				// Keep the type and state, though									
				const auto state = GetUnconstrainedState();
				const auto meta = mType;
				Reset();
				mType = meta;
				mState += state;
			}
		}
	}

	/// Deallocate all elements, but retain type-constraints							
	void Any::Reset() {
		Block::Free();
		Block::ResetInner();
	}

	/// Swap two anies																			
	///	@param other - the container to swap contents with							
	void Any::Swap(Any& other) noexcept {
		other = ::std::exchange(*this, pcMove(other));
	}

	/// Pick a region																				
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the block																		
	Any Any::Crop(pcptr start, pcptr count) const {
		return Any {Block::Crop(start, count)};
	}

	/// Make memory block vacuum (a.k.a. missing)										
	///	@return reference to itself														
	Any& Any::MakeMissing() {
		Block::MakeMissing();
		return *this;
	}

	/// Make memory block static (unmovable and unresizable)							
	///	@return reference to itself														
	Any& Any::MakeStatic() {
		Block::MakeStatic();
		return *this;
	}

	/// Make memory block constant (unresizable and unchangable)					
	///	@return reference to itself														
	Any& Any::MakeConstant() {
		Block::MakeConstant();
		return *this;
	}

	/// Make memory block type-immutable													
	///	@return reference to itself														
	Any& Any::MakeTypeConstrained() {
		Block::MakeTypeConstrained();
		return *this;
	}

	/// Make memory block exlusive (a.k.a. OR container)								
	///	@return reference to itself														
	Any& Any::MakeOr() {
		Block::MakeOr();
		return *this;
	}

	/// Make memory block inclusive (a.k.a. AND container)							
	///	@return reference to itself														
	Any& Any::MakeAnd() {
		Block::MakeAnd();
		return *this;
	}

	/// Make memory block left-polar															
	///	@return reference to itself														
	Any& Any::MakeLeft() {
		Block::MakeLeft();
		return *this;
	}

	/// Make memory block right-polar														
	///	@return reference to itself														
	Any& Any::MakeRight() {
		Block::MakeRight();
		return *this;
	}

} // namespace Langulus::Anyness
