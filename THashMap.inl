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
		, mValues {Move(other.mValues)} { }

	/// Shallow-copy construction without referencing									
	///	@param other - the disowned table to copy										
	TABLE_TEMPLATE()
	TABLE()::THashMap(Disowned<THashMap>&& other) noexcept
		: mKeys {other.mValue.mKeys}
		, mInfo {other.mValue.mInfo}
		, mValues {Disown(other.mValue.mValues)} { }

	/// Minimal move construction from abandoned table									
	///	@param other - the abandoned table to move									
	TABLE_TEMPLATE()
	TABLE()::THashMap(Abandoned<THashMap>&& other) noexcept
		: mKeys {other.mValue.mKeys}
		, mInfo {other.mValue.mInfo}
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
	void TABLE()::CloneInner(const uint8_t* info, const T* from, const T* fromEnd, T* to) {
		using TD = Decay<T>;

		if constexpr (CT::Sparse<T>) {
			TAny<TD> coalesced;
			coalesced.Allocate(GetCount());

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
				if (!*info) {
					// Skip uninitialized elements									
					++from; ++to; ++info;
					continue;
				}

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

		// Allocate keys																	
		Offset infoOffset;
		result.mKeys = Inner::Allocator::Allocate(
			RequestKeyAndInfoSize(GetReserved(), infoOffset)
		);

		// Precalculate the info pointer, it's costly							
		result.mInfo = reinterpret_cast<uint8_t*>(
			result.mKeys->GetBlockStart() + infoOffset
		);

		// Clone the info bytes															
		::std::memcpy(result.GetInfo(), GetInfo(), GetReserved() + 1);

		// Clone the keys and values													
		CloneInner(GetInfo(), GetRawKeys(), GetRawKeysEnd(), result.GetRawKeys());
		CloneInner(GetInfo(), GetRawValues(), GetRawValuesEnd(), result.GetRawValues());

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
	constexpr const K* TABLE()::GetRawKeys() const noexcept {
		return const_cast<TABLE()*>(this)->GetRawKeys();
	}

	/// Get the raw key array																	
	TABLE_TEMPLATE()
	constexpr K* TABLE()::GetRawKeys() noexcept {
		return reinterpret_cast<K*>(mKeys->GetBlockStart());
	}

	/// Get the end of the raw key array													
	TABLE_TEMPLATE()
	constexpr const K* TABLE()::GetRawKeysEnd() const noexcept {
		return GetRawKeys() + GetReserved();
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
		return mValues.GetRaw() + GetReserved();
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
		constexpr Size alignment {LANGULUS(ALIGN)};
		const Size keymemory = request * sizeof(K);
		infoStart = keymemory + alignment - (keymemory % alignment);
		return infoStart + request + 1;
	}

	/// Get the tombstone array end															
	///	@return a pointer to the end of the array										
	TABLE_TEMPLATE()
	uint8_t* TABLE()::GetSentinel() noexcept {
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
		AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
	}

	/// Allocate or reallocate key and info array										
	///	@attention assumes count is a power-of-two									
	///	@attention assumes mValues has not been yet reallocated					
	///	@tparam REUSE - true to reallocate, false to allocate fresh				
	///	@param count - the new number of keys and info bytes						
	TABLE_TEMPLATE()
	template<bool REUSE>
	void TABLE()::AllocateKeys(const Count& count) {
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

		// Precalculate the info pointer, it's costly							
		auto oldInfo = mInfo;
		mInfo = reinterpret_cast<uint8_t*>(
			mKeys->GetBlockStart() + infoOffset
		);

		// Zero the info array, we'll be rehashing it anyways					
		::std::memset(mInfo, 0, count);
		*GetSentinel() = 1;

		if (IsEmpty()) {
			// Nothing else to do														
			if (oldKeys != mKeys)
				Inner::Allocator::Deallocate(oldKeys);
			return;
		}

		if (oldKeys != mKeys) {
			// Keys moved																	
			// Reinsert each one of them to rehash									
			const auto oldInfoEnd = oldInfo + GetReserved();
			auto key = oldKeys->As<K>();
			while (oldInfo != oldInfoEnd) {
				if (0 == *oldInfo) {
					++key; ++oldInfo;
					continue;
				}

				if constexpr (REUSE)
					InsertKey(Move(*key));
				else
					InsertKey(*key);
				++key; ++oldInfo;
			}

			// Free the old entry														
			Inner::Allocator::Deallocate(oldKeys);
		}
		else {
			// Keys remain in the same place, swap them around to rehash	
			TODO();
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
		if (count < GetReserved())
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

	/// Insert a number of items via initializer list									
	///	@param ilist - the first element													
	TABLE_TEMPLATE()
	Count TABLE()::Insert(::std::initializer_list<Pair> ilist) {
		Allocate(GetCount() + ilist.size());
		Count result {};
		for (auto&& i : ilist)
			result += Insert(Move(i));
		return result;
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
		Pair swapper {item};

		// Get the starting index based on the key hash							
		const auto start = HashData(swapper.mKey) & (GetReserved() - 1);
		auto psl = GetInfo() + start;
		auto candidate = GetRawKeys() + start;
		uint8_t attempts {1};
		while (*psl) {
			if (*candidate == swapper.mKey) {
				// Neat, the key already exists - just set value and go		
				const auto index = psl - GetInfo();
				Overwrite(Move(swapper.mValue), GetValue(index));
				return 1;
			}

			if (attempts > *psl) {
				// The pair we're inserting is closer to bucket, so swap		
				const auto index = psl - GetInfo();
				::std::swap(Move(GetKey(index)), Move(swapper.mKey));
				::std::swap(Move(GetValue(index)), Move(swapper.mValue));
				::std::swap(attempts, *psl);
			}

			++attempts; ++psl; ++candidate;

			// Shouldn't really happen, like ever (?), because we always	
			// allocate more than we need, and we rehash on each realloc,	
			// spreading the values and lowering the required PSLs			
			//TODO test this extensively, just in case							
			SAFETY(if (attempts == 255u)
				Throw<Except::Overflow>("Map PSL overflow")
			);
		}

		// If reached, empty slot reached, so put the pair there				
		// Might not seem like it, but we gave a guarantee, that this is	
		// eventually reached, unless key exists and returns early			
		const auto index = psl - GetInfo();
		new (&GetKey(index)) K {Move(swapper.mKey)};
		new (&GetValue(index)) V {Move(swapper.mValue)};
		*psl = attempts;
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

		// Get the starting index based on the key hash							
		const auto start = HashData(item.mKey) & (GetReserved() - 1);
		auto psl = GetInfo() + start;
		auto candidate = GetRawKeys() + start;
		uint8_t attempts {1};
		while (*psl) {
			if (*candidate == item.mKey) {
				// Neat, the key already exists - just set value and go		
				const auto index = psl - GetInfo();
				Overwrite(Move(item.mValue), GetValue(index));
				return 1;
			}

			if (attempts > *psl) {
				// The pair we're inserting is closer to bucket, so swap		
				const auto index = psl - GetInfo();
				::std::swap(Move(GetKey(index)), Move(item.mKey));
				::std::swap(Move(GetValue(index)), Move(item.mValue));
				::std::swap(attempts, *psl);
			}

			++attempts; ++psl; ++candidate;

			// Shouldn't really happen, like ever (?), because we always	
			// allocate more than we need, and we rehash on each realloc,	
			// spreading the values and lowering the required PSLs			
			//TODO test this extensively, just in case							
			SAFETY(if (attempts == 255u)
				Throw<Except::Overflow>("Map PSL overflow")
			);
		}

		// If reached, empty slot reached, so put the pair there				
		// Might not seem like it, but we gave a guarantee, that this is	
		// eventually reached, unless key exists and returns early			
		const auto index = psl - GetInfo();
		new (&GetKey(index)) K {Move(item.mKey)};
		new (&GetValue(index)) V {Move(item.mValue)};
		*psl = attempts;
		return 1;
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

	/// Erases element at a specific index													
	///	@attention assumes that index points to a valid entry						
	///	@param start - the index to remove												
	TABLE_TEMPLATE()
	void TABLE()::RemoveIndex(const Offset& start) noexcept {
		auto psl = GetInfo() + start;
		auto candidate = GetRawKeys() + start;
		auto value = GetRawValues() + start;

		// Destroy the key, info and value there									
		RemoveInner<V>(value);
		RemoveInner<K>(candidate);
		*psl = 0;

		++psl;
		++candidate;
		++value;

		// And shift backwards, until a zero or 1 is reached					
		// That way we move every entry that is far from its start			
		// closer to it. Moving is costly, unless you use pointers			
		while (*psl > 1) {
			psl[-1] = (*psl) - 1;
			new (candidate - 1) K {Move(*candidate)};
			new (value - 1) V {Move(*value)};

			++psl;
			++candidate;
			++value;
		}

		// Success																			
		--const_cast<Count&>(mValues.GetCount());
	}

	/// Destroy a single value or key, either sparse or dense						
	///	@tparam T - the type to remove, either key or value (deducible)		
	///	@param element - the address of the element to remove						
	TABLE_TEMPLATE()
	template<class T>
	void TABLE()::RemoveInner(T* element) noexcept {
		using TD = Decay<T>;
		if constexpr (CT::Sparse<T>) {
			// Value is sparse, free and deallocate if ours						
			auto entry = Inner::Allocator::Find(MetaData::Of<T>(), *element);
			if (entry) {
				if (entry->GetUses() == 1) {
					(*element)->~TD();
					Inner::Allocator::Deallocate(entry);
				}
				else entry->Free();
			}
		}
		else if constexpr (CT::Destroyable<T>) {
			// Value is dense, just call destructor								
			element->~TD();
		}
	}

	/// Insert a single value or key, either sparse or dense							
	///	@tparam T - the type to add, either key or value (deducible)			
	///	@param element - the address of the element to remove						
	TABLE_TEMPLATE()
	template<class T>
	void TABLE()::Overwrite(T&& from, T& to) noexcept {
		// Remove the old entry															
		RemoveInner(&to);

		if constexpr (CT::Sparse<T>) {
			// Value is sparse, reference it if ours								
			auto entry = Inner::Allocator::Find(MetaData::Of<T>(), from);
			if (entry)
				entry->Keep();
			to = from;
			from = nullptr;
		}
		else {
			// Value is dense, reconstruct it in place							
			using TD = Decay<T>;
			new (&to) TD {Forward<T>(from)};
		}
	}

	/// Erase a pair via key																	
	///	@param key - the key to search for												
	///	@return the number of removed pairs												
	TABLE_TEMPLATE()
	Count TABLE()::RemoveKey(const K& key) {
		// Get the starting index based on the key hash							
		auto start = HashData(key) & (GetReserved() - 1);
		auto psl = GetInfo() + start;
		auto candidate = GetRawKeys() + start;
		while (*psl > 1) {
			if (*candidate != key) {
				// There might be more keys to the right, check them			
				++psl;
				++candidate;
				continue;
			}

			// Match found, destroy the key, info and value there				
			auto value = GetRawValues() + (psl - GetInfo());
			RemoveInner<V>(value);
			RemoveInner<K>(candidate);
			*psl = 0;

			++psl;
			++candidate;
			++value;

			// And shift backwards, until a zero or 1 is reached				
			// That way we move every entry that is far from its start		
			// closer to it. Moving is costly, unless you use pointers		
			while (*psl > 1) {
				psl[-1] = *psl - 1;
				new (candidate - 1) K {Move(*candidate)};
				new (value - 1) V {Move(*value)};
				++psl;
				++candidate;
				++value;
			}

			// Success																		
			--const_cast<Count&>(mValues.GetCount());
			return 1;
		}

		// Nothing found to delete														
		return 0;
	}

	/// Erase all pairs with a given value													
	///	@param value - the value to search for											
	///	@return the number of removed pairs												
	TABLE_TEMPLATE()
	Count TABLE()::RemoveValue(const V& value) {
		Count removed {};
		auto it = GetRawValues();
		const auto end = GetRawValuesEnd();
		while (it != end) {
			if (*it == value) {
				RemoveIndex(it - GetRawValues());
				++removed;
			}
			else ++it;
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

	/// Search for a value inside the table												
	///	@param value - the value to search for											
	///	@return true if value is found, false otherwise								
	TABLE_TEMPLATE()
	bool TABLE()::ContainsValue(const V& value) const {
		auto it = GetRawValues();
		const auto end = GetRawValuesEnd();
		while (it != end) {
			if (*it == value)
				return true;
			++it;
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
		return const_cast<TABLE()>(*this).At(key);
	}

	/// Find the index of a pair by key														
	///	@param key - the key to search for												
	///	@return the index																		
	TABLE_TEMPLATE()
	Offset TABLE()::FindIndex(const K& key) const {
		// Get the starting index based on the key hash							
		// Since reserved elements are always power-of-two, we use them	
		// as a mask to the hash, to extract the relevant bucket				
		auto start = HashData(key) & (GetReserved() - 1);
		auto psl = GetInfo() + start;
		auto candidate = GetRawKeys() + start;
		while (*psl > 1) {
			if (*candidate != key) {
				// There might be more keys to the right, check them			
				++psl;
				++candidate;
				continue;
			}

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
