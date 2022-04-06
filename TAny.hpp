#pragma once
#include "inner/Block.hpp"

namespace Langulus::Anyness
{

	///																								
	///	TEMPLATED DATA CONTAINER															
	///																								
	/// Employs some compile-time optimizations to an Any container				
	///																								
	template<RTTI::ReflectedData T>
	class TAny : public Any {
		static_assert(!Same<T, void>, 
			"Can't create void container");
		static_assert(!Same<T, Block>, 
			"TPointer to Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks");

		REFLECT_MANUALLY(TAny) {
			static_assert(sizeof(Any) == sizeof(TAny<Any>), "Size mismatch");
			static_assert(sizeof(Any) == sizeof(TAny<int>), "Size mismatch");
			static_assert(sizeof(Any) == sizeof(TAny<char>), "Size mismatch");
			static Text name, info;
			if (name.IsEmpty()) {
				name += "Any";
				name += DataID::Reflect<T>()->GetToken();
				name = name + "," + name + "Ptr," + name + "ConstPtr";
				info += "a type-constrained container for type ";
				info += DataID::Reflect<T>()->GetToken();
			}

			auto reflection = RTTI::ReflectData::From<ME>(name, info);
			reflection.mIsDeep = true;
			reflection.template SetBases<ME>(
				REFLECT_BASE(Any));
			return reflection;
		}

	public:
		static constexpr bool NotCustom =
			Sparse<T> || (!Same<T, ME> && !Same<T, Any> && !Same<T, Block>);

		TAny();

		TAny(const TAny<T>&);
		TAny(TAny<T>&&) noexcept;

		TAny(const Any&);
		TAny(Any&&);

		TAny(const Block&);
		TAny(Block&&);

		TAny(T&&) requires (TAny<T>::NotCustom);
		TAny(const T&) requires (TAny<T>::NotCustom);
		TAny(T&) requires (TAny<T>::NotCustom);

		TAny<T>& operator = (const TAny<T>&);
		TAny<T>& operator = (TAny<T>&&) noexcept;

		TAny<T>& operator = (const Any&);
		TAny<T>& operator = (Any&&);

		TAny<T>& operator = (const Block&);
		TAny<T>& operator = (Block&&);

		TAny<T>& operator = (const T&) requires (TAny<T>::NotCustom);
		TAny<T>& operator = (T&) requires (TAny<T>::NotCustom);
		TAny<T>& operator = (T&&) requires (TAny<T>::NotCustom);

	public:
		NOD() static TAny<T> Wrap(const T&);
		template<pcptr COUNT>
		NOD() static TAny<T> Wrap(const T(&anything)[COUNT]);
		NOD() static TAny<T> Wrap(const T*, const pcptr);

		void Null(pcptr);
		void Clear();
		void Reset();
		NOD() TAny<T> Clone() const;

		NOD() const T* GetRaw() const noexcept;
		NOD() T* GetRaw() noexcept;

		template<RTTI::ReflectedData K = T>
		NOD() decltype(auto) Get(const pcptr) const noexcept;
		template<RTTI::ReflectedData K = T>
		NOD() decltype(auto) Get(const pcptr) noexcept;

		NOD() decltype(auto) operator [] (const pcptr&) const noexcept requires Dense<T>;
		NOD() decltype(auto) operator [] (const pcptr&) noexcept requires Dense<T>;
		NOD() decltype(auto) operator [] (const Index&) const SAFE_NOEXCEPT() requires Dense<T>;
		NOD() decltype(auto) operator [] (Index) SAFE_NOEXCEPT() requires Dense<T>;

		struct SparseElement {
			Conditional<Constant<T>, const T&, T&> mElement;

			SparseElement& operator = (T newPtr) SAFE_NOEXCEPT() {
				if (mElement == newPtr)
					return *this;

				static const auto meta = MetaData::Of<T>();
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

			operator const T () const noexcept { return mElement; }
			operator T () noexcept { return mElement; }

			auto operator -> () const SAFE_NOEXCEPT() {
				SAFETY(if (!mElement)
					throw Except::BadAccess("Invalid pointer"));
				return mElement;
			}

			auto operator -> () SAFE_NOEXCEPT() {
				SAFETY(if (!mElement)
					throw Except::BadAccess("Invalid pointer"));
				return mElement;
			}

			decltype(auto) operator * () const SAFE_NOEXCEPT() {
				SAFETY(if (!mElement)
					throw Except::BadAccess("Invalid pointer"));
				return *mElement;
			}

			decltype(auto) operator * () SAFE_NOEXCEPT() {
				SAFETY(if (!mElement)
					throw Except::BadAccess("Invalid pointer"));
				return *mElement;
			}
		};

		NOD() decltype(auto) operator [] (const pcptr&) const noexcept requires Sparse<T>;
		NOD() SparseElement operator [] (const pcptr&) noexcept requires Sparse<T>;
		NOD() decltype(auto) operator [] (const Index&) const SAFE_NOEXCEPT() requires Sparse<T>;
		NOD() SparseElement operator [] (Index) SAFE_NOEXCEPT() requires Sparse<T>;

		NOD() Any Decay() const;

		NOD() decltype(auto) Last();
		NOD() decltype(auto) Last() const;

		NOD() constexpr bool IsSparse() const noexcept;
		NOD() constexpr bool IsDense() const noexcept;
		NOD() constexpr pcptr GetStride() const noexcept;

		PC_RANGED_FOR_INTEGRATION(T, GetRaw(), GetCount())

		pcptr Emplace(T&&, const Index& = uiBack);

		pcptr Insert(const T*, const pcptr = 1, const Index& = uiBack);
		TAny<T>& operator << (const T&);
		TAny<T>& operator << (T&&);
		TAny<T>& operator >> (const T&);
		TAny<T>& operator >> (T&&);

		pcptr Merge(const T*, const pcptr = 1, const Index& = uiBack);
		TAny<T>& operator <<= (const T&);
		TAny<T>& operator >>= (const T&);

		NOD() Index Find(MakeConst<T>, const Index& = uiFront) const;
		pcptr Remove(MakeConst<T>, const Index& = uiFront);

		void Sort(const Index&);

		NOD() TAny<T> Crop(pcptr, pcptr) const;
		NOD() TAny<T>& Trim(const pcptr);

		void Swap(pcptr, pcptr);
		void Swap(Index, Index);
	};

} // namespace Langulus::Anyness

#include "TAny.inl"
