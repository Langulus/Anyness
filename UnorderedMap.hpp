///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Any.hpp"
#include "TPair.hpp"

namespace Langulus::Anyness
{

	///																								
	///	Type-erased unordered map															
	///																								
	class UnorderedMap {
		using Allocator = Inner::Allocator;
		static constexpr Count MinimalAllocation = 8;
	protected:
		using InfoType = uint8_t;

		// A precomputed pointer for the info bytes								
		// Points to an offset inside mKeys allocation							
		// Each byte represents a pair, and can be three things:				
		//		0 - the index is not used, data is not initialized				
		//		1 - the index is used, and key is exactly where it should be
		//		2+ - the index is used, but bucket is info-1 buckets to		
		//			  the right of this index											
		InfoType* mInfo {};

		// The block that contains the keys and info bytes						
		Any mKeys;

		// The block that contains the values										
		// It's size and reserve also used for the keys and tombstones		
		// The redundant data inside mKeys is required for binary			
		// compatibility with the type-erased equivalents						
		Any mValues;

	public:
		static constexpr bool Ordered = false;

		UnorderedMap() = default;
		template<CT::Data K, CT::Data V>
		UnorderedMap(::std::initializer_list<TPair<K, V>>);
		UnorderedMap(const UnorderedMap&);
		UnorderedMap(UnorderedMap&&) noexcept;

		UnorderedMap(Disowned<UnorderedMap>&&) noexcept;
		UnorderedMap(Abandoned<UnorderedMap>&&) noexcept;
		~UnorderedMap();

		UnorderedMap& operator = (const UnorderedMap&);
		UnorderedMap& operator = (UnorderedMap&&) noexcept;

		UnorderedMap& operator = (const Pair&);
		UnorderedMap& operator = (Pair&&) noexcept;

		template<CT::Data K, CT::Data V>
		UnorderedMap& operator = (const TPair<K, V>&);
		template<CT::Data K, CT::Data V>
		UnorderedMap& operator = (TPair<K, V>&&) noexcept;

	public:
		NOD() DMeta GetKeyType() const noexcept;
		NOD() DMeta GetValueType() const noexcept;

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

		template<CT::Data K, CT::Data V>
		void Mutate();
		void Mutate(DMeta, bool, DMeta, bool);
		void Allocate(const Count&);

		NOD() UnorderedMap Clone() const;

		bool operator == (const UnorderedMap&) const;

		///																							
		///	INSERTION																			
		///																							
		template<CT::Data K, CT::Data V>
		Count Insert(const K&, const V&);
		template<CT::Data K, CT::Data V>
		Count Insert(K&&, const V&);
		template<CT::Data K, CT::Data V>
		Count Insert(const K&, V&&);
		template<CT::Data K, CT::Data V>
		Count Insert(K&&, V&&);

		template<CT::Data K, CT::Data V>
		UnorderedMap& operator << (const TPair<K, V>&);
		template<CT::Data K, CT::Data V>
		UnorderedMap& operator << (TPair<K, V>&&);

		UnorderedMap& operator << (const Pair&);
		UnorderedMap& operator << (Pair&&);

		///																							
		///	REMOVAL																				
		///																							
		template<CT::Data K>
		Count RemoveKey(const K&);
		template<CT::Data V>
		Count RemoveValue(const V&);
		template<CT::Data K, CT::Data V>
		Count RemovePair(const TPair<K, V>&);
		Count RemoveIndex(const Index&);

		void Clear();
		void Reset();
		void Compact();

		///																							
		///	SEARCH																				
		///																							
		template<CT::Data K>
		NOD() bool ContainsKey(const K&) const;
		template<CT::Data V>
		NOD() bool ContainsValue(const V&) const;
		template<CT::Data K, CT::Data V>
		NOD() bool ContainsPair(const TPair<K, V>&) const;
		template<CT::Data K>
		NOD() Index FindKeyIndex(const K&) const;

		template<CT::Data K>
		NOD() decltype(auto) At(const K&);
		template<CT::Data K>
		NOD() decltype(auto) At(const K&) const;

		template<CT::Data K>
		NOD() Any operator[] (const K&) const;
		template<CT::Data K>
		NOD() Any operator[] (const K&);

		NOD() Block GetKey(const Index&) const;
		NOD() Block GetKey(const Index&);
		NOD() Block GetValue(const Index&) const;
		NOD() Block GetValue(const Index&);
		NOD() Pair GetPair(const Index&) const;
		NOD() Pair GetPair(const Index&);

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

		Count InsertUnknown(const Block&, const Block&);
		Count InsertUnknown(Block&&, Block&&);

		template<bool CHECK_FOR_MATCH>
		void InsertInnerUnknown(const Offset&, Block&&, Block&&);
		template<bool CHECK_FOR_MATCH, CT::Data K, CT::Data V>
		void InsertInner(const Offset&, K&&, V&&);

		void ClearInner();

		template<class T>
		static void CloneInner(const Count&, const InfoType*, const T*, const T*, T*);

		NOD() Size RequestKeyAndInfoSize(Count, Offset&) noexcept;

		void RemoveIndex(const Offset&) noexcept;

		NOD() Block GetKey(const Offset&) const noexcept;
		NOD() Block GetKey(const Offset&) noexcept;
		NOD() Block GetValue(const Offset&) const noexcept;
		NOD() Block GetValue(const Offset&) noexcept;
		NOD() Pair GetPair(const Offset&) const noexcept;
		NOD() Pair GetPair(const Offset&) noexcept;

		template<CT::Data K>
		NOD() Offset GetBucket(const K&) const noexcept;
		template<CT::Data K>
		NOD() Offset FindIndex(const K&) const;

	#ifdef LANGULUS_ENABLE_TESTING
		public:
	#endif
		NOD() const InfoType* GetInfo() const noexcept;
		NOD() InfoType* GetInfo() noexcept;
		NOD() const InfoType* GetInfoEnd() const noexcept;

		template<CT::Data K>
		NOD() constexpr decltype(auto) GetRawKeys() const noexcept;
		template<CT::Data K>
		NOD() constexpr decltype(auto) GetRawKeys() noexcept;
		template<CT::Data K>
		NOD() constexpr decltype(auto) GetRawKeysEnd() const noexcept;

		template<CT::Data V>
		NOD() constexpr decltype(auto) GetRawValues() const noexcept;
		template<CT::Data V>
		NOD() constexpr decltype(auto) GetRawValues() noexcept;
		template<CT::Data V>
		NOD() constexpr decltype(auto) GetRawValuesEnd() const noexcept;

	#ifdef LANGULUS_ENABLE_TESTING
		NOD() constexpr const void* GetRawKeysMemory() const noexcept;
		NOD() constexpr const void* GetRawValuesMemory() const noexcept;
	#endif
	};

	///																								
	///	Unordered map iterator																
	///																								
	template<bool MUTABLE>
	struct UnorderedMap::TIterator {
	protected:
		friend class UnorderedMap;

		const InfoType* mInfo {};
		const InfoType* mSentinel {};
		Block mKey;
		Block mValue;

		TIterator(const InfoType*, const InfoType*, const Block&, const Block&) noexcept;

	public:
		TIterator() noexcept = default;
		TIterator(const TIterator&) noexcept = default;
		TIterator(TIterator&&) noexcept = default;

		NOD() bool operator == (const TIterator&) const noexcept;

		NOD() Pair operator * () const noexcept;

		// Prefix operator																
		TIterator& operator ++ () noexcept;

		// Suffix operator																
		NOD() TIterator operator ++ (int) noexcept;
	};

} // namespace Langulus::Anyness

#include "UnorderedMap.inl"
