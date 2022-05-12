#include "TestMain.hpp"
#include <catch2/catch.hpp>
#include <unordered_map>

using uint = unsigned int;
using MapType = unordered_map<Text, int>;
using MapTypeStd = std::unordered_map<Text, int>;
using StdPair = std::pair<Text, int>;

namespace std {
	template<>
	struct hash<Text> {
		size_t operator()(const Text& str) const noexcept {
			return str.GetHash();
		}
	};
}

SCENARIO("THashMap", "[containers]") {
	GIVEN("A THashMap instance") {
		MapType::Pair value("five hundred", 555);
		StdPair valueStd("five hundred", 555);
		MapType map;
		auto meta1 = map.GetKeyType();
		auto meta2 = map.GetValueType();

		WHEN("Given a default-constructed THashMap") {
			THEN("Various traits change") {
				REQUIRE(meta1);
				REQUIRE(meta2);
				REQUIRE(meta1->Is<Text>());
				REQUIRE(meta2->Is<int>());
				REQUIRE(map.IsKeyTypeConstrained());
				REQUIRE(map.IsValueTypeConstrained());
				REQUIRE(map.IsEmpty());
				REQUIRE_FALSE(map.IsAllocated());
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: hundreds of times faster than STD
				BENCHMARK_ADVANCED("Anyness::THashMap::default construction") (Catch::Benchmark::Chronometer meter) {
					std::vector<Catch::Benchmark::storage_for<MapType>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct();
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::default construction") (Catch::Benchmark::Chronometer meter) {
					std::vector<Catch::Benchmark::storage_for<MapTypeStd>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct();
					});
				};
			#endif
		}

		WHEN("Given a pair by copy") {
			map = value;

			THEN("Various traits change") {
				REQUIRE(map.IsAllocated());
				REQUIRE(map.GetCount() == 1);
				REQUIRE(map["five hundred"] == 555);
				REQUIRE_THROWS(map["missing"] == 555);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 2:1 due to RTTI
				BENCHMARK_ADVANCED("Anyness::THashMap::operator = (single pair copy)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapType> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::insert(single pair copy)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapTypeStd> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].insert(valueStd);
					});
				};
			#endif
		}

		WHEN("Given a pair by move") {
			map = Move(value);

			THEN("Various traits change") {
				REQUIRE(map.IsAllocated());
				REQUIRE(map.GetCount() == 1);
				REQUIRE(map["five hundred"] == 555);
				REQUIRE_THROWS(map["missing"] == 555);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 2:1 due to RTTI
				BENCHMARK_ADVANCED("Anyness::THashMap::operator = (single pair copy)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapType::Pair> source(meter.runs());
					for(auto& i : source)
						i = value;
						
					std::vector<MapType> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(source[i]);
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::insert(single pair copy)") (Catch::Benchmark::Chronometer meter) {
					std::vector<StdPair> source(meter.runs());
					for(auto& i : source)
						i = valueStd;

					std::vector<MapTypeStd> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].emplace(Move(source[i]));
					});
				};
			#endif
		}
	}

	GIVEN("THashMap with some items") {
		// Arrays are dynamic to avoid constexprification						
		auto darray1 = new MapType::Pair[5] {
			{"one", 1}, 
			{"two", 2}, 
			{"three", 3},
			{"four", 4},
			{"five", 5}
		};
		auto darray2 = new MapType::Pair[5] {
			{"six", 6}, 
			{"seven", 7}, 
			{"eight", 8},
			{"nine", 9},
			{"ten", 10}
		};

		auto darray1std = new StdPair[5] {
			{"one", 1}, 
			{"two", 2}, 
			{"three", 3},
			{"four", 4},
			{"five", 5}
		};
		auto darray2std = new StdPair[5] {
			{"six", 6}, 
			{"seven", 7}, 
			{"eight", 8},
			{"nine", 9},
			{"ten", 10}
		};

		MapType map;
		map << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
		//auto memory = map.GetRaw();

		REQUIRE(map.GetCount() == 5);
		//REQUIRE(map.GetReserved() >= 5);
		REQUIRE(map.KeyIs<Text>());
		REQUIRE(map.ValueIs<int>());
		REQUIRE_FALSE(map.KeyIs<int>());
		REQUIRE_FALSE(map.KeyIs<char>());
		REQUIRE_FALSE(map.ValueIs<float>());
		REQUIRE_FALSE(map.ValueIs<unsigned char>());
		//REQUIRE(map.GetRaw());
		REQUIRE(map["one"] == 1);
		REQUIRE(map["two"] == 2);
		REQUIRE(map["three"] == 3);
		REQUIRE(map["four"] == 4);
		REQUIRE(map["five"] == 5);

		WHEN("Shallow-copy more of the same stuff") {
			map << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(map.GetCount() == 10);
				REQUIRE(map["one"] == 1);
				REQUIRE(map["two"] == 2);
				REQUIRE(map["three"] == 3);
				REQUIRE(map["four"] == 4);
				REQUIRE(map["five"] == 5);
				REQUIRE(map["six"] == 6);
				REQUIRE(map["seven"] == 7);
				REQUIRE(map["eight"] == 8);
				REQUIRE(map["nine"] == 9);
				REQUIRE(map["ten"] == 10);

				/*REQUIRE(map.GetReserved() >= 10);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(map.GetRaw() == memory);
				#endif
				REQUIRE(map.Is<int>());*/
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 1:1
				BENCHMARK_ADVANCED("Anyness::THashMap::operator << (5 consecutive pair copies)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapType> storage(meter.runs());
					for (auto& i : storage)
						i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i] << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::insert(5 consecutive pair copies)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapTypeStd> storage(meter.runs());
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

				// Last result: 1:1
				BENCHMARK_ADVANCED("Anyness::THashMap::operator [] (retrieval by key from a map with 10 pairs)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapType> storage(meter.runs());
					for (auto& i : storage) {
						i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
						i << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
					}

					meter.measure([&](int i) {
						return storage[i]["seven"];
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::operator [] (retrieval by key from a map with 10 pairs)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapTypeStd> storage(meter.runs());
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
			map << Move(darray2[0]) << Move(darray2[1]) << Move(darray2[2]) << Move(darray2[3]) << Move(darray2[4]);

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(map.GetCount() == 10);
				REQUIRE(map["one"] == 1);
				REQUIRE(map["two"] == 2);
				REQUIRE(map["three"] == 3);
				REQUIRE(map["four"] == 4);
				REQUIRE(map["five"] == 5);
				REQUIRE(map["six"] == 6);
				REQUIRE(map["seven"] == 7);
				REQUIRE(map["eight"] == 8);
				REQUIRE(map["nine"] == 9);
				REQUIRE(map["ten"] == 10);

				/*REQUIRE(map.GetReserved() >= 10);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(map.GetRaw() == memory);
				#endif
				REQUIRE(map.Is<int>());*/
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 
				BENCHMARK_ADVANCED("Anyness::THashMap::operator << (5 consecutive trivial moves)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapType> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] << Move(darray2[0]) << Move(darray2[1]) << Move(darray2[2]) << Move(darray2[3]) << Move(darray2[4]);
					});
				};

				BENCHMARK_ADVANCED("std::unordered_map::emplace_back(5 consecutive trivial moves)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapTypeStd> storage(meter.runs());
					meter.measure([&](int i) {
						storage[i].emplace_back(Move(darray2[0]));
						storage[i].emplace_back(Move(darray2[1]));
						storage[i].emplace_back(Move(darray2[2]));
						storage[i].emplace_back(Move(darray2[3]));
						return storage[i].emplace_back(Move(darray2[4]));
					});
				};
			#endif
		}

		WHEN("The size is reduced by finding and removing elements by value, but reserved memory should remain the same on shrinking") {
			const auto removed2 = map.RemoveValue(2);
			const auto removed4 = map.RemoveValue(4);
			THEN("The size changes but not capacity") {
				REQUIRE(removed2 == 1);
				REQUIRE(removed4 == 1);
				REQUIRE(map["one"] == 1);
				REQUIRE(map["three"] == 3);
				REQUIRE(map["five"] == 5);
				REQUIRE_THROWS(map["two"] == 2);
				REQUIRE_THROWS(map["four"] == 4);
				REQUIRE(map.GetCount() == 3);
				/*REQUIRE(map.GetReserved() >= 5);
				REQUIRE(map.GetRaw() == memory);*/
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 
				BENCHMARK_ADVANCED("Anyness::TAny::Remove(single element by value)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapType> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].RemoveValue(2);
					});
				};

				BENCHMARK_ADVANCED("Anyness::vector::erase-remove(single element by value)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapTypeStd> storage(meter.runs());
					for (auto&& o : storage)
						o = {darray1[0], darray1[1], darray1[2], darray1[3], darray1[4]};

					meter.measure([&](int i) {
						// Erase-remove idiom											
						return storage[i].erase(std::remove(storage[i].begin(), storage[i].end(), 2), storage[i].end());
					});
				};
			#endif
		}

		WHEN("The size is reduced by finding and removing elements by key, but reserved memory should remain the same on shrinking") {
			const auto removed2 = map.RemoveKey("two");
			const auto removed4 = map.RemoveKey("four");
			THEN("The size changes but not capacity") {
				REQUIRE(removed2 == 1);
				REQUIRE(removed4 == 1);
				REQUIRE(map["one"] == 1);
				REQUIRE(map["three"] == 3);
				REQUIRE(map["five"] == 5);
				REQUIRE_THROWS(map["two"] == 2);
				REQUIRE_THROWS(map["four"] == 4);
				REQUIRE(map.GetCount() == 3);
				/*REQUIRE(map.GetReserved() >= 5);
				REQUIRE(map.GetRaw() == memory);*/
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 
				BENCHMARK_ADVANCED("Anyness::TAny::Remove(single element by value)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapType> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].RemoveValue(2);
					});
				};

				BENCHMARK_ADVANCED("Anyness::vector::erase-remove(single element by value)") (Catch::Benchmark::Chronometer meter) {
					std::vector<MapTypeStd> storage(meter.runs());
					for (auto&& o : storage)
						o = {darray1[0], darray1[1], darray1[2], darray1[3], darray1[4]};

					meter.measure([&](int i) {
						// Erase-remove idiom											
						return storage[i].erase(std::remove(storage[i].begin(), storage[i].end(), 2), storage[i].end());
					});
				};
			#endif
		}

		WHEN("Removing non-available elements") {
			const auto removed9 = map.RemoveValue(9);

			THEN("Nothing should change") {
				REQUIRE(removed9 == 0);
				REQUIRE(map["one"] == 1);
				REQUIRE(map["two"] == 2);
				REQUIRE(map["three"] == 3);
				REQUIRE(map["four"] == 4);
				REQUIRE(map["five"] == 5);
				REQUIRE(map.GetCount() == 5);
				/*REQUIRE(map.GetReserved() >= 5);
				REQUIRE(map.GetRaw() == memory);*/
			}
		}

		WHEN("More capacity is reserved") {
			map.Allocate(20);

			THEN("The capacity changes but not the size, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(map.GetCount() == 5);
				/*REQUIRE(map.GetReserved() >= 20);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(map.GetRaw() == memory);
				#endif*/
			}
		}

		WHEN("Less capacity is reserved") {
			map.Allocate(2);

			THEN("Capacity and count remain unchanged") {
				REQUIRE(map.GetCount() == 5);
				/*REQUIRE(map.GetReserved() >= 5);
				REQUIRE(map.GetRaw() == memory);*/
			}
		}

		WHEN("Map is cleared") {
			map.Clear();

			THEN("Size goes to zero, capacity and types are unchanged") {
				REQUIRE(map.GetCount() == 0);
				REQUIRE(map.IsAllocated());
				REQUIRE(map.KeyIs<Text>());
				REQUIRE(map.ValueIs<int>());
				REQUIRE(map.IsKeyTypeConstrained());
				REQUIRE(map.IsValueTypeConstrained());
				REQUIRE(map.IsEmpty());
				/*REQUIRE(map.GetReserved() >= 5);
				REQUIRE(map.GetRaw() == memory);*/
			}
		}

		WHEN("Map is reset") {
			map.Reset();

			THEN("Size and capacity goes to zero, types are unchanged") {
				REQUIRE(map.GetCount() == 0);
				REQUIRE_FALSE(map.IsAllocated());
				REQUIRE(map.KeyIs<Text>());
				REQUIRE(map.ValueIs<int>());
				REQUIRE(map.IsKeyTypeConstrained());
				REQUIRE(map.IsValueTypeConstrained());
				REQUIRE(map.IsEmpty());
			}
		}

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			WHEN("Pack is reset, then immediately allocated again") {
				map.Reset();
				map << int(6) << int(7) << int(8) << int(9) << int(10);

				THEN("Block manager should reuse the memory, if MANAGED_MEMORY feature is enabled") {
					REQUIRE(map.GetRaw() == memory);
				}
			}
		#endif

		WHEN("Map is shallow-copied") {
			auto copy = map;

			THEN("The new map should keep the state and refer to original data") {
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
			MapType moved = Move(map);

			THEN("The new pack should keep the state and data") {
				/*REQUIRE(map.GetRaw() == nullptr);
				REQUIRE(map.GetCount() == 0);
				REQUIRE(map.GetReserved() == 0);
				REQUIRE(map.IsTypeConstrained());
				REQUIRE(map.GetType() == moved.GetType());*/
			}
		}

		WHEN("Maps are compared") {
			MapType sameMap;
			sameMap << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
			MapType clonedMap {map.Clone()};
			MapType copiedMap {map};
			MapType differentMap1;
			differentMap1 << darray1[0] << darray1[0] << darray1[2] << darray1[3] << darray1[4];
			MapType differentMap2;
			differentMap2 << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4] << darray1[4];

			THEN("The comparisons should be adequate") {
				REQUIRE(map == sameMap);
				REQUIRE(map == clonedMap);
				REQUIRE(map == copiedMap);
				REQUIRE(map != differentMap1);
				REQUIRE(map != differentMap2);
			}
		}
	}

	GIVEN("Two THashMaps") {
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
				REQUIRE(pack1.GetReferences() == 2);
				REQUIRE(pack2.GetReferences() == 2);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
			}
		}

		WHEN("Shallow copy pack1 in pack2 and then reset pack1") {
			pack2 = pack1;
			pack1.Reset();

			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetReferences() == 1);
				REQUIRE_FALSE(pack1.GetRaw());
				REQUIRE(pack1.GetReserved() == 0);
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
			}
		}

		WHEN("Deep copy pack1 in pack2") {
			pack2 = pack1.Clone();

			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetReferences() == 1);
				REQUIRE(pack2.GetReferences() == 1);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(static_cast<Block&>(pack2) != memory2);
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
			}
		}

		WHEN("Deep copy pack1 in pack2, then reset pack1") {
			pack2 = pack1.Clone();
			const auto memory3 = static_cast<Block>(pack2);
			pack1.Reset();

			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE_FALSE(Allocator::Find(memory1.GetType(), memory1.GetRaw()));
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				REQUIRE(pack2.GetReferences() == 1);
				REQUIRE(memory3.GetReferences() == 1);
			}
		}
	}
}
