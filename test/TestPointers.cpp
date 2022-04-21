#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Shared pointer manipulation", "[TPointer]") {
	GIVEN("A templated shared pointer") {
		TPointer<int> pointer;
		TPointer<int> pointer2;

		REQUIRE(!pointer.Get());
		REQUIRE(!pointer);
		REQUIRE(pointer == nullptr);

		WHEN("Create an instance") {
			pointer = TPointer<int>::Create(5);

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE(*pointer == 5);
				REQUIRE(pointer.CheckJurisdiction());
				REQUIRE(1 == pointer.GetBlockReferences());
			}
		}

		WHEN("Create and copy an instance") {
			pointer = TPointer<int>::Create(5);
			pointer2 = pointer;

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer == pointer2);
				REQUIRE(*pointer == 5);
				REQUIRE(*pointer2 == 5);
				REQUIRE(pointer.CheckJurisdiction());
				REQUIRE(pointer2.CheckJurisdiction());
				REQUIRE(2 == pointer.GetBlockReferences());
				REQUIRE(2 == pointer2.GetBlockReferences());
			}
		}

		WHEN("Create and move an instance") {
			pointer = TPointer<int>::Create(5);
			pointer2 = pcMove(pointer);

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE(pointer == nullptr);
				REQUIRE(pointer2 != nullptr);
				REQUIRE(*pointer2 == 5);
				REQUIRE(!pointer.CheckJurisdiction());
				REQUIRE(pointer2.CheckJurisdiction());
				REQUIRE(1 == pointer.GetBlockReferences());
			}
		}

		WHEN("Overwrite an instance") {
			pointer = TPointer<int>::Create(5);
			auto backup = pointer.Get();
			pointer2 = TPointer<int>::Create(6);
			pointer = pointer2;

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer == pointer2);
				REQUIRE(*pointer == 6);
				REQUIRE(*pointer2 == 6);
				REQUIRE(PCMEMORY.CheckJurisdiction(pointer.GetMeta(), backup));
				REQUIRE(!PCMEMORY.CheckUsage(pointer.GetMeta(), backup));
				REQUIRE(pointer2.CheckJurisdiction());
				REQUIRE(pointer.CheckJurisdiction());
				REQUIRE(2 == pointer.GetBlockReferences());
			}
		}
	}

	GIVEN("A templated shared pointer filled with deep items") {
		TPointer<Any> pointer;

		WHEN("Given an xvalue pointer") {
			auto raw = new Any{ 3 };
			const auto rawBackUp = raw;
			pointer = pcMove(raw);

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer == rawBackUp);
				REQUIRE(*pointer == *rawBackUp);
				REQUIRE(raw == rawBackUp);
				REQUIRE(pointer.CheckJurisdiction());
				REQUIRE(2 == pointer.GetBlockReferences());
			}
		}

		WHEN("Given an immediate xvalue pointer - a very bad practice!") {
			pointer = new Any{ 3 };

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer.CheckJurisdiction());
				REQUIRE(2 == pointer.GetBlockReferences());
			}
		}

		WHEN("Given an xvalue pointer and then reset") {
			auto raw = new Any{ 3 };
			pointer = pcMove(raw);
			PCMEMORY.Reference(pointer.GetMeta(), raw, -1);
			pointer = nullptr;

			THEN("Should have released the resources") {
				REQUIRE(!raw->CheckJurisdiction());
				REQUIRE(PCMEMORY.CheckJurisdiction(pointer.GetMeta(), raw));
				REQUIRE(!PCMEMORY.CheckUsage(pointer.GetMeta(), raw));
				REQUIRE(!pointer.CheckJurisdiction());
			}
		}

		WHEN("Given an lvalue pointer") {
			const auto raw = new Any{ 4 };
			pointer = raw;

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE(pointer == raw);
				REQUIRE(*pointer == *raw);
				REQUIRE(pointer.CheckJurisdiction());
				REQUIRE(2 == pointer.GetBlockReferences());
			}
		}
	}
}
