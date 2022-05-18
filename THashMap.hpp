///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Map.hpp"
#include "Iterator.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <memory> // only to support hash of smart pointers
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <string_view>

#define TABLE_TEMPLATE() template<bool DENSE, Count MaxLoadFactor100, CT::Data K, class V>
#define TABLE() Table<DENSE, MaxLoadFactor100, K, V>

namespace Langulus::Anyness
{

	namespace Inner
	{

		/// Type needs to be wider than uint8_t											
		using InfoType = uint32_t;

		///                                                                     
		/// Loosely based on martinus's robin-hood-hashing project              
		/// Their code is designed to be drop-in replacement for						
		/// std::unordered_map. Our code is designed to use the Langulus			
		/// nomenclature and is a completely different animal. For example,		
		/// our table doesn't get cloned when copied, but gets referenced			
		/// instead, as well as many other changes that make it consistent for	
		/// use with Anyness, and Langulus as a whole.									
		/// Our code has also been ported to C++20 and uses concepts, instead	
		/// of the SFINAE patterns and partial template specializations.			
		///                                                                     
		/// Either way, credit where credit's due:										
		/// https://github.com/martinus/robin-hood-hashing                      
		///                                                                     
		///                                                                     
		/// A highly optimized hashmap implementation, using the Robin Hood		
		/// algorithm. The implementation uses the following memory layout:		
		///																							
		/// [Node, Node, ... Node | info, info, ... infoSentinel ]					
		///																							
		/// * Node: either a DataNode that directly has the std::pair<key, val>	
		///   as member, or a DataNode with a pointer to std::pair<key,val>.		
		///   Which DataNode representation to use depends on how fast the		
		///   swap() operation is. Heuristically, this is automatically choosen	
		/// 	based on sizeof(). there are always 2^n Nodes.							
		///																							
		/// * info: Each Node in the map has a corresponding info byte, so		
		///   there are 2^n info bytes. Each byte is initialized to 0, meaning	
		///   the corresponding Node is empty. Set to 1 means the corresponding	
		///   node contains data. Set to 2 means the corresponding Node is		
		///	filled, but it actually belongs to the previous position and was	
		///   pushed out because that place is already taken.							
		///																							
		/// * infoSentinel: Sentinel byte set to 1, so that iterator's ++ can	
		///	stop at end() without the need for an idx variable.					
		///																							
		TABLE_TEMPLATE()
		class Table {
		public:
			static_assert(CT::Comparable<K>, "Can't compare keys for map");

			static constexpr bool IsMap = not CT::Void<V>;
			static constexpr bool IsSet = CT::Void<V>;
			static constexpr bool IsOnHeap = !DENSE;
			static constexpr bool IsOnStack = DENSE;

			//using Base = TableAllocator<METHOD, MaxLoadFactor100, K, V>;
			using Type = Conditional<IsMap, TPair<K, V>, K>;
			using Key = K;
			using Value = V;
			using Self = TABLE();
			using Node = Conditional<DENSE, Type, Ptr<Type>>;

		private:
			static_assert(MaxLoadFactor100 > 10 && MaxLoadFactor100 < 100,
				"MaxLoadFactor100 needs to be >10 && < 100");

			// Members are sorted so no padding occurs							
			uint64_t mHashMultiplier = UINT64_C(0xc4ceb9fe1a85ec53);                // 8 byte  8
			Entry* mEntry = nullptr;																// 8 byte 16

			// Pointer to the first pair, usually wrapped inside a Node		
			// Depending on the size of the pair, that node can be either	
			// on stack, or on heap. Which node representation to use		
			// depends on how fast the swap() operation is. Heuristically, 
			// this is automatically choosen based on sizeof(). There are	
			// always 2^n pairs.															
			// Initially, this pointer always reinterprets the mMask 		
			union {																						// 8 byte 24
				Node* mNodes;
				Byte* mNodeBytes;
			};

			// Each pair in the map has a corresponding info byte, so		
			// there are 2^n info bytes. Each byte is initialized to 0,		
			// meaning the corresponding spot is empty. Set to 1 means the 
			// corresponding spot contains data. Set to 2 means the			
			// corresponding Node is filled, but it actually belongs to		
			// the previous position and was	pushed out because that place	
			// is already taken.															
			// Initially, this pointer always reinterprets the mMask			
			// The start of the info array coincides with the end of the	
			// pair array, since it is always allocated at the back of it	
			union {																						// 8 byte 32
				uint8_t* mInfo;
				Node* mNodesEnd;
			};

