#pragma once
#include "Text.hpp"

namespace Langulus::Anyness
{

	///																								
	///	An owned value, dense or sparse													
	///																								
	///	Provides only ownership, for when you need to cleanup after a move	
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

		NOD() Block GetBlock() const;

		void Reset() noexcept;

		constexpr TOwned& operator = (const TOwned&) noexcept = default;
		constexpr TOwned& operator = (TOwned&&) noexcept;
		constexpr TOwned& operator = (const T&) noexcept;

		NOD() Hash GetHash() const requires IsHashable<T>;

		NOD() decltype(auto) Get() const noexcept;
		NOD() decltype(auto) Get() noexcept;

		template<ReflectedData>
		NOD() auto As() const noexcept requires IsSparse<T>;

		NOD() auto operator -> () const requires IsSparse<T>;
		NOD() auto operator -> () requires IsSparse<T>;
		NOD() decltype(auto) operator * () const requires IsSparse<T>;
		NOD() decltype(auto) operator * () requires IsSparse<T>;

		NOD() explicit operator bool() const noexcept;
		NOD() explicit operator const T&() const noexcept;
		NOD() explicit operator T&() noexcept;

		NOD() bool operator == (const TOwned&) const noexcept;
		NOD() bool operator != (const TOwned&) const noexcept;
		NOD() bool operator == (const T&) const noexcept;
		NOD() bool operator != (const T&) const noexcept;
		NOD() bool operator == (::std::nullptr_t) const noexcept requires IsSparse<T>;
		NOD() bool operator != (::std::nullptr_t) const noexcept requires IsSparse<T>;
	};


	///																								
	///	A shared pointer																		
	///																								
	///	Provides ownership and referencing. Also, for single-element			
	/// containment, it is a bit more efficient than TAny. So, essentially		
	/// its equivalent to std::shared_ptr													
	///																								
	template<ReflectedData T, bool DOUBLE_REFERENCED>
	class TPointer : protected TOwned<Conditional<IsConstant<T>, const T*, T*>> {
	protected:
		using BASE = TOwned<Conditional<IsConstant<T>, const T*, T*>>;
		using Type = typename TOwned<Conditional<IsConstant<T>, const T*, T*>>::Type;
		Entry* mEntry {};

	public:
		constexpr TPointer() noexcept = default;
		TPointer(const TPointer&);
		TPointer(TPointer&&) noexcept;
		TPointer(Type);
		~TPointer();

		NOD() Block GetBlock() const;
		NOD() bool HasAuthority() const;
		NOD() Count GetReferences() const;
		NOD() DMeta GetType() const;
		using BASE::Get;

		NOD() static TPointer Create(const Decay<T>&) requires IsCopyConstructible<Decay<T>>;
		NOD() static TPointer Create(Decay<T>&&) requires IsMoveConstructible<Decay<T>>;
		NOD() static TPointer Create() requires IsDefaultConstructible<Decay<T>>;

		template<typename... ARGS>
		NOD() static TPointer New(ARGS&&...);

		void Reset();

		NOD() explicit operator bool() const noexcept;
		NOD() explicit operator const T* () const noexcept;
		NOD() explicit operator Type () noexcept;

		TPointer& operator = (const TPointer&);
		TPointer& operator = (TPointer&&);
		TPointer& operator = (Type);

		template<IsSparse ALT_T>
		TPointer& operator = (ALT_T);
		template<ReflectedData ALT_T>
		TPointer& operator = (const TPointer<ALT_T, DOUBLE_REFERENCED>&);

		NOD() operator TPointer<const T, DOUBLE_REFERENCED>() const noexcept requires IsMutable<T>;

		NOD() bool operator == (const TPointer&) const noexcept;
		NOD() bool operator != (const TPointer&) const noexcept;
		NOD() bool operator == (const T*) const noexcept;
		NOD() bool operator != (const T*) const noexcept;
		NOD() bool operator == (::std::nullptr_t) const noexcept;
		NOD() bool operator != (::std::nullptr_t) const noexcept;

		NOD() auto operator -> () const;
		NOD() auto operator -> ();
		NOD() decltype(auto) operator * () const;
		NOD() decltype(auto) operator * ();
	};

	/// Just a handle for a pointer, that provides ownage								
	/// Pointer will be explicitly nulled after a move									
	template<ReflectedData T>
	using Own = TOwned<T>;

	/// A shared pointer, that provides ownage and basic reference counting		
	/// Referencing comes from the block of memory that the pointer points to	
	/// The memory block might contain more data, that will be implicitly		
	/// referenced, too																			
	template<ReflectedData T>
	using Ptr = TPointer<T, false>;

	/// A shared pointer, that provides ownage and more reference counting		
	/// Referencing comes first from the block of memory that the pointer		
	/// points to, and second - the instance's individual reference counter		
	/// Useful for keeping track not only of the memory, but of the individual	
	/// element inside the memory block														
	template<ReflectedData T>
	using Ref = TPointer<T, true>;

} // namespace Langulus::Anyness

#include "TPointer.inl"
