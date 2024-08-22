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
   BANK.Reset();

   static Allocator::State memoryState;

   auto p1 = CreateElement<Many*, true>(1);
   auto p2 = CreateElement<Many*, true>(2);
   auto entry1 = Fractalloc::Allocator::Find(MetaOf<Many>(), p1);
   auto entry2 = Fractalloc::Allocator::Find(MetaOf<Many>(), p2);

   REQUIRE(entry1->GetUses() == 1);
   REQUIRE(entry2->GetUses() == 1);


   GIVEN("An empty container") {
      Many pack;

      WHEN("Pushed the first pointer") {
         pack << p1;

         REQUIRE(pack == p1);
         REQUIRE(pack.GetCount() == 1);
         REQUIRE(pack.IsExact<Many*>());
         REQUIRE(p1->GetUses() == 1);
         REQUIRE(p2->GetUses() == 1);
         REQUIRE(entry1->GetUses() == 2);
         REQUIRE(entry2->GetUses() == 1);

         THEN("Push-back the first again and then the second") {
            pack << p1;
            REQUIRE(pack.GetEntries()[0] == entry1);
            REQUIRE(pack.GetEntries()[1] == entry1);

            pack << p2;
            REQUIRE(pack.GetEntries()[0] == entry1);
            REQUIRE(pack.GetEntries()[1] == entry1);
            REQUIRE(pack.GetEntries()[2] == entry2);

            REQUIRE(pack.GetCount() == 3);
            REQUIRE(pack.IsExact<Many*>());
            REQUIRE(p1->GetUses() == 1);
            REQUIRE(p2->GetUses() == 1);
            REQUIRE(entry1->GetUses() == 3);
            REQUIRE(entry2->GetUses() == 2);
         }

         THEN("Push-front the first again and then the second") {
            pack >> p1;
            REQUIRE(pack.GetEntries()[0] == entry1);
            REQUIRE(pack.GetEntries()[1] == entry1);

            pack >> p2;
            REQUIRE(pack.GetEntries()[0] == entry2);
            REQUIRE(pack.GetEntries()[1] == entry1);
            REQUIRE(pack.GetEntries()[2] == entry1);

            REQUIRE(pack.GetCount() == 3);
            REQUIRE(pack.IsExact<Many*>());
            REQUIRE(p1->GetUses() == 1);
            REQUIRE(p2->GetUses() == 1);
            REQUIRE(entry1->GetUses() == 3);
            REQUIRE(entry2->GetUses() == 2);
         }

         THEN("Smart-push-back the first again and then the second, but packed together") {
            pack.SmartPush(IndexBack, Many {p1, p2});

            REQUIRE(pack.GetCount() == 3);
            REQUIRE(pack.IsExact<Many*>());
            REQUIRE(p1->GetUses() == 1);
            REQUIRE(p2->GetUses() == 1);
            REQUIRE(entry1->GetUses() == 3);
            REQUIRE(entry2->GetUses() == 2);
         }

         THEN("Smart-push-front the first again and then the second, but packed together") {
            pack.SmartPush(IndexFront, Many {p1, p2});

            REQUIRE(pack.GetCount() == 3);
            REQUIRE(pack.IsExact<Many*>());
            REQUIRE(p1->GetUses() == 1);
            REQUIRE(p2->GetUses() == 1);
            REQUIRE(entry1->GetUses() == 3);
            REQUIRE(entry2->GetUses() == 2);
         }
      }
   }

   REQUIRE(p1->GetUses() == 1);
   REQUIRE(p2->GetUses() == 1);
   REQUIRE(entry1->GetUses() == 1);
   REQUIRE(entry2->GetUses() == 1);

   DestroyElement<true>(p1);
   DestroyElement<true>(p2);

   REQUIRE(memoryState.Assert());
}