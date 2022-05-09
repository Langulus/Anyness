#pragma once
#include "Pair.hpp"

namespace Langulus::Anyness
{

	///																								
	/// A helper structure for pairing keys and values of any type					
	///																								
	template<ReflectedData K, ReflectedData V>
	struct TPair : public APair {
		using Key = K;
		using Value = V;

		Key mKey;
		Value mValue;

		TPair() = delete;
		constexpr TPair(const TPair&) = default;
		constexpr TPair(TPair&&) noexcept = default;
		constexpr TPair(Key key, Value value)
			: mKey {key}
			, mValue {value} {}
	};

	
	/// A custom pair implementation is used in the map because std::pair is	
	/// not is_trivially_copyable, which means it would not be allowed to be	
	/// used in std::memcpy. This struct is copyable, which is also tested.		
	template <typename T1, typename T2>
	struct pair {
		T1 first;
		T2 second;

		using first_type = T1;
		using second_type = T2;

		template<IsDefaultConstructible U1 = T1, IsDefaultConstructible U2 = T2>
		constexpr pair() noexcept(noexcept(U1()) && noexcept(U2()))
			: first()
			, second() {}

		/// Pair constructors are explicit so we don't accidentally call this	
		/// ctor when we don't have to														
		explicit constexpr pair(std::pair<T1, T2> const& o) noexcept(noexcept(T1(std::declval<T1 const&>())) && noexcept(T2(std::declval<T2 const&>())))
			: first(o.first)
			, second(o.second) {}

		/// Pair constructors are explicit so we don't accidentally call this	
		/// ctor when we don't have to														
		explicit constexpr pair(std::pair<T1, T2>&& o) noexcept(noexcept(T1(Move(std::declval<T1&&>()))) && noexcept(T2(Move(std::declval<T2&&>()))))
			: first(Move(o.first))
			, second(Move(o.second)) {}

		constexpr pair(T1&& a, T2&& b) noexcept(noexcept(T1(Move(std::declval<T1&&>()))) && noexcept(T2(Move(std::declval<T2&&>()))))
			: first(Move(a))
			, second(Move(b)) {}

		template <typename U1, typename U2>
		constexpr pair(U1&& a, U2&& b) noexcept(noexcept(T1(Forward<U1>(std::declval<U1&&>()))) && noexcept(T2(Forward<U2>(std::declval<U2&&>()))))
			: first(Forward<U1>(a))
			, second(Forward<U2>(b)) {}

		template <typename... U1, typename... U2>
		constexpr pair(std::piecewise_construct_t /*unused*/, std::tuple<U1...> a, std::tuple<U2...> b) noexcept(noexcept(pair(std::declval<std::tuple<U1...>&>(), std::declval<std::tuple<U2...>&>(), std::index_sequence_for<U1...>(), std::index_sequence_for<U2...>())))
			: pair(a, b, std::index_sequence_for<U1...>(), std::index_sequence_for<U2...>()) {}

		/// Constructor called from the std::piecewise_construct_t ctor			
		template <typename... U1, size_t... I1, typename... U2, size_t... I2>
		pair(std::tuple<U1...>& a, std::tuple<U2...>& b, std::index_sequence<I1...>, std::index_sequence<I2...>) noexcept(noexcept(T1(Forward<U1>(std::get<I1>(std::declval<std::tuple<U1...>&>()))...)) && noexcept(T2(Forward<U2>(std::get<I2>(std::declval<std::tuple<U2...>&>()))...)))
			: first(Forward<U1>(std::get<I1>(a))...)
			, second(Forward<U2>(std::get<I2>(b))...) { }

		void swap(pair<T1, T2>& o) noexcept (std::is_nothrow_swappable_v<T1> && std::is_nothrow_swappable_v<T2>) {
			std::swap(first, o.first);
			std::swap(second, o.second);
		}
	};

} // namespace Langulus::Anyness