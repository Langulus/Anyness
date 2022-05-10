#pragma once
#include "THashMap.hpp"

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
		insert(first, last);
	}

	///																								
	TABLE_TEMPLATE()
	TABLE()::Table(std::initializer_list<value_type> initlist, size_t) {
		insert(initlist.begin(), initlist.end());
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
	
	///																								
	TABLE_TEMPLATE()
	void TABLE()::CloneInner(const Table& o) {
		if constexpr (IsFlat && std::is_trivially_copyable_v<Node>) {
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
		Table result;
		result.CloneInner(*this);
		return result;
	}

	TABLE_TEMPLATE()
	DMeta TABLE()::GetKeyType() const {
		return MetaData::Of<Key>();
	}

	TABLE_TEMPLATE()
	DMeta TABLE()::GetValueType() const {
		return MetaData::Of<T>();
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
				rehashPowerOfTwo(newSize, false);
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
	TABLE()::Insertion TABLE()::Emplace(Args&&... args) {
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
	TABLE()::iterator TABLE()::emplace_hint(const_iterator position, Args&&... args) {
		(void) position;
		return emplace(std::forward<Args>(args)...).first;
	}

	TABLE_TEMPLATE()
	template<class... Args>
	TABLE()::Insertion TABLE()::try_emplace(const key_type& key, Args&&... args) {
		return try_emplace_impl(key, std::forward<Args>(args)...);
	}

	TABLE_TEMPLATE()
	template<class... Args>
	TABLE()::Insertion TABLE()::try_emplace(key_type&& key, Args&&... args) {
		return try_emplace_impl(std::move(key), std::forward<Args>(args)...);
	}

	TABLE_TEMPLATE()
	template<class... Args>
	TABLE()::iterator TABLE()::try_emplace(const_iterator hint, const key_type& key, Args&&... args) {
		(void) hint;
		return try_emplace_impl(key, std::forward<Args>(args)...).first;
	}

	TABLE_TEMPLATE()
	template<class... Args>
	TABLE()::iterator TABLE()::try_emplace(const_iterator hint, key_type&& key, Args&&... args) {
		(void) hint;
		return try_emplace_impl(std::move(key), std::forward<Args>(args)...).first;
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	TABLE()::Insertion TABLE()::insert_or_assign(const key_type& key, Mapped&& obj) {
		return insertOrAssignImpl(key, std::forward<Mapped>(obj));
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	TABLE()::Insertion TABLE()::insert_or_assign(key_type&& key, Mapped&& obj) {
		return insertOrAssignImpl(Move(key), Forward<Mapped>(obj));
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	TABLE()::iterator TABLE()::insert_or_assign(const_iterator hint, const key_type& key, Mapped&& obj) {
		(void) hint;
		return insertOrAssignImpl(key, Forward<Mapped>(obj)).first;
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	TABLE()::iterator TABLE()::insert_or_assign(const_iterator hint, key_type&& key, Mapped&& obj) {
		(void) hint;
		return insertOrAssignImpl(Move(key), Forward<Mapped>(obj)).first;
	}

	TABLE_TEMPLATE()
	TABLE()::Insertion TABLE()::Insert(const value_type& keyval) {
		return Emplace(keyval);
	}

	TABLE_TEMPLATE()
	TABLE()::iterator TABLE()::Insert(const_iterator hint, const value_type& keyval) {
		(void) hint;
		return Emplace(keyval).first;
	}

	TABLE_TEMPLATE()
	TABLE()::Insertion TABLE()::Insert(value_type&& keyval) {
		return Emplace(Move(keyval));
	}

	TABLE_TEMPLATE()
	TABLE()::iterator TABLE()::Insert(const_iterator hint, value_type&& keyval) {
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
	TABLE()::Insertion TABLE()::try_emplace_impl(OtherKey&& key, Args&&... args) {
		auto idxAndState = insertKeyPrepareEmptySpot(key);
		switch (idxAndState.second) {
		case InsertionState::key_found:
			break;

		case InsertionState::new_node:
			::new (static_cast<void*>(&mKeyVals[idxAndState.first])) Node(
				*this, std::piecewise_construct, std::forward_as_tuple(std::forward<OtherKey>(key)),
				std::forward_as_tuple(std::forward<Args>(args)...));
			break;

		case InsertionState::overwrite_node:
			mKeyVals[idxAndState.first] = Node(*this, std::piecewise_construct,
				std::forward_as_tuple(std::forward<OtherKey>(key)),
				std::forward_as_tuple(std::forward<Args>(args)...));
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
	TABLE()::Insertion TABLE()::insertOrAssignImpl(OtherKey&& key, Mapped&& obj) {
		auto idxAndState = insertKeyPrepareEmptySpot(key);
		switch (idxAndState.second) {
		case InsertionState::key_found:
			mKeyVals[idxAndState.first].getSecond() = std::forward<Mapped>(obj);
			break;

		case InsertionState::new_node:
			::new (static_cast<void*>(&mKeyVals[idxAndState.first])) Node(
				*this, std::piecewise_construct, std::forward_as_tuple(std::forward<OtherKey>(key)),
				std::forward_as_tuple(std::forward<Mapped>(obj)));
			break;

		case InsertionState::overwrite_node:
			mKeyVals[idxAndState.first] = Node(*this, std::piecewise_construct,
				std::forward_as_tuple(std::forward<OtherKey>(key)),
				std::forward_as_tuple(std::forward<Mapped>(obj)));
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
				static_assert(IsComparable<Key, OtherKey>, "Can't compare keys");
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
			return std::make_pair(insertion_idx, idx == insertion_idx
				? InsertionState::new_node
				: InsertionState::overwrite_node);
		}

		// Enough attempts failed, so finally give up							
		return std::make_pair(size_t(0), InsertionState::overflow_error);
	}

	/// Clears all data, without resizing													
	TABLE_TEMPLATE()
	void TABLE()::Clear() {
		if (IsEmpty()) {
			// Don't do anything! also important because we don't want to	
			// write to DummyInfoByte::b, even though we would just write	
			// 0 to it																		
			return;
		}

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
		Clear();
	}

	TABLE_TEMPLATE()
	TABLE()::iterator TABLE()::RemoveIndex(const_iterator pos) {
		// its safe to perform const cast here
		return erase(iterator {const_cast<Node*>(pos.mKeyVals), const_cast<uint8_t*>(pos.mInfo)});
	}

	/// Erases element at pos, returns iterator to the next element				
	TABLE_TEMPLATE()
	TABLE()::iterator TABLE()::RemoveIndex(iterator pos) {
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
	size_t TABLE()::RemoveKey(const key_type& key) {
		static_assert(IsComparable<Key>, "Can't compare keys");
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
	size_t TABLE()::count(const key_type& key) const {
		auto kv = mKeyVals + findIdx(key);
		if (kv != reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			return 1;
		return 0;
	}

	TABLE_TEMPLATE()
	template<typename OtherKey, typename Self_>
	size_t TABLE()::count(const OtherKey& key) const requires IsTransparent<Self_> {
		auto kv = mKeyVals + findIdx(key);
		if (kv != reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			return 1;
		return 0;
	}

	TABLE_TEMPLATE()
	bool TABLE()::contains(const key_type& key) const {
		return 1U == count(key);
	}

	TABLE_TEMPLATE()
	template <typename OtherKey, typename Self_>
	bool TABLE()::contains(const OtherKey& key) const requires IsTransparent<Self_> {
		return 1U == count(key);
	}

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	template<ReflectedData Q>
	Q& TABLE()::at(key_type const& key) {
		auto kv = mKeyVals + findIdx(key);
		if (kv == reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			doThrow<std::out_of_range>("key not found");
		return kv->getSecond();
	}

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	template<ReflectedData Q>
	Q const& TABLE()::at(key_type const& key) const {
		auto kv = mKeyVals + findIdx(key);
		if (kv == reinterpret_cast_no_cast_align_warning<Node*>(mInfo))
			doThrow<std::out_of_range>("key not found");
		return kv->getSecond();
	}

	/// Find																							
	TABLE_TEMPLATE()
	template<class ALT_K>
	TABLE()::const_iterator TABLE()::find(const ALT_K& key) const {
		const size_t idx = findIdx(key);
		return const_iterator {mKeyVals + idx, mInfo + idx};
	}

	TABLE_TEMPLATE()
	template<class ALT_K>
	TABLE()::iterator TABLE()::find(const ALT_K& key) {
		const size_t idx = findIdx(key);
		return iterator {mKeyVals + idx, mInfo + idx};
	}

	/// Copy of find(), except that it returns iterator instead of					
	/// const_iterator																			
	TABLE_TEMPLATE()
	template<class Other>
	size_t TABLE()::findIdx(Other const& key) const {
		static_assert(IsComparable<Other, Key>, "Can't compare keys");
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
	template <ReflectedData Q>
	Q& TABLE()::operator[] (const key_type& key) {
		auto idxAndState = insertKeyPrepareEmptySpot(key);
		switch (idxAndState.second) {
		case InsertionState::key_found:
			break;

		case InsertionState::new_node:
			::new (static_cast<void*>(&mKeyVals[idxAndState.first])) 
				Node(*this, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple());
			break;

		case InsertionState::overwrite_node:
			mKeyVals[idxAndState.first] = 
				Node(*this, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple());
			break;

		case InsertionState::overflow_error:
			throwOverflowError();
		}

		return mKeyVals[idxAndState.first].getSecond();
	}

	/// Access value by key																		
	///	@param key - the key to find														
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	template<ReflectedData Q>
	Q& TABLE()::operator[] (key_type&& key) {
		auto idxAndState = insertKeyPrepareEmptySpot(key);
		switch (idxAndState.second) {
		case InsertionState::key_found:
			break;

		case InsertionState::new_node:
			::new (static_cast<void*>(&mKeyVals[idxAndState.first])) 
				Node(*this, std::piecewise_construct, std::forward_as_tuple(Move(key)), std::forward_as_tuple());
			break;

		case InsertionState::overwrite_node:
			mKeyVals[idxAndState.first] = 
				Node(*this, std::piecewise_construct, std::forward_as_tuple(Move(key)), std::forward_as_tuple());
			break;

		case InsertionState::overflow_error:
			throwOverflowError();
		}

		return mKeyVals[idxAndState.first].getSecond();
	}
	
	/// Get the beginning of internal data													
	///	@return the iterator																	
	TABLE_TEMPLATE()
	TABLE()::iterator TABLE()::begin() {
		if (IsEmpty())
			return end();
		return iterator(mKeyVals, mInfo, fast_forward_tag {});
	}

	/// Get the beginning of internal data (const)										
	///	@return the iterator																	
	TABLE_TEMPLATE()
	TABLE()::const_iterator TABLE()::begin() const {
		return cbegin();
	}

	TABLE_TEMPLATE()
	TABLE()::const_iterator TABLE()::cbegin() const {
		if (IsEmpty())
			return cend();
		return const_iterator(mKeyVals, mInfo, fast_forward_tag {});
	}

	TABLE_TEMPLATE()
	TABLE()::iterator TABLE()::end() {
		// No need to supply valid info pointer: end() must not be			
		// dereferenced, and only node pointer is compared						
		return iterator {reinterpret_cast_no_cast_align_warning<Node*>(mInfo), nullptr};
	}

	TABLE_TEMPLATE()
	TABLE()::const_iterator TABLE()::end() const {
		return cend();
	}

	TABLE_TEMPLATE()
	TABLE()::const_iterator TABLE()::cend() const {
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
		return numElements + (std::min) (maxNumElementsAllowed, (static_cast<size_t>(0xFF)));
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

} // namespace Langulus::Anyness::Inner
