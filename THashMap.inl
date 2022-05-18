///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "THashMap.hpp"
#include "inner/Hashing.hpp"

namespace Langulus::Anyness::Inner
{

	/// Default constructor																		
	TABLE_TEMPLATE()
	constexpr TABLE()::Table() noexcept {
		mNodes = reinterpret_cast<Node*>(&mMask);
		mInfo = reinterpret_cast<uint8_t*>(&mMask);
	}

	/// Manual construction via range														
	TABLE_TEMPLATE()
	template<class IT>
	TABLE()::Table(IT first, IT last)
		: Table {} {
		Insert(first, last);
	}

	/// Manual construction via an initializer list										
	///	@param initlist - the initializer list to forward							
	TABLE_TEMPLATE()
	TABLE()::Table(std::initializer_list<Pair> initlist)
		: Table {} {
		Insert(initlist.begin(), initlist.end());
	}

	/// Move construction																		
	///	@param other - the table to move													
	TABLE_TEMPLATE()
	TABLE()::Table(Table&& other) noexcept
		: mHashMultiplier {other.mHashMultiplier}
		, mEntry {other.mEntry}
		, mNumElements {other.mNumElements}
		, mMask {other.mMask}
		, mMaxNumElementsAllowed {other.mMaxNumElementsAllowed}
		, mInfoInc {other.mInfoInc}
		, mInfoHashShift {other.mInfoHashShift} {
		// Special care for unions														
		if (mMask) {
			mNodes = other.mNodes;
			mInfo = other.mInfo;
		}
		else {
			mNodes = reinterpret_cast<Node*>(&mMask);
			mInfo = reinterpret_cast<uint8_t*>(&mMask);
		}

		other.init();
	}

	/// Shallow-copy construction																
	///	@param other - the table to copy													
	TABLE_TEMPLATE()
	TABLE()::Table(const Table& other)
		: mHashMultiplier {other.mHashMultiplier}
		, mEntry {other.mEntry}
		, mNumElements {other.mNumElements}
		, mMask {other.mMask}
		, mMaxNumElementsAllowed {other.mMaxNumElementsAllowed}
		, mInfoInc {other.mInfoInc}
		, mInfoHashShift {other.mInfoHashShift} {
		// Special care for unions														
		if (mMask) {
			mNodes = other.mNodes;
			mInfo = other.mInfo;
		}
		else {
			mNodes = reinterpret_cast<Node*>(&mMask);
			mInfo = reinterpret_cast<uint8_t*>(&mMask);
		}

		mEntry->Keep();
	}

	/// Shallow-copy construction without referencing									
	///	@param other - the disowned table to copy										
	TABLE_TEMPLATE()
	TABLE()::Table(Disowned<Table>&& other) noexcept
		: mHashMultiplier {other.mValue.mHashMultiplier}
		, mEntry {other.mValue.mEntry}
		, mNumElements {other.mValue.mNumElements}
		, mMask {other.mValue.mMask}
		, mMaxNumElementsAllowed {other.mValue.mMaxNumElementsAllowed}
		, mInfoInc {other.mValue.mInfoInc}
		, mInfoHashShift {other.mValue.mInfoHashShift} {
		// Special care for unions														
		if (mMask) {
			mNodes = other.mValue.mNodes;
			mInfo = other.mValue.mInfo;
		}
		else {
			mNodes = reinterpret_cast<Node*>(&mMask);
			mInfo = reinterpret_cast<uint8_t*>(&mMask);
		}
	}

	/// Minimal move construction from abandoned table									
	///	@param other - the abandoned table to move									
	TABLE_TEMPLATE()
	TABLE()::Table(Abandoned<Table>&& other) noexcept
		: mHashMultiplier {other.mValue.mHashMultiplier}
		, mEntry {other.mValue.mEntry}
		, mNumElements {other.mValue.mNumElements}
		, mMask {other.mValue.mMask}
		, mMaxNumElementsAllowed {other.mValue.mMaxNumElementsAllowed}
		, mInfoInc {other.mValue.mInfoInc}
		, mInfoHashShift {other.mValue.mInfoHashShift} {
		// Special care for unions														
		if (mMask) {
			mNodes = other.mValue.mNodes;
			mInfo = other.mValue.mInfo;
		}
		else {
			mNodes = reinterpret_cast<Node*>(&mMask);
			mInfo = reinterpret_cast<uint8_t*>(&mMask);
		}

		// Clear only the mask, so destructor does nothing, since 'other'	
		// is guaranteed to not be used anymore because abandoned			
		other.mValue.mMask = 0;
	}

