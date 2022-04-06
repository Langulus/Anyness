#include "../include/PCFW.Memory.hpp"

namespace Langulus::Anyness
{

	/// Copy construction																		
	///	@param copy - the block to copy													
	Trait::Trait(const Any& copy)
		: Any {copy} { }

	/// Move construction																		
	///	@param copy - the block to copy													
	Trait::Trait(Any&& copy) noexcept
		: Any {pcForward<Any>(copy)} { }

	/// Copy construction																		
	///	@param copy - the block to copy													
	Trait::Trait(const Block& copy)
		: Any {copy} { }

	/// Move construction																		
	///	@param copy - the block to copy													
	Trait::Trait(Block&& copy) noexcept
		: Any {pcForward<Block>(copy)} { }

	/// Copy construction																		
	///	@param copy - the trait to copy													
	Trait::Trait(const Trait& copy)
		: Any {static_cast<const Any&>(copy)}
		, mTraitType {copy.mTraitType} { }

	/// Move construction																		
	///	@param copy - the trait to copy													
	Trait::Trait(Trait&& copy) noexcept
		: Any {pcForward<Any>(copy)}
		, mTraitType {pcMove(copy.mTraitType)} { }

	/// Manual construction																		
	///	@param type - the trait id															
	Trait::Trait(TraitID type)
		: Trait {type.GetMeta()} {}

	/// Manual construction																		
	///	@param type - the trait meta														
	Trait::Trait(TMeta type)
		: Any { }
		, mTraitType {type} { }

	/// Manual construction (reference memory block)									
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, const Any& data)
		: Any {data}
		, mTraitType {type} { }

	/// Manual construction (move memory block)											
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, Any&& data) noexcept
		: Any {pcForward<Any>(data)}
		, mTraitType {type} { }

	/// Manual construction (reference memory block)									
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, const Block& data)
		: Any {data}
		, mTraitType {type} { }

	/// Move construction - moves block and references content						
	/// Since we are not aware if that block is referenced, we reference it		
	/// We do not reset the other block to avoid memory leaks						
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, Block&& data) noexcept
		: Any {pcForward<Block>(data)}
		, mTraitType {type} { }

	/// Clone the trait																			
	///	@return the cloned trait															
	Trait Trait::Clone() const {
		return { mTraitType, Any::Clone() };
	}

	/// Get the trait ID																			
	///	@return the trait ID																	
	TraitID Trait::GetTraitID() const noexcept {
		return mTraitType ? mTraitType->GetID() : utInvalid;
	}

	/// Get the trait ID																			
	///	@return the trait ID																	
	pcptr Trait::GetTraitSwitch() const noexcept {
		return GetTraitID().GetHash().GetValue();
	}

	/// Set the trait ID																			
	///	@param type - the trait ID															
	void Trait::SetTraitID(const TraitID& type) noexcept {
		mTraitType = type.GetMeta();
	}

	/// Return the meta trait associated with this trait instance					
	///	@return the trait meta definition												
	TMeta Trait::GetTraitMeta() const noexcept {
		return mTraitType;
	}

	/// Check if trait is valid																
	///	@return true if trait is valid													
	bool Trait::IsTraitValid() const noexcept {
		return mTraitType && GetCount() > 0;
	}

	/// Check if trait is similar to another												
	///	@param other - the trait to test against										
	///	@return true if trait is valid													
	bool Trait::IsSimilar(const Trait& other) const noexcept {
		return (!mTraitType || other.mTraitType == mTraitType) && other.InterpretsAs(mType);
	}

	/// Check if trait is a specific ID														
	///	@param traitId - the id to match													
	///	@return true if ID matches															
	bool Trait::TraitIs(TraitID traitId) const {
		return (!mTraitType && !traitId) || (mTraitType->GetID() == traitId);
	}

	/// Check if trait has the correct data												
	///	@return true if meta trait definition filter is compatible				
	bool Trait::HasCorrectData() const {
		if (!mTraitType)
			return true;

		auto filter = mTraitType->GetDataMeta();
		return InterpretsAs(filter);
	}

	/// Compare traits																			
	///	@param other - the trait to compare with										
	///	@return true if traits are the same												
	bool Trait::operator == (const Trait& other) const noexcept {
		return mTraitType == other.mTraitType && Compare(other);
	}

	/// Compare trait type																		
	///	@param other - the trait ID to compare with									
	///	@return true if same																	
	bool Trait::operator == (const TraitID& other) const noexcept {
		return (!other && !mTraitType) || (mTraitType && mTraitType->GetID() == other);
	}

	/// Reset the trait																			
	void Trait::Reset() {
		Any::Reset();
		mTraitType = nullptr;
	}

	/// Move operator																				
	///	@param other - the trait to move													
	Trait& Trait::operator = (Trait&& other) noexcept {
		Any::operator = (pcForward<Any>(other));
		mTraitType = other.mTraitType;
		other.mTraitType = nullptr;
		return *this;
	}

	/// Shallow copy operator with trait type												
	///	@param other - the trait to copy													
	Trait& Trait::operator = (const Trait& other) {
		Any::operator = (static_cast<const Any&>(other));
		mTraitType = other.mTraitType;
		return *this;
	}

	/// Shallow copy operator for block only, leaving trait unchanged				
	///	@param other - the block to copy													
	Trait& Trait::operator = (const Block& other) {
		Any::operator = (other);
		return *this;
	}

	/// Move copy operator for block only, leaving trait unchanged					
	///	@param other - the block to move													
	Trait& Trait::operator = (Block&& other) noexcept {
		Any::operator = (pcForward<Block>(other));
		return *this;
	}

} // namespace Langulus::Anyness
