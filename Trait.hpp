///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
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
		Trait(Any&&) noexcept;

		Trait(const Block&);
		Trait(Block&&) noexcept;

		Trait(const Trait&);
		Trait(Trait&&) noexcept;

		Trait(TMeta, const Any&);
		Trait(TMeta, Any&&) noexcept;

		Trait(TMeta, const Block&);
		Trait(TMeta, Block&&) noexcept;

		Trait& operator = (const Any&);
		Trait& operator = (Any&&) noexcept;

		Trait& operator = (const Trait&);
		Trait& operator = (Trait&&) noexcept;

		Trait& operator = (const Block&);
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

		template<CT::Data TRAIT, CT::Data DATA>
		NOD() static Trait From();
		template<CT::Data DATA>
		NOD() static Trait From(TMeta, const DATA&);
		template<CT::Data DATA>
		NOD() static Trait From(TMeta, DATA&&);

		template<CT::Data TRAIT>
		NOD() static Trait FromMemory(const Block&);
		template<CT::Data TRAIT>
		NOD() static Trait FromMemory(Block&&);

		template<CT::Data TRAIT, CT::Data DATA>
		NOD() static Trait From(const DATA&);
		template<CT::Data TRAIT, CT::Data DATA>
		NOD() static Trait From(DATA&&);

		NOD() static Trait FromMeta(TMeta, DMeta);

	public:
		void SetTrait(TMeta) noexcept;
		NOD() TMeta GetTrait() const noexcept;
		NOD() bool IsTraitValid() const noexcept;
		NOD() bool IsSimilar(const Trait&) const noexcept;
		NOD() bool TraitIs(TMeta) const;
		template<CT::Data TRAIT>
		NOD() bool TraitIs() const;
		NOD() bool HasCorrectData() const;

		NOD() bool operator == (const Trait&) const noexcept;
		NOD() bool operator == (TMeta) const noexcept;
	};

} // namespace Langulus::Anyness


namespace Langulus::Traits
{

	using Anyness::Trait;

	struct Count : public Trait {
		using Trait::Trait;
	};

	struct Name : public Trait {
		using Trait::Trait;
	};

	struct Context : public Trait {
		using Trait::Trait;
	};

} // namespace Langulus::Traits


namespace Langulus::CT
{

	/// A reflected trait type is any type that inherits Trait, and is			
	/// binary compatible to a Trait															
	template<class... T>
	concept Trait = ((DerivedFrom<T, ::Langulus::Anyness::Trait>
		&& sizeof(T) == sizeof(::Langulus::Anyness::Trait)) && ...);

} // namespace Langulus::CT

#include "Trait.inl"
