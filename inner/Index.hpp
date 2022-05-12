#pragma once
#include "Exceptions.hpp"

namespace Langulus::Anyness
{

	///																								
	///	A multipurpose index, used to access common elements in containers	
	///																								
	struct Index {
		using Type = ::std::ptrdiff_t;

		/// These are defines useful for special indices								
		static constexpr Type MaxIndex = ::std::numeric_limits<Type>::max();
		static constexpr Type MinIndex = ::std::numeric_limits<Type>::min();
		static constexpr Type SymbolCount = ::std::numeric_limits<Type>::digits + 1;

		enum SpecialIndices : Type {
			// All, Many, and Single must be compared in separate context	
			All = MinIndex,
			Many,
			Single,

			// Back, Middle, Front, and None must be compared separately	
			None,
			Front,
			Middle,
			Back,

			// These can't be compared													
			Mode,
			Biggest,
			Smallest,
			Auto,
			Random,

			// This signifies the end of the special indices					
			SpecialIndexCounter,

			// These must be wrapped before compared								
			Last = -1,

			// These fit into the non-special category							
			First = 0
		};

		static constexpr Token Names[SpecialIndexCounter - MinIndex] = {
			u8"All",
			u8"Many",
			u8"Single",

			u8"None",
			u8"Front",
			u8"Middle",
			u8"Back",

			u8"Mode",
			u8"Biggest",
			u8"Smallest",
			u8"Auto",
			u8"Random"
		};

		#if LANGULUS_DEBUG()
			union {
				// Named index (useful for debugging)								
				SpecialIndices mNamedIndex {SpecialIndices::None};
				// Raw index																
				Type mIndex;
			};
		#else
			Type mIndex {SpecialIndices::None};
		#endif

	public:
		constexpr Index() noexcept = default;
		constexpr Index(const Index&) noexcept = default;
		constexpr Index(const SpecialIndices&) noexcept;
		template<CT::SignedInteger T>
		constexpr Index(const T&) noexcept;
		template<CT::UnsignedInteger T>
		constexpr Index(const T&);

	public:
		NOD() constexpr Index Constrained(Count) const noexcept;
		NOD() constexpr Offset GetOffset() const;

		constexpr void Constrain(Count) noexcept;
		constexpr void Concat(const Index&) noexcept;

		NOD() constexpr bool IsValid() const noexcept;
		NOD() constexpr bool IsInvalid() const noexcept;
		NOD() constexpr bool IsSpecial() const noexcept;
		NOD() constexpr bool IsReverse() const noexcept;
		NOD() constexpr bool IsArithmetic() const noexcept;

		NOD() explicit constexpr operator bool() const noexcept;
		NOD() explicit constexpr operator const Type& () const noexcept;

		constexpr void operator ++ () noexcept;
		constexpr void operator -- () noexcept;
		constexpr void operator += (const Index&) noexcept;
		constexpr void operator -= (const Index&) noexcept;
		constexpr void operator *= (const Index&) noexcept;
		constexpr void operator /= (const Index&) noexcept;

		NOD() constexpr Index operator + (const Index&) const noexcept;
		NOD() constexpr Index operator - (const Index&) const noexcept;
		NOD() constexpr Index operator * (const Index&) const noexcept;
		NOD() constexpr Index operator / (const Index&) const noexcept;
		NOD() constexpr Index operator - () const noexcept;

		NOD() constexpr bool operator == (const Index&) const noexcept;
		NOD() constexpr bool operator != (const Index&) const noexcept;

		NOD() constexpr bool operator < (const Index&) const noexcept;
		NOD() constexpr bool operator > (const Index&) const noexcept;
		NOD() constexpr bool operator <= (const Index&) const noexcept;
		NOD() constexpr bool operator >= (const Index&) const noexcept;
	};

} // namespace Langulus::Anyness

#include "Index.inl"
