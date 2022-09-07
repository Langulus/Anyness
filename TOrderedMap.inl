///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "TOrderedMap.hpp"

#define TEMPLATE() template<CT::Data K, CT::Data V>
#define MAP() TOrderedMap<K, V>

namespace Langulus::Anyness
{

	/// Copy-construct a map from a disowned map											
	/// The disowned map's contents will not be referenced							
	///	@param other - the map to disown													
	TEMPLATE()
	MAP()::TOrderedMap(Disowned<TOrderedMap>&& other) noexcept
		: TUnorderedMap<K, V> {other.Forward<TUnorderedMap<K, V>>()} { }

	/// Move-construct a map from an abandoned map										
	/// The abandoned map will be minimally reset, saving on some instructions	
	///	@param other - the map to abandon												
	TEMPLATE()
	MAP()::TOrderedMap(Abandoned<TOrderedMap>&& other) noexcept
		: TUnorderedMap<K, V> {other.Forward<TUnorderedMap<K, V>>()} { }

	/// Clone the map																				
	///	@return the cloned map																
	TEMPLATE()
	MAP() MAP()::Clone() const {
		TOrderedMap<K, V> cloned;
		static_cast<TUnorderedMap<K, V>&>(cloned) = TUnorderedMap<K, V>::Clone();
		return Abandon(cloned);
	}

	/*
	
	/// Iteration																					
	TEMPLATE()
	template<class F>
	Count MAP()::ForEach(F&& call) {
		using Iterator = decltype(GetLambdaArguments(&F::operator()));
		if constexpr (CT::DerivedFrom<Iterator, APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(CT::DerivedFrom<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(CT::DerivedFrom<Value, ItVal>, "Incompatible value type for map iteration");
			using R = decltype(call(::std::declval<ItKey>(), ::std::declval<ItVal>()));
			return ForEachInner<R, ItKey, ItVal, false>(Forward<F>(call));
		}
		else TODO();
	}

	/// Reverse iteration																		
	TEMPLATE()
	template<class F>
	Count MAP()::ForEachRev(F&& call) {
		using Iterator = decltype(GetLambdaArguments(&F::operator()));
		if constexpr (CT::DerivedFrom<Iterator, APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(CT::DerivedFrom<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(CT::DerivedFrom<Value, ItVal>, "Incompatible value type for map iteration");
			using R = decltype(call(::std::declval<ItKey>(), ::std::declval<ItVal>()));
			return ForEachInner<R, ItKey, ItVal, true>(Forward<F>(call));
		}
		else TODO();
	}

	TEMPLATE()
	template<class F>
	Count MAP()::ForEach(F&& call) const {
		using Iterator = decltype(GetLambdaArguments(&F::operator()));
		if constexpr (CT::DerivedFrom<Iterator, APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(CT::DerivedFrom<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(CT::DerivedFrom<Value, ItVal>, "Incompatible value type for map iteration");
			static_assert(CT::Constant<ItKey>, "Non constant key iterator for constant map");
			static_assert(CT::Constant<ItVal>, "Non constant value iterator for constant map");
			using R = decltype(call(::std::declval<ItKey>(), ::std::declval<ItVal>()));
			return ForEachInner<R, ItKey, ItVal, false>(Forward<F>(call));
		}
		else TODO();
	}

	TEMPLATE()
	template<class F>
	Count MAP()::ForEachRev(F&& call) const {
		using Iterator = decltype(GetLambdaArguments(&F::operator()));
		if constexpr (CT::DerivedFrom<Iterator, APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(CT::DerivedFrom<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(CT::DerivedFrom<Value, ItVal>, "Incompatible value type for map iteration");
			static_assert(CT::Constant<ItKey>, "Non constant key iterator for constant map");
			static_assert(CT::Constant<ItVal>, "Non constant value iterator for constant map");
			using R = decltype(call(::std::declval<ItKey>(), ::std::declval<ItVal>()));
			return ForEachInner<R, ItKey, ItVal, true>(Forward<F>(call));
		}
		else TODO();
	}

	/// CT::Constant iteration																		
	TEMPLATE()
	template<class R, CT::Data ALT_KEY, CT::Data ALT_VALUE, bool REVERSE>
	Count MAP()::ForEachInner(TFunctor<R(ALT_KEY, ALT_VALUE)>&& call) {
		if (IsEmpty())
			return 0;

		constexpr bool HasBreaker = CT::Same<bool, R>;
		const auto count = GetCount();
		Count index {};
		while (index < count) {
			if constexpr (REVERSE) {
				const auto i = count - index - 1;
				if constexpr (HasBreaker) {
					if (!call(GetKey<ALT_KEY>(i), GetValue<ALT_VALUE>(i)))
						return index + 1;
				}
				else call(GetKey<ALT_KEY>(i), GetValue<ALT_VALUE>(i));
			}
			else {
				if constexpr (HasBreaker) {
					if (!call(GetKey<ALT_KEY>(index), GetValue<ALT_VALUE>(index)))
						return index + 1;
				}
				else call(GetKey<ALT_KEY>(index), GetValue<ALT_VALUE>(index));
			}

			++index;
		}

		return index;
	}

	/// CT::Constant iteration																		
	TEMPLATE()
	template<class R, CT::Data ALT_K, CT::Data ALT_V, bool REVERSE>
	Count MAP()::ForEachInner(TFunctor<R(ALT_K, ALT_V)>&& call) const {
		return const_cast<TMap*>(this)
			->ForEachInner<R, ALT_K, ALT_V, REVERSE>(Forward<decltype(call)>(call));
	}
	*/
} // namespace Langulus::Anyness

#undef MAP
#undef TEMPLATE

	
