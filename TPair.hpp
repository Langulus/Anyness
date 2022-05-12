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

		/// Default constructor (noexcept)													
		constexpr TPair() noexcept requires CT::DefaultableNoexcept<K, V>
			: mKey{}
			, mValue{} {}
			
		/// Default constructor																	
		constexpr TPair() requires CT::Defaultable<K, V>
			: mKey{}
			, mValue{} {}

		/// Copy construction (noexcept)														
		constexpr explicit TPair(const TPair& other) noexcept requires CT::CopyMakableNoexcept<K, V>
			: mKey {other.mKey}
			, mValue {other.mValue} {}

		/// Copy construction																	
		constexpr explicit TPair(const TPair& other) requires CT::CopyMakable<K, V>
			: mKey {other.mKey}
			, mValue {other.mValue} {}

		/// Move construction (noexcept)														
		constexpr explicit TPair(TPair&& other) noexcept requires CT::MoveMakableNoexcept<K, V>
			: mKey {Move(other.mKey)}
			, mValue {Move(other.mValue)} {}
			
		/// Move construction																	
		constexpr explicit TPair(TPair&& other) requires CT::MoveMakable<K, V>
			: mKey {Move(other.mKey)}
			, mValue {Move(other.mValue)} {}

		/// Initialize manually	(noexcept)													
		///	@param key - the key to use													
		///	@param value - the value to use												
		constexpr TPair(K&& key, V&& value) noexcept requires CT::MoveMakableNoexcept<K, V>
			: mKey {Forward<K>(key)}
			, mValue {Forward<V>(value)} {}

		/// Initialize manually																	
		///	@param key - the key to use													
		///	@param value - the value to use												
		constexpr TPair(K&& key, V&& value) noexcept requires CT::MoveMakable<K, V>
			: mKey {Forward<K>(key)}
			, mValue {Forward<V>(value)} {}

		/// Piecewise constructor																
		template<class... U1, class... U2>
		constexpr TPair(std::piecewise_construct_t, std::tuple<U1...> a, std::tuple<U2...> b)
		noexcept(noexcept(TPair(
			Uneval<std::tuple<U1...>&>(),
			Uneval<std::tuple<U2...>&>(),
			std::index_sequence_for<U1...>(),
			std::index_sequence_for<U2...>())))
			: TPair{ a, b, std::index_sequence_for<U1...>(), std::index_sequence_for<U2...>() } {}

		/// Constructor called from the std::piecewise_construct_t ctor			
		template<class... U1, size_t... I1, class... U2, size_t... I2>
		TPair(std::tuple<U1...>& a, std::tuple<U2...>& b, std::index_sequence<I1...>, std::index_sequence<I2...>)
			noexcept(
				noexcept(K {Forward<U1>(std::get<I1>(Uneval<std::tuple<U1...>&>()))...})
			&& noexcept(V {Forward<U2>(std::get<I2>(Uneval<std::tuple<U2...>&>()))...}))
			: mKey {Forward<U1>(std::get<I1>(a))...}
			, mValue {Forward<U2>(std::get<I2>(b))...} {
			(void)a;
			(void)b;
		}

		/// Swap (noexcept)																		
		///	@param other - the pair to swap with										
		constexpr void Swap(TPair& other) noexcept requires IsSwappableNoexcept<K, V> {
			::std::swap(mKey, other.mKey);
			::std::swap(mValue, other.mValue);
		}

		/// Swap																						
		///	@param other - the pair to swap with										
		constexpr void Swap(TPair& other) requires IsSwappable<K, V> {
			::std::swap(mKey, other.mKey);
			::std::swap(mValue, other.mValue);
		}

		/// Equality/inequality comparison													
		bool operator == (const TPair&) const noexcept = default;

		/// Ordering comparison																	
		auto operator <=> (const TPair& rhs) const noexcept {
			const ::std::strong_ordering result = mKey <=> rhs.mKey;
			if (result != 0)
				return result;
			return mValue <=> rhs.mValue;
		}
		
		/// Copy assignment (noexcept)														
		TPair& operator = (const TPair& rhs) noexcept requires CT::CopyableNoexcept<K, V> {
			mKey = rhs.mKey;
			mValue = rhs.mValue;
			return *this;
		}
		
		/// Copy assignment																		
		TPair& operator = (const TPair& rhs) requires CT::Copyable<K, V> {
			mKey = rhs.mKey;
			mValue = rhs.mValue;
			return *this;
		}
		
		/// Move assignment (noexcept)														
		TPair& operator = (TPair&& rhs) noexcept requires CT::MovableNoexcept<K, V> {
			mKey = Move(rhs.mKey);
			mValue = Move(rhs.mValue);
			return *this;
		}
		
		/// Move assignment																		
		TPair& operator = (TPair&& rhs) requires CT::Movable<K, V> {
			mKey = Move(rhs.mKey);
			mValue = Move(rhs.mValue);
			return *this;
		}
	};

} // namespace Langulus::Anyness
