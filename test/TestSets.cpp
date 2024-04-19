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
#include <Anyness/TSet.hpp>
#include <Anyness/Set.hpp>
#include <unordered_set>
#include "Common.hpp"


/// The main test for TOrderedSet/TUnorderedSet/OrderedSet/UnorderedSet       
/// containers, with all kinds of items, from sparse to dense, from trivial   
/// to complex, from flat to deep                                             
TEMPLATE_TEST_CASE(
   "TOrderedSet/TUnorderedSet/OrderedSet/UnorderedSet", "[set]",
   (TypePair<TUnorderedSet<int*>, int*>),
   (TypePair<TUnorderedSet<Trait*>, Trait*>),
   (TypePair<TUnorderedSet<Traits::Count*>, Traits::Count*>),
   (TypePair<TUnorderedSet<Any*>, Any*>),
   (TypePair<TUnorderedSet<RT*>, RT*>),

   (TypePair<TOrderedSet<int*>, int*>),
   (TypePair<TOrderedSet<Trait*>, Trait*>),
   (TypePair<TOrderedSet<Traits::Count*>, Traits::Count*>),
   (TypePair<TOrderedSet<Any*>, Any*>),
   (TypePair<TOrderedSet<RT*>, RT*>),

   (TypePair<UnorderedSet, int*>),
   (TypePair<UnorderedSet, Trait*>),
   (TypePair<UnorderedSet, Traits::Count*>),
   (TypePair<UnorderedSet, Any*>),
   (TypePair<UnorderedSet, RT*>),

   (TypePair<OrderedSet, int*>),
   (TypePair<OrderedSet, Trait*>),
   (TypePair<OrderedSet, Traits::Count*>),
   (TypePair<OrderedSet, Any*>),
   (TypePair<OrderedSet, RT*>),

   (TypePair<TUnorderedSet<int>, int>),
   (TypePair<TUnorderedSet<Trait>, Trait>),
   (TypePair<TUnorderedSet<Traits::Count>, Traits::Count>),
   (TypePair<TUnorderedSet<Any>, Any>),

   (TypePair<TOrderedSet<int>, int>),
   (TypePair<TOrderedSet<Trait>, Trait>),
   (TypePair<TOrderedSet<Traits::Count>, Traits::Count>),
   (TypePair<TOrderedSet<Any>, Any>),

   (TypePair<UnorderedSet, int>),
   (TypePair<UnorderedSet, Trait>),
   (TypePair<UnorderedSet, Traits::Count>),
   (TypePair<UnorderedSet, Any>),

   (TypePair<OrderedSet, int>),
   (TypePair<OrderedSet, Trait>),
   (TypePair<OrderedSet, Traits::Count>),
   (TypePair<OrderedSet, Any>)
) {
   static Allocator::State memoryState;

   using T = typename TestType::LHS;
   using K = typename TestType::RHS;

   const auto element = CreateElement<K>(555);

   // Arrays are dynamic to avoid constexprification                 
   const K darray1[5] {
      CreateElement<K>(1),
      CreateElement<K>(2),
      CreateElement<K>(3),
      CreateElement<K>(4),
      CreateElement<K>(5)
   };
   const K darray2[5] {
      CreateElement<K>(6),
      CreateElement<K>(7),
      CreateElement<K>(8),
      CreateElement<K>(9),
      CreateElement<K>(10)
   };

   GIVEN("A default-initialized set instance") {
      T set;

      WHEN("Given a default-constructed map") {
         if constexpr (CT::Typed<T>) {
            REQUIRE(set.template Is<K>());
            REQUIRE(set.GetType()->template Is<K>());
         }

         REQUIRE(set.IsTypeConstrained() == CT::Typed<T>);
         REQUIRE(!set);
         REQUIRE(set.GetUses() == 0);
         REQUIRE_FALSE(set.IsAllocated());
         REQUIRE_FALSE(set.HasAuthority());

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::set::default construction") (timer meter) {
               some<uninitialized<MapType>> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i].construct();
               });
            };

            BENCHMARK_ADVANCED("std::set::default construction") (timer meter) {
               some<uninitialized<MapTypeStd>> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i].construct();
               });
            };
         #endif
      }

      WHEN("Assigned a value by move") {
         T movable = element;
         set = ::std::move(movable);

         REQUIRE(movable != element);
         REQUIRE(set.IsTypeConstrained() == CT::Typed<T>);
         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.template Is<K>());
         REQUIRE(set.IsAllocated());
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetCount() == 1);
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set[0] == element);

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::TUnorderedSet::operator = (single element copy)") (timer meter) {
               some<Pair> source(meter.runs());
               for (auto& i : source)
                  i = CreatePair<Pair, K, V>("five hundred"_text, 555);
                  
               some<MapType> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i] = ::std::move(source[i]);
               });
            };

            BENCHMARK_ADVANCED("std::unordered_set::insert(single element copy)") (timer meter) {
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

   GIVEN("A copy-initialized set instance") {
      WHEN("Given an element-constructed set") {
         T set {element};

         REQUIRE(set.IsTypeConstrained() == CT::Typed<T>);
         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.template Is<K>());
         REQUIRE(set.IsAllocated());
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetCount() == 1);
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set.Contains(element));

         //TODO benchmark
      }
   }
   
   GIVEN("Five elements") {
      WHEN("Unfold-initialized using the five elements as an array") {
         T set {darray1};

         REQUIRE(set.GetCount() == 5);
         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.template Is<K>());
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         REQUIRE(set.GetReserved() >= 5);

         //TODO benchmark
      }
   }

   GIVEN("Set with some items") {
      T set;
      set << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
      auto memory = set.GetRawMemory();

      WHEN("Given a preinitialized set with 5 elements") {
         REQUIRE(set.GetCount() == 5);
         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.template Is<K>());
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         REQUIRE(set.GetReserved() >= 5);
      }

      /*WHEN("Create 2048 and then 4096 sets, and initialize them (weird corner case test)") {
         auto storage = new some<T>;
         storage->resize(2048);

         for (auto& i : *storage) {
            i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

            // Make sure that the set doesn't share memory with any other set 
            for (auto& i2 : *storage) {
               if (&i2 >= &i)
                  break;

               REQUIRE(i.GetEntry() != i2.GetEntry());
               REQUIRE(!i2.GetEntry()->Contains(i.GetRawMemory()));
            }

            REQUIRE(i.HasAuthority());
            REQUIRE(i.GetUses() == 1);
            REQUIRE(i.GetCount() == 5);
            REQUIRE(i.GetReserved() == 8);
            for (auto& k : darray1)
               REQUIRE(set.Contains(k));
         }

         delete storage;
         storage = new some<T>;
         storage->resize(4096);

         for (auto& i : *storage) {
            i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

            // Make sure that the set doesn't share memory with any other set 
            for (auto& i2 : *storage) {
               if (&i2 >= &i)
                  break;

               REQUIRE(i.GetEntry() != i2.GetEntry());
               REQUIRE(!i2.GetEntry()->Contains(i.GetRawMemory()));
            }

            REQUIRE(i.HasAuthority());
            REQUIRE(i.GetUses() == 1);
            REQUIRE(i.GetCount() == 5);
            REQUIRE(i.GetReserved() == 8);
            for (auto& k : darray1)
               REQUIRE(set.Contains(k));
         }

         delete storage;
      }*/

      WHEN("Shallow-copy more of the same stuff") {
         for (auto& i : darray1)
            REQUIRE(set.Contains(i));

         set << darray2[0];

         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         REQUIRE(set.Contains(darray2[0]));

         set << darray2[1];

         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         REQUIRE(set.Contains(darray2[0]));
         REQUIRE(set.Contains(darray2[1]));

         const auto backup = darray2[2];
         set << darray2[2];
         REQUIRE(backup == darray2[2]);

         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         REQUIRE(set.Contains(darray2[0]));
         REQUIRE(set.Contains(darray2[1]));
         REQUIRE(set.Contains(darray2[2]));

         //set.Dump();
         set << darray2[3];
         //set.Dump();

         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         REQUIRE(set.Contains(darray2[0]));
         REQUIRE(set.Contains(darray2[1]));
         REQUIRE(set.Contains(darray2[2]));
         REQUIRE(set.Contains(darray2[3]));

         set << darray2[4];

         REQUIRE(set.IsTypeConstrained() == CT::Typed<T>);
         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.template Is<K>());
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set.GetCount() == 10);
         REQUIRE(set.GetReserved() >= 10);

         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         for (auto& i : darray2)
            REQUIRE(set.Contains(i));

         //#if LANGULUS_FEATURE(MANAGED_MEMORY)
         //   REQUIRE(set.GetRawMemory() == memory);
         //#endif

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::TUnorderedSet::operator << (5 consecutive copies)") (timer meter) {
               some<MapType> storage(meter.runs());
               for (auto& i : storage)
                  i << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

               meter.measure([&](int i) {
                  return storage[i] << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
               });
            };

            BENCHMARK_ADVANCED("std::unordered_set::insert(5 consecutive copies)") (timer meter) {
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
         K movableDarray2[5] {
            darray2[0],
            darray2[1],
            darray2[2],
            darray2[3],
            darray2[4]
         };

         set
            << ::std::move(movableDarray2[0])
            << ::std::move(movableDarray2[1])
            << ::std::move(movableDarray2[2])
            << ::std::move(movableDarray2[3])
            << ::std::move(movableDarray2[4]);

         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set.GetCount() == 10);
         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.GetReserved() >= 10);

         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
         for (auto& i : darray2)
            REQUIRE(set.Contains(i));

         //#if LANGULUS_FEATURE(MANAGED_MEMORY)
         //   REQUIRE(set.GetRawMemory() == memory);
         //#endif

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::TUnorderedSet::operator << (5 consecutive trivial moves)") (timer meter) {
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

            BENCHMARK_ADVANCED("std::unordered_set::emplace_back(5 consecutive trivial moves)") (timer meter) {
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

      for (int iii = 0; iii < 10; ++iii) {
      WHEN(std::string("The size is reduced by finding and removing elements by value, but reserved memory should remain the same on shrinking #") + std::to_string(iii)) {
         const auto removed2 = set.Remove(darray1[1]);
         const auto removed4 = set.Remove(darray1[3]);

         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         REQUIRE(removed2 == 1);
         REQUIRE(removed4 == 1);
         REQUIRE(set.GetCount() == 3);
         REQUIRE(set.GetRawMemory() == memory);
         REQUIRE(set.GetReserved() >= 5);

         REQUIRE(set.Contains(darray1[0]));
         REQUIRE_FALSE(set.Contains(darray1[1]));
         REQUIRE(set.Contains(darray1[2]));
         REQUIRE_FALSE(set.Contains(darray1[3]));
         REQUIRE(set.Contains(darray1[4]));

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::TUnorderedSet::Remove") (timer meter) {
               some<MapType> storage(meter.runs());
               for (auto&& o : storage)
                  o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

               meter.measure([&](int i) {
                  return storage[i].RemoveValue(2);
               });
            };

            BENCHMARK_ADVANCED("std::unordered_set::erase(by value)") (timer meter) {
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
      }

      WHEN("Removing non-available elements") {
         const auto removed9 = set.Remove(darray2[3]);

         REQUIRE(removed9 == 0);
         REQUIRE(set.GetCount() == 5);
         REQUIRE(set.GetRawMemory() == memory);
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set.GetReserved() >= 5);

         for (auto& i : darray1)
            REQUIRE(set.Contains(i));
      }

      WHEN("More capacity is reserved") {
         set.Reserve(20);

         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set.GetCount() == 5);
         REQUIRE(set.GetReserved() >= 20);

         //#if LANGULUS_FEATURE(MANAGED_MEMORY)
         //   REQUIRE(set.GetRawMemory() == memory);
         //#endif
      }

      WHEN("Less capacity is reserved") {
         set.Reserve(2);

         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set.GetCount() == 5);
         REQUIRE(set.GetRawMemory() == memory);
         REQUIRE(set.GetReserved() >= 5);
      }

      WHEN("Set is cleared") {
         set.Clear();

         REQUIRE(set.GetCount() == 0);
         REQUIRE(set.IsAllocated());
         REQUIRE(set.GetType()->template Is<K>());
         REQUIRE(set.template Is<K>());
         REQUIRE(set.IsTypeConstrained() == CT::Typed<T>);
         REQUIRE(!set);
         REQUIRE(set.GetRawMemory() == memory);
         REQUIRE(set.HasAuthority());
         REQUIRE(set.GetUses() == 1);
         REQUIRE(set.GetReserved() >= 5);
      }

      WHEN("Set is reset") {
         set.Reset();

         REQUIRE(set.GetCount() == 0);
         REQUIRE_FALSE(set.IsAllocated());
         REQUIRE_FALSE(set.HasAuthority());
         if constexpr (CT::Typed<T>) {
            REQUIRE(set.template Is<K>());
            REQUIRE(set.GetType()->template Is<K>());
         }
         REQUIRE(set.IsTypeConstrained() == CT::Typed<T>);
         REQUIRE(!set);
         REQUIRE(set.GetRawMemory() != memory);
         REQUIRE(set.GetUses() == 0);
      }

      WHEN("Set is shallow-copied") {
         T copy = set;

         REQUIRE(copy == set);
         REQUIRE(copy.GetType()->template Is<K>());
         REQUIRE(copy.IsAllocated());
         REQUIRE(copy.HasAuthority());
         REQUIRE(copy.GetUses() == 2);
         REQUIRE(copy.GetCount() == set.GetCount());
         REQUIRE(copy.GetCount() == 5);
         REQUIRE(copy.GetRawMemory() == set.GetRawMemory());

         for (auto& comparer : darray1) {
            REQUIRE(copy.Contains(comparer));
            REQUIRE(set.Contains(comparer));
         }
      }

      WHEN("Set is cloned") {
         if constexpr (CT::CloneMakable<K>) {
            T clone = Langulus::Clone(set);

            REQUIRE((clone != set) == CT::Sparse<K>);
            REQUIRE(clone.GetType()->template Is<K>());
            REQUIRE(clone.IsAllocated());
            REQUIRE(clone.HasAuthority());
            REQUIRE(clone.GetUses() == 1);
            REQUIRE(clone.GetCount() == set.GetCount());
            REQUIRE(clone.GetCount() == 5);
            REQUIRE(clone.GetRawMemory() != set.GetRawMemory());

            if constexpr (CT::Sparse<K>) {
               for (auto item1 : clone) {
                  int found = 0;
                  for (auto item2 : set) {
                     if (*item1 == *item2)
                        ++found;
                  }

                  REQUIRE(found == 1);
               }
            }
            else {
               for (auto& item : clone)
                  REQUIRE(set.Contains(item));
            }
         }
         else if constexpr (CT::Untyped<T>) {
            T clone;
            REQUIRE_THROWS(new (&clone) T {Langulus::Clone(set)});
         }
      }

      WHEN("Set is move-constructed") {
         T movable = set;
         T moved = ::std::move(movable);

         REQUIRE(moved == set);
         REQUIRE(moved != movable);
         REQUIRE(moved.GetType()->template Is<K>());
         REQUIRE(moved.GetRawMemory() == memory);
         REQUIRE(moved.IsAllocated());
         REQUIRE(moved.GetCount() == 5);
         REQUIRE(moved.HasAuthority());
         REQUIRE(moved.GetUses() == 2);
         REQUIRE_FALSE(movable.IsAllocated());
         REQUIRE(!movable);
         REQUIRE(movable.GetRawMemory() == nullptr);
         REQUIRE(movable.GetCount() == 0);
         REQUIRE(movable.IsTypeConstrained() == CT::Typed<T>);

         for (auto& comparer : darray1) {
            REQUIRE(moved.Contains(comparer));
            REQUIRE(set.Contains(comparer));
            REQUIRE_FALSE(movable.Contains(comparer));
         }

         if constexpr (CT::Sparse<K>) {
            for (auto item1 : moved) {
               int found = 0;
               for (auto item2 : set) {
                  if (*item1 == *item2)
                     ++found;
               }

               REQUIRE(found == 1);
            }
         }
         else {
            for (auto& item : moved)
               REQUIRE(set.Contains(item));
         }
      }

      WHEN("Sets are compared") {
         T sameSet;
         sameSet << darray1;
         T copiedSet {set};
         T differentSet1;
         differentSet1 << darray1[0] << darray1[0] << darray1[2] << darray1[3] << darray1[4];

         REQUIRE(set == sameSet);
         REQUIRE(set == copiedSet);
         REQUIRE(set != differentSet1);

         if constexpr (CT::CloneMakable<K>) {
            T clonedSet {Clone(set)};
            REQUIRE((set != clonedSet) == CT::Sparse<K>);
         }
      }

      WHEN("Sets are iterated with ranged-for") {
         for (auto& comparer : darray1)
            REQUIRE(set.Contains(comparer));

         unsigned i = 0;
         for (auto item : set) {
            // Different architectures result in different hashes       
            if constexpr (CT::Dense<K>) {
               if constexpr (Bitness == 32) {
                  switch (i) {
                  case 0:
                     REQUIRE(item == darray1[0]);
                     break;
                  case 1:
                     REQUIRE(item == darray1[3]);
                     break;
                  case 2:
                     REQUIRE(item == darray1[2]);
                     break;
                  case 3:
                     REQUIRE(item == darray1[1]);
                     break;
                  case 4:
                     REQUIRE(item == darray1[4]);
                     break;
                  default:
                     FAIL("Index out of bounds in ranged-for");
                     break;
                  }
               }
               else if constexpr (Bitness == 64) {
                  switch (i) {
                  case 0:
                     REQUIRE(item == darray1[2]);
                     break;
                  case 1:
                     REQUIRE(item == darray1[1]);
                     break;
                  case 2:
                     REQUIRE(item == darray1[0]);
                     break;
                  case 3:
                     REQUIRE(item == darray1[3]);
                     break;
                  case 4:
                     REQUIRE(item == darray1[4]);
                     break;
                  default:
                     FAIL("Index out of bounds in ranged-for");
                     break;
                  }
               }
               else break;
            }

            ++i;
         }

         REQUIRE(i == set.GetCount());
      }

      WHEN("ForEach flat dense key (immutable)") {
         for (auto& comparer : darray1)
            REQUIRE(set.Contains(comparer));

         unsigned i = 0;
         const auto done = set.ForEach([&](const K& key) {
            // Different architectures result in different hashes       
            if constexpr (CT::Dense<K>) {
               if constexpr (Bitness == 32) {
                  switch (i) {
                  case 0:
                     REQUIRE(key == darray1[0]);
                     break;
                  case 1:
                     REQUIRE(key == darray1[3]);
                     break;
                  case 2:
                     REQUIRE(key == darray1[2]);
                     break;
                  case 3:
                     REQUIRE(key == darray1[1]);
                     break;
                  case 4:
                     REQUIRE(key == darray1[4]);
                     break;
                  default:
                     FAIL("Index out of bounds in ranged-for");
                     break;
                  }
               }
               else if constexpr (Bitness == 64) {
                  switch (i) {
                  case 0:
                     REQUIRE(key == darray1[2]);
                     break;
                  case 1:
                     REQUIRE(key == darray1[1]);
                     break;
                  case 2:
                     REQUIRE(key == darray1[0]);
                     break;
                  case 3:
                     REQUIRE(key == darray1[3]);
                     break;
                  case 4:
                     REQUIRE(key == darray1[4]);
                     break;
                  default:
                     FAIL("Index out of bounds in ranged-for");
                     break;
                  }
               }
               else return false;
            }

            ++i;
            return true;
         });

         REQUIRE(i == set.GetCount());
         REQUIRE(i == done);
      }
   }

   if constexpr (CT::Sparse<K>) {
      if constexpr (CT::Referencable<Deptr<K>>)
         element->Reference(-1);
      delete element;

      for (auto i : darray1) {
         if constexpr (CT::Referencable<Deptr<K>>)
            i->Reference(-1);
         delete i;
      }

      for (auto i : darray2) {
         if constexpr (CT::Referencable<Deptr<K>>)
            i->Reference(-1);
         delete i;
      }
   }

   REQUIRE(memoryState.Assert());
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
TEMPLATE_TEST_CASE("Set corner cases", "[set]",
   (TypePair<UnorderedSet, DMeta>),
   (TypePair<TUnorderedSet<DMeta>, DMeta>),
   (TypePair<TOrderedSet<DMeta>, DMeta>),
   (TypePair<OrderedSet, DMeta>)
) {
   static Allocator::State memoryState;

   using T = typename TestType::LHS;
   using K = typename TestType::RHS;

   GIVEN("Map instance initialized with 10 specific pairs for the corner case") {
      const K keys[10] = {
         MetaOf<VulkanLayer>(),
         MetaOf<VulkanRenderer>(),
         MetaOf<VulkanCamera>(),
         MetaOf<Platform>(),
         MetaOf<Vulkan>(),
         MetaOf<Window>(),
         MetaOf<VulkanLight>(),
         MetaOf<Monitor>(),
         MetaOf<VulkanRenderable>(),
         MetaOf<Cursor>()
      };

      T set {keys};

      WHEN("Removing around-the-end elements (corner case)") {
         Count removed {};
         removed += set.Remove(MetaOf<VulkanRenderer>());
         removed += set.Remove(MetaOf<VulkanCamera>());
         removed += set.Remove(MetaOf<Vulkan>());
         removed += set.Remove(MetaOf<VulkanRenderable>());
         removed += set.Remove(MetaOf<VulkanLight>());
         removed += set.Remove(MetaOf<VulkanLayer>());

         REQUIRE(removed == 6);
         REQUIRE(set.GetCount() == 4);
         REQUIRE_FALSE(set.Contains(MetaOf<VulkanLayer>()));
         REQUIRE_FALSE(set.Contains(MetaOf<VulkanRenderer>()));
         REQUIRE_FALSE(set.Contains(MetaOf<VulkanCamera>()));
         REQUIRE_FALSE(set.Contains(MetaOf<Vulkan>()));
         REQUIRE_FALSE(set.Contains(MetaOf<VulkanLight>()));
         REQUIRE_FALSE(set.Contains(MetaOf<VulkanRenderable>()));
         REQUIRE(set.Contains(MetaOf<Platform>()));
         REQUIRE(set.Contains(MetaOf<Window>()));
         REQUIRE(set.Contains(MetaOf<Monitor>()));
         REQUIRE(set.Contains(MetaOf<Cursor>()));
      }
   }

   REQUIRE(memoryState.Assert());
}
