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

#define TABLE_TEMPLATE() template<AllocationMethod METHOD, Count MaxLoadFactor100, class K, class V>
#define TABLE() Table<METHOD, MaxLoadFactor100, K, V>

namespace Langulus::Anyness
{

	namespace Inner
	{

		/// Type needs to be wider than uint8_t											
		using InfoType = uint32_t;

		//TABLE_TEMPLATE()
		//using TableAllocator = NodeAllocator<Conditional<CT::Void<V>, K, TPair<K, V>>, 4, 16384, METHOD>;

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
		class Table/* : public TableAllocator<METHOD, MaxLoadFactor100, K, V> */{
		public:
			static_assert(CT::Comparable<K>, "Can't compare keys for map");

			static constexpr AllocationMethod Method = METHOD;
			static constexpr bool IsMap = not CT::Void<V>;
			static constexpr bool IsSet = CT::Void<V>;
			static constexpr bool IsOnHeap = Method == AllocationMethod::Heap;
			static constexpr bool IsOnStack = Method == AllocationMethod::Stack;

			//using Base = TableAllocator<METHOD, MaxLoadFactor100, K, V>;
			using Type = Conditional<IsMap, TPair<K, V>, K>;
			using Key = K;
			using Value = V;
			using Self = TABLE();
			using Node = Conditional<Method == AllocationMethod::Stack, Type, Type*>;

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
			Table(std::initializer_list<Pair>);

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
			NOD() constexpr bool KeyIs() const noexcept requires IsMap;
			template<class ALT_V>
			NOD() constexpr bool ValueIs() const noexcept requires IsMap;
			template<class ALT_T>
			NOD() constexpr bool Is() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyUntyped() const noexcept requires IsMap;
			NOD() constexpr bool IsValueUntyped() const noexcept requires IsMap;
			NOD() constexpr bool IsUntyped() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyTypeConstrained() const noexcept requires IsMap;
			NOD() constexpr bool IsValueTypeConstrained() const noexcept requires IsMap;
			NOD() constexpr bool IsTypeConstrained() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyAbstract() const noexcept requires IsMap;
			NOD() constexpr bool IsValueAbstract() const noexcept requires IsMap;
			NOD() constexpr bool IsAbstract() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyConstructible() const noexcept requires IsMap;
			NOD() constexpr bool IsValueConstructible() const noexcept requires IsMap;
			NOD() constexpr bool IsConstructible() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyDeep() const noexcept requires IsMap;
			NOD() constexpr bool IsValueDeep() const noexcept requires IsMap;
			NOD() constexpr bool IsDeep() const noexcept requires IsSet;

			NOD() constexpr bool IsKeySparse() const noexcept requires IsMap;
			NOD() constexpr bool IsValueSparse() const noexcept requires IsMap;
			NOD() constexpr bool IsSparse() const noexcept requires IsSet;

			NOD() constexpr bool IsKeyDense() const noexcept requires IsMap;
			NOD() constexpr bool IsValueDense() const noexcept requires IsMap;
			NOD() constexpr bool IsDense() const noexcept requires IsSet;

			NOD() constexpr Size GetPairStride() const noexcept requires IsMap;
			NOD() constexpr Size GetKeyStride() const noexcept requires IsMap;
			NOD() constexpr Size GetValueStride() const noexcept requires IsMap;
			NOD() constexpr Size GetStride() const noexcept requires IsSet;

			NOD() constexpr Size GetSize() const noexcept;
			NOD() constexpr Count GetCount() const noexcept;
			NOD() constexpr bool IsEmpty() const noexcept;
			NOD() constexpr bool IsAllocated() const noexcept;

			template<bool REHASH = false>
			void Allocate(size_t);
			void Rehash(size_t);

			Table& operator << (Type&&);
			Table& operator << (const Type&);

		private:
			/*NOD() K const& getFirstConst(Node const&) const noexcept;
			NOD() K const& getFirstConst(K const&) const noexcept;
			template <CT::Data Q = V>
			NOD() K const& getFirstConst(Type const&) const noexcept;*/
			template<class HashKey>
			void keyToIdx(HashKey&&, size_t*, InfoType*) const;
			void next(InfoType*, size_t*) const noexcept;
			void nextWhileLess(InfoType*, size_t*) const noexcept;
			void shiftUp(size_t, size_t const) noexcept(CT::MovableNoexcept<Node>);
			void shiftDown(size_t) noexcept(CT::MovableNoexcept<Node>);
			void CloneInner(const Table&);

