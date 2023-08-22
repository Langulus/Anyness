#include "Main.hpp"
#include <Anyness/Neat.hpp>
#include <catch2/catch.hpp>

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
