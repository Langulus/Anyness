/// Loosely based on martinus's robin-hood-hashing project                    
/// https://github.com/martinus/robin-hood-hashing                            
///                                                                           
#pragma once
#include "Map.hpp"
#include "Iterator.hpp"
#include "inner/DataNode.hpp"
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


namespace Langulus::Anyness
{
	namespace Inner
	{

		template <typename T>
		constexpr T rotr(T x, unsigned k) {
			return (x >> k) | (x << (8U * sizeof(T) - k));
		}


	} // namespace Langulus::Anyness::Inner

	struct is_transparent_tag {};


	namespace Inner
	{

		template<class T>
		struct void_type {
			using type = void;
		};


		template<class T>
		concept has_is_transparent = requires { T::is_transparent; };


		/*template<class T>
		concept IsOnHeap = T::Method == AllocationMethod::Heap;
		template<class T>
		concept IsOnStack = T::Method == AllocationMethod::Stack;*/
		//template<class T>
		//concept IsTransparent = T::is_transparent;

		/// Type needs to be wider than uint8_t											
		using InfoType = uint32_t;

		#define TABLE_TEMPLATE() template<AllocationMethod METHOD, Count MaxLoadFactor100, class K, class V>
		#define TABLE() Table<METHOD, MaxLoadFactor100, K, V>

		TABLE_TEMPLATE()
		using TableAllocator = NodeAllocator<Conditional<CT::Void<V>, K, TPair<K, V>>, 4, 16384, METHOD>;

		/// A highly optimized hashmap implementation, using the Robin Hood		
		/// algorithm																				
		/// In most cases, this map should be usable as a drop-in replacement	
		/// for std::unordered_map, but be about 2x faster in most cases and		
		/// require much less allocations.													
		///																							
		/// This implementation uses the following memory layout:					
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
		///	stop at end() without the need for a idx variable.						
		///																							
		/// According to STL, order of templates has effect on throughput.		
		/// That's why I've moved the boolean to the front.							
		/// https://www.reddit.com/r/cpp/comments/ahp6iu/compile_time_binary_size_reductions_and_cs_future/eeguck4/
		///																							
		TABLE_TEMPLATE()
		class Table : public TableAllocator<METHOD, MaxLoadFactor100, K, V> {
		public:
			static constexpr AllocationMethod Method = METHOD;
			static constexpr bool IsMap = not CT::Void<V>;
			static constexpr bool IsSet = CT::Void<V>;
			static constexpr bool IsOnHeap = Method == AllocationMethod::Heap;
			static constexpr bool IsOnStack = Method == AllocationMethod::Stack;

			using Base = TableAllocator<METHOD, MaxLoadFactor100, K, V>;
			using Pair = Conditional<IsMap, TPair<K, V>, K>;
			using Key = K;
			using Value = V;
			using Self = TABLE();
			using Node = Conditional<
				Method == AllocationMethod::Stack,
				DataNodeOnStack<Self, Pair, V>,
				DataNodeOnHeap<Self, Pair, V>
			>;

		private:
			static_assert(MaxLoadFactor100 > 10 && MaxLoadFactor100 < 100,
				"MaxLoadFactor100 needs to be >10 && < 100");

			// members are sorted so no padding occurs
			uint64_t mHashMultiplier = UINT64_C(0xc4ceb9fe1a85ec53);                // 8 byte  8
			Node* mKeyVals = reinterpret_cast_no_cast_align_warning<Node*>(&mMask); // 8 byte 16
			uint8_t* mInfo = reinterpret_cast<uint8_t*>(&mMask);                    // 8 byte 24
			size_t mNumElements = 0;                                                // 8 byte 32
			size_t mMask = 0;                                                       // 8 byte 40
			size_t mMaxNumElementsAllowed = 0;                                      // 8 byte 48
			InfoType mInfoInc = InitialInfoInc;                                     // 4 byte 52
			InfoType mInfoHashShift = InitialInfoHashShift;                         // 4 byte 56
			// 16 byte 56 if NodeAllocator

			// Configuration defaults													
			// Make sure we have 8 elements, needed to quickly rehash mInfo
			static constexpr size_t InitialNumElements = sizeof(uint64_t);
			static constexpr uint32_t InitialInfoNumBits = 5;
			static constexpr uint8_t InitialInfoInc = 1U << InitialInfoNumBits;
			static constexpr size_t InfoMask = InitialInfoInc - 1U;
			static constexpr uint8_t InitialInfoHashShift = 0;
			using DataPool = NodeAllocator<Pair, 4, 16384, METHOD>;

		public:
			Table() noexcept;
			explicit Table(size_t) noexcept;

