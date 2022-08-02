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

	/// Manual trait construction by movement												
	///	@tparam T - type of the contained data											
	///	@param type - type of the trait													
	///	@param data - data to move inside trait										
	template<class T>
	Trait::Trait(TMeta type, T&& data)
		: Any {Forward<T>(data)}
		, mTraitType {type} {}

	/// Create a trait from a trait definition											
	template<CT::Data TRAIT, CT::Data DATA>
	Trait Trait::From() {
		static_assert(CT::Trait<TRAIT>, "TRAIT must be a trait definition");
		return Trait(TRAIT::Reflect(), Block::From<DATA>());
	}

	/// Create a trait from a trait definition											
	template<CT::Data TRAIT>
	Trait Trait::FromMemory(const Block& memory) {
		static_assert(CT::Trait<TRAIT>, "TRAIT must be a trait definition");
		return Trait(TRAIT::Reflect(), memory);
	}

	/// Create a trait from a trait definition											
	template<CT::Data TRAIT>
	Trait Trait::FromMemory(Block&& memory) {
		static_assert(CT::Trait<TRAIT>, "TRAIT must be a trait definition");
		return Trait(TRAIT::Reflect(), Forward<Block>(memory));
	}

	/// Create a trait from a trait definition and data								
	template<CT::Data TRAIT, CT::Data DATA>
	Trait Trait::From(const DATA& stuff) {
		static_assert(CT::Trait<TRAIT>, "TRAIT must be a trait definition");
		return Trait(TRAIT::Reflect(), Any(stuff));
	}

	/// Create a trait from a trait definition by moving data						
	template<CT::Data TRAIT, CT::Data DATA>
	Trait Trait::From(DATA&& stuff) {
		static_assert(CT::Trait<TRAIT>, "TRAIT must be a trait definition");
		return Trait(TRAIT::Reflect(), Any(Forward<DATA>(stuff)));
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
		//return Trait(tmeta, Block(DataState::Default, dmeta));
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

	/// Assign by shallow-copying some value different from Trait					
	///	@param value - the value to copy													
	/*template<CT::Data T>
	Trait& Trait::operator = (const T& value) requires (Trait::NotCustom<T>) {
		Any::operator = (Any {value});
		return *this;
	}

	/// Assign by shallow-copying some value different from Trait					
	///	@param value - the value to copy													
	template<CT::Data T>
	Trait& Trait::operator = (T& value) requires (Trait::NotCustom<T>) {
		Any::operator = (const_cast<const T&>(value));
		return *this;
	}

	/// Assign by moving some value different from Any									
	///	@param value - the value to move													
	template<CT::Data T>
	Trait& Trait::operator = (T&& value) requires (Trait::NotCustom<T>) {
		Any::operator = (Any {Forward<T>(value)});
		return *this;
	}*/

} // namespace Langulus::Anyness
