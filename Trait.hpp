///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	///																								
	///	TRAIT																						
	///																								
	/// A tagged container																		
	///																								
	class Trait : public Any {
		LANGULUS(DEEP) false;
		LANGULUS_BASES(Any);
	protected:
		TMeta mTraitType {};

	public:
		template<class T>
		static constexpr bool NotCustom =
			CT::Sparse<T> || (!CT::Same<T, Any> && !CT::Same<T, Block> && !CT::Same<T, Trait>);

		constexpr Trait() noexcept : Any{} {}
		Trait(TMeta);

		Trait(const Any&);
		Trait(Any&);
		Trait(Any&&) noexcept;

		Trait(const Block&);
		Trait(Block&);
		Trait(Block&&) noexcept;

		Trait(const Trait&);
		Trait(Trait&);
		Trait(Trait&&) noexcept;

		Trait(TMeta, const Any&);
		Trait(TMeta, Any&);
		Trait(TMeta, Any&&) noexcept;

		Trait(TMeta, const Block&);
		Trait(TMeta, Block&);
		Trait(TMeta, Block&&) noexcept;

		Trait& operator = (const Any&);
		Trait& operator = (Any&);
		Trait& operator = (Any&&) noexcept;

		Trait& operator = (const Trait&);
		Trait& operator = (Trait&);
		Trait& operator = (Trait&&) noexcept;

		Trait& operator = (const Block&);
		Trait& operator = (Block&);
		Trait& operator = (Block&&) noexcept;

		template<CT::Data T>
		Trait& operator = (const T&) requires (Trait::NotCustom<T>);
		template<CT::Data T>
		Trait& operator = (T&) requires (Trait::NotCustom<T>);
		template<CT::Data T>
		Trait& operator = (T&&) requires (Trait::NotCustom<T>);

	public:
		void Reset();
		NOD() Trait Clone() const;

		template<CT::Trait TRAIT, CT::Data DATA>
		NOD() static Trait From();
		template<CT::Data DATA>
		NOD() static Trait From(TMeta, const DATA&);
		template<CT::Data DATA>
		NOD() static Trait From(TMeta, DATA&&);

		template<CT::Trait TRAIT>
		NOD() static Trait FromMemory(const Block&);
		template<CT::Trait TRAIT>
		NOD() static Trait FromMemory(Block&&);

		template<CT::Trait TRAIT, CT::Data DATA>
		NOD() static Trait From(const DATA&);
		template<CT::Trait TRAIT, CT::Data DATA>
		NOD() static Trait From(DATA&&);

		NOD() static Trait FromMeta(TMeta, DMeta);

	public:
		void SetTrait(TMeta) noexcept;
		NOD() TMeta GetTrait() const noexcept;
		NOD() bool IsTraitValid() const noexcept;
		NOD() bool IsSimilar(const Trait&) const noexcept;
		NOD() bool TraitIs(TMeta) const;
		template<CT::Trait TRAIT>
		NOD() bool TraitIs() const;
		NOD() bool HasCorrectData() const;

		NOD() bool operator == (const Trait&) const noexcept;
		NOD() bool operator == (TMeta) const noexcept;

		NOD() bool operator != (const Trait&) const noexcept;
		NOD() bool operator != (TMeta) const noexcept;
	};

	namespace Traits
	{

		class Count : public Trait {
		public:
			using Trait::Trait;
		};

	} // namespace Langulus::Anyness::Traits

} // namespace Langulus::Anyness

#include "Trait.inl"
