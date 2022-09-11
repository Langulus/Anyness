///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "UnorderedMap.hpp"
#include "TAny.hpp"

namespace Langulus::Anyness
{

	/// Manual construction via an initializer list										
	///	@param initlist - the initializer list to forward							
	template<CT::Data K, CT::Data V>
	UnorderedMap::UnorderedMap(::std::initializer_list<TPair<K, V>> initlist)
		: UnorderedMap {} {
		Mutate<K, V>();
		Allocate(initlist.size());
		for (auto& it : initlist)
			Insert(*it);
	}

	/// Shallow-copy construction																
	///	@param other - the table to copy													
	inline UnorderedMap::UnorderedMap(const UnorderedMap& other)
		: mKeys {Disown(other.mKeys)}
		, mInfo {other.mInfo}
		, mValues {other.mValues} {}

	/// Move construction																		
	///	@param other - the table to move													
	inline UnorderedMap::UnorderedMap(UnorderedMap&& other) noexcept
		: mKeys {Move(other.mKeys)}
		, mInfo {other.mInfo}
		, mValues {Move(other.mValues)} {}

	/// Shallow-copy construction without referencing									
	///	@param other - the disowned table to copy										
	inline UnorderedMap::UnorderedMap(Disowned<UnorderedMap>&& other) noexcept
		: mKeys {Disown(other.mValue.mKeys)}
		, mInfo {other.mValue.mInfo}
		, mValues {Disown(other.mValue.mValues)} {}

	/// Minimal move construction from abandoned table									
	///	@param other - the abandoned table to move									
	inline UnorderedMap::UnorderedMap(Abandoned<UnorderedMap>&& other) noexcept
		: mKeys {Abandon(other.mValue.mKeys)}
		, mInfo {other.mValue.mInfo}
		, mValues {Abandon(other.mValue.mValues)} {}

	/// Destroys the map and all it's contents											
	inline UnorderedMap::~UnorderedMap() {
		if (!mValues.mEntry)
			return;

		if (mValues.mEntry->GetUses() == 1) {
			// Remove all used keys and values, they're used only here		
			ClearInner();

			// Deallocate stuff															
			Allocator::Deallocate(mKeys.mEntry);
			Allocator::Deallocate(mValues.mEntry);
		}
		else {
			// Data is used from multiple locations, just deref values		
			// Notice how we don't dereference keys, since we use only the	
			// values' references to save on some redundancy					
			mValues.mEntry->Free();
		}

		mKeys.mEntry = nullptr;
		mValues.mEntry = nullptr;
	}

	/// Checks if both tables contain the same entries									
	/// Order is irrelevant																		
	///	@param other - the table to compare against									
	///	@return true if tables match														
	inline bool UnorderedMap::operator == (const UnorderedMap& other) const {
		if (other.GetCount() != GetCount())
			return false;

		const auto keyEnd = mKeys.mRaw + mKeys.GetByteSize();
		auto key = mKeys.mRaw;
		auto info = GetInfo();
		while (key != keyEnd) {
			if (!*(info++)) {
				key += mKeys.GetStride();
				continue;
			}

			const auto rhs = other.FindIndex(*key);
			if (rhs == other.GetReserved() || GetValue(key - mKeys.mRaw) != other.GetValue(rhs))
				return false;

			++key; ++info;
		}

		return true;
	}

	/// Move a table																				
	///	@param rhs - the table to move													
	///	@return a reference to this table												
	inline UnorderedMap& UnorderedMap::operator = (UnorderedMap&& rhs) noexcept {
		if (&rhs == this)
			return *this;

		Reset();
		new (this) UnorderedMap {Forward<UnorderedMap>(rhs)};
		return *this;
	}

	/// Creates a shallow copy of the given table										
	///	@param rhs - the table to reference												
	///	@return a reference to this table												
	inline UnorderedMap& UnorderedMap::operator = (const UnorderedMap& rhs) {
		if (&rhs == this)
			return *this;

		Reset();
		new (this) UnorderedMap {rhs};
		return *this;
	}

	/// Emplace a single pair into a cleared map											
	///	@param pair - the pair to emplace												
	///	@return a reference to this table												
	template<CT::Data K, CT::Data V>
	UnorderedMap& UnorderedMap::operator = (TPair<K, V>&& pair) noexcept {
		Clear();
		Insert(Move(pair.mKey), Move(pair.mValue));
		return *this;
	}

	/// Insert a single pair into a cleared map											
	///	@param pair - the pair to copy													
	///	@return a reference to this table												
	template<CT::Data K, CT::Data V>
	UnorderedMap& UnorderedMap::operator = (const TPair<K, V>& pair) {
		Clear();
		Insert(pair.mKey, pair.mValue);
		return *this;
	}

	/// Clone all elements in a range														
	///	@param count - number of relevant elements to clone						
	///	@param info - info bytes for checking valid entries						
	///	@param from - start of the elements to copy									
	///	@param fromEnd - end of the elements to copy									
	///	@param to - destrination memory													
	template<class T>
	void UnorderedMap::CloneInner(const Count& count, const InfoType* info, const T* from, const T* fromEnd, T* to) {
		using TD = Decay<T>;

		if constexpr (CT::Sparse<T>) {
			TAny<TD> coalesced;
			coalesced.Allocate(count);

			// Clone data behind each valid pointer								
			auto cache = coalesced.GetRaw();
			while (from < fromEnd) {
				if (!*info) {
					// Skip uninitialized pointers									
					++from; ++to; ++info;
					continue;
				}
				else if (!*from) {
					// Skip zero pointers												
					*to = nullptr;
					++from; ++to; ++info;
					continue;
				}
				
				if constexpr (CT::Clonable<T>)
					new (cache) TD {(*from)->Clone()};
				else if constexpr (CT::POD<T>)
					::std::memcpy(cache, **from, sizeof(TD));
				else
					new (cache) TD {**from};

				*to = cache;
				++from; ++to; ++cache; ++info;
			}
			
			coalesced.Reference(cache - coalesced.GetRaw());
		}
		else if constexpr (CT::Clonable<T>) {
			// Clone dense keys by their Clone() methods							
			while (from < fromEnd) {
				if (*info)
					new (to) TD {from->Clone()};
				++from; ++to; ++info;
			}
		}
		else if constexpr (CT::POD<T>) {
			// Batch clone dense data at once										
			::std::memcpy(to, from, (fromEnd - from) * sizeof(T));
		}
		else {
			// As a fallback, just do shallow copies								
			while (from < fromEnd) {
				if (*info)
					new (to) TD {*from};
				++from; ++to; ++info;
			}
		}
	}

