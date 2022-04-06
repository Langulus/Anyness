namespace Langulus::Anyness
{

	/// Get the keys container (constant)													
	inline const Any& Map::Keys() const noexcept {
		return mKeys;
	}

	/// Get the keys container																	
	inline Any& Map::Keys() noexcept {
		return mKeys;
	}

	/// Get the values container																
	inline const Any& Map::Values() const noexcept {
		return static_cast<const Any&>(*this);
	}

	/// Get the values container (constant)												
	inline Any& Map::Values() noexcept {
		return static_cast<Any&>(*this);
	}

	/// Create a strictly typed container, using templates							
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	Map Map::From(const DState& state) noexcept {
		return Map{
			Block{ state, DataID::Reflect<KEY>() },
			Block{ state, DataID::Reflect<VALUE>() }
		};
	}

	/// Get the meta definition associated with the contained key type			
	inline DMeta Map::KeyMeta() const {
		return mKeys.GetMeta();
	}

	/// Get the meta definition associated with the contained value type			
	inline DMeta Map::ValueMeta() const {
		return Any::GetMeta();
	}

	/// Get the index of a key																	
	template<RTTI::ReflectedData KEY>
	Index Map::FindKey(const KEY& key) const {
		return mKeys.Find<KEY>(key);
	}

	/// Get the index of a value																
	template<RTTI::ReflectedData VALUE>
	Index Map::FindValue(const VALUE& value) const {
		return Any::Find<VALUE>(value);
	}

	/// Get pair at a special index															
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	auto Map::GetPair(const Index& index) {
		const auto idx = mKeys.ConstrainMore<KEY>(index);
		if (idx.IsSpecial())
			return {};
		return GetPair<KEY, VALUE>(static_cast<pcptr>(idx.mIndex));
	}

	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	auto Map::GetPair(const Index& index) const {
		return const_cast<Map*>(this)->GetPair<KEY, VALUE>(index);
	}

	/// Get pair at a raw index																
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	auto Map::GetPair(pcptr index) {
		return TPair<decltype(GetKey<KEY>(index)), decltype(GetValue<VALUE>(index))>{
			GetKey<KEY>(index), 
			GetValue<VALUE>(index)
		};
	}

	/// Get pair at a raw index (const)														
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	auto Map::GetPair(pcptr index) const {
		return const_cast<Map*>(this)->GetPair<KEY, VALUE>(index);
	}

	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	bool Map::IsMapInsertable() {
		if (mKeys.IsUntyped())
			mKeys.SetDataID<KEY>(false);
		if (Any::IsUntyped())
			Any::SetDataID<VALUE>(false);
		return mKeys.IsInsertable<KEY>() && Any::IsInsertable<VALUE>();
	}

	
	/// Get the key by special index (const)												
	///	@param idx - the index																
	///	@return a constant reference to the key										
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetKey(const Index& idx) const {
		return Keys().As<K>(idx);
	}

	/// Get the key by special index															
	///	@param idx - the index																
	///	@return a reference to the key													
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetKey(const Index& idx) {
		return Keys().As<K>(idx);
	}

	/// Get a key by simple index (const)													
	///	@param idx - the index																
	///	@return a reference to the key													
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetKey(const pcptr idx) const {
		return Keys().As<K>(idx);
	}

	/// Get a key by simple index																
	///	@param idx - the index																
	///	@return a constant reference to the key										
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetKey(const pcptr idx) {
		return Keys().As<K>(idx);
	}

	/// Get the value by special index (const)											
	///	@param idx - the index																
	///	@return a constant reference to the value										
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetValue(const Index& idx) const {
		return Values().As<K>(idx);
	}

	/// Get the value by special index														
	///	@param idx - the index																
	///	@return a reference to the value													
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetValue(const Index& idx) {
		return Values().As<K>(idx);
	}

	/// Get a value by simple index (const)												
	///	@param idx - the index																
	///	@return a constant reference to the value										
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetValue(const pcptr idx) const {
		return Values().As<K>(idx);
	}

	/// Get a value by simple index															
	///	@param idx - the index																
	///	@return a reference to the value													
	template<RTTI::ReflectedData K>
	decltype(auto) Map::GetValue(const pcptr idx) {
		return Values().As<K>(idx);
	}

	/// Emplace anything compatible to container											
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	pcptr Map::Emplace(TPair<KEY, VALUE>&& item, const Index& index) {
		if (!IsMapInsertable<KEY, VALUE>())
			throw Except::BadMove("Bad emplace in map - type not insertable");

		const auto insertedKeys = mKeys.Emplace<KEY>(pcMove(item.Key), index);
		const auto insertedValues = Any::Emplace<VALUE>(pcMove(item.Value), index);
		if (insertedKeys != insertedValues)
			throw Except::BadMove("Bad emplace in map - move failed");

		return insertedKeys;
	}

	/// Insert anything compatible to container											
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	pcptr Map::Insert(const TPair<KEY, VALUE>* items, const pcptr count, const Index& index) {
		if (!IsMapInsertable<KEY, VALUE>())
			throw Except::BadCopy("Bad insert in map");

		pcptr insertedKeys = 0;
		pcptr insertedValues = 0;
		for (pcptr i = 0; i < count; ++i) {
			insertedKeys += mKeys.Insert<KEY>(&items[i].Key, 1, index);
			insertedValues += Any::Insert<VALUE>(&items[i].Value, 1, index);
		}

		return insertedKeys;
	}

	/// Push any data at the back																
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	Map& Map::operator << (const TPair<KEY, VALUE>& other) {
		Insert<KEY, VALUE>(&other, 1, uiBack);
		return *this;
	}

	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	Map& Map::operator << (TPair<KEY, VALUE>&& other) {
		Emplace<KEY, VALUE>(pcForward<TPair<KEY, VALUE>>(other), uiBack);
		return *this;
	}

	/// Push any data at the front															
	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	Map& Map::operator >> (const TPair<KEY, VALUE>& other) {
		Insert<KEY, VALUE>(&other, 1, uiFront);
		return *this;
	}

	template<RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE>
	Map& Map::operator >> (TPair<KEY, VALUE>&& other) {
		Emplace<KEY, VALUE>(pcForward<TPair<KEY, VALUE>>(other), uiFront);
		return *this;
	}

	/// Iteration																					
	template<class FUNCTION>
	pcptr Map::ForEachPair(FUNCTION&& call) {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		return ForEachPairInner<ReturnType, KeyType, ValueType, false>(
			pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	pcptr Map::ForEachPairRev(FUNCTION&& call) {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		return ForEachPairInner<ReturnType, KeyType, ValueType, true>(
			pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	pcptr Map::ForEachPair(FUNCTION&& call) const {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		static_assert(Constant<KeyType>, "Non constant key iterator for constant map");
		static_assert(Constant<ValueType>, "Non constant value iterator for constant map");
		return ForEachPairInner<ReturnType, KeyType, ValueType, false>(
			pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	pcptr Map::ForEachPairRev(FUNCTION&& call) const {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		static_assert(Constant<KeyType>, "Non constant key iterator for constant map");
		static_assert(Constant<ValueType>, "Non constant value iterator for constant map");
		return ForEachPairInner<ReturnType, KeyType, ValueType, true>(
			pcForward<FUNCTION>(call));
	}

	/// Constant iteration																		
	template<class RETURN, RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE, bool REVERSE>
	pcptr Map::ForEachPairInner(TFunctor<RETURN(KEY, VALUE)>&& call) {
		if (!mCount || !mKeys.InterpretsAs<KEY>() || !InterpretsAs<VALUE>())
			return 0;

		constexpr bool HasBreaker = Same<bool, RETURN>;
		const auto count = GetCount();
		pcptr index = 0;
		while (index < count) {
			if constexpr (Dense<VALUE>) {
				// Value iterator is dense												
				if constexpr (Dense<KEY>) {
					// Key iterator is dense											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<KEY>(i), As<VALUE>(i)))
								return index + 1;
						}
						else {
							call(mKeys.As<KEY>(i), As<VALUE>(i));
						}
					}
					else {
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<KEY>(index), As<VALUE>(index)))
								return index + 1;
						}
						else {
							call(mKeys.As<KEY>(index), As<VALUE>(index));
						}
					}
				}
				else {
					// Key iterator is sparse											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						auto keyPointer = mKeys.As<KEY>(i);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, As<VALUE>(i)))
								return index + 1;
						}
						else {
							call(keyPointer, As<VALUE>(i));
						}
					}
					else {
						auto keyPointer = mKeys.As<KEY>(index);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, As<VALUE>(index)))
								return index + 1;
						}
						else {
							call(keyPointer, As<VALUE>(index));
						}
					}
				}
			}
			else {
				// Value iterator is sparse											
				if constexpr (Dense<KEY>) {
					// Key iterator is dense											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						auto valuePointer = As<VALUE>(i);
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<KEY>(i), valuePointer))
								return index + 1;
						}
						else {
							call(mKeys.As<KEY>(i), valuePointer);
						}
					}
					else {
						auto valuePointer = As<VALUE>(index);
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<KEY>(index), valuePointer))
								return index + 1;
						}
						else {
							call(mKeys.As<KEY>(index), valuePointer);
						}
					}
				}
				else {
					// Key iterator is sparse											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						auto keyPointer = mKeys.As<KEY>(i);
						auto valuePointer = As<VALUE>(i);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, valuePointer))
								return index + 1;
						}
						else {
							call(keyPointer, valuePointer);
						}
					}
					else {
						auto keyPointer = mKeys.As<KEY>(index);
						auto valuePointer = As<VALUE>(index);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, valuePointer))
								return index + 1;
						}
						else {
							call(keyPointer, valuePointer);
						}
					}
				}
			}

			++index;
		}

		return index;
	}

	/// Constant iteration																		
	template<class RETURN, RTTI::ReflectedData KEY, RTTI::ReflectedData VALUE, bool REVERSE>
	pcptr Map::ForEachPairInner(TFunctor<RETURN(KEY, VALUE)>&& call) const {
		return const_cast<Map*>(this)->ForEachPairInner<RETURN, KEY, VALUE, REVERSE>(pcForward<decltype(call)>(call));
	}

} // namespace Langulus::Anyness