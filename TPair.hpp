#pragma once
#include "Pair.hpp"

namespace Langulus::Anyness
{

	///																								
	///	A helper structure for pairing keys and values of any type				
	///																								
	template<CT::Data K, CT::Data V>
	struct TPair : public APair {
		using Key = K;
		using Value = V;

		Key mKey;
		Value mValue;

		TPair() = default;
		TPair(const TPair&) = default;
		TPair(TPair&&) noexcept = default;

		constexpr TPair(K&&, V&&) noexcept requires CT::MoveMakableNoexcept<K, V>;
		constexpr TPair(K&&, V&&) noexcept requires CT::MoveMakable<K, V>;

		template<class... U1, class... U2>
		constexpr TPair(::std::piecewise_construct_t, ::std::tuple<U1...> a, ::std::tuple<U2...> b)
			noexcept(noexcept(TPair(Uneval(a), Uneval(b),
				::std::index_sequence_for<U1...>(),
				::std::index_sequence_for<U2...>())));

		template<class... U1, size_t... I1, class... U2, size_t... I2>
		TPair(::std::tuple<U1...>& a, ::std::tuple<U2...>& b, ::std::index_sequence<I1...>, ::std::index_sequence<I2...>)
			noexcept(
				noexcept(K {Forward<U1>(::std::get<I1>(Uneval(a)))... })
			&& noexcept(V {Forward<U2>(::std::get<I2>(Uneval(b)))... }));

		constexpr void Swap(TPair&) noexcept requires CT::SwappableNoexcept<K, V>;
		constexpr void Swap(TPair&) requires CT::Swappable<K, V>;

		TPair Clone() const;

		bool operator == (const TPair&) const noexcept = default;
		auto operator <=> (const TPair&) const noexcept;
		TPair& operator = (const TPair&) = default;
		TPair& operator = (TPair&&) noexcept = default;
	};

} // namespace Langulus::Anyness

#include "TPair.inl"