	/// Clone the table																			
	///	@return the new table																
	inline UnorderedMap UnorderedMap::Clone() const {
		if (IsEmpty())
			return {};

		UnorderedMap result {Disown(*this)};

		// Allocate keys and info														
		result.mKeys.mEntry = Allocator::Allocate(mKeys.mEntry->GetAllocatedSize());
		if (!result.mKeys.mEntry) {
			Throw<Except::Allocate>(
				"Out of memory on cloning UnorderedMap keys", 
				LANGULUS_LOCATION());
		}

		// Allocate values																
		result.mValues.mEntry = Allocator::Allocate(result.mValues.GetReservedSize());
		if (!result.mValues.mEntry) {
			Allocator::Deallocate(result.mKeys.mEntry);
			result.mValues.mEntry = nullptr;
			Throw<Except::Allocate>(
				"Out of memory on cloning UnorderedMap values"
				LANGULUS_LOCATION());
		}

		// Clone the info bytes															
		result.mKeys.mRaw = result.mKeys.mEntry->GetBlockStart();
		result.mInfo = reinterpret_cast<InfoType*>(result.mKeys.mRaw)
			+ (mInfo - reinterpret_cast<const InfoType*>(mKeys.mRaw));
		::std::memcpy(result.GetInfo(), GetInfo(), GetReserved() + 1);

		// Clone or shallow-copy the keys											
		CloneInner(result.mValues.mCount, GetInfo(),
			mKeys.GetRaw(), mKeys.GetRawEnd(), result.mKeys.GetRaw());

		// Clone or shallow-copy the values											
		result.mValues.mRaw = result.mValues.mEntry->GetBlockStart();
		CloneInner(result.mValues.mCount, GetInfo(), 
			mValues.GetRaw(), mValues.GetRawEnd(), result.mValues.GetRaw());

		return Abandon(result);
	}
	
	/// Templated tables are always typed													
	///	@return false																			
	constexpr bool UnorderedMap::IsKeyUntyped() const noexcept {
		return mKeys.IsUntyped();
	}
	
	/// Templated tables are always typed													
	///	@return false																			
	constexpr bool UnorderedMap::IsValueUntyped() const noexcept {
		return mValues.IsUntyped();
	}
	
	/// Templated tables are always type-constrained									
	///	@return true																			
	constexpr bool UnorderedMap::IsKeyTypeConstrained() const noexcept {
		return mKeys.IsTypeConstrained();;
	}
	
	/// Templated tables are always type-constrained									
	///	@return true																			
	constexpr bool UnorderedMap::IsValueTypeConstrained() const noexcept {
		return mValues.IsTypeConstrained();;
	}
	
	/// Check if key type is abstract														
	constexpr bool UnorderedMap::IsKeyAbstract() const noexcept {
		return mKeys.IsAbstract() && mKeys.IsDense();
	}
	
	/// Check if value type is abstract														
	constexpr bool UnorderedMap::IsValueAbstract() const noexcept {
		return mValues.IsAbstract() && mKeys.IsDense();
	}
	
	/// Check if key type is default-constructible										
	constexpr bool UnorderedMap::IsKeyConstructible() const noexcept {
		return mKeys.IsDefaultable();
	}
	
	/// Check if value type is default-constructible									
	constexpr bool UnorderedMap::IsValueConstructible() const noexcept {
		return mValues.IsDefaultable();
	}
	
	/// Check if key type is deep																
	constexpr bool UnorderedMap::IsKeyDeep() const noexcept {
		return mKeys.IsDeep();
	}
	
	/// Check if value type is deep															
	constexpr bool UnorderedMap::IsValueDeep() const noexcept {
		return mValues.IsDeep();
	}

	/// Check if the key type is a pointer													
	constexpr bool UnorderedMap::IsKeySparse() const noexcept {
		return mKeys.IsSparse();
	}
	
	/// Check if the value type is a pointer												
	constexpr bool UnorderedMap::IsValueSparse() const noexcept {
		return mValues.IsSparse();
	}

	/// Check if the key type is not a pointer											
	constexpr bool UnorderedMap::IsKeyDense() const noexcept {
		return mKeys.IsDense();
	}

	/// Check if the value type is not a pointer											
	constexpr bool UnorderedMap::IsValueDense() const noexcept {
		return mValues.IsDense();
	}

	/// Get the size of a single key, in bytes											
	///	@return the number of bytes a single key contains							
	constexpr Size UnorderedMap::GetKeyStride() const noexcept {
		return mKeys.GetStride();
	}
	
	/// Get the size of a single value, in bytes											
	///	@return the number of bytes a single value contains						
	constexpr Size UnorderedMap::GetValueStride() const noexcept {
		return mValues.GetStride();
	}

	/// Get the raw key array (const)														
	template<CT::Data K>
	constexpr decltype(auto) UnorderedMap::GetRawKeys() const noexcept {
		return reinterpret_cast<const TAny<K>&>(mKeys).GetRaw();
	}

	/// Get the raw key array																	
	template<CT::Data K>
	constexpr decltype(auto) UnorderedMap::GetRawKeys() noexcept {
		return reinterpret_cast<TAny<K>&>(mKeys).GetRaw();
	}

	/// Get the end of the raw key array													
	template<CT::Data K>
	constexpr decltype(auto) UnorderedMap::GetRawKeysEnd() const noexcept {
		return GetRawKeys<K>() + GetReserved();
	}

	/// Get the raw value array (const)														
	template<CT::Data V>
	constexpr decltype(auto) UnorderedMap::GetRawValues() const noexcept {
		return reinterpret_cast<const TAny<V>&>(mValues).GetRaw();
	}

	/// Get the raw value array																
	template<CT::Data V>
	constexpr decltype(auto) UnorderedMap::GetRawValues() noexcept {
		return reinterpret_cast<TAny<V>&>(mValues).GetRaw();
	}

	/// Get end of the raw value array														
	template<CT::Data V>
	constexpr decltype(auto) UnorderedMap::GetRawValuesEnd() const noexcept {
		return GetRawValues<V>() + GetReserved();
	}

	#ifdef LANGULUS_ENABLE_TESTING
		/// Get raw key memory pointer, used only in testing							
		///	@return the pointer																
		constexpr const void* UnorderedMap::GetRawKeysMemory() const noexcept {
			return mKeys.mRaw;
		}

		/// Get raw value memory pointer, used only in testing						
		///	@return the pointer																
		constexpr const void* UnorderedMap::GetRawValuesMemory() const noexcept {
			return mValues.mRaw;
		}
	#endif

	/// Get the size of all pairs, in bytes												
	///	@return the total amount of initialized bytes								
	constexpr Size UnorderedMap::GetByteSize() const noexcept {
		return sizeof(Pair) * GetCount(); 
	}

