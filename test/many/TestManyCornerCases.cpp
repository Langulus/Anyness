///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "TestManyCommon.hpp"


SCENARIO("Pushing one sparse container, and then two more, one being the first", "[many]") {
   static Allocator::State memoryState;

   auto p1 = CreateElement<Many*, true>(1);
   auto p2 = CreateElement<Many*, true>(2);

   GIVEN("An empty container") {
      Many pack;

      WHEN("Pushed the first pointer") {
         pack << p1;

         REQUIRE(pack == p1);
         REQUIRE(pack.GetCount() == 1);
         REQUIRE(pack.IsExact<Many*>());
         REQUIRE(p1->GetUses() == 2);
         REQUIRE(p2->GetUses() == 1);

         THEN("Push-back the first again and then the second") {
            pack << p1;
            pack << p2;

            REQUIRE(pack.GetCount() == 3);
            REQUIRE(pack.IsExact<Many*>());
            REQUIRE(p1->GetUses() == 3);
            REQUIRE(p2->GetUses() == 2);
         }

         THEN("Push-front the first again and then the second") {
            pack >> p1;
            pack >> p2;

            REQUIRE(pack.GetCount() == 3);
            REQUIRE(pack.IsExact<Many*>());
            REQUIRE(p1->GetUses() == 3);
            REQUIRE(p2->GetUses() == 2);
         }

         THEN("Smart-push-back the first again and then the second, but packed together") {
            pack.SmartPush(IndexBack, Many {p1, p2});

            REQUIRE(pack.GetCount() == 2);
            REQUIRE(pack.IsExact<Many>());
            REQUIRE(p1->GetUses() == 3);
            REQUIRE(p2->GetUses() == 2);
            REQUIRE(pack[0].IsExact<Many*>());
            REQUIRE(pack[0].GetCount() == 1);
            REQUIRE(pack[0] == p1);
            REQUIRE(pack[1].GetCount() == 2);
            REQUIRE(pack[1] == Many {p1, p2});
         }

         THEN("Smart-push-front the first again and then the second, but packed together") {
            pack.SmartPush(IndexFront, Many {p1, p2});

            REQUIRE(pack.GetCount() == 2);
            REQUIRE(pack.IsExact<Many>());
            REQUIRE(p1->GetUses() == 3);
            REQUIRE(p2->GetUses() == 2);
            REQUIRE(pack[1].IsExact<Many*>());
            REQUIRE(pack[1].GetCount() == 1);
            REQUIRE(pack[1] == p1);
            REQUIRE(pack[0].GetCount() == 2);
            REQUIRE(pack[0] == Many {p1, p2});
         }
      }
   }

   DestroyElement<true>(p1);
   DestroyElement<true>(p2);
}