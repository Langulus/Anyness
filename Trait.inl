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

	/// Create a trait from a trait definition and data								
	template<CT::Data DATA>
	Trait Trait::From(TMeta meta, const DATA& stuff) {
		return Trait(meta, Any(stuff));
	}

	/// Create a trait from a trait definition and data								
	template<CT::Data DATA>
	Trait Trait::From(TMeta meta, DATA&& stuff) {
		return Trait(meta, Any(Forward<DATA>(stuff)));
	}

	/// Create a trait from a trait definition and data								
	inline Trait Trait::FromMeta(TMeta tmeta, DMeta dmeta) {
		return Trait(tmeta, Block(DataState::Default, dmeta));
	}

	template<CT::Data TRAIT>
	bool Trait::TraitIs() const {
		static_assert(CT::Trait<TRAIT>, "TRAIT must be a trait definition");
		return TraitIs(TRAIT::ID);
	}

	/// Assign by shallow-copying some value different from Trait					
	///	@param value - the value to copy													
	template<CT::Data T>
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
	}

} // namespace Langulus::Anyness