	/// Get the key meta data																	
	inline DMeta UnorderedMap::GetKeyType() const noexcept {
		return mKeys.GetType();
	}

	/// Get the value meta data																
	inline DMeta UnorderedMap::GetValueType() const noexcept {
		return mValues.GetType();
	}

	/// Check if key type exactly matches another										
	template<class ALT_K>
	constexpr bool UnorderedMap::KeyIs() const noexcept {
		return mKeys.Is<ALT_K>();
	}

	/// Check if value type exactly matches another										
	template<class ALT_V>
	constexpr bool UnorderedMap::ValueIs() const noexcept {
		return mValues.Is<ALT_V>();
	}

	/// Request a new size of keys and info via the value container				
	/// The memory layout is:																	
	///	[keys for each bucket]																
	///			[padding for alignment]														
	///					[info for each bucket]												
	///							[one sentinel byte for terminating loops]				
	///	@attention assumes key type has been set										
	///	@param infoStart - [out] the offset at which info bytes start			
	///	@return the requested byte size													
	inline Size UnorderedMap::RequestKeyAndInfoSize(const Count request, Offset& infoStart) noexcept {
		const Size keymemory = request * mKeys.GetStride();
		infoStart = keymemory + Alignment - (keymemory % Alignment);
		return infoStart + request + 1;
	}

	/// Get the info array (const)															
	///	@return a pointer to the first element inside the info array			
	inline const UnorderedMap::InfoType* UnorderedMap::GetInfo() const noexcept {
		return mInfo;
	}

	/// Get the info array																		
	///	@return a pointer to the first element inside the info array			
	inline UnorderedMap::InfoType* UnorderedMap::GetInfo() noexcept {
		return mInfo;
	}

	/// Get the end of the info array														
	///	@return a pointer to the first element inside the info array			
	inline const UnorderedMap::InfoType* UnorderedMap::GetInfoEnd() const noexcept {
		return mInfo + GetReserved();
	}

	/// Checks type compatibility and sets type for the type-erased map			
	///	@tparam K - the key type															
	///	@tparam V - the value type															
	template<CT::Data K, CT::Data V>
	void UnorderedMap::Mutate() {
		Mutate(
			MetaData::Of<Decay<K>>(), CT::Sparse<K>, 
			MetaData::Of<Decay<V>>(), CT::Sparse<V>
		);
	}

	/// Checks type compatibility and sets type for the type-erased map			
	///	@param key - the key type															
	///	@param sparseKey - whether key type is sparse								
	///	@param value - the value type														
	///	@param sparseValue - whether value type is sparse							
	inline void UnorderedMap::Mutate(DMeta key, bool sparseKey, DMeta value, bool sparseValue) {
		if (!mKeys.mType) {
			// Set a fresh key type														
			mKeys.mType = key;
			if (sparseKey)
				mKeys.MakeSparse();
		}
		else {
			// Key type already set, so check compatibility						
			LANGULUS_ASSERT(
				mKeys.Is(key) && mKeys.IsSparse() == sparseKey,
				Except::Mutate,
				"Attempting to mutate type-erased unordered map's key type"
			);
		}

		if (!mValues.mType) {
			// Set a fresh value type													
			mValues.mType = value;
			if (sparseValue)
				mValues.MakeSparse();
		}
		else {
			// Value type already set, so check compatibility					
			LANGULUS_ASSERT(
				mValues.Is(value) && mValues.IsSparse() == sparseValue,
				Except::Mutate,
				"Attempting to mutate type-erased unordered map's value type"
			);
		}
	}

	/// Reserves space for the specified number of pairs								
	///	@attention does nothing if reserving less than current reserve			
	///	@param count - number of pairs to allocate									
	inline void UnorderedMap::Allocate(const Count& count) {
		AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
	}

	/// Allocate or reallocate key and info array										
	///	@attention assumes count is a power-of-two									
	///	@tparam REUSE - true to reallocate, false to allocate fresh				
	///	@param count - the new number of pairs											
	template<bool REUSE>
	void UnorderedMap::AllocateKeys(const Count& count) {
		LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
			"Table reallocation count is not a power-of-two");

		Offset infoOffset;
		auto oldInfo = mInfo;
		const auto oldCount = GetReserved();
		const auto oldInfoEnd = oldInfo + oldCount;

		// Allocate new keys																
		const Block oldKeys {mKeys};
		const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
		if constexpr (REUSE)
			mKeys.mEntry = Allocator::Reallocate(keyAndInfoSize, mKeys.mEntry);
		else
			mKeys.mEntry = Allocator::Allocate(keyAndInfoSize);

		LANGULUS_ASSERT(mKeys.mEntry, Except::Allocate,
			"Out of memory on allocating/reallocating keys");

		// Allocate new values															
		const Block oldValues {mValues};
		const auto valueByteSize = count * mValues.GetStride();
			if constexpr (REUSE)
			mValues.mEntry = Allocator::Reallocate(valueByteSize, mValues.mEntry);
		else
			mValues.mEntry = Allocator::Allocate(valueByteSize);

		if (!mValues.mEntry) {
			Allocator::Deallocate(mKeys.mEntry);
			Throw<Except::Allocate>(
				"Out of memory on allocating/reallocating values",
				LANGULUS_LOCATION());
		}

		mValues.mRaw = mValues.mEntry->GetBlockStart();
		mValues.mReserved = count;
		mValues.mCount = 0;

		// Precalculate the info pointer, it's costly							
		mKeys.mRaw = mKeys.mEntry->GetBlockStart();
		mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
		// Set the sentinel																
		mInfo[count] = 1;

		// Zero or move the info array												
		if constexpr (REUSE) {
			// Check if keys were reused												
			if (mKeys.mEntry == oldKeys.mEntry) {
				// Keys were reused, but info always moves (null the rest)	
				::std::memmove(mInfo, oldInfo, oldCount);
				::std::memset(mInfo + oldCount, 0, count - oldCount);

				if (mValues.mEntry == oldValues.mEntry) {
					// Both keys and values remain in the same place			
					Rehash(count, oldCount);
					return;
				}
			}
			else ::std::memset(mInfo, 0, count);
		}
		else ::std::memset(mInfo, 0, count);

		if (oldValues.IsEmpty()) {
			// There are no old values, the previous map was empty			
			// Just do an early return right here									
			return;
		}

