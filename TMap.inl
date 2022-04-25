#pragma once
#include "TMap.hpp"

#define TEMPLATE() template<ReflectedData K, ReflectedData V>
#define MAP() TMap<K, V>

namespace Langulus::Anyness
{

	/// Default templated map construction													
	TEMPLATE()
	MAP()::TMap()
		: Map { } {
		Map::mKeys.SetType<K>(true);
		Any::SetType<V>(true);
	}

	/// Copy construction																		
	///	@param copy - the map to shallow copy											
	TEMPLATE()
	MAP()::TMap(const TMap& copy)
		: Map {static_cast<const Map&>(copy)} {}

	/// Move construction																		
	TEMPLATE()
	MAP()::TMap(TMap&& copy) noexcept
		: Map {Forward<Map>(copy)} {}

	/// Manual construction																		
	TEMPLATE()
	MAP()::TMap(const DataState& state, Key* keydata, Value* valuedata, const Count& count)
		: Map {state, keydata, valuedata, count} {}

	/// Copy operator. Doesn't clone data, only references it						
	/// Never copies if types are not compatible, only clears.						
	TEMPLATE()
	MAP()& MAP()::operator = (const TMap& anypack) {
		Reset();
		new (this) TMap(anypack);
		return *this;
	}

	/// Copy operator. Doesn't clone data, only references it						
	/// Never copies if types are not compatible, only clears.						
	TEMPLATE()
	MAP()& MAP()::operator = (TMap&& anypack) noexcept {
		Reset();
		new (this) TMap(Forward<TMap>(anypack));
		return *this;
	}

	/// Clear the map by dereferencing each pointer										
	TEMPLATE()
	void MAP()::Clear() {
		Map::Clear();
		mKeys.Clear();
	}

	/// Reset a map by dereferencing each pointer										
	TEMPLATE()
	void MAP()::Reset() {
		Map::Reset();
		mKeys.Reset();
	}

	/// Clone the map																				
	TEMPLATE()
	MAP() MAP()::Clone() const {
		MAP() clone;
		static_cast<Map&>(clone) = Map::Clone();
		clone.mKeys = mKeys.Clone();
		return clone;
	}

	/// Get the keys																				
	TEMPLATE()
	decltype(auto) MAP()::Keys() const noexcept {
		return ReinterpretCast<const KeyList&>(mKeys);
	}

	TEMPLATE()
	decltype(auto) MAP()::Keys() noexcept {
		return ReinterpretCast<KeyList&>(mKeys);
	}

	/// Get the values																			
	TEMPLATE()
	decltype(auto) MAP()::Values() const noexcept {
		return ReinterpretCast<const ValueList&>(static_cast<const Any&>(*this));
	}

	TEMPLATE()
	decltype(auto) MAP()::Values() noexcept {
		return ReinterpretCast<ValueList&>(static_cast<Any&>(*this));
	}
		
	/// Get the index of a key 																
	///	@return the index key, or uiNone if such key exists						
	TEMPLATE()
	Index MAP()::FindKey(const Key& key) const {
		return Keys().Find(key);
	}

	/// Get the index of a value																
	///	@return the index key, or uiNone if such key exists						
	TEMPLATE()
	Index MAP()::FindValue(const Value& value) const {
		return Values().Find(value);
	}
		
	/// Get pair at a special index															
	TEMPLATE()
	auto MAP()::GetPair(const Index& index) {
		return GetPair(Keys().ConstrainMore(index).GetOffset());
	}

	TEMPLATE()
	auto MAP()::GetPair(const Index& index) const {
		return GetPair(Keys().ConstrainMore(index).GetOffset());
	}

	/// Get pair at a raw index																
	TEMPLATE()
	auto MAP()::GetPair(const Offset& offset) noexcept {
		return PairPtr {&Keys()[offset], &Values()[offset]};
	}

	TEMPLATE()
	auto MAP()::GetPair(const Offset& offset) const noexcept {
		return PairConstPtr {&Keys()[offset], &Values()[offset]};
	}

	/// Access values																				
	TEMPLATE()
	decltype(auto) MAP()::operator [] (const Key& key) {
		return Values()[FindKey(key).GetOffset()];
	}

	TEMPLATE()
	decltype(auto) MAP()::operator [] (const Key& key) const {
		return Values()[FindKey(key).GetOffset()];
	}

	/// Get the key by special index (const)												
	///	@param index - the index															
	///	@return a constant reference to the key										
	TEMPLATE()
	template<ReflectedData ALT_K>
	decltype(auto) MAP()::GetKey(const Index& i) const {
		return Keys().Get<ALT_K>(Keys().ConstrainMore<K>(i).GetOffset());
	}

	/// Get the key by special index															
	///	@param index - the index															
	///	@return a reference to the key													
	TEMPLATE()
	template<ReflectedData ALT_K>
	decltype(auto) MAP()::GetKey(const Index& i) {
		return Keys().Get<ALT_K>(Keys().ConstrainMore<K>(i).GetOffset());
	}

