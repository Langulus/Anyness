///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "THashMap.hpp"

#define TABLE_TEMPLATE() template<CT::Data K, CT::Data V>
#define TABLE() THashMap<K, V>

namespace Langulus::Anyness
{

	/// Manual construction via an initializer list										
	///	@param initlist - the initializer list to forward							
	TABLE_TEMPLATE()
	TABLE()::THashMap(::std::initializer_list<Pair> initlist)
		: THashMap{} {
		Allocate(initlist.size());
		for (auto& it : initlist)
			Insert(*it);
	}

	/// Shallow-copy construction																
	///	@param other - the table to copy													
	TABLE_TEMPLATE()
	TABLE()::THashMap(const THashMap& other)
		: mKeys {other.mKeys}
		, mInfo {other.mInfo}
		, mValues {other.mValues} {}

	/// Move construction																		
	///	@param other - the table to move													
	TABLE_TEMPLATE()
	TABLE()::THashMap(THashMap&& other) noexcept
		: mKeys {other.mKeys}
		, mInfo {other.mInfo}
		, mValues {Move(other.mValues)} {}

	/// Shallow-copy construction without referencing									
	///	@param other - the disowned table to copy										
	TABLE_TEMPLATE()
	TABLE()::THashMap(Disowned<THashMap>&& other) noexcept
		: mKeys {other.mValue.mKeys}
		, mInfo {other.mValue.mInfo}
		, mValues {Disown(other.mValue.mValues)} {}

	/// Minimal move construction from abandoned table									
	///	@param other - the abandoned table to move									
	TABLE_TEMPLATE()
	TABLE()::THashMap(Abandoned<THashMap>&& other) noexcept
		: mKeys {other.mValue.mKeys}
		, mInfo {other.mValue.mInfo}
		, mValues {Abandon(other.mValue.mValues)} {}

	/// Destroys the map and all it's contents											
	TABLE_TEMPLATE()
	TABLE()::~THashMap() {
		if (!mValues.mEntry)
			return;

		if (mValues.mEntry->GetUses() == 1) {
			// Remove all used keys and values, they're used only here		
			ClearInner();

			// Deallocate stuff															
			Inner::Allocator::Deallocate(mKeys);
			Inner::Allocator::Deallocate(mValues.mEntry);
		}
		else {
			// Data is used from multiple locations, just deref values		
			mValues.mEntry->Free();
		}

		mValues.mEntry = nullptr;
	}

	/// Checks if both tables contain the same entries									
	/// Order is irrelevant																		
	///	@param other - the table to compare against									
	///	@return true if tables match														
	TABLE_TEMPLATE()
	bool TABLE()::operator == (const THashMap& other) const {
		if (other.GetCount() != GetCount())
			return false;

		const auto keyEnd = GetRawKeysEnd();
		auto key = GetRawKeys();
		auto info = GetInfo();
		while (key != keyEnd) {
			if (0 == *info) {
				++key; ++info;
				continue;
			}

			const auto rhs = other.FindIndex(*key);
			if (rhs == other.GetReserved() || GetValue(key - GetRawKeys()) != other.GetValue(rhs))
				return false;

			++key; ++info;
		}

		return true;
	}

	/// Move a table																				
	///	@param rhs - the table to move													
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (THashMap&& rhs) noexcept {
		if (&rhs == this)
			return *this;

		Reset();
		new (this) Self {Forward<THashMap>(rhs)};
		return *this;
	}

	/// Creates a shallow copy of the given table										
	///	@param rhs - the table to reference												
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (const THashMap& rhs) {
		if (&rhs == this)
			return *this;

		Reset();
		new (this) Self {rhs};
		return *this;
	}

	/// Emplace a single pair into a cleared map											
	///	@param pair - the pair to emplace												
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (Pair&& pair) noexcept {
		Clear();
		Insert(Forward<Pair>(pair));
		return *this;
	}

	/// Insert a single pair into a cleared map											
	///	@param pair - the pair to copy													
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (const Pair& pair) {
		Clear();
		Insert(pair);
		return *this;
	}

	/// Clone all elements in a range														
	///	@param info - info bytes for checking valid entries						
	///	@param from - start of the elements to copy									
	///	@param fromEnd - end of the elements to copy									
	///	@param to - destrination memory													
	TABLE_TEMPLATE()
	template<class T>
	void TABLE()::CloneInner(const Count& count, const uint8_t* info, const T* from, const T* fromEnd, T* to) {
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
				
				if constexpr (CT::CloneMakable<T>)
					new (cache) TD {(*from)->Clone()};
				else if constexpr (CT::POD<T>)
					::std::memcpy(cache, **from, sizeof(TD));
				else
					LANGULUS_ASSERT("Can't clone map component made of non-clonable/non-POD type");
				
				*to = cache;

				++from; ++to; ++cache; ++info;
			}
			
