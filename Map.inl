///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
namespace Langulus::Anyness
{

	/// Get the keys container (constant)													
	inline const Any& Map::GetKeys() const noexcept {
		return mKeys;
	}

	/// Get the keys container																	
	inline Any& Map::GetKeys() noexcept {
		return mKeys;
	}

	/// Get the values container																
	inline const Any& Map::GetValues() const noexcept {
		return mValues;
	}

	/// Get the values container (constant)												
	inline Any& Map::GetValues() noexcept {
		return mValues;
	}

	/// Get the values container (constant)												
	constexpr const Count& Map::GetCount() const noexcept {
		return mValues.GetCount();
	}

	/// Check if map is empty																	
	constexpr bool Map::IsEmpty() const noexcept {
		return GetCount() == 0;
	}

	/// Get the raw data inside the value container										
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr Byte* Map::GetRaw() noexcept {
		return mValues.GetRaw();
	}

	/// Get the raw data inside the value container (const)							
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr const Byte* Map::GetRaw() const noexcept {
		return mValues.GetRaw();
	}

	/// Get the end raw data pointer inside the value container						
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr Byte* Map::GetRawEnd() noexcept {
		return mValues.GetRawEnd();
	}

	/// Get the end raw data pointer inside the value container (const)			
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr const Byte* Map::GetRawEnd() const noexcept {
		return mValues.GetRawEnd();
	}

