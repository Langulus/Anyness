///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Trait.hpp"

namespace Langulus::Anyness
{

	/// Manual trait construction by copy													
	///	@tparam T - type of the contained data											
	///	@param type - type of the trait													
	///	@param data - data to copy inside trait										
	template<class T>
	Trait::Trait(TMeta type, const T& data)
		: Any {data}
		, mTraitType {type} {}

	/// Manual trait construction by copy													
	///	@tparam T - type of the contained data											
	///	@param type - type of the trait													
	///	@param data - data to copy inside trait										
	template<class T>
	Trait::Trait(TMeta type, T& data)
		: Any {data}
		, mTraitType {type} {}

	/// Manual trait construction by movement												
	///	@tparam T - type of the contained data											
	///	@param type - type of the trait													
	///	@param data - data to move inside trait										
	template<class T>
	Trait::Trait(TMeta type, T&& data)
		: Any {Forward<T>(data)}
		, mTraitType {type} {}

	/// Create a trait from a trait definition											
	template<CT::Data T, CT::Data D>
	Trait Trait::From() {
		static_assert(CT::Trait<T>, "T must be a trait definition");
		return Trait {MetaTrait::Of<Decay<T>>(), Block::From<D>()};
	}

	/// Create a trait from a trait definition											
	template<CT::Data T>
	Trait Trait::FromMemory(const Block& memory) {
		static_assert(CT::Trait<T>, "T must be a trait definition");
		return Trait {MetaTrait::Of<Decay<T>>(), memory};
	}

	/// Create a trait from a trait definition											
	template<CT::Data T>
	Trait Trait::FromMemory(Block&& memory) {
		static_assert(CT::Trait<T>, "T must be a trait definition");
		return Trait {MetaTrait::Of<Decay<T>>(), Forward<Block>(memory)};
	}

	/// Create a trait from a trait definition and data								
	template<CT::Data T, CT::Data D>
	Trait Trait::From(const D& stuff) {
		static_assert(CT::Trait<T>, "T must be a trait definition");
		return Trait {MetaTrait::Of<Decay<T>>(), Any {stuff}};
	}

	/// Create a trait from a trait definition by moving data						
	template<CT::Data T, CT::Data D>
	Trait Trait::From(D&& stuff) {
		static_assert(CT::Trait<T>, "T must be a trait definition");
		return Trait {MetaTrait::Of<Decay<T>>(), Any {Forward<D>(stuff)}};
	}

	/// Create a trait from a trait definition and copy of data						
	///	@tparam DATA - type of data to shallow-copy									
	///	@param meta - the trait definition												
	///	@param stuff - the data to copy													
	///	@return the trait																		
	template<CT::Data DATA>
	Trait Trait::From(TMeta meta, const DATA& stuff) {
		Trait result {stuff};
		result.SetTrait(meta);
		return Abandon(result);
	}

	/// Create a trait from a trait definition and moved data						
	///	@tparam DATA - type of data to move in											
	///	@param meta - the trait definition												
	///	@param stuff - the data to copy													
	///	@return the trait																		
	template<CT::Data DATA>
	Trait Trait::From(TMeta meta, DATA&& stuff) {
		Trait result {Forward<DATA>(stuff)};
		result.SetTrait(meta);
		return Abandon(result);
	}

	/// Create a trait from a trait definition and data								
	inline Trait Trait::FromMeta(TMeta tmeta, DMeta dmeta) {
		Trait result {Block(DataState::Default, dmeta)};
		result.SetTrait(tmeta);
		return Abandon(result);
	}

	/// Set the trait type via a static type												
	///	@tparam T - the trait																
	template<CT::Data T>
	void Trait::SetTrait() noexcept {
		static_assert(CT::Trait<T>, "TRAIT must be a trait definition");
		mTraitType = MetaTrait::Of<T>();
	}

	/// Set the trait type via a dynamic type												
	///	@tparam trait - the trait															
	constexpr void Trait::SetTrait(TMeta trait) noexcept {
		mTraitType = trait;
	}

