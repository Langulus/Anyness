///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>
#include <unordered_map>

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md			
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
	const Text serialized {ex};
	return ::std::string {Token {serialized}};
}

using uint = unsigned int;
using timer = Catch::Benchmark::Chronometer;
template<class T>
using some = std::vector<T>;
template<class T>
using uninitialized = Catch::Benchmark::storage_for<T>;

template<class C, class K, class V>
struct TypePair {
	using Container = C;
	using Key = K;
	using Value = V;
};

namespace std {
	template<>
	struct hash<Text> {
		size_t operator()(const Text& str) const noexcept {
			return str.GetHash().mHash;
		}
	};
}

template<class P, class K, class V, class ALT_K, class ALT_V>
P CreatePair(const ALT_K& key, const ALT_V& value) {
	K keyValue;
	if constexpr (CT::Sparse<K>)
		keyValue = new Decay<K> {key};
	else
		keyValue = key;

	V valueValue;
	if constexpr (CT::Sparse<V>)
		valueValue = new Decay<V> {value};
	else
		valueValue = value;

	return P {keyValue, valueValue};
}

/// Detect if type is TAny, instead of Any, by searching for [] operator		
template<class T>
concept IsStaticallyOptimized = requires (Decay<T> a) { typename T::Key; typename T::Value; };


