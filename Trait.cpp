#include "Trait.hpp"

namespace Langulus::Anyness
{

	/// Copy construction from immutable Any												
	///	@param copy - the block to copy													
	Trait::Trait(const Any& copy)
		: Any {copy} { }

	/// Copy construction from mutable Any													
	///	@param copy - the block to copy													
	Trait::Trait(Any& copy)
		: Any {copy} { }

	/// Move construction from Any															
	///	@param copy - the block to copy													
	Trait::Trait(Any&& copy) noexcept
		: Any {Forward<Any>(copy)} { }

	/// Copy construction from immutable block											
	///	@param copy - the block to copy													
	Trait::Trait(const Block& copy)
		: Any {copy} { }

	/// Copy construction from mutable block												
	///	@param copy - the block to copy													
	Trait::Trait(Block& copy)
		: Any {copy} { }

	/// Move construction from Block															
	///	@attention since we are not aware if that block is referenced, we		
	///				  reference it and we do not reset the other block to avoid	
	///				  memory leaks																
	///	@param copy - the block to copy													
	Trait::Trait(Block&& copy) noexcept
		: Any {Forward<Block>(copy)} { }

	/// Shallow-copy construction from immutable trait									
	///	@param copy - the trait to copy													
	Trait::Trait(const Trait& copy)
		: Any {static_cast<const Any&>(copy)}
		, mTraitType {copy.mTraitType} { }

	/// Shallow-copy construction from mutable trait									
	///	@param copy - the trait to copy													
	Trait::Trait(Trait& copy)
		: Any {static_cast<Any&>(copy)}
		, mTraitType {copy.mTraitType} { }

	/// Move construction																		
	///	@param copy - the trait to copy													
	Trait::Trait(Trait&& copy) noexcept
		: Any {Forward<Any>(copy)}
		, mTraitType {Move(copy.mTraitType)} { }

	/// Manual construction by shallow-copying a constant container				
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, const Any& data)
		: Any {data}
		, mTraitType {type} { }

	/// Manual construction by shallow-copying a mutable container					
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, Any& data)
		: Any {data}
		, mTraitType {type} { }

	/// Manual construction (move memory block)											
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, Any&& data) noexcept
		: Any {Forward<Any>(data)}
		, mTraitType {type} { }

	/// Manual construction by shallow-copying a constant block						
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, const Block& data)
		: Any {data}
		, mTraitType {type} { }

	/// Manual construction by shallow-copying a mutable block						
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, Block& data)
		: Any {data}
		, mTraitType {type} { }

	/// Move construction - moves block and references content						
	///	@attention since we are not aware if that block is referenced, we		
	///				  reference it and we do not reset the other block to avoid	
	///				  memory leaks																
	///	@param type - the trait meta														
	///	@param data - the data for the trait											
	Trait::Trait(TMeta type, Block&& data) noexcept
		: Any {Forward<Block>(data)}
		, mTraitType {type} { }

	/// Clone the trait																			
	///	@return the cloned trait															
	Trait Trait::Clone() const {
		return {mTraitType, Any::Clone()};
	}

	/// Get the trait type																		
	///	@return the trait type																
	TMeta Trait::GetTrait() const noexcept {
		return mTraitType;
	}

	/// Check if trait is valid																
	///	@return true if trait is valid													
	bool Trait::IsTraitValid() const noexcept {
		return mTraitType && !Any::IsEmpty();
	}

	/// Check if trait is similar to another												
	///	@param other - the trait to test against										
	///	@return true if trait is valid													
	bool Trait::IsSimilar(const Trait& other) const noexcept {
		return mTraitType->Is(other.mTraitType) && other.CastsToMeta(mType);
	}

	/// Check if trait is a specific type													
	///	@param traitId - the id to match													
	///	@return true if ID matches															
	bool Trait::TraitIs(TMeta trait) const {
		return mTraitType->Is(trait);
	}

	/// Check if trait has correct data (always true if trait has no filter)	
	///	@return true if trait definition filter is compatible						
	bool Trait::HasCorrectData() const {
		if (!mTraitType)
			return true;
		return CastsToMeta(mTraitType->mDataType);
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
	bool Trait::operator == (TMeta other) const noexcept {
		return TraitIs(other);
	}

	/// Reset the trait																			
	void Trait::Reset() {
		Any::Reset();
	}

	/// Move operator																				
	///	@param other - the trait to move													
	Trait& Trait::operator = (Trait&& other) noexcept {
		Any::operator = (Forward<Any>(other));
		mTraitType = other.mTraitType;
		return *this;
	}

	/// Shallow copy operator with trait type												
	///	@param other - the trait to copy													
	Trait& Trait::operator = (const Trait& other) {
		Any::operator = (static_cast<const Any&>(other));
		mTraitType = other.mTraitType;
		return *this;
	}

	/// Shallow copy operator with trait type												
	///	@param other - the trait to copy													
	Trait& Trait::operator = (Trait& other) {
		Any::operator = (static_cast<Any&>(other));
		mTraitType = other.mTraitType;
		return *this;
	}

	/// Shallow copy operator for block only, leaving trait unchanged (const)	
	///	@param other - the block to copy													
	Trait& Trait::operator = (const Block& other) {
		Any::operator = (other);
		return *this;
	}

	/// Shallow copy operator for block only, leaving trait unchanged				
	///	@param other - the block to copy													
	Trait& Trait::operator = (Block& other) {
		Any::operator = (other);
		return *this;
	}

	/// Move copy operator for block only, leaving trait unchanged					
	///	@attention since we are not aware if that block is referenced, we		
	///				  reference it and we do not reset the other block to avoid	
	///				  memory leaks																
	///	@param other - the block to move													
	Trait& Trait::operator = (Block&& other) noexcept {
		Any::operator = (Forward<Block>(other));
		return *this;
	}

} // namespace Langulus::Anyness
