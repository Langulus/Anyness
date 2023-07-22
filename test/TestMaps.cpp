///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <Anyness/Text.hpp>
#include <Anyness/Trait.hpp>
#include <Anyness/TPair.hpp>
#include <Anyness/TUnorderedMap.hpp>
#include <Anyness/TMap.hpp>
#include <Anyness/UnorderedMap.hpp>
#include <Anyness/Map.hpp>
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

template<class K, class V>
struct TypePair2 {
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

/// Cross-container consistency tests                                         
TEMPLATE_TEST_CASE(
   "Cross-container consistency tests for TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap", "[map]",
   (TypePair2<Text, Trait>),
   (TypePair2<Text, Any>),
   (TypePair2<Text, int>),
   (TypePair2<Text, Traits::Count>),
   (TypePair2<Text, int*>),
   (TypePair2<Text, Trait*>),
   (TypePair2<Text, Traits::Count*>),
   (TypePair2<Text, Any*>)
) {
   GIVEN("A single element initialized maps of all kinds") {
      using K = typename TestType::Key;
      using V = typename TestType::Value;

      const auto pair = CreatePair<TPair<K, V>, K, V>(
         "five hundred"_text, 555);

      TUnorderedMap<K, V> uset1 {pair};
      UnorderedMap uset2 {pair};
      TOrderedMap<K, V> oset1 {pair};
      OrderedMap oset2 {pair};

      WHEN("Their hashes are taken") {
         const auto elementHash = HashOf(pair);

         const auto uhash1 = uset1.GetHash();
         const auto uhash2 = uset2.GetHash();
         const auto ohash1 = oset1.GetHash();
         const auto ohash2 = oset2.GetHash();

         THEN("These hashes should all be the same as the element") {
            REQUIRE(uhash1 == uhash2);
            REQUIRE(ohash1 == ohash2);
            REQUIRE(uhash1 == ohash1);
            REQUIRE(uhash1 == elementHash);
         }
      }
   }
}

/// The main test for TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap       
/// containers, with all kinds of items, from sparse to dense, from trivial   
/// to complex, from flat to deep                                             
TEMPLATE_TEST_CASE(
   "TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap", "[map]",
   (TypePair<UnorderedMap, Text, int>),
   (TypePair<TUnorderedMap<Text, int>, Text, int>),
   (TypePair<TUnorderedMap<Text, Trait>, Text, Trait>),
   (TypePair<TUnorderedMap<Text, Traits::Count>, Text, Traits::Count>),
   (TypePair<TUnorderedMap<Text, Any>, Text, Any>),
   (TypePair<TUnorderedMap<Text, int*>, Text, int*>),
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
   (TypePair<UnorderedMap, Text, Trait>),
   (TypePair<UnorderedMap, Text, Traits::Count>),
   (TypePair<UnorderedMap, Text, Any>),
   (TypePair<UnorderedMap, Text, int*>),
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
   using K = typename TestType::Key;
   using V = typename TestType::Value;
   using Pair = TPair<K, V>;
   using StdPair = ::std::pair<K, V>;

   GIVEN("A default-initialized map instance") {
      const auto pair = CreatePair<Pair, K, V>(
         "five hundred"_text, 555);
      const auto stdpair = CreatePair<StdPair, K, V>(
         "five hundred"_text, 555);

      T map {};

      WHEN("Given a default-constructed map") {
         THEN("These properties should be correct") {
            if constexpr (CT::Typed<T>) {
               REQUIRE(map.template KeyIs<K>());
               REQUIRE(map.template ValueIs<V>());
               REQUIRE(map.GetKeyType()->template Is<K>());
               REQUIRE(map.GetValueType()->template Is<V>());
            }

            REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
            REQUIRE(!map);
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

      WHEN("Assigned a pair by move") {
         IF_LANGULUS_MANAGED_MEMORY(Fractalloc.CollectGarbage());

         auto movablePair = pair;
         map = ::std::move(movablePair);

         THEN("Various traits change") {
            REQUIRE(movablePair != pair);
            REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.GetValueType()->template Is<V>());
            REQUIRE(map.template KeyIs<K>());
            REQUIRE(map.template ValueIs<V>());
            REQUIRE(map.IsAllocated());
            REQUIRE(map.HasAuthority());
            REQUIRE(map.GetCount() == 1);
            REQUIRE(map.GetUses() == 1);
            REQUIRE(map[pair.mKey] == pair.mValue);
            REQUIRE(map["missing"_text] != pair.mValue);
         }

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::TUnorderedMap::operator = (single pair copy)") (timer meter) {
               some<Pair> source(meter.runs());
               for (auto& i : source)
                  i = CreatePair<Pair, K, V>("five hundred"_text, 555);
                  
               some<MapType> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i] = ::std::move(source[i]);
               });
            };