/// The main test for TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap			
/// containers, with all kinds of items, from sparse to dense, from trivial	
/// to complex, from flat to deep															
TEMPLATE_TEST_CASE(
	"TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap", "[map]",
	(TypePair<TUnorderedMap<Text, int*>, Text, int*>),
	(TypePair<UnorderedMap, Text, int*>),
	(TypePair<TUnorderedMap<Text, int>, Text, int>),
	(TypePair<TUnorderedMap<Text, Trait>, Text, Trait>),
	(TypePair<TUnorderedMap<Text, Traits::Count>, Text, Traits::Count>),
	(TypePair<TUnorderedMap<Text, Any>, Text, Any>),
	(TypePair<TUnorderedMap<Text, Trait*>, Text, Trait*>),
	(TypePair<TUnorderedMap<Text, Traits::Count*>, Text, Traits::Count*>),
	(TypePair<TUnorderedMap<Text, Any*>, Text, Any*>),
	(TypePair<TOrderedMap<Text, int>, Text, int>),
	(TypePair<TOrderedMap<Text, Trait>, Text, Trait>),
	(TypePair<TOrderedMap<Text, Traits::Count>, Text, Traits::Count>),
	(TypePair<TOrderedMap<Text, Any>, Text, Any>),
	(TypePair<TOrderedMap<Text, int*>, Text, int*>),
	(TypePair<TOrderedMap<Text, Trait*>, Text, Trait*>),
	(TypePair<TOrderedMap<Text, Traits::Count*>, Text, Traits::Count*>),
	(TypePair<TOrderedMap<Text, Any*>, Text, Any*>),
	(TypePair<UnorderedMap, Text, int>),
	(TypePair<UnorderedMap, Text, Trait>),
	(TypePair<UnorderedMap, Text, Traits::Count>),
	(TypePair<UnorderedMap, Text, Any>),
	(TypePair<UnorderedMap, Text, Trait*>),
	(TypePair<UnorderedMap, Text, Traits::Count*>),
	(TypePair<UnorderedMap, Text, Any*>),
	(TypePair<OrderedMap, Text, int>),
	(TypePair<OrderedMap, Text, Trait>),
	(TypePair<OrderedMap, Text, Traits::Count>),
	(TypePair<OrderedMap, Text, Any>),
	(TypePair<OrderedMap, Text, int*>),
	(TypePair<OrderedMap, Text, Trait*>),
	(TypePair<OrderedMap, Text, Traits::Count*>),
	(TypePair<OrderedMap, Text, Any*>)
) {
	using T = typename TestType::Container;
	constexpr bool ordered = T::Ordered;
	constexpr bool statico = IsStaticallyOptimized<T>;
	using K = typename TestType::Key;
	using V = typename TestType::Value;
	using StdT = Conditional<ordered, ::std::map<K, V>, ::std::unordered_map<K, V>>;
	using Pair = TPair<K, V>;
	using StdPair = ::std::pair<K, V>;
	using DenseK = Decay<K>;
	using DenseV = Decay<V>;

	GIVEN("A default-initialized map instance") {
		const auto pair = CreatePair<Pair, K, V>(
			"five hundred"_text, 555);
		const auto stdpair = CreatePair<StdPair, K, V>(
			"five hundred"_text, 555);

		const T map {};

		WHEN("Given a default-constructed map") {
			THEN("These properties should be correct") {
				if constexpr (IsStaticallyOptimized<T>) {
					REQUIRE(map.template KeyIs<K>());
					REQUIRE(map.template ValueIs<V>());
				}

				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.IsKeyTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsValueTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsEmpty());
				REQUIRE(map.GetUses() == 0);
				REQUIRE_FALSE(map.IsAllocated());
				REQUIRE_FALSE(map.HasAuthority());
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::map::default construction") (timer meter) {
					some<uninitialized<MapType>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct();
					});
				};

				BENCHMARK_ADVANCED("std::map::default construction") (timer meter) {
					some<uninitialized<MapTypeStd>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct();
					});
				};
			#endif
		}

		WHEN("Given a pair by copy") {
			#include "CollectGarbage.inl"

			const_cast<T&>(map) = pair;

			THEN("Various traits change") {
				REQUIRE(map.IsKeyTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsValueTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.template KeyIs<K>());
				REQUIRE(map.template ValueIs<V>());
				REQUIRE(map.GetKeyStride() == (CT::Dense<K> ? sizeof(K) : sizeof(Block::KnownPointer)));
				REQUIRE(map.GetValueStride() == (CT::Dense<V> ? sizeof(V) : sizeof(Block::KnownPointer)));
				REQUIRE(map.IsAllocated());
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetCount() == 1);
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map[pair.mKey] == pair.mValue);
				REQUIRE_THROWS(map["missing"_text] == pair.mValue);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TUnorderedMap::operator = (single pair copy)") (timer meter) {
					some<MapType> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = pair;
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::insert(single pair copy)") (timer meter) {
					some<MapTypeStd> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].insert(stdpair);
					});
				};
			#endif
		}

		WHEN("Given a pair by move") {
			#include "CollectGarbage.inl"

			auto movablePair = pair;
			const_cast<T&>(map) = Move(movablePair);

			THEN("Various traits change") {
				REQUIRE(map.IsKeyTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsValueTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(movablePair != pair);
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.template KeyIs<K>());
				REQUIRE(map.template ValueIs<V>());
				REQUIRE(map.IsAllocated());
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetCount() == 1);
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map[pair.mKey] == pair.mValue);
				REQUIRE_THROWS(map["missing"_text] == pair.mValue);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TUnorderedMap::operator = (single pair copy)") (timer meter) {
					some<Pair> source(meter.runs());
					for (auto& i : source)
						i = CreatePair<Pair, K, V>("five hundred"_text, 555);
						
					some<MapType> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(source[i]);
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::insert(single pair copy)") (timer meter) {
					some<StdPair> source(meter.runs());
					for(auto& i : source)
						i = valueStd;

					some<MapTypeStd> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].emplace(Move(source[i]));
					});
				};
			#endif
		}
	}

	GIVEN("Map with some items") {
		#include "CollectGarbage.inl"

		// Arrays are dynamic to avoid constexprification						
		const Pair darray1[5] {
			CreatePair<Pair, K, V>("one"_text, 1), 
			CreatePair<Pair, K, V>("two"_text, 2), 
			CreatePair<Pair, K, V>("three"_text, 3), 
			CreatePair<Pair, K, V>("four"_text, 4), 
			CreatePair<Pair, K, V>("five"_text, 5)
		};
		const Pair darray2[5] {
			CreatePair<Pair, K, V>("six"_text, 6),
			CreatePair<Pair, K, V>("seven"_text, 7),
			CreatePair<Pair, K, V>("eight"_text, 8),
			CreatePair<Pair, K, V>("nine"_text, 9),
			CreatePair<Pair, K, V>("ten"_text, 10)
		};

		const StdPair darray1std[5] {
			CreatePair<StdPair, K, V>("one"_text, 1),
			CreatePair<StdPair, K, V>("two"_text, 2),
			CreatePair<StdPair, K, V>("three"_text, 3),
			CreatePair<StdPair, K, V>("four"_text, 4),
			CreatePair<StdPair, K, V>("five"_text, 5)
		};
		const StdPair darray2std[5] {
			CreatePair<StdPair, K, V>("six"_text, 6),
			CreatePair<StdPair, K, V>("seven"_text, 7),
			CreatePair<StdPair, K, V>("eight"_text, 8),
			CreatePair<StdPair, K, V>("nine"_text, 9),
			CreatePair<StdPair, K, V>("ten"_text, 10)
		};

		const T map {};
		const_cast<T&>(map) << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
		auto keyMemory = map.GetRawKeysMemory();
		auto valueMemory = map.GetRawValuesMemory();

		WHEN("Given a preinitialized map with 5 elements") {
			THEN("These properties should be correct") {
				REQUIRE(map.GetCount() == 5);
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.template KeyIs<K>());
				REQUIRE(map.template ValueIs<V>());
				REQUIRE_FALSE(map.template KeyIs<int>());
				REQUIRE_FALSE(map.template KeyIs<char>());
				REQUIRE_FALSE(map.template ValueIs<float>());
				REQUIRE_FALSE(map.template ValueIs<unsigned char>());
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				for (auto& comparer : darray1)
					REQUIRE(map[comparer.mKey] == comparer.mValue);
				REQUIRE(map.GetReserved() >= 5);
			}
		}

		WHEN("Create 2048 and then 4096 maps, and initialize them (weird corner case test)") {
			auto storage = new some<T> {2048};
			const void* prevKeys = nullptr;
			const void* prevValues = nullptr;

			for (auto& i : *storage) {
				i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
				if (prevKeys && prevValues) {
					REQUIRE(prevKeys != i.GetRawKeysMemory());
					REQUIRE(prevValues != i.GetRawValuesMemory());
					REQUIRE(i == *(&i - 1));
				}

				prevKeys = i.GetRawKeysMemory();
				prevValues = i.GetRawValuesMemory();

				REQUIRE(i.HasAuthority());
				REQUIRE(i.GetUses() == 1);
				REQUIRE(i.GetCount() == 5);
				REQUIRE(i.GetReserved() == 8);
				for (auto& comparer : darray1)
					REQUIRE(i[comparer.mKey] == comparer.mValue);
			}

			delete storage;
			storage = new some<T>();

			prevValues = nullptr;
			prevKeys = nullptr;

			for (auto& i : *storage) {
				i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
				if (prevKeys && prevValues) {
					REQUIRE(prevKeys != i.GetRawKeysMemory());
					REQUIRE(prevValues != i.GetRawValuesMemory());
					REQUIRE(i == *(&i - 1));
				}

				prevKeys = i.GetRawKeysMemory();
				prevValues = i.GetRawValuesMemory();

				REQUIRE(i.HasAuthority());
				REQUIRE(i.GetUses() == 1);
				REQUIRE(i.GetCount() == 5);
				REQUIRE(i.GetReserved() == 8);
				for (auto& comparer : darray1)
					REQUIRE(i[comparer.mKey] == comparer.mValue);
			}

			delete storage;
		}

		WHEN("Shallow-copy more of the same stuff") {
			for (auto& comparer : darray1)
				REQUIRE(map[comparer.mKey] == comparer.mValue);

			const_cast<T&>(map) << darray2[0];
			for (auto& comparer : darray1)
				REQUIRE(map[comparer.mKey] == comparer.mValue);

			const_cast<T&>(map) << darray2[1];
			for (auto& comparer : darray1)
				REQUIRE(map[comparer.mKey] == comparer.mValue);

			const_cast<T&>(map) << darray2[2];
			for (auto& comparer : darray1)
				REQUIRE(map[comparer.mKey] == comparer.mValue);

			const_cast<T&>(map) << darray2[3];
			for (auto& comparer : darray1)
				REQUIRE(map[comparer.mKey] == comparer.mValue);

			const_cast<T&>(map) << darray2[4];
			for (auto& comparer : darray1)
				REQUIRE(map[comparer.mKey] == comparer.mValue);

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(map.IsKeyTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsValueTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.template KeyIs<K>());
				REQUIRE(map.template ValueIs<V>());
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map.GetCount() == 10);
				for (auto& comparer : darray1)
					REQUIRE(map[comparer.mKey] == comparer.mValue);
				for (auto& comparer : darray2)
					REQUIRE(map[comparer.mKey] == comparer.mValue);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(map.GetRawKeysMemory() == keyMemory);
					REQUIRE(map.GetRawValuesMemory() == valueMemory);
				#endif
				REQUIRE(map.GetReserved() >= 10);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TUnorderedMap::operator << (5 consecutive pair copies)") (timer meter) {
					some<MapType> storage(meter.runs());
					for (auto& i : storage)
						i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i] << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::insert(5 consecutive pair copies)") (timer meter) {
					some<MapTypeStd> storage(meter.runs());
					for (auto& i : storage) {
						i.insert(darray1std[0]);
						i.insert(darray1std[1]);
						i.insert(darray1std[2]);
						i.insert(darray1std[3]);
						i.insert(darray1std[4]);
					}

					meter.measure([&](int i) {
						storage[i].insert(darray2std[0]);
						storage[i].insert(darray2std[1]);
						storage[i].insert(darray2std[2]);
						storage[i].insert(darray2std[3]);
						return storage[i].insert(darray2std[4]);
					});
				};

				// Last result: 1:1, slightly slower than STD, can be further improved
				BENCHMARK_ADVANCED("Anyness::TUnorderedMap::operator [] (retrieval by key from a map with 10 pairs)") (timer meter) {
					some<MapType> storage(meter.runs());
					for (auto& i : storage) {
						i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
						i << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
					}

					meter.measure([&](int i) {
						return storage[i]["seven"];
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::operator [] (retrieval by key from a map with 10 pairs)") (timer meter) {
					some<MapTypeStd> storage(meter.runs());
					for (auto& i : storage) {
						i.insert(darray1std[0]);
						i.insert(darray1std[1]);
						i.insert(darray1std[2]);
						i.insert(darray1std[3]);
						i.insert(darray1std[4]);
						i.insert(darray2std[0]);
						i.insert(darray2std[1]);
						i.insert(darray2std[2]);
						i.insert(darray2std[3]);
						i.insert(darray2std[4]);
					}

					meter.measure([&](int i) {
						return storage[i]["seven"];
					});
				};
			#endif
		}

		WHEN("Move more of the same stuff") {
			Pair movableDarray2[5] {
				darray2[0],
				darray2[1],
				darray2[2],
				darray2[3],
				darray2[4]
			};

			const_cast<T&>(map)
				<< Move(movableDarray2[0])
				<< Move(movableDarray2[1])
				<< Move(movableDarray2[2])
				<< Move(movableDarray2[3])
				<< Move(movableDarray2[4]);

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map.GetCount() == 10);
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				for (auto& comparer : darray1)
					REQUIRE(map[comparer.mKey] == comparer.mValue);
				for (auto& comparer : darray2)
					REQUIRE(map[comparer.mKey] == comparer.mValue);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(map.GetRawKeysMemory() == keyMemory);
					REQUIRE(map.GetRawValuesMemory() == valueMemory);
				#endif
				REQUIRE(map.GetReserved() >= 10);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TUnorderedMap::operator << (5 consecutive trivial moves)") (timer meter) {
					some<MapType> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] << Move(darray2[0]) << Move(darray2[1]) << Move(darray2[2]) << Move(darray2[3]) << Move(darray2[4]);
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::emplace_back(5 consecutive trivial moves)") (timer meter) {
					some<MapTypeStd> storage(meter.runs());
					meter.measure([&](int i) {
						storage[i].emplace(Move(darray2std[0]));
						storage[i].emplace(Move(darray2std[1]));
						storage[i].emplace(Move(darray2std[2]));
						storage[i].emplace(Move(darray2std[3]));
						return storage[i].emplace(Move(darray2std[4]));
					});
				};
			#endif
		}

		WHEN("The size is reduced by finding and removing elements by value, but reserved memory should remain the same on shrinking") {
			const auto removed2 = const_cast<T&>(map).RemoveValue(darray1[1].mValue);
			const auto removed4 = const_cast<T&>(map).RemoveValue(darray1[3].mValue);

			THEN("The size changes but not capacity") {
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(removed2 == 1);
				REQUIRE(removed4 == 1);
				REQUIRE(map[darray1[0].mKey] == darray1[0].mValue);
				REQUIRE(map[darray1[2].mKey] == darray1[2].mValue);
				REQUIRE(map[darray1[4].mKey] == darray1[4].mValue);
				REQUIRE_THROWS(map[darray1[1].mKey] == darray1[1].mValue);
				REQUIRE_THROWS(map[darray1[3].mKey] == darray1[3].mValue);
				REQUIRE(map.GetCount() == 3);
				REQUIRE(map.GetRawKeysMemory() == keyMemory);
				REQUIRE(map.GetRawValuesMemory() == valueMemory);
				REQUIRE(map.GetReserved() >= 5);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TUnorderedMap::RemoveValue") (timer meter) {
					some<MapType> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].RemoveValue(2);
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::erase(by value)") (timer meter) {
					some<MapTypeStd> storage(meter.runs());
					for (auto&& i : storage) {
						i.insert(darray1std[0]);
						i.insert(darray1std[1]);
						i.insert(darray1std[2]);
						i.insert(darray1std[3]);
						i.insert(darray1std[4]);
					}

					meter.measure([&](int i) {
						auto it = storage[i].begin();
						while (it != storage[i].end()) {
							if (it->second == 2) {
								it = storage[i].erase(it);
								continue;
							}
							it++;
						}
						return it;
					});
				};
			#endif
		}

		WHEN("The size is reduced by finding and removing elements by key, but reserved memory should remain the same on shrinking") {
			const auto removed2 = const_cast<T&>(map).RemoveKey(darray1[1].mKey);
			const auto removed4 = const_cast<T&>(map).RemoveKey(darray1[3].mKey);

			THEN("The size changes but not capacity") {
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(removed2 == 1);
				REQUIRE(removed4 == 1);
				REQUIRE(map[darray1[0].mKey] == darray1[0].mValue);
				REQUIRE(map[darray1[2].mKey] == darray1[2].mValue);
				REQUIRE(map[darray1[4].mKey] == darray1[4].mValue);
				REQUIRE_THROWS(map[darray1[1].mKey] == darray1[1].mValue);
				REQUIRE_THROWS(map[darray1[3].mKey] == darray1[3].mValue);
				REQUIRE(map.GetCount() == 3);
				REQUIRE(map.GetRawKeysMemory() == keyMemory);
				REQUIRE(map.GetRawValuesMemory() == valueMemory);
				REQUIRE(map.GetReserved() >= 5);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TUnorderedMap::RemoveKey") (timer meter) {
					some<MapType> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].RemoveKey("two");
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::erase(by key)") (timer meter) {
					some<MapTypeStd> storage(meter.runs());
					for (auto&& i : storage) {
						i.insert(darray1std[0]);
						i.insert(darray1std[1]);
						i.insert(darray1std[2]);
						i.insert(darray1std[3]);
						i.insert(darray1std[4]);
					}

					meter.measure([&](int i) {
						return storage[i].erase("two");
					});
				};
			#endif
		}

		WHEN("Removing non-available elements") {
			const auto removed9 = const_cast<T&>(map).RemoveValue(darray2[3].mValue);

			THEN("Nothing should change") {
				REQUIRE(removed9 == 0);
				for (auto& comparer : darray1)
					REQUIRE(map[comparer.mKey] == comparer.mValue);
				REQUIRE(map.GetCount() == 5);
				REQUIRE(map.GetRawKeysMemory() == keyMemory);
				REQUIRE(map.GetRawValuesMemory() == valueMemory);
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map.GetReserved() >= 5);
			}
		}

		WHEN("More capacity is reserved") {
			const_cast<T&>(map).Allocate(20);

			THEN("The capacity changes but not the size, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map.GetCount() == 5);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(map.GetRawKeysMemory() == keyMemory);
					REQUIRE(map.GetRawValuesMemory() == valueMemory);
				#endif
				REQUIRE(map.GetReserved() >= 20);
			}
		}

		WHEN("Less capacity is reserved") {
			const_cast<T&>(map).Allocate(2);

			THEN("Capacity and count remain unchanged") {
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map.GetCount() == 5);
				REQUIRE(map.GetRawKeysMemory() == keyMemory);
				REQUIRE(map.GetRawValuesMemory() == valueMemory);
				REQUIRE(map.GetReserved() >= 5);
			}
		}

		WHEN("Map is cleared") {
			const_cast<T&>(map).Clear();

			THEN("Size goes to zero, capacity and types are unchanged") {
				REQUIRE(map.GetCount() == 0);
				REQUIRE(map.IsAllocated());
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				REQUIRE(map.template KeyIs<K>());
				REQUIRE(map.template ValueIs<V>());
				REQUIRE(map.IsKeyTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsValueTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsEmpty());
				REQUIRE(map.GetRawKeysMemory() == keyMemory);
				REQUIRE(map.GetRawValuesMemory() == valueMemory);
				REQUIRE(map.HasAuthority());
				REQUIRE(map.GetUses() == 1);
				REQUIRE(map.GetReserved() >= 5);
			}
		}

		WHEN("Map is reset") {
			const_cast<T&>(map).Reset();

			THEN("Size and capacity goes to zero, types are unchanged if statically optimized") {
				REQUIRE(map.GetCount() == 0);
				REQUIRE_FALSE(map.IsAllocated());
				REQUIRE_FALSE(map.HasAuthority());
				REQUIRE(map.GetKeyTypeInner() == map.GetKeyType());
				REQUIRE(map.GetValueTypeInner() == map.GetValueType());
				if constexpr (IsStaticallyOptimized<T>) {
					REQUIRE(map.template KeyIs<K>());
					REQUIRE(map.template ValueIs<V>());
				}
				REQUIRE(map.IsKeyTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsValueTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(map.IsEmpty());
				REQUIRE(map.GetRawKeysMemory() != keyMemory);
				REQUIRE(map.GetRawValuesMemory() != valueMemory);
				REQUIRE(map.GetUses() == 0);
			}
		}

		/*#if LANGULUS_FEATURE(MANAGED_MEMORY)
			WHEN("Pack is reset, then immediately allocated again") {
				map.Reset();
				map << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];

				THEN("Block manager should reuse the memory, if MANAGED_MEMORY feature is enabled") {
					REQUIRE(map.GetRawKeys() == keyMemory);
					REQUIRE(map.GetRawValues() == valueMemory);
				}
			}
		#endif*/

		WHEN("Map is shallow-copied") {
			auto copy = map;

			THEN("The new map should keep the state and refer to original data") {
				REQUIRE(copy == map);
				REQUIRE(copy.GetKeyTypeInner() == copy.GetKeyType());
				REQUIRE(copy.GetValueTypeInner() == copy.GetValueType());
				REQUIRE(copy.IsAllocated());
				REQUIRE(copy.HasAuthority());
				REQUIRE(copy.GetUses() == 2);
				REQUIRE(copy.GetCount() == map.GetCount());
				REQUIRE(copy.GetCount() == 5);
				REQUIRE(copy.GetRawKeysMemory() == map.GetRawKeysMemory());
				REQUIRE(copy.GetRawValuesMemory() == map.GetRawValuesMemory());
				for (auto& comparer : darray1)
					REQUIRE(copy[comparer.mKey] == comparer.mValue);

				if constexpr (IsStaticallyOptimized<T>) {
					for (auto& comparer : darray1)
						REQUIRE(&map[comparer.mKey] == &copy[comparer.mKey]);
				}

				/*REQUIRE(copy.GetRaw() == map.GetRaw());
				REQUIRE(copy.GetCount() == map.GetCount());
				REQUIRE(copy.GetReserved() == map.GetReserved());
				REQUIRE(copy.GetState() == map.GetState());
				REQUIRE(copy.GetType() == map.GetType());
				REQUIRE(copy.GetReferences() == 2);*/
			}
		}

		WHEN("Map is cloned") {
			auto clone = map.Clone();

			THEN("The new map should keep the state, but refer to new data") {
				REQUIRE(clone == map);
				REQUIRE(clone.GetKeyTypeInner() == clone.GetKeyType());
				REQUIRE(clone.GetValueTypeInner() == clone.GetValueType());
				REQUIRE(clone.IsAllocated());
				REQUIRE(clone.HasAuthority());
				REQUIRE(clone.GetUses() == 1);
				REQUIRE(clone.GetCount() == map.GetCount());
				REQUIRE(clone.GetCount() == 5);
				REQUIRE(clone.GetRawKeysMemory() != map.GetRawKeysMemory());
				REQUIRE(clone.GetRawValuesMemory() != map.GetRawValuesMemory());
				for (auto& comparer : darray1) {
					if constexpr (CT::Sparse<V>) {
						if constexpr (IsStaticallyOptimized<T>) {
							REQUIRE(*map[comparer.mKey] == *clone[comparer.mKey]);
							REQUIRE(map[comparer.mKey] != clone[comparer.mKey]);
							REQUIRE(clone[comparer.mKey] != comparer.mValue);
						}
						else {
							REQUIRE(clone[comparer.mKey] == comparer.mValue);
							REQUIRE(map[comparer.mKey] == clone[comparer.mKey]);
						}
					}
					else {
						REQUIRE(clone[comparer.mKey] == comparer.mValue);
						REQUIRE(map[comparer.mKey] == clone[comparer.mKey]);
					}
					
					REQUIRE(map[comparer.mKey] == comparer.mValue);
					
					if constexpr (IsStaticallyOptimized<T>)
						REQUIRE(&map[comparer.mKey] != &clone[comparer.mKey]);
					else
						REQUIRE(map[comparer.mKey].GetRaw() != clone[comparer.mKey].GetRaw());
				}

				/*REQUIRE(clone.GetRaw() != map.GetRaw());
				REQUIRE(clone.GetCount() == map.GetCount());
				REQUIRE(clone.GetReserved() >= clone.GetCount());
				REQUIRE(clone.GetState() == map.GetState());
				REQUIRE(clone.GetType() == map.GetType());
				REQUIRE(clone.GetReferences() == 1);
				REQUIRE(map.GetReferences() == 1);*/
			}
		}

		WHEN("Map is move-constructed") {
			T movable = map;
			T moved = Move(movable);

			THEN("The new pack should keep the state and data") {
				REQUIRE(moved == map);
				REQUIRE(moved != movable);
				REQUIRE(moved.GetKeyTypeInner() == moved.GetKeyType());
				REQUIRE(moved.GetValueTypeInner() == moved.GetValueType());
				REQUIRE(moved.GetRawKeysMemory() == keyMemory);
				REQUIRE(moved.GetRawValuesMemory() == valueMemory);
				REQUIRE(moved.IsAllocated());
				REQUIRE(moved.GetCount() == 5);
				REQUIRE(moved.HasAuthority());
				REQUIRE(moved.GetUses() == 2);
				for (auto& comparer : darray1)
					REQUIRE(moved[comparer.mKey] == comparer.mValue);
				REQUIRE_FALSE(movable.IsAllocated());
				REQUIRE(movable.IsEmpty());
				//REQUIRE(map.GetRawKeys() == nullptr); // not really required
				REQUIRE(movable.GetRawValuesMemory() == nullptr);
				REQUIRE(movable.GetCount() == 0);
				REQUIRE(movable.IsValueTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(movable.IsKeyTypeConstrained() == IsStaticallyOptimized<T>);
				//REQUIRE(map.GetReserved() == 0);
				//REQUIRE(map.GetType() == moved.GetType());
			}
		}

		WHEN("Maps are compared") {
			T sameMap;
			sameMap << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
			T clonedMap {map.Clone()};
			T copiedMap {map};
			T differentMap1;
			differentMap1 << darray1[0] << darray1[0] << darray1[2] << darray1[3] << darray1[4];

			THEN("The comparisons should be adequate") {
				REQUIRE(map == sameMap);
				REQUIRE(map == clonedMap);
				REQUIRE(map == copiedMap);
				REQUIRE(map != differentMap1);
			}
		}

		WHEN("Maps are iterated with ranged-for") {
			for (auto& comparer : darray1)
				REQUIRE(map[comparer.mKey] == comparer.mValue);

			int i = 0;
			for (auto pair : map) {
				static_assert(!IsStaticallyOptimized<T> || ::std::is_reference_v<decltype(pair.mKey)>,
					"Pair key type is not a reference for statically optimized map");
				static_assert(!IsStaticallyOptimized<T> || ::std::is_reference_v<decltype(pair.mValue)>,
					"Pair value type is not a reference for statically optimized map");

				// Different architectures result in different hashes
				if constexpr (Bitness == 32) {
					switch (i) {
					case 0:
						REQUIRE(pair.mKey == darray1[2].mKey);
						REQUIRE(pair.mValue == darray1[2].mValue);
						break;
					case 1:
						REQUIRE(pair.mKey == darray1[3].mKey);
						REQUIRE(pair.mValue == darray1[3].mValue);
						break;
					case 2:
						REQUIRE(pair.mKey == darray1[1].mKey);
						REQUIRE(pair.mValue == darray1[1].mValue);
						break;
					case 3:
						REQUIRE(pair.mKey == darray1[4].mKey);
						REQUIRE(pair.mValue == darray1[4].mValue);
						break;
					case 4:
						REQUIRE(pair.mKey == darray1[0].mKey);
						REQUIRE(pair.mValue == darray1[0].mValue);
						break;
					default:
						FAIL("Index out of bounds in ranged-for");
						break;
					}
				}
				else if constexpr (Bitness == 64) {
					switch (i) {
					case 0:
						REQUIRE(pair.mKey == darray1[1].mKey);
						REQUIRE(pair.mValue == darray1[1].mValue);
						break;
					case 1:
						REQUIRE(pair.mKey == darray1[2].mKey);
						REQUIRE(pair.mValue == darray1[2].mValue);
						break;
					case 2:
						REQUIRE(pair.mKey == darray1[3].mKey);
						REQUIRE(pair.mValue == darray1[3].mValue);
						break;
					case 3:
						REQUIRE(pair.mKey == darray1[4].mKey);
						REQUIRE(pair.mValue == darray1[4].mValue);
						break;
					case 4:
						REQUIRE(pair.mKey == darray1[0].mKey);
						REQUIRE(pair.mValue == darray1[0].mValue);
						break;
					default:
						FAIL("Index out of bounds in ranged-for");
						break;
					}
				}
				else break;

				++i;
			}

			THEN("The comparisons should be adequate") {
				REQUIRE(i == map.GetCount());
			}
		}
	}

	GIVEN("Two maps") {
		#include "CollectGarbage.inl"

		TAny<int> pack1;
		TAny<int> pack2;
		pack1 << int(1) << int(2) << int(3) << int(4) << int(5);
		pack2 << int(6) << int(7) << int(8) << int(9) << int(10);
		const auto memory1 = static_cast<Block>(pack1);
		const auto memory2 = static_cast<Block>(pack2);

		REQUIRE(memory1 != memory2);

		WHEN("Shallow copy pack1 in pack2") {
			pack2 = pack1;

			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetUses() == 2);
				REQUIRE(pack2.GetUses() == 2);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}

		WHEN("Shallow copy pack1 in pack2 and then reset pack1") {
			pack2 = pack1;
			pack1.Reset();

			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetUses() == 1);
				REQUIRE_FALSE(pack1.GetRaw());
				REQUIRE(pack1.GetReserved() == 0);
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}

		WHEN("Deep copy pack1 in pack2") {
			pack2 = pack1.Clone();

			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetUses() == 1);
				REQUIRE(pack2.GetUses() == 1);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(static_cast<Block&>(pack2) != memory2);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}

		WHEN("Deep copy pack1 in pack2, then reset pack1") {
			pack2 = pack1.Clone();
			const auto memory3 = static_cast<Block>(pack2);
			pack1.Reset();

			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetUses() == 1);
				REQUIRE(memory3.GetUses() == 1);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory1.GetType(), memory1.GetRaw()));
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}
	}
}