	/// Destroys the map and all it's contents											
	TABLE_TEMPLATE()
	TABLE()::~Table() {
		destroy();
	}
	
	/// Reserves space for at least the specified number of elements				
	/// Only works if numBuckets is power-of-two											
	///	@param numBuckets - the number of buckets										
	TABLE_TEMPLATE()
	void TABLE()::rehashPowerOfTwo(size_t numBuckets) {
		// These will be reset via initData, so back them up					
		auto const oldEntry = mEntry;
		auto const oldNodes = mNodes;
		auto const oldInfo = mInfo;
		auto const oldMaxElementsWithBuffer = GetElementsWithBuffer(mMask + 1);

		// Resize and move stuff														
		initData(numBuckets);

		if (oldMaxElementsWithBuffer > 1) {
			for (size_t i = 0; i < oldMaxElementsWithBuffer; ++i) {
				if (oldInfo[i] == 0)
					continue;

				// Might throw an exception, which is really bad				
				// since we are in the middle of moving stuff					
				auto& node = oldNodes[i];
				MoveInsertNode(Move(node));
				node.~Node();
			}

			oldEntry->Free<true>();
		}
	}

	/// Initialize container and reserve data												
	///	@attention inner function, assumes mEntry is empty							
	///	@param maxElements - number of elements to reserve							
	TABLE_TEMPLATE()
	void TABLE()::initData(size_t maxElements) {
		mNumElements = 0;
		mMask = maxElements - 1;
		mMaxNumElementsAllowed = GetMaxElementsAllowed(maxElements);

		// Malloc & zero mInfo - faster than calloc everything				
		auto const numElementsWithBuffer = GetElementsWithBuffer(maxElements);
		auto const numBytesTotal = GetBytesTotal(numElementsWithBuffer);
		mEntry = Allocator::Allocate(numBytesTotal);
		mNodes = mEntry->As<Node>();
		mInfo = reinterpret_cast<uint8_t*>(mNodes + numElementsWithBuffer);
		::std::memset(mInfo, 0, numBytesTotal - numElementsWithBuffer * sizeof(Node));

		// Set sentinel																	
		mInfo[numElementsWithBuffer] = 1;
		mInfoInc = InitialInfoInc;
		mInfoHashShift = InitialInfoHashShift;
	}

	/// @brief																						
	TABLE_TEMPLATE()
	bool TABLE()::try_increase_info() {
		if (mInfoInc <= 2) {
			// Need to be > 2 so that shift works (otherwise					
			// undefined behavior!)														
			return false;
		}

		// We got space left, try to make info smaller							
		mInfoInc = static_cast<uint8_t>(mInfoInc >> 1U);

		// Remove one bit of the hash, leaving more space for the			
		// distance info. This is extremely fast because we can				
		// operate on 8 bytes at once													
		++mInfoHashShift;

		auto const numElementsWithBuffer = GetElementsWithBuffer(mMask + 1);
		for (size_t i = 0; i < numElementsWithBuffer; i += 8) {
			auto val = unaligned_load<uint64_t>(mInfo + i);
			val = (val >> 1U) & UINT64_C(0x7f7f7f7f7f7f7f7f);
			::std::memcpy(mInfo + i, &val, sizeof(val));
		}

		// Update sentinel, which might have been cleared out!				
		mInfo[numElementsWithBuffer] = 1;
		mMaxNumElementsAllowed = GetMaxElementsAllowed(mMask + 1);
		return true;
	}

	/// Increase the contained size															
	///	@return true if resize was possible, false otherwise						
	TABLE_TEMPLATE()
	bool TABLE()::increase_size() {
		// Nothing allocated yet? just allocate InitialNumElements			
		if (0 == mMask) {
			initData(InitialNumElements);
			return true;
		}

		auto const maxNumElementsAllowed = GetMaxElementsAllowed(mMask + 1);
		if (mNumElements < maxNumElementsAllowed && try_increase_info())
			return true;

		if (mNumElements * 2 < GetMaxElementsAllowed(mMask + 1)) {
			// We have to resize, even though there would still be			
			// plenty of space left! Try to rehash instead. Delete			
			// freed memory so we don't steadyily increase mem in				
			// case we have to rehash a few times									
			nextHashMultiplier();
			rehashPowerOfTwo(mMask + 1);
		}
		else {
			// We've reached the capacity of the map, so the hash				
			// seems to work nice. Keep using it									
			rehashPowerOfTwo((mMask + 1) * 2);
		}

		return true;
	}

	TABLE_TEMPLATE()
	void TABLE()::nextHashMultiplier() {
		// Adding an *even* number, so that the multiplier will				
		// always stay odd. This is necessary so that the hash				
		// stays a mixing function (and thus doesn't have any					
		// information loss)																
		mHashMultiplier += UINT64_C(0xc4ceb9fe1a85ec54);
	}

