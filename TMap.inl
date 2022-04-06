namespace Langulus::Anyness
{

	#define TEMPLATE template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>

	/// Default templated map construction													
	TEMPLATE
	TMap<KEY, VALUE>::TMap()
		: Map{ } {
		// Initialize the non-templated containers manually					
		Map::mKeys.SetDataID<KEY>(true);
		Any::SetDataID<VALUE>(true);
	}

	/// Copy construction																		
	///	@param copy - the map to shallow copy											
	TEMPLATE
	TMap<KEY, VALUE>::TMap(const TMap<KEY, VALUE>& copy)
		: Map{ static_cast<const Map&>(copy) } { }

	/// Move construction																		
	TEMPLATE
	TMap<KEY, VALUE>::TMap(TMap<KEY, VALUE>&& copy) noexcept
		: Map{ pcForward<Map>(copy) } {}

	/// Manual construction																		
	TEMPLATE
	TMap<KEY, VALUE>::TMap(const DState& state, KEY* keydata, VALUE* valuedata, const pcptr count)
		: Map{ state, keydata, valuedata, count } {}

	/// Copy operator. Doesn't clone data, only references it						
	/// Never copies if types are not compatible, only clears.						
	TEMPLATE
	TMap<KEY, VALUE>& TMap<KEY, VALUE>::operator = (const ME& anypack) {
		Reset();
		new (this) ME(anypack);
		return *this;
	}

	/// Copy operator. Doesn't clone data, only references it						
	/// Never copies if types are not compatible, only clears.						
	TEMPLATE
	TMap<KEY, VALUE>& TMap<KEY, VALUE>::operator = (ME&& anypack) noexcept {
		Reset();
		new (this) ME(pcForward<ME>(anypack));
		return *this;
	}

	/// Clear the map by dereferencing each pointer										
	TEMPLATE
	void TMap<KEY, VALUE>::Clear() {
		Map::Clear();
		mKeys.Clear();
	}

	/// Reset a map by dereferencing each pointer										
	TEMPLATE
	void TMap<KEY, VALUE>::Reset() {
		Map::Reset();
		mKeys.Reset();
	}

	/// Clone the map																				
	TEMPLATE
	TMap<KEY, VALUE> TMap<KEY, VALUE>::Clone() const {
		TMap<KEY, VALUE> clone;
		static_cast<Map&>(clone) = Map::Clone();
		clone.mKeys = mKeys.Clone();
		return clone;
	}

	/// Get the keys																				
	TEMPLATE
	const TAny<KEY>& TMap<KEY, VALUE>::Keys() const noexcept {
		return pcReinterpret<const TAny<KEY>&>(mKeys);
	}

	TEMPLATE
	TAny<KEY>& TMap<KEY, VALUE>::Keys() noexcept {
		return pcReinterpret<TAny<KEY>&>(mKeys);
	}

	/// Get the values																			
	TEMPLATE
	const TAny<VALUE>& TMap<KEY, VALUE>::Values() const noexcept {
		return pcReinterpret<const TAny<VALUE>&>(static_cast<const Any&>(*this));
	}

	TEMPLATE
	TAny<VALUE>& TMap<KEY, VALUE>::Values() noexcept {
		return pcReinterpret<TAny<VALUE>&>(static_cast<Any&>(*this));
	}
		
	/// Get the index of a key 																
	///	@return the index key, or uiNone if such key exists						
	TEMPLATE
	Index TMap<KEY, VALUE>::FindKey(const KEY& key) const {
		return Keys().Find(key);
	}

	/// Get the index of a value																
	///	@return the index key, or uiNone if such key exists						
	TEMPLATE
	Index TMap<KEY, VALUE>::FindValue(const VALUE& value) const {
		return Values().Find(value);
	}
		
	/// Get pair at a special index															
	TEMPLATE
	TPair<KEY*, VALUE*> TMap<KEY, VALUE>::GetPair(const Index& index) {
		const auto idx = Keys().ConstrainMore(index);
		if (idx.IsSpecial())
			return {};

		return GetPair(pcptr(idx.mIndex));
	}

	TEMPLATE
	TPair<const KEY*, const VALUE*> TMap<KEY, VALUE>::GetPair(const Index& index) const {
		return const_cast<TMap<KEY, VALUE>*>(this)->GetPair(index);
	}

	/// Get pair at a raw index																
	TEMPLATE
	TPair<KEY*, VALUE*> TMap<KEY, VALUE>::GetPair(pcptr index) {
		return { &Keys()[index], &Values()[index] };
	}

	TEMPLATE
	TPair<const KEY*, const VALUE*> TMap<KEY, VALUE>::GetPair(pcptr index) const {
		return const_cast<TMap<KEY, VALUE>*>(this)->GetPair(index);
	}

	/// Access values																				
	TEMPLATE
	auto& TMap<KEY, VALUE>::operator [] (const KEY& key) {
		const auto index = FindKey(key);
		if (index.IsSpecial())
			throw Except::BadAccess("Invalid key index");
		return Values()[static_cast<pcptr>(index)];
	}

	TEMPLATE
	auto& TMap<KEY, VALUE>::operator [] (const KEY& idx) const {
		return const_cast<TMap<KEY, VALUE>&>(*this)[idx];
	}

	/// Get the key by special index (const)												
	///	@param index - the index															
	///	@return a constant reference to the key										
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetKey(const Index& idx) const {
		return const_cast<TMap<KEY, VALUE>*>(this)->GetKey<K>(idx);
	}

	/// Get the key by special index															
	///	@param index - the index															
	///	@return a reference to the key													
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetKey(Index index) {
		index = Keys().template ConstrainMore<KEY>(index);
		if (index.IsSpecial())
			throw Except::BadAccess("Can't reference special index");
		return Keys().template Get<K>(static_cast<pcptr>(index));
	}

	/// Get a key by simple index (const)													
	///	@param index - the index															
	///	@return a reference to the key													
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetKey(const pcptr index) const {
		return Keys().template Get<K>(index);
	}

	/// Get a key by simple index																
	///	@param index - the index															
	///	@return a constant reference to the key										
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetKey(const pcptr index) {
		return Keys().template Get<K>(index);
	}

	/// Get the value by special index (const)											
	///	@param index - the index															
	///	@return a constant reference to the value										
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetValue(const Index& index) const {
		return const_cast<TMap<KEY, VALUE>*>(this)->template GetValue<K>(index);
	}

	/// Get the value by special index														
	///	@param index - the index															
	///	@return a reference to the value													
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetValue(Index index) {
		index = Values().template ConstrainMore<VALUE>(index);
		if (index.IsSpecial())
			throw Except::BadAccess("Can't reference special index");
		return Values().template Get<K>(static_cast<pcptr>(index));
	}

	/// Get a value by simple index (const)												
	///	@param index - the index															
	///	@return a constant reference to the value										
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetValue(const pcptr index) const {
		return Values().template Get<K>(index);
	}

	/// Get a value by simple index															
	///	@param index - the index															
	///	@return a reference to the value													
	TEMPLATE
	template<RTTI::ReflectedData K>
	decltype(auto) TMap<KEY, VALUE>::GetValue(const pcptr index) {
		return Values().template Get<K>(index);
	}

	/// Remove matching pairs by key															
	TEMPLATE
	pcptr TMap<KEY, VALUE>::RemoveKey(const KEY& item) {
		const auto index = FindKey(item);
		if (index.IsSpecial())
			return 0;

		return Keys().RemoveIndex(index.mIndex) && Values().RemoveIndex(index.mIndex) 
			? 1 : 0;
	}

	/// Remove matching pairs by value														
	TEMPLATE
	pcptr TMap<KEY, VALUE>::RemoveValue(const VALUE& item) {
		const auto index = FindValue(item);
		if (index.IsSpecial())
			return 0;

		return Keys().RemoveIndex(index.mIndex) && Values().RemoveIndex(index.mIndex)
			? 1 : 0;
	}

	/// Merge if value is a container. Pushes only if not already there			
	TEMPLATE template<class MERGED_VALUE>
	pcptr TMap<KEY, VALUE>::Merge(const KEY& key, const MERGED_VALUE& value)
	requires CopyConstructible<KEY> && CopyConstructible<VALUE> {
		const auto found = FindKey(key);
		if constexpr (pcIsDeep<VALUE>) {
			if constexpr (pcIsDeep<MERGED_VALUE>) {
				if (found.IsSpecial())
					return Add(key, value);

				return GetValue(pcptr(found.mIndex)).Merge(value);
			}
			else {
				if (found.IsSpecial()) {
					VALUE temp;
					temp << value;
					return Add(key, pcMove(temp));
				}

				return GetValue(pcptr(found.mIndex)).Merge(&value);
			}
		}
		else {
			if (found.IsSpecial())
				return Add(key, value);

			return Values().Insert(&value, 1, found);
		}
	}

	/// Merge two maps																			
	TEMPLATE
	pcptr TMap<KEY, VALUE>::Merge(const ME& other)
	requires CopyConstructible<KEY> && CopyConstructible<VALUE> {
		pcptr added = 0;
		for (pcptr i = 0; i < other.Count(); ++i) {
			auto pair = other.GetPair(i);
			added += Merge(*pair.Key, *pair.Value);
		}

		return added;
	}

	/// Emplace anything compatible to container											
	TEMPLATE
	pcptr TMap<KEY, VALUE>::Emplace(Pair&& item, const Index& index) {
		Keys().Emplace(pcMove(item.Key), index);
		return Values().Emplace(pcMove(item.Value), index);
	}

	/// Insert anything compatible to container											
	TEMPLATE
	pcptr TMap<KEY, VALUE>::Insert(const Pair* items, const pcptr count, const Index& index) {
		for (pcptr i = 0; i < count; ++i) {
			Keys().Insert(&items[i].Key, 1, index);
			Values().Insert(&items[i].Value, 1, index);
		}
		return count;
	}

	/// Push any data at the back																
	TEMPLATE
	TMap<KEY, VALUE>& TMap<KEY, VALUE>::operator << (Pair&& other) {
		Emplace(pcForward<Pair>(other), uiBack);
		return *this;
	}

	/// Push any data at the front															
	TEMPLATE
	TMap<KEY, VALUE>& TMap<KEY, VALUE>::operator >> (Pair&& other) {
		Emplace(pcForward<Pair>(other), uiFront);
		return *this;
	}

	/// Add a pair																					
	TEMPLATE
	pcptr TMap<KEY, VALUE>::Add(KEY&& k, VALUE&& v, const Index& index)
	requires MoveConstructible<KEY> && MoveConstructible<VALUE> {
		return Emplace(Pair(pcForward<KEY>(k), pcForward<VALUE>(v)), index);
	}

	TEMPLATE
	pcptr TMap<KEY, VALUE>::Add(const KEY& k, VALUE&& v, const Index& index)
	requires CopyConstructible<KEY> && MoveConstructible<VALUE> {
		return Emplace(Pair(k, pcForward<VALUE>(v)), index);
	}

	TEMPLATE
	pcptr TMap<KEY, VALUE>::Add(KEY&& k, const VALUE& v, const Index& index)
	requires MoveConstructible<KEY> && CopyConstructible<VALUE> {
		return Emplace(Pair(pcForward<KEY>(k), v), index);
	}

	TEMPLATE
	pcptr TMap<KEY, VALUE>::Add(const KEY& k, const VALUE& v, const Index& index)
	requires CopyConstructible<KEY> && CopyConstructible<VALUE> {
		return Emplace(Pair(k, v), index);
	}

	TEMPLATE
	pcptr TMap<KEY, VALUE>::Add(KEY& k, VALUE& v, const Index& index)
	requires CopyConstructible<KEY> && CopyConstructible<VALUE> {
		return Emplace(Pair(k, v), index);
	}

	/// Sort the map																				
	TEMPLATE 
	void TMap<KEY, VALUE>::Sort(const Index& first) {
		auto data = Keys().GetRaw();
		if (!data)
			return;

		pcptr j = 0, i = 0;
		if (first == uiSmallest) {
			for (; i < GetCount(); ++i) {
				for (; j < i; ++j) {
					if (*pcPtr(data[i]) > *pcPtr(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
				for (j = i + 1; j < GetCount(); ++j) {
					if (*pcPtr(data[i]) > *pcPtr(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
			}
		}
		else {
			for (; i < GetCount(); ++i) {
				for (; j < i; ++j) {
					if (*pcPtr(data[i]) < *pcPtr(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
				for (j = i + 1; j < GetCount(); ++j) {
					if (*pcPtr(data[i]) < *pcPtr(data[j])) {
						Keys().Swap(i, j);
						Values().Swap(i, j);
					}
				}
			}
		}
	}

	/// Iteration																					
	TEMPLATE
	template<class FUNCTION>
	pcptr TMap<KEY, VALUE>::ForEach(FUNCTION&& call) {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		static_assert(pcHasBase<KEY, KeyType>, "Incompatible key type for map iteration");
		static_assert(pcHasBase<VALUE, ValueType>, "Incompatible value type for map iteration");
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		return ForEachInner<ReturnType, KeyType, ValueType, false>(
			pcForward<FUNCTION>(call));
	}

	TEMPLATE
	template<class FUNCTION>
	pcptr TMap<KEY, VALUE>::ForEachRev(FUNCTION&& call) {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		static_assert(pcHasBase<KEY, KeyType>, "Incompatible key type for map iteration");
		static_assert(pcHasBase<VALUE, ValueType>, "Incompatible value type for map iteration");
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		return ForEachInner<ReturnType, KeyType, ValueType, true>(
			pcForward<FUNCTION>(call));
	}

	TEMPLATE
	template<class FUNCTION>
	pcptr TMap<KEY, VALUE>::ForEach(FUNCTION&& call) const {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		static_assert(pcHasBase<KEY, KeyType>, "Incompatible key type for map iteration");
		static_assert(pcHasBase<VALUE, ValueType>, "Incompatible value type for map iteration");
		static_assert(Constant<KeyType>, "Non constant key iterator for constant map");
		static_assert(Constant<ValueType>, "Non constant value iterator for constant map");
		return ForEachInner<ReturnType, KeyType, ValueType, false>(
			pcForward<FUNCTION>(call));
	}

	TEMPLATE
	template<class FUNCTION>
	pcptr TMap<KEY, VALUE>::ForEachRev(FUNCTION&& call) const {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		static_assert(pcHasBase<KEY, KeyType>, "Incompatible key type for map iteration");
		static_assert(pcHasBase<VALUE, ValueType>, "Incompatible value type for map iteration");
		static_assert(Constant<KeyType>, "Non constant key iterator for constant map");
		static_assert(Constant<ValueType>, "Non constant value iterator for constant map");
		return ForEachInner<ReturnType, KeyType, ValueType, true>(
			pcForward<FUNCTION>(call));
	}

	/// Constant iteration																		
	TEMPLATE
	template<class RETURN, RTTI::ReflectedData ALT_KEY, RTTI::ReflectedData ALT_VALUE, bool REVERSE>
	pcptr TMap<KEY, VALUE>::ForEachInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&& call) {
		if (IsEmpty())
			return 0;

		constexpr bool HasBreaker = Same<bool, RETURN>;
		const auto count = GetCount();
		pcptr index = 0;
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

	/// Constant iteration																		
	TEMPLATE
	template<class RETURN, RTTI::ReflectedData ALT_KEY, RTTI::ReflectedData ALT_VALUE, bool REVERSE>
	pcptr TMap<KEY, VALUE>::ForEachInner(TFunctor<RETURN(ALT_KEY, ALT_VALUE)>&& call) const {
		return const_cast<TMap<KEY, VALUE>*>(this)
			->ForEachInner<RETURN, ALT_KEY, ALT_VALUE, REVERSE>(
				pcForward<decltype(call)>(call));
	}

	#undef TEMPLATE

} // namespace Langulus::Anyness
