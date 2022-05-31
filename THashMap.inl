///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "THashMap.hpp"
#include "inner/Hashing.hpp"

#define TABLE_TEMPLATE() template<CT::Data K, CT::Data V>
#define TABLE() THashMap<K, V>

namespace Langulus::Anyness
{

	/// Manual construction via an initializer list										
	///	@param initlist - the initializer list to forward							
	TABLE_TEMPLATE()
	TABLE()::THashMap(::std::initializer_list<Pair> initlist)
		: THashMap{} {
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
		, mValues {Move(other.mValues)} { }

	/// Shallow-copy construction without referencing									
	///	@param other - the disowned table to copy										
	TABLE_TEMPLATE()
	TABLE()::THashMap(Disowned<THashMap>&& other) noexcept
		: mKeys {other.mKeys}
		, mInfo {other.mInfo}
		, mValues {Disown(other.mValue.mValues)} { }

	/// Minimal move construction from abandoned table									
	///	@param other - the abandoned table to move									
	TABLE_TEMPLATE()
	TABLE()::THashMap(Abandoned<THashMap>&& other) noexcept
		: mKeys {other.mKeys}
		, mInfo {other.mInfo}
		, mValues {Abandon(other.mValue.mValues)} { }

	/// Destroys the map and all it's contents											
	TABLE_TEMPLATE()
	TABLE()::~THashMap() {
		if (GetUses() == 1) {
			// Values will be deallocated, so deallocate keys too				
			Inner::Allocator::Deallocate(mKeys);
		}
	}

	/// Checks if both tables contain the same entries									
	/// Order is irrelevant																		
	///	@param other - the table to compare against									
	///	@return true if tables match														
	TABLE_TEMPLATE()
	bool TABLE()::operator == (const THashMap& other) const {
		if (other.GetCount() != GetCount())
			return false;

		for (const auto& otherEntry : other) {
			if (!ContainsPair(otherEntry))
				return false;
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
		new (this) TABLE() {Move(rhs)};
		return *this;
	}

	/// Creates a shallow copy of the given table										
	///	@param rhs - the table to reference												
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (const THashMap& rhs) {
		// Always reference, before dereferencing, in the rare case that	
		// this table is the same as the other										
		rhs.mValues.Keep();

		Reset();
		new (this) TABLE() {rhs};
		return *this;
	}

	/// Emplace a single pair into a cleared map											
	///	@param pair - the pair to emplace												
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (Pair&& pair) noexcept {
		Reset();
		Emplace(Forward<Pair>(pair));
		return *this;
	}

	/// Insert a single pair into a cleared map											
	///	@param pair - the pair to copy													
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (const Pair& pair) {
		Reset();
		Insert(pair);
		return *this;
	}

	/// Clone the table																			
	///	@return the new table																
	TABLE_TEMPLATE()
	TABLE() TABLE()::Clone() const {
		if (IsEmpty())
			return {};

		THashMap result;
		result.mKeys = Inner::Allocator::Allocate(mKeys->GetAllocatedSize());

		// Clone the keys																	
		using KD = Decay<K>;
		auto from = GetRawKeys();
		const auto fromEnd = GetRawKeysEnd();
		auto to = result.GetRawKeys();

		if constexpr (CT::Sparse<K>) {
			TAny<KD> coalesced;
			coalesced.Allocate(GetCount());

			// Clone data behind each valid pointer								
			Count counter {};
			while (from < fromEnd) {
				if (!*from) {
					// Skip zero pointers												
					*to = nullptr;
					++from; ++to;
					continue;
				}
				
				if constexpr (CT::CloneMakable<K>)
					new (&coalesced[counter]) KD {(*from)->Clone()};
				else if constexpr (CT::POD<K>)
					::std::memcpy(&coalesced[counter], **from, sizeof(KD));
				else
					LANGULUS_ASSERT("Can't clone map keys made of non-clonable/non-POD type");
				
				*to = &coalesced[counter];
				++from; ++to;
				++counter;
			}
			
			coalesced.Reference(counter);
		}
		else if constexpr (CT::CloneMakable<K>) {
			// Clone dense keys by their Clone() methods							
			while (from < fromEnd) {
				new (to) KD {from->Clone()};
				++from; ++to;
			}
		}
		else if constexpr (CT::POD<K>) {
			// Batch clone dense keys at once, together with tombstones		
			::std::memcpy(to, from, mKeysAndTombstones->GetAllocatedSize());

			// Clone the values and early return									
			result.mValues = mValues.Clone();
			return Abandon(result);
		}
		else
			LANGULUS_ASSERT("Can't clone map keys made of non-clonable/non-POD type");

		// Clone the tombstones															
		::std::memcpy(to, from, GetCount());

		// Clone the values																
		result.mValues = mValues.Clone();
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

	/// Get the size of a single pair, in bytes											
	///	@return the number of bytes a single pair contains							
	TABLE_TEMPLATE()
	constexpr Size TABLE()::GetPairStride() const noexcept {
		return sizeof(Pair); 
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
	constexpr const K* TABLE()::GetRawKeys() const noexcept {
		return const_cast<TABLE()*>(this)->GetRawKeys();
	}

	/// Get the raw key array																	
	TABLE_TEMPLATE()
	constexpr K* TABLE()::GetRawKeys() noexcept {
		return reinterpret_cast<K*>(mKeysAndTombstones->GetBlockStart());
	}

	/// Get the end of the raw key array													
	TABLE_TEMPLATE()
	constexpr const K* TABLE()::GetRawKeysEnd() const noexcept {
		return reinterpret_cast<const K*>(mInfo);
	}

	/// Get the raw value array (const)														
	TABLE_TEMPLATE()
	constexpr const V* TABLE()::GetRawValues() const noexcept {
		return mValues.GetRaw();
	}

	/// Get the raw value array																
	TABLE_TEMPLATE()
	constexpr V* TABLE()::GetRawValues() noexcept {
		return mValues.GetRaw();
	}

	/// Get end of the raw value array														
	TABLE_TEMPLATE()
	constexpr const V* TABLE()::GetRawValuesEnd() const noexcept {
		return mValues.GetRawEnd();
	}

	/// Get the size of all pairs, in bytes												
	///	@return the total amount of initialized bytes								
	TABLE_TEMPLATE()
	constexpr Size TABLE()::GetSize() const noexcept {
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
		Emplace(Forward<Pair>(item));
		return *this;
	}

	/// Copy-insert a pair inside the map													
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator << (const Pair& item) {
		Insert(item);
		return *this;
	}

	/// Request a new size of keys and tombstones via the value container		
	///	@return the requested byte size													
	TABLE_TEMPLATE()
	Size TABLE()::RequestKeyAndTombstoneSize() const noexcept {
		return Roof2(mValues.GetReserved() * sizeof(K)) + mValues.GetReserved();
	}

	/// Get the tombstone array end															
	///	@return a pointer to the end of the array										
	TABLE_TEMPLATE()
	const uint8_t* TABLE()::GetInfoEnd() const noexcept {
		return GetInfo() + mValues.GetReserved();
	}

	/// Get the tombstone array (const)														
	///	@return a pointer to the first element inside the tombstone array		
	TABLE_TEMPLATE()
	const uint8_t* TABLE()::GetInfo() const noexcept {
		return mInfo;
	}

	/// Get the tombstone array																
	///	@return a pointer to the first element inside the tombstone array		
	TABLE_TEMPLATE()
	uint8_t* TABLE()::GetInfo() noexcept {
		return mInfo;
	}

	/// Reserves space for the specified number of pairs								
	///	@attention does nothing if reserving less than current reserve			
	///	@param count - number of pairs to allocate									
	TABLE_TEMPLATE()
	void TABLE()::Allocate(const Count& count) {
		// Shrinking is never allowed, you'll have to do it explicitly 	
		// via Compact()																	
		if (count < GetReserved())
			return;

		const auto oldReserve = mValues.GetReserved();

		// Allocate/Reallocate the keys and tombstones							
		if (mValues.IsAllocated()) {
			// Reallocate																	
			auto oldKeys = GetRawKeys();
			const auto oldInfo = GetInfo();
			const auto oldEntry = mKeys;
			const auto oldUses = mValues.GetUses();

			// Allocate values first, we'll use their properties				
			mValues.Allocate<false>(count);

			if (oldUses == 1) {
				// Memory is used only once and it is safe to move it			
				mKeys = Inner::Allocator::Reallocate(
					RequestKeyAndTombstoneSize(), mKeys
				);

				// Precalculate the info pointer, it's costly					
				mInfo = reinterpret_cast<uint8_t*>(
					mKeys->GetBlockStart() +
					Roof2(mValues.GetReserved() * sizeof(K))
				);

				if (mKeys != oldEntry) {
					// Copy the tombstones												
					::std::memcpy(mInfo, oldInfo, oldReserve);

					// Keys moved, and we should call move-construction		
					if constexpr (CT::Sparse<K> || CT::POD<K>) {
						// Copy pointers/POD												
						const auto size = sizeof(K) * oldReserve;
						::std::memcpy(GetRawKeys(), oldKeys, size);
					}
					else {
						// Call the move-constructor for each key					
						static_assert(CT::MoveMakable<K>,
							"Trying to move-construct key but it's impossible for this type");

						auto to = GetRawKeys();
						const auto toEnd = GetRawKeysEnd();
						while (to != toEnd) {
							new (to) K {Move(*oldKeys)};
							if constexpr (CT::Destroyable<K>)
								oldKeys->~K();
							++oldKeys; ++to;
						}
					}

					// Destroy the old entry, it had one reference and is		
					// no longer in use													
					Inner::Allocator::Deallocate(oldEntry);
				}
				else {
					// Keys didn't move, but tombstones always do				
					// Just make sure they're copied safely						
					::std::memmove(mInfo, oldInfo, oldReserve);
				}
			}
			else {
				// Memory is used from multiple locations, and we must		
				// copy the memory for this block - we can't move it!			
				mKeys = Inner::Allocator::Allocate(RequestKeyAndTombstoneSize());

				// Precalculate the info pointer, it's costly					
				mInfo = reinterpret_cast<uint8_t*>(
					mKeys->GetBlockStart() +
					Roof2(mValues.GetReserved() * sizeof(K))
				);

				// Move the tombstones													
				::std::memcpy(mInfo, oldInfo, oldReserve);

				// We should call copy-construction for each key				
				if constexpr (CT::Sparse<K> || CT::POD<K>) {
					// Copy pointers/POD													
					const auto size = sizeof(K) * oldReserve;
					::std::memcpy(GetRawKeys(), oldKeys, size);

					if constexpr (CT::Sparse<K>) {
						// Since we're copying pointers, we have to reference	
						// dense memory behind each one of them					
						const auto oldKeysEnd = oldKeys + oldReserve;
						while (oldKeys != oldKeysEnd) {
							// Reference each pointer									
							Inner::Allocator::Keep(GetKeyType(), *oldKeys, 1);
							++oldKeys;
						}
					}
				}
				else {
					// Call the move-constructor for each key						
					static_assert(CT::CopyMakable<K>,
						"Trying to copy-construct key but it's impossible for this type");

					auto to = GetRawKeys();
					const auto toEnd = GetRawKeysEnd();
					while (to != toEnd) {
						new (to) K {*oldKeys};
						++oldKeys; ++to;
					}
				}

				// Dereference the old keys											
				oldEntry->Free();
			}

			if (oldReserve) {
				// The old tombstones remain, so a rehash is required			
				TODO();
			}
		}
		else {
			// Allocate a fresh set of elements										
			// Allocate values first, we'll use their properties				
			mValues.Allocate<false>(count);
			mKeys = Inner::Allocator::Allocate(RequestKeyAndTombstoneSize());

			// Precalculate the info pointer, it's costly						
			mInfo = reinterpret_cast<uint8_t*>(
				mKeys->GetBlockStart() +
				Roof2(mValues.GetReserved() * sizeof(K))
			);

			// Zero the tombstones														
			// No need for a rehash, because map was empty						
			::std::memset(mInfo, 0, mValues.GetReserved());
		}
	}

	/// Insert a number of items via initializer list									
	///	@param ilist - the first element													
	TABLE_TEMPLATE()
	Count TABLE()::Insert(::std::initializer_list<Pair> ilist) {
		Count result {};
		for (auto&& i : ilist)
			result += Insert(Move(i));
		return result;
	}

	/// Emplace a single pair inside table													
	///	@param ...args - items to add														
	///	@return a pair containing the first new item & status of insertion	
	TABLE_TEMPLATE()
	template<class... Args>
	Count TABLE()::Emplace(Args&&... args) {
		TODO();
	}

	TABLE_TEMPLATE()
	Count TABLE()::Insert(const Pair& item) {
		return Emplace(item);
	}

	TABLE_TEMPLATE()
	Count TABLE()::Insert(Pair&& item) {
		return Emplace(Forward<Pair>(item));
	}

	/// Clears all data, without resizing													
	TABLE_TEMPLATE()
	void TABLE()::Clear() {
		if (!mValues.IsAllocated())
			return;
		mValues.Clear();
		::std::memset(mInfo, 0, mValues.GetReserved());
	}

	/// Clears all data and deallocates														
	TABLE_TEMPLATE()
	void TABLE()::Reset() {
		if (mValues.GetUses() == 1)
			Inner::Allocator::Deallocate(mKeys);
		mValues.Reset();
	}

	/// Erases element at index, returns iterator to the next element				
	TABLE_TEMPLATE()
	void TABLE()::RemoveIndex(const Offset& idx) {
		// We assume that pos always points to a valid entry, not end()	
		shiftDown(idx);
		--mNumElements;
	}

	/// Erase a pair via key																	
	///	@param key - the key to search for												
	///	@return the number of removed pairs												
	TABLE_TEMPLATE()
	Count TABLE()::RemoveKey(const K& key) {
		size_t idx {};
		InfoType info {};
		keyToIdx(key, &idx, &info);

		// Check while info matches with the source idx							
		do {
			if (info == mInfo[idx] && key == GetKey(idx)) {
				shiftDown(idx);
				--mNumElements;
				return 1;
			}

			next(&info, &idx);
		}
		while (info <= mInfo[idx]);

		// Nothing found to delete														
		return 0;
	}

	/// Erase all pairs with a given value													
	///	@param value - the value to search for											
	///	@return the number of removed pairs												
	TABLE_TEMPLATE()
	Count TABLE()::RemoveValue(const V& value) {
		Count removed{};
		auto it = begin();
		while (it != end()) {
			if (it->mValue == value) {
				it = RemoveIndex(it);
				++removed;
			}
			else ++it;
		}

		return removed;
	}

	/// If possible reallocates the map to a smaller one								
	TABLE_TEMPLATE()
	void TABLE()::Compact() {
		auto newSize = InitialNumElements;
		while (GetMaxElementsAllowed(newSize) < mNumElements && newSize != 0)
			newSize *= 2;

		if (LANGULUS_UNLIKELY(newSize == 0))
			Throw<Except::Overflow>("Table overflow");

		// Only actually do anything when the new size is bigger				
		// than the old one. This prevents to continuously allocate			
		// for each reserve() call														
		if (newSize < mMask + 1)
			rehashPowerOfTwo(newSize);
	}

	///																								
	///	SEARCH																					
	///																								
	/// Search for a key inside the table													
	///	@return true if key is found, false otherwise								
	TABLE_TEMPLATE()
	bool TABLE()::ContainsKey(const K& key) const {
		const auto found = mNodes + FindIndex(key);
		return found != mNodesEnd;
	}

	/// Search for a value inside the table												
	///	@return true if value is found, false otherwise								
	TABLE_TEMPLATE()
	bool TABLE()::ContainsValue(const V& value) const {
		auto it = begin();
		while (it != end()) {
			if (it->mValue == value)
				return true;
			++it;
		}

		return false;
	}

	/// Search for a pair inside the table													
	///	@return true if pair is found, false otherwise								
	TABLE_TEMPLATE()
	bool TABLE()::ContainsPair(const Pair& e) const {
		const auto found = Find(e.mKey);
		return found != end() && found->mValue == e.mValue;
	}

	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetKey(const Offset& i) const noexcept {
		return GetRawKeys()[i];
	}

	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetKey(const Offset& i) noexcept {
		return GetRawKeys()[i];
	}

	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetValue(const Offset& i) const noexcept {
		return GetRawValues()[i];
	}

	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetValue(const Offset& i) noexcept {
		return GetRawValues()[i];
	}

	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetPair(const Offset& i) const noexcept {
		return TPair<const K&, const V&> {GetKey(i), GetValue(i)};
	}

	TABLE_TEMPLATE()
	decltype(auto) TABLE()::GetPair(const Offset& i) noexcept {
		return TPair<K&, V&> {GetKey(i), GetValue(i)};
	}

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::At(const K& key) {
		auto found = mNodes + FindIndex(key);
		if (found == mNodesEnd)
			Throw<Except::OutOfRange>("Key not found");

		return (*found)->mValue;
	}

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::At(const K& key) const {
		return const_cast<TABLE()>(*this).At(key);
	}

	/// Find a value by key																		
	TABLE_TEMPLATE()
	decltype(auto) TABLE()::Find(const K& key) const {
		const auto idx = FindIndex(key);
		return {mNodes + idx, mInfo + idx};
	}

	/// Find the index of a value by key													
	TABLE_TEMPLATE()
	Offset TABLE()::FindIndex(const K& key) const {
		size_t idx {};
		InfoType info {};
		keyToIdx(key, &idx, &info);

		do {
			// Unrolling this twice gives a bit of a speedup					
			// More unrolling did not help											
			if (info == mInfo[idx] && LANGULUS_LIKELY(key == GetKey(idx)))
				return idx;

			next(&info, &idx);

			if (info == mInfo[idx] && LANGULUS_LIKELY(key == GetKey(idx)))
				return idx;

			next(&info, &idx);
		}
		while (info <= mInfo[idx]);

		// If reached, then nothing found											
		return mMask == 0 ? 0 : static_cast<size_t>(mNodesEnd - mNodes);
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
