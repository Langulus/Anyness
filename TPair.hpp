#pragma once
#include "Pair.hpp"

namespace Langulus::Anyness
{

	template<class... T>
	concept IsNoexceptDefaultConstructible = (noexcept(T{}) && ...);

	template<class... T>
	concept IsNoexceptCopyConstructible = (noexcept(T{ Uneval<const T&>() }) && ...);

	template<class... T>
	concept IsNoexceptMoveConstructible = (noexcept(T{ Move(Uneval<T&&>()) }) && ...);

	template<class... T>
	concept IsNoexceptSwappable = (::std::is_nothrow_swappable_v<T> && ...);


	///																								
	///	A helper structure for pairing keys and values of any type				
	///																								
	template<ReflectedData K, ReflectedData V>
	struct TPair : public APair {
		using Key = K;
		using Value = V;

		Key mKey;
		Value mValue;

		/// Default constructor																	
		constexpr TPair() noexcept(IsNoexceptDefaultConstructible<K, V>)
		requires (IsDefaultConstructible<K, V>)
			: mKey{}
			, mValue{} {}

		/// Pair constructors are explicit so we don't accidentally call this	
		/// ctor when we don't have to														
		constexpr explicit TPair(const TPair& other) noexcept(IsNoexceptCopyConstructible<K, V>)
			: mKey {other.mKey}
			, mValue {other.mValue} {}

		/// Pair constructors are explicit so we don't accidentally call this	
		/// ctor when we don't have to														
		constexpr explicit TPair(TPair&& other) noexcept(IsNoexceptMoveConstructible<K, V>)
			: mKey {Move(other.mKey)}
			, mValue {Move(other.mValue)} {}

		/// Initialize manually																	
		/// @param key - the key to use														
		/// @param value - the value to use													
		constexpr TPair(K&& key, V&& value) noexcept(IsNoexceptMoveConstructible<K, V>)
			: mKey {Forward<K>(key)}
			, mValue {Forward<V>(value)} {}

		/// Swap																						
		///	@param other - the pair to swap with										
		constexpr void Swap(TPair& other) noexcept (IsNoexceptSwappable<K, V>) {
			::std::swap(mKey, other.mKey);
			::std::swap(mValue, other.mValue);
		}

		/// Equality/inequality comparison													
		constexpr bool operator == (const TPair&) const noexcept = default;

		/// Ordering comparison																	
		constexpr auto operator <=> (const TPair& rhs) const noexcept {
			const ::std::strong_ordering result = mKey <=> rhs.mKey;
			if (result != 0)
				return result;
			return mValue <=> rhs.mValue;
		}
	};

} // namespace Langulus::Anyness