	/// Check if a trait matches a static definition									
	///	@tparam T - the trait																
	///	@return true if this trait is of the given type								
	template<CT::Data T>
	bool Trait::TraitIs() const {
		static_assert(CT::Trait<T>, "TRAIT must be a trait definition");
		return TraitIs(MetaTrait::Of<T>());
	}

	/// Compare traits																			
	///	@param other - the thing to compare with										
	///	@return true if things are the same												
	template<CT::Data T>
	bool Trait::operator == (const T& other) const {
		if constexpr (CT::Trait<T>)
			return TraitIs(DenseCast(other).mTraitType)
				&& Any::operator == (static_cast<const Any&>(other));
		else
			return Any::operator == (other);
	}



	///																								
	///	Static trait implementation														
	///																								
	
	/// Default trait construction															
	template<class TRAIT>
	StaticTrait<TRAIT>::StaticTrait()
		: Trait {MetaTrait::Of<TRAIT>(), Any{}} {}

	/// Trait copy-construction with anything not abandoned or disowned			
	template<class TRAIT>
	template<CT::NotAbandonedOrDisowned T>
	StaticTrait<TRAIT>::StaticTrait(const T& data)
		: Trait {MetaTrait::Of<TRAIT>(), data} {}

	/// Trait copy-construction with anything not abandoned or disowned			
	template<class TRAIT>
	template<CT::NotAbandonedOrDisowned T>
	StaticTrait<TRAIT>::StaticTrait(T& data)
		: Trait {MetaTrait::Of<TRAIT>(), data} {}

	/// Trait move-construction with anything not abandoned or disowned			
	template<class TRAIT>
	template<CT::NotAbandonedOrDisowned T>
	StaticTrait<TRAIT>::StaticTrait(T&& data)
		: Trait {MetaTrait::Of<TRAIT>(), Forward<T>(data)} {}

	template<class TRAIT>
	StaticTrait<TRAIT>::StaticTrait(Disowned<TRAIT>&& other)
		: Trait {other.mValue.mTraitType, other.template Forward<Any>()} {}

	template<class TRAIT>
	StaticTrait<TRAIT>::StaticTrait(Abandoned<TRAIT>&& other)
		: Trait {other.mValue.mTraitType, other.template Forward<Any>()} {}

	template<class TRAIT>
	template<CT::NotAbandonedOrDisowned T>
	TRAIT& StaticTrait<TRAIT>::operator = (const T& data) {
		if constexpr (CT::Same<T, TRAIT>)
			Any::operator = (data);
		else
			Trait::operator = (data);
		return static_cast<TRAIT&>(*this);
	}

	template<class TRAIT>
	template<CT::NotAbandonedOrDisowned T>
	TRAIT& StaticTrait<TRAIT>::operator = (T& data) {
		return operator = (const_cast<const T&>(data));
	}

	template<class TRAIT>
	template<CT::NotAbandonedOrDisowned T>
	TRAIT& StaticTrait<TRAIT>::operator = (T&& data) {
		if constexpr (CT::Same<T, TRAIT>)
			Any::operator = (data);
		else
			Trait::operator = (data);
		return static_cast<TRAIT&>(*this);
	}

	template<class TRAIT>
	TRAIT& StaticTrait<TRAIT>::operator = (Disowned<TRAIT>&& other) {
		Any::operator = (other.template Forward<Any>());
		return static_cast<TRAIT&>(*this);
	}

	template<class TRAIT>
	TRAIT& StaticTrait<TRAIT>::operator = (Abandoned<TRAIT>&& other) {
		Any::operator = (other.template Forward<Any>());
		return static_cast<TRAIT&>(*this);
	}

	template<class TRAIT>
	template<CT::Data T>
	bool StaticTrait<TRAIT>::operator == (const T& other) const {
		if constexpr (CT::Same<T, StaticTrait<TRAIT>>)
			return Any::operator == (static_cast<const Any&>(DenseCast(other)));
		else if constexpr (CT::Trait<T>)
			return Trait::operator == (static_cast<const Trait&>(DenseCast(other)));
		else
			return Any::operator == (other);
	}

	template<class TRAIT>
	TRAIT StaticTrait<TRAIT>::Clone() const {
		return TRAIT {Any::Clone()};
	}

} // namespace Langulus::Anyness
