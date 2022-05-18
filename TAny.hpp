///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
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
	public:
		TAny();
		~TAny();
		
		TAny(const TAny&);
		TAny(TAny&);
		TAny(TAny&&) noexcept;
		
		TAny(const Any&);
		TAny(Any&);
		TAny(Any&&);
		
		TAny(const Block&);
		TAny(Block&);
		TAny(Block&&);

		TAny(const Disowned<TAny>&) noexcept;
		TAny(Abandoned<TAny>&&) noexcept;

		TAny(T&&) requires CT::CustomData<T>;
		TAny(const T&) requires CT::CustomData<T>;
		TAny(T&) requires CT::CustomData<T>;
		TAny(const T*, const Count&);
		
		TAny& operator = (const TAny&);
		TAny& operator = (TAny&);
		TAny& operator = (TAny&&) noexcept;

		template<CT::Data ALT_T>
		TAny& operator = (ALT_T&);
		template<CT::Data ALT_T>
		TAny& operator = (ALT_T&&);

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

		NOD() TAny Clone() const;

		NOD() const T* GetRaw() const noexcept;
		NOD() T* GetRaw() noexcept;
		NOD() const T* GetRawEnd() const noexcept;
		NOD() T* GetRawEnd() noexcept;
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

		struct SparseElement;

		NOD() decltype(auto) operator [] (const Offset&) const SAFETY_NOEXCEPT() requires CT::Sparse<T>;
		NOD() SparseElement  operator [] (const Offset&) SAFETY_NOEXCEPT() requires CT::Sparse<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires CT::Sparse<T>;
		NOD() SparseElement  operator [] (const Index&) requires CT::Sparse<T>;

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

		Count Emplace(T&&, const Index& = Index::Back);

		Count Insert(const T*, const Count& = 1, const Index& = Index::Back);
		TAny& operator << (const T&);
		TAny& operator << (T&&);
		TAny& operator >> (const T&);
		TAny& operator >> (T&&);

		Count Merge(const T*, const Count& = 1, const Index& = Index::Back);
		TAny& operator <<= (const T&);
		TAny& operator <<= (T&&);
		TAny& operator >>= (const T&);
		TAny& operator >>= (T&&);

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

		/// Concatenate anything with this container										
		template<class LHS, class WRAPPER = TAny>
		NOD() friend WRAPPER operator + (const LHS& lhs, const TAny<T>& rhs) requires (!CT::DerivedFrom<LHS, TAny<T>>) {
			if constexpr (CT::Sparse<LHS>)
				return ::Langulus::Anyness::operator + (*lhs, rhs);
			else if constexpr (CT::Convertible<LHS, WRAPPER>) {
				auto result = static_cast<WRAPPER>(lhs);
				result += rhs;
				return result;
			}
			else LANGULUS_ASSERT("Can't concatenate - LHS is not convertible to WRAPPER");
		}

	protected:
		Size RequestByteSize(const Count&) const noexcept;

		template<bool OVERWRITE>
		void CopyProperties(const Block&) noexcept;
		void CallDefaultConstructors(const Count&);
		void CallCopyConstructors(const Count&, const TAny&);
	};


	///																								
	/// A sparse element access that dereferences on overwrite						
	///																								
	template<CT::Data T>
	struct TAny<T>::SparseElement {
		Conditional<CT::Constant<T>, const T&, T&> mElement;

		SparseElement& operator = (T);
		SparseElement& operator = (::std::nullptr_t);

		operator const T() const noexcept;
		operator T () noexcept;

		auto operator -> () const;
		auto operator -> ();

		decltype(auto) operator * () const;
		decltype(auto) operator * ();
	};

} // namespace Langulus::Anyness

#include "TAny.inl"
