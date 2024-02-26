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
#include <Anyness/Referenced.hpp>
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

	GIVEN("A hive instance") {
		THive<Producible> hive;

		WHEN("Default-constructed") {
			REQUIRE(hive.mReusable == nullptr);
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
			REQUIRE(hive.mFrames.GetCount() == 1);
         REQUIRE(hive.mReusable == hive.mFrames[0].GetRaw() + 2);
         REQUIRE(hive.GetType() == MetaOf<Producible>());
			REQUIRE(hive.mFrames[0].GetRaw()[0].mData == Producible {1});
			REQUIRE(hive.mFrames[0].GetRaw()[1].mData == Producible {2});
		}

      // Check for memory leaks after each cycle                        
   }

   REQUIRE(memoryState.Assert());
}