		// If reached, then keys or values (or both) moved						
		// Reinsert all pairs to rehash												
		auto key = oldKeys.GetElement();
		auto value = oldValues.GetElement();
		while (oldInfo != oldInfoEnd) {
			if (!*(oldInfo++)) {
				key.Next();
				value.Next();
				continue;
			}

			InsertUnknown(Move(key), Move(value));

			if constexpr (REUSE) {
				key.CallUnknownDestructors();
				value.CallUnknownDestructors();
			}

			key.Next();
			value.Next();
		}

		// Free the old allocations													
		if constexpr (REUSE) {
			// When reusing, keys and values can potentially remain same	
			// Avoid deallocating them if that's the case						
			if (oldValues.mEntry != mValues.mEntry)
				Allocator::Deallocate(oldValues.mEntry);
			if (oldKeys.mEntry != mKeys.mEntry)
				Allocator::Deallocate(oldKeys.mEntry);
		}
		else if (oldValues.mEntry) {
			// Not reusing, so either deallocate, or dereference				
			// (keys are always present, if values are present)				
			if (oldValues.mEntry->GetUses() > 1)
				oldValues.mEntry->Free();
			else {
				Allocator::Deallocate(oldValues.mEntry);
				Allocator::Deallocate(oldKeys.mEntry);
			}
		}
	}

	/// Rehashes and reinserts each pair													
	///	@attention assumes count and oldCount are power-of-two					
	///	@param count - the new number of pairs											
	///	@param oldCount - the old number of pairs										
	inline void UnorderedMap::Rehash(const Count& count, const Count& oldCount) {
		LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
			"New count is not a power-of-two");
		LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
			"Old count is not a power-of-two");

		auto oldInfo = GetInfo();
		const auto oldInfoEnd = oldInfo + oldCount;

		// Prepare a set of preallocated swappers									
		Block keyswap {GetKeyType()};
		Block valswap {GetValueType()};
		keyswap.Allocate(1);
		valswap.Allocate(1);

		// For each old existing key...												
		while (oldInfo != oldInfoEnd) {
			if (!*oldInfo) {
				++oldInfo;
				continue;
			}

			// Rehash and check if hashes match										
			const auto oldIndex = oldInfo - GetInfo();
			auto oldKey = mKeys.GetElement(oldIndex);
			const auto newIndex = oldKey.GetHash().mHash & (count - 1);
			if (oldIndex != newIndex) {
				// Move key & value to swapper										
				auto oldValue = mValues.GetElement(oldIndex);
				keyswap.CallUnknownMoveConstructors<false>(1, oldKey);
				valswap.CallUnknownMoveConstructors<false>(1, oldValue);
				keyswap.mCount = valswap.mCount = 1;

				// Clean the old abandoned slots (just in case)					
				oldKey.CallUnknownDestructors();
				oldValue.CallUnknownDestructors();
				*oldInfo = 0;

				// Insert the swapper													
				InsertInnerUnknown<false>(newIndex, Move(keyswap), Move(valswap));
			}
			else {
				// Nothing inserted, but since count has been previously		
				// cleared, restore the count and move forward					
				++mValues.mCount;
			}

			++oldInfo;
		}

		// Free the allocated swapper memory										
		keyswap.Free();
		valswap.Free();
	}

	/// Reserves space for the specified number of pairs								
	///	@attention does nothing if reserving less than current reserve			
	///	@attention assumes count is a power-of-two number							
	///	@param count - number of pairs to allocate									
	inline void UnorderedMap::AllocateInner(const Count& count) {
		// Shrinking is never allowed, you'll have to do it explicitly 	
		// via Compact()																	
		if (count <= GetReserved())
			return;

		// Allocate/Reallocate the keys and info									
		if (IsAllocated() && GetUses() == 1)
			AllocateKeys<true>(count);
		else
			AllocateKeys<false>(count);
	}

	/// Inner insertion function																
	///	@param start - the starting index												
	///	@param key - key to move in														
	///	@param value - value to move in													
	template<bool CHECK_FOR_MATCH, CT::Data K, CT::Data V>
	void UnorderedMap::InsertInner(const Offset& start, K&& key, V&& value) {
		// Get the starting index based on the key hash							
		auto psl = GetInfo() + start;
		const auto pslEnd = GetInfoEnd();
		InfoType attempts {1};
		while (*psl) {
			const auto index = psl - GetInfo();

			if constexpr (CHECK_FOR_MATCH) {
				const auto candidate = GetRawKeys<K>() + index;
				if (*candidate == key) {
					// Neat, the key already exists - just set value and go	
					GetRawValues<V>()[index] = Forward<V>(value);
					return;
				}
			}

			if (attempts > *psl) {
				// The pair we're inserting is closer to bucket, so swap		
				::std::swap(GetRawKeys<K>()[index], key);
				::std::swap(GetRawValues<V>()[index], value);
				::std::swap(attempts, *psl);
			}

			++attempts;

			if (psl < pslEnd - 1) LIKELY() {
				++psl;
			}
			else UNLIKELY() {
				// Wrap around and start from the beginning						
				psl = GetInfo();
			}
		}

		// If reached, empty slot reached, so put the pair there				
		// Might not seem like it, but we gave a guarantee, that this is	
		// eventually reached, unless key exists and returns early			
		const auto index = psl - GetInfo();
		if constexpr (CT::MoveMakable<K>)
			new (&GetRawKeys<K>()[index]) K {Move(key)};
		else if constexpr (CT::CopyMakable<K>)
			new (&GetRawKeys<K>()[index]) K {key};
		else LANGULUS_ERROR("Can't instantiate key");

		if constexpr (CT::MoveMakable<V>)
			new (&GetRawValues<V>()[index]) V {Move(value)};
		else if constexpr (CT::CopyMakable<V>)
			new (&GetRawValues<V>()[index]) V {value};
		else LANGULUS_ERROR("Can't instantiate value");

		*psl = attempts;
		++mValues.mCount;
	}
	
	/// Inner insertion function based on reflected move-assignment				
	///	@attention after this call, key and/or value might be empty				
	///	@param start - the starting index												
	///	@param key - key to move in														
	///	@param value - value to move in													
	template<bool CHECK_FOR_MATCH>
	void UnorderedMap::InsertInnerUnknown(const Offset& start, Block&& key, Block&& value) {
		// Get the starting index based on the key hash							
		auto psl = GetInfo() + start;
		const auto pslEnd = GetInfoEnd();
		InfoType attempts {1};
		while (*psl) {
			const auto index = psl - GetInfo();

			if constexpr (CHECK_FOR_MATCH) {
				const auto candidate = GetKey(index);
				if (candidate == key) {
					// Neat, the key already exists - just set value and go	
					GetValue(index)
						.CallUnknownMoveAssignment<false>(1, value);
					value.CallUnknownDestructors();
					value.mCount = 0;
					return;
				}
			}

			if (attempts > *psl) {
				// The pair we're inserting is closer to bucket, so swap		
				GetKey(index).SwapUnknown(key);
				GetValue(index).SwapUnknown(value);
				::std::swap(attempts, *psl);
			}

			++attempts;

			if (psl < pslEnd - 1) LIKELY() {
				++psl;
			}
			else UNLIKELY() {
				// Wrap around and start from the beginning						
				psl = GetInfo();
			}
		}

		// If reached, empty slot reached, so put the pair there				
		// Might not seem like it, but we gave a guarantee, that this is	
		// eventually reached, unless key exists and returns early			
		const auto index = psl - GetInfo();
		GetKey(index)
			.CallUnknownMoveConstructors<false>(1, key);
		GetValue(index)
			.CallUnknownMoveConstructors<false>(1, value);

		key.CallUnknownDestructors();
		value.CallUnknownDestructors();
		key.mCount = value.mCount = 0;

		*psl = attempts;
		++mValues.mCount;
	}

	/// Get the bucket index, depending on key hash										
	///	@param key - the key to hash														
	///	@return the bucket offset															
	template<CT::Data K>
	LANGULUS(ALWAYSINLINE) Offset UnorderedMap::GetBucket(const K& key) const noexcept {
		return HashData(key).mHash & (GetReserved() - 1);
	}

	/// Insert a single pair inside table via copy										
	///	@param key - the key to add														
	///	@param value - the value to add													
	///	@return 1 if pair was inserted, zero otherwise								
	template<CT::Data K, CT::Data V>
	Count UnorderedMap::Insert(const K& key, const V& value) {
		static_assert(CT::CopyMakable<K>,
			"Key needs to be copy-constructible, but isn't");
		static_assert(CT::CopyMakable<V>,
			"Value needs to be copy-constructible, but isn't");

		Mutate<K, V>();
		Allocate(GetCount() + 1);
		using KeyInner = typename TAny<K>::TypeInner;
		using ValInner = typename TAny<V>::TypeInner;
		InsertInner<true>(GetBucket(key), KeyInner {key}, ValInner {value});
		return 1;
	}

	/// Insert a single pair inside table via key copy and value move				
	///	@param key - the key to add														
	///	@param value - the value to add													
	///	@return 1 if pair was inserted, zero otherwise								
	template<CT::Data K, CT::Data V>
	Count UnorderedMap::Insert(const K& key, V&& value) {
		static_assert(CT::CopyMakable<K>,
			"Key needs to be copy-constructible, but isn't");
		static_assert(CT::MoveMakable<V>,
			"Value needs to be move-constructible, but isn't");

		Mutate<K, V>();
		Allocate(GetCount() + 1);
		using KeyInner = typename TAny<K>::TypeInner;
		using ValInner = typename TAny<V>::TypeInner;
		if constexpr (CT::Sparse<V>)
			InsertInner<true>(GetBucket(key), KeyInner {key}, ValInner {value});
		else
			InsertInner<true>(GetBucket(key), KeyInner {key}, Forward<V>(value));
		return 1;
	}

	/// Insert a single pair inside table via key move and value copy				
	///	@param key - the key to add														
	///	@param value - the value to add													
	///	@return 1 if pair was inserted, zero otherwise								
	template<CT::Data K, CT::Data V>
	Count UnorderedMap::Insert(K&& key, const V& value) {
		static_assert(CT::MoveMakable<K>,
			"Key needs to be move-constructible, but isn't");
		static_assert(CT::CopyMakable<V>,
			"Value needs to be copy-constructible, but isn't");

		Mutate<K, V>();
		Allocate(GetCount() + 1);
		using KeyInner = typename TAny<K>::TypeInner;
		using ValInner = typename TAny<V>::TypeInner;
		if constexpr (CT::Sparse<K>)
			InsertInner<true>(GetBucket(key), KeyInner {key}, ValInner {value});
		else
			InsertInner<true>(GetBucket(key), Forward<K>(key), ValInner {value});
		return 1;
	}

	/// Insert a single pair inside table via move										
	///	@param key - the key to add														
	///	@param value - the value to add													
	///	@return 1 if pair was inserted, zero otherwise								
	template<CT::Data K, CT::Data V>
	Count UnorderedMap::Insert(K&& key, V&& value) {
		static_assert(CT::MoveMakable<K>,
			"Key needs to be move-constructible, but isn't");
		static_assert(CT::MoveMakable<V>,
			"Value needs to be move-constructible, but isn't");

		Mutate<K, V>();
		Allocate(GetCount() + 1);
		using KeyInner = typename TAny<K>::TypeInner;
		using ValInner = typename TAny<V>::TypeInner;
		if constexpr (CT::Sparse<K>) {
			if constexpr (CT::Sparse<V>)
				InsertInner<true>(GetBucket(key), KeyInner {key}, ValInner {value});
			else
				InsertInner<true>(GetBucket(key), KeyInner {key}, Forward<V>(value));
		}
		else {
			if constexpr (CT::Sparse<V>)
				InsertInner<true>(GetBucket(key), Forward<K>(key), ValInner {value});
			else
				InsertInner<true>(GetBucket(key), Forward<K>(key), Forward<V>(value));
		}
		return 1;
	}

	/// Insert a single pair inside table via copy (unknown version)				
	///	@param key - the key to add														
	///	@param value - the value to add													
	///	@return 1 if pair was inserted, zero otherwise								
	inline Count UnorderedMap::InsertUnknown(const Block& key, const Block& value) {
		Mutate(key.mType, key.IsSparse(), value.mType, value.IsSparse());
		Allocate(GetCount() + 1);

		Block keySwapper {key.mType};
		keySwapper.Allocate<false, true>(1);
		keySwapper.CallUnknownCopyConstructors<true>(1, key);

		Block valSwapper {value.mType};
		valSwapper.Allocate<false, true>(1);
		valSwapper.CallUnknownCopyConstructors<true>(1, value);

		InsertInnerUnknown<true>(GetBucket(key), Move(keySwapper), Move(valSwapper));

		keySwapper.Free();
		valSwapper.Free();
		return 1;
	}

	/// Insert a single pair inside table via move (unknown version)				
	///	@param key - the key to add														
	///	@param value - the value to add													
	///	@return 1 if pair was inserted, zero otherwise								
	inline Count UnorderedMap::InsertUnknown(Block&& key, Block&& value) {
		Mutate(key.mType, key.IsSparse(), value.mType, value.IsSparse());
		Allocate(GetCount() + 1);
		InsertInnerUnknown<true>(GetBucket(key), Move(key), Move(value));
		return 1;
	}

	/// Copy-insert a templated pair inside the map										
	///	@param item - the pair to insert													
	///	@return a reference to this map for chaining									
	template<CT::Data K, CT::Data V>
	UnorderedMap& UnorderedMap::operator << (const TPair<K, V>& item) {
		Insert(item.mKey, item.mValue);
		return *this;
	}

	/// Move-insert a templated pair inside the map										
	///	@param item - the pair to insert													
	///	@return a reference to this map for chaining									
	template<CT::Data K, CT::Data V>
	UnorderedMap& UnorderedMap::operator << (TPair<K ,V>&& item) {
		Insert(Move(item.mKey), Move(item.mValue));
		return *this;
	}

	/// Copy-insert a type-erased pair inside the map									
	///	@param item - the pair to insert													
	///	@return a reference to this map for chaining									
	inline UnorderedMap& UnorderedMap::operator << (const Pair& item) {
		InsertUnknown(item.mKey, item.mValue);
		return *this;
	}

	/// Move-insert a type-erased pair inside the map									
	///	@param item - the pair to insert													
	///	@return a reference to this map for chaining									
	inline UnorderedMap& UnorderedMap::operator << (Pair&& item) {
		InsertUnknown(Move(item.mKey), Move(item.mValue));
		return *this;
	}

	/// Destroy everything valid inside the map											
	inline void UnorderedMap::ClearInner() {
		auto inf = GetInfo();
		const auto infEnd = GetInfoEnd();
		while (inf != infEnd) {
			if (*inf) {
				const auto offset = inf - GetInfo();
				GetKey(offset).CallUnknownDestructors();
				GetValue(offset).CallUnknownDestructors();
			}

			++inf;
		}
	}

	/// Clears all data, but doesn't deallocate											
	inline void UnorderedMap::Clear() {
		if (IsEmpty())
			return;

		if (GetUses() == 1) {
			// Remove all used keys and values, they're used only here		
			ClearInner();

			// Clear all info to zero													
			::std::memset(GetInfo(), 0, GetReserved());
			mValues.mCount = 0;
		}
		else {
			// Data is used from multiple locations, don't change data		
			// We're forced to dereference and reset memory pointers			
			mInfo = nullptr;
			mValues.mEntry->Free();
			mKeys.ResetMemory();
			mValues.ResetMemory();
		}
	}

	/// Clears all data and deallocates														
	inline void UnorderedMap::Reset() {
		if (GetUses() == 1) {
			// Remove all used keys and values, they're used only here		
			ClearInner();

			// No point in resetting info, we'll be deallocating it			
			Allocator::Deallocate(mKeys.mEntry);
			Allocator::Deallocate(mValues.mEntry);
		}
		else {
			// Data is used from multiple locations, just deref values		
			mValues.mEntry->Free();
		}

		mInfo = nullptr;
		mKeys.ResetState();
		mValues.ResetState();
		mKeys.ResetMemory();
		mValues.ResetMemory();
	}

	/// Erases element at a specific index													
	///	@attention assumes that offset points to a valid entry					
	///	@param offset - the index to remove												
	inline void UnorderedMap::RemoveIndex(const Offset& offset) noexcept {
		auto psl = GetInfo() + offset;
		const auto pslEnd = GetInfoEnd();
		auto key = mKeys.GetElement(offset);
		auto val = mValues.GetElement(offset);

		// Destroy the key, info and value at the offset						
		key.CallUnknownDestructors();
		val.CallUnknownDestructors();

		*(psl++) = 0;
		key.Next();
		val.Next();

		// And shift backwards, until a zero or 1 is reached					
		// That way we move every entry that is far from its start			
		// closer to it. Moving is costly, unless you use pointers			
		try_again:
		while (*psl > 1) {
			psl[-1] = (*psl) - 1;

			const_cast<const Block&>(key).Prev()
				.CallUnknownMoveConstructors<false>(1, Move(key));
			const_cast<const Block&>(val).Prev()
				.CallUnknownMoveConstructors<false>(1, Move(val));

			key.CallUnknownDestructors();
			val.CallUnknownDestructors();

			*(psl++) = 0;
			key.Next();
			val.Next();
		}

		// Be aware, that psl might loop around									
		if (psl == pslEnd && *GetInfo() > 1) UNLIKELY() {
			psl = GetInfo();
			key = mKeys.GetElement();
			val = mValues.GetElement();

			// Shift first entry to the back											
			GetInfo()[mValues.mReserved] = (*psl) - 1;

			GetKey(mValues.mReserved - 1)
				.CallUnknownMoveConstructors<false>(1, Move(key));
			GetValue(mValues.mReserved - 1)
				.CallUnknownMoveConstructors<false>(1, Move(val));

			key.CallUnknownDestructors();
			val.CallUnknownDestructors();

			*(psl++) = 0;
			key.Next();
			val.Next();

			// And continue the vicious cycle										
			goto try_again;
		}

		// Success																			
		--mValues.mCount;
	}

	/// Erase a pair via key																	
	///	@param match - the key to search for											
	///	@return the number of removed pairs												
	template<CT::Data K>
	Count UnorderedMap::RemoveKey(const K& match) {
		// Get the starting index based on the key hash							
		const auto start = GetBucket(match);
		auto key = GetRawKeys<K>() + start;
		const auto keyEnd = GetRawKeysEnd<K>();
		auto info = GetInfo() + start;

		while (key != keyEnd) {
			if (*info && *key == match) {
				// Found it																	
				RemoveIndex(info - GetInfo());
				return 1;
			}

			++key; ++info;
		}
		
		// No such key was found														
		return 0;
	}

	/// Erase all pairs with a given value													
	///	@param match - the value to search for											
	///	@return the number of removed pairs												
	template<CT::Data V>
	Count UnorderedMap::RemoveValue(const V& match) {
		Count removed {};
		auto value = GetRawValues<V>();
		const auto valueEnd = GetRawValuesEnd<V>();
		auto info = GetInfo();

		while (value != valueEnd) {
			if (*info && *value == match) {
				// Found it, but there may be more									
				RemoveIndex(info - GetInfo());
				++removed;
			}

			++value; ++info;
		}

		return removed;
	}

	/// If possible reallocates the map to a smaller one								
	inline void UnorderedMap::Compact() {
		//TODO();
	}

	///																								
	///	SEARCH																					
	///																								
	/// Search for a key inside the table													
	///	@param key - the key to search for												
	///	@return true if key is found, false otherwise								
	template<CT::Data K>
	bool UnorderedMap::ContainsKey(const K& key) const {
		if (IsEmpty())
			return false;
		return FindIndex(key) != GetReserved();
	}

	/// Search for a key inside the table, and return it if found					
	///	@param key - the key to search for												
	///	@return the index if key was found, or IndexNone if not					
	template<CT::Data K>
	Index UnorderedMap::FindKeyIndex(const K& key) const {
		const auto offset = FindIndex(key);
		return offset != GetReserved() ? Index {offset} : IndexNone;
	}

	/// Search for a value inside the table												
	///	@param value - the value to search for											
	///	@return true if value is found, false otherwise								
	template<CT::Data V>
	bool UnorderedMap::ContainsValue(const V& match) const {
		if (IsEmpty())
			return false;

		auto value = GetRawValues<V>();
		auto info = GetInfo();
		const auto valueEnd = GetRawValuesEnd<V>();

		while (value != valueEnd) {
			if (*info && *value == match)
				return true;

			++value; ++info;
		}

		return false;
	}

	/// Search for a pair inside the table													
	///	@param pair - the pair to search for											
	///	@return true if pair is found, false otherwise								
	template<CT::Data K, CT::Data V>
	bool UnorderedMap::ContainsPair(const TPair<K, V>& pair) const {
		const auto found = FindIndex(pair.mKey);
		return found != GetReserved() && GetValue(found) == pair.mValue;
	}

	/// Get a key by an unsafe offset (const)												
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the key													
	inline Block UnorderedMap::GetKey(const Offset& i) const noexcept {
		return mKeys.GetElement(i);
	}

	/// Get a key by an unsafe offset 														
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the key													
	inline Block UnorderedMap::GetKey(const Offset& i) noexcept {
		return mKeys.GetElement(i);
	}

	/// Get a value by an unsafe offset (const)											
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the value													
	inline Block UnorderedMap::GetValue(const Offset& i) const noexcept {
		return mValues.GetElement(i);
	}

	/// Get a value by an unsafe offset 													
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the value													
	inline Block UnorderedMap::GetValue(const Offset& i) noexcept {
		return mValues.GetElement(i);
	}

	/// Get a pair by an unsafe offset (const)											
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return the pair																		
	inline Pair UnorderedMap::GetPair(const Offset& i) const noexcept {
		return {GetKey(i), GetValue(i)};
	}

	/// Get a pair by an unsafe offset 														
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return the pair																		
	inline Pair UnorderedMap::GetPair(const Offset& i) noexcept {
		return {GetKey(i), GetValue(i)};
	}

	/// Returns a reference to the value found for key									
	/// Throws Except::OutOfRange if element cannot be found							
	///	@param key - the key to search for												
	///	@return a reference to the value													
	template<CT::Data K>
	decltype(auto) UnorderedMap::At(const K& key) {
		auto found = GetRawValues<K>() + FindIndex<K>(key);
		if (found == GetRawValuesEnd<K>())
			Throw<Except::OutOfRange>(
				"Key not found", LANGULUS_LOCATION());
		return *found;
	}

	/// Returns a reference to the value found for key (const)						
	/// Throws Except::OutOfRange if element cannot be found							
	///	@param key - the key to search for												
	///	@return a reference to the value													
	template<CT::Data K>
	decltype(auto) UnorderedMap::At(const K& key) const {
		return const_cast<UnorderedMap&>(*this).template At<K>(key);
	}

	/// Get a key by a safe index (const)													
	///	@param index - the index to use													
	///	@return a reference to the key													
	inline Block UnorderedMap::GetKey(const Index& index) const {
		return const_cast<UnorderedMap&>(*this).GetKey(index);
	}

	/// Get a key by a safe index 															
	///	@param index - the index to use													
	///	@return a reference to the key													
	inline Block UnorderedMap::GetKey(const Index& index) {
		const auto offset = index.GetOffset();
		if (offset >= GetReserved() || 0 == GetInfo()[offset])
			Throw<Except::OutOfRange>(
				"Bad index for UnorderedMap::GetKey", LANGULUS_LOCATION());
		return GetKey(offset);
	}

	/// Get a value by a safe index (const)												
	///	@param index - the index to use													
	///	@return a reference to the value													
	inline Block UnorderedMap::GetValue(const Index& index) const {
		return const_cast<UnorderedMap&>(*this).GetValue(index);
	}

	/// Get a value by a safe index 															
	///	@param index - the index to use													
	///	@return a reference to the value													
	inline Block UnorderedMap::GetValue(const Index& index) {
		const auto offset = index.GetOffset();
		if (offset >= GetReserved() || 0 == GetInfo()[offset])
			Throw<Except::OutOfRange>(
				"Bad index for UnorderedMap::GetValue", LANGULUS_LOCATION());
		return GetValue(offset);
	}

	/// Get a pair by a safe index (const)													
	///	@param index - the index to use													
	///	@return the pair																		
	inline Pair UnorderedMap::GetPair(const Index& index) const {
		return const_cast<UnorderedMap&>(*this).GetPair(index);
	}

	/// Get a pair by a safe index 															
	///	@param index - the index to use													
	///	@return the pair																		
	inline Pair UnorderedMap::GetPair(const Index& index) {
		const auto offset = index.GetOffset();
		if (offset >= GetReserved() || 0 == GetInfo()[offset])
			Throw<Except::OutOfRange>(
				"Bad index for UnorderedMap::GetPair", LANGULUS_LOCATION());
		return GetPair(offset);
	}

	/// Find the index of a pair by key														
	///	@param key - the key to search for												
	///	@return the index																		
	template<CT::Data K>
	Offset UnorderedMap::FindIndex(const K& key) const {
		// Get the starting index based on the key hash							
		// Since reserved elements are always power-of-two, we use them	
		// as a mask to the hash, to extract the relevant bucket				
		const auto start = GetBucket(key);
		auto psl = GetInfo() + start;
		const auto pslEnd = GetInfoEnd() - 1;
		auto candidate = GetRawKeys<K>() + start;
		Count attempts{};
		while (*psl > attempts) {
			if (*candidate != key) {
				// There might be more keys to the right, check them			
				if (psl == pslEnd) UNLIKELY() {
					// By 'to the right' I also mean looped back to start		
					psl = GetInfo();
					candidate = GetRawKeys<K>();
				}
				else LIKELY() {
					++psl;
					++candidate;
				}

				++attempts;
				continue;
			}

			// Found																			
			return psl - GetInfo();
		}

		// Nothing found, return end offset											
		return GetReserved();
	}

	/// Access value by key																		
	///	@param key - the key to find														
	///	@return a the value wrapped inside an Any										
	template<CT::Data K>
	Any UnorderedMap::operator[] (const K& key) const {
		auto found = FindIndex<K>(key);
		if (found == GetReserved())
			Throw<Except::OutOfRange>("Key not found", LANGULUS_LOCATION());
		Block element {GetValue(found)};
		return Disown(element);
	}

	/// Access value by key																		
	///	@param key - the key to find														
	///	@return a the value wrapped inside an Any										
	template<CT::Data K>
	Any UnorderedMap::operator[] (const K& key) {
		auto found = FindIndex<K>(key);
		if (found == GetReserved())
			Throw<Except::OutOfRange>("Key not found", LANGULUS_LOCATION());
		Block element {GetValue(found)};
		return Disown(element);
	}

	/// Get the number of inserted pairs													
	///	@return the number of inserted pairs											
	constexpr Count UnorderedMap::GetCount() const noexcept {
		return mValues.GetCount();
	}

	/// Get the number of allocated pairs													
	///	@return the number of allocated pairs											
	constexpr Count UnorderedMap::GetReserved() const noexcept {
		return mValues.GetReserved();
	}

	/// Check if there are any pairs in this map											
	///	@return true if there's at least one pair available						
	constexpr bool UnorderedMap::IsEmpty() const noexcept {
		return mValues.IsEmpty();
	}

	/// Check if the map has been allocated												
	///	@return true if the map uses dynamic memory									
	constexpr bool UnorderedMap::IsAllocated() const noexcept {
		return mValues.IsAllocated();
	}

	/// Check if the memory for the table is owned by us								
	/// This is always true, since the map can't be initialized with outside	
	/// memory - the memory layout requirements are too strict to allow for it	
	///	@return true																			
	constexpr bool UnorderedMap::HasAuthority() const noexcept {
		return IsAllocated();
	}

	/// Get the number of references for the allocated memory						
	///	@attention always returns zero if we don't have authority				
	///	@return the number of references													
	constexpr Count UnorderedMap::GetUses() const noexcept {
		return mValues.GetUses();
	}

	
	/// Get iterator to first element														
	///	@return an iterator to the first element, or end if empty				
	inline typename UnorderedMap::Iterator UnorderedMap::begin() noexcept {
		static_assert(sizeof(Iterator) == sizeof(ConstIterator),
			"Size mismatch - types must be binary-compatible");
		const auto constant = const_cast<const UnorderedMap*>(this)->begin();
		return reinterpret_cast<const Iterator&>(constant);
	}

	/// Get iterator to end																		
	///	@return an iterator to the end element											
	inline typename UnorderedMap::Iterator UnorderedMap::end() noexcept {
		static_assert(sizeof(Iterator) == sizeof(ConstIterator),
			"Size mismatch - types must be binary-compatible");
		const auto constant = const_cast<const UnorderedMap*>(this)->end();
		return reinterpret_cast<const Iterator&>(constant);
	}

	/// Get iterator to the last element													
	///	@return an iterator to the last element, or end if empty					
	inline typename UnorderedMap::Iterator UnorderedMap::last() noexcept {
		static_assert(sizeof(Iterator) == sizeof(ConstIterator),
			"Size mismatch - types must be binary-compatible");
		const auto constant = const_cast<const UnorderedMap*>(this)->last();
		return reinterpret_cast<const Iterator&>(constant);
	}

	/// Get iterator to first element														
	///	@return a constant iterator to the first element, or end if empty		
	inline typename UnorderedMap::ConstIterator UnorderedMap::begin() const noexcept {
		if (IsEmpty())
			return end();

		// Seek first valid info, or hit sentinel at the end					
		auto info = GetInfo();
		while (!*info) ++info;

		const auto offset = info - GetInfo();
		return {
			info, GetInfoEnd(), 
			GetKey(offset),
			GetValue(offset)
		};
	}

	/// Get iterator to end																		
	///	@return a constant iterator to the end element								
	inline typename UnorderedMap::ConstIterator UnorderedMap::end() const noexcept {
		return {GetInfoEnd(), GetInfoEnd(), {}, {}};
	}

	/// Get iterator to the last valid element											
	///	@return a constant iterator to the last element, or end if empty		
	inline typename UnorderedMap::ConstIterator UnorderedMap::last() const noexcept {
		if (IsEmpty())
			return end();

		// Seek first valid info in reverse, until one past first is met	
		auto info = GetInfoEnd();
		while (info >= GetInfo() && !*--info);

		const auto offset = info - GetInfo();
		return {
			info, GetInfoEnd(),
			GetKey(offset),
			GetValue(offset)
		};
	}


	///																								
	///	Unordered map iterator																
	///																								

	/// Construct an iterator																	
	///	@param info - the info pointer													
	///	@param sentinel - the end of info pointers									
	///	@param key - pointer to the key element										
	///	@param value - pointer to the value element									
	template<bool MUTABLE>
	LANGULUS(ALWAYSINLINE)
	UnorderedMap::TIterator<MUTABLE>::TIterator(
		const InfoType* info, 
		const InfoType* sentinel, 
		const Block& key, 
		const Block& value
	) noexcept
		: mInfo {info}
		, mSentinel {sentinel}
		, mKey {key}
		, mValue {value} {}

	/// Prefix increment operator																
	///	@attention assumes iterator points to a valid element						
	///	@return the modified iterator														
	template<bool MUTABLE>
	LANGULUS(ALWAYSINLINE)
	typename UnorderedMap::TIterator<MUTABLE>& UnorderedMap::TIterator<MUTABLE>::operator ++ () noexcept {
		if (mInfo == mSentinel)
			return *this;

		// Seek next valid info, or hit sentinel at the end					
		const auto previous = mInfo;
		while (!*++mInfo);
		const auto offset = mInfo - previous;
		mKey.mRaw += offset * mKey.GetStride();
		mValue.mRaw += offset * mValue.GetStride();
		return *this;
	}

	/// Suffix increment operator																
	///	@attention assumes iterator points to a valid element						
	///	@return the previous value of the iterator									
	template<bool MUTABLE>
	LANGULUS(ALWAYSINLINE)
	typename UnorderedMap::TIterator<MUTABLE> UnorderedMap::TIterator<MUTABLE>::operator ++ (int) noexcept {
		const auto backup = *this;
		operator ++ ();
		return backup;
	}

	/// Compare unordered map entries														
	///	@param rhs - the other iterator													
	///	@return true if entries match														
	template<bool MUTABLE>
	LANGULUS(ALWAYSINLINE)
	bool UnorderedMap::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
		return mInfo == rhs.mInfo;
	}

	/// Iterator access operator																
	///	@return a pair at the current iterator position								
	template<bool MUTABLE>
	LANGULUS(ALWAYSINLINE)
	Pair UnorderedMap::TIterator<MUTABLE>::operator * () const noexcept {
		return {Disown(mKey), Disown(mValue)};
	}

} // namespace Langulus::Anyness