	/// Create a strictly typed container, using templates							
	template<CT::Data KEY, CT::Data VALUE>
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
		return mValues.GetType();
	}

	/// Get the index of a key																	
	template<CT::Data KEY>
	Index Map::FindKey(const KEY& key) const {
		return mKeys.Find<KEY>(key);
	}

	/// Get the index of a value																
	template<CT::Data VALUE>
	Index Map::FindValue(const VALUE& value) const {
		return mValues.Find<VALUE>(value);
	}

	/// Get pair at a special index															
	template<CT::Data KEY, CT::Data VALUE>
	auto Map::GetPair(const Index& index) {
		return GetPair<KEY, VALUE>(mKeys.ConstrainMore<KEY>(index).GetOffset());
	}

	template<CT::Data KEY, CT::Data VALUE>
	auto Map::GetPair(const Index& index) const {
		return const_cast<Map*>(this)->GetPair<KEY, VALUE>(index);
	}

	/// Get pair at a raw index																
	template<CT::Data KEY, CT::Data VALUE>
	auto Map::GetPair(const Offset index) {
		return TPair<decltype(GetKey<KEY>(index)), decltype(GetValue<VALUE>(index))>{
			GetKey<KEY>(index), 
			GetValue<VALUE>(index)
		};
	}

	/// Get pair at a raw index (const)														
	template<CT::Data KEY, CT::Data VALUE>
	auto Map::GetPair(const Offset index) const {
		return const_cast<Map*>(this)->GetPair<KEY, VALUE>(index);
	}

	/// Check if a KEY VALUE pair is insertable to map									
	///	@return true if pair is insertable												
	template<CT::Data KEY, CT::Data VALUE>
	bool Map::IsMapInsertable() {
		if (mKeys.IsUntyped())
			mKeys.SetType<KEY, false>();
		if (mValues.IsUntyped())
			mValues.SetType<VALUE, false>();
		return mKeys.IsInsertable<KEY>() && mValues.IsInsertable<VALUE>();
	}

	
	/// Get the key by special index (const)												
	///	@param idx - the index																
	///	@return a constant reference to the key										
	template<CT::Data T>
	decltype(auto) Map::GetKey(const Index& idx) const {
		return mKeys.As<T>(idx);
	}

	/// Get the key by special index															
	///	@param idx - the index																
	///	@return a reference to the key													
	template<CT::Data T>
	decltype(auto) Map::GetKey(const Index& idx) {
		return mKeys.As<T>(idx);
	}

	/// Get a key by simple index (const)													
	///	@param idx - the index																
	///	@return a reference to the key													
	template<CT::Data T>
	decltype(auto) Map::GetKey(const Offset idx) const {
		return mKeys.As<T>(idx);
	}

	/// Get a key by simple index																
	///	@param idx - the index																
	///	@return a constant reference to the key										
	template<CT::Data T>
	decltype(auto) Map::GetKey(const Offset idx) {
		return mKeys.As<T>(idx);
	}

	/// Get the value by special index (const)											
	///	@param idx - the index																
	///	@return a constant reference to the value										
	template<CT::Data T>
	decltype(auto) Map::GetValue(const Index& idx) const {
		return mValues.As<T>(idx);
	}

	/// Get the value by special index														
	///	@param idx - the index																
	///	@return a reference to the value													
	template<CT::Data T>
	decltype(auto) Map::GetValue(const Index& idx) {
		return mValues.As<T>(idx);
	}

	/// Get a value by simple index (const)												
	///	@param idx - the index																
	///	@return a constant reference to the value										
	template<CT::Data T>
	decltype(auto) Map::GetValue(const Offset idx) const {
		return mValues.As<T>(idx);
	}

	/// Get a value by simple index															
	///	@param idx - the index																
	///	@return a reference to the value													
	template<CT::Data T>
	decltype(auto) Map::GetValue(const Offset idx) {
		return mValues.As<T>(idx);
	}

	/// Move-insert anything compatible to container									
	template<CT::Data K, CT::Data V>
	Count Map::Insert(TPair<K, V>&& item, const Index& index) {
		if (!IsMapInsertable<K, V>())
			Throw<Except::Move>("Bad emplace in map - pair is not insertable");

		mKeys.Insert<Any, true, false>(Move(item.mKey), index);
		mValues.Insert<Any, true, false>(Move(item.mValue), index);
		return 1;
	}

	/// Copy-insert anything compatible to container									
	template<CT::Data K, CT::Data V>
	Count Map::Insert(const TPair<K, V>* items, const Count& count, const Index& index) {
		if (!IsMapInsertable<K, V>())
			Throw<Except::Copy>("Bad insert in map");

		Count insertedKeys = 0;
		Count insertedValues = 0;
		for (Count i = 0; i < count; ++i) {
			insertedKeys += mKeys.Insert<Any, true, false>(&items[i].mKey, 1, index);
			insertedValues += mValues.Insert<Any, true, false>(&items[i].mValue, 1, index);
		}

		return insertedKeys;
	}

	/// Push any data at the back																
	template<CT::Data K, CT::Data V>
	Map& Map::operator << (const TPair<K, V>& other) {
		Insert<K, V>(&other, 1, Index::Back);
		return *this;
	}

	template<CT::Data K, CT::Data V>
	Map& Map::operator << (TPair<K, V>&& other) {
		Insert<K, V>(Forward<TPair<K, V>>(other), Index::Back);
		return *this;
	}

	/// Push any data at the front															
	template<CT::Data K, CT::Data V>
	Map& Map::operator >> (const TPair<K, V>& other) {
		Insert<K, V>(&other, 1, Index::Front);
		return *this;
	}

	template<CT::Data K, CT::Data V>
	Map& Map::operator >> (TPair<K, V>&& other) {
		Insert<K, V>(Forward<TPair<K, V>>(other), Index::Front);
		return *this;
	}

	/// Iteration																					
	template<class F>
	Count Map::ForEachPair(F&& call) {
		using P = decltype(GetLambdaArguments(&F::operator()));
		using K = typename P::Key;
		using V = typename P::Value;
		using R = decltype(call(std::declval<K>(), std::declval<V>()));
		return ForEachPairInner<R, K, V, false>(Forward<F>(call));
	}

	template<class F>
	Count Map::ForEachPairRev(F&& call) {
		using P = decltype(GetLambdaArguments(&F::operator()));
		using K = typename P::Key;
		using V = typename P::Value;
		using R = decltype(call(std::declval<K>(), std::declval<V>()));
		return ForEachPairInner<R, K, V, true>(Forward<F>(call));
	}

	template<class F>
	Count Map::ForEachPair(F&& call) const {
		using P = decltype(GetLambdaArguments(&F::operator()));
		using K = typename P::Key;
		using V = typename P::Value;
		using R = decltype(call(std::declval<K>(), std::declval<V>()));
		static_assert(CT::Constant<K>, "Non constant key iterator for constant map");
		static_assert(CT::Constant<V>, "Non constant value iterator for constant map");
		return ForEachPairInner<R, K, V, false>(Forward<F>(call));
	}

	template<class F>
	Count Map::ForEachPairRev(F&& call) const {
		using P = decltype(GetLambdaArguments(&F::operator()));
		using K = typename P::KeyType;
		using V = typename P::ValueType;
		using R = decltype(call(std::declval<K>(), std::declval<V>()));
		static_assert(CT::Constant<K>, "Non constant key iterator for constant map");
		static_assert(CT::Constant<V>, "Non constant value iterator for constant map");
		return ForEachPairInner<R, K, V, true>(Forward<F>(call));
	}

	/// Constant iteration																		
	template<class R, CT::Data K, CT::Data V, bool REVERSE>
	Count Map::ForEachPairInner(TFunctor<R(K, V)>&& call) {
		if (IsEmpty() || !mKeys.CastsTo<K>() || !mValues.CastsTo<V>())
			return 0;

		constexpr bool HasBreaker = CT::Same<bool, R>;
		const auto count = GetCount();
		Count index = 0;
		while (index < count) {
			if constexpr (CT::Dense<V>) {
				// Value iterator is dense												
				if constexpr (CT::Dense<K>) {
					// Key iterator is dense											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<K>(i), mValues.As<V>(i)))
								return index + 1;
						}
						else call(mKeys.As<K>(i), mValues.As<V>(i));
					}
					else {
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<K>(index), mValues.As<V>(index)))
								return index + 1;
						}
						else call(mKeys.As<K>(index), mValues.As<V>(index));
					}
				}
				else {
					// Key iterator is sparse											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						auto keyPointer = mKeys.As<K>(i);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, mValues.As<V>(i)))
								return index + 1;
						}
						else call(keyPointer, mValues.As<V>(i));
					}
					else {
						auto keyPointer = mKeys.As<K>(index);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, mValues.As<V>(index)))
								return index + 1;
						}
						else call(keyPointer, mValues.As<V>(index));
					}
				}
			}
			else {
				// Value iterator is sparse											
				if constexpr (CT::Dense<K>) {
					// Key iterator is dense											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						auto valuePointer = mValues.As<V>(i);
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<K>(i), valuePointer))
								return index + 1;
						}
						else call(mKeys.As<K>(i), valuePointer);
					}
					else {
						auto valuePointer = mValues.As<V>(index);
						if constexpr (HasBreaker) {
							if (!call(mKeys.As<K>(index), valuePointer))
								return index + 1;
						}
						else call(mKeys.As<K>(index), valuePointer);
					}
				}
				else {
					// Key iterator is sparse											
					if constexpr (REVERSE) {
						const auto i = count - index - 1;
						auto keyPointer = mKeys.As<K>(i);
						auto valuePointer = mValues.As<V>(i);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, valuePointer))
								return index + 1;
						}
						else call(keyPointer, valuePointer);
					}
					else {
						auto keyPointer = mKeys.As<K>(index);
						auto valuePointer = mValues.As<V>(index);
						if constexpr (HasBreaker) {
							if (!call(keyPointer, valuePointer))
								return index + 1;
						}
						else call(keyPointer, valuePointer);
					}
				}
			}

			++index;
		}

		return index;
	}

	/// Constant iteration																		
	template<class R, CT::Data K, CT::Data V, bool REVERSE>
	Count Map::ForEachPairInner(TFunctor<R(K, V)>&& call) const {
		return const_cast<Map*>(this)->ForEachPairInner<R, K, V, REVERSE>(
			Forward<decltype(call)>(call)
		);
	}

} // namespace Langulus::Anyness
