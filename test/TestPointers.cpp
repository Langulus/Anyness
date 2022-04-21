#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Shared pointer manipulation", "[TPointer]") {
	GIVEN("A templated shared pointer") {
		Ptr<int> pointer;
		Ptr<int> pointer2;

		REQUIRE(!pointer.Get());
		REQUIRE(!pointer);
		REQUIRE_FALSE(pointer);

		WHEN("Create an instance") {
			pointer = Ptr<int>::Create(5);

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE(*pointer == 5);
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer.GetReferences() == 1);
			}
		}

		WHEN("Create and copy an instance") {
			pointer = Ptr<int>::Create(5);
			pointer2 = pointer;

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer == pointer2);
				REQUIRE(*pointer == 5);
				REQUIRE(*pointer2 == 5);
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer2.HasAuthority());
				REQUIRE(pointer.GetReferences() == 2);
				REQUIRE(pointer2.GetReferences() == 2);
			}
		}

		WHEN("Create and move an instance") {
			pointer = Ptr<int>::Create(5);
			pointer2 = Move(pointer);

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE_FALSE(pointer);
				REQUIRE(pointer2);
				REQUIRE(*pointer2 == 5);
				REQUIRE_FALSE(pointer.HasAuthority());
				REQUIRE(pointer2.HasAuthority());
				REQUIRE(pointer.GetReferences() == 1);
			}
		}

		WHEN("Overwrite an instance") {
			pointer = Ptr<int>::Create(5);
			auto backup = pointer.Get();
			pointer2 = Ptr<int>::Create(6);
			pointer = pointer2;

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer == pointer2);
				REQUIRE(*pointer == 6);
				REQUIRE(*pointer2 == 6);
				REQUIRE(PCMEMORY.CheckJurisdiction(pointer.GetMeta(), backup));
				REQUIRE(!PCMEMORY.CheckUsage(pointer.GetMeta(), backup));
				REQUIRE(pointer2.HasAuthority());
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer.GetReferences() == 2);
			}
		}
	}

	GIVEN("A templated shared pointer filled with deep items") {
		Ptr<Any> pointer;

		WHEN("Given an xvalue pointer") {
			auto raw = new Any {3};
			const auto rawBackUp = raw;
			pointer = Move(raw);

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer == rawBackUp);
				REQUIRE(*pointer == *rawBackUp);
				REQUIRE(raw == rawBackUp);
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer.GetReferences() == 2);
			}
		}

		WHEN("Given an immediate xvalue pointer - a very bad practice!") {
			pointer = new Any {3};

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer.GetReferences() == 2);
			}
		}

		WHEN("Given an xvalue pointer and then reset") {
			auto raw = new Any {3};
			pointer = Move(raw);
			PCMEMORY.Reference(pointer.GetMeta(), raw, -1);
			pointer = nullptr;

			THEN("Should have released the resources") {
				REQUIRE(!raw->CheckJurisdiction());
				REQUIRE(PCMEMORY.CheckJurisdiction(pointer.GetMeta(), raw));
				REQUIRE(!PCMEMORY.CheckUsage(pointer.GetMeta(), raw));
				REQUIRE(!pointer.HasAuthority());
			}
		}

		WHEN("Given an lvalue pointer") {
			const auto raw = new Any {4};
			pointer = raw;

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE(pointer == raw);
				REQUIRE(*pointer == *raw);
				REQUIRE(pointer.HasAuthority());
				REQUIRE(2 == pointer.GetReferences());
			}
		}
	}
}