	/// Get a key by simple index (const)													
	///	@param index - the index															
	///	@return a reference to the key													
	TEMPLATE()
	template<ReflectedData ALT_K>
	decltype(auto) MAP()::GetKey(const Offset& i) const noexcept {
		return Keys().Get<ALT_K>(i);
	}

	/// Get a key by simple index																
	///	@param index - the index															
	///	@return a constant reference to the key										
	TEMPLATE()
	template<ReflectedData ALT_K>
	decltype(auto) MAP()::GetKey(const Offset& i) noexcept {
		return Keys().Get<ALT_K>(i);
	}

	/// Get the value by special index (const)											
	///	@param index - the index															
	///	@return a constant reference to the value										
	TEMPLATE()
	template<ReflectedData ALT_V>
	decltype(auto) MAP()::GetValue(const Index& i) const {
		return Values().Get<ALT_V>(Values().ConstrainMore<V>(i).GetOffset());
	}

	/// Get the value by special index														
	///	@param index - the index															
	///	@return a reference to the value													
	TEMPLATE()
	template<ReflectedData ALT_V>
	decltype(auto) MAP()::GetValue(const Index& i) {
		return Values().Get<ALT_V>(Values().ConstrainMore<V>(i).GetOffset());
	}

	/// Get a value by simple index (const)												
	///	@param index - the index															
	///	@return a constant reference to the value										
	TEMPLATE()
	template<ReflectedData ALT_V>
	decltype(auto) MAP()::GetValue(const Offset& i) const noexcept {
		return Values().Get<ALT_V>(i);
	}

	/// Get a value by simple index															
	///	@param index - the index															
	///	@return a reference to the value													
	TEMPLATE()
	template<ReflectedData ALT_V>
	decltype(auto) MAP()::GetValue(const Offset& i) noexcept {
		return Values().Get<ALT_V>(i);
	}

	/// Remove matching pairs by key															
	TEMPLATE()
	Count MAP()::RemoveKey(const Key& item) {
		const auto i = FindKey(item).GetOffset();
		return Keys().RemoveIndex(i) == Values().RemoveIndex(i) ? 1 : 0;
	}

	/// Remove matching pairs by value														
	TEMPLATE()
	Count MAP()::RemoveValue(const Value& item) {
		const auto i = FindValue(item).GetOffset();
		return Keys().RemoveIndex(i) == Values().RemoveIndex(i) ? 1 : 0;
	}

	/// Merge if value is a container. Pushes only if not already there			
	TEMPLATE()
	template<class ALT_V>
	Count MAP()::Merge(const Key& key, const ALT_V& value)
	requires IsCopyConstructible<K> && IsCopyConstructible<V> {
		const auto found = FindKey(key);
		if constexpr (Anyness::IsDeep<Value>) {
			if constexpr (Anyness::IsDeep<ALT_V>) {
				if (!found)
					return Add(key, value);
				return GetValue(found.GetOffset()).Merge(value);
			}
			else {
				if (!found) {
					V temp;
					temp << value;
					return Add(key, Move(temp));
				}

				return GetValue(found.GetOffset()).Merge(&value);
			}
		}
		else {
			if (!found)
				return Add(key, value);
			return Values().Insert(&value, 1, found);
		}
	}

	/// Merge two maps																			
	TEMPLATE()
	Count MAP()::Merge(const TMap& other)
	requires IsCopyConstructible<K> && IsCopyConstructible<V> {
		Count added {};
		for (Count i = 0; i < other.Count(); ++i) {
			auto pair = other.GetPair(i);
			added += Merge(*pair.mKey, *pair.mValue);
		}
		return added;
	}

	/// Emplace anything compatible to container											
	TEMPLATE()
	Count MAP()::Emplace(Pair&& item, const Index& index) {
		Keys().Emplace(Move(item.Key), index);
		return Values().Emplace(Move(item.Value), index);
	}

	/// Insert anything compatible to container											
	TEMPLATE()
	Count MAP()::Insert(const Pair* items, const Count& count, const Index& index) {
		for (Count i = 0; i < count; ++i) {
			Keys().Insert(&items[i].Key, 1, index);
			Values().Insert(&items[i].Value, 1, index);
		}
		return count;
	}

	/// Push any data at the back																
	TEMPLATE()
	MAP()& MAP()::operator << (Pair&& other) {
		Emplace(Forward<Pair>(other), Index::Back);
		return *this;
	}

	/// Push any data at the front															
	TEMPLATE()
	MAP()& MAP()::operator >> (Pair&& other) {
		Emplace(Forward<Pair>(other), Index::Front);
		return *this;
	}

	/// Add a pair																					
	TEMPLATE()
	Count MAP()::Add(Key&& k, Value&& v, const Index& index)
	requires IsMoveConstructible<K> && IsMoveConstructible<V> {
		return Emplace(Pair(Forward<Key>(k), Forward<Value>(v)), index);
	}