			template<typename Iter>
			Table(Iter, Iter, size_t = 0);
			Table(std::initializer_list<Pair>, size_t = 0);

			Table(Table&&) noexcept;
			Table(const Table&);

			Table(Disowned<Table>&&) noexcept;
			Table(Abandoned<Table>&&) noexcept;
			~Table();

			Table& operator = (Table&&) noexcept;
			Table& operator = (const Table&);

			Table& operator = (Pair&&) noexcept;
			Table& operator = (const Pair&);

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
			NOD() constexpr Size GetPairStride() const noexcept;
			NOD() constexpr Size GetKeyStride() const noexcept;
			NOD() constexpr Size GetValueStride() const noexcept;
			NOD() constexpr Size GetSize() const noexcept;
			NOD() constexpr Count GetCount() const noexcept;
			NOD() constexpr bool IsEmpty() const noexcept;
			NOD() constexpr bool IsAllocated() const noexcept;

			template<bool REHASH = false>
			void Allocate(size_t);
			void Rehash(size_t);

			Table& operator << (Pair&&);
			Table& operator << (const Pair&);

			NOD() K const& getFirstConst(Node const&) const noexcept;
			NOD() K const& getFirstConst(K const&) const noexcept;
			template <CT::Data Q = V>
			NOD() K const& getFirstConst(Pair const&) const noexcept;
			template<class HashKey>
			void keyToIdx(HashKey&&, size_t*, InfoType*) const;
			void next(InfoType*, size_t*) const noexcept;
			void nextWhileLess(InfoType*, size_t*) const noexcept;
			void shiftUp(size_t, size_t const) noexcept(std::is_nothrow_move_assignable_v<Node>);
			void shiftDown(size_t) noexcept(std::is_nothrow_move_assignable_v<Node>);

		private:
			void CloneInner(const Table&);
		public:
			NOD() Table Clone() const;

		public:
			using iterator = Iterator<false, Self>;
			using const_iterator = Iterator<true, Self>;

			bool operator == (const Table&) const;


			///																						
			///	INSERTION																		
			///																						
			using Insertion = ::std::pair<iterator, bool>;

			template<class Iter>
			void Insert(Iter, Iter);
			void Insert(::std::initializer_list<Pair>);

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

			Insertion Insert(const Pair&);
			iterator Insert(const_iterator, const Pair&);
			Insertion Insert(Pair&&);
			iterator Insert(const_iterator, Pair&&);

			void insert_move(Node&&);
		private:
			template <class OtherKey, class... Args>
			Insertion try_emplace_impl(OtherKey&&, Args&&...);
			template <class OtherKey, class Mapped>
			Insertion insertOrAssignImpl(OtherKey&&, Mapped&&);

			enum class InsertionState {
				overflow_error,
				key_found,
				new_node,
				overwrite_node
			};

			/// Finds key, and if not already present prepares a spot where to	
			/// pot the key & value. This potentially shifts nodes out of the		
			/// way, updates mInfo and number of inserted elements, so the only	
			/// operation left to do is create/assign a new node at that spot		
			template <class OtherKey>
			std::pair<size_t, InsertionState> insertKeyPrepareEmptySpot(OtherKey&&);

		public:
			///																						
			///	REMOVAL																			
			///																						
			void Clear();
			void Reset();
			iterator RemoveIndex(const_iterator);
			iterator RemoveIndex(iterator);
			size_t RemoveKey(const K&);
			size_t RemoveValue(const V&);
			void compact();
		private:
			void destroy();
			template<bool DEALLOCATE>
			void DestroyNodes() noexcept;

		public:
			///																						
			///	SEARCH																			
			///																						
			size_t count(const K&) const;
			bool contains(const K&) const;

			V& at(const K&);
			const V& at(const K&) const;

			const_iterator find(const K&) const;
			iterator find(const K&);

			template<class Other>
			NOD() size_t findIdx(const Other&) const;

			const V& operator[] (const K&) const;
			V& operator[] (const K&);
			
			NOD() bool has(const Pair& e) const {
				auto it = find(e.mKey);
				return it != end() && it->mValue == e.mValue;
			}


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
			NOD() size_t calcMaxNumElementsAllowed(size_t) const noexcept;
			NOD() size_t calcNumBytesInfo(size_t) const noexcept;
			NOD() size_t calcNumElementsWithBuffer(size_t) const noexcept;
			NOD() size_t calcNumBytesTotal(size_t) const;

		private:
			/// Reserves space for at least the specified number of elements		
			/// Only works if numBuckets is power-of-two									
			template<bool FREE>
			void rehashPowerOfTwo(size_t numBuckets) {
				Node* const oldKeyVals = mKeyVals;
				uint8_t const* const oldInfo = mInfo;
				const size_t oldMaxElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);

