///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Construct.hpp>
#include "Common.hpp"


SCENARIO("Constructs", "[construct]") {
   static Allocator::State memoryState;

	GIVEN("A complex descriptor") {
		Any descriptor;

		WHEN("Normalized") {
			Construct c;

			THEN("The requirements should be met") {
				REQUIRE(true);
			}
		}
	}

   REQUIRE(memoryState.Assert());
}
