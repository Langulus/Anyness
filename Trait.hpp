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
	///	A named container, used to give containers a standard intent of use	
	///	A count is a count, no matter how you call it. So when your type		
	/// contains a count variable, you can tag it with a Traits::Count tag		
	///	Traits are used to access members of objects at runtime, or access	
	/// global objects, or supply paremeters for content desciptors, such as	
	/// Flow::Construct, as well ass parameters for any Flow::Verb call			
	///																								
	class Trait : public Any {
		LANGULUS(DEEP) false;
		LANGULUS_BASES(Any);

	protected:
		TMeta mTraitType {};

	public:
		using Any::Any;

		template<class T>
		Trait(TMeta, const T&);
		template<class T>
		Trait(TMeta, T&);
		template<class T>
		Trait(TMeta, T&&);

		Trait(Disowned<Trait>&&);
		Trait(Abandoned<Trait>&&);

		using Any::operator =;

		Trait& operator = (Disowned<Trait>&&);
		Trait& operator = (Abandoned<Trait>&&);

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
		template<CT::Data T>
		void SetTrait() noexcept;
		constexpr void SetTrait(TMeta) noexcept;

		template<CT::Data T>
		NOD() bool TraitIs() const;
		NOD() bool TraitIs(TMeta) const;

		NOD() TMeta GetTrait() const noexcept;

		NOD() bool IsTraitValid() const noexcept;
		NOD() bool IsSimilar(const Trait&) const noexcept;
		NOD() bool HasCorrectData() const;

		NOD() bool operator == (const Trait&) const;
	};


	///																								
	///	A statically named trait, used for integrating any custom trait by	
	/// using it as a CRTP																		
	///																								
	template<class TRAIT>
	struct StaticTrait : public Trait {
		LANGULUS(TRAIT) RTTI::LastNameOf<TRAIT>();
		LANGULUS_BASES(Trait);

		StaticTrait();
		template<CT::NotAbandonedOrDisowned T>
		StaticTrait(const T&);
		template<CT::NotAbandonedOrDisowned T>
		StaticTrait(T&);
		template<CT::NotAbandonedOrDisowned T>
		StaticTrait(T&&);

		StaticTrait(Disowned<TRAIT>&&);
		StaticTrait(Abandoned<TRAIT>&&);

		template<CT::NotAbandonedOrDisowned T>
		TRAIT& operator = (const T&);
		template<CT::NotAbandonedOrDisowned T>
		TRAIT& operator = (T&);
		template<CT::NotAbandonedOrDisowned T>
		TRAIT& operator = (T&&);

		TRAIT& operator = (Disowned<TRAIT>&&);
		TRAIT& operator = (Abandoned<TRAIT>&&);

		NOD() bool operator == (const StaticTrait<TRAIT>&) const;

		NOD() TRAIT Clone() const;
	};

} // namespace Langulus::Anyness

namespace Langulus::CT
{

	/// A reflected trait type is any type that inherits Trait, and is			
	/// binary compatible to a Trait															
	template<class... T>
	concept Trait = ((DerivedFrom<T, ::Langulus::Anyness::Trait>
		&& sizeof(T) == sizeof(::Langulus::Anyness::Trait)) && ...);

} // namespace Langulus::CT

namespace Langulus::Traits
{

	using Anyness::StaticTrait;

	/// Logger trait, used to access the logger instance								
	struct Logger : public StaticTrait<Logger> {
		using StaticTrait::StaticTrait;
		using StaticTrait::operator =;
	};

	/// Count trait, used all over the place												
	struct Count : public StaticTrait<Count> {
		using StaticTrait::StaticTrait;
		using StaticTrait::operator =;
	};

	/// Name trait, used all over the place												
	struct Name : public StaticTrait<Name> {
		using StaticTrait::StaticTrait;
		using StaticTrait::operator =;
	};

	/// Context trait, used to access the current environment						
	struct Context : public StaticTrait<Context> {
		using StaticTrait::StaticTrait;
		using StaticTrait::operator =;
	};

} // namespace Langulus::Traits

#include "Trait.inl"