			coalesced.Reference(cache - coalesced.GetRaw());
		}
		else if constexpr (CT::CloneMakable<T>) {
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
		else LANGULUS_ASSERT("Can't clone map components made of non-clonable/non-POD type");
	}

	/// Clone the table																			
	///	@return the new table																
	TABLE_TEMPLATE()
	TABLE() TABLE()::Clone() const {
		if (IsEmpty())
			return {};

		THashMap result {Disown(*this)};

		// Allocate keys and info														
		result.mKeys = Inner::Allocator::Allocate(mKeys->GetAllocatedSize());
		if (!result.mKeys)
			Throw<Except::Allocate>("Out of memory on cloning THashMap keys");

		result.mInfo = reinterpret_cast<uint8_t*>(result.mKeys) 
			+ (mInfo - reinterpret_cast<const uint8_t*>(mKeys));

		// Clone the info bytes															
		::std::memcpy(result.GetInfo(), GetInfo(), GetReserved() + 1);

		// Clone the keys																	
		CloneInner(result.mValues.mCount, GetInfo(), 
			GetRawKeys(), GetRawKeysEnd(), result.GetRawKeys());

		// Allocate and clone values													
		result.mValues.mEntry = Inner::Allocator::Allocate(result.mValues.GetReservedSize());
		if (!result.mValues.mEntry) {
			Inner::Allocator::Deallocate(result.mKeys);
			result.mValues.mEntry = nullptr;
			Throw<Except::Allocate>("Out of memory on cloning THashMap values");
		}

		result.mValues.mRaw = result.mValues.mEntry->GetBlockStart();
		CloneInner(result.mValues.mCount, GetInfo(), 
			GetRawValues(), GetRawValuesEnd(), result.GetRawValues());
		return Abandon(result);
	}
	
	/// Templated tables are always typed													
	///	@return false																			
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyUntyped() const noexcept {
		return false;
	}
	
	/// Templated tables are always typed													
	///	@return false																			
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueUntyped() const noexcept {
		return false;
	}
	
	/// Templated tables are always type-constrained									
	///	@return true																			
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyTypeConstrained() const noexcept {
		return true;
	}
	
	/// Templated tables are always type-constrained									
	///	@return true																			
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueTypeConstrained() const noexcept {
		return true;
	}
	
	/// Check if key type is abstract														
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyAbstract() const noexcept {
		return CT::Abstract<K> && !IsKeySparse();
	}
	
	/// Check if value type is abstract														
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueAbstract() const noexcept {
		return CT::Abstract<V> && !IsValueSparse();
	}
	
	/// Check if key type is default-constructible										
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyConstructible() const noexcept {
		return CT::Defaultable<K>;
	}
	
	/// Check if value type is default-constructible									
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueConstructible() const noexcept {
		return CT::Defaultable<V>;
	}
	
	/// Check if key type is deep																
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyDeep() const noexcept {
		return CT::Deep<K>;
	}
	
	/// Check if value type is deep															
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueDeep() const noexcept {
		return CT::Deep<V>;
	}

	/// Check if the key type is a pointer													
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeySparse() const noexcept {
		return CT::Sparse<K>;
	}
	
	/// Check if the value type is a pointer												
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueSparse() const noexcept {
		return CT::Sparse<V>;
	}

	/// Check if the key type is not a pointer											
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyDense() const noexcept {
		return CT::Dense<K>;
	}

	/// Check if the value type is not a pointer											
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueDense() const noexcept {
		return CT::Dense<V>;
	}

	/// Get the size of a single key, in bytes											
	///	@return the number of bytes a single key contains							
	TABLE_TEMPLATE()
	constexpr Size TABLE()::GetKeyStride() const noexcept {
		return sizeof(K); 
	}
	
	/// Get the size of a single value, in bytes											
	///	@return the number of bytes a single value contains						
	TABLE_TEMPLATE()
	constexpr Size TABLE()::GetValueStride() const noexcept {
		return sizeof(V); 
	}

	/// Get the raw key array (const)														
	TABLE_TEMPLATE()
	constexpr auto TABLE()::GetRawKeys() const noexcept {
		return const_cast<TABLE()*>(this)->GetRawKeys();
	}

	/// Get the raw key array																	
	TABLE_TEMPLATE()
	constexpr auto TABLE()::GetRawKeys() noexcept {
		if constexpr (CT::Sparse<K>)
			return reinterpret_cast<typename TAny<K>::KnownPointer*>(mKeys->GetBlockStart());
		else
			return reinterpret_cast<K*>(mKeys->GetBlockStart());
	}

	/// Get the end of the raw key array													
	TABLE_TEMPLATE()
	constexpr auto TABLE()::GetRawKeysEnd() const noexcept {
		return GetRawKeys() + GetReserved();
	}

	/// Get the raw value array (const)														
	TABLE_TEMPLATE()
	constexpr auto TABLE()::GetRawValues() const noexcept {
		return mValues.GetRaw();
	}

	/// Get the raw value array																
	TABLE_TEMPLATE()
	constexpr auto TABLE()::GetRawValues() noexcept {
		return mValues.GetRaw();
	}

	/// Get end of the raw value array														
	TABLE_TEMPLATE()
	constexpr auto TABLE()::GetRawValuesEnd() const noexcept {
		return mValues.GetRaw() + GetReserved();
	}

	/// Get the size of all pairs, in bytes												
	///	@return the total amount of initialized bytes								
	TABLE_TEMPLATE()
	constexpr Size TABLE()::GetByteSize() const noexcept {
		return sizeof(Pair) * GetCount(); 
	}

	/// Get the key meta data																	
	TABLE_TEMPLATE()
	DMeta TABLE()::GetKeyType() const {
		return MetaData::Of<K>();
	}

	/// Get the value meta data																
	TABLE_TEMPLATE()
	DMeta TABLE()::GetValueType() const {
		return MetaData::Of<V>();
	}

	/// Check if key type exactly matches another										
	TABLE_TEMPLATE()
	template<class ALT_K>
	constexpr bool TABLE()::KeyIs() const noexcept {
		return CT::Same<K, ALT_K>;
	}

	/// Check if value type exactly matches another										
	TABLE_TEMPLATE()
	template<class ALT_V>
	constexpr bool TABLE()::ValueIs() const noexcept {
		return CT::Same<V, ALT_V>;
	}

	/// Move-insert a pair inside the map													
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator << (Pair&& item) {
		Insert(Forward<Pair>(item));
		return *this;
	}

	/// Copy-insert a pair inside the map													
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator << (const Pair& item) {
		Insert(item);
		return *this;
	}

	/// Request a new size of keys and info via the value container				
	/// The memory layout is:																	
	///	[keys for each bucket]																
	///			[padding for alignment]														
	///					[info for each bucket]												
	///							[one sentinel byte for terminating loops]				
	///	@param infoStart - [out] the offset at which info bytes start			
	///	@return the requested byte size													
	TABLE_TEMPLATE()
	Size TABLE()::RequestKeyAndInfoSize(const Count request, Offset& infoStart) noexcept {
		const Size keymemory = request * sizeof(K);
		infoStart = keymemory + Alignment - (keymemory % Alignment);
		return infoStart + request + 1;
	}

	/// Get the info array (const)															
	///	@return a pointer to the first element inside the info array			
	TABLE_TEMPLATE()
	const uint8_t* TABLE()::GetInfo() const noexcept {
		return mInfo;
	}

	/// Get the info array																		
	///	@return a pointer to the first element inside the info array			
	TABLE_TEMPLATE()
	uint8_t* TABLE()::GetInfo() noexcept {
		return mInfo;
	}

	/// Get the end of the info array														
	///	@return a pointer to the first element inside the info array			
	TABLE_TEMPLATE()
	const uint8_t* TABLE()::GetInfoEnd() const noexcept {
		return mInfo + GetReserved();
	}

	/// Reserves space for the specified number of pairs								
	///	@attention does nothing if reserving less than current reserve			
	///	@param count - number of pairs to allocate									
	TABLE_TEMPLATE()
	void TABLE()::Allocate(const Count& count) {
		AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
	}

	/// Allocate or reallocate key and info array										
	///	@attention assumes count is a power-of-two									
	///	@tparam REUSE - true to reallocate, false to allocate fresh				
	///	@param count - the new number of pairs											
	TABLE_TEMPLATE()
	template<bool REUSE>
	void TABLE()::AllocateKeys(const Count& count) {
		#if LANGULUS(SAFE)
			if (!IsPowerOfTwo(count))
				Throw<Except::Allocate>("Table reallocation count is not a power-of-two");
		#endif

		Offset infoOffset;
		const auto oldKeys = mKeys;
		if constexpr (REUSE) {
			// Reallocate the key and info arrays									
			mKeys = Inner::Allocator::Reallocate(
				RequestKeyAndInfoSize(count, infoOffset), mKeys
			);
		}
		else {
			// Allocate a fresh set of keys and info								
			// Assumes nothing's there to move and initialize					
			mKeys = Inner::Allocator::Allocate(
				RequestKeyAndInfoSize(count, infoOffset)
			);
		}

		if (!mKeys)
			Throw<Except::Allocate>("Out of memory on allocating/reallocating THashMap keys");

		// Precalculate the info pointer, it's costly							
		auto oldInfo = mInfo;
		mInfo = reinterpret_cast<uint8_t*>(
			mKeys->GetBlockStart() + infoOffset
		);

		const auto oldCount = GetReserved();
		const auto oldInfoEnd = oldInfo + oldCount;
		auto key = oldKeys->As<K>();

		// Zero or move the info array												
		if constexpr (REUSE) {
			// Check if keys were reused												
			if (mKeys == oldKeys) {
				// Keys were reused, but info always moves (null the rest)	
				::std::memmove(mInfo, oldInfo, oldCount);
				::std::memset(mInfo + oldCount, 0, count - oldCount);
			}
			else {
				// Keys weren't reused, so clear the new ones					
				::std::memset(mInfo, 0, count);
			}
		}
		else ::std::memset(mInfo, 0, count);

		// Set the sentinel																
		mInfo[count] = 1;

		// Allocate new values															
		const auto oldValues = mValues.mEntry;
		auto value = mValues.GetRaw();
		if constexpr (REUSE)
			mValues.mEntry = Inner::Allocator::Reallocate(count * sizeof(V), oldValues);
		else
			mValues.mEntry = Inner::Allocator::Allocate(count * sizeof(V));

		if (!mValues.mEntry) {
			Inner::Allocator::Deallocate(mKeys);
			Throw<Except::Allocate>("Out of memory on allocating/reallocating THashMap values");
		}

		mValues.mRaw = mValues.mEntry->GetBlockStart();
		mValues.mReserved = count;
		mValues.mCount = 0;

		if constexpr (REUSE) {
			if (mValues.mEntry == oldValues && oldKeys == mKeys) {
				// Both keys and values remain in the same place, so rehash	
				Rehash(count, oldCount);
				return;
			}
		}

		// If reached, then keys or values (or both) moved						
		// Reinsert all pairs to rehash												
		while (oldInfo != oldInfoEnd) {
			if (0 == *oldInfo) {
				++key; ++oldInfo; ++value;
				continue;
			}

			if constexpr (REUSE) {
				Insert(Pair {Move(*key), Move(*value)});
				if constexpr (CT::Dense<K>)
					RemoveInner(key);
				if constexpr (CT::Dense<V>)
					RemoveInner(value);
			}
			else {
				Insert(Pair {*key, *value});
			}

			++key; ++oldInfo; ++value;
		}

		// Free the old allocations													
		if constexpr (REUSE) {
			// When reusing, keys and values can potentially remain same	
			// Avoid deallocating them if that's the case						
			if (oldValues != mValues.mEntry)
				Inner::Allocator::Deallocate(oldValues);
			if (oldKeys != mKeys)
				Inner::Allocator::Deallocate(oldKeys);
		}
		else if (oldValues) {
			// Not reusing, so either deallocate, or dereference				
			// (keys are always present, if values are present)				
			if (oldValues->GetUses() > 1)
				oldValues->Free();
			else {
				Inner::Allocator::Deallocate(oldValues);
				Inner::Allocator::Deallocate(oldKeys);
			}
		}
	}

	/// Similar to insertion, but rehashes each key 									
	///	@attention does nothing if reserving less than current reserve			
	///	@attention assumes count is a power-of-two number							
	///	@param count - the new number of pairs											
	TABLE_TEMPLATE()
	void TABLE()::Rehash(const Count& count, const Count& oldCount) {
		auto oldKey = GetRawKeys();
		auto oldInfo = GetInfo();
		const auto oldKeyEnd = oldKey + oldCount;

		// For each old existing key...												
		while (oldKey != oldKeyEnd) {
			if (!*oldInfo) {
				++oldKey; ++oldInfo;
				continue;
			}

			// Rehash and check if hashes match										
			const auto oldIndex = oldInfo - GetInfo();
			const auto newIndex = HashData(*oldKey).mHash & (count - 1);
			if (oldIndex != newIndex) {
				// Immediately move the old pair to the swapper					
				auto oldValue = &GetValue(oldIndex);
				Pair swapper {Move(*oldKey), Move(*oldValue)};

				// Clean the old slot													
				if constexpr (CT::Dense<K>)
					RemoveInner(oldKey);
				if constexpr (CT::Dense<V>)
					RemoveInner(oldValue);

				*oldInfo = 0;

				// Insert the swapper													
				InsertInner(newIndex, swapper.mKey, swapper.mValue);
			}
			else {
				// Nothing inserted, but since count has been previously		
				// cleared, restore the count and move forward					
				++mValues.mCount;
			}

			++oldKey; ++oldInfo;
		}
	}

	/// Reserves space for the specified number of pairs								
	///	@attention does nothing if reserving less than current reserve			
	///	@attention assumes count is a power-of-two number							
	///	@param count - number of pairs to allocate									
	TABLE_TEMPLATE()
	void TABLE()::AllocateInner(const Count& count) {
		// Shrinking is never allowed, you'll have to do it explicitly 	
		// via Compact()																	
		if (count <= GetReserved())
			return;

		// Allocate/Reallocate the keys and info									
		if (IsAllocated()) {
			if (GetUses() == 1)
				AllocateKeys<true>(count);
			else
				AllocateKeys<false>(count);
		}
		else AllocateKeys<false>(count);
	}

	/// Inner insertion function																
	///	@param start - the starting index												
	///	@param key - key to move in														
	///	@param value - value to move in													
	TABLE_TEMPLATE()
	void TABLE()::InsertInner(const Offset& start, K& key, V& value) {
		// Used for swapping key/value known pointer entries, to avoid		
		// losing that information when swapping sparse stuff					
		using KKP = typename TAny<K>::KnownPointer;
		using VKP = typename TAny<V>::KnownPointer;
		UNUSED() KKP keyBackup;
		UNUSED() VKP valueBackup;
		if constexpr (CT::Sparse<K>)
			new (&keyBackup) KKP {key};
		if constexpr (CT::Sparse<V>)
			new (&valueBackup) VKP {value};

		// Get the starting index based on the key hash							
		auto psl = GetInfo() + start;
		const auto pslEnd = GetInfoEnd();
		auto candidate = GetRawKeys() + start;
		uint8_t attempts {1};
		while (*psl) {
			if (*candidate == key) {
				// Neat, the key already exists - just set value and go		
				const auto index = psl - GetInfo();
				Overwrite(Move(value), GetValue(index));
				return;
			}

			if (attempts > *psl) {
				// The pair we're inserting is closer to bucket, so swap		
				const auto index = psl - GetInfo();
				if constexpr (CT::Sparse<K>)
					::std::swap(GetKey(index), keyBackup);
				else
					::std::swap(GetKey(index), key);

				if constexpr (CT::Sparse<V>)
					::std::swap(GetValue(index), valueBackup);
				else
					::std::swap(GetValue(index), value);

				::std::swap(attempts, *psl);
			}

			++attempts;

			if (psl < pslEnd - 1) LIKELY() {
				++psl;
				++candidate;
			}
			else UNLIKELY() {
				// Wrap around and start from the beginning						
				psl = GetInfo();
				candidate = GetRawKeys();
			}
		}

		// If reached, empty slot reached, so put the pair there				
		// Might not seem like it, but we gave a guarantee, that this is	
		// eventually reached, unless key exists and returns early			
		const auto index = psl - GetInfo();
		new (&GetKey(index)) K {Move(key)};
		new (&GetValue(index)) V {Move(value)};
		*psl = attempts;
		++mValues.mCount;
	}

	/// Get the bucket index, depending on key hash										
	///	@param key - the key to hash														
	///	@return the bucket offset															
	TABLE_TEMPLATE()
	Offset TABLE()::GetBucket(const K& key) const noexcept {
		return HashData(key).mHash & (GetReserved() - 1);
	}

	/// Insert a single pair inside table via shallow-copy							
	/// Guarantees that original item remains unchanged								
	///	@param item - pair to add															
	///	@return 1 if pair was inserted													
	TABLE_TEMPLATE()
	Count TABLE()::Insert(const Pair& item) {
		// Guarantee that there's at least one free space						
		Allocate(GetCount() + 1);

		// Make a temporary swapper, so that original item never changes	
		const auto bucket = GetBucket(item.mKey);
		Pair swapper {item};
		InsertInner(bucket, swapper.mKey, swapper.mValue);
		return 1;
	}

	/// Insert a single pair inside table via move										
	///	@attention original item may change, because it's used as swapper		
	///	@param item - pair to add															
	///	@return 1 if pair was inserted													
	TABLE_TEMPLATE()
	Count TABLE()::Insert(Pair&& item) {
		// Guarantee that there's at least one free space						
		Allocate(GetCount() + 1);

		// Use the original item as the swapper									
		const auto bucket = GetBucket(item.mKey);
		InsertInner(bucket, item.mKey, item.mValue);
		return 1;
	}

	/// Destroy everything valid inside the map											
	TABLE_TEMPLATE()
	void TABLE()::ClearInner() {
		auto key = GetRawKeys();
		auto val = GetRawValues();
		auto inf = GetInfo();
		const auto infEnd = GetInfoEnd();
		while (inf != infEnd) {
			if (*inf) {
				RemoveInner(key);
				RemoveInner(val);
			}

			++key; ++val; ++inf;
		}
	}

	/// Clears all data, but doesn't deallocate											
	TABLE_TEMPLATE()
	void TABLE()::Clear() {
		if (IsEmpty())
			return;

		if (GetUses() == 1) {
			// Remove all used keys and values, they're used only here		
			ClearInner();

			// Clear all info to zero													
			::std::memset(GetInfo(), 0, GetReserved());
		}
		else {
			// Data is used from multiple locations, don't change data		
			// We're forced to dereference and reset memory pointers			
			mKeys = nullptr;
			mInfo = nullptr;
			mValues.mEntry->Free();
			mValues.mEntry = nullptr;
			mValues.mRaw = nullptr;
			mValues.mReserved = 0;
		}

		mValues.mCount = 0;
	}

	/// Clears all data and deallocates														
	TABLE_TEMPLATE()
	void TABLE()::Reset() {
		if (GetUses() == 1) {
			// Remove all used keys and values, they're used only here		
			ClearInner();

			// No point in resetting info, we'll be deallocating it			
			Inner::Allocator::Deallocate(mKeys);
			Inner::Allocator::Deallocate(mValues.mEntry);
		}
		else {
			// Data is used from multiple locations, just deref values		
			mValues.mEntry->Free();
		}

		mKeys = nullptr;
		mInfo = nullptr;
		mValues.ResetState();
		mValues.ResetMemory();
	}

	/// Erases element at a specific index													
	///	@attention assumes that index points to a valid entry						
	///	@param start - the index to remove												
	TABLE_TEMPLATE()
	void TABLE()::RemoveIndex(const Offset& start) noexcept {
		auto psl = GetInfo() + start;
		const auto pslEnd = GetInfoEnd();
		auto key = GetRawKeys() + start;
		auto value = GetRawValues() + start;
		using KeyTransfer = Decay<decltype(key)>;
		using ValTransfer = Decay<decltype(value)>;

		// Destroy the key, info and value at the start							
		RemoveInner(value);
		RemoveInner(key);

		*psl = 0;

		++psl;
		++key;
		++value;

		// And shift backwards, until a zero or 1 is reached					
		// That way we move every entry that is far from its start			
		// closer to it. Moving is costly, unless you use pointers			
		try_again:
		while (*psl > 1) {
			psl[-1] = (*psl) - 1;

			#if LANGULUS_COMPILER_GCC()
				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Wplacement-new"
			#endif
				new (key - 1)   KeyTransfer {Move(*key)};
				new (value - 1) ValTransfer {Move(*value)};
			#if LANGULUS_COMPILER_GCC()
				#pragma GCC diagnostic pop
			#endif

			if constexpr (CT::Dense<K>)
				RemoveInner(key);
			if constexpr (CT::Dense<V>)
				RemoveInner(value);

			*psl = 0;

			++psl;
			++key;
			++value;
		}

		// Be aware, that psl might loop around									
		if (psl == pslEnd && *GetInfo() > 1) UNLIKELY() {
			psl = GetInfo();
			key = GetRawKeys();
			value = GetRawValues();

			// Shift first entry to the back											
			GetInfo()[mValues.mReserved] = (*psl) - 1;

			new (GetRawKeys() + mValues.mReserved)
				KeyTransfer {Move(*key)};
			new (GetRawValues() + mValues.mReserved)
				ValTransfer {Move(*value)};

			if constexpr (CT::Dense<K>)
				RemoveInner(key);
			if constexpr (CT::Dense<V>)
				RemoveInner(value);

			*psl = 0;

			++psl;
			++key;
			++value;

			// And continue the vicious cycle										
			goto try_again;
		}

		// Success																			
		--mValues.mCount;
	}

	/// Destroy a single value or key, either sparse or dense						
	///	@tparam T - the type to remove, either key or value (deducible)		
	///	@param element - the address of the element to remove						
	TABLE_TEMPLATE()
	template<class T>
	void TABLE()::RemoveInner(T* element) noexcept {
		if constexpr (CT::Destroyable<T>)
			element->~T();
	}

	/// Insert a single value or key, either sparse or dense							
	///	@tparam T - the type to add, either key or value (deducible)			
	///	@param element - the address of the element to remove						
	TABLE_TEMPLATE()
	template<class T>
	void TABLE()::Overwrite(T&& from, T& to) noexcept {
		// Remove the old entry															
		RemoveInner(&to);

		// Reconstruct the new one in place											
		new (&to) T {Forward<T>(from)};
	}

	/// Erase a pair via key																	
	///	@param key - the key to search for												
	///	@return the number of removed pairs												
	TABLE_TEMPLATE()
	Count TABLE()::RemoveKey(const K& match) {
		// Get the starting index based on the key hash							
		const auto start = GetBucket(match);
		auto key = GetRawKeys() + start;
		auto info = GetInfo() + start;
		const auto keyEnd = GetRawKeysEnd();

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
	///	@param value - the value to search for											
	///	@return the number of removed pairs												
	TABLE_TEMPLATE()
	Count TABLE()::RemoveValue(const V& match) {
		Count removed {};
		auto value = GetRawValues();
		auto info = GetInfo();
		const auto valueEnd = GetRawValuesEnd();

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
	TABLE_TEMPLATE()
	void TABLE()::Compact() {
		TODO();
	}

	///																								
	///	SEARCH																					
	///																								
	/// Search for a key inside the table													
	///	@param key - the key to search for												
	///	@return true if key is found, false otherwise								
	TABLE_TEMPLATE()
	bool TABLE()::ContainsKey(const K& key) const {
		return FindIndex(key) != GetReserved();
	}

	/// Search for a key inside the table, and return it if found					
	///	@param key - the key to search for												
	///	@return the index if key was found, or Index::None if not				
	TABLE_TEMPLATE()
	Index TABLE()::FindKeyIndex(const K& key) const {
		const auto offset = FindIndex(key);
		return offset != GetReserved() ? Index {offset} : Index::None;
	}

	/// Search for a value inside the table												
	///	@param value - the value to search for											
	///	@return true if value is found, false otherwise								
	TABLE_TEMPLATE()
	bool TABLE()::ContainsValue(const V& match) const {
		auto value = GetRawValues();
		auto info = GetInfo();
		const auto valueEnd = GetRawValuesEnd();

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
	TABLE_TEMPLATE()
	bool TABLE()::ContainsPair(const Pair& pair) const {
		const auto found = FindIndex(pair.mKey);
		return found != GetReserved() && GetValue(found) == pair.mValue;
	}

	/// Get a key by an unsafe offset (const)												
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the key													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetKey(const Offset& i) const noexcept {
		return GetRawKeys()[i];
	}

	/// Get a key by an unsafe offset 														
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the key													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetKey(const Offset& i) noexcept {
		return GetRawKeys()[i];
	}

	/// Get a value by an unsafe offset (const)											
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetValue(const Offset& i) const noexcept {
		return GetRawValues()[i];
	}

	/// Get a value by an unsafe offset 													
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetValue(const Offset& i) noexcept {
		return GetRawValues()[i];
	}

	/// Get a pair by an unsafe offset (const)											
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return the pair																		
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetPair(const Offset& i) const noexcept {
		return TPair<const K&, const V&> {GetKey(i), GetValue(i)};
	}

	/// Get a pair by an unsafe offset 														
	///	@attention as unsafe as it gets, for internal use only					
	///	@param i - the offset to use														
	///	@return the pair																		
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetPair(const Offset& i) noexcept {
		return TPair<K&, V&> {GetKey(i), GetValue(i)};
	}

	/// Returns a reference to the value found for key									
	/// Throws Except::OutOfRange if element cannot be found							
	///	@param key - the key to search for												
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::At(const K& key) {
		auto found = GetRawValues() + FindIndex(key);
		if (found == GetRawValuesEnd())
			Throw<Except::OutOfRange>("Key not found");
		return *found;
	}

	/// Returns a reference to the value found for key (const)						
	/// Throws Except::OutOfRange if element cannot be found							
	///	@param key - the key to search for												
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::At(const K& key) const {
		return const_cast<TABLE()&>(*this).At(key);
	}

	/// Get a key by a safe index (const)													
	///	@param index - the index to use													
	///	@return a reference to the key													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetKey(const Index& index) const {
		const_cast<TABLE()&>(*this).GetKey(index);
	}

	/// Get a key by a safe index 															
	///	@param index - the index to use													
	///	@return a reference to the key													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetKey(const Index& index) {
		const auto offset = index.GetOffset();
		if (offset >= GetReserved() || 0 == GetInfo()[offset])
			Throw<Except::OutOfRange>("Bad index for THashMap::GetKey");
		return GetKey(offset);
	}

	/// Get a value by a safe index (const)												
	///	@param index - the index to use													
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetValue(const Index& index) const {
		const_cast<TABLE()&>(*this).GetValue(index);
	}

	/// Get a value by a safe index 															
	///	@param index - the index to use													
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetValue(const Index& index) {
		const auto offset = index.GetOffset();
		if (offset >= GetReserved() || 0 == GetInfo()[offset])
			Throw<Except::OutOfRange>("Bad index for THashMap::GetValue");
		return GetValue(offset);
	}

	/// Get a pair by a safe index (const)													
	///	@param index - the index to use													
	///	@return the pair																		
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetPair(const Index& index) const {
		const_cast<TABLE()&>(*this).GetPair(index);
	}

	/// Get a pair by a safe index 															
	///	@param index - the index to use													
	///	@return the pair																		
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetPair(const Index& index) {
		const auto offset = index.GetOffset();
		if (offset >= GetReserved() || 0 == GetInfo()[offset])
			Throw<Except::OutOfRange>("Bad index for THashMap::GetPair");
		return GetPair(offset);
	}

	/// Find the index of a pair by key														
	///	@param key - the key to search for												
	///	@return the index																		
	TABLE_TEMPLATE()
	Offset TABLE()::FindIndex(const K& key) const {
		// Get the starting index based on the key hash							
		// Since reserved elements are always power-of-two, we use them	
		// as a mask to the hash, to extract the relevant bucket				
		const auto start = GetBucket(key);
		auto psl = GetInfo() + start;
		const auto pslEnd = GetInfoEnd() - 1;
		auto candidate = GetRawKeys() + start;
		Count attempts{};
		while (*psl > attempts) {
			if (*candidate != key) {
				// There might be more keys to the right, check them			
				if (psl == pslEnd) UNLIKELY() {
					// By 'to the right' I also mean looped back to start		
					psl = GetInfo();
					candidate = GetRawKeys();
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
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::operator[] (const K& key) const {
		return At(key);
	}

	/// Access value by key																		
	///	@param key - the key to find														
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::operator[] (const K& key) {
		return At(key);
	}

	/// Get the number of inserted pairs													
	///	@return the number of inserted pairs											
	TABLE_TEMPLATE()
	constexpr Count TABLE()::GetCount() const noexcept {
		return mValues.GetCount();
	}

	/// Get the number of allocated pairs													
	///	@return the number of allocated pairs											
	TABLE_TEMPLATE()
	constexpr Count TABLE()::GetReserved() const noexcept {
		return mValues.GetReserved();
	}

	/// Check if there are any pairs in this map											
	///	@return true if there's at least one pair available						
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsEmpty() const noexcept {
		return mValues.IsEmpty();
	}

	/// Check if the map has been allocated												
	///	@return true if the map uses dynamic memory									
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsAllocated() const noexcept {
		return mValues.IsAllocated();
	}

	/// Check if the memory for the table is owned by us								
	/// This is always true, since the map can't be initialized with outside	
	/// memory - the memory layout requirements are too strict to allow for it	
	///	@return true																			
	TABLE_TEMPLATE()
	constexpr bool TABLE()::HasAuthority() const noexcept {
		return IsAllocated();
	}

	/// Get the number of references for the allocated memory						
	///	@attention always returns zero if we don't have authority				
	///	@return the number of references													
	TABLE_TEMPLATE()
	constexpr Count TABLE()::GetUses() const noexcept {
		return mValues.GetUses();
	}

} // namespace Langulus::Anyness

#undef TABLE_TEMPLATE
#undef TABLE
