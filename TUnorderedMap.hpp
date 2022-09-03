///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "TAny.hpp"

namespace Langulus::Anyness
{

	///																								
	/// A highly optimized unordered hashmap implementation, using the Robin	
	/// Hood algorithm																			
	///																								
	template<CT::Data K, CT::Data V>
	class TUnorderedMap {
	public:
		static_assert(CT::Comparable<K>, "Can't compare keys for map");
		using Pair = TPair<K, V>;
		using PairRef = TPair<K&, V&>;
		using PairConstRef = TPair<const K&, const V&>;
		using Key = K;
		using Value = V;
		using KeyInner = typename TAny<K>::TypeInner;
		using ValueInner = typename TAny<V>::TypeInner;
		using Self = TUnorderedMap<K, V>;
		using Allocator = Inner::Allocator;
		static constexpr Count MinimalAllocation = 8;

	protected:
		using InfoType = uint8_t;

		// The allocation which holds keys and tombstones						
		Inner::Allocation* mKeys {};

		// A precomputed pointer for the info bytes								
		// Points to an offset inside mKeys allocation							
		// Each byte represents a pair, and can be three things:				
		//		0 - the index is not used, data is not initialized				
		//		1 - the index is used, and key is exactly where it should be
		//		2+ - the index is used, but bucket is info-1 buckets to		
		//			  the left of this index											
		InfoType* mInfo {};

		// The block that contains the values										
		// It's size and reserve also used for the keys and tombstones		
		TAny<V> mValues;

	public:
		TUnorderedMap() = default;
		TUnorderedMap(::std::initializer_list<Pair>);
		TUnorderedMap(const TUnorderedMap&);
		TUnorderedMap(TUnorderedMap&&) noexcept;

		TUnorderedMap(Disowned<TUnorderedMap>&&) noexcept;
		TUnorderedMap(Abandoned<TUnorderedMap>&&) noexcept;
		~TUnorderedMap();

		TUnorderedMap& operator = (const TUnorderedMap&);
		TUnorderedMap& operator = (TUnorderedMap&&) noexcept;

		TUnorderedMap& operator = (const Pair&);
		TUnorderedMap& operator = (Pair&&) noexcept;

	public:
		NOD() DMeta GetKeyType() const;
		NOD() DMeta GetValueType() const;

		template<class ALT_K>
		NOD() constexpr bool KeyIs() const noexcept;
		template<class ALT_V>
		NOD() constexpr bool ValueIs() const noexcept;

		NOD() constexpr bool IsKeyUntyped() const noexcept;
		NOD() constexpr bool IsValueUntyped() const noexcept;

		NOD() constexpr bool IsKeyTypeConstrained() const noexcept;
		NOD() constexpr bool IsValueTypeConstrained() const noexcept;

		NOD() constexpr bool IsKeyAbstract() const noexcept;
		NOD() constexpr bool IsValueAbstract() const noexcept;

		NOD() constexpr bool IsKeyConstructible() const noexcept;
		NOD() constexpr bool IsValueConstructible() const noexcept;

		NOD() constexpr bool IsKeyDeep() const noexcept;
		NOD() constexpr bool IsValueDeep() const noexcept;

		NOD() constexpr bool IsKeySparse() const noexcept;
		NOD() constexpr bool IsValueSparse() const noexcept;

		NOD() constexpr bool IsKeyDense() const noexcept;
		NOD() constexpr bool IsValueDense() const noexcept;

		NOD() constexpr Size GetKeyStride() const noexcept;
		NOD() constexpr Size GetValueStride() const noexcept;

		NOD() constexpr Size GetByteSize() const noexcept;
		NOD() constexpr Count GetCount() const noexcept;
		NOD() constexpr Count GetReserved() const noexcept;
		NOD() constexpr bool IsEmpty() const noexcept;
		NOD() constexpr bool IsAllocated() const noexcept;

		NOD() constexpr bool HasAuthority() const noexcept;
		NOD() constexpr Count GetUses() const noexcept;

		void Allocate(const Count&);

		NOD() TUnorderedMap Clone() const;

		TUnorderedMap& operator << (Pair&&);
		TUnorderedMap& operator << (const Pair&);

		bool operator == (const TUnorderedMap&) const;

		///																							
		///	INSERTION																			
		///																							
		Count Insert(const K&, const V&);
		Count Insert(K&&, const V&);
		Count Insert(const K&, V&&);
		Count Insert(K&&, V&&);

		///																							
		///	REMOVAL																				
		///																							
		Count RemoveKey(const K&);
		Count RemoveValue(const V&);
		Count RemovePair(const Pair&);
		Count RemoveIndex(const Index&);

		void Clear();
		void Reset();
		void Compact();

		///																							
		///	SEARCH																				
		///																							
		NOD() bool ContainsKey(const K&) const;
		NOD() bool ContainsValue(const V&) const;
		NOD() bool ContainsPair(const Pair&) const;
		NOD() Index FindKeyIndex(const K&) const;

		NOD() decltype(auto) At(const K&);
		NOD() decltype(auto) At(const K&) const;

		NOD() decltype(auto) operator[] (const K&) const;
		NOD() decltype(auto) operator[] (const K&);

		NOD() decltype(auto) GetKey(const Index&) const;
		NOD() decltype(auto) GetKey(const Index&);
		NOD() decltype(auto) GetValue(const Index&) const;
		NOD() decltype(auto) GetValue(const Index&);
		NOD() decltype(auto) GetPair(const Index&) const;
		NOD() decltype(auto) GetPair(const Index&);

