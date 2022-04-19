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
	template<ReflectedData T>
	class TAny : public Any {
	public:
		static constexpr bool NotCustom =
			Sparse<T> || (!Same<T, TAny<T>> && !Same<T, Any> && !Same<T, Block>);

		TAny();
		TAny(const TAny&);
		TAny(TAny&&) noexcept;
		TAny(const Any&);
		TAny(Any&&);
		TAny(const Block&);
		TAny(Block&&);

		TAny(const Disowned<TAny>&) noexcept;
		TAny(Abandoned<TAny>&&) noexcept;

		TAny(T&&) requires (TAny<T>::NotCustom);
		TAny(const T&) requires (TAny<T>::NotCustom);
		TAny(T&) requires (TAny<T>::NotCustom);
		TAny(const T*, const Count&);

		TAny& operator = (const TAny&);
		TAny& operator = (TAny&&);

		TAny& operator = (const Any&);
		TAny& operator = (Any&&);

		TAny& operator = (const Block&);
		TAny& operator = (Block&&);

		TAny& operator = (const Disowned<TAny>&);
		TAny& operator = (Abandoned<TAny>&&) noexcept;

		TAny& operator = (const T&) requires (TAny<T>::NotCustom);
		TAny& operator = (T&) requires (TAny<T>::NotCustom);
		TAny& operator = (T&&) requires (TAny<T>::NotCustom);

	public:
		NOD() bool InterpretsAs(DMeta) const;
		NOD() bool InterpretsAs(DMeta, Count) const;
		
		NOD() static TAny Wrap(const T&);
		template<Count COUNT>
		NOD() static TAny Wrap(const T(&anything)[COUNT]);
		NOD() static TAny Wrap(const T*, const Count&);

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
		NOD() decltype(auto) Last() const;
		NOD() decltype(auto) Last();

		template<ReflectedData ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) const noexcept;
		template<ReflectedData ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) noexcept;

		NOD() decltype(auto) operator [] (const Offset&) const noexcept requires Dense<T>;
		NOD() decltype(auto) operator [] (const Offset&) noexcept requires Dense<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires Dense<T>;
		NOD() decltype(auto) operator [] (const Index&) requires Dense<T>;

		struct SparseElement;

		NOD() decltype(auto) operator [] (const Offset&) const noexcept requires Sparse<T>;
		NOD() SparseElement  operator [] (const Offset&) noexcept requires Sparse<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires Sparse<T>;
		NOD() SparseElement  operator [] (const Index&) requires Sparse<T>;

		NOD() constexpr bool IsSparse() const noexcept;
		NOD() constexpr bool IsDense() const noexcept;
		NOD() constexpr Stride GetStride() const noexcept;

		RANGED_FOR_INTEGRATION(TAny, T);

		Count Emplace(T&&, const Index& = Index::Back);

		Count Insert(const T*, Count = 1, const Index& = Index::Back);
		TAny& operator << (const T&);
		TAny& operator << (T&&);
		TAny& operator >> (const T&);
		TAny& operator >> (T&&);

		Count Merge(const T*, Count = 1, const Index& = Index::Back);
		TAny& operator <<= (const T&);
		TAny& operator >>= (const T&);

		NOD() Index Find(MakeConst<T>, const Index& = Index::Front) const;
		Count Remove(MakeConst<T>, const Index& = Index::Front);

		void Sort(const Index&);

		NOD() TAny& Trim(const Count&);
		template<Deep WRAPPER = TAny>
		NOD() WRAPPER Crop(const Offset&, const Count&) const;
		template<Deep WRAPPER = TAny>
		NOD() WRAPPER Crop(const Offset&, const Count&);
		template<Deep WRAPPER = TAny>
		NOD() WRAPPER Extend(const Count&);

		void Swap(const Offset&, const Offset&);
		void Swap(const Index&, const Index&);

		template<class WRAPPER = TAny, class RHS>
		WRAPPER& operator += (const RHS&);
		template<class WRAPPER = TAny, class RHS>
		NOD() WRAPPER operator + (const RHS&) const;

		/// Concatenate anything with this container										
		template<class WRAPPER = TAny, class LHS>
		NOD() friend WRAPPER operator + (const LHS& lhs, const TAny<T>& rhs) requires (!Inherits<LHS, TAny<T>>) {
			if constexpr (Sparse<LHS>)
				return operator + (*lhs, rhs);
			else if constexpr (Convertible<LHS, WRAPPER>) {
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
	};


	///																								
	/// A sparse element access that dereferences on overwrite						
	///																								
	template<ReflectedData T>
	struct TAny<T>::SparseElement {
		Conditional<Constant<T>, const T&, T&> mElement;

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
