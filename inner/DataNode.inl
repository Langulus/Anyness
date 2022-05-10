#pragma once
#include "DataNode.hpp"

#define NODE_TEMPLATE()	template<class M, class T, class V>
#define NODE(a) DataNodeOn##a<M, T, V>

namespace Langulus::Anyness::Inner
{

	/// 																								
	///	DataNodeOnStack																		
	/// 																								
	NODE_TEMPLATE() 
	template <class... Args>
	NODE(Stack)::DataNodeOnStack(M&, Args&&... args) noexcept(noexcept(T(Forward<Args>(args)...)))
		: mData {Forward<Args>(args)...} {}

	NODE_TEMPLATE()
	NODE(Stack)::DataNodeOnStack(M&, DataNodeOnStack&& n) noexcept(IsNoexceptMoveConstructible<T>)
		: mData {Move(n.mData)} {}

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
	template<class VT>
	typename VT::Key& NODE(Stack)::getFirst() noexcept requires IsMap<M> {
		return mData.mKey;
	}

	NODE_TEMPLATE()
	template<class VT>
	VT& NODE(Stack)::getFirst() noexcept requires IsSet<M> {
		return mData;
	}

	NODE_TEMPLATE()
	template<class VT>
	typename VT::Key const& NODE(Stack)::getFirst() const noexcept requires IsMap<M> {
		return mData.mKey;
	}

	NODE_TEMPLATE()
	template<class VT>
	VT const& NODE(Stack)::getFirst() const noexcept requires IsSet<M> {
		return mData;
	}

	NODE_TEMPLATE()
	template<class MT>
	MT& NODE(Stack)::getSecond() noexcept requires IsMap<M> {
		return mData.mValue;
	}

	NODE_TEMPLATE()
	template<class MT>
	MT const& NODE(Stack)::getSecond() const noexcept requires IsSet<M> {
		return mData.mValue;
	}

	NODE_TEMPLATE()
	void NODE(Stack)::swap(DataNodeOnStack& o) noexcept(noexcept(Uneval<T>().swap(Uneval<T>()))) {
		mData.swap(o.mData);
	}

	/// 																								
	///	DataNodeOnHeap																			
	/// 																								
	NODE_TEMPLATE()
	template<class... Args>
	NODE(Heap)::DataNodeOnHeap(M& map, Args&&... args)
		: mData(map.allocate()) {
		::new (static_cast<void*>(mData)) T(Forward<Args>(args)...);
	}

	NODE_TEMPLATE()
	NODE(Heap)::DataNodeOnHeap(M&, DataNodeOnHeap&& n) noexcept
		: mData(Move(n.mData)) {}

	NODE_TEMPLATE()
	void NODE(Heap)::destroy(M& map) noexcept {
		// Don't deallocate, just put it into list of datapool				
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
	template<class VT>
	typename VT::Key& NODE(Heap)::getFirst() noexcept requires IsMap<M> {
		return mData->mKey;
	}

	NODE_TEMPLATE()
	template<class VT>
	VT& NODE(Heap)::getFirst() noexcept requires IsSet<M> {
		return *mData;
	}

	NODE_TEMPLATE()
	template<class VT>
	typename VT::Key const& NODE(Heap)::getFirst() const noexcept requires IsMap<M> {
		return mData->mKey;
	}

	NODE_TEMPLATE()
	template<class VT>
	VT const& NODE(Heap)::getFirst() const noexcept requires IsSet<M> {
		return *mData;
	}

	NODE_TEMPLATE()
	template<class MT>
	MT& NODE(Heap)::getSecond() noexcept requires IsMap<M> {
		return mData->mValue;
	}

	NODE_TEMPLATE()
	template<class MT>
	MT const& NODE(Heap)::getSecond() const noexcept requires IsMap<M> {
		return mData->mValue;
	}

	NODE_TEMPLATE()
	void NODE(Heap)::swap(DataNodeOnHeap& o) noexcept {
		std::swap(mData, o.mData);
	}

} // namespace Langulus::Anyness::Inner

#undef NODE_TEMPLATE
#undef NODE
