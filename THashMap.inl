#pragma once
#include "THashMap.hpp"
#include "inner/Hashing.hpp"

namespace Langulus::Anyness::Inner
{

	///																								
	///	Table implementation																	
	///																								

	/// Default constructor																		
	TABLE_TEMPLATE()
	TABLE()::Table() noexcept { }

	/// Creates an empty hash map. Nothing is allocated yet, this happens at	
	/// the first insert. This tremendously speeds up ctor &	dtor of a map		
	/// that never receives an element. The penalty is payed at the first		
	/// insert, and not before. Lookup of this empty map works because			
	/// everybody points to DummyInfoByte::b. parameter bucket_count is			
	/// dictated by the standard, but we can ignore it									
	TABLE_TEMPLATE()
	TABLE()::Table(size_t) noexcept { }

	///																								
	TABLE_TEMPLATE()
	template<class IT>
	TABLE()::Table(IT first, IT last, size_t) {
		Insert(first, last);
	}

	///																								
	TABLE_TEMPLATE()
	TABLE()::Table(std::initializer_list<Pair> initlist, size_t) {
		Insert(initlist.begin(), initlist.end());
	}

	///																								
	TABLE_TEMPLATE()
	TABLE()::Table(Table&& o) noexcept
		: DataPool(Forward<DataPool&>(o)) {
		if (!o.mMask)
			return;

		mHashMultiplier = Move(o.mHashMultiplier);
		mKeyVals = Move(o.mKeyVals);
		mInfo = Move(o.mInfo);
		mNumElements = Move(o.mNumElements);
		mMask = Move(o.mMask);
		mMaxNumElementsAllowed = Move(o.mMaxNumElementsAllowed);
		mInfoInc = Move(o.mInfoInc);
		mInfoHashShift = Move(o.mInfoHashShift);
		// Set other's mask to 0 so its destructor won't do anything		
		o.init();
	}

	///																								
	TABLE_TEMPLATE()
	TABLE()::Table(const Table& o)
		: DataPool(static_cast<const DataPool&>(o)) {
		//TODO this should reference instead of clone
		if (o.IsEmpty())
			return;

		// Not empty: create an exact copy. It is also possible to just	
		// iterate through all elements and insert them, but copying is	
		// probably faster																
		auto const numElementsWithBuffer = calcNumElementsWithBuffer(o.mMask + 1);
		auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);

