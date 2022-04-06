#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	///																								
	///	TRAIT																						
	///																								
	/// A tagged container																		
	///																								
	class PC_API_MMS Trait : public Any, NOT_DEEP {
		REFLECT(Trait);
	public:
		template<class T>
		static constexpr bool NotCustom =
			Sparse<T> || (!Same<T, Any> && !Same<T, Block> && !Same<T, Trait>);

		constexpr Trait() noexcept : Any{} {}
		Trait(TraitID);
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

		template<RTTI::ReflectedData T>
		Trait& operator = (const T&) requires (Trait::NotCustom<T>);
		template<RTTI::ReflectedData T>
		Trait& operator = (T&) requires (Trait::NotCustom<T>);
		template<RTTI::ReflectedData T>
		Trait& operator = (T&&) requires (Trait::NotCustom<T>);

	public:
		void Reset();
		NOD() Trait Clone() const;

		template<RTTI::ReflectedTrait TRAIT, RTTI::ReflectedData DATA = void>
		NOD() static Trait From();
		template<RTTI::ReflectedData DATA>
		NOD() static Trait From(TMeta, const DATA&);
		template<RTTI::ReflectedData DATA>
		NOD() static Trait From(TMeta, DATA&&);

		template<RTTI::ReflectedTrait TRAIT>
		NOD() static Trait FromMemory(const Block&);
		template<RTTI::ReflectedTrait TRAIT>
		NOD() static Trait FromMemory(Block&&);

		template<RTTI::ReflectedTrait TRAIT, RTTI::ReflectedData DATA>
		NOD() static Trait From(const DATA&);
		template<RTTI::ReflectedTrait TRAIT, RTTI::ReflectedData DATA>
		NOD() static Trait From(DATA&&);

		NOD() static Trait FromMeta(TMeta, DMeta);

	public:
		NOD() TraitID GetTraitID() const noexcept;
		NOD() pcptr GetTraitSwitch() const noexcept;
		NOD() TMeta GetTraitMeta() const noexcept;
		NOD() bool IsTraitValid() const noexcept;
		NOD() bool IsSimilar(const Trait&) const noexcept;

		NOD() bool TraitIs(TraitID) const;

		template<RTTI::ReflectedTrait TRAIT>
		NOD() bool TraitIs() const;

		void SetTraitID(const TraitID&) noexcept;
		bool HasCorrectData() const;

		NOD() bool operator == (const Trait&) const noexcept;
		NOD() bool operator == (const TraitID&) const noexcept;

		NOD() bool operator != (const Trait&) const noexcept;
		NOD() bool operator != (const TraitID&) const noexcept;

	protected:
		// The trait tag																	
		TMeta mTraitType = nullptr;
	};

} // namespace Langulus::Anyness

#include "Trait.inl"