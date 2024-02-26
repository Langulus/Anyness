///                                                                           
/// Langulus::RTTI                                                            
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Text.hpp>
#include <Anyness/Path.hpp>
#include <Anyness/TMap.hpp>
#include <Anyness/TSet.hpp>
#include "Common.hpp"


SCENARIO("Hashing different kinds of containers", "[hash]") {
   WHEN("Hashing all kinds of containers") {
      Token same1 = "Same1"; 
      std::string same1str = "Same1";
      Text same1txt = "Same1";
      Path same1pat = "Same1";
      Any same1any {'S', 'a', 'm', 'e', '1'};
      std::vector<char> same1vec = {'S', 'a', 'm', 'e', '1'};
      //std::span<char, 5> sape1spa = "Same1";
      std::array<char, 5> sape1arr = {'S', 'a', 'm', 'e', '1'};

      REQUIRE(HashOf(same1) == HashOf(same1str));
      REQUIRE(HashOf(same1) == HashOf(same1any));
      REQUIRE(HashOf(same1) == HashOf(same1txt));
      REQUIRE(HashOf(same1) == HashOf(same1pat));
      REQUIRE(HashOf(same1) == HashOf(same1vec));
      //REQUIRE(HashOf(same1) == HashOf(sape1spa));
      REQUIRE(HashOf(same1) == HashOf(sape1arr));
      REQUIRE(HashOf(same1) == HashBytes("Same1", 5));
   }
}


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

         REQUIRE(uhash1 == uhash2);
         REQUIRE(ohash1 == ohash2);
         REQUIRE(uhash1 == ohash1);
         REQUIRE(uhash1 == elementHash);
      }

      // Check for memory leaks after each cycle                        
      REQUIRE(memoryState.Assert());
   }
}

/// Cross-container consistency tests                                         
TEMPLATE_TEST_CASE(
   "Cross-container consistency tests for TOrderedSet/TUnorderedSet/OrderedSet/UnorderedSet", "[set]",
   int,  Trait,  Traits::Count,  Any,
   int*, Trait*, Traits::Count*, Any*
) {
   const auto element = CreateElement<TestType>(555);
   const auto elementHash = HashOf(element);

   GIVEN("A single element initialized sets of all kinds") {
      TUnorderedSet<TestType> uset1 {element};
      UnorderedSet uset2 {element};
      TOrderedSet<TestType> oset1 {element};
      OrderedSet oset2 {element};

      WHEN("Their hashes are taken") {
         REQUIRE(uset1.template IsExact<TestType>());
         REQUIRE(uset2.template IsExact<TestType>());
         REQUIRE(oset1.template IsExact<TestType>());
         REQUIRE(oset2.template IsExact<TestType>());

         const auto uhash1 = uset1.GetHash();
         const auto uhash2 = uset2.GetHash();
         const auto ohash1 = oset1.GetHash();
         const auto ohash2 = oset2.GetHash();

         REQUIRE(uhash1 == uhash2);
         REQUIRE(ohash1 == ohash2);
         REQUIRE(uhash1 == ohash1);
         REQUIRE(uhash1 == elementHash);
      }
   }

   if constexpr (CT::Sparse<TestType>)
      delete element;
}
