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
	protected:
		// The trait tag																	
		TMeta mTraitType {};

	public:
		template<class T>
		static constexpr bool NotCustom =
			Sparse<T> || (!Same<T, Any> && !Same<T, Block> && !Same<T, Trait>);

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

		template<ReflectedData T>
		Trait& operator = (const T&) requires (Trait::NotCustom<T>);
		template<ReflectedData T>
		Trait& operator = (T&) requires (Trait::NotCustom<T>);
		template<ReflectedData T>
		Trait& operator = (T&&) requires (Trait::NotCustom<T>);

	public:
		void Reset();
		NOD() Trait Clone() const;

		template<ReflectedTrait TRAIT, ReflectedData DATA>
		NOD() static Trait From();
		template<ReflectedData DATA>
		NOD() static Trait From(TMeta, const DATA&);
		template<ReflectedData DATA>
		NOD() static Trait From(TMeta, DATA&&);

		template<ReflectedTrait TRAIT>
		NOD() static Trait FromMemory(const Block&);
		template<ReflectedTrait TRAIT>
		NOD() static Trait FromMemory(Block&&);

		template<ReflectedTrait TRAIT, ReflectedData DATA>
		NOD() static Trait From(const DATA&);
		template<ReflectedTrait TRAIT, ReflectedData DATA>
		NOD() static Trait From(DATA&&);

		NOD() static Trait FromMeta(TMeta, DMeta);

	public:
		void SetTrait(TMeta) noexcept;
		NOD() TMeta GetTrait() const noexcept;
		NOD() bool IsTraitValid() const noexcept;
		NOD() bool IsSimilar(const Trait&) const noexcept;
		NOD() bool TraitIs(TMeta) const;
		template<ReflectedTrait TRAIT>
		NOD() bool TraitIs() const;
		NOD() bool HasCorrectData() const;

		NOD() bool operator == (const Trait&) const noexcept;
		NOD() bool operator == (TMeta) const noexcept;

		NOD() bool operator != (const Trait&) const noexcept;
		NOD() bool operator != (TMeta) const noexcept;
	};

} // namespace Langulus::Anyness

#include "Trait.inl"