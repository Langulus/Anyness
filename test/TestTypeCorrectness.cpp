#include "TestMain.hpp"
#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE("Unsigned integer RTTI interpretation", "[metadata]", pcu8, pcu16, pcu32, pcu64) {
	GIVEN("An unsigned integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(meta->template InterpretsAs<ANumber>());
				REQUIRE(meta->template InterpretsAs<AInteger>());
				REQUIRE(!meta->template InterpretsAs<AReal>());
				REQUIRE(meta->template InterpretsAs<AUnsigned>());
				REQUIRE(!meta->template InterpretsAs<ASigned>());
				REQUIRE(meta->template InterpretsAs<AUnsignedInt>());
				REQUIRE(!meta->template InterpretsAs<ASignedInt>());

				REQUIRE(meta->template InterpretsAs<ANumber>(1));
				REQUIRE(meta->template InterpretsAs<AInteger>(1));
				REQUIRE(!meta->template InterpretsAs<AReal>(1));
				REQUIRE(meta->template InterpretsAs<AUnsigned>(1));
				REQUIRE(!meta->template InterpretsAs<ASigned>(1));
				REQUIRE(meta->template InterpretsAs<AUnsignedInt>(1));
				REQUIRE(!meta->template InterpretsAs<ASignedInt>(1));

				REQUIRE(!meta->template InterpretsAs<ANumber>(2));
				REQUIRE(!meta->template InterpretsAs<AInteger>(2));
				REQUIRE(!meta->template InterpretsAs<AReal>(2));
				REQUIRE(!meta->template InterpretsAs<AUnsigned>(2));
				REQUIRE(!meta->template InterpretsAs<ASigned>(2));
				REQUIRE(!meta->template InterpretsAs<AUnsignedInt>(2));
				REQUIRE(!meta->template InterpretsAs<ASignedInt>(2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Signed integer RTTI interpretation", "[metadata]", pci8, pci16, pci32, pci64) {
	GIVEN("A signed integer type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(meta->template InterpretsAs<ANumber>());
				REQUIRE(meta->template InterpretsAs<AInteger>());
				REQUIRE(!meta->template InterpretsAs<AReal>());
				REQUIRE(!meta->template InterpretsAs<AUnsigned>());
				REQUIRE(meta->template InterpretsAs<ASigned>());
				REQUIRE(!meta->template InterpretsAs<AUnsignedInt>());
				REQUIRE(meta->template InterpretsAs<ASignedInt>());

				REQUIRE(meta->template InterpretsAs<ANumber>(1));
				REQUIRE(meta->template InterpretsAs<AInteger>(1));
				REQUIRE(!meta->template InterpretsAs<AReal>(1));
				REQUIRE(!meta->template InterpretsAs<AUnsigned>(1));
				REQUIRE(meta->template InterpretsAs<ASigned>(1));
				REQUIRE(!meta->template InterpretsAs<AUnsignedInt>(1));
				REQUIRE(meta->template InterpretsAs<ASignedInt>(1));

				REQUIRE(!meta->template InterpretsAs<ANumber>(2));
				REQUIRE(!meta->template InterpretsAs<AInteger>(2));
				REQUIRE(!meta->template InterpretsAs<AReal>(2));
				REQUIRE(!meta->template InterpretsAs<AUnsigned>(2));
				REQUIRE(!meta->template InterpretsAs<ASigned>(2));
				REQUIRE(!meta->template InterpretsAs<AUnsignedInt>(2));
				REQUIRE(!meta->template InterpretsAs<ASignedInt>(2));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Real number RTTI interpretation", "[metadata]", pcr32, pcr64) {
	GIVEN("A real number type") {
		auto meta = MetaData::Of<TestType>();
		REQUIRE(meta != nullptr);

		WHEN("Interpreted as another type") {
			THEN("Requirements should be met") {
				REQUIRE(meta->template InterpretsAs<ANumber>());
				REQUIRE(!meta->template InterpretsAs<AInteger>());
				REQUIRE(meta->template InterpretsAs<AReal>());
				REQUIRE(!meta->template InterpretsAs<AUnsigned>());
				REQUIRE(meta->template InterpretsAs<ASigned>());
				REQUIRE(!meta->template InterpretsAs<AUnsignedInt>());
				REQUIRE(!meta->template InterpretsAs<ASignedInt>());

				REQUIRE(meta->template InterpretsAs<ANumber>(1));
				REQUIRE(!meta->template InterpretsAs<AInteger>(1));
				REQUIRE(meta->template InterpretsAs<AReal>(1));
				REQUIRE(!meta->template InterpretsAs<AUnsigned>(1));
				REQUIRE(meta->template InterpretsAs<ASigned>(1));
				REQUIRE(!meta->template InterpretsAs<AUnsignedInt>(1));
				REQUIRE(!meta->template InterpretsAs<ASignedInt>(1));

				REQUIRE(!meta->template InterpretsAs<ANumber>(2));
				REQUIRE(!meta->template InterpretsAs<AInteger>(2));
				REQUIRE(!meta->template InterpretsAs<AReal>(2));
				REQUIRE(!meta->template InterpretsAs<AUnsigned>(2));
				REQUIRE(!meta->template InterpretsAs<ASigned>(2));
				REQUIRE(!meta->template InterpretsAs<AUnsignedInt>(2));
				REQUIRE(!meta->template InterpretsAs<ASignedInt>(2));
			}
		}
	}
}