		public:
			NOD() Table Clone() const;

		public:
			using iterator = Iterator<false, Table>;
			using const_iterator = Iterator<true, Table>;

			bool operator == (const Table&) const;


			///																						
			///	INSERTION																		
			///																						
			using Insertion = ::std::pair<iterator, bool>;

			template<class Iter>
			void Insert(Iter, Iter);
			void Insert(::std::initializer_list<Type>);

			template<class... Args>
			Insertion Emplace(Args&&...) requires IsMap;

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

			void insert_move(Type&&);
		private:
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

		public:
			///																						
			///	REMOVAL																			
			///																						
			void Clear();
			void Reset();
			iterator RemoveIndex(const_iterator);
			iterator RemoveIndex(iterator);
			Count RemoveKey(const K&) requires IsMap;
			Count RemoveValue(const V&) requires IsMap;
			Count RemovePair(const Type&) requires IsMap;
			Count Remove(const Type&) requires IsSet;
			void compact();
		private:
			void destroy();
			template<bool DEALLOCATE>
			void DestroyNodes() noexcept;

		public:
			///																						
			///	SEARCH																			
			///																						
			NOD() bool ContainsKey(const K&) const requires IsMap;
			NOD() bool ContainsValue(const V&) const requires IsMap;
			NOD() bool ContainsPair(const Type&) const requires IsMap;
			NOD() bool Contains(const Type&) const requires IsSet;

			NOD() const K& GetKey(const Offset&) const noexcept requires IsMap;
			NOD() K& GetKey(const Offset&) noexcept requires IsMap;
			NOD() const V& GetValue(const Offset&) const noexcept requires IsMap;
			NOD() V& GetValue(const Offset&) noexcept requires IsMap;
			NOD() const Type& GetPair(const Offset&) const noexcept requires IsMap;
			NOD() Type& GetPair(const Offset&) noexcept requires IsMap;

			NOD() V& At(const K&) requires IsMap;
			NOD() const V& At(const K&) const requires IsMap;

			NOD() const_iterator Find(const K&) const;
			NOD() iterator Find(const K&);

			NOD() Offset FindIndex(const K&) const;

			NOD() const V& operator[] (const K&) const requires IsMap;
			NOD() V& operator[] (const K&) requires IsMap;


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
			NOD() static Count GetMaxElementsAllowed(Count) noexcept;
			NOD() static Count GetBytesInfo(Count) noexcept;
			NOD() static Count GetElementsWithBuffer(Count) noexcept;
			NOD() static Count GetBytesTotal(Count);

			/// Reserves space for at least the specified number of elements		
			/// Only works if numBuckets is power-of-two									
			///	@param numBuckets - the number of buckets								
			template<bool FREE>
			void rehashPowerOfTwo(size_t numBuckets) {
				// These will be reset via initData, so back them up			
				auto const oldEntry = mEntry;
				auto const oldNodes = mNodes;
				auto const oldInfo = mInfo;
				auto const oldMaxElementsWithBuffer = GetElementsWithBuffer(mMask + 1);

				// Resize and move stuff												
				initData(numBuckets);

				if (oldMaxElementsWithBuffer > 1) {
					for (size_t i = 0; i < oldMaxElementsWithBuffer; ++i) {
						if (oldInfo[i] == 0)
							continue;

						// Might throw an exception, which is really bad		
						// since we are in the middle of moving stuff			
						if constexpr (CT::Sparse<Node>) {
							insert_move(Move(*oldNodes[i]));
							if constexpr (CT::Destroyable<Type>)
								oldNodes[i]->~Type();
						}
						else {
							insert_move(Move(oldNodes[i]));
							if constexpr (CT::Destroyable<Type>)
								oldNodes[i].~Type();
						}
					}

					// Don't destroy old data: put it into the pool instead	
					//if constexpr (FREE)
						oldEntry->Free<true>();
					//else
					//	Base::AddOrFree(reinterpret_cast<Byte*>(oldNodes), GetBytesTotal(oldMaxElementsWithBuffer));
				}
			}

			LANGULUS(NOINLINE) void throwOverflowError() const {
				throw std::overflow_error("robin_hood::map overflow");
			}

