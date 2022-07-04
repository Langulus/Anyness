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
	/// A highly optimized hashmap implementation, using the Robin Hood			
	/// algorithm																					
	///																								
	template<CT::Data K, CT::Data V>
	class THashMap {
	public:
		static_assert(CT::Comparable<K>, "Can't compare keys for map");
		using Pair = TPair<K, V>;
		using Key = K;
		using Value = V;
		using Self = THashMap<K, V>;
		static constexpr Count MinimalAllocation = 8;

	protected:
		// The allocation which holds keys and tombstones						
		Inner::Allocation* mKeys {};

		// A precomputed pointer for the info bytes								
		// Points to an offset inside mKeys allocation							
		// Each byte represents a pair, and can be three things:				
		//		0 - the index is not used, data is not initialized				
		//		1 - the index is used, and key is exactly where it should be
		//		2+ - the index is used, but bucket is info-1 buckets to		
		//			  the left of this index											
		uint8_t* mInfo {};

		// The block that contains the values										
		// It's size and reserve also used for the keys and tombstones		
		TAny<V> mValues;

	public:
		THashMap() = default;
		THashMap(::std::initializer_list<Pair>);
		THashMap(const THashMap&);
		THashMap(THashMap&&) noexcept;

		THashMap(Disowned<THashMap>&&) noexcept;
		THashMap(Abandoned<THashMap>&&) noexcept;
		~THashMap();

		THashMap& operator = (const THashMap&);
		THashMap& operator = (THashMap&&) noexcept;

		THashMap& operator = (const Pair&);
		THashMap& operator = (Pair&&) noexcept;

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

		NOD() constexpr Size GetSize() const noexcept;
		NOD() constexpr Count GetCount() const noexcept;
		NOD() constexpr Count GetReserved() const noexcept;
		NOD() constexpr bool IsEmpty() const noexcept;
		NOD() constexpr bool IsAllocated() const noexcept;

		NOD() constexpr bool HasAuthority() const noexcept;
		NOD() constexpr Count GetUses() const noexcept;

		NOD() constexpr auto GetRawKeys() const noexcept;
		NOD() constexpr auto GetRawKeys() noexcept;
		NOD() constexpr auto GetRawKeysEnd() const noexcept;

		NOD() constexpr auto GetRawValues() const noexcept;
		NOD() constexpr auto GetRawValues() noexcept;
		NOD() constexpr auto GetRawValuesEnd() const noexcept;

		void Allocate(const Count&);

		NOD() THashMap Clone() const;

		THashMap& operator << (Pair&&);
		THashMap& operator << (const Pair&);

		bool operator == (const THashMap&) const;


		///																							
		///	INSERTION																			
		///																							
		Count Insert(const Pair&);
		Count Insert(Pair&&);


		///																							
		///	REMOVAL																				
		///																							
		Count RemoveKey(const K&);
		Count RemoveValue(const V&);
		Count RemovePair(const Pair&);

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

		NOD() const K& GetKey(const Index&) const;
		NOD() K& GetKey(const Index&);
		NOD() decltype(auto) GetValue(const Index&) const;
		NOD() decltype(auto) GetValue(const Index&);
		NOD() decltype(auto) GetPair(const Index&) const;
		NOD() decltype(auto) GetPair(const Index&);

	protected:
		template<bool REUSE>
		void AllocateKeys(const Count&);
		void AllocateInner(const Count&);
		void Rehash(const Count&, const Count&);
		void InsertInner(const Offset&, K&, V&);
		void ClearInner();

		template<class T>
		static void CloneInner(const Count&, const uint8_t*, const T*, const T*, T*);

		template<class T>
		static void RemoveInner(T*) noexcept;
		template<class T>
		static void Overwrite(T&&, T&) noexcept;

		NOD() static Size RequestKeyAndInfoSize(Count, Offset&) noexcept;

		void RemoveIndex(const Offset&) noexcept;

		NOD() const K& GetKey(const Offset&) const noexcept;
		NOD() K& GetKey(const Offset&) noexcept;
		NOD() decltype(auto) GetValue(const Offset&) const noexcept;
		NOD() decltype(auto) GetValue(const Offset&) noexcept;
		NOD() decltype(auto) GetPair(const Offset&) const noexcept;
		NOD() decltype(auto) GetPair(const Offset&) noexcept;
		NOD() Offset GetBucket(const K&) const noexcept;
		NOD() Offset FindIndex(const K&) const;
		NOD() const uint8_t* GetInfo() const noexcept;
		NOD() uint8_t* GetInfo() noexcept;
		NOD() const uint8_t* GetInfoEnd() const noexcept;
	};

} // namespace Langulus::Anyness

#include "THashMap.inl"