		///																							
		///	ITERATION																			
		///																							
		template<bool MUTABLE>
		struct TIterator;

		using Iterator = TIterator<true>;
		using ConstIterator = TIterator<false>;
		
		NOD() Iterator begin() noexcept;
		NOD() Iterator end() noexcept;
		NOD() Iterator last() noexcept;
		NOD() ConstIterator begin() const noexcept;
		NOD() ConstIterator end() const noexcept;
		NOD() ConstIterator last() const noexcept;

		Count ForEachKeyElement(TFunctor<bool(const Block&)>&&) const;
		Count ForEachKeyElement(TFunctor<bool(Block&)>&&);
		Count ForEachKeyElement(TFunctor<void(const Block&)>&&) const;
		Count ForEachKeyElement(TFunctor<void(Block&)>&&);

		Count ForEachValueElement(TFunctor<bool(const Block&)>&&) const;
		Count ForEachValueElement(TFunctor<bool(Block&)>&&);
		Count ForEachValueElement(TFunctor<void(const Block&)>&&) const;
		Count ForEachValueElement(TFunctor<void(Block&)>&&);

		template<bool MUTABLE = true, class... F>
		Count ForEachKey(F&&...);
		template<class... F>
		Count ForEachKey(F&&...) const;
		template<bool MUTABLE = true, class... F>
		Count ForEachValue(F&&...);
		template<class... F>
		Count ForEachValue(F&&...) const;
	
		template<bool MUTABLE = true, class... F>
		Count ForEachKeyRev(F&&...);
		template<class... F>
		Count ForEachKeyRev(F&&...) const;
		template<bool MUTABLE = true, class... F>
		Count ForEachValueRev(F&&...);
		template<class... F>
		Count ForEachValueRev(F&&...) const;
	
		template<bool SKIP = true, bool MUTABLE = true, class... F>
		Count ForEachKeyDeep(F&&...);
		template<bool SKIP = true, class... F>
		Count ForEachKeyDeep(F&&...) const;
		template<bool SKIP = true, bool MUTABLE = true, class... F>
		Count ForEachValueDeep(F&&...);
		template<bool SKIP = true, class... F>
		Count ForEachValueDeep(F&&...) const;
	
		template<bool SKIP = true, bool MUTABLE = true, class... F>
		Count ForEachKeyDeepRev(F&&...);
		template<bool SKIP = true, class... F>
		Count ForEachKeyDeepRev(F&&...) const;
		template<bool SKIP = true, bool MUTABLE = true, class... F>
		Count ForEachValueDeepRev(F&&...);
		template<bool SKIP = true, class... F>
		Count ForEachValueDeepRev(F&&...) const;

	protected:
		template<bool REUSE>
		void AllocateKeys(const Count&);
		void AllocateInner(const Count&);
		void Rehash(const Count&, const Count&);
		void InsertInner(const Offset&, K&&, V&&);
		void ClearInner();

		template<class T>
		static void CloneInner(const Count&, const InfoType*, const T*, const T*, T*);

		template<class T>
		static void RemoveInner(T*) noexcept;

		template<class T>
		static void Overwrite(T&&, T&) noexcept;

		NOD() static Size RequestKeyAndInfoSize(Count, Offset&) noexcept;

		void RemoveIndex(const Offset&) noexcept;

		NOD() decltype(auto) GetKey(const Offset&) const noexcept;
		NOD() decltype(auto) GetKey(const Offset&) noexcept;
		NOD() decltype(auto) GetValue(const Offset&) const noexcept;
		NOD() decltype(auto) GetValue(const Offset&) noexcept;
		NOD() decltype(auto) GetPair(const Offset&) const noexcept;
		NOD() decltype(auto) GetPair(const Offset&) noexcept;

		NOD() Offset GetBucket(const K&) const noexcept;
		NOD() Offset FindIndex(const K&) const;

	#ifdef LANGULUS_ENABLE_TESTING
		public:
	#endif
		NOD() const InfoType* GetInfo() const noexcept;
		NOD() InfoType* GetInfo() noexcept;
		NOD() const InfoType* GetInfoEnd() const noexcept;

		NOD() constexpr auto GetRawKeys() const noexcept;
		NOD() constexpr auto GetRawKeys() noexcept;
		NOD() constexpr auto GetRawKeysEnd() const noexcept;

		NOD() constexpr auto GetRawValues() const noexcept;
		NOD() constexpr auto GetRawValues() noexcept;
		NOD() constexpr auto GetRawValuesEnd() const noexcept;
	};


	///																								
	///	Unordered map iterator																
	///																								
	template<CT::Data K, CT::Data V>
	template<bool MUTABLE>
	struct TUnorderedMap<K, V>::TIterator {
	protected:
		friend class TUnorderedMap<K, V>;

		const InfoType* mInfo {};
		const InfoType* mSentinel {};
		const KeyInner* mKey {};
		const ValueInner* mValue {};

		TIterator(const InfoType*, const InfoType*, const KeyInner*, const ValueInner*) noexcept;

	public:
		TIterator() noexcept = default;
		TIterator(const TIterator&) noexcept = default;
		TIterator(TIterator&&) noexcept = default;

		NOD() bool operator == (const TIterator&) const noexcept;

		NOD() PairRef operator * () const noexcept requires (MUTABLE);
		NOD() PairConstRef operator * () const noexcept requires (!MUTABLE);

		// Prefix operator																
		TIterator& operator ++ () noexcept;

		// Suffix operator																
		NOD() TIterator operator ++ (int) noexcept;
	};

} // namespace Langulus::Anyness

#include "TUnorderedMap.inl"
