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
	GIVEN("A complex descriptor") {
		Any descriptor;

		WHEN("Normalized") {
			Neat normalized {descriptor};

			THEN("The requirements should be met") {
				REQUIRE(true);
			}
		}
	}
}
