///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "TestPairCommon.hpp"


/// The main test for TPair/Pair containers, with all kinds of items, from    
/// sparse to dense, from trivial to complex, from flat to deep               
TEMPLATE_TEST_CASE("Dense TPair/Pair", "[pair]",
   (MapTest<TPair<Text, int>, Text, int>),
   (MapTest<TPair<Text, Trait>, Text, Trait>),
   (MapTest<TPair<Text, Traits::Count>, Text, Traits::Count>),
   (MapTest<TPair<Text, Many>, Text, Many>),

   (MapTest<Pair, Text, int>),
   (MapTest<Pair, Text, Trait>),
   (MapTest<Pair, Text, Traits::Count>),
   (MapTest<Pair, Text, Many>)
) {
   IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());
   static Allocator::State memoryState;

   using T = typename TestType::Container;
   using K = typename TestType::Key;
   using V = typename TestType::Value;
   using stdT = ::std::pair<K, V>;

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

   const auto lp = CreatePair<   T, K, V>("five hundred", 555);
   const auto sp = CreatePair<stdT, K, V>("five hundred", 555);


   GIVEN("A default-initialized pair instance") {
      T pair {};

      WHEN("Given a default-constructed pair") {
         Pair_CheckState_Default<K, V>(pair);

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::pair::default construction") (timer meter) {
               some<uninitialized<T>> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i].construct();
               });
            };

            BENCHMARK_ADVANCED("std::pair::default construction") (timer meter) {
               some<uninitialized<stdT>> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i].construct();
               });
            };
         #endif
      }

      WHEN("Assigned a pair by move") {
         auto movablePair = lp;
         pair = ::std::move(movablePair);

         Any_Helper_TestType<K>(pair.GetKey());
         Any_Helper_TestType<V>(pair.GetValue());

         REQUIRE(movablePair != lp);
         REQUIRE(pair == lp);
         REQUIRE(pair.mKey == lp.mKey);
         REQUIRE(pair.mValue == lp.mValue);

         #ifdef LANGULUS_STD_BENCHMARK
            BENCHMARK_ADVANCED("Anyness::pair::operator = (single pair copy)") (timer meter) {
               some<T> source(meter.runs());
               for (auto& i : source)
                  i = CreatePair<Pair, K, V>("five hundred"_text, 555);
                  
               some<T> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i] = ::std::move(source[i]);
               });
            };

            BENCHMARK_ADVANCED("std::pair::insert(single pair copy)") (timer meter) {
               some<stdT> source(meter.runs());
               for(auto& i : source)
                  i = valueStd;

               some<stdT> storage(meter.runs());
               meter.measure([&](int i) {
                  return storage[i].emplace(::std::move(source[i]));
               });
            };
         #endif
      }
   }
   
   GIVEN("A copy-initialized pair instance") {
      T pair {lp};

      Any_Helper_TestType<K>(pair.GetKey());
      Any_Helper_TestType<V>(pair.GetValue());

      REQUIRE(pair == lp);
      REQUIRE(pair.mKey == lp.mKey);
      REQUIRE(pair.mValue == lp.mValue);
   }
   
   GIVEN("Pair with some items") {
      T pair {lp};

      WHEN("Pair is cleared") {
         pair.Clear();

         Any_Helper_TestType<K>(pair.GetKey());
         Any_Helper_TestType<V>(pair.GetValue());

         REQUIRE(pair != lp);
         REQUIRE(pair.mKey != lp.mKey);
         REQUIRE(pair.mValue != lp.mValue);
      }

      WHEN("Pair is reset") {
         pair.Reset();

         Pair_CheckState_Default<K, V>(pair);
      }

      WHEN("Pair is shallow-copied") {
         auto copy = pair;

         Any_Helper_TestType<K>(copy.GetKey());
         Any_Helper_TestType<V>(copy.GetValue());

         REQUIRE(copy == pair);
         REQUIRE(copy.mKey == pair.mKey);
         REQUIRE(copy.mValue == pair.mValue);
      }

      WHEN("Pair is cloned") {
         T clone = Clone(pair);

         Any_Helper_TestType<K>(clone.GetKey());
         Any_Helper_TestType<V>(clone.GetValue());

         REQUIRE((clone != pair) == (CT::Sparse<K> or CT::Sparse<V>));
         REQUIRE(clone.mKey == pair.mKey);
         REQUIRE(clone.mValue == pair.mValue);
      }

      WHEN("Pair is move-constructed") {
         T movable = pair;
         T moved = ::std::move(movable);

         Pair_CheckState_Default<K, V>(movable);

         Any_Helper_TestType<K>(moved.GetKey());
         Any_Helper_TestType<V>(moved.GetValue());

         REQUIRE(moved == pair);
         REQUIRE(moved != movable);
      }

      WHEN("Pairs are compared") {
         T samePair   {lp};
         T clonedPair {Clone(pair)};
         T copiedPair {pair};
         T differentPair1 {CreatePair<T, K, V>("not five hundred", -555)};

         REQUIRE(pair == samePair);
         REQUIRE((pair != clonedPair) == (CT::Sparse<K> or CT::Sparse<V>));
         REQUIRE(pair == copiedPair);
         REQUIRE(pair != differentPair1);
      }
   }

   REQUIRE(memoryState.Assert());
}