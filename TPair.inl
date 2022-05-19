///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Pair.hpp"

#define TEMPLATE() template<CT::Data K, CT::Data V>
#define PAIR() TPair<K, V>

namespace Langulus::Anyness
{

	/// Initialize manually	(noexcept)														
	///	@param key - the key to use														
	///	@param value - the value to use													
	TEMPLATE()
	constexpr PAIR()::TPair(K&& key, V&& value) noexcept requires CT::MoveMakableNoexcept<K, V>
		: mKey {Forward<K>(key)}
		, mValue {Forward<V>(value)} {}

	/// Initialize manually																		
	///	@param key - the key to use														
	///	@param value - the value to use													
	TEMPLATE()
	constexpr PAIR()::TPair(K&& key, V&& value) noexcept requires CT::MoveMakable<K, V>
		: mKey {Forward<K>(key)}
		, mValue {Forward<V>(value)} {}

	/// Piecewise constructor																	
	TEMPLATE()
	template<class... U1, class... U2>
	constexpr PAIR()::TPair(::std::piecewise_construct_t, ::std::tuple<U1...> a, ::std::tuple<U2...> b)
	noexcept(noexcept(TPair(Uneval(a), Uneval(b),
		::std::index_sequence_for<U1...>(),
		::std::index_sequence_for<U2...>())))
		: TPair {a, b, ::std::index_sequence_for<U1...>(), ::std::index_sequence_for<U2...>()} {}

	/// Constructor called from the std::piecewise_construct_t ctor				
	TEMPLATE()
	template<class... U1, size_t... I1, class... U2, size_t... I2>
	PAIR()::TPair(::std::tuple<U1...>& a, ::std::tuple<U2...>& b, ::std::index_sequence<I1...>, ::std::index_sequence<I2...>)
		noexcept(
			noexcept(K {Forward<U1>(::std::get<I1>(Uneval(a)))...})
		&& noexcept(V {Forward<U2>(::std::get<I2>(Uneval(b)))...}))
		: mKey {Forward<U1>(::std::get<I1>(a))...}
		, mValue {Forward<U2>(::std::get<I2>(b))...} {
		(void)a;
		(void)b;
	}

	/// Swap (noexcept)																			
	///	@param other - the pair to swap with											
	TEMPLATE()
	constexpr void PAIR()::Swap(TPair& other) noexcept requires CT::SwappableNoexcept<K, V> {
		::std::swap(mKey, other.mKey);
		::std::swap(mValue, other.mValue);
	}

	/// Swap																							
	///	@param other - the pair to swap with											
	TEMPLATE()
	constexpr void PAIR()::Swap(TPair& other) requires CT::Swappable<K, V> {
		::std::swap(mKey, other.mKey);
		::std::swap(mValue, other.mValue);
	}

	/// Ordering comparison																		
	TEMPLATE()
	auto PAIR()::operator <=> (const TPair& rhs) const noexcept {
		const ::std::strong_ordering result = mKey <=> rhs.mKey;
		if (result != 0)
			return result;
		return mValue <=> rhs.mValue;
	}

	/// Clone the pair																			
	///	@return a cloned pair																
	TEMPLATE()
	PAIR() PAIR()::Clone() const {
		TPair result;
		if constexpr (CT::CloneCopyable<K>)
			result.mKey = mKey.Clone();
		else if constexpr (CT::POD<K>)
			result.mKey = mKey;
		else
			LANGULUS_ASSERT("Key type is not clonable");

		if constexpr (CT::CloneCopyable<V>)
			result.mValue = mValue.Clone();
		else if constexpr (CT::POD<V>)
			result.mValue = mValue;
		else
			LANGULUS_ASSERT("Value type is not clonable");
		return result;
	}

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef PAIR

