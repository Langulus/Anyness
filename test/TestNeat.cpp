///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Neat.hpp>
#include "Common.hpp"


SCENARIO("Data normalization", "[neat]") {
   static Allocator::State memoryState;

   static_assert(not CT::Abstract<Any>);
   static_assert(not CT::Abstract<TAny<Any>>);
   static_assert(not CT::Abstract<TMeta>);
   static_assert(not CT::Abstract<TPair<TMeta, TAny<Any>>>);

   Any test1aa;
   Any test2aa {Clone(test1aa)};
   Any test3aa {Copy(test1aa)};
   Any test4aa {Refer(test1aa)};

   SemanticNew(&test3aa, Copy(test1aa));
   SemanticNew(&test3aa, Clone(test1aa));
   SemanticNew(&test3aa, Refer(test1aa));

   static_assert(CT::Complete<Any>);

   static_assert(CT::SemanticMakable<Copied, Any>);
   static_assert(CT::SemanticMakable<Referred, Any>);

   static_assert(CT::Inner::SemanticMakable<Copied, Any>);
   static_assert(CT::Inner::SemanticMakable<Referred, Any>);

   static_assert(CT::DeepMakable<Any, Cloned<TAny<Any>>>);
   static_assert(CT::CopyMakable<Any>);
   static_assert(CT::ReferMakable<Any>);
   static_assert(CT::CloneMakable<Any>);

   static_assert(CT::CopyMakable<TMeta>);
   static_assert(CT::ReferMakable<TMeta>);
   static_assert(CT::CloneMakable<TMeta>);

   static_assert(CT::CopyMakable<TAny<Any>>);
   static_assert(CT::ReferMakable<TAny<Any>>);
   static_assert(CT::CloneMakable<TAny<Any>>);

   TAny<Any> test1a;
   TAny<Any> test2a {Clone(test1a)};
   TAny<Any> test3a {Copy(test1a)};
   TAny<Any> test4a {Refer(test1a)};

   TPair<TMeta, TAny<Any>> test1;
   TPair<TMeta, TAny<Any>> test2 {Clone(test1)};
   TPair<TMeta, TAny<Any>> test3 {Copy(test1)};
   TPair<TMeta, TAny<Any>> test4 {Refer(test1)};



   static_assert(CT::Exact<typename SemanticOf<Cloned<const int>>::template
      As<float>, Cloned<float>>);
   static_assert(CT::Exact<typename SemanticOf<Cloned<const int>&>::template
      As<float>, Cloned<float>>);
   static_assert(CT::Exact<typename SemanticOf<Cloned<const int>&&>::template
      As<float>, Cloned<float>>);
   static_assert(CT::Exact<typename SemanticOf<const Cloned<const int>&>::template
      As<float>, Cloned<float>>);

   static_assert(CT::Pair<Desem<Cloned<TPair<TMeta, TAny<Any>>>>>);
   static_assert(CT::PairMakable<TMeta, TAny<Any>, Cloned<TPair<TMeta, TAny<Any>>>>);
   static_assert(CT::PairAssignable<TMeta, TAny<Any>, Cloned<TPair<TMeta, TAny<Any>>>>);

   static_assert(CT::Inner::SemanticMakableAlt<Copied<TMeta>>);
   static_assert(CT::Inner::SemanticMakableAlt<Referred<TMeta>>);
   static_assert(CT::Inner::SemanticMakableAlt<Cloned<TMeta>>);

   static_assert(CT::Inner::SemanticMakableAlt<Copied<TAny<Any>>>);
   static_assert(CT::Inner::SemanticMakableAlt<Referred<TAny<Any>>>);
   static_assert(CT::Inner::SemanticMakableAlt<Cloned<TAny<Any>>>);

   static_assert(CT::Inner::SemanticMakableAlt<Copied<TPair<TMeta, TAny<Any>>>>);
   static_assert(CT::Inner::SemanticMakableAlt<Referred<TPair<TMeta, TAny<Any>>>>);
   static_assert(CT::Inner::SemanticMakableAlt<Cloned<TPair<TMeta, TAny<Any>>>>);

   static_assert(CT::CopyMakable<TUnorderedMap<TMeta, TAny<Any>>>);
   static_assert(CT::ReferMakable<TUnorderedMap<TMeta, TAny<Any>>>);
   static_assert(CT::CloneMakable<TUnorderedMap<TMeta, TAny<Any>>>);

   static_assert(CT::MoveMakable<TPair<TMeta, TAny<Any>>>);
   static_assert(CT::CopyMakable<TPair<TMeta, TAny<Any>>>);
   static_assert(CT::ReferMakable<TPair<TMeta, TAny<Any>>>);
   static_assert(CT::CloneMakable<TPair<TMeta, TAny<Any>>>);

	GIVEN("An empty messy descriptor") {
		Any descriptor;

		WHEN("Normalized") {
			Neat normalized {descriptor};
		}
	}

	GIVEN("A messy descriptor with contents") {
      TAny<Byte> data;
      data.New(8192);

      WHEN("Filled with contents") {
         Neat normalized {data};
      }
	}
   
	GIVEN("A neat container full of many things") {
      struct ComplexStuff {
         int x = 1;
         float y = 2;
         double z = 3;
         std::string name;

         bool operator == (const ComplexStuff&) const = default;

         ~ComplexStuff() {
            x = 0;
            y = 1;
            z = 2;
         }
      };

      Neat neat {
         Traits::Name {"Root"},
         Construct::From<int>(),
         Construct::From<float>(),
         Construct::From<double>(),
         Construct::From<ComplexStuff>(
            Traits::Name {"Child1"},
            Construct::From<int>(),
            Construct::From<float>(),
            Construct::From<ComplexStuff>(Traits::Name {"GrandChild1"}),
            Construct::From<ComplexStuff>(Traits::Name {"GrandChild2"})
         ),
         Construct::From<ComplexStuff>(Traits::Name {"Child2"})
      };

      WHEN("Copied") {
         Neat copied = neat;

         //REQUIRE(neat == copied);
      }
	}

   REQUIRE(memoryState.Assert());
}
