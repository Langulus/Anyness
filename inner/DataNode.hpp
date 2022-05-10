#pragma once
#include "Integration.hpp"

namespace Langulus::Anyness::Inner
{
	
	///																								
	///	Data nodes																				
	///																								
	///	Primary templates for the data node. We have special implementations	
	/// for small and big objects. For large objects it is assumed that swap()	
	/// is fairly slow, so we allocate these on the heap so swap merely swaps	
	/// a pointer.																					

	///																								
	///	Small: just allocate on the stack												
	///																								
	template<class M, class T, class V>
	class DataNodeOnStack final {
	private:
		T mData;

	public:
		template <class... Args>
		explicit DataNodeOnStack(M&, Args&&...args) noexcept(noexcept(T(Forward<Args>(args)...)));
		DataNodeOnStack(M&, DataNodeOnStack&&) noexcept(IsNoexceptMoveConstructible<T>);

		void destroy(M&) noexcept {}
		void destroyDoNotDeallocate() noexcept {}

		T const* operator->() const noexcept;
		T* operator->() noexcept;

		const T& operator*() const noexcept;
		T& operator*() noexcept;

		void swap(DataNodeOnStack&) noexcept(noexcept(Uneval<T>().swap(Uneval<T>())));

		// Pair interface																	
		template<class VT = T>
		NOD() typename VT::Key& getFirst() noexcept requires IsMap<M>;
		template<class VT = T>
		NOD() typename VT::Key const& getFirst() const noexcept requires IsMap<M>;
		template<class MT = V>
		NOD() MT& getSecond() noexcept requires IsMap<M>;

		// Value interface																
		template<class VT = T>
		NOD() VT& getFirst() noexcept requires IsSet<M>;
		template<class VT = T>
		NOD() VT const& getFirst() const noexcept requires IsSet<M>;
		template<class MT = V>
		NOD() MT const& getSecond() const noexcept requires IsSet<M>;
	};


	///																								
	///	Big object: allocate on heap														
	///																								
	template<class M, class T, class V>
	class DataNodeOnHeap {
	private:
		T* mData;

	public:
		template <class... Args>
		explicit DataNodeOnHeap(M&, Args&&...);
		DataNodeOnHeap(M&, DataNodeOnHeap&&) noexcept;

		void destroy(M&) noexcept;
		void destroyDoNotDeallocate() noexcept;

		T const* operator->() const noexcept;
		T* operator->() noexcept;

		const T& operator*() const;
		T& operator*();

		void swap(DataNodeOnHeap&) noexcept;

		// Pair interface																	
		template<class VT = T>
		NOD() typename VT::Key& getFirst() noexcept requires IsMap<M>;
		template<class VT = T>
		NOD() typename VT::Key const& getFirst() const noexcept requires IsMap<M>;
		template<class MT = V>
		NOD() MT& getSecond() noexcept requires IsMap<M>;
		template<class MT = V>
		NOD() MT const& getSecond() const noexcept requires IsMap<M>;

		// Value interface																
		template<class VT = T>
		NOD() VT const& getFirst() const noexcept requires IsSet<M>;
		template<class VT = T>
		NOD() VT& getFirst() noexcept requires IsSet<M>;
	};

} // namespace Langulus::Anyness::Inner

#include "DataNode.inl"