			/// Initialize container and reserve data										
			///	@param max_elements - number of elements to reserve				
			void initData(size_t max_elements) {
				mNumElements = 0;
				mMask = max_elements - 1;
				mMaxNumElementsAllowed = GetMaxElementsAllowed(max_elements);

				// Malloc & zero mInfo - faster than calloc everything		
				auto const numElementsWithBuffer = GetElementsWithBuffer(max_elements);
				auto const numBytesTotal = GetBytesTotal(numElementsWithBuffer);
				mEntry = Allocator::Allocate(numBytesTotal);
				mNodes = mEntry->As<Node>();
				mInfo = reinterpret_cast<uint8_t*>(mNodes + numElementsWithBuffer);
				std::memset(mInfo, 0, numBytesTotal - numElementsWithBuffer * sizeof(Node));

				// Set sentinel															
				mInfo[numElementsWithBuffer] = 1;
				mInfoInc = InitialInfoInc;
				mInfoHashShift = InitialInfoHashShift;
			}

			bool try_increase_info() {
				if (mInfoInc <= 2) {
					// Need to be > 2 so that shift works (otherwise			
					// undefined behavior!)												
					return false;
				}

				// We got space left, try to make info smaller					
				mInfoInc = static_cast<uint8_t>(mInfoInc >> 1U);

				// Remove one bit of the hash, leaving more space for the	
				// distance info. This is extremely fast because we can		
				// operate on 8 bytes at once											
				++mInfoHashShift;

				auto const numElementsWithBuffer = GetElementsWithBuffer(mMask + 1);
				for (size_t i = 0; i < numElementsWithBuffer; i += 8) {
					auto val = unaligned_load<uint64_t>(mInfo + i);
					val = (val >> 1U) & UINT64_C(0x7f7f7f7f7f7f7f7f);
					std::memcpy(mInfo + i, &val, sizeof(val));
				}

				// Update sentinel, which might have been cleared out!		
				mInfo[numElementsWithBuffer] = 1;
				mMaxNumElementsAllowed = GetMaxElementsAllowed(mMask + 1);
				return true;
			}

			/// Increase the contained size													
			///	@return true if resize was possible, false otherwise				
			bool increase_size() {
				// Nothing allocated yet? just allocate InitialNumElements	
				if (0 == mMask) {
					initData(InitialNumElements);
					return true;
				}

				auto const maxNumElementsAllowed = GetMaxElementsAllowed(mMask + 1);
				if (mNumElements < maxNumElementsAllowed && try_increase_info())
					return true;

				if (mNumElements * 2 < GetMaxElementsAllowed(mMask + 1)) {
					// We have to resize, even though there would still be	
					// plenty of space left! Try to rehash instead. Delete	
					// freed memory so we don't steadyily increase mem in		
					// case we have to rehash a few times							
					nextHashMultiplier();
					rehashPowerOfTwo<true>(mMask + 1);
				}
				else {
					// We've reached the capacity of the map, so the hash		
					// seems to work nice. Keep using it							
					rehashPowerOfTwo<false>((mMask + 1) * 2);
				}

				return true;
			}

			void nextHashMultiplier() {
				// Adding an *even* number, so that the multiplier will		
				// always stay odd. This is necessary so that the hash		
				// stays a mixing function (and thus doesn't have any			
				// information loss)														
				mHashMultiplier += UINT64_C(0xc4ceb9fe1a85ec54);
			}

			void init() noexcept {
				mEntry = nullptr;
				mNodes = reinterpret_cast<Node*>(&mMask);
				mInfo = reinterpret_cast<uint8_t*>(&mMask);
				mNumElements = 0;
				mMask = 0;
				mMaxNumElementsAllowed = 0;
				mInfoInc = InitialInfoInc;
				mInfoHashShift = InitialInfoHashShift;
			}

		};

	} // namespace Langulus::Anyness::Inner


	/// Map																							
	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_flat_map = Inner::Table<AllocationMethod::Stack, MaxLoadFactor100, K, V>;

	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_node_map = Inner::Table<AllocationMethod::Heap, MaxLoadFactor100, K, V>;

	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_map = Inner::Table<CT::OnStackCriteria<TPair<K, V>> ? AllocationMethod::Stack : AllocationMethod::Heap, MaxLoadFactor100, K, V>;

	/// Set																							
	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_flat_set = Inner::Table<AllocationMethod::Stack, MaxLoadFactor100, K, void>;

	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_node_set = Inner::Table<AllocationMethod::Heap, MaxLoadFactor100, K, void>;

	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_set = Inner::Table<CT::OnStackCriteria<K> ? AllocationMethod::Stack : AllocationMethod::Heap, MaxLoadFactor100, K, void>;

} // namespace Langulus::Anyness

#include "THashMap.inl"

#undef TABLE_TEMPLATE
#undef TABLE