	TABLE_TEMPLATE()
	void TABLE()::init() noexcept {
		mEntry = nullptr;
		mNodes = reinterpret_cast<Node*>(&mMask);
		mInfo = reinterpret_cast<uint8_t*>(&mMask);
		mNumElements = 0;
		mMask = 0;
		mMaxNumElementsAllowed = 0;
		mInfoInc = InitialInfoInc;
		mInfoHashShift = InitialInfoHashShift;
	}

	/// Checks if both tables contain the same entries									
	/// Order is irrelevant																		
	///	@param other - the table to compare against									
	///	@return true if tables match														
	TABLE_TEMPLATE()
	bool TABLE()::operator == (const Table& other) const {
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
	TABLE()& TABLE()::operator = (Table&& rhs) noexcept {
		if (&rhs == this)
			return *this;

		if (rhs.mMask) {
			// Move stuff if the other map actually has some data				
			destroy();
			mHashMultiplier = rhs.mHashMultiplier;
			mEntry = rhs.mEntry;
			mNodes = rhs.mNodes;
			mInfo = rhs.mInfo;
			mNumElements = rhs.mNumElements;
			mMask = rhs.mMask;
			mMaxNumElementsAllowed = rhs.mMaxNumElementsAllowed;
			mInfoInc = rhs.mInfoInc;
			mInfoHashShift = rhs.mInfoHashShift;
			Base::operator = (Forward<Base>(rhs));

			// Reset rhs																	
			rhs.init();
			return *this;
		}

		// Nothing in the other map => just clear this one						
		Clear();
		return *this;
	}

	/// Creates a shallow copy of the given table										
	///	@param rhs - the table to reference												
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (const Table& rhs) {
		// Always reference, before dereferencing, in the rare case that	
		// this table is the same as the other										
		rhs.mEntry->Keep();

		// Dereference this table (and eventually destroy elements)			
		if (mEntry) {
			if (mEntry->GetUses() == 1) {
				// Destroy all elements but don't deallocate the entry		
				DestroyNodes();
			}
			else {
				// If reached, then data is referenced from multiple places	
				// Don't call destructors, just clear it up and dereference	
				mEntry->Free<false>();
			}
		}

		mHashMultiplier = rhs.mHashMultiplier;
		mNumElements = rhs.mNumElements;
		mMask = rhs.mMask;
		mMaxNumElementsAllowed = rhs.mMaxNumElementsAllowed;
		mInfoInc = rhs.mInfoInc;
		mInfoHashShift = rhs.mInfoHashShift;
		return *this;
	}

	/// Emplace a single pair into a cleared map											
	///	@param pair - the pair to emplace												
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (Type&& pair) noexcept {
		Clear();
		Emplace(Forward<Type>(pair));
		return *this;
	}

	/// Insert a single pair into a cleared map											
	///	@param pair - the pair to copy													
	///	@return a reference to this table												
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator = (const Type& pair) {
		Clear();
		Insert(pair);
		return *this;
	}
	
	/// Clone all nodes, as well as info bytes											
	///	@param o - the table to clone														
	TABLE_TEMPLATE()
	void TABLE()::CloneInner(const Table& other) {
		if constexpr (CT::POD<Node>) {
			// Copy all pointers to pairs, together with info bytes			
			auto const numElementsWithBuffer = GetElementsWithBuffer(mMask + 1);
			::std::copy(other.mNodeBytes, other.mNodeBytes + GetBytesTotal(numElementsWithBuffer), mNodeBytes);
		}
		else {
			static_assert(CT::CloneMakable<Node>, "Contained type is not clone-makable");

			// Copy the info bytes first												
			auto const numElementsWithBuffer = GetElementsWithBuffer(mMask + 1);
			::std::copy(other.mInfo, other.mInfo + GetBytesInfo(numElementsWithBuffer), mInfo);

			// We're on the stack, copy each pair individually unless POD	
			//TODO POD optimization
			for (size_t i = 0; i < numElementsWithBuffer; ++i) {
				if (!mInfo[i])
					continue;

				::new (mNodes + i) Node {other.mNodes[i].Clone()};
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
		const auto numElementsWithBuffer = GetElementsWithBuffer(mMask + 1);
		const auto numBytesTotal = GetBytesTotal(numElementsWithBuffer);
		result.mEntry = Allocator::Allocate(numBytesTotal);
		result.mNodes = result.mEntry->As<Node>();
		result.mInfo = reinterpret_cast<uint8_t*>(result.mNodes + numElementsWithBuffer);
		result.CloneInner(*this);
		return result;
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
	
	/// Templated tables are always typed													
	///	@return false																			
	/*TABLE_TEMPLATE()
	constexpr bool TABLE()::IsUntyped() const noexcept requires IsSet {
		return false;
	}*/
	
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
	
	/// Templated tables are always type-constrained									
	///	@return true																			
	/*TABLE_TEMPLATE()
	constexpr bool TABLE()::IsTypeConstrained() const noexcept requires IsSet {
		return true;
	}*/
	
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
	
	/// Check if value type is abstract														
	/*TABLE_TEMPLATE()
	constexpr bool TABLE()::IsAbstract() const noexcept requires IsSet {
		return CT::Abstract<Type> && !IsSparse();
	}*/
	
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
	
	/// Check if value type is default-constructible									
	/*TABLE_TEMPLATE()
	constexpr bool TABLE()::IsConstructible() const noexcept requires IsSet {
		return CT::Defaultable<Type>;
	}*/
	
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

	/// Check if value type is deep															
	/*TABLE_TEMPLATE()
	constexpr bool TABLE()::IsDeep() const noexcept requires IsSet {
		return CT::Deep<Type>;
	}*/

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

	/// Check if the value type is a pointer												
	/*TABLE_TEMPLATE()
	constexpr bool TABLE()::IsSparse() const noexcept requires IsSet {
		return CT::Sparse<Type>;
	}*/

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

	/// Check if the value type is not a pointer											
	/*TABLE_TEMPLATE()
	constexpr bool TABLE()::IsDense() const noexcept requires IsSet {
		return CT::Dense<Type>;
	}*/

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

	/// Get the size of a single value, in bytes											
	///	@return the number of bytes a single value contains						
	/*TABLE_TEMPLATE()
	constexpr Size TABLE()::GetStride() const noexcept requires IsSet {
		return sizeof(Type);
	}*/

	/// Get the raw node array inside the map												
	TABLE_TEMPLATE()
	constexpr typename TABLE()::Node* TABLE()::GetRaw() const noexcept {
		return mNodes;
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

	/// Check if value type exactly matches another										
	/*TABLE_TEMPLATE()
	template<class ALT_T>
	constexpr bool TABLE()::Is() const noexcept requires IsSet {
		return CT::Same<Type, ALT_T>;
	}*/

	/// Move-insert a pair inside the map													
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator << (Type&& item) {
		Emplace(Forward<Type>(item));
		return *this;
	}

	/// Copy-insert a pair inside the map													
	TABLE_TEMPLATE()
	TABLE()& TABLE()::operator << (const Type& item) {
		Insert(item);
		return *this;
	}

	/// Reserves space for the specified number of elements. Makes sure the		
	/// data fits. Exactly the same as rehash(c). Use rehash(0)	to shrink-fit	
	///	@tparam REHASH - force rehash if true											
	///	@param c - number of elements to allocate										
	TABLE_TEMPLATE()
	template<bool REHASH>
	void TABLE()::Allocate(Count c) {
		auto const minElementsAllowed = (::std::max) (c, mNumElements);
		auto newSize = InitialNumElements;
		while (GetMaxElementsAllowed(newSize) < minElementsAllowed && newSize != 0)
			newSize *= 2;

		if (LANGULUS_UNLIKELY(newSize == 0))
			Throw<Except::Overflow>("Table overflow");
				
		if constexpr (REHASH) {
			// Force a rehash																
			rehashPowerOfTwo(newSize);
		}
		else {
			// Only actually do anything when the new size is bigger than	
			// the old one. This prevents to continuously allocate for		
			// each reserve() call														
			if (newSize > mMask + 1)
				rehashPowerOfTwo(newSize);
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
	void TABLE()::Insert(::std::initializer_list<Type> ilist) {
		for (auto&& vt : ilist)
			Insert(Move(vt));
	}

	/// Emplace a single pair inside table													
	///	@param ...args - items to add														
	///	@return a pair containing the first new item & status of insertion	
	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::Insertion TABLE()::Emplace(Args&&... args) {
		// This will be the new node													
		Type n {Forward<Args>(args)...};

		// Search for an empty spot and shift things around if we have to	
		const auto spot = InsertKeyAndPrepareEmptySpot(Move(n.mKey));

		// Check the insertion state													
		switch (spot.mState) {
		case InsertionState::key_found:
			// Pair already exists, so we don't need the new node				
			//n.destroy(*this);
			break;
		case InsertionState::new_node:
			// We can emplace the new node											
			if constexpr (CT::Same<Node, Type>)
				::new (&GetNode(spot.mOffset)) Node {Move(n)};
			else
				::new (&GetNode(spot.mOffset)) Node {Node::Create(Move(n))};
			break;
		case InsertionState::overwrite_node:
			// The node is unused and we can overwrite it						
			if constexpr (CT::Same<Node, Type>)
				GetNode(spot.mOffset) = Move(n);
			else {
				auto& node = GetNode(spot.mOffset);
				if (node)
					*node = Move(n);
				else
					node = Node::Create(Move(n));
			}
			break;
		case InsertionState::overflow_error:
			// Max capacity reached, destroy the node and throw				
			//n.destroy(*this);
			Throw<Except::Overflow>("Table overflow");
			break;
		}

		// If reached, then we successfully emplaced the new pair			
		return {
			iterator(mNodes + spot.mOffset, mInfo + spot.mOffset),
			InsertionState::key_found != spot.mState
		};
	}

	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::iterator TABLE()::emplace_hint(const_iterator, Args&&... args) {
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
	typename TABLE()::iterator TABLE()::try_emplace(const_iterator, const K& key, Args&&... args) {
		return try_emplace_impl(key, std::forward<Args>(args)...).first;
	}

	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::iterator TABLE()::try_emplace(const_iterator, K&& key, Args&&... args) {
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
	typename TABLE()::iterator TABLE()::insert_or_assign(const_iterator, const K& key, Mapped&& obj) {
		return insertOrAssignImpl(key, Forward<Mapped>(obj)).first;
	}

	TABLE_TEMPLATE()
	template<class Mapped>
	typename TABLE()::iterator TABLE()::insert_or_assign(const_iterator, K&& key, Mapped&& obj) {
		return insertOrAssignImpl(Move(key), Forward<Mapped>(obj)).first;
	}

	TABLE_TEMPLATE()
	typename TABLE()::Insertion TABLE()::Insert(const Type& item) {
		return Emplace(item);
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::Insert(const_iterator, const Type& item) {
		return Emplace(item).first;
	}

	TABLE_TEMPLATE()
	typename TABLE()::Insertion TABLE()::Insert(Type&& item) {
		return Emplace(Forward<Type>(item));
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::Insert(const_iterator, Type&& item) {
		return Emplace(Forward<Type>(item)).first;
	}

	/// Inserts an item that is guaranteed to be new, e.g. when the				
	/// table is resized - no need to search for key to reuse items				
	TABLE_TEMPLATE()
	void TABLE()::MoveInsertNode(Node&& item) {
		// We don't retry, fail if overflowing, don't need to check max	
		// num elements																	
		if (0 == mMaxNumElementsAllowed && !try_increase_info())
			Throw<Except::Overflow>("Table overflow");

		size_t idx {};
		InfoType info {};
		if constexpr (IsMap)
			keyToIdx(item->mKey, &idx, &info);
		else
			keyToIdx(item, &idx, &info);

		// Skip forward																	
		// Use <= because we are certain that the element is not there		
		while (info <= mInfo[idx]) {
			++idx;
			info += mInfoInc;
		}

		// We are now exactly where we want to insert it						
		auto const insertion_idx = idx;
		auto const insertion_info = static_cast<uint8_t>(info);
		if (LANGULUS_UNLIKELY(insertion_info + mInfoInc > 0xFF))
			mMaxNumElementsAllowed = 0;

		// Find an empty spot															
		while (0 != mInfo[idx])
			next(&info, &idx);

		auto& l = GetNode(insertion_idx);
		if (idx == insertion_idx) {
			::new (&l) Node(Forward<Node>(item));
		}
		else {
			shiftUp(idx, insertion_idx);
			l = Forward<Node>(item);
		}

		// Put at empty spot																
		mInfo[insertion_idx] = insertion_info;
		++mNumElements;
	}

	/// Try to emplace an item using piecewise construction							
	TABLE_TEMPLATE()
	template<class... Args>
	typename TABLE()::Insertion TABLE()::try_emplace_impl(K&& key, Args&&... args) {
		const auto spot = InsertKeyAndPrepareEmptySpot(key);

		switch (spot.mState) {
		case InsertionState::key_found:
			break;

		case InsertionState::new_node:
			::new (mNodes + spot.mOffset) Node(
				*this, 
				::std::piecewise_construct, 
				::std::forward_as_tuple(Forward<K>(key)),
				::std::forward_as_tuple(Forward<Args>(args)...));
			break;

		case InsertionState::overwrite_node:
			mNodes[spot.mOffset] = Node(
				*this, 
				::std::piecewise_construct,
				::std::forward_as_tuple(Forward<K>(key)),
				::std::forward_as_tuple(Forward<Args>(args)...));
			break;

		case InsertionState::overflow_error:
			Throw<Except::Overflow>("Table overflow");
			break;
		}

		return {
			iterator(mNodes + spot.mOffset, mInfo + spot.mOffset),
			InsertionState::key_found != spot.mState
		};
	}

	/// Try to insert/assign an item using piecewise construction					
	TABLE_TEMPLATE()
	template<class Mapped>
	typename TABLE()::Insertion TABLE()::insertOrAssignImpl(K&& key, Mapped&& obj) {
		const auto spot = InsertKeyAndPrepareEmptySpot(key);

		switch (spot.mState) {
		case InsertionState::key_found:
			mNodes[spot.mOffset].getSecond() = Forward<Mapped>(obj);
			break;

		case InsertionState::new_node:
			::new (mNodes + spot.mOffset) Node(
				*this, 
				::std::piecewise_construct,
				::std::forward_as_tuple(Forward<K>(key)),
				::std::forward_as_tuple(Forward<Mapped>(obj)));
			break;

		case InsertionState::overwrite_node:
			mNodes[spot.mOffset] = Node(
				*this, 
				::std::piecewise_construct,
				::std::forward_as_tuple(Forward<K>(key)),
				::std::forward_as_tuple(Forward<Mapped>(obj)));
			break;

		case InsertionState::overflow_error:
			Throw<Except::Overflow>("Table overflow");
			break;
		}

		return std::make_pair(
			iterator(mNodes + spot.mOffset, mInfo + spot.mOffset),
			InsertionState::key_found != spot.mState
		);
	}

	/// Finds key, and if not already present prepares a spot where to pot the	
	/// key & value. This potentially shifts nodes out of the way, updates		
	/// mInfo and number of inserted elements, so the only operation left to	
	/// do is create/assign a new node at that spot										
	///	@param key - the key to push														
	///	@return a pair containing the prepared pair offset and state			
	TABLE_TEMPLATE()
	typename TABLE()::EmptySpot TABLE()::InsertKeyAndPrepareEmptySpot(K&& key) {
		for (int i = 0; i < 256; ++i) {
			size_t idx {};
			InfoType info {};
			keyToIdx(key, &idx, &info);
			nextWhileLess(&info, &idx);

			// While we potentially have a match, check for a match			
			while (info == mInfo[idx]) {
				if (key == GetKey(idx)) {
					// Key already exists, do NOT insert							
					return {idx, InsertionState::key_found};
				}

				// Continue searching for a match									
				next(&info, &idx);
			}

			// No match was found if this is reached								
			// Very unlikely that the container has reached max elements	
			if (LANGULUS_UNLIKELY(mNumElements >= mMaxNumElementsAllowed)) {
				if (!increase_size())
					return {0, InsertionState::overflow_error};
				continue;
			}

			// We are now exactly where we want to insert it					
			auto const insertion_idx = idx;
			auto const insertion_info = info;
			if (LANGULUS_UNLIKELY(insertion_info + mInfoInc > 0xFF))
				mMaxNumElementsAllowed = 0;

			// Find an empty spot														
			while (0 != mInfo[idx]) {
				// There's something at that spot, so move on					
				next(&info, &idx);
			}

			if (idx != insertion_idx)
				shiftUp(idx, insertion_idx);

			// Put at empty spot															
			mInfo[insertion_idx] = static_cast<uint8_t>(insertion_info);
			++mNumElements;
			return {
				insertion_idx,
				idx == insertion_idx
				? InsertionState::new_node
				: InsertionState::overwrite_node
			};
		}

		// Enough attempts failed, so finally give up							
		return {0, InsertionState::overflow_error};
	}

	/// Clears all data, without resizing													
	TABLE_TEMPLATE()
	void TABLE()::Clear() {
		if (IsEmpty())
			return;

		DestroyNodes();

		auto const numElementsWithBuffer = GetElementsWithBuffer(mMask + 1);

		// Clear everything, then set the sentinel again						
		uint8_t const z = 0;
		::std::fill(mInfo, mInfo + GetBytesInfo(numElementsWithBuffer), z);
		mInfo[numElementsWithBuffer] = 1;
		mInfoInc = InitialInfoInc;
		mInfoHashShift = InitialInfoHashShift;
	}

	/// Clears all data and deallocates														
	TABLE_TEMPLATE()
	void TABLE()::Reset() {
		destroy();
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
		// We assume that pos always points to a valid entry, not end()	
		const auto idx = static_cast<size_t>(pos.mNode - mNodes);
		shiftDown(idx);
		--mNumElements;

		if (*pos.mInfo) {
			// We've backward shifted, return this again							
			return pos;
		}

		// No backward shift, return next element									
		return ++pos;
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

	/// If possible reallocates the map to a smaller one. This frees the			
	/// underlying table. Does not do anything if load_factor is too				
	/// large for decreasing the table's size.											
	TABLE_TEMPLATE()
	void TABLE()::compact() {
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

	/// Destroy contents																			
	TABLE_TEMPLATE()
	void TABLE()::destroy() {
		if (0 == mMask)
			return;

		DestroyNodes();
		mEntry->Free<true>();
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
	bool TABLE()::ContainsPair(const Type& e) const {
		const auto found = Find(e.mKey);
		return found != end() && found->mValue == e.mValue;
	}

	/// Search for an entry inside the table												
	///	@return true if pair is found, false otherwise								
	/*TABLE_TEMPLATE()
	bool TABLE()::Contains(const Type& e) const requires IsSet {
		const auto found = Find(e);
		return found != end() && *found == e;
	}*/

	TABLE_TEMPLATE()
	const K& TABLE()::GetKey(const Offset& i) const noexcept {
		return const_cast<TABLE()&>(*this).GetKey(i);
	}

	TABLE_TEMPLATE()
	K& TABLE()::GetKey(const Offset& i) noexcept {
		return GetPair(i).mKey;
	}

	TABLE_TEMPLATE()
	const V& TABLE()::GetValue(const Offset& i) const noexcept {
		return const_cast<TABLE()&>(*this).GetValue(i);
	}

	TABLE_TEMPLATE()
	V& TABLE()::GetValue(const Offset& i) noexcept {
		return GetPair(i).mValue;
	}

	TABLE_TEMPLATE()
	const typename TABLE()::Type& TABLE()::GetPair(const Offset& i) const noexcept {
		return const_cast<TABLE()&>(*this).GetPair(i);
	}

	TABLE_TEMPLATE()
	typename TABLE()::Type& TABLE()::GetPair(const Offset& i) noexcept {
		if constexpr (IsOnHeap)
			return *mNodes[i];
		else
			return mNodes[i];
	}

	TABLE_TEMPLATE()
	const typename TABLE()::Node& TABLE()::GetNode(const Offset& i) const noexcept {
		return const_cast<TABLE()&>(*this).GetNode(i);
	}

	TABLE_TEMPLATE()
	typename TABLE()::Node& TABLE()::GetNode(const Offset& i) noexcept {
		return mNodes[i];
	}

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	V& TABLE()::At(const K& key) {
		auto found = mNodes + FindIndex(key);
		if (found == mNodesEnd)
			Throw<Except::OutOfRange>("Key not found");

		return (*found)->mValue;
	}

	/// Returns a reference to the value found for key									
	/// Throws std::out_of_range if element cannot be found							
	TABLE_TEMPLATE()
	const V& TABLE()::At(const K& key) const {
		return const_cast<TABLE()>(*this).At(key);
	}

	/// Find																							
	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::Find(const K& key) const {
		const auto idx = FindIndex(key);
		return {mNodes + idx, mInfo + idx};
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::Find(const K& key) {
		const auto idx = FindIndex(key);
		return {mNodes + idx, mInfo + idx};
	}

	/// Copy of find(), except that it returns iterator instead of					
	/// const_iterator																			
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
	const V& TABLE()::operator[] (const K& key) const {
		return At(key);
	}

	/// Access value by key																		
	///	@param key - the key to find														
	///	@return a reference to the value													
	TABLE_TEMPLATE()
	V& TABLE()::operator[] (const K& key) {
		return At(key);
	}
	
	/// Get the beginning of internal data													
	///	@return the iterator																	
	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::begin() {
		if (IsEmpty())
			return end();
		return {mNodes, mInfo, fast_forward_tag {}};
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
		return {mNodes, mInfo, fast_forward_tag {}};
	}

	TABLE_TEMPLATE()
	typename TABLE()::iterator TABLE()::end() {
		// No need to supply valid info pointer: end() must not be			
		// dereferenced, and only node pointer is compared						
		return {mNodesEnd, nullptr};
	}

	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::end() const {
		return cend();
	}

	TABLE_TEMPLATE()
	typename TABLE()::const_iterator TABLE()::cend() const {
		return {mNodesEnd, nullptr};
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
		return mMask > 0;
	}

	/// Check if the memory for the table is owned by us								
	///	@return true if memory is ours													
	TABLE_TEMPLATE()
	bool TABLE()::HasAuthority() const noexcept {
		return mEntry != nullptr;
	}

	/// Get the number of references for the allocated memory						
	///	@attention always returns zero if we don't have authority				
	///	@return the number of references													
	TABLE_TEMPLATE()
	Count TABLE()::GetUses() const noexcept {
		return mEntry ? mEntry->GetUses() : 0;
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

	/// Get the maximum allowed number of pairs, based on load factor				
	///	@param requested - the requested element count								
	///	@return the maximum allowed count												
	TABLE_TEMPLATE()
	Count TABLE()::GetMaxElementsAllowed(Count requested) noexcept {
		if (LANGULUS_LIKELY(requested <= CountMax / 100))
			return requested * MaxLoadFactor100 / 100;

		// We might be a bit inprecise, but since maxElements is quite		
		// large that doesn't matter													
		return (requested / 100) * MaxLoadFactor100;
	}

	TABLE_TEMPLATE()
	Count TABLE()::GetBytesInfo(Count numElements) noexcept {
		// We add a uint64_t, which houses the sentinel (first byte) and	
		// padding so we can load 64bit types										
		return numElements + sizeof(uint64_t);
	}

	TABLE_TEMPLATE()
	Count TABLE()::GetElementsWithBuffer(Count numElements) noexcept {
		auto maxNumElementsAllowed = GetMaxElementsAllowed(numElements);
		return numElements + (::std::min) (maxNumElementsAllowed, Count {0xFF});
	}

	/// Calculation only allowed for 2^n values											
	///	@param numElements - number of nodes to allocate							
	///	@return the byte size for the required nodes, as well as info bytes	
	TABLE_TEMPLATE()
	Count TABLE()::GetBytesTotal(Count numElements) {
		#if LANGULUS(BITNESS) == 64
			return numElements * sizeof(Node) + GetBytesInfo(numElements);
		#else
			// Make sure we're doing 64bit operations, so we are at least	
			// safe against 32bit overflows											
			auto const ne = static_cast<uint64_t>(numElements);
			auto const s = static_cast<uint64_t>(sizeof(Node));
			auto const infos = static_cast<uint64_t>(GetBytesInfo(numElements));

			auto const total64 = ne * s + infos;
			auto const total = static_cast<size_t>(total64);

			if (LANGULUS_UNLIKELY(static_cast<uint64_t>(total) != total64))
				Throw<Except::Overflow>("Table overflow");

			return total;
		#endif
	}

	/// Destroyer																					
	TABLE_TEMPLATE()
	void TABLE()::DestroyNodes() noexcept {
		mNumElements = 0;

		if constexpr (DENSE || !::std::is_trivially_destructible_v<Node>) {
			// Clear also resets mInfo to 0, that's sometimes not				
			// necessary																	
			auto const numElementsWithBuffer = GetElementsWithBuffer(mMask + 1);
			for (size_t idx = 0; idx < numElementsWithBuffer; ++idx) {
				if (0 == mInfo[idx])
					continue;

				mNodes[idx].~Node();
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
	void TABLE()::shiftUp(size_t startIdx, size_t const insertion_idx) noexcept(CT::MovableNoexcept<Node>) {
		auto idx = startIdx;

		::new (mNodes + idx) Node(Move(mNodes[idx - 1]));

		while (--idx != insertion_idx)
			mNodes[idx] = Move(mNodes[idx - 1]);

		idx = startIdx;
		while (idx != insertion_idx) {
			mInfo[idx] = static_cast<uint8_t>(mInfo[idx - 1] + mInfoInc);
			if (LANGULUS_UNLIKELY(mInfo[idx] + mInfoInc > 0xFF))
				mMaxNumElementsAllowed = 0;
			--idx;
		}
	}

	TABLE_TEMPLATE()
	void TABLE()::shiftDown(Offset idx) noexcept(CT::MovableNoexcept<Node>) {
		// Until we find one that is either empty or has zero offset		
		// TODO(martinus) we don't need to move everything, just				
		// the last one for the same bucket											
		if constexpr (CT::Sparse<Node>) {
			auto data = mNodes[idx];
			const auto found = Allocator::Find(MetaData::Of<Type>(), data);
			if (found) {
				if (found->GetUses() == 1) {
					if constexpr (CT::Destroyable<Type>)
						(*data).~Type();
					Allocator::Deallocate(found);
				}
				else found->template Free<false>();
			}
		}

		// Until we find one that is either empty or has zero offset		
		while (mInfo[idx + 1] >= 2 * mInfoInc) {
			mInfo[idx] = static_cast<uint8_t>(mInfo[idx + 1] - mInfoInc);
			mNodes[idx] = Move(mNodes[idx + 1]);
			++idx;
		}

		mInfo[idx] = 0;
		//mNodes[idx].~Node();
	}

} // namespace Langulus::Anyness::Inner
