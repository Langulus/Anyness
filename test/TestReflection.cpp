///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE("Unsigned integer RTTI interpretation", "[metadata]", uint8_t, uint16_t, uint32_t, uint64_t) {
	Allocator::CollectGarbage();

	GIVEN("An unsigned integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(RTTI::CastsTo<ANumber>(meta));
				REQUIRE(RTTI::CastsTo<AInteger>(meta));
				REQUIRE(RTTI::CastsTo<AUnsigned>(meta));
				REQUIRE(RTTI::CastsTo<AUnsignedInteger>(meta));

				REQUIRE_FALSE(RTTI::CastsTo<AReal>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<ASigned>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<ASignedInteger>(meta));

				REQUIRE(RTTI::CastsTo<ANumber>(meta, 1));
				REQUIRE(RTTI::CastsTo<AInteger>(meta, 1));
				REQUIRE(RTTI::CastsTo<AUnsigned>(meta, 1));
				REQUIRE(RTTI::CastsTo<AUnsignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<AReal>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<ASigned>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<ASignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<ANumber>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AReal>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<ASigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsignedInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<ASignedInteger>(meta, 2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Signed integer RTTI interpretation", "[metadata]", int8_t, int16_t, int32_t, int64_t) {
	Allocator::CollectGarbage();

	GIVEN("A signed integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(RTTI::CastsTo<ANumber>(meta));
				REQUIRE(RTTI::CastsTo<AInteger>(meta));
				REQUIRE(RTTI::CastsTo<ASigned>(meta));
				REQUIRE(RTTI::CastsTo<ASignedInteger>(meta));

				REQUIRE_FALSE(RTTI::CastsTo<AReal>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsigned>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsignedInteger>(meta));

				REQUIRE(RTTI::CastsTo<ANumber>(meta, 1));
				REQUIRE(RTTI::CastsTo<AInteger>(meta, 1));
				REQUIRE(RTTI::CastsTo<ASigned>(meta, 1));
				REQUIRE(RTTI::CastsTo<ASignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<AReal>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsigned>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<ANumber>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AReal>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<ASigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsignedInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<ASignedInteger>(meta, 2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Real number RTTI interpretation", "[metadata]", float, double) {
	Allocator::CollectGarbage();

	GIVEN("A real number type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(RTTI::CastsTo<ANumber>(meta));
				REQUIRE(RTTI::CastsTo<AReal>(meta));
				REQUIRE(RTTI::CastsTo<ASigned>(meta));

				REQUIRE_FALSE(RTTI::CastsTo<AInteger>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsigned>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsignedInteger>(meta));
				REQUIRE_FALSE(RTTI::CastsTo<ASignedInteger>(meta));

				REQUIRE(RTTI::CastsTo<ANumber>(meta, 1));
				REQUIRE(RTTI::CastsTo<AReal>(meta, 1));
				REQUIRE(RTTI::CastsTo<ASigned>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<AInteger>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsigned>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsignedInteger>(meta, 1));
				REQUIRE_FALSE(RTTI::CastsTo<ASignedInteger>(meta, 1));

				REQUIRE_FALSE(RTTI::CastsTo<ANumber>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AReal>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<ASigned>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<AUnsignedInteger>(meta, 2));
				REQUIRE_FALSE(RTTI::CastsTo<ASignedInteger>(meta, 2));
			}
		}
	}
}