		mHashMultiplier = o.mHashMultiplier;
		mKeyVals = static_cast<Node*>(assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));
		// No need for calloc because cloneData does memcpy					
		mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
		mNumElements = o.mNumElements;
		mMask = o.mMask;
		mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
		mInfoInc = o.mInfoInc;
		mInfoHashShift = o.mInfoHashShift;
		CloneInner(o);
	}

	TABLE_TEMPLATE()
	TABLE()::Table(Disowned<Table>&&) noexcept {

	}

	TABLE_TEMPLATE()
	TABLE()::Table(Abandoned<Table>&&) noexcept {

	}

	/// Destroys the map and all it's contents											
	TABLE_TEMPLATE()
	TABLE()::~Table() {
		destroy();
	}
	
	/// Checks if both tables contain the same entries									
	/// Order is irrelevant																		
	TABLE_TEMPLATE()
	bool TABLE()::operator == (const Table& other) const {
		if (other.GetCount() != GetCount())
			return false;

		for (auto const& otherEntry : other) {
			if (!has(otherEntry))
				return false;
		}

		return true;
	}

	/// Move a table																				
	///	@param o - the table to move														
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (Table&& o) noexcept {
		if (&o == this)
			return *this;

		if (o.mMask) {
			// Move stuff if the other map actually has some data				
			destroy();
			mHashMultiplier = Move(o.mHashMultiplier);
			mKeyVals = Move(o.mKeyVals);
			mInfo = Move(o.mInfo);
			mNumElements = Move(o.mNumElements);
			mMask = Move(o.mMask);
			mMaxNumElementsAllowed = Move(o.mMaxNumElementsAllowed);
			mInfoInc = Move(o.mInfoInc);
			mInfoHashShift = Move(o.mInfoHashShift);
			DataPool::operator = (Move(static_cast<DataPool&>(o)));
			o.init();
			return *this;
		}

		// Nothing in the other map => just clear it								
		Clear();
		return *this;
	}

	/// Creates a copy of the given map														
	/// Copy constructor of each entry is used											
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (Table const& o) {
		if (&o == this) {
			// Prevent assigning of itself											
			return *this;
		}

		// We keep using the old allocator and not assign the new one,		
		// because we want to keep the memory available. when it is the	
		// same size																		
		if (o.empty()) {
			if (0 == mMask) {
				// Nothing to do, we are empty too									
				return *this;
			}

			// Not empty: destroy what we have there clear also resets		
			// mInfo to 0, that's sometimes not necessary						
			destroy();
			init();
			DataPool::operator=(static_cast<DataPool const&>(o));
			return *this;
		}

		// Clean up old stuff															
		DestroyNodes<true>();

		if (mMask != o.mMask) {
			// No luck: we don't have the same array size allocated, so we	
			// need to realloc															
			if (0 != mMask) {
				// Only deallocate if we actually have data!						
				std::free(mKeyVals);
			}

			auto const numElementsWithBuffer = calcNumElementsWithBuffer(o.mMask + 1);
			auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);
			mKeyVals = static_cast<Node*>(assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));

			// No need for calloc here because cloneData performs a memcpy	
			mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
			// Sentinel is set in cloneData											
		}

		DataPool::operator = (static_cast<DataPool const&>(o));
		mHashMultiplier = o.mHashMultiplier;
		mNumElements = o.mNumElements;
		mMask = o.mMask;
		mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
		mInfoInc = o.mInfoInc;
		mInfoHashShift = o.mInfoHashShift;
		CloneInner(o);
		return *this;
	}

	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (Pair&& pair) noexcept {
		Clear();
		Emplace(Forward<Pair>(pair));
		return *this;
	}

	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (const Pair& pair) {
		Clear();
		Insert(pair);
		return *this;
	}
	
	///																								
	TABLE_TEMPLATE()
	void TABLE()::CloneInner(const Table& o) {
		if constexpr (IsOnHeap && std::is_trivially_copyable_v<Node>) {
			auto const* const src = reinterpret_cast<char const*>(o.mKeyVals);
			auto* tgt = reinterpret_cast<char*>(mKeyVals);
			auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);
			std::copy(src, src + calcNumBytesTotal(numElementsWithBuffer), tgt);
		}
		else {
			auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);
			std::copy(o.mInfo, o.mInfo + calcNumBytesInfo(numElementsWithBuffer), mInfo);

			for (size_t i = 0; i < numElementsWithBuffer; ++i) {
				if (mInfo[i])
					::new (static_cast<void*>(mKeyVals + i)) Node(*this, *o.mKeyVals[i]);
			}
		}
	}

	/// Clone the table																			
	///	@return the new table																
	TABLE_TEMPLATE()
	TABLE() TABLE()::Clone() const {
		if (IsEmpty())
			return {};

		Table result {Disown(*this)};
		auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);
		auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);
		result.mHashMultiplier = mHashMultiplier;
		result.mKeyVals = static_cast<Node*>(assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));
		result.mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
		result.mNumElements = mNumElements;
		result.mMask = mMask;
		result.mMaxNumElementsAllowed = mMaxNumElementsAllowed;
		result.mInfoInc = mInfoInc;
		result.mInfoHashShift = mInfoHashShift;
		result.CloneInner(*this);
		return result;
	}
	
	/// Templated maps are always typed														
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyUntyped() const noexcept {
		return false;
	}
	
	/// Templated maps are always typed														
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsValueUntyped() const noexcept {
		return false;
	}
	
	/// Templated maps are always type-constrained										
	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsKeyTypeConstrained() const noexcept {
		return true;
	}
	
	/// Templated maps are always type-constrained										
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
	TABLE()& TABLE()::operator << (Pair&& pair) {
		Emplace(Forward<Pair>(pair));
		return *this;
	}

	/// Copy-insert a pair inside the map													
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator << (const Pair& pair) {
		Insert(pair);
		return *this;
	}

	/// Reserves space for the specified number of elements. Makes sure the		
	/// data fits. Exactly the same as rehash(c). Use rehash(0)	to shrink-fit	
	///	@tparam REHASH - force rehash if true											
	///	@param c - number of elements to allocate										
	TABLE_TEMPLATE()
	template<bool REHASH>
	void TABLE()::Allocate(size_t c) {
		auto const minElementsAllowed = (std::max) (c, mNumElements);
		auto newSize = InitialNumElements;
		while (calcMaxNumElementsAllowed(newSize) < minElementsAllowed && newSize != 0)
			newSize *= 2;

		if (LANGULUS_UNLIKELY(newSize == 0))
			throwOverflowError();
				
		if constexpr (REHASH) {
			rehashPowerOfTwo(newSize, false);
		}
		else {
			// Only actually do anything when the new size is bigger than	
			// the old one. This prevents to continuously allocate for		
			// each reserve() call														
			if (newSize > mMask + 1)
				rehashPowerOfTwo<false>(newSize);
		}
	}

	/// Reserves space for the specified number of elements. Makes sure			
	/// the old data fits. Exactly the same as Allocate(c)							
	TABLE_TEMPLATE()
	void TABLE()::Rehash(size_t c) {
		Allocate<true>(c);
	}

	/// Insert a number of items																
	///	@param first - the first element													
	///	@param last - the last element													
	TABLE_TEMPLATE()
	template<class IT>
	void TABLE()::Insert(IT first, IT last) {
		for (; first != last; ++first) {
			// value_type ctor needed because this might be called			
			// with std::pair's															
			Insert(value_type(*first));
		}
	}

	/// Insert a number of items via initializer list									
	///	@param ilist - the first element													
	TABLE_TEMPLATE()
	void TABLE()::Insert(::std::initializer_list<Pair> ilist) {
		for (auto&& vt : ilist)
			Insert(Move(vt));
	}

	/// Emplace an items inside map															
	///	@param ...args - items to add														
	///	@return a pair containing the new pair & status of insertion			
	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::Insertion TABLE()::Emplace(Args&&... args) {
		Node n {*this, Forward<Args>(args)...};

		auto idxAndState = insertKeyPrepareEmptySpot(getFirstConst(n));
		switch (idxAndState.second) {
		case InsertionState::key_found:
			n.destroy(*this);
			break;

		case InsertionState::new_node:
			::new (static_cast<void*>(&mKeyVals[idxAndState.first]))
				Node(*this, Move(n));
			break;

		case InsertionState::overwrite_node:
			mKeyVals[idxAndState.first] = Move(n);
			break;

		case InsertionState::overflow_error:
			n.destroy(*this);
			throwOverflowError();
			break;
		}

		return std::make_pair(
			iterator(mKeyVals + idxAndState.first, mInfo + idxAndState.first),
			InsertionState::key_found != idxAndState.second
		);
	}

	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::iterator TABLE()::emplace_hint(const_iterator position, Args&&... args) {
		(void) position;
		return emplace(std::forward<Args>(args)...).first;
	}

	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::Insertion TABLE()::try_emplace(const K& key, Args&&... args) {
		return try_emplace_impl(key, std::forward<Args>(args)...);
	}

	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::Insertion TABLE()::try_emplace(K&& key, Args&&... args) {
		return try_emplace_impl(std::move(key), std::forward<Args>(args)...);
	}

	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::iterator TABLE()::try_emplace(const_iterator hint, const K& key, Args&&... args) {
		(void) hint;
		return try_emplace_impl(key, std::forward<Args>(args)...).first;
	}

	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::iterator TABLE()::try_emplace(const_iterator hint, K&& key, Args&&... args) {
		(void) hint;
		return try_emplace_impl(std::move(key), std::forward<Args>(args)...).first;
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	typename TABLE()::Insertion TABLE()::insert_or_assign(const K& key, Mapped&& obj) {
		return insertOrAssignImpl(key, std::forward<Mapped>(obj));
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	typename TABLE()::Insertion TABLE()::insert_or_assign(K&& key, Mapped&& obj) {
		return insertOrAssignImpl(Move(key), Forward<Mapped>(obj));
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	typename TABLE()::iterator TABLE()::insert_or_assign(const_iterator hint, const K& key, Mapped&& obj) {
		(void) hint;
		return insertOrAssignImpl(key, Forward<Mapped>(obj)).first;
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	typename TABLE()::iterator TABLE()::insert_or_assign(const_iterator hint, K&& key, Mapped&& obj) {
		(void) hint;
		return insertOrAssignImpl(Move(key), Forward<Mapped>(obj)).first;
	}

	TABLE_TEMPLATE()
	typename TABLE()::Insertion TABLE()::Insert(const Pair& keyval) {
		return Emplace(keyval);
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::Insert(const_iterator hint, const Pair& keyval) {
		(void) hint;
		return Emplace(keyval).first;
	}

	TABLE_TEMPLATE()
	typename TABLE()::Insertion TABLE()::Insert(Pair&& keyval) {
		return Emplace(Move(keyval));
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::Insert(const_iterator hint, Pair&& keyval) {
		(void) hint;
		return Emplace(Move(keyval)).first;
	}

	/// Inserts a keyval that is guaranteed to be new, e.g. when the				
	/// hashmap is resized																		
	TABLE_TEMPLATE()
	void TABLE()::insert_move(Node&& keyval) {
		// We don't retry, fail if overflowing don't need to check max		
		// num elements																	
		if (0 == mMaxNumElementsAllowed && !try_increase_info())
			throwOverflowError();

		size_t idx {};
		InfoType info {};
		keyToIdx(keyval.getFirst(), &idx, &info);

		// Skip forward																	
		// Use <= because we are certain that the element is not there		
		while (info <= mInfo[idx]) {
			idx = idx + 1;
			info += mInfoInc;
		}

		// Key not found, so we are now exactly where we want to insert it
		auto const insertion_idx = idx;
		auto const insertion_info = static_cast<uint8_t>(info);
		if (LANGULUS_UNLIKELY(insertion_info + mInfoInc > 0xFF))
			mMaxNumElementsAllowed = 0;

		// Find an empty spot															
		while (0 != mInfo[idx])
			next(&info, &idx);

		auto& l = mKeyVals[insertion_idx];
		if (idx == insertion_idx) {
			::new (static_cast<void*>(&l)) Node(std::move(keyval));
		}
		else {
			shiftUp(idx, insertion_idx);
			l = std::move(keyval);
		}

		// Put at empty spot																
		mInfo[insertion_idx] = insertion_info;
		++mNumElements;
	}

	TABLE_TEMPLATE()
	template<class OtherKey, class... Args>
	typename TABLE()::Insertion TABLE()::try_emplace_impl(OtherKey&& key, Args&&... args) {
		auto idxAndState = insertKeyPrepareEmptySpot(key);
		switch (idxAndState.second) {
		case InsertionState::key_found:
			break;

		case InsertionState::new_node:
			::new (static_cast<void*>(&mKeyVals[idxAndState.first])) Node(
				*this, std::piecewise_construct, std::forward_as_tuple(std::forward<OtherKey>(key)),
				std::forward_as_tuple(Forward<Args>(args)...));
			break;

		case InsertionState::overwrite_node:
			mKeyVals[idxAndState.first] = Node(*this, std::piecewise_construct,
				std::forward_as_tuple(Forward<OtherKey>(key)),
				std::forward_as_tuple(Forward<Args>(args)...));
			break;

		case InsertionState::overflow_error:
			throwOverflowError();
			break;
		}

		return std::make_pair(
			iterator(mKeyVals + idxAndState.first, mInfo + idxAndState.first),
			InsertionState::key_found != idxAndState.second
		);
	}

	TABLE_TEMPLATE()
	template<class OtherKey, class Mapped>
	typename TABLE()::Insertion TABLE()::insertOrAssignImpl(OtherKey&& key, Mapped&& obj) {
		auto idxAndState = insertKeyPrepareEmptySpot(key);
		switch (idxAndState.second) {
		case InsertionState::key_found:
			mKeyVals[idxAndState.first].getSecond() = Forward<Mapped>(obj);
			break;

		case InsertionState::new_node:
			::new (static_cast<void*>(&mKeyVals[idxAndState.first])) Node(
				*this, 
				std::piecewise_construct, 
				std::forward_as_tuple(Forward<OtherKey>(key)),
				std::forward_as_tuple(Forward<Mapped>(obj)));
			break;

		case InsertionState::overwrite_node:
			mKeyVals[idxAndState.first] = Node(
				*this, 
				std::piecewise_construct,
				std::forward_as_tuple(Forward<OtherKey>(key)),
				std::forward_as_tuple(Forward<Mapped>(obj)));
			break;

		case InsertionState::overflow_error:
			throwOverflowError();
			break;
		}

		return std::make_pair(
			iterator(mKeyVals + idxAndState.first, mInfo + idxAndState.first),
			InsertionState::key_found != idxAndState.second
		);
	}

	/// Finds key, and if not already present prepares a spot where to pot the	
	/// key & value. This potentially shifts nodes out of the way, updates		
	/// mInfo and number of inserted elements, so the only operation left to	
	/// do is create/assign a new node at that spot										
	TABLE_TEMPLATE()
	template <class OtherKey>
	std::pair<size_t, typename TABLE()::InsertionState> TABLE()::insertKeyPrepareEmptySpot(OtherKey&& key) {
		for (int i = 0; i < 256; ++i) {
			size_t idx {};
			InfoType info {};
			keyToIdx(key, &idx, &info);
			nextWhileLess(&info, &idx);

			// While we potentially have a match									
			while (info == mInfo[idx]) {
				static_assert(CT::Comparable<Key, OtherKey>, "Can't compare keys");
				if (key == mKeyVals[idx].getFirst()) {
					// Key already exists, do NOT insert							
					return std::make_pair(idx, InsertionState::key_found);
				}

				next(&info, &idx);
			}

			// Unlikely that this evaluates to true								
			if (LANGULUS_UNLIKELY(mNumElements >= mMaxNumElementsAllowed)) {
				if (!increase_size())
					return std::make_pair(size_t(0), InsertionState::overflow_error);

				continue;
			}

			// Key not found, so we are now exactly where we want to			
			// insert it																	
			auto const insertion_idx = idx;
			auto const insertion_info = info;
			if (LANGULUS_UNLIKELY(insertion_info + mInfoInc > 0xFF))
				mMaxNumElementsAllowed = 0;

			// Find an empty spot														
			while (0 != mInfo[idx])
				next(&info, &idx);

			if (idx != insertion_idx)
				shiftUp(idx, insertion_idx);

			// Put at empty spot															
			mInfo[insertion_idx] = static_cast<uint8_t>(insertion_info);
			++mNumElements;
			return std::make_pair(
				insertion_idx, 
				idx == insertion_idx
				? InsertionState::new_node
				: InsertionState::overwrite_node
			);
		}

		// Enough attempts failed, so finally give up							
		return std::make_pair(size_t(0), InsertionState::overflow_error);
	}

	/// Clears all data, without resizing													
	TABLE_TEMPLATE()
	void TABLE()::Clear() {
		if (IsEmpty())
			return;

		DestroyNodes<true>();

		auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);

		// Clear everything, then set the sentinel again						
		uint8_t const z = 0;
		std::fill(mInfo, mInfo + calcNumBytesInfo(numElementsWithBuffer), z);
		mInfo[numElementsWithBuffer] = 1;
		mInfoInc = InitialInfoInc;
		mInfoHashShift = InitialInfoHashShift;
	}

	/// Clears all data and deallocates														
	TABLE_TEMPLATE()
	void TABLE()::Reset() {
		DestroyNodes<true>();
		Base::reset();
		::new (this) TABLE();
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::RemoveIndex(const_iterator pos) {
		// its safe to perform const cast here
		return erase(iterator {const_cast<Node*>(pos.mKeyVals), const_cast<uint8_t*>(pos.mInfo)});
	}

	/// Erases element at pos, returns iterator to the next element				
	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::RemoveIndex(iterator pos) {
		// we assume that pos always points to a valid entry, and not end().
		auto const idx = static_cast<size_t>(pos.mKeyVals - mKeyVals);
		shiftDown(idx);
		--mNumElements;

		if (*pos.mInfo) {
			// we've backward shifted, return this again
			return pos;
		}

		// no backward shift, return next element
		return ++pos;
	}

	/// Erase a pair																		
	///	@param key - the key to search for										
	///	@return the number of removed pairs										
	TABLE_TEMPLATE()
	size_t TABLE()::RemoveKey(const K& key) {
		static_assert(CT::Comparable<Key>, "Can't compare keys");
		size_t idx {};
		InfoType info {};
		keyToIdx(key, &idx, &info);

		// Check while info matches with the source idx					
		do {
			if (info == mInfo[idx] && key == mKeyVals[idx].getFirst()) {
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

	/// Erase a pair																		
	///	@param key - the key to search for										
	///	@return the number of removed pairs										
	TABLE_TEMPLATE()
	size_t TABLE()::RemoveValue(const V& value) {
		static_assert(CT::Comparable<Key>, "Can't compare keys");
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

	/// If possible reallocates the map to a smaller one. This frees the			
	/// underlying table. Does not do anything if load_factor is too				
	/// large for decreasing the table's size.											
	TABLE_TEMPLATE()
	void TABLE()::compact() {
		auto newSize = InitialNumElements;
		while (calcMaxNumElementsAllowed(newSize) < mNumElements && newSize != 0)
			newSize *= 2;

		if (LANGULUS_UNLIKELY(newSize == 0))
			throwOverflowError();

		// Only actually do anything when the new size is bigger				
		// than the old one. This prevents to continuously allocate			
		// for each reserve() call														
		if (newSize < mMask + 1)
			rehashPowerOfTwo(newSize, true);
	}

	/// Destroy contents																			
	TABLE_TEMPLATE()
	void TABLE()::destroy() {
		if (0 == mMask) {
			// Don't deallocate!															
			return;
		}

		DestroyNodes<false>();

		// This protection against not deleting mMask shouldn't be needed	
		// as it's sufficiently protected with the 0==mMask check, but I	
		// have this anyways because g++ 7 otherwise reports a compile		
		// error: attempt to free a non-heap object 'fm'						
		// [-Werror=free-nonheap-object]												
		//if (mKeyVals != reinterpret_cast_no_cast_align_warning<Node*>(&mMask))
			std::free(mKeyVals);
	}

	///																								
	///	SEARCH																					
	///																								
	/// Returns 1 if key is found, 0 otherwise											
	TABLE_TEMPLATE()
	size_t TABLE()::count(const K& key) const {
		auto kv = mKeyVals + findIdx(key);
		if (kv != reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			return 1;
		return 0;
	}

	/*TABLE_TEMPLATE()
	template<typename OtherKey, typename Self_>
	size_t TABLE()::count(const OtherKey& key) const requires IsTransparent<Self_> {
		auto kv = mKeyVals + findIdx(key);
		if (kv != reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			return 1;
		return 0;
	}*/

	TABLE_TEMPLATE()
	bool TABLE()::contains(const K& key) const {
		return 1U == count(key);
	}

	/*TABLE_TEMPLATE()
	template <typename OtherKey, typename Self_>
	bool TABLE()::contains(const OtherKey& key) const requires IsTransparent<Self_> {
		return 1U == count(key);
	}*/

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	V& TABLE()::at(const K& key) {
		auto kv = mKeyVals + findIdx(key);
		if (kv == reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			doThrow<std::out_of_range>("key not found");
		return kv->getSecond();
	}

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	const V& TABLE()::at(const K& key) const {
		auto kv = mKeyVals + findIdx(key);
		if (kv == reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			doThrow<std::out_of_range>("key not found");
		return kv->getSecond();
	}

	/// Find																							
	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::find(const K& key) const {
		const size_t idx = findIdx(key);
		return const_iterator {mKeyVals + idx, mInfo + idx};
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::find(const K& key) {
		const size_t idx = findIdx(key);
		return iterator {mKeyVals + idx, mInfo + idx};
	}

	/// Copy of find(), except that it returns iterator instead of					
	/// const_iterator																			
	TABLE_TEMPLATE()
	template<class Other>
	size_t TABLE()::findIdx(Other const& key) const {
		static_assert(CT::Comparable<Other, Key>, "Can't compare keys");
		size_t idx {};
		InfoType info {};
		keyToIdx(key, &idx, &info);

		do {
			// Unrolling this twice gives a bit of a speedup					
			// More unrolling did not help											
			if (info == mInfo[idx] && LANGULUS_LIKELY(key == mKeyVals[idx].getFirst()))
				return idx;
			next(&info, &idx);
			if (info == mInfo[idx] && LANGULUS_LIKELY(key == mKeyVals[idx].getFirst()))
				return idx;
			next(&info, &idx);
		}
		while (info <= mInfo[idx]);

		// If reached, then nothing found											
		return mMask == 0 ? 0
			: static_cast<size_t>(std::distance(mKeyVals, reinterpret_cast_no_cast_align_warning<Node*>(mInfo)));
	}

	/// Access value by key																		
	///	@param key - the key to find														
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	const V& TABLE()::operator[] (const K& key) const {
		return at(key);
	}

	/// Access value by key																		
	///	@param key - the key to find														
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	V& TABLE()::operator[] (const K& key) {
		return at(key);
	}
	
	/// Get the beginning of internal data													
	///	@return the iterator																	
	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::begin() {
		if (IsEmpty())
			return end();
		return iterator(mKeyVals, mInfo, fast_forward_tag {});
	}

	/// Get the beginning of internal data (const)										
	///	@return the iterator																	
	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::begin() const {
		return cbegin();
	}

	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::cbegin() const {
		if (IsEmpty())
			return cend();
		return const_iterator(mKeyVals, mInfo, fast_forward_tag {});
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::end() {
		// No need to supply valid info pointer: end() must not be			
		// dereferenced, and only node pointer is compared						
		return iterator {reinterpret_cast_no_cast_align_warning<Node*>(mInfo), nullptr};
	}

	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::end() const {
		return cend();
	}

	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::cend() const {
		return const_iterator {reinterpret_cast_no_cast_align_warning<Node*>(mInfo), nullptr};
	}

	TABLE_TEMPLATE()
	constexpr Count TABLE()::GetCount() const noexcept {
		return mNumElements;
	}

	TABLE_TEMPLATE()
	constexpr Count TABLE()::max_size() const noexcept {
		return static_cast<Count>(-1);
	}

	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsEmpty() const noexcept {
		return 0 == mNumElements;
	}

	TABLE_TEMPLATE()
	constexpr bool TABLE()::IsAllocated() const noexcept {
		return Base::IsAllocated();
	}

	TABLE_TEMPLATE()
	constexpr float TABLE()::max_load_factor() const noexcept {
		return MaxLoadFactor100 / 100.0f;
	}

	/// Average number of elements per bucket. Since we allow only 1 per bucket
	TABLE_TEMPLATE()
	constexpr float TABLE()::load_factor() const noexcept {
		return static_cast<float>(GetCount()) / static_cast<float>(mMask + 1);
	}

	TABLE_TEMPLATE()
	size_t TABLE()::mask() const noexcept {
		return mMask;
	}

	TABLE_TEMPLATE()
	size_t TABLE()::calcMaxNumElementsAllowed(size_t maxElements) const noexcept {
		if (LANGULUS_LIKELY(maxElements <= (std::numeric_limits<size_t>::max)() / 100))
			return maxElements * MaxLoadFactor100 / 100;

		// We might be a bit inprecise, but since maxElements is quite		
		// large that doesn't matter													
		return (maxElements / 100) * MaxLoadFactor100;
	}

	TABLE_TEMPLATE()
	size_t TABLE()::calcNumBytesInfo(size_t numElements) const noexcept {
		// We add a uint64_t, which houses the sentinel (first byte) and	
		// padding so we can load 64bit types										
		return numElements + sizeof(uint64_t);
	}

	TABLE_TEMPLATE()
	size_t TABLE()::calcNumElementsWithBuffer(size_t numElements) const noexcept {
		auto maxNumElementsAllowed = calcMaxNumElementsAllowed(numElements);
		return numElements + (::std::min) (maxNumElementsAllowed, (static_cast<size_t>(0xFF)));
	}

	/// Calculation only allowed for 2^n values											
	TABLE_TEMPLATE()
	size_t TABLE()::calcNumBytesTotal(size_t numElements) const {
		#if LANGULUS(BITNESS) == 64
			return numElements * sizeof(Node) + calcNumBytesInfo(numElements);
		#else
			// make sure we're doing 64bit operations, so we are at least safe against 32bit overflows.
			auto const ne = static_cast<uint64_t>(numElements);
			auto const s = static_cast<uint64_t>(sizeof(Node));
			auto const infos = static_cast<uint64_t>(calcNumBytesInfo(numElements));

			auto const total64 = ne * s + infos;
			auto const total = static_cast<size_t>(total64);

			if (LANGULUS_UNLIKELY(static_cast<uint64_t>(total) != total64))
				throwOverflowError();

			return total;
		#endif
	}

	/// Helpers for insertKeyPrepareEmptySpot: extract first entry					
	/// (only const required)																	
	TABLE_TEMPLATE()
	typename TABLE()::Key const& TABLE()::getFirstConst(const Node& node) const noexcept {
		return node.getFirst();
	}

	/// In case we have void mapped_type, we are not using a pair, thus we		
	/// just route k through. No need to disable this because it's just not		
	/// used if not applicable																	
	TABLE_TEMPLATE()
	typename TABLE()::Key const& TABLE()::getFirstConst(const K& key) const noexcept {
		return key;
	}

	/// In case we have non-void mapped_type, we have a standard					
	/// robin_hood::pair																			
	TABLE_TEMPLATE()
	template <CT::Data Q>
	typename TABLE()::Key const& TABLE()::getFirstConst(const Pair& pair) const noexcept {
		return pair.mKey;
	}

	/// Destroyer																					
	TABLE_TEMPLATE()
	template<bool DEALLOCATE>
	void TABLE()::DestroyNodes() noexcept {
		mNumElements = 0;

		if constexpr (Method == AllocationMethod::Stack || !::std::is_trivially_destructible_v<Node>) {
			// Clear also resets mInfo to 0, that's sometimes not				
			// necessary																	
			auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);
			for (size_t idx = 0; idx < numElementsWithBuffer; ++idx) {
				if (0 != mInfo[idx]) {
					Node& n = mKeyVals[idx];
					if constexpr (DEALLOCATE)
						n.destroy(*this);
					else
						n.destroyDoNotDeallocate();
					n.~Node();
				}
			}
		}
	}

	/// Highly performance relevant code													
	/// Lower bits are used for indexing into the array (2^n size)					
	/// The upper 1-5 bits need to be a reasonable good hash, to save				
	/// comparisons																				
	TABLE_TEMPLATE()
	template<class HashKey>
	void TABLE()::keyToIdx(HashKey&& key, size_t* idx, InfoType* info) const {
		auto h = static_cast<uint64_t>(HashData(key));

		// In addition to whatever hash is used, add another mul &			
		// shift so we get better hashing. This serves as a bad				
		// hash prevention, if the given data is badly mixed.					
		h *= mHashMultiplier;
		h ^= h >> 33U;

		// The lower InitialInfoNumBits are reserved for info					
		*info = mInfoInc + static_cast<InfoType>((h & InfoMask) >> mInfoHashShift);
		*idx = (static_cast<size_t>(h) >> InitialInfoNumBits) & mMask;
	}

	/// Forwards the index by one, wrapping around at the end						
	TABLE_TEMPLATE()
	void TABLE()::next(InfoType* info, size_t* idx) const noexcept {
		*idx = *idx + 1;
		*info += mInfoInc;
	}

	TABLE_TEMPLATE()
	void TABLE()::nextWhileLess(InfoType* info, size_t* idx) const noexcept {
		// Unrolling this by hand did not bring any speedups					
		while (*info < mInfo[*idx])
			next(info, idx);
	}

	/// Shift everything up by one element													
	/// Tries to move stuff around															
	TABLE_TEMPLATE()
	void TABLE()::shiftUp(size_t startIdx, size_t const insertion_idx) noexcept(std::is_nothrow_move_assignable_v<Node>) {
		auto idx = startIdx;
		::new (static_cast<void*>(mKeyVals + idx)) Node(Move(mKeyVals[idx - 1]));
		while (--idx != insertion_idx)
			mKeyVals[idx] = Move(mKeyVals[idx - 1]);

		idx = startIdx;
		while (idx != insertion_idx) {
			mInfo[idx] = static_cast<uint8_t>(mInfo[idx - 1] + mInfoInc);
			if (LANGULUS_UNLIKELY(mInfo[idx] + mInfoInc > 0xFF))
				mMaxNumElementsAllowed = 0;
			--idx;
		}
	}

	TABLE_TEMPLATE()
	void TABLE()::shiftDown(size_t idx) noexcept(std::is_nothrow_move_assignable_v<Node>) {
		// Until we find one that is either empty or has zero offset		
		// TODO(martinus) we don't need to move everything, just				
		// the last one for the same bucket											
		mKeyVals[idx].destroy(*this);

		// Until we find one that is either empty or has zero offset		
		while (mInfo[idx + 1] >= 2 * mInfoInc) {
			mInfo[idx] = static_cast<uint8_t>(mInfo[idx + 1] - mInfoInc);
			mKeyVals[idx] = Move(mKeyVals[idx + 1]);
			++idx;
		}

		mInfo[idx] = 0;
		// Don't destroy, we've moved it
		// mKeyVals[idx].destroy(*this);
		mKeyVals[idx].~Node();
	}

} // namespace Langulus::Anyness::Inner
