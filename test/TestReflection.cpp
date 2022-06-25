///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE("Unsigned integer RTTI interpretation", "[metadata]", /*uint8_t,*/ uint16_t, uint32_t, uint64_t) {
	#include "CollectGarbage.inl"

	GIVEN("An unsigned integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(RTTI::CastsTo<A::Number>(meta));
				REQUIRE(RTTI::CastsTo<A::Integer>(meta));
				REQUIRE(RTTI::CastsTo<A::Unsigned>(meta));
				REQUIRE(RTTI::CastsTo<A::UnsignedInteger>(meta));

				REQUIRE_FALSE(RTTI::CastsTo<A::Real>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<A::Signed>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<A::SignedInteger>(meta));

				REQUIRE(RTTI::CastsTo<A::Number>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::Integer>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::Unsigned>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::UnsignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<A::Real>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<A::Signed>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<A::SignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<A::Number>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Integer>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Real>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Unsigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Signed>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::UnsignedInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::SignedInteger>(meta, 2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Signed integer RTTI interpretation", "[metadata]", int8_t, int16_t, int32_t, int64_t) {
	#include "CollectGarbage.inl"

	GIVEN("A signed integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(RTTI::CastsTo<A::Number>(meta));
				REQUIRE(RTTI::CastsTo<A::Integer>(meta));
				REQUIRE(RTTI::CastsTo<A::Signed>(meta));
				REQUIRE(RTTI::CastsTo<A::SignedInteger>(meta));

				REQUIRE_FALSE(RTTI::CastsTo<A::Real>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<A::Unsigned>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<A::UnsignedInteger>(meta));

				REQUIRE(RTTI::CastsTo<A::Number>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::Integer>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::Signed>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::SignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<A::Real>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<A::Unsigned>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<A::UnsignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<A::Number>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Integer>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Real>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Unsigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Signed>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::UnsignedInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::SignedInteger>(meta, 2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Real number RTTI interpretation", "[metadata]", float, double) {
	#include "CollectGarbage.inl"

	GIVEN("A real number type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(RTTI::CastsTo<A::Number>(meta));
				REQUIRE(RTTI::CastsTo<A::Real>(meta));
				REQUIRE(RTTI::CastsTo<A::Signed>(meta));

				REQUIRE_FALSE(RTTI::CastsTo<A::Integer>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<A::Unsigned>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<A::UnsignedInteger>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<A::SignedInteger>(meta));

				REQUIRE(RTTI::CastsTo<A::Number>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::Real>(meta, 1));
				REQUIRE(RTTI::CastsTo<A::Signed>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<A::Integer>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<A::Unsigned>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<A::UnsignedInteger>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<A::SignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<A::Number>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Integer>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Real>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Unsigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::Signed>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::UnsignedInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<A::SignedInteger>(meta, 2));
			}
		}
	}
}