			Count mNumElements = 0;																	// 8 byte 40
			Count mMask = 0;																			// 8 byte 48
			Count mMaxNumElementsAllowed = 0;													// 8 byte 56
			InfoType mInfoInc = InitialInfoInc;                                     // 4 byte 60
			InfoType mInfoHashShift = InitialInfoHashShift;                         // 4 byte 64

			// Configuration defaults													
			// Make sure we have 8 elements, needed to quickly rehash mInfo
			static constexpr Count InitialNumElements = sizeof(uint64_t);
			static constexpr uint32_t InitialInfoNumBits = 5;
			static constexpr uint8_t InitialInfoInc = 1U << InitialInfoNumBits;
			static constexpr Count InfoMask = InitialInfoInc - 1U;
			static constexpr uint8_t InitialInfoHashShift = 0;

		public:
			constexpr Table() noexcept;

			template<typename Iter>
			Table(Iter, Iter);
			Table(::std::initializer_list<Pair>);

			Table(Table&&) noexcept;
			Table(const Table&);

			Table(Disowned<Table>&&) noexcept;
			Table(Abandoned<Table>&&) noexcept;
			~Table();

			Table& operator = (Table&&) noexcept;
			Table& operator = (const Table&);

			Table& operator = (Type&&) noexcept;
			Table& operator = (const Type&);

			NOD() DMeta GetKeyType() const;
			NOD() DMeta GetValueType() const;

			template<class ALT_K>
			NOD() constexpr bool KeyIs() const noexcept;
			template<class ALT_V>
			NOD() constexpr bool ValueIs() const noexcept;
			//template<class ALT_T>
			//NOD() constexpr bool Is() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyUntyped() const noexcept;
			NOD() constexpr bool IsValueUntyped() const noexcept;
			//NOD() constexpr bool IsUntyped() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyTypeConstrained() const noexcept;
			NOD() constexpr bool IsValueTypeConstrained() const noexcept;
			//NOD() constexpr bool IsTypeConstrained() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyAbstract() const noexcept;
			NOD() constexpr bool IsValueAbstract() const noexcept;
			//NOD() constexpr bool IsAbstract() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyConstructible() const noexcept;
			NOD() constexpr bool IsValueConstructible() const noexcept;
			//NOD() constexpr bool IsConstructible() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyDeep() const noexcept;
			NOD() constexpr bool IsValueDeep() const noexcept;
			//NOD() constexpr bool IsDeep() const noexcept requires IsSet;

			NOD() constexpr bool IsKeySparse() const noexcept;
			NOD() constexpr bool IsValueSparse() const noexcept;
			//NOD() constexpr bool IsSparse() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyDense() const noexcept;
			NOD() constexpr bool IsValueDense() const noexcept;
			//NOD() constexpr bool IsDense() const noexcept requires IsSet;

			NOD() constexpr Size GetPairStride() const noexcept;
			NOD() constexpr Size GetKeyStride() const noexcept;
			NOD() constexpr Size GetValueStride() const noexcept;
			//NOD() constexpr Size GetStride() const noexcept requires IsSet;

			NOD() constexpr Node* GetRaw() const noexcept;
			NOD() constexpr Size GetSize() const noexcept;
			NOD() constexpr Count GetCount() const noexcept;
			NOD() constexpr bool IsEmpty() const noexcept;
			NOD() constexpr bool IsAllocated() const noexcept;

			NOD() bool HasAuthority() const noexcept;
			NOD() Count GetUses() const noexcept;

			template<bool REHASH = false>
			void Allocate(size_t);
			void Rehash(size_t);
			NOD() Table Clone() const;

			Table& operator << (Type&&);
			Table& operator << (const Type&);

			bool operator == (const Table&) const;


			///																						
			///	INSERTION																		
			///																						
			using iterator = Iterator<false, Table>;
			using const_iterator = Iterator<true, Table>;
			using Insertion = ::std::pair<iterator, bool>;

			template<class Iter>
			void Insert(Iter, Iter);
			void Insert(::std::initializer_list<Type>);

			template<class... Args>
			Insertion Emplace(Args&&...);

			template <class... Args>
			iterator emplace_hint(const_iterator, Args&&...);

			template <class... Args>
			Insertion try_emplace(const K&, Args&&...);
			template <class... Args>
			Insertion try_emplace(K&&, Args&&...);
			template <class... Args>
			iterator try_emplace(const_iterator, const K&, Args&&...);
			template <class... Args>
			iterator try_emplace(const_iterator, K&&, Args&&...);

