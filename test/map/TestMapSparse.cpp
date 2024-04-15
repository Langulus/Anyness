///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Text.hpp>
#include <Anyness/Trait.hpp>
#include <Anyness/TMap.hpp>
#include <Anyness/Map.hpp>
#include <unordered_map>
#include "../Common.hpp"


/// The main test for TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap       
/// containers, with all kinds of items, from sparse to dense, from trivial   
/// to complex, from flat to deep                                             
TEMPLATE_TEST_CASE(
   "Sparse TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap", "[map]",
   (MapPair<UnorderedMap, Traits::Count*, RT*>),

   (MapPair<TUnorderedMap<Text, int*>, Text, int*>),
   (MapPair<TUnorderedMap<Text, Trait*>, Text, Trait*>),
   (MapPair<TUnorderedMap<Text, Traits::Count*>, Text, Traits::Count*>),
   (MapPair<TUnorderedMap<Text, Any*>, Text, Any*>),
   (MapPair<TUnorderedMap<Text, RT*>, Text, RT*>),

   (MapPair<TUnorderedMap<Trait*, RT*>, Trait*, RT*>),
   (MapPair<TUnorderedMap<Traits::Count*, RT*>, Traits::Count*, RT*>),
   (MapPair<TUnorderedMap<Any*, RT*>, Any*, RT*>),
   (MapPair<TUnorderedMap<RT*, RT*>, RT*, RT*>),

   (MapPair<TOrderedMap<Text, int*>, Text, int*>),
   (MapPair<TOrderedMap<Text, Trait*>, Text, Trait*>),
   (MapPair<TOrderedMap<Text, Traits::Count*>, Text, Traits::Count*>),
   (MapPair<TOrderedMap<Text, Any*>, Text, Any*>),
   (MapPair<TOrderedMap<Text, RT*>, Text, RT*>),

   (MapPair<TOrderedMap<Trait*, RT*>, Trait*, RT*>),
   (MapPair<TOrderedMap<Traits::Count*, RT*>, Traits::Count*, RT*>),
   (MapPair<TOrderedMap<Any*, RT*>, Any*, RT*>),
   (MapPair<TOrderedMap<RT*, RT*>, RT*, RT*>),

   (MapPair<UnorderedMap, Text, int*>),
   (MapPair<UnorderedMap, Text, Trait*>),
   (MapPair<UnorderedMap, Text, Traits::Count*>),
   (MapPair<UnorderedMap, Text, Any*>),
   (MapPair<UnorderedMap, Text, RT*>),

   (MapPair<UnorderedMap, Trait*, RT*>),

   (MapPair<UnorderedMap, Any*, RT*>),
   (MapPair<UnorderedMap, RT*, RT*>),

   (MapPair<OrderedMap, Text, int*>),
   (MapPair<OrderedMap, Text, Trait*>),
   (MapPair<OrderedMap, Text, Traits::Count*>),
   (MapPair<OrderedMap, Text, Any*>),
   (MapPair<OrderedMap, Text, RT*>),

   (MapPair<OrderedMap, Trait*, RT*>),
   (MapPair<OrderedMap, Traits::Count*, RT*>),
   (MapPair<OrderedMap, Any*, RT*>),
   (MapPair<OrderedMap, RT*, RT*>)
) {
   IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

   static Allocator::State memoryState;

   using T = typename TestType::Container;
   using K = typename TestType::Key;
   using V = typename TestType::Value;
   using Pair = TPair<K, V>;
   using StdPair = ::std::pair<K, V>;

   const auto pair = CreatePair<Pair, K, V>("five hundred", 555);
   const auto pairMissing = CreatePair<Pair, K, V>("missing", 554);
   const auto stdpair = CreatePair<StdPair, K, V>("five hundred", 555);

   const Pair darray1[5] {
      CreatePair<Pair, K, V>("one", 1),
      CreatePair<Pair, K, V>("two", 2),
      CreatePair<Pair, K, V>("three", 3),
      CreatePair<Pair, K, V>("four", 4),
      CreatePair<Pair, K, V>("five", 5)
   };

   const Pair darray2[5] {
      CreatePair<Pair, K, V>("six", 6),
      CreatePair<Pair, K, V>("seven", 7),
      CreatePair<Pair, K, V>("eight", 8),
      CreatePair<Pair, K, V>("nine", 9),
      CreatePair<Pair, K, V>("ten", 10)
   };

   const StdPair darray1std[5] {
      CreatePair<StdPair, K, V>("one", 1),
      CreatePair<StdPair, K, V>("two", 2),
      CreatePair<StdPair, K, V>("three", 3),
      CreatePair<StdPair, K, V>("four", 4),
      CreatePair<StdPair, K, V>("five", 5)
   };

   const StdPair darray2std[5] {
      CreatePair<StdPair, K, V>("six", 6),
      CreatePair<StdPair, K, V>("seven", 7),
      CreatePair<StdPair, K, V>("eight", 8),
      CreatePair<StdPair, K, V>("nine", 9),
      CreatePair<StdPair, K, V>("ten", 10)
   };

   if constexpr (CT::Untyped<T>) {
      // All type-erased containers should have all semantic            
      // constructors and assigners available, and errors will instead  
      // be thrown as exceptions at runtime                             
      static_assert(CT::CopyMakable<T>);
      static_assert(CT::ReferMakable<T>);
      static_assert(CT::AbandonMakable<T>);
      static_assert(CT::MoveMakable<T>);
      static_assert(CT::CloneMakable<T>);
      static_assert(CT::DisownMakable<T>);

      static_assert(CT::CopyAssignable<T>);
      static_assert(CT::ReferAssignable<T>);
      static_assert(CT::AbandonAssignable<T>);
      static_assert(CT::MoveAssignable<T>);
      static_assert(CT::CloneAssignable<T>);
      static_assert(CT::DisownAssignable<T>);
   }


   GIVEN("A default-initialized map instance") {
      T map {};

      WHEN("Given a default-constructed map") {
         if constexpr (CT::Typed<T>) {
            REQUIRE(map.template IsKey<K>());
            REQUIRE(map.template IsValue<V>());
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
         }

         REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
         REQUIRE(!map);
         REQUIRE(map.GetUses() == 0);
         REQUIRE_FALSE(map.IsAllocated());
         REQUIRE_FALSE(map.HasAuthority());

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
         IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

         auto movablePair = pair;
         map = ::std::move(movablePair);

         if constexpr (CT::Dense<K> or CT::Dense<V>)
            REQUIRE(movablePair != pair);
         REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.template IsKey<K>());
         REQUIRE(map.template IsValue<V>());
         REQUIRE(map.IsAllocated());
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetCount() == 1);
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map[pair.mKey] == pair.mValue);

         if constexpr (CT::Text<K>) {
            REQUIRE(map["five hundred"] == pair.mValue);
            REQUIRE_THROWS(map["missing"] != pair.mValue);
         }
         else {
            REQUIRE(map[pair.mKey] == pair.mValue);
            REQUIRE_THROWS(map[pairMissing.mKey] != pair.mValue);
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
      T map {pair};

      WHEN("Given a pair-constructed map") {
         REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.template IsKey<K>());
         REQUIRE(map.template IsValue<V>());
         REQUIRE(map.IsAllocated());
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetCount() == 1);
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map[pair.mKey] == pair.mValue);

         if constexpr (CT::Text<K>) {
            REQUIRE(map["five hundred"] == pair.mValue);
            REQUIRE_THROWS(map["missing"] != pair.mValue);
         }
         else {
            REQUIRE(map[pair.mKey] == pair.mValue);
            REQUIRE_THROWS(map[pairMissing.mKey] != pair.mValue);
         }

         //TODO benchmark
      }
   }
   
   GIVEN("A pair array copy-initialized map instance") {
      T map {darray1};

      WHEN("Given a preinitialized map with 5 elements") {
         REQUIRE(map.GetCount() == 5);
         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.template IsKey<K>());
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.template IsValue<V>());
         REQUIRE_FALSE(map.template IsKey<int>());
         REQUIRE_FALSE(map.template IsKey<char>());
         REQUIRE_FALSE(map.template IsValue<float>());
         REQUIRE_FALSE(map.template IsValue<unsigned char>());
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);
         REQUIRE(map.GetReserved() >= 5);

         //TODO benchmark
      }
   }

   GIVEN("Map with some items") {
      T map {};
      map << darray1[0];
      map << darray1[1];
      map << darray1[2];
      map << darray1[3];
      map << darray1[4];

      auto keyMemory = map.GetRawKeysMemory();
      auto valueMemory = map.GetRawValsMemory();

      WHEN("Given a preinitialized map with 5 elements") {
         REQUIRE(map.GetCount() == 5);
         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.template IsKey<K>());
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.template IsValue<V>());
         REQUIRE_FALSE(map.template IsKey<int>());
         REQUIRE_FALSE(map.template IsKey<char>());
         REQUIRE_FALSE(map.template IsValue<float>());
         REQUIRE_FALSE(map.template IsValue<unsigned char>());
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);
         REQUIRE(map.GetReserved() >= 5);
      }

      /*WHEN("Create 2048 and then 4096 maps, and initialize them (weird corner case test)") {
         auto storage = new some<T>;
         storage->resize(2048);
         const void* prevKeys = nullptr;
         const void* prevValues = nullptr;

         for (auto& i : *storage) {
            i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
            if (prevKeys && prevValues) {
               REQUIRE(prevKeys != i.GetRawKeysMemory());
               REQUIRE(prevValues != i.GetRawValsMemory());
               REQUIRE(i == *(&i - 1));
            }

            prevKeys = i.GetRawKeysMemory();
            prevValues = i.GetRawValsMemory();

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
               REQUIRE(prevValues != i.GetRawValsMemory());
               REQUIRE(i == *(&i - 1));
            }

            prevKeys = i.GetRawKeysMemory();
            prevValues = i.GetRawValsMemory();

            REQUIRE(i.HasAuthority());
            REQUIRE(i.GetUses() == 1);
            REQUIRE(i.GetCount() == 5);
            REQUIRE(i.GetReserved() == 8);
            for (auto& comparer : darray1)
               REQUIRE(i[comparer.mKey] == comparer.mValue);
         }

         delete storage;
      }*/

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

         REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.template IsKey<K>());
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.template IsValue<V>());
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map.GetCount() == 10);
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);
         for (auto& comparer : darray2)
            REQUIRE(map[comparer.mKey] == comparer.mValue);
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            REQUIRE(map.GetRawKeysMemory() == keyMemory);
            REQUIRE(map.GetRawValsMemory() == valueMemory);
         #endif
         REQUIRE(map.GetReserved() >= 10);

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
            REQUIRE(map.GetRawValsMemory() == valueMemory);
         #endif
         REQUIRE(map.GetReserved() >= 10);

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

         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(removed2 == 1);
         REQUIRE(removed4 == 1);
         REQUIRE(map.GetCount() == 3);
         REQUIRE(map.GetRawKeysMemory() == keyMemory);
         REQUIRE(map.GetRawValsMemory() == valueMemory);
         REQUIRE(map.GetReserved() >= 5);

         REQUIRE(map.ContainsKey(darray1[0].mKey));
         REQUIRE_FALSE(map.ContainsKey(darray1[1].mKey));
         REQUIRE(map.ContainsKey(darray1[2].mKey));
         REQUIRE_FALSE(map.ContainsKey(darray1[3].mKey));
         REQUIRE(map.ContainsKey(darray1[4].mKey));

         REQUIRE(map.ContainsValue(darray1[0].mValue));
         REQUIRE_FALSE(map.ContainsValue(darray1[1].mValue));
         REQUIRE(map.ContainsValue(darray1[2].mValue));
         REQUIRE_FALSE(map.ContainsValue(darray1[3].mValue));
         REQUIRE(map.ContainsValue(darray1[4].mValue));

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

      for (int iii = 0; iii < 10; ++iii) {
      WHEN(std::string("Removing elements by key #") + std::to_string(iii)) {
         const auto removed2 = map.RemoveKey(darray1[1].mKey);
         const auto removed4 = map.RemoveKey(darray1[3].mKey);

         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(removed2 == 1);
         REQUIRE(removed4 == 1);
         REQUIRE(map.GetCount() == 3);
         REQUIRE(map.GetRawKeysMemory() == keyMemory);
         REQUIRE(map.GetRawValsMemory() == valueMemory);
         REQUIRE(map.GetReserved() >= 5);

         REQUIRE(map.ContainsKey(darray1[0].mKey));
         REQUIRE_FALSE(map.ContainsKey(darray1[1].mKey));
         REQUIRE(map.ContainsKey(darray1[2].mKey));
         REQUIRE_FALSE(map.ContainsKey(darray1[3].mKey));
         REQUIRE(map.ContainsKey(darray1[4].mKey));

         REQUIRE(map.ContainsValue(darray1[0].mValue));
         REQUIRE_FALSE(map.ContainsValue(darray1[1].mValue));
         REQUIRE(map.ContainsValue(darray1[2].mValue));
         REQUIRE_FALSE(map.ContainsValue(darray1[3].mValue));
         REQUIRE(map.ContainsValue(darray1[4].mValue));

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
      }

      WHEN("Removing non-available elements by value") {
         const auto removed9 = map.RemoveValue(darray2[3].mValue);

         REQUIRE(removed9 == 0);
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);
         REQUIRE(map.GetCount() == 5);
         REQUIRE(map.GetRawKeysMemory() == keyMemory);
         REQUIRE(map.GetRawValsMemory() == valueMemory);
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map.GetReserved() >= 5);

         REQUIRE(map.ContainsKey(darray1[0].mKey));
         REQUIRE(map.ContainsKey(darray1[1].mKey));
         REQUIRE(map.ContainsKey(darray1[2].mKey));
         REQUIRE(map.ContainsKey(darray1[3].mKey));
         REQUIRE(map.ContainsKey(darray1[4].mKey));

         REQUIRE(map.ContainsValue(darray1[0].mValue));
         REQUIRE(map.ContainsValue(darray1[1].mValue));
         REQUIRE(map.ContainsValue(darray1[2].mValue));
         REQUIRE(map.ContainsValue(darray1[3].mValue));
         REQUIRE(map.ContainsValue(darray1[4].mValue));
      }
      
      WHEN("Removing non-available elements by key") {
         const auto removed9 = map.RemoveKey(darray2[3].mKey);

         REQUIRE(removed9 == 0);
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);
         REQUIRE(map.GetCount() == 5);
         REQUIRE(map.GetRawKeysMemory() == keyMemory);
         REQUIRE(map.GetRawValsMemory() == valueMemory);
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map.GetReserved() >= 5);

         REQUIRE(map.ContainsKey(darray1[0].mKey));
         REQUIRE(map.ContainsKey(darray1[1].mKey));
         REQUIRE(map.ContainsKey(darray1[2].mKey));
         REQUIRE(map.ContainsKey(darray1[3].mKey));
         REQUIRE(map.ContainsKey(darray1[4].mKey));

         REQUIRE(map.ContainsValue(darray1[0].mValue));
         REQUIRE(map.ContainsValue(darray1[1].mValue));
         REQUIRE(map.ContainsValue(darray1[2].mValue));
         REQUIRE(map.ContainsValue(darray1[3].mValue));
         REQUIRE(map.ContainsValue(darray1[4].mValue));
      }
      
      WHEN("More capacity is reserved") {
         //TODO this causes the Leftover exception
         map.Reserve(20);

         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map.GetCount() == 5);
         REQUIRE(map.GetReserved() >= 20);
      }

      WHEN("Less capacity is reserved") {
         map.Reserve(2);

         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map.GetCount() == 5);
         REQUIRE(map.GetRawKeysMemory() == keyMemory);
         REQUIRE(map.GetRawValsMemory() == valueMemory);
         REQUIRE(map.GetReserved() >= 5);
      }

      WHEN("Map is cleared") {
         map.Clear();

         REQUIRE(map.GetCount() == 0);
         REQUIRE(map.IsAllocated());
         REQUIRE(map.GetKeyType()->template Is<K>());
         REQUIRE(map.GetValueType()->template Is<V>());
         REQUIRE(map.template IsKey<K>());
         REQUIRE(map.template IsValue<V>());
         REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
         REQUIRE(!map);
         REQUIRE(map.GetRawKeysMemory() == keyMemory);
         REQUIRE(map.GetRawValsMemory() == valueMemory);
         REQUIRE(map.HasAuthority());
         REQUIRE(map.GetUses() == 1);
         REQUIRE(map.GetReserved() >= 5);
      }

      WHEN("Map is reset") {
         map.Reset();

         REQUIRE(map.GetCount() == 0);
         REQUIRE_FALSE(map.IsAllocated());
         REQUIRE_FALSE(map.HasAuthority());
         if constexpr (CT::Typed<T>) {
            REQUIRE(map.template IsKey<K>());
            REQUIRE(map.template IsValue<V>());
            REQUIRE(map.GetKeyType()->template Is<K>());
            REQUIRE(map.GetValueType()->template Is<V>());
         }
         REQUIRE(map.IsKeyTypeConstrained() == CT::Typed<T>);
         REQUIRE(map.IsValueTypeConstrained() == CT::Typed<T>);
         REQUIRE(!map);
         REQUIRE(map.GetRawKeysMemory() != keyMemory);
         REQUIRE(map.GetRawValsMemory() != valueMemory);
         REQUIRE(map.GetUses() == 0);
      }

      WHEN("Map is shallow-copied") {
         auto copy = map;

         REQUIRE(copy == map);
         REQUIRE(copy.GetKeyType()->template Is<K>());
         REQUIRE(copy.GetValueType()->template Is<V>());
         REQUIRE(copy.IsAllocated());
         REQUIRE(copy.HasAuthority());
         REQUIRE(copy.GetUses() == 2);
         REQUIRE(copy.GetCount() == map.GetCount());
         REQUIRE(copy.GetCount() == 5);
         REQUIRE(copy.GetRawKeysMemory() == map.GetRawKeysMemory());
         REQUIRE(copy.GetRawValsMemory() == map.GetRawValsMemory());
         for (auto& comparer : darray1)
            REQUIRE(copy[comparer.mKey] == comparer.mValue);

         if constexpr (CT::Typed<T>) {
            for (auto& comparer : darray1)
               REQUIRE(&map[comparer.mKey] == &copy[comparer.mKey]);
         }
      }

      WHEN("Map is cloned") {
         if constexpr (CT::CloneMakable<K> and CT::CloneMakable<V>) {
            T clone = Langulus::Clone(map);

            REQUIRE((clone != map) == (CT::Sparse<K> or CT::Sparse<V>));
            REQUIRE(clone.GetKeyType()->template Is<K>());
            REQUIRE(clone.GetValueType()->template Is<V>());
            REQUIRE(clone.IsAllocated());
            REQUIRE(clone.HasAuthority());
            REQUIRE(clone.GetUses() == 1);
            REQUIRE(clone.GetCount() == map.GetCount());
            REQUIRE(clone.GetCount() == 5);
            REQUIRE(clone.GetRawKeysMemory() != map.GetRawKeysMemory());
            REQUIRE(clone.GetRawValsMemory() != map.GetRawValsMemory());
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
         else if constexpr (CT::Untyped<T>) {
            T clone;
            REQUIRE_THROWS(new (&clone) T {Langulus::Clone(map)});
         }
      }

      WHEN("Map is move-constructed") {
         T movable = map;
         T moved = ::std::move(movable);

         REQUIRE(moved == map);
         REQUIRE(moved != movable);
         REQUIRE(moved.GetKeyType()->template Is<K>());
         REQUIRE(moved.GetValueType()->template Is<V>());
         REQUIRE(moved.GetRawKeysMemory() == keyMemory);
         REQUIRE(moved.GetRawValsMemory() == valueMemory);
         REQUIRE(moved.IsAllocated());
         REQUIRE(moved.GetCount() == 5);
         REQUIRE(moved.HasAuthority());
         REQUIRE(moved.GetUses() == 2);
         for (auto& comparer : darray1)
            REQUIRE(moved[comparer.mKey] == comparer.mValue);
         REQUIRE_FALSE(movable.IsAllocated());
         REQUIRE(!movable);
         REQUIRE(movable.GetRawValsMemory() == nullptr);
         REQUIRE(movable.GetCount() == 0);
         REQUIRE(movable.IsValueTypeConstrained() == CT::Typed<T>);
         REQUIRE(movable.IsKeyTypeConstrained() == CT::Typed<T>);
      }

      WHEN("Maps are compared") {
         T sameMap;
         sameMap << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
         T copiedMap {map};
         T differentMap1;
         differentMap1 << darray1[0] << darray1[0] << darray1[2] << darray1[3] << darray1[4];

         REQUIRE(map == sameMap);
         REQUIRE(map == copiedMap);
         REQUIRE(map != differentMap1);

         if constexpr (CT::CloneMakable<K> and CT::CloneMakable<V>) {
            T clonedMap {Clone(map)};
            REQUIRE((map != clonedMap) == (CT::Sparse<K> or CT::Sparse<V>));
         }
      }

      WHEN("Maps are iterated with ranged-for") {
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         unsigned i = 0;
         for (auto pair : map) {
            static_assert(CT::Untyped<T> or CT::Reference<decltype(pair.mKey)>,
               "Pair key type is not a reference for statically optimized map");
            static_assert(CT::Untyped<T> or CT::Reference<decltype(pair.mValue)>,
               "Pair value type is not a reference for statically optimized map");

            // Different architectures result in different hashes       
            if constexpr (CT::Dense<K>) {
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
            }
            else {
               //TODO
            }

            ++i;
         }

         REQUIRE(i == map.GetCount());
      }

      WHEN("ForEach flat dense key (immutable)") {
         for (auto& comparer : darray1)
            REQUIRE(map[comparer.mKey] == comparer.mValue);

         unsigned i = 0;
         const auto done = map.ForEachKey([&](const K& key) {
            // Different architectures result in different hashes       
            if constexpr (CT::Dense<K>) {
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
            }
            else {
               //TODO
            }

            ++i;
            return true;
         });

         THEN("The comparisons should be adequate") {
            REQUIRE(i == map.GetCount());
            REQUIRE(i == done);
         }
      }
   }

   DestroyPair(pair);
   DestroyPair(pairMissing);
   DestroyPair(stdpair);

   for (auto& i : darray1)
      DestroyPair(i);
   for (auto& i : darray2)
      DestroyPair(i);
   for (auto& i : darray1std)
      DestroyPair(i);
   for (auto& i : darray2std)
      DestroyPair(i);

   REQUIRE(memoryState.Assert());
}
