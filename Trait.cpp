///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Trait.hpp"

namespace Langulus::Anyness
{

	/// Same as copy-construction, but doesn't reference anything					
	///	@param other - the trait to copy													
	Trait::Trait(Disowned<Trait>&& other)
		: Any {other.Forward<Any>()}
		, mTraitType {other.mValue.mTraitType} {}

	/// Same as move-construction but doesn't fully reset other, saving some	
	/// instructions																				
	///	@param other - the trait to move													
	Trait::Trait(Abandoned<Trait>&& other)
		: Any {other.Forward<Any>()}
		, mTraitType {other.mValue.mTraitType} {}

	/// Shallow-copy a disowned trait (doesn't reference anything)					
	///	@param other - the trait to copy													
	///	@return a reference to this trait												
	Trait& Trait::operator = (Disowned<Trait>&& other) {
		Any::operator = (other.Forward<Any>());
		mTraitType = other.mValue.mTraitType;
		return *this;
	}

	/// Move an abandoned trait, minimally resetting the source						
	///	@param other - the container to move and reset								
	///	@return a reference to this trait												
	Trait& Trait::operator = (Abandoned<Trait>&& other) {
		Any::operator = (other.Forward<Any>());
		mTraitType = other.mValue.mTraitType;
		return *this;
	}

	/// Clone the trait																			
	///	@return the cloned trait															
	Trait Trait::Clone() const {
		return Trait::From(mTraitType, Any::Clone());
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
		return mTraitType == trait || (mTraitType && mTraitType->Is(trait));
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
		return TraitIs(other.mTraitType) && Compare(other);
	}

	/// Reset the trait																			
	void Trait::Reset() {
		Any::Reset();
	}

} // namespace Langulus::Anyness

namespace Langulus::Traits
{

	using RTTI::MetaTrait;

	Logger::Logger()
		: Trait {MetaTrait::Of<Logger>(), Any{}} {}
	Logger::Logger(Disowned<Logger>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}
	Logger::Logger(Abandoned<Logger>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}

	Count::Count()
		: Trait {MetaTrait::Of<Count>(), Any{}} {}
	Count::Count(Disowned<Count>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}
	Count::Count(Abandoned<Count>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}

	Name::Name()
		: Trait {MetaTrait::Of<Name>(), Any{}} {}
	Name::Name(Disowned<Name>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}
	Name::Name(Abandoned<Name>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}

	Context::Context()
		: Trait {MetaTrait::Of<Context>(), Any{}} {}
	Context::Context(Disowned<Context>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}
	Context::Context(Abandoned<Context>&& other)
		: Trait {other.mValue.mTraitType, other.Forward<Any>()} {}

} // namespace Langulus::Traits