			template <class Mapped>
			Insertion insert_or_assign(const K&, Mapped&&);
			template <class Mapped>
			Insertion insert_or_assign(K&&, Mapped&&);
			template <class Mapped>
			iterator insert_or_assign(const_iterator, const K&, Mapped&&);
			template <class Mapped>
			iterator insert_or_assign(const_iterator, K&&, Mapped&&);

			Insertion Insert(const Type&);
			iterator Insert(const_iterator, const Type&);
			Insertion Insert(Type&&);
			iterator Insert(const_iterator, Type&&);


			///																						
			///	REMOVAL																			
			///																						
			void Clear();
			void Reset();
			iterator RemoveIndex(const_iterator);
			iterator RemoveIndex(iterator);
			Count RemoveKey(const K&);
			Count RemoveValue(const V&);
			Count RemovePair(const Type&);
			//Count Remove(const Type&) requires IsSet;
			void compact();


			///																						
			///	SEARCH																			
			///																						
			NOD() bool ContainsKey(const K&) const;
			NOD() bool ContainsValue(const V&) const;
			NOD() bool ContainsPair(const Type&) const;
			//NOD() bool Contains(const Type&) const requires IsSet;

			NOD() const K& GetKey(const Offset&) const noexcept;
			NOD() K& GetKey(const Offset&) noexcept;
			NOD() const V& GetValue(const Offset&) const noexcept;
			NOD() V& GetValue(const Offset&) noexcept;
			NOD() const Type& GetPair(const Offset&) const noexcept;
			NOD() Type& GetPair(const Offset&) noexcept;

			NOD() V& At(const K&);
			NOD() const V& At(const K&) const;

			NOD() const_iterator Find(const K&) const;
			NOD() iterator Find(const K&);

			NOD() Offset FindIndex(const K&) const;

			NOD() const V& operator[] (const K&) const;
			NOD() V& operator[] (const K&);


			///																						
			///	ITERATION																		
			///																						
			iterator begin();
			const_iterator begin() const;
			const_iterator cbegin() const;
			iterator end();
			const_iterator end() const;
			const_iterator cend() const;
			NOD() constexpr Count max_size() const noexcept;
			NOD() constexpr float max_load_factor() const noexcept;
			NOD() constexpr float load_factor() const noexcept;
			NOD() size_t mask() const noexcept;

		private:
			template<class HashKey>
			void keyToIdx(HashKey&&, size_t*, InfoType*) const;
			void next(InfoType*, size_t*) const noexcept;
			void nextWhileLess(InfoType*, size_t*) const noexcept;
			void shiftUp(size_t, size_t const) noexcept(CT::MovableNoexcept<Node>);
			void shiftDown(size_t) noexcept(CT::MovableNoexcept<Node>);
			void CloneInner(const Table&);
			void destroy();
			void DestroyNodes() noexcept;
			NOD() const Node& GetNode(const Offset&) const noexcept;
			NOD() Node& GetNode(const Offset&) noexcept;
			NOD() static Count GetMaxElementsAllowed(Count) noexcept;
			NOD() static Count GetBytesInfo(Count) noexcept;
			NOD() static Count GetElementsWithBuffer(Count) noexcept;
			NOD() static Count GetBytesTotal(Count);
			void rehashPowerOfTwo(size_t numBuckets);
			void initData(size_t maxElements);
			bool try_increase_info();
			bool increase_size();
			void nextHashMultiplier();
			void init() noexcept;
			void MoveInsertNode(Node&&);
			template <class... Args>
			Insertion try_emplace_impl(K&&, Args&&...);
			template <class Mapped>
			Insertion insertOrAssignImpl(K&&, Mapped&&);

			enum class InsertionState {
				overflow_error,
				key_found,
				new_node,
				overwrite_node
			};

			struct EmptySpot {
				Offset mOffset;
				InsertionState mState;
			};

			EmptySpot InsertKeyAndPrepareEmptySpot(K&&);
		};

	} // namespace Langulus::Anyness::Inner


	/// Map																							
	template <CT::Data K, CT::Data V, Count MaxLoadFactor100 = 80>
	using THashDenseMap = Inner::Table<true, MaxLoadFactor100, K, V>;

	template <CT::Data K, CT::Data V, Count MaxLoadFactor100 = 80>
	using THashSparseMap = Inner::Table<false, MaxLoadFactor100, K, V>;

	template <CT::Data K, CT::Data V, Count MaxLoadFactor100 = 80>
	using THashMap = Inner::Table<CT::OnStackCriteria<TPair<K, V>>, MaxLoadFactor100, K, V>;

} // namespace Langulus::Anyness

#include "THashMap.inl"

#undef TABLE_TEMPLATE
#undef TABLE