	TEMPLATE()
	Count MAP()::Add(const Key& k, Value&& v, const Index& index)
	requires IsCopyConstructible<K> && IsMoveConstructible<V> {
		return Emplace(Pair(k, Forward<Value>(v)), index);
	}

	TEMPLATE()
	Count MAP()::Add(Key&& k, const Value& v, const Index& index)
	requires IsMoveConstructible<K> && IsCopyConstructible<V> {
		return Emplace(Pair(Forward<Key>(k), v), index);
	}

	TEMPLATE()
	Count MAP()::Add(const Key& k, const Value& v, const Index& index)
	requires IsCopyConstructible<K> && IsCopyConstructible<V> {
		return Emplace(Pair(k, v), index);
	}

	TEMPLATE()
	Count MAP()::Add(Key& k, Value& v, const Index& index)
	requires IsCopyConstructible<K> && IsCopyConstructible<V> {
		return Emplace(Pair(k, v), index);
	}

	/// Sort the map																				
	TEMPLATE() 
	void MAP()::Sort(const Index& first) {
		auto data = Keys().GetRaw();
		if (!data)
			return;

		Count j {}, i {};
		if (first == Index::Smallest) {
			for (; i < GetCount(); ++i) {
				for (; j < i; ++j) {
					if (MakeDense(data[i]) > MakeDense(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
				for (j = i + 1; j < GetCount(); ++j) {
					if (MakeDense(data[i]) > MakeDense(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
			}
		}
		else {
			for (; i < GetCount(); ++i) {
				for (; j < i; ++j) {
					if (MakeDense(data[i]) < MakeDense(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
				for (j = i + 1; j < GetCount(); ++j) {
					if (MakeDense(data[i]) < MakeDense(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
			}
		}
	}
	
	/// Iteration																					
	TEMPLATE()
	template<class F>
	Count MAP()::ForEach(F&& call) {
		using Iterator = decltype(GetLambdaArguments(&F::operator()));
		if constexpr (Inherits<Iterator, Inner::APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(Inherits<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(Inherits<Value, ItVal>, "Incompatible value type for map iteration");
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
		if constexpr (Inherits<Iterator, Inner::APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(Inherits<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(Inherits<Value, ItVal>, "Incompatible value type for map iteration");
			using R = decltype(call(::std::declval<ItKey>(), ::std::declval<ItVal>()));
			return ForEachInner<R, ItKey, ItVal, true>(Forward<F>(call));
		}
		else TODO();
	}

	TEMPLATE()
	template<class F>
	Count MAP()::ForEach(F&& call) const {
		using Iterator = decltype(GetLambdaArguments(&F::operator()));
		if constexpr (Inherits<Iterator, Inner::APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(Inherits<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(Inherits<Value, ItVal>, "Incompatible value type for map iteration");
			static_assert(Langulus::IsConstant<ItKey>, "Non constant key iterator for constant map");
			static_assert(Langulus::IsConstant<ItVal>, "Non constant value iterator for constant map");
			using R = decltype(call(::std::declval<ItKey>(), ::std::declval<ItVal>()));
			return ForEachInner<R, ItKey, ItVal, false>(Forward<F>(call));
		}
		else TODO();
	}

	TEMPLATE()
	template<class F>
	Count MAP()::ForEachRev(F&& call) const {
		using Iterator = decltype(GetLambdaArguments(&F::operator()));
		if constexpr (Inherits<Iterator, Inner::APair>) {
			// We're iterating pairs													
			using ItKey = typename Iterator::Key;
			using ItVal = typename Iterator::Value;
			static_assert(Inherits<Key, ItKey>, "Incompatible key type for map iteration");
			static_assert(Inherits<Value, ItVal>, "Incompatible value type for map iteration");
			static_assert(Langulus::IsConstant<ItKey>, "Non constant key iterator for constant map");
			static_assert(Langulus::IsConstant<ItVal>, "Non constant value iterator for constant map");
			using R = decltype(call(::std::declval<ItKey>(), ::std::declval<ItVal>()));
			return ForEachInner<R, ItKey, ItVal, true>(Forward<F>(call));
		}
		else TODO();
	}

	/// IsConstant iteration																		
	TEMPLATE()
	template<class R, ReflectedData ALT_KEY, ReflectedData ALT_VALUE, bool REVERSE>
	Count MAP()::ForEachInner(TFunctor<R(ALT_KEY, ALT_VALUE)>&& call) {
		if (IsEmpty())
			return 0;

		constexpr bool HasBreaker = IsSame<bool, R>;
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

	/// IsConstant iteration																		
	TEMPLATE()
	template<class R, ReflectedData ALT_K, ReflectedData ALT_V, bool REVERSE>
	Count MAP()::ForEachInner(TFunctor<R(ALT_K, ALT_V)>&& call) const {
		return const_cast<TMap*>(this)
			->ForEachInner<R, ALT_K, ALT_V, REVERSE>(Forward<decltype(call)>(call));
	}

} // namespace Langulus::Anyness

#undef MAP
#undef TEMPLATE

	
