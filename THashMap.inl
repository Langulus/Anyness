#pragma once
#include "THashMap.hpp"

namespace Langulus::Anyness::Inner
{

	/// 																								
	///	DataNodeOnStack																		
	/// 																								
	NODE_TEMPLATE() 
	template <typename... Args>
	NODE(Stack)::DataNodeOnStack(M&, Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
		: mData {std::forward<Args>(args)...} {}

	NODE_TEMPLATE()
	NODE(Stack)::DataNodeOnStack(M&, DataNodeOnStack&& n) noexcept(std::is_nothrow_move_constructible<T>::value)
		: mData {std::move(n.mData)} {}

	NODE_TEMPLATE()
	T const* NODE(Stack)::operator->() const noexcept {
		return &mData;
	}

	NODE_TEMPLATE()
	T* NODE(Stack)::operator->() noexcept {
		return &mData;
	}

	NODE_TEMPLATE()
	const T& NODE(Stack)::operator*() const noexcept {
		return mData;
	}

	NODE_TEMPLATE()
	T& NODE(Stack)::operator*() noexcept {
		return mData;
	}

	NODE_TEMPLATE()
	template<typename VT>
	typename VT::first_type& NODE(Stack)::getFirst() noexcept requires IsMap<M> {
		return mData.first;
	}

	NODE_TEMPLATE()
	template<typename VT>
	VT& NODE(Stack)::getFirst() noexcept requires IsSet<M> {
		return mData;
	}

	NODE_TEMPLATE()
	template<typename VT>
	typename VT::first_type const& NODE(Stack)::getFirst() const noexcept requires IsMap<M> {
		return mData.first;
	}

	NODE_TEMPLATE()
	template<typename VT>
	VT const& NODE(Stack)::getFirst() const noexcept requires IsSet<M> {
		return mData;
	}

	NODE_TEMPLATE()
	template<typename MT>
	MT& NODE(Stack)::getSecond() noexcept requires IsMap<M> {
		return mData.second;
	}

	NODE_TEMPLATE()
	template<typename MT>
	MT const& NODE(Stack)::getSecond() const noexcept requires IsSet<M> {
		return mData.second;
	}

	NODE_TEMPLATE()
	void NODE(Stack)::swap(DataNodeOnStack& o) noexcept(noexcept(std::declval<T>().swap(std::declval<T>()))) {
		mData.swap(o.mData);
	}

	/// 																								
	///	DataNodeOnHeap																			
	/// 																								
	NODE_TEMPLATE()
	template <typename... Args>
	NODE(Heap)::DataNodeOnHeap(M& map, Args&&... args)
		: mData(map.allocate()) {
		::new (static_cast<void*>(mData)) T(std::forward<Args>(args)...);
	}

	NODE_TEMPLATE()
	NODE(Heap)::DataNodeOnHeap(M&, DataNodeOnHeap&& n) noexcept
		: mData(std::move(n.mData)) {}

	NODE_TEMPLATE()
	void NODE(Heap)::destroy(M& map) noexcept {
		// don't deallocate, just put it into list of datapool.
		mData->~T();
		map.deallocate(mData);
	}

	NODE_TEMPLATE()
	void NODE(Heap)::destroyDoNotDeallocate() noexcept {
		mData->~T();
	}

	NODE_TEMPLATE()
	T const* NODE(Heap)::operator->() const noexcept {
		return mData;
	}

	NODE_TEMPLATE()
	T* NODE(Heap)::operator->() noexcept {
		return mData;
	}

	NODE_TEMPLATE()
	const T& NODE(Heap)::operator*() const {
		return *mData;
	}

	NODE_TEMPLATE()
	T& NODE(Heap)::operator*() {
		return *mData;
	}

	NODE_TEMPLATE()
	template<typename VT>
	typename VT::first_type& NODE(Heap)::getFirst() noexcept requires IsMap<M> {
		return mData->first;
	}

	NODE_TEMPLATE()
	template<typename VT>
	VT& NODE(Heap)::getFirst() noexcept requires IsSet<M> {
		return *mData;
	}

	NODE_TEMPLATE()
	template<typename VT>
	typename VT::first_type const& NODE(Heap)::getFirst() const noexcept requires IsMap<M> {
		return mData->first;
	}

	NODE_TEMPLATE()
	template<typename VT>
	VT const& NODE(Heap)::getFirst() const noexcept requires IsSet<M> {
		return *mData;
	}

	NODE_TEMPLATE()
	template<typename MT>
	MT& NODE(Heap)::getSecond() noexcept requires IsMap<M> {
		return mData->second;
	}

	NODE_TEMPLATE()
	template<typename MT>
	MT const& NODE(Heap)::getSecond() const noexcept requires IsMap<M> {
		return mData->second;
	}

	NODE_TEMPLATE()
	void NODE(Heap)::swap(DataNodeOnHeap& o) noexcept {
		std::swap(mData, o.mData);
	}


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

	TABLE_TEMPLATE()
	template<class IT>
	TABLE()::Table(IT first, IT last, size_t) {
		insert(first, last);
	}

	TABLE_TEMPLATE()
	TABLE()::Table(std::initializer_list<value_type> initlist, size_t) {
		insert(initlist.begin(), initlist.end());
	}

	TABLE_TEMPLATE()
	TABLE()::Table(Table&& o) noexcept
		: DataPool(Forward<DataPool&>(o)) {
		if (o.mMask) {
			mHashMultiplier = Move(o.mHashMultiplier);
			mKeyVals = Move(o.mKeyVals);
			mInfo = Move(o.mInfo);
			mNumElements = Move(o.mNumElements);
			mMask = Move(o.mMask);
			mMaxNumElementsAllowed = Move(o.mMaxNumElementsAllowed);
			mInfoInc = Move(o.mInfoInc);
			mInfoHashShift = Move(o.mInfoHashShift);
			// set other's mask to 0 so its destructor won't do anything
			o.init();
		}
	}

	TABLE_TEMPLATE()
	TABLE()::Table(const Table& o)
		: DataPool(static_cast<const DataPool&>(o)) {
		if (!o.empty()) {
			// not empty: create an exact copy. it is also possible to just iterate through all
			// elements and insert them, but copying is probably faster.

			auto const numElementsWithBuffer = calcNumElementsWithBuffer(o.mMask + 1);
			auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);

			mHashMultiplier = o.mHashMultiplier;
			mKeyVals = static_cast<Node*>(Inner::assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));
			// No need for calloc because cloneData does memcpy				
			mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
			mNumElements = o.mNumElements;
			mMask = o.mMask;
			mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
			mInfoInc = o.mInfoInc;
			mInfoHashShift = o.mInfoHashShift;
			cloneData(o);
		}
	}

	/// Destroys the map and all it's contents											
	TABLE_TEMPLATE()
	TABLE()::~Table() {
		destroy();
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
		}
		else {
			// Nothing in the other map => just clear it							
			Clear();
		}

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
			mKeyVals = static_cast<Node*>(Inner::assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));

			// No need for calloc here because cloneData performs a memcpy	
			mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
			// Sentinel is set in cloneData											
		}

		DataPool::operator=(static_cast<DataPool const&>(o));
		mHashMultiplier = o.mHashMultiplier;
		mNumElements = o.mNumElements;
		mMask = o.mMask;
		mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
		mInfoInc = o.mInfoInc;
		mInfoHashShift = o.mInfoHashShift;
		cloneData(o);
		return *this;
	}

	TABLE_TEMPLATE()
	DMeta TABLE()::GetKeyType() const {
		return MetaData::Of<Key>();
	}

	TABLE_TEMPLATE()
	DMeta TABLE()::GetValueType() const {
		return MetaData::Of<T>();
	}

} // namespace Langulus::Anyness::Inner
