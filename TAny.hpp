///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{
	
	///																								
	///	TAny																						
	///																								
	///	Unlike Any, this one is statically optimized to perform faster, due	
	/// to not being type-erased. In that sense, this container is equivalent	
	/// to std::vector																			
	///																								
	template<CT::Data T>
	class TAny : public Any {
		template<CT::Data, CT::Data>
		friend class THashMap;

	public:
		TAny();
		~TAny();
		
		TAny(const TAny&);
		TAny(TAny&&) noexcept;

		TAny(Disowned<TAny>&&) noexcept;
		TAny(Abandoned<TAny>&&) noexcept;

		TAny(const Any&);
		TAny(Any&&);

		TAny(Disowned<Any>&&);
		TAny(Abandoned<Any>&&);

		TAny(const Block&);
		TAny(Block&&);

		TAny(const T&) requires CT::CustomData<T>;
		TAny(T&&) requires CT::CustomData<T>;

		TAny(Disowned<T>&&) noexcept requires  (CT::CustomData<T> && CT::Dense<T>);
		TAny(Abandoned<T>&&) noexcept requires (CT::CustomData<T> && CT::Dense<T>);
		TAny(Disowned<T>&&) noexcept requires  (CT::CustomData<T> && CT::Sparse<T>);
		TAny(Abandoned<T>&&) noexcept requires (CT::CustomData<T> && CT::Sparse<T>);

		TAny(const T*, const Count&);
		TAny(Disowned<const T*>&&, const Count&) noexcept;
		
		TAny& operator = (const TAny&);
		TAny& operator = (TAny&&) noexcept;

		TAny& operator = (Disowned<TAny>&&) noexcept;
		TAny& operator = (Abandoned<TAny>&&) noexcept;

		TAny& operator = (const Any&);
		TAny& operator = (Any&&);

		TAny& operator = (Disowned<Any>&&);
		TAny& operator = (Abandoned<Any>&&);

		TAny& operator = (const Block&);
		TAny& operator = (Block&&);

		TAny& operator = (const T&) requires CT::CustomData<T>;
		TAny& operator = (T&&) requires CT::CustomData<T>;

		TAny& operator = (Disowned<T>&&) noexcept requires CT::CustomData<T>;
		TAny& operator = (Abandoned<T>&&) noexcept requires CT::CustomData<T>;

	public:
		NOD() bool CastsToMeta(DMeta) const;
		NOD() bool CastsToMeta(DMeta, Count) const;
		
		NOD() static TAny Wrap(const T&);
		template<Count COUNT>
		NOD() static TAny Wrap(const T(&anything)[COUNT]);
		NOD() static TAny Wrap(const T*, const Count&);

		template<bool CREATE = false>
		void Allocate(Count);
	
		void Null(const Count&);
		void Clear();
		void Reset();
		void ResetState() noexcept;
		void TakeAuthority();
		void Free();

		NOD() TAny Clone() const requires (CT::CloneMakable<T> || CT::POD<T>);

		NOD() auto GetRaw() const noexcept;
		NOD() auto GetRaw() noexcept;
		NOD() auto GetRawEnd() const noexcept;
		NOD() auto GetRawEnd() noexcept;
		NOD() decltype(auto) Last() const SAFETY_NOEXCEPT();
		NOD() decltype(auto) Last() SAFETY_NOEXCEPT();

		template<CT::Data ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) const SAFETY_NOEXCEPT();
		template<CT::Data ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) SAFETY_NOEXCEPT();

		NOD() decltype(auto) operator [] (const Offset&) const SAFETY_NOEXCEPT() requires CT::Dense<T>;
		NOD() decltype(auto) operator [] (const Offset&) SAFETY_NOEXCEPT() requires CT::Dense<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires CT::Dense<T>;
		NOD() decltype(auto) operator [] (const Index&) requires CT::Dense<T>;

		struct KnownPointer;

		NOD() decltype(auto) operator [] (const Offset&) const SAFETY_NOEXCEPT() requires CT::Sparse<T>;
		NOD() KnownPointer&  operator [] (const Offset&) SAFETY_NOEXCEPT() requires CT::Sparse<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires CT::Sparse<T>;
		NOD() KnownPointer&  operator [] (const Index&) requires CT::Sparse<T>;

		NOD() constexpr bool IsUntyped() const noexcept;
		NOD() constexpr bool IsTypeConstrained() const noexcept;
		NOD() constexpr bool IsAbstract() const noexcept;
		NOD() constexpr bool IsConstructible() const noexcept;
		NOD() constexpr bool IsDeep() const noexcept;
		NOD() constexpr bool IsSparse() const noexcept;
		NOD() constexpr bool IsDense() const noexcept;
		NOD() constexpr Size GetStride() const noexcept;
		NOD() constexpr Size GetSize() const noexcept;

		NOD() bool Compare(const TAny&) const noexcept;
		NOD() bool CompareLoose(const TAny&) const noexcept requires CT::Character<T>;
		NOD() Count Matches(const TAny&) const noexcept;
		NOD() Count MatchesLoose(const TAny&) const noexcept requires CT::Character<T>;

		RANGED_FOR_INTEGRATION(TAny, T);

		template<bool KEEP = true>
		Count InsertAt(const T*, const T*, Index);
		template<bool KEEP = true>
		Count InsertAt(const T*, const T*, Offset);
		template<bool KEEP = true>
		Count InsertAt(T&&, Index);
		template<bool KEEP = true>
		Count InsertAt(T&&, Offset);

		template<Index INDEX, bool KEEP = true>
		Count Insert(const T*, const T*);
		template<Index INDEX, bool KEEP = true>
		Count Insert(T&&);

		TAny& operator << (const T&);
		TAny& operator << (T&&);
		TAny& operator << (Disowned<T>&&);
		TAny& operator << (Abandoned<T>&&);

		TAny& operator >> (const T&);
		TAny& operator >> (T&&);
		TAny& operator >> (Disowned<T>&&);
		TAny& operator >> (Abandoned<T>&&);

		template<bool KEEP = true>
		Count MergeAt(const T*, const T*, Index);
		template<bool KEEP = true>
		Count MergeAt(const T*, const T*, Offset);
		template<bool KEEP = true>
		Count MergeAt(T&&, Index);
		template<bool KEEP = true>
		Count MergeAt(T&&, Offset);

		template<Index INDEX, bool KEEP = true>
		Count Merge(const T*, const T*);
		template<Index INDEX, bool KEEP = true>
		Count Merge(T&&);

		TAny& operator <<= (const T&);
		TAny& operator <<= (T&&);
		TAny& operator <<= (Disowned<T>&&);
		TAny& operator <<= (Abandoned<T>&&);

		TAny& operator >>= (const T&);
		TAny& operator >>= (T&&);
		TAny& operator >>= (Disowned<T>&&);
		TAny& operator >>= (Abandoned<T>&&);

		template<CT::Data ALT_T>
		bool operator == (const TAny<ALT_T>&) const noexcept;
		bool operator == (const Any&) const noexcept;

		template<CT::Data ALT_T = T, bool REVERSE = false>
		NOD() Index Find(const ALT_T&) const;
		template<CT::Data ALT_T = T, bool REVERSE = false>
		Count RemoveValue(const ALT_T&);
		Count RemoveIndex(const Count&, const Count&);

		template<bool ASCEND = false>
		void Sort();

		NOD() TAny& Trim(const Count&);
		template<CT::Block WRAPPER = TAny>
		NOD() WRAPPER Crop(const Offset&, const Count&) const;
		template<CT::Block WRAPPER = TAny>
		NOD() WRAPPER Crop(const Offset&, const Count&);
		template<CT::Block WRAPPER = TAny>
		NOD() WRAPPER Extend(const Count&);

		void Swap(const Offset&, const Offset&);
		void Swap(const Index&, const Index&);

		template<class WRAPPER = TAny, class RHS>
		TAny<T>& operator += (const RHS&);
		template<class WRAPPER = TAny, class RHS>
		NOD() WRAPPER operator + (const RHS&) const;

	protected:
		NOD() auto RequestSize(const Count&) const noexcept;

		template<bool OVERWRITE>
		void CopyProperties(const Block&) noexcept;
	};


	///																								
	/// A sparse element access that dereferences on overwrite						
	///																								
	template<CT::Data T>
	struct TAny<T>::KnownPointer {
		T mPointer;
		Inner::Allocation* mEntry;

		KnownPointer& operator = (T);
		KnownPointer& operator = (::std::nullptr_t);

		operator T() const noexcept;
		operator T() noexcept;

		auto operator -> () const;
		auto operator -> ();

		decltype(auto) operator * () const;
		decltype(auto) operator * ();
	};

	/// Concatenate anything with TAny container											
	template<class T, class LHS, class WRAPPER = TAny<T>>
	NOD() WRAPPER operator + (const LHS& lhs, const TAny<T>& rhs) requires (!CT::DerivedFrom<LHS, TAny<T>>) {
		if constexpr (CT::Sparse<LHS>)
			return operator + (*lhs, rhs);
		else if constexpr (CT::Convertible<LHS, WRAPPER>) {
			auto result = static_cast<WRAPPER>(lhs);
			result += rhs;
			return result;
		}
		else LANGULUS_ASSERT("Can't concatenate - LHS is not convertible to WRAPPER");
	}

} // namespace Langulus::Anyness

#include "TAny.inl"
