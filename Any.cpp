#include "Any.hpp"

namespace Langulus::Anyness
{

	/// Assign by referencing another Any													
	/// Dereferences old contents, and does a type-constraint before copy		
	///	@param other - the container to shallow copy									
	///	@return a reference to this container											
	Any& Any::operator = (const Any& other) {
		if (IsTypeConstrained() && !InterpretsAs(other.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad shallow-copy-assignment for Any: from "
				<< GetToken() << " to " << other.GetToken());
		}

		other.Keep();
		Free();
		Block::operator = (other);
		return *this;
	}

	/// Assign by moving another Any															
	///	@param other - the container to move											
	///	@return a reference to this container											
	Any& Any::operator = (Any&& other) {
		if (IsTypeConstrained() && !InterpretsAs(other.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad move-copy-assignment for Any: from "
				<< GetToken() << " to " << other.GetToken());
		}

		Free();
		Block::operator = (Forward<Block>(other));
		other.ResetMemory();
		other.ResetState();
		return *this;
	}

	/// Assign by referencing a memory block												
	///	@param other - the memory block to reference									
	///	@return a reference to this container											
	Any& Any::operator = (const Block& other) {
		return Any::operator = (Any {other});
	}

	/// Assign by shallow-copying a memory block											
	///	@attention this is not actually a move, data will be referenced and	
	///				  'other' will not be reset to avoid memory leaks				
	///	@param other - the memory block to shallow-copy								
	///	@return a reference to this container											
	Any& Any::operator = (Block&& other) {
		return Any::operator = (Any {Forward<Block>(other)});
	}

	/// Clone anyness																				
	///	@return the cloned container														
	Any Any::Clone() const {
		Any clone;
		Block::Clone(clone);
		return Abandon(clone);
	}

	/// Destroy all elements, but retain allocated memory if possible				
	void Any::Clear() {
		if (IsEmpty())
			return;

		if (GetReferences() == 1) {
			// Only one use - just destroy elements and reset count,			
			// reusing the allocation for later										
			CallDestructors();
			ClearInner();
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

	/// Reset the container																		
	void Any::Reset() {
		Free();
		ResetMemory();
		ResetState();
	}

	/// Reset container state																	
	void Any::ResetState() {
		if (IsTypeConstrained())
			Block::ResetState<true>();
		else
			Block::ResetState<false>();
	}

	/// Swap two container's contents														
	///	@param other - [in/out] the container to swap contents with				
	void Any::Swap(Any& other) noexcept {
		other = ::std::exchange(*this, Move(other));
	}

	/// Pick a constant region and reference it from another container			
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the container																
	Any Any::Crop(const Offset& start, const Count& count) const {
		return Any {Block::Crop(start, count)};
	}

	/// Pick a region and reference it from another container						
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the container																
	Any Any::Crop(const Offset& start, const Count& count) {
		return Any {Block::Crop(start, count)};
	}

} // namespace Langulus::Anyness