            BENCHMARK_ADVANCED("std::unordered_map::insert(single pair copy)") (timer meter) {
               some<StdPair> source(meter.runs());
               for(auto& i : source)
                  i = valueStd;

               some<MapTypeStd> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i].emplace(::std::move(source[i]));
               });
            };
         #endif
      }
   }
   
   GIVEN("A pair copy-initialized map instance") {
      const auto pair = CreatePair<Pair, K, V>(
         "five hundred"_text, 555);
      const auto stdpair = CreatePair<StdPair, K, V>(
         "five hundred"_text, 555);

      T map {pair};

      WHEN("Given a pair-constructed map") {
         THEN("These properties should be correct") {
            REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.GetValueType()->template Is<V>());
            REQUIRE(map.template KeyIs<K>());
            REQUIRE(map.template ValueIs<V>());
            REQUIRE(map.IsAllocated());
            REQUIRE(map.HasAuthority());
            REQUIRE(map.GetCount() == 1);
            REQUIRE(map.GetUses() == 1);
            REQUIRE(map[pair.mKey] == pair.mValue);
            REQUIRE(map["missing"_text] != pair.mValue);
         }

         //TODO benchmark
      }
   }
   
   GIVEN("A pair array copy-initialized map instance") {
      const Pair darray1[5] {
         CreatePair<Pair, K, V>("one"_text, 1),
         CreatePair<Pair, K, V>("two"_text, 2),
         CreatePair<Pair, K, V>("three"_text, 3),
         CreatePair<Pair, K, V>("four"_text, 4),
         CreatePair<Pair, K, V>("five"_text, 5)
      };

      const StdPair darray1std[5] {
         CreatePair<StdPair, K, V>("one"_text, 1),
         CreatePair<StdPair, K, V>("two"_text, 2),
         CreatePair<StdPair, K, V>("three"_text, 3),
         CreatePair<StdPair, K, V>("four"_text, 4),
         CreatePair<StdPair, K, V>("five"_text, 5)
      };

      T map {darray1};

      WHEN("Given a preinitialized map with 5 elements") {
         THEN("These properties should be correct") {
            REQUIRE(map.GetCount() == 5);
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.template KeyIs<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
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

         //TODO benchmark
      }
   }

   GIVEN("Map with some items") {
      IF_LANGULUS_MANAGED_MEMORY(Fractalloc.CollectGarbage());

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

      T map {};
      map << darray1[0];
      map << darray1[1];
      map << darray1[2];
      map << darray1[3];
      map << darray1[4];

      auto keyMemory = map.GetRawKeysMemory();
      auto valueMemory = map.GetRawValuesMemory();

      WHEN("Given a preinitialized map with 5 elements") {
         THEN("These properties should be correct") {
            REQUIRE(map.GetCount() == 5);
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.template KeyIs<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
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
         auto storage = new some<T>;
         storage->resize(2048);
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
         storage = new some<T>;
         storage->resize(4096);

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

         map << darray2[0];
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         map << darray2[1];
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         map << darray2[2];
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         map << darray2[3];
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         map << darray2[4];
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
            REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.template KeyIs<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
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

         map
            << ::std::move(movableDarray2[0])
            << ::std::move(movableDarray2[1])
            << ::std::move(movableDarray2[2])
            << ::std::move(movableDarray2[3])
            << ::std::move(movableDarray2[4]);

         THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
            REQUIRE(map.HasAuthority());
            REQUIRE(map.GetUses() == 1);
            REQUIRE(map.GetCount() == 10);
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
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
                  return storage[i] 
                     << ::std::move(darray2[0]) 
                     << ::std::move(darray2[1]) 
                     << ::std::move(darray2[2]) 
                     << ::std::move(darray2[3]) 
                     << ::std::move(darray2[4]);
               });
            };

            BENCHMARK_ADVANCED("std::unordered_map::emplace_back(5 consecutive trivial moves)") (timer meter) {
               some<MapTypeStd> storage(meter.runs());
               meter.measure([&](int i) {
                  storage[i].emplace(::std::move(darray2std[0]));
                  storage[i].emplace(::std::move(darray2std[1]));
                  storage[i].emplace(::std::move(darray2std[2]));
                  storage[i].emplace(::std::move(darray2std[3]));
                  return storage[i].emplace(::std::move(darray2std[4]));
               });
            };
         #endif
      }

      WHEN("Removing elements by value") {
         const auto removed2 = map.RemoveValue(darray1[1].mValue);
         const auto removed4 = map.RemoveValue(darray1[3].mValue);

         THEN("The size changes but not capacity") {
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
            REQUIRE(map.HasAuthority());
            REQUIRE(map.GetUses() == 1);
            REQUIRE(removed2 == 1);
            REQUIRE(removed4 == 1);
            REQUIRE(map.GetCount() == 3);
            REQUIRE(map.GetRawKeysMemory() == keyMemory);
            REQUIRE(map.GetRawValuesMemory() == valueMemory);
            REQUIRE(map.GetReserved() >= 5);
            REQUIRE(map[darray1[0].mKey] == darray1[0].mValue);
            REQUIRE(map[darray1[1].mKey] != darray1[1].mValue);
            REQUIRE(map[darray1[2].mKey] == darray1[2].mValue);
            REQUIRE(map[darray1[3].mKey] != darray1[3].mValue);
            REQUIRE(map[darray1[4].mKey] == darray1[4].mValue);
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

      WHEN("Removing elements by key") {
         const auto removed2 = map.RemoveKey(darray1[1].mKey);
         const auto removed4 = map.RemoveKey(darray1[3].mKey);

         THEN("The size changes but not capacity") {
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
            REQUIRE(map.HasAuthority());
            REQUIRE(map.GetUses() == 1);
            REQUIRE(removed2 == 1);
            REQUIRE(removed4 == 1);
            REQUIRE(map.GetCount() == 3);
            REQUIRE(map.GetRawKeysMemory() == keyMemory);
            REQUIRE(map.GetRawValuesMemory() == valueMemory);
            REQUIRE(map.GetReserved() >= 5);
            REQUIRE(map[darray1[0].mKey] == darray1[0].mValue);
            REQUIRE(map[darray1[1].mKey] != darray1[1].mValue);
            REQUIRE(map[darray1[2].mKey] == darray1[2].mValue);
            REQUIRE(map[darray1[3].mKey] != darray1[3].mValue);
            REQUIRE(map[darray1[4].mKey] == darray1[4].mValue);
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

      WHEN("Removing non-available elements by value") {
         const auto removed9 = map.RemoveValue(darray2[3].mValue);

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
      
      WHEN("Removing non-available elements by key") {
         const auto removed9 = map.RemoveKey(darray2[3].mKey);

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
         map.Reserve(20);

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
         map.Reserve(2);

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
         map.Clear();

         THEN("Size goes to zero, capacity and types are unchanged") {
            REQUIRE(map.GetCount() == 0);
            REQUIRE(map.IsAllocated());
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
            REQUIRE(map.template KeyIs<K>());
            REQUIRE(map.template ValueIs<V>());
            REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
            REQUIRE(!map);
            REQUIRE(map.GetRawKeysMemory() == keyMemory);
            REQUIRE(map.GetRawValuesMemory() == valueMemory);
            REQUIRE(map.HasAuthority());
            REQUIRE(map.GetUses() == 1);
            REQUIRE(map.GetReserved() >= 5);
         }
      }

      WHEN("Map is reset") {
         map.Reset();

         THEN("Size and capacity goes to zero, types are unchanged if statically optimized") {
            REQUIRE(map.GetCount() == 0);
            REQUIRE_FALSE(map.IsAllocated());
            REQUIRE_FALSE(map.HasAuthority());
            if constexpr (CT::Typed<T>) {
               REQUIRE(map.template KeyIs<K>());
               REQUIRE(map.template ValueIs<V>());
               REQUIRE(map.GetKeyType()->template Is<K>());
               REQUIRE(map.GetValueType()->template Is<V>());
            }
            REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
            REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
            REQUIRE(!map);
            REQUIRE(map.GetRawKeysMemory() != keyMemory);
            REQUIRE(map.GetRawValuesMemory() != valueMemory);
            REQUIRE(map.GetUses() == 0);
         }
      }

      WHEN("Map is shallow-copied") {
         auto copy = map;

         THEN("The new map should keep the state and refer to original data") {
            REQUIRE(copy == map);
            REQUIRE(copy.GetKeyType()->template Is<K>());
            REQUIRE(copy.GetValueType()->template Is<V>());
            REQUIRE(copy.IsAllocated());
            REQUIRE(copy.HasAuthority());
            REQUIRE(copy.GetUses() == 2);
            REQUIRE(copy.GetCount() == map.GetCount());
            REQUIRE(copy.GetCount() == 5);
            REQUIRE(copy.GetRawKeysMemory() == map.GetRawKeysMemory());
            REQUIRE(copy.GetRawValuesMemory() == map.GetRawValuesMemory());
            for (auto& comparer : darray1)
               REQUIRE(copy[comparer.mKey] == comparer.mValue);

            if constexpr (CT::Typed<T>) {
               for (auto& comparer : darray1)
                  REQUIRE(&map[comparer.mKey] == &copy[comparer.mKey]);
            }
         }
      }

      WHEN("Map is cloned") {
         T clone = Langulus::Clone(map);

         THEN("The new map should keep the state, but refer to new data") {
            REQUIRE((clone != map) == (CT::Sparse<K> || CT::Sparse<V>));
            REQUIRE(clone.GetKeyType()->template Is<K>());
            REQUIRE(clone.GetValueType()->template Is<V>());
            REQUIRE(clone.IsAllocated());
            REQUIRE(clone.HasAuthority());
            REQUIRE(clone.GetUses() == 1);
            REQUIRE(clone.GetCount() == map.GetCount());
            REQUIRE(clone.GetCount() == 5);
            REQUIRE(clone.GetRawKeysMemory() != map.GetRawKeysMemory());
            REQUIRE(clone.GetRawValuesMemory() != map.GetRawValuesMemory());
            for (auto& comparer : darray1) {
               if constexpr (CT::Sparse<V>) {
                  REQUIRE(clone[comparer.mKey] != comparer.mValue);
                  REQUIRE(map[comparer.mKey] != clone[comparer.mKey]);
               }
               else {
                  REQUIRE(clone[comparer.mKey] == comparer.mValue);
                  REQUIRE(map[comparer.mKey] == clone[comparer.mKey]);
               }
               
               REQUIRE(map[comparer.mKey] == comparer.mValue);
               
               if constexpr (CT::Typed<T>)
                  REQUIRE(&map[comparer.mKey] != &clone[comparer.mKey]);
               else
                  REQUIRE(map[comparer.mKey].GetRaw() != clone[comparer.mKey].GetRaw());
            }
         }
      }

      WHEN("Map is move-constructed") {
         T movable = map;
         T moved = ::std::move(movable);

         THEN("The new pack should keep the state and data") {
            REQUIRE(moved == map);
            REQUIRE(moved != movable);
            REQUIRE(moved.GetKeyType()->template Is<K>());
            REQUIRE(moved.GetValueType()->template Is<V>());
            REQUIRE(moved.GetRawKeysMemory() == keyMemory);
            REQUIRE(moved.GetRawValuesMemory() == valueMemory);
            REQUIRE(moved.IsAllocated());
            REQUIRE(moved.GetCount() == 5);
            REQUIRE(moved.HasAuthority());
            REQUIRE(moved.GetUses() == 2);
            for (auto& comparer : darray1)
               REQUIRE(moved[comparer.mKey] == comparer.mValue);
            REQUIRE_FALSE(movable.IsAllocated());
            REQUIRE(!movable);
            REQUIRE(movable.GetRawValuesMemory() == nullptr);
            REQUIRE(movable.GetCount() == 0);
            REQUIRE(movable.IsValueTypeConstrained() == CT::Typed<T>);
            REQUIRE(movable.IsKeyTypeConstrained() == CT::Typed<T>);
         }
      }

      WHEN("Maps are compared") {
         T sameMap;
         sameMap << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
         T clonedMap {Clone(map)};
         T copiedMap {map};
         T differentMap1;
         differentMap1 << darray1[0] << darray1[0] << darray1[2] << darray1[3] << darray1[4];

         THEN("The comparisons should be adequate") {
            REQUIRE(map == sameMap);
            REQUIRE((map != clonedMap) == (CT::Sparse<K> || CT::Sparse<V>));
            REQUIRE(map == copiedMap);
            REQUIRE(map != differentMap1);
         }
      }

      WHEN("Maps are iterated with ranged-for") {
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         unsigned i = 0;
         for (auto pair : map) {
            static_assert(!CT::Typed<T> || ::std::is_reference_v<decltype(pair.mKey)>,
               "Pair key type is not a reference for statically optimized map");
            static_assert(!CT::Typed<T> || ::std::is_reference_v<decltype(pair.mValue)>,
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

      WHEN("ForEach flat dense key (immutable)") {
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         unsigned i = 0;
         const auto done = map.ForEachKey([&](const K& key) {
            // Different architectures result in different hashes       
            if constexpr (Bitness == 32) {
               switch (i) {
               case 0:
                  REQUIRE(key == darray1[2].mKey);
                  break;
               case 1:
                  REQUIRE(key == darray1[3].mKey);
                  break;
               case 2:
                  REQUIRE(key == darray1[1].mKey);
                  break;
               case 3:
                  REQUIRE(key == darray1[4].mKey);
                  break;
               case 4:
                  REQUIRE(key == darray1[0].mKey);
                  break;
               default:
                  FAIL("Index out of bounds in ranged-for");
                  break;
               }
            }
            else if constexpr (Bitness == 64) {
               switch (i) {
               case 0:
                  REQUIRE(key == darray1[1].mKey);
                  break;
               case 1:
                  REQUIRE(key == darray1[2].mKey);
                  break;
               case 2:
                  REQUIRE(key == darray1[3].mKey);
                  break;
               case 3:
                  REQUIRE(key == darray1[4].mKey);
                  break;
               case 4:
                  REQUIRE(key == darray1[0].mKey);
                  break;
               default:
                  FAIL("Index out of bounds in ranged-for");
                  break;
               }
            }
            else return false;

            ++i;
            return true;
         });

         THEN("The comparisons should be adequate") {
            REQUIRE(i == map.GetCount());
            REQUIRE(i == done);
         }
      }
   }

   /*GIVEN("Two maps") {
      IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

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
         pack2 = Clone(pack1);

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
         pack2 = Clone(pack1);
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
   }*/
}

struct VulkanLayer {};
struct VulkanRenderer {};
struct VulkanCamera {};
struct Platform {};
struct Vulkan {};
struct Window {};
struct VulkanLight {};
struct Monitor {};
struct VulkanRenderable {};
struct Cursor {};

/// Testing some corner cases encountered during the use of the container     
TEMPLATE_TEST_CASE("Map corner cases", "[map]",
   (TypePair<UnorderedMap, DMeta, Text>),
   (TypePair<TUnorderedMap<DMeta, Text>, DMeta, Text>),
   (TypePair<TOrderedMap<DMeta, Text>, DMeta, Text>),
   (TypePair<OrderedMap, DMeta, Text>)
) {
   using T = typename TestType::Container;
   using K = typename TestType::Key;
   using V = typename TestType::Value;
   using Pair = TPair<K, V>;

   GIVEN("Map instance initialized with 10 specific pairs for the corner case") {
      const Pair pairs[10] = {
         {MetaData::Of<VulkanLayer>(), "VulkanLayer"},
         {MetaData::Of<VulkanRenderer>(), "VulkanRenderer"},
         {MetaData::Of<VulkanCamera>(), "VulkanCamera"},
         {MetaData::Of<Platform>(), "Platform"},
         {MetaData::Of<Vulkan>(), "Vulkan"},
         {MetaData::Of<Window>(), "Window"},
         {MetaData::Of<VulkanLight>(), "VulkanLight"},
         {MetaData::Of<Monitor>(), "Monitor"},
         {MetaData::Of<VulkanRenderable>(), "VulkanRenderable"},
         {MetaData::Of<Cursor>(), "Cursor"}
      };

      T map {pairs};

      WHEN("Removing around-the-end elements by value (corner case)") {
         Count removed {};
         removed += map.RemoveValue("VulkanRenderer"_text);
         removed += map.RemoveValue("VulkanCamera"_text);
         removed += map.RemoveValue("Vulkan"_text);
         removed += map.RemoveValue("VulkanRenderable"_text);
         removed += map.RemoveValue("VulkanLight"_text);
         removed += map.RemoveValue("VulkanLayer"_text);

         THEN("The map should be correct") {
            REQUIRE(removed == 6);
            REQUIRE(map.GetCount() == 4);
            REQUIRE(map[MetaData::Of<VulkanLayer>()] == ""_text);
            REQUIRE(map[MetaData::Of<VulkanRenderer>()] == ""_text);
            REQUIRE(map[MetaData::Of<VulkanCamera>()] == ""_text);
            REQUIRE(map[MetaData::Of<Platform>()] == "Platform"_text);
            REQUIRE(map[MetaData::Of<Vulkan>()] == ""_text);
            REQUIRE(map[MetaData::Of<Window>()] == "Window"_text);
            REQUIRE(map[MetaData::Of<VulkanLight>()] == ""_text);
            REQUIRE(map[MetaData::Of<Monitor>()] == "Monitor"_text);
            REQUIRE(map[MetaData::Of<VulkanRenderable>()] == ""_text);
            REQUIRE(map[MetaData::Of<Cursor>()] == "Cursor"_text);
         }
      }

      WHEN("Removing around-the-end elements by key (corner case)") {
         Count removed {};
         removed += map.RemoveKey(MetaData::Of<VulkanRenderer>());
         removed += map.RemoveKey(MetaData::Of<VulkanCamera>());
         removed += map.RemoveKey(MetaData::Of<Vulkan>());
         removed += map.RemoveKey(MetaData::Of<VulkanRenderable>());
         removed += map.RemoveKey(MetaData::Of<VulkanLight>());
         removed += map.RemoveKey(MetaData::Of<VulkanLayer>());

         THEN("The map should be correct") {
            REQUIRE(removed == 6);
            REQUIRE(map.GetCount() == 4);
            REQUIRE(map[MetaData::Of<VulkanLayer>()] == ""_text);
            REQUIRE(map[MetaData::Of<VulkanRenderer>()] == ""_text);
            REQUIRE(map[MetaData::Of<VulkanCamera>()] == ""_text);
            REQUIRE(map[MetaData::Of<Platform>()] == "Platform"_text);
            REQUIRE(map[MetaData::Of<Vulkan>()] == ""_text);
            REQUIRE(map[MetaData::Of<Window>()] == "Window"_text);
            REQUIRE(map[MetaData::Of<VulkanLight>()] == ""_text);
            REQUIRE(map[MetaData::Of<Monitor>()] == "Monitor"_text);
            REQUIRE(map[MetaData::Of<VulkanRenderable>()] == ""_text);
            REQUIRE(map[MetaData::Of<Cursor>()] == "Cursor"_text);
         }
      }
   }
}
