#pragma once
#include "Text.hpp"

namespace Langulus::Anyness
{

	///																								
	///	An owned value, dense or sparse													
	///																								
	/// Provides only ownership, for when you need to cleanup after a move		
	/// By default, fundamental types and pointers are not reset after a move	
	/// Wrapping them inside this ensures they are										
	///																								
	template<ReflectedData T>
	class TOwned {
	public:
		T mValue {};

	public:
		using Type = T;

		constexpr TOwned() noexcept = default;
		constexpr TOwned(const TOwned&) noexcept = default;
		constexpr TOwned(TOwned&&) noexcept;
		constexpr TOwned(const T&) noexcept;

		//NOD() DMeta GetMeta() const;
		NOD() Block GetBlock() const;
		//NOD() bool CheckJurisdiction() const;
		//NOD() Count GetBlockReferences() const;

		void Reset() noexcept;

		constexpr TOwned& operator = (const TOwned&) noexcept = default;
		constexpr TOwned& operator = (TOwned&&) noexcept;
		constexpr TOwned& operator = (const T&) noexcept;

		NOD() Hash GetHash() const requires Hashable<T>;

		NOD() decltype(auto) Get() const noexcept;
		NOD() decltype(auto) Get() noexcept;

		template<class D>
		NOD() auto As() const noexcept requires Sparse<T>;

		NOD() auto operator -> () const requires Sparse<T>;
		NOD() auto operator -> () requires Sparse<T>;
		NOD() decltype(auto) operator * () const requires Sparse<T>;
		NOD() decltype(auto) operator * () requires Sparse<T>;

		NOD() explicit operator bool() const noexcept;
		NOD() operator const T&() const noexcept;
		NOD() operator T&() noexcept;

		NOD() bool operator == (const TOwned&) const noexcept;
		NOD() bool operator != (const TOwned&) const noexcept;
		NOD() bool operator == (const T&) const noexcept;
		NOD() bool operator != (const T&) const noexcept;
		NOD() bool operator == (::std::nullptr_t) const noexcept;
		NOD() bool operator != (::std::nullptr_t) const noexcept;
		
		NOD() friend bool operator == (T lhs, const TOwned& rhs) noexcept requires Sparse<T> {
			return lhs == rhs.Get();
		}
		
		NOD() friend bool operator != (T lhs, const TOwned& rhs) noexcept requires Sparse<T> {
			return lhs != rhs.Get();
		}
				
		NOD() friend bool operator == (::std::nullptr_t, const TOwned& rhs) noexcept requires Sparse<T> {
			return nullptr == rhs.Get();
		}
				
		NOD() friend bool operator != (::std::nullptr_t, const TOwned& rhs) noexcept requires Sparse<T> {
			return nullptr != rhs.Get();
		}
	};


	///																								
	///	A shared pointer																		
	///																								
	/// Provides ownership and referencing													
	///																								
	template<ReflectedData T, bool DOUBLE_REFERENCED>
	class TPointer : public TOwned<Conditional<Constant<T>, const T*, T*>> {
	protected:
		using BASE = TOwned<Conditional<Constant<T>, const T*, T*>>;
		using SparseT = typename BASE::Type;

	public:
		constexpr TPointer() noexcept = default;
		TPointer(const TPointer&);
		TPointer(TPointer&&) noexcept;
		TPointer(SparseT);
		~TPointer();

		//NOD() DMeta GetMeta() const;
		NOD() Block GetBlock() const;
		//NOD() bool CheckJurisdiction() const;
		//NOD() RefCount GetBlockReferences() const;

		NOD() static TPointer Create(const Decay<T>&) requires CopyConstructible<Decay<T>>;
		NOD() static TPointer Create(Decay<T>&&) requires MoveConstructible<Decay<T>>;
		NOD() static TPointer Create() requires DefaultConstructible<Decay<T>>;

		template<typename... ARGS>
		NOD() static TPointer New(ARGS&&...);

		void Reset();

		TPointer& operator = (const TPointer&);
		TPointer& operator = (TPointer&&);
		TPointer& operator = (SparseT);

		template<Sparse ANY_POINTER>
		TPointer& operator = (ANY_POINTER);
		template<class ANY_POINTER>
		TPointer& operator = (const TPointer<ANY_POINTER, DOUBLE_REFERENCED>&);

		NOD() operator TPointer<const T, DOUBLE_REFERENCED>() const noexcept requires Mutable<T>;
	};

	/// Just a handle for a pointer, that provides ownage								
	/// Pointer will be explicitly nulled after a move									
	template<class T>
	using Own = TOwned<T>;

	/// A shared pointer, that provides ownage and basic reference counting		
	/// Referencing comes from the block of memory that the pointer points to	
	/// The memory block might contain more data, that will be implicitly		
	/// referenced, too																			
	template<class T>
	using Ptr = TPointer<T, false>;

	/// A shared pointer, that provides ownage and more reference counting		
	/// Referencing comes first from the block of memory that the pointer		
	/// points to, and second - the instance's individual reference counter		
	/// Useful for keeping track not only of the memory, but of the individual	
	/// element inside the memory block														
	template<class T>
	using Ref = TPointer<T, true>;

} // namespace Langulus::Anyness

#include "TPointer.inl"
