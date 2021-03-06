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

	/// Initialize manually by a move (noexcept)											
	///	@param key - the key to use														
	///	@param value - the value to use													
	TEMPLATE()
	constexpr PAIR()::TPair(K&& key, V&& value) noexcept requires CT::MoveMakableNoexcept<K, V>
		: mKey {Forward<K>(key)}
		, mValue {Forward<V>(value)} {}

	/// Initialize manually by a move (noexcept)											
	///	@param key - the key to use														
	///	@param value - the value to use													
	TEMPLATE()
	constexpr PAIR()::TPair(K&& key, V&& value) requires CT::MoveMakable<K, V>
		: mKey {Forward<K>(key)}
		, mValue {Forward<V>(value)} {}

	/// Initialize manually by a shallow-copy (noexcept)								
	///	@param key - the key to use														
	///	@param value - the value to use													
	TEMPLATE()
	constexpr PAIR()::TPair(const K& key, const V& value) noexcept requires CT::CopyMakableNoexcept<K, V>
		: mKey {key}
		, mValue {value} {}

	/// Initialize manually by a shallow-copy												
	///	@param key - the key to use														
	///	@param value - the value to use													
	TEMPLATE()
	constexpr PAIR()::TPair(const K& key, const V& value) requires CT::CopyMakable<K, V>
		: mKey {key}
		, mValue {value} {}

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

