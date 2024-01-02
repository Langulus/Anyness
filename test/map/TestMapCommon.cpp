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


/// Cross-container consistency tests                                         
TEMPLATE_TEST_CASE(
   "Cross-container consistency tests for TOrderedMap/TUnorderedMap/OrderedMap/UnorderedMap", "[map]",
   (MapPair2<Text, int>),
   (MapPair2<Text, Trait>),
   (MapPair2<Text, Any>),
   (MapPair2<Text, Traits::Count>),
   (MapPair2<Text, int*>),
   (MapPair2<Text, Trait*>),
   (MapPair2<Text, Traits::Count*>),
   (MapPair2<Text, Any*>)
) {
   Allocator::State memoryState;

   GIVEN("A single element initialized maps of all kinds") {
      using K = typename TestType::Key;
      using V = typename TestType::Value;

      const auto pair = CreatePair<TPair<K, V>, K, V>(
         "five hundred", 555);

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

      // Check for memory leaks after each cycle                     
      REQUIRE(memoryState.Assert());
   }
}
