///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <Anyness/THive.hpp>
#include <catch2/catch.hpp>


struct Producible : Referenced {
   int v;

   Producible(int vv) : v {vv} {}

   bool operator == (const Producible& a) const noexcept {
      return v == a.v;
   }
};

SCENARIO("Test hives", "[hive]") {
   static Allocator::State memoryState;
   const auto one = Producible {1};
   const auto two = Producible {2};

	GIVEN("A hive instance") {
		THive<Producible> hive;

		WHEN("Default-constructed") {
			REQUIRE(hive.GetReusable() == nullptr);
			REQUIRE(hive.IsEmpty());
			REQUIRE(hive.GetType() == MetaOf<Producible>());
			REQUIRE(hive.GetCount() == 0);
		}

		WHEN("Two elements produced") {
         auto p1 = hive.New(1);
         auto p2 = hive.New(2);

         REQUIRE(p1);
         REQUIRE(p2);

			REQUIRE(hive.GetCount() == 2);
			REQUIRE(hive.GetFrames().GetCount() == 1);
         REQUIRE(hive.GetReusable() == hive.GetFrames()[0].GetRaw() + 2);
         REQUIRE(hive.GetType() == MetaOf<Producible>());
			REQUIRE(hive.GetFrames()[0].GetRaw()[0].mData == one);
			REQUIRE(hive.GetFrames()[0].GetRaw()[1].mData == two);
		}
   }

   one.Reference(-1);
   two.Reference(-1);

   REQUIRE(memoryState.Assert());
}
