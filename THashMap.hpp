/// Loosely based on martinus's robin-hood-hashing project                    
/// https://github.com/martinus/robin-hood-hashing                            
///                                                                           
#pragma once
#include "Map.hpp"
#include "Iterator.hpp"
#include "Inner/DataNode.hpp"
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


	inline size_t hash_bytes(void const* ptr, size_t len) noexcept {
		static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
		static constexpr uint64_t seed = UINT64_C(0xe17a1465);
		static constexpr unsigned int r = 47;

		auto const* const data64 = static_cast<uint64_t const*>(ptr);
		uint64_t h = seed ^ (len * m);

		size_t const n_blocks = len / 8;
		for (size_t i = 0; i < n_blocks; ++i) {
			auto k = unaligned_load<uint64_t>(data64 + i);

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		auto const* const data8 = reinterpret_cast<uint8_t const*>(data64 + n_blocks);
		switch (len & 7U) {
		case 7:
			h ^= static_cast<uint64_t>(data8[6]) << 48U;
			[[fallthrough]];
		case 6:
			h ^= static_cast<uint64_t>(data8[5]) << 40U;
			[[fallthrough]];
		case 5:
			h ^= static_cast<uint64_t>(data8[4]) << 32U;
			[[fallthrough]];
		case 4:
			h ^= static_cast<uint64_t>(data8[3]) << 24U;
			[[fallthrough]];
		case 3:
			h ^= static_cast<uint64_t>(data8[2]) << 16U;
			[[fallthrough]];
		case 2:
			h ^= static_cast<uint64_t>(data8[1]) << 8U;
			[[fallthrough]];
		case 1:
			h ^= static_cast<uint64_t>(data8[0]);
			h *= m;
			[[fallthrough]];
		default:
			break;
		}

		h ^= h >> r;

		// not doing the final step here, because this will be done by keyToIdx anyways
		// h *= m;
		// h ^= h >> r;
		return static_cast<size_t>(h);
	}

	inline size_t hash_int(uint64_t x) noexcept {
		// tried lots of different hashes, let's stick with murmurhash3. It's simple, fast, well tested,
		// and doesn't need any special 128bit operations.
		x ^= x >> 33U;
		x *= UINT64_C(0xff51afd7ed558ccd);
		x ^= x >> 33U;

		// not doing the final step here, because this will be done by keyToIdx anyways
		// x *= UINT64_C(0xc4ceb9fe1a85ec53);
		// x ^= x >> 33U;
		return static_cast<size_t>(x);
	}

	// A thin wrapper around std::hash, performing an additional simple mixing step of the result.
	template <typename T, typename Enable = void>
	struct hash : public std::hash<T> {
		size_t operator()(T const& obj) const
			noexcept(noexcept(std::declval<std::hash<T>>().operator()(std::declval<T const&>()))) {
			// call base hash
			auto result = std::hash<T>::operator()(obj);
			// return mixed of that, to be save against identity has
			return hash_int(static_cast<Pointer>(result));
		}
	};

	template <typename CharT>
	struct hash<std::basic_string<CharT>> {
		size_t operator()(std::basic_string<CharT> const& str) const noexcept {
			return hash_bytes(str.data(), sizeof(CharT) * str.size());
		}
	};

	template <typename CharT>
	struct hash<std::basic_string_view<CharT>> {
		size_t operator()(std::basic_string_view<CharT> const& sv) const noexcept {
			return hash_bytes(sv.data(), sizeof(CharT) * sv.size());
		}
	};

	template <class T>
	struct hash<T*> {
		size_t operator()(T* ptr) const noexcept {
			return hash_int(reinterpret_cast<Pointer>(ptr));
		}
	};

	template <class T>
	struct hash<std::unique_ptr<T>> {
		size_t operator()(std::unique_ptr<T> const& ptr) const noexcept {
			return hash_int(reinterpret_cast<Pointer>(ptr.get()));
		}
	};

	template <class T>
	struct hash<std::shared_ptr<T>> {
		size_t operator()(std::shared_ptr<T> const& ptr) const noexcept {
			return hash_int(reinterpret_cast<Pointer>(ptr.get()));
		}
	};

	template <typename Enum>
	struct hash<Enum, typename std::enable_if<std::is_enum<Enum>::value>::type> {
		size_t operator()(Enum e) const noexcept {
			using Underlying = typename std::underlying_type<Enum>::type;
			return hash<Underlying>{}(static_cast<Underlying>(e));
		}
	};

#define ROBIN_HOOD_HASH_INT(T)                           \
    template <>                                          \
    struct hash<T> {                                     \
        size_t operator()(T const& obj) const noexcept { \
            return hash_int(static_cast<uint64_t>(obj)); \
        }                                                \
    }

#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

	// see https://en.cppreference.com/w/cpp/utility/hash
	ROBIN_HOOD_HASH_INT(bool);
	ROBIN_HOOD_HASH_INT(char);
	ROBIN_HOOD_HASH_INT(signed char);
	ROBIN_HOOD_HASH_INT(unsigned char);
	ROBIN_HOOD_HASH_INT(char16_t);
	ROBIN_HOOD_HASH_INT(char32_t);
	ROBIN_HOOD_HASH_INT(wchar_t);
	ROBIN_HOOD_HASH_INT(short);
	ROBIN_HOOD_HASH_INT(unsigned short);
	ROBIN_HOOD_HASH_INT(int);
	ROBIN_HOOD_HASH_INT(unsigned int);
	ROBIN_HOOD_HASH_INT(long);
	ROBIN_HOOD_HASH_INT(long long);
	ROBIN_HOOD_HASH_INT(unsigned long);
	ROBIN_HOOD_HASH_INT(unsigned long long);
#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic pop
#endif

	namespace Inner
	{

		template<class T>
		struct void_type {
			using type = void;
		};


		template<class T>
		concept has_is_transparent = requires { T::is_transparent; };

		// using wrapper classes for hash and key_equal prevents the diamond problem when the same type
		// is used. see https://stackoverflow.com/a/28771920/48181
		template <typename T>
		struct WrapHash : public T {
			WrapHash() = default;
			explicit WrapHash(T const& o) noexcept(noexcept(T(std::declval<T const&>())))
				: T(o) {}
		};

		template <typename T>
		struct WrapKeyEqual : public T {
			WrapKeyEqual() = default;
			explicit WrapKeyEqual(T const& o) noexcept(noexcept(T(std::declval<T const&>())))
				: T(o) {}
		};

		template<class T>
		concept IsFlatMap = T::IsFlat;
		template<class T>
		concept IsMap = T::is_map;
		template<class T>
		concept IsSet = T::is_set;
		template<class T>
		concept IsTransparent = T::is_transparent;

		/// Type needs to be wider than uint8_t											
		using InfoType = uint32_t;

#define TABLE_TEMPLATE() template<bool IsFlat, size_t MaxLoadFactor100, class Key, class T>
#define TABLE() Table<IsFlat, MaxLoadFactor100, Key, T>



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
		class Table : public NodeAllocator<Conditional<IsVoid<T>, Key, TPair<Conditional<IsFlat, Key, Key const>, T>>, 4, 16384, IsFlat>
		{
		public:
			static constexpr bool is_flat = IsFlat;
			static constexpr bool is_map = not IsVoid<T>;
			static constexpr bool is_set = !is_map;
			//static constexpr bool is_transparent = has_is_transparent<Hash> && has_is_transparent<KeyEqual>;

			using PairInner = TPair<Conditional<IsFlat, Key, const Key>, T>;
			using Pair = TPair<Key, T>;
			using key_type = Key;
			using mapped_type = T;
			using value_type = Conditional<is_set, Key, PairInner>;
			using Self = TABLE();
			using Node = Conditional<
				IsFlat,
				DataNodeOnStack<Self, value_type, mapped_type>,
				DataNodeOnHeap<Self, value_type, mapped_type>
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
			using DataPool = NodeAllocator<value_type, 4, 16384, IsFlat>;

		public:
			Table() noexcept;
			explicit Table(size_t) noexcept;

			template<typename Iter>
			Table(Iter, Iter, size_t = 0);
			Table(std::initializer_list<value_type>, size_t = 0);
			Table(Table&&) noexcept;
			Table(const Table&);
			~Table();

			Table& operator = (Table&&) noexcept;
			Table& operator = (const Table&);

			Table& operator = (Pair&&) noexcept;
			Table& operator = (const Pair&);

			DMeta GetKeyType() const;
			DMeta GetValueType() const;

			template<class ALT_T>
			bool KeyIs() const noexcept;
			template<class ALT_T>
			bool ValueIs() const noexcept;

			template<bool REHASH = false>
			void Allocate(size_t);
			void Rehash(size_t);

			Table& operator << (Pair&&);
			Table& operator << (const Pair&);
			Table& operator >> (Pair&&);
			Table& operator >> (const Pair&);


			/// Helpers for insertKeyPrepareEmptySpot: extract first entry			
			/// (only const required)															
			NOD() key_type const& getFirstConst(Node const& n) const noexcept {
				return n.getFirst();
			}

			/// In case we have void mapped_type, we are not using a pair, thus	
			/// we just route k through. No need to disable this because it's		
			/// just not used if not applicable												
			NOD() key_type const& getFirstConst(key_type const& k) const noexcept {
				return k;
			}

			/// In case we have non-void mapped_type, we have a standard			
			/// robin_hood::pair																	
			template <ReflectedData Q = mapped_type>
			NOD() key_type const& getFirstConst(value_type const& vt) const noexcept {
				return vt.first;
			}

			/// Destroyer																			
			template<bool DEALLOCATE>
			void DestroyNodes() noexcept {
				mNumElements = 0;

				if constexpr (!IsFlat || !std::is_trivially_destructible_v<Node>) {
					// Clear also resets mInfo to 0, that's sometimes not		
					// necessary															
					auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);
					for (size_t idx = 0; idx < numElementsWithBuffer; ++idx) {
						if (0 != mInfo[idx]) {
							Node& n = mKeyVals[idx];
							if constexpr (DEALLOCATE)
								n.destroy(*this);
							else
								n.destroyDoNotDeallocate();
							n.~Node();
						}
					}
				}
			}

			/// Highly performance relevant code											
			/// Lower bits are used for indexing into the array (2^n size)			
			/// The upper 1-5 bits need to be a reasonable good hash, to save		
			/// comparisons																		
			template<class HashKey>
			void keyToIdx(HashKey&& key, size_t* idx, InfoType* info) const {
				static_assert(IsHashable<HashKey>, "Contained key type is not hashable");
				// In addition to whatever hash is used, add another mul &	
				// shift so we get better hashing. This serves as a bad		
				// hash prevention, if the given data is badly mixed.			
				uint64_t h = static_cast<uint64_t>(key.GetHash());
				h *= mHashMultiplier;
				h ^= h >> 33U;

				// The lower InitialInfoNumBits are reserved for info			
				*info = mInfoInc + static_cast<InfoType>((h & InfoMask) >> mInfoHashShift);
				*idx = (static_cast<size_t>(h) >> InitialInfoNumBits) & mMask;
			}

			// Forwards the index by one, wrapping around at the end			
			void next(InfoType* info, size_t* idx) const noexcept {
				*idx = *idx + 1;
				*info += mInfoInc;
			}

			void nextWhileLess(InfoType* info, size_t* idx) const noexcept {
				// Unrolling this by hand did not bring any speedups			
				while (*info < mInfo[*idx])
					next(info, idx);
			}

			// Shift everything up by one element									
			// Tries to move stuff around												
			void shiftUp(size_t startIdx, size_t const insertion_idx) noexcept(std::is_nothrow_move_assignable_v<Node>) {
				auto idx = startIdx;
				::new (static_cast<void*>(mKeyVals + idx)) Node(std::move(mKeyVals[idx - 1]));
				while (--idx != insertion_idx)
					mKeyVals[idx] = std::move(mKeyVals[idx - 1]);

				idx = startIdx;
				while (idx != insertion_idx) {
					mInfo[idx] = static_cast<uint8_t>(mInfo[idx - 1] + mInfoInc);
					if (LANGULUS_UNLIKELY(mInfo[idx] + mInfoInc > 0xFF))
						mMaxNumElementsAllowed = 0;
					--idx;
				}
			}

			void shiftDown(size_t idx) noexcept(std::is_nothrow_move_assignable_v<Node>) {
				// Until we find one that is either empty or has zero offset
				// TODO(martinus) we don't need to move everything, just		
				// the last one for the same bucket									
				mKeyVals[idx].destroy(*this);

				// Until we find one that is either empty or has zero offset
				while (mInfo[idx + 1] >= 2 * mInfoInc) {
					mInfo[idx] = static_cast<uint8_t>(mInfo[idx + 1] - mInfoInc);
					mKeyVals[idx] = std::move(mKeyVals[idx + 1]);
					++idx;
				}

				mInfo[idx] = 0;
				// Don't destroy, we've moved it
				// mKeyVals[idx].destroy(*this);
				mKeyVals[idx].~Node();
			}

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
			Insertion try_emplace(const key_type&, Args&&...);
			template <class... Args>
			Insertion try_emplace(key_type&&, Args&&...);
			template <class... Args>
			iterator try_emplace(const_iterator, const key_type&, Args&&...);
			template <class... Args>
			iterator try_emplace(const_iterator, key_type&&, Args&&...);

			template <class Mapped>
			Insertion insert_or_assign(const key_type&, Mapped&&);
			template <class Mapped>
			Insertion insert_or_assign(key_type&&, Mapped&&);
			template <class Mapped>
			iterator insert_or_assign(const_iterator, const key_type&, Mapped&&);
			template <class Mapped>
			iterator insert_or_assign(const_iterator, key_type&&, Mapped&&);

			Insertion Insert(const value_type&);
			iterator Insert(const_iterator, const value_type&);
			Insertion Insert(value_type&&);
			iterator Insert(const_iterator, value_type&&);

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
			size_t RemoveKey(const key_type&);
			size_t RemoveValue(const mapped_type&);
			void compact();
		private:
			void destroy();

		public:
			///																						
			///	SEARCH																			
			///																						
			size_t count(const key_type&) const;
			template <class OtherKey, class Self_ = Self>
			size_t count(const OtherKey&) const requires IsTransparent<Self_>;
			bool contains(const key_type&) const;
			template <class OtherKey, class Self_ = Self>
			bool contains(const OtherKey&) const requires IsTransparent<Self_>;

			template<ReflectedData Q = mapped_type>
			Q& at(const key_type&);
			template<ReflectedData Q = mapped_type>
			const Q& at(const key_type&) const;

			template<class ALT_K>
			const_iterator find(const ALT_K&) const;
			template<class ALT_K>
			iterator find(const ALT_K&);

			template<class Other>
			NOD() size_t findIdx(const Other&) const;

			template <ReflectedData Q = mapped_type>
			Q& operator[] (const key_type&);
			template <ReflectedData Q = mapped_type>
			Q& operator[] (key_type&&);


			///																						
			///	ITERATION																		
			///																						
			iterator begin();
			const_iterator begin() const;
			const_iterator cbegin() const;
			iterator end();
			const_iterator end() const;
			const_iterator cend() const;
			NOD() constexpr Count GetCount() const noexcept;
			NOD() constexpr Count max_size() const noexcept;
			NOD() constexpr bool IsEmpty() const noexcept;
			NOD() constexpr float max_load_factor() const noexcept;
			NOD() constexpr float load_factor() const noexcept;
			NOD() size_t mask() const noexcept;
			NOD() size_t calcMaxNumElementsAllowed(size_t) const noexcept;
			NOD() size_t calcNumBytesInfo(size_t) const noexcept;
			NOD() size_t calcNumElementsWithBuffer(size_t) const noexcept;
			NOD() size_t calcNumBytesTotal(size_t) const;

		private:
			template <ReflectedData Q = mapped_type>
			NOD() bool has(const value_type& e) const {
				auto it = find(e.first);
				return it != end() && it->second == e.second;
			}

			template <IsVoid Q = mapped_type>
			NOD() bool has(const value_type& e) const {
				return find(e) != end();
			}

			/// Reserves space for at least the specified number of elements.		
			/// Only works if numBuckets if power of two. True on success			
			void rehashPowerOfTwo(size_t numBuckets, bool forceFree) {
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
						if (forceFree)
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

			/// True if resize was possible, false otherwise							
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
					rehashPowerOfTwo(mMask + 1, true);
				}
				else {
					// We've reached the capacity of the map, so the hash		
					// seems to work nice. Keep using it							
					rehashPowerOfTwo((mMask + 1) * 2, false);
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

	} // namespace detail

	template<class... T>
	concept IsNoexceptMoveConstructibleOrAssignable = 
		((std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) && ...);

	/// Map																							
	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_flat_map = Inner::Table<true, MaxLoadFactor100, K, V>;

	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_node_map = Inner::Table<false, MaxLoadFactor100, K, V>;

	template <class K, class V, Count MaxLoadFactor100 = 80>
	using unordered_map = Inner::Table<sizeof(TPair<K, V>) <= sizeof(Count) * 6 && IsNoexceptMoveConstructibleOrAssignable<TPair<K, V>>, MaxLoadFactor100, K, V>;

	/// Set																							
	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_flat_set = Inner::Table<true, MaxLoadFactor100, K, void>;

	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_node_set = Inner::Table<false, MaxLoadFactor100, K, void>;

	template <class K, Count MaxLoadFactor100 = 80>
	using unordered_set = Inner::Table<sizeof(K) <= sizeof(Count) * 6 && IsNoexceptMoveConstructibleOrAssignable<K>, MaxLoadFactor100, K, void>;

} // namespace Langulus::Anyness

#include "THashMap.inl"

#undef TABLE_TEMPLATE
#undef TABLE
