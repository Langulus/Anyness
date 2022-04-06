#pragma once
#include "Text.hpp"

namespace Langulus::Anyness
{

	///																								
	///	An owned value, dense or sparse													
	///																								
	/// Provides only ownership, for when you need to cleanup after move			
	/// Value will be reset to default after move										
	/// T must be default constructible														
	///																								
	template<class T>
	class EMPTY_BASE() TOwned : NOT_REFLECTED {
	public:
		T mValue = {};

	public:
		using Type = T;

		constexpr TOwned() noexcept = default;
		constexpr TOwned(const TOwned&) noexcept = default;
		constexpr TOwned(TOwned&&) noexcept;
		constexpr TOwned(const T&) noexcept;

		NOD() DMeta GetMeta() const;
		NOD() Block GetBlock() const;
		NOD() bool CheckJurisdiction() const;
		NOD() pcref GetBlockReferences() const;

		void Reset() noexcept;

		constexpr TOwned& operator = (const TOwned&) noexcept = default;
		constexpr TOwned& operator = (TOwned&&) noexcept;
		constexpr TOwned& operator = (const T&) noexcept;

		NOD() Hash GetHash() const requires Hashable<T>;

		NOD() decltype(auto) Get() const noexcept;
		NOD() decltype(auto) Get() noexcept;

		template<class D>
		NOD() auto As() const noexcept requires Sparse<T>;

		NOD() auto operator -> () const SAFE_NOEXCEPT() requires Sparse<T>;
		NOD() auto operator -> () SAFE_NOEXCEPT() requires Sparse<T>;
		NOD() decltype(auto) operator * () const SAFE_NOEXCEPT() requires Sparse<T>;
		NOD() decltype(auto) operator * () SAFE_NOEXCEPT() requires Sparse<T>;

		NOD() explicit operator bool() const noexcept;
		NOD() operator const T&() const noexcept;
		NOD() operator T&() noexcept;

		NOD() bool operator == (const TOwned<T>&) const noexcept;
		NOD() bool operator != (const TOwned<T>&) const noexcept;
		NOD() bool operator == (const T&) const noexcept;
		NOD() bool operator != (const T&) const noexcept;
		NOD() bool operator == (std::nullptr_t) const noexcept;
		NOD() bool operator != (std::nullptr_t) const noexcept;
	};


	///																								
	///	A shared pointer																		
	///																								
	/// Provides ownership and referencing													
	///																								
	template<class T, bool DOUBLE_REFERENCED>
	class TPointer : public TOwned<Conditional<Constant<T>, const T*, T*>> {
	protected:
		static inline DMeta sMeta = nullptr;

		using BASE = TOwned<Conditional<Constant<T>, const T*, T*>>;
		using SparseT = typename BASE::Type;

		static_assert(!Same<T, void>,
			"Can't create void container");
		static_assert(!Same<T, Block>, 
			"TPointer to Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks");

		REFLECT_MANUALLY(TPointer) {
			static_assert(sizeof(SparseT) == sizeof(ME), "Size mismatch");
			static_assert(sizeof(pcptr) == sizeof(ME), "Size mismatch");
			static Text name, info;
			if (name.IsEmpty()) {
				name += "Ptr";
				name += DataID::Reflect<pcDecay<T>>()->GetToken();
				name = name + "," + name + "Ptr," + name + "ConstPtr";
				info += "a pointer to ";
				info += DataID::Reflect<pcDecay<T>>()->GetToken();
			}

			auto reflection = RTTI::ReflectData::From<ME>(name, info);
			reflection.template SetBases<ME>(
				REFLECT_BASE_MAP(SparseT, 1));
			return reflection;
		}

	public:
		TPointer();
		TPointer(const TPointer&);
		TPointer(TPointer&&) noexcept;
		TPointer(SparseT);
		~TPointer();

		NOD() DMeta GetMeta() const;
		NOD() Block GetBlock() const;
		NOD() bool CheckJurisdiction() const;
		NOD() pcref GetBlockReferences() const;

		NOD() static ME Create(const pcDecay<T>&) requires CopyConstructible<pcDecay<T>>;
		NOD() static ME Create(pcDecay<T>&&) requires MoveConstructible<pcDecay<T>>;
		NOD() static ME Create() requires DefaultConstructible<pcDecay<T>>;

		template<typename... ARGS>
		NOD() static ME New(ARGS&&...);

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

	template<class T>
	NOD() bool operator == (const pcDecay<T>*, const TOwned<T>&) noexcept;
	template<class T>
	NOD() bool operator != (const pcDecay<T>*, const TOwned<T>&) noexcept;
	template<class T>
	NOD() bool operator == (std::nullptr_t, const TOwned<T>&) noexcept;
	template<class T>
	NOD() bool operator != (std::nullptr_t, const TOwned<T>&) noexcept;

} // namespace Langulus::Anyness

#include "TPointer.inl"