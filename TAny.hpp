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

		TAny(const Inner::Block&);
		TAny(Inner::Block&&);

		TAny(T&&) requires (TAny<T>::NotCustom);
		TAny(const T&) requires (TAny<T>::NotCustom);
		TAny(T&) requires (TAny<T>::NotCustom);

		TAny& operator = (const TAny&);
		TAny& operator = (TAny&&) noexcept;

		TAny& operator = (const Any&);
		TAny& operator = (Any&&);

		TAny& operator = (const Inner::Block&);
		TAny& operator = (Inner::Block&&);

		TAny& operator = (const T&) requires (TAny<T>::NotCustom);
		TAny& operator = (T&) requires (TAny<T>::NotCustom);
		TAny& operator = (T&&) requires (TAny<T>::NotCustom);

	public:
		NOD() static TAny Wrap(const T&);
		template<Count COUNT>
		NOD() static TAny Wrap(const T(&anything)[COUNT]);
		NOD() static TAny Wrap(const T*, const Count&);

		void Null(const Count&);
		void Clear();
		void Reset();
		NOD() TAny Clone() const;

		NOD() const T* GetRaw() const noexcept;
		NOD() T* GetRaw() noexcept;

		template<ReflectedData ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) const noexcept;
		template<ReflectedData ALT_T = T>
		NOD() decltype(auto) Get(const Offset&) noexcept;

		NOD() decltype(auto) operator [] (const Offset&) const noexcept requires Dense<T>;
		NOD() decltype(auto) operator [] (const Offset&) noexcept requires Dense<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires Dense<T>;
		NOD() decltype(auto) operator [] (const Index&) requires Dense<T>;

		struct SparseElement {
			Conditional<Constant<T>, const T&, T&> mElement;

			SparseElement& operator = (T newPtr) {
				if (mElement == newPtr)
					return *this;

				auto meta = MetaData::Of<T>();
				if (mElement) {
					auto refs = PCMEMORY.GetReferences(meta, mElement);
					if (refs - 1 <= 0)
						delete mElement;
					PCMEMORY.Reference(meta, mElement, -1);
				}

				mElement = newPtr;
				PCMEMORY.Reference(meta, newPtr, 1);
				return *this;
			}

			operator const T () const noexcept {
				return mElement;
			}
			operator T () noexcept {
				return mElement;
			}

			auto operator -> () const {
				if (!mElement)
					throw Except::Access("Invalid pointer");
				return mElement;
			}

			auto operator -> () {
				if (!mElement)
					throw Except::Access("Invalid pointer");
				return mElement;
			}

			decltype(auto) operator * () const {
				if (!mElement)
					throw Except::Access("Invalid pointer");
				return *mElement;
			}

			decltype(auto) operator * () {
				if (!mElement)
					throw Except::Access("Invalid pointer");
				return *mElement;
			}
		};

		NOD() decltype(auto) operator [] (const Offset&) const noexcept requires Sparse<T>;
		NOD() SparseElement operator [] (const Offset&) noexcept requires Sparse<T>;
		NOD() decltype(auto) operator [] (const Index&) const requires Sparse<T>;
		NOD() SparseElement operator [] (const Index&) requires Sparse<T>;

		NOD() decltype(auto) Last();
		NOD() decltype(auto) Last() const;

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

		NOD() TAny Crop(const Offset&, const Count&) const;
		NOD() TAny& Trim(const Count&);

		void Swap(const Offset&, const Offset&);
		void Swap(const Index&, const Index&);
	};

} // namespace Langulus::Anyness

#include "TAny.inl"
