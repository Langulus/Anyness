#include "TestMain.hpp"
#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE("Unsigned integer RTTI interpretation", "[metadata]", uint8_t, uint16_t, uint32_t, uint64_t) {
	GIVEN("An unsigned integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(meta->InterpretsAs<ANumber>());
				REQUIRE(meta->InterpretsAs<AInteger>());
				REQUIRE(meta->InterpretsAs<AUnsigned>());
				REQUIRE(meta->InterpretsAs<AUnsignedInteger>());

				REQUIRE_FALSE(meta->InterpretsAs<AReal>());
				REQUIRE_FALSE(meta->InterpretsAs<ASigned>());
				REQUIRE_FALSE(meta->InterpretsAs<ASignedInteger>());

				REQUIRE(meta->InterpretsAs<ANumber>(1));
				REQUIRE(meta->InterpretsAs<AInteger>(1));
				REQUIRE(meta->InterpretsAs<AUnsigned>(1));
				REQUIRE(meta->InterpretsAs<AUnsignedInteger>(1));

				REQUIRE_FALSE(meta->InterpretsAs<AReal>(1));
				REQUIRE_FALSE(meta->InterpretsAs<ASigned>(1));
				REQUIRE_FALSE(meta->InterpretsAs<ASignedInteger>(1));

				REQUIRE_FALSE(meta->InterpretsAs<ANumber>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AInteger>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AReal>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsigned>(2));
				REQUIRE_FALSE(meta->InterpretsAs<ASigned>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsignedInteger>(2));
				REQUIRE_FALSE(meta->InterpretsAs<ASignedInteger>(2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Signed integer RTTI interpretation", "[metadata]", int8_t, int16_t, int32_t, int64_t) {
	GIVEN("A signed integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(meta->InterpretsAs<ANumber>());
				REQUIRE(meta->InterpretsAs<AInteger>());
				REQUIRE(meta->InterpretsAs<ASigned>());
				REQUIRE(meta->InterpretsAs<ASignedInteger>());

				REQUIRE_FALSE(meta->InterpretsAs<AReal>());
				REQUIRE_FALSE(meta->InterpretsAs<AUnsigned>());
				REQUIRE_FALSE(meta->InterpretsAs<AUnsignedInteger>());

				REQUIRE(meta->InterpretsAs<ANumber>(1));
				REQUIRE(meta->InterpretsAs<AInteger>(1));
				REQUIRE(meta->InterpretsAs<ASigned>(1));
				REQUIRE(meta->InterpretsAs<ASignedInteger>(1));

				REQUIRE_FALSE(meta->InterpretsAs<AReal>(1));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsigned>(1));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsignedInteger>(1));

				REQUIRE_FALSE(meta->InterpretsAs<ANumber>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AInteger>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AReal>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsigned>(2));
				REQUIRE_FALSE(meta->InterpretsAs<ASigned>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsignedInteger>(2));
				REQUIRE_FALSE(meta->InterpretsAs<ASignedInteger>(2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Real number RTTI interpretation", "[metadata]", float, double) {
	GIVEN("A real number type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(meta->InterpretsAs<ANumber>());
				REQUIRE(meta->InterpretsAs<AReal>());
				REQUIRE(meta->InterpretsAs<ASigned>());

				REQUIRE_FALSE(meta->InterpretsAs<AInteger>());
				REQUIRE_FALSE(meta->InterpretsAs<AUnsigned>());
				REQUIRE_FALSE(meta->InterpretsAs<AUnsignedInteger>());
				REQUIRE_FALSE(meta->InterpretsAs<ASignedInteger>());

				REQUIRE(meta->InterpretsAs<ANumber>(1));
				REQUIRE(meta->InterpretsAs<AReal>(1));
				REQUIRE(meta->InterpretsAs<ASigned>(1));

				REQUIRE_FALSE(meta->InterpretsAs<AInteger>(1));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsigned>(1));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsignedInteger>(1));
				REQUIRE_FALSE(meta->InterpretsAs<ASignedInteger>(1));

				REQUIRE_FALSE(meta->InterpretsAs<ANumber>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AInteger>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AReal>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsigned>(2));
				REQUIRE_FALSE(meta->InterpretsAs<ASigned>(2));
				REQUIRE_FALSE(meta->InterpretsAs<AUnsignedInteger>(2));
				REQUIRE_FALSE(meta->InterpretsAs<ASignedInteger>(2));
			}
		}
	}
}