				// Resize operation: move stuff										
				initData(numBuckets);
				if (oldMaxElementsWithBuffer > 1) {
					for (size_t i = 0; i < oldMaxElementsWithBuffer; ++i) {
						if (oldInfo[i] != 0) {
							// Might throw an exception, which is really bad	
							// since we are in the middle of moving stuff		
							insert_move(Move(oldKeyVals[i]));
							// Destroy the node but DON'T destroy the data		
							oldKeyVals[i].~Node();
						}
					}

					// This check is not necessary as it's guarded by the		
					// previous if, but it helps silence g++'s overeager		
					// "attempt to free a non-heap object 'map'					
					// [-Werror=free-nonheap-object]" warning						
					//if (oldKeyVals != reinterpret_cast_no_cast_align_warning<Node*>(&mMask)) {
						// don't destroy old data: put it into the pool instead
						if constexpr (FREE)
							std::free(oldKeyVals);
						else
							DataPool::addOrFree(oldKeyVals, calcNumBytesTotal(oldMaxElementsWithBuffer));
					//}
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
				mMaxNumElementsAllowed = calcMaxNumElementsAllowed(max_elements);

				auto const numElementsWithBuffer = calcNumElementsWithBuffer(max_elements);

				// Malloc & zero mInfo - faster than calloc everything		
				auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);
				mKeyVals = reinterpret_cast<Node*>(assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));
				mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
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

				auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);
				for (size_t i = 0; i < numElementsWithBuffer; i += 8) {
					auto val = unaligned_load<uint64_t>(mInfo + i);
					val = (val >> 1U) & UINT64_C(0x7f7f7f7f7f7f7f7f);
					std::memcpy(mInfo + i, &val, sizeof(val));
				}

				// Update sentinel, which might have been cleared out!		
				mInfo[numElementsWithBuffer] = 1;
				mMaxNumElementsAllowed = calcMaxNumElementsAllowed(mMask + 1);
				return true;
			}

			///	@return true if resize was possible, false otherwise				
			bool increase_size() {
				// Nothing allocated yet? just allocate InitialNumElements	
				if (0 == mMask) {
					initData(InitialNumElements);
					return true;
				}

				auto const maxNumElementsAllowed = calcMaxNumElementsAllowed(mMask + 1);
				if (mNumElements < maxNumElementsAllowed && try_increase_info())
					return true;

				if (mNumElements * 2 < calcMaxNumElementsAllowed(mMask + 1)) {
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
				mKeyVals = reinterpret_cast_no_cast_align_warning<Node*>(&mMask);
				mInfo = reinterpret_cast<uint8_t*>(&mMask);
				mNumElements = 0;
				mMask = 0;
				mMaxNumElementsAllowed = 0;
				mInfoInc = InitialInfoInc;
				mInfoHashShift = InitialInfoHashShift;
			}

		};

	} // namespace Langulus::Anyness::Inner

	template<class... T>
	concept IsNoexceptMoveConstructibleOrAssignable = 
		((::std::is_nothrow_move_constructible_v<T> && ::std::is_nothrow_move_assignable_v<T>) && ...);

	/// Map																							
	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_flat_map = Inner::Table<AllocationMethod::Stack, MaxLoadFactor100, K, V>;

	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_node_map = Inner::Table<AllocationMethod::Heap, MaxLoadFactor100, K, V>;

	template <class K, class V>
	constexpr bool MapOnStackCriteria = sizeof(TPair<K, V>) <= sizeof(Count) * 6 && IsNoexceptMoveConstructibleOrAssignable<TPair<K, V>>;

	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_map = Inner::Table<MapOnStackCriteria<K, V> ? AllocationMethod::Stack : AllocationMethod::Heap, MaxLoadFactor100, K, V>;

	/// Set																							
	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_flat_set = Inner::Table<AllocationMethod::Stack, MaxLoadFactor100, K, void>;

	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_node_set = Inner::Table<AllocationMethod::Heap, MaxLoadFactor100, K, void>;

	template <class K>
	constexpr bool SetOnStackCriteria = sizeof(K) <= sizeof(Count) * 6 && IsNoexceptMoveConstructibleOrAssignable<K>;

	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_set = Inner::Table<SetOnStackCriteria<K> ? AllocationMethod::Stack : AllocationMethod::Heap, MaxLoadFactor100, K, void>;

} // namespace Langulus::Anyness

#include "THashMap.inl"

#undef TABLE_TEMPLATE
#undef TABLE
