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
	template<ReflectedData KEY, ReflectedData VALUE>
	Map Map::From(const DataState& state) noexcept {
		return {
			Block {state, MetaData::Of<KEY>()},
			Block {state, MetaData::Of<VALUE>()}
		};
	}

	/// Get the key type																			
	inline DMeta Map::KeyType() const {
		return mKeys.GetType();
	}

	/// Get the value type																		
	inline DMeta Map::ValueType() const {
		return Any::GetType();
	}

	/// Get the index of a key																	
	template<ReflectedData KEY>
	Index Map::FindKey(const KEY& key) const {
		return mKeys.Find<KEY>(key);
	}

	/// Get the index of a value																
	template<ReflectedData VALUE>
	Index Map::FindValue(const VALUE& value) const {
		return Any::Find<VALUE>(value);
	}

	/// Get pair at a special index															
	template<ReflectedData KEY, ReflectedData VALUE>
	auto Map::GetPair(const Index& index) {
		return GetPair<KEY, VALUE>(mKeys.ConstrainMore<KEY>(index).GetOffset());
	}

	template<ReflectedData KEY, ReflectedData VALUE>
	auto Map::GetPair(const Index& index) const {
		return const_cast<Map*>(this)->GetPair<KEY, VALUE>(index);
	}

	/// Get pair at a raw index																
	template<ReflectedData KEY, ReflectedData VALUE>
	auto Map::GetPair(const Offset index) {
		return TPair<decltype(GetKey<KEY>(index)), decltype(GetValue<VALUE>(index))>{
			GetKey<KEY>(index), 
			GetValue<VALUE>(index)
		};
	}

	/// Get pair at a raw index (const)														
	template<ReflectedData KEY, ReflectedData VALUE>
	auto Map::GetPair(const Offset index) const {
		return const_cast<Map*>(this)->GetPair<KEY, VALUE>(index);
	}

	template<ReflectedData KEY, ReflectedData VALUE>
	bool Map::IsMapInsertable() {
		if (mKeys.IsUntyped())
			mKeys.SetType<KEY>(false);
		if (Any::IsUntyped())
			Any::SetType<VALUE>(false);
		return mKeys.IsInsertable<KEY>() && Any::IsInsertable<VALUE>();
	}

	
	/// Get the key by special index (const)												
	///	@param idx - the index																
	///	@return a constant reference to the key										
	template<ReflectedData T>
	decltype(auto) Map::GetKey(const Index& idx) const {
		return Keys().As<T>(idx);
	}

	/// Get the key by special index															
	///	@param idx - the index																
	///	@return a reference to the key													
	template<ReflectedData T>
	decltype(auto) Map::GetKey(const Index& idx) {
		return Keys().As<T>(idx);
	}

	/// Get a key by simple index (const)													
	///	@param idx - the index																
	///	@return a reference to the key													
	template<ReflectedData T>
	decltype(auto) Map::GetKey(const Offset idx) const {
		return Keys().As<T>(idx);
	}

	/// Get a key by simple index																
	///	@param idx - the index																
	///	@return a constant reference to the key										
	template<ReflectedData T>
	decltype(auto) Map::GetKey(const Offset idx) {
		return Keys().As<T>(idx);
	}

	/// Get the value by special index (const)											
	///	@param idx - the index																
	///	@return a constant reference to the value										
	template<ReflectedData T>
	decltype(auto) Map::GetValue(const Index& idx) const {
		return Values().As<T>(idx);
	}

	/// Get the value by special index														
	///	@param idx - the index																
	///	@return a reference to the value													
	template<ReflectedData T>
	decltype(auto) Map::GetValue(const Index& idx) {
		return Values().As<T>(idx);
	}

	/// Get a value by simple index (const)												
	///	@param idx - the index																
	///	@return a constant reference to the value										
	template<ReflectedData T>
	decltype(auto) Map::GetValue(const Offset idx) const {
		return Values().As<T>(idx);
	}

	/// Get a value by simple index															
	///	@param idx - the index																
	///	@return a reference to the value													
	template<ReflectedData T>
	decltype(auto) Map::GetValue(const Offset idx) {
		return Values().As<T>(idx);
	}

	/// Emplace anything compatible to container											
	template<ReflectedData KEY, ReflectedData VALUE>
	Count Map::Emplace(TPair<KEY, VALUE>&& item, const Index& index) {
		if (!IsMapInsertable<KEY, VALUE>())
			throw Except::Move("Bad emplace in map - type not insertable");

		const auto insertedKeys = mKeys.Emplace<KEY>(Move(item.Key), index);
		const auto insertedValues = Any::Emplace<VALUE>(Move(item.Value), index);
		if (insertedKeys != insertedValues)
			throw Except::Move("Bad emplace in map - move failed");

		return insertedKeys;
	}

	/// Insert anything compatible to container											
	template<ReflectedData KEY, ReflectedData VALUE>
	Count Map::Insert(const TPair<KEY, VALUE>* items, const Count count, const Index& index) {
		if (!IsMapInsertable<KEY, VALUE>())
			throw Except::Copy("Bad insert in map");

		Count insertedKeys = 0;
		Count insertedValues = 0;
		for (Count i = 0; i < count; ++i) {
			insertedKeys += mKeys.Insert<KEY>(&items[i].Key, 1, index);
			insertedValues += Any::Insert<VALUE>(&items[i].Value, 1, index);
		}

		return insertedKeys;
	}

	/// Push any data at the back																
	template<ReflectedData KEY, ReflectedData VALUE>
	Map& Map::operator << (const TPair<KEY, VALUE>& other) {
		Insert<KEY, VALUE>(&other, 1, Index::Back);
		return *this;
	}

	template<ReflectedData KEY, ReflectedData VALUE>
	Map& Map::operator << (TPair<KEY, VALUE>&& other) {
		Emplace<KEY, VALUE>(Forward<TPair<KEY, VALUE>>(other), Index::Back);
		return *this;
	}

	/// Push any data at the front															
	template<ReflectedData KEY, ReflectedData VALUE>
	Map& Map::operator >> (const TPair<KEY, VALUE>& other) {
		Insert<KEY, VALUE>(&other, 1, Index::Front);
		return *this;
	}

	template<ReflectedData KEY, ReflectedData VALUE>
	Map& Map::operator >> (TPair<KEY, VALUE>&& other) {
		Emplace<KEY, VALUE>(Forward<TPair<KEY, VALUE>>(other), Index::Front);
		return *this;
	}

	/// Iteration																					
	template<class FUNCTION>
	Count Map::ForEachPair(FUNCTION&& call) {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		return ForEachPairInner<ReturnType, KeyType, ValueType, false>(
			pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	Count Map::ForEachPairRev(FUNCTION&& call) {
		using PairType = decltype(GetLambdaArguments(&FUNCTION::operator()));
		using KeyType = typename PairType::KeyType;
		using ValueType = typename PairType::ValueType;
		using ReturnType = decltype(call(std::declval<KeyType>(), std::declval<ValueType>()));
		return ForEachPairInner<ReturnType, KeyType, ValueType, true>(
			pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	Count Map::ForEachPair(FUNCTION&& call) const {
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
	Count Map::ForEachPairRev(FUNCTION&& call) const {
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
	template<class RETURN, ReflectedData KEY, ReflectedData VALUE, bool REVERSE>
	Count Map::ForEachPairInner(TFunctor<RETURN(KEY, VALUE)>&& call) {
		if (!mCount || !mKeys.InterpretsAs<KEY>() || !InterpretsAs<VALUE>())
			return 0;

		constexpr bool HasBreaker = Same<bool, RETURN>;
		const auto count = GetCount();
		Count index = 0;
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
	template<class RETURN, ReflectedData KEY, ReflectedData VALUE, bool REVERSE>
	Count Map::ForEachPairInner(TFunctor<RETURN(KEY, VALUE)>&& call) const {
		return const_cast<Map*>(this)->ForEachPairInner<RETURN, KEY, VALUE, REVERSE>(Forward<decltype(call)>(call));
	}

} // namespace Langulus::Anyness
