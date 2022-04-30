#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	template<class T>
	concept IsNotAny = ReflectedData<T> && !IsDeep<T> && !IsSame<T, Disowned<Any>> && !IsSame<T, Abandoned<Any>>;
	
	///																								
	///	TAny																						
	///																								
	///	Unlike Any, this one is statically optimized to perform faster, due	
	/// to not being type-erased. In that sense, this container is equivalent	
	/// to std::vector																			
	///																								
	template<ReflectedData T>
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

		TAny(T&&) requires IsCustom<T>;
		TAny(const T&) requires IsCustom<T>;
		TAny(T&) requires IsCustom<T>;
		TAny(const T*, const Count&);

		TAny& operator = (const TAny&);
		TAny& operator = (TAny&);
		TAny& operator = (TAny&&);

		TAny& operator = (const Any&);
		TAny& operator = (Any&);
		TAny& operator = (Any&&);

		TAny& operator = (const Block&);
		TAny& operator = (Block&);
		TAny& operator = (Block&&);

		TAny& operator = (const Disowned<TAny>&);
		TAny& operator = (Abandoned<TAny>&&) noexcept;

		TAny& operator = (const T&) requires IsCustom<T>;
		TAny& operator = (T&) requires IsCustom<T>;
		TAny& operator = (T&&) requires IsCustom<T>;

	public:
		NOD() bool InterpretsAs(DMeta) const;
		NOD() bool InterpretsAs(DMeta, Count) const;
		
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

		NOD() TAny Clone() const;

		NOD() const T* GetRaw() const noexcept;
		NOD() T* GetRaw() noexcept;
		NOD() const T* GetRawEnd() const noexcept;
		NOD() T* GetRawEnd() noexcept;
		NOD() decltype(auto) Last() const SAFETY_NOEXCEPT();
		NOD() decltype(auto) Last() SAFETY_NOEXCEPT();

		template<ReflectedData ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) const SAFETY_NOEXCEPT();
		template<ReflectedData ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) SAFETY_NOEXCEPT();

		NOD() decltype(auto) operator [] (const Offset&) const SAFETY_NOEXCEPT() requires Langulus::IsDense<T>;
		NOD() decltype(auto) operator [] (const Offset&) SAFETY_NOEXCEPT() requires Langulus::IsDense<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires Langulus::IsDense<T>;
		NOD() decltype(auto) operator [] (const Index&) requires Langulus::IsDense<T>;

		struct SparseElement;

		NOD() decltype(auto) operator [] (const Offset&) const SAFETY_NOEXCEPT() requires Langulus::IsSparse<T>;
		NOD() SparseElement  operator [] (const Offset&) SAFETY_NOEXCEPT() requires Langulus::IsSparse<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires Langulus::IsSparse<T>;
		NOD() SparseElement  operator [] (const Index&) requires Langulus::IsSparse<T>;

		NOD() constexpr bool IsSparse() const noexcept;
		NOD() constexpr bool IsDense() const noexcept;
		NOD() constexpr Size GetStride() const noexcept;
		NOD() constexpr Size GetSize() const noexcept;

		NOD() bool Compare(const TAny&) const noexcept;
		NOD() bool CompareLoose(const TAny&) const noexcept requires IsCharacter<T>;
		NOD() Count Matches(const TAny&) const noexcept;
		NOD() Count MatchesLoose(const TAny&) const noexcept requires IsCharacter<T>;

		RANGED_FOR_INTEGRATION(TAny, T);

		Count Emplace(T&&, const Index& = Index::Back);

		Count Insert(T*, Count = 1, const Index& = Index::Back);
		TAny& operator << (const T&);
		TAny& operator << (T&&);
		TAny& operator >> (const T&);
		TAny& operator >> (T&&);

		Count Merge(const T*, Count = 1, const Index& = Index::Back);
		TAny& operator <<= (const T&);
		TAny& operator >>= (const T&);

		template<ReflectedData ALT_T = T>
		NOD() Index Find(const ALT_T&, const Index& = Index::Front) const;
		template<ReflectedData ALT_T = T>
		Count Remove(const ALT_T&, const Index& = Index::Front);

		void Sort(const Index&);

		NOD() TAny& Trim(const Count&);
		template<Anyness::IsBlock WRAPPER = TAny>
		NOD() WRAPPER Crop(const Offset&, const Count&) const;
		template<Anyness::IsBlock WRAPPER = TAny>
		NOD() WRAPPER Crop(const Offset&, const Count&);
		template<Anyness::IsBlock WRAPPER = TAny>
		NOD() WRAPPER Extend(const Count&);

		void Swap(const Offset&, const Offset&);
		void Swap(const Index&, const Index&);

		template<class WRAPPER = TAny, class RHS>
		TAny<T>& operator += (const RHS&);
		template<class WRAPPER = TAny, class RHS>
		NOD() WRAPPER operator + (const RHS&) const;

		/// Concatenate anything with this container										
		template<class WRAPPER = TAny, class LHS>
		NOD() friend WRAPPER operator + (const LHS& lhs, const TAny<T>& rhs) requires (!Inherits<LHS, TAny<T>>) {
			if constexpr (Langulus::IsSparse<LHS>)
				return operator + (*lhs, rhs);
			else if constexpr (Langulus::IsConvertible<LHS, WRAPPER>) {
				auto result = static_cast<WRAPPER>(lhs);
				result += rhs;
				return result;
			}
			else LANGULUS_ASSERT("Can't concatenate - LHS is not convertible to WRAPPER");
		}

	protected:
		template<bool OVERWRITE>
		void CopyProperties(const Block&) noexcept;
		void CallDefaultConstructors(const Count&);
		void CallCopyConstructors(const TAny&);
		void CallMoveConstructors(TAny&&);
		void CallDestructors();
	};


	///																								
	/// A sparse element access that dereferences on overwrite						
	///																								
	template<ReflectedData T>
	struct TAny<T>::SparseElement {
		Conditional<Langulus::IsConstant<T>, const T&, T&> mElement;

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
