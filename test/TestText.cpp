#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Text manipulation", "[text]") {
	GIVEN("An empty utf8 text container") {
		Text text;

		WHEN("More capacity is reserved, via Extend()") {
			text.Allocate(500);
			auto memory = text.GetRaw();

			REQUIRE(text.IsEmpty());
			REQUIRE(text.GetCount() == 0);
			REQUIRE(text.GetReserved() >= 500);

			auto region = text.Extend(10);
			THEN("The capacity and size change") {
				REQUIRE(text.GetCount() == 10);
				REQUIRE(text.GetReserved() >= 500);
				REQUIRE(text.GetRaw() == memory);
				REQUIRE(text.HasAuthority());
				REQUIRE(region.GetCount() == 10);
				REQUIRE(region.GetRaw() == memory);
			}
		}
	}

	GIVEN("A filled utf8 text container") {
		Text text {"test1"};
		auto memory = text.GetRaw();

		REQUIRE(text.GetCount() == 5);
		REQUIRE(text.GetReserved() >= 5);
		REQUIRE(text.Is<char8_t>());
		REQUIRE(text.GetRaw());
		REQUIRE(text.HasAuthority());
		REQUIRE(text == "test1");
		REQUIRE(text[0] == 't');
		REQUIRE(text[1] == 'e');
		REQUIRE(text[2] == 's');
		REQUIRE(text[3] == 't');
		REQUIRE(text[4] == '1');

		WHEN("Add more text") {
			text += "test2";
			THEN("The size and capacity change, type will never change, and memory won't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(text == "test1test2");
				REQUIRE(text.GetCount() == 10);
				REQUIRE(text.GetReserved() >= 10);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(text.GetRaw() == memory);
				#endif
				REQUIRE(text.HasAuthority());
				REQUIRE(text.Is<char8_t>());
			}
		}

		/*WHEN("We select parts of text") {
			auto selection = text.Select("st");
			THEN("Nothing really changes, but the selection must be valid") {
				REQUIRE(selection[0] == 's');
				REQUIRE(selection[1] == 't');
				REQUIRE(selection.GetText().GetRaw() == text.GetRaw());
				REQUIRE(selection.GetEnd() == 4);
				REQUIRE(selection.GetStart() == 2);
			}
		}

		WHEN("The size is reduced by removing elements") {
			text.Select("st").Delete();
			THEN("The size changes but not capacity") {
				REQUIRE(text[0] == 't');
				REQUIRE(text[1] == 'e');
				REQUIRE(text[2] == 's');
				REQUIRE(text.GetCount() == 3);
				REQUIRE(text.GetReserved() >= 5);
				REQUIRE(text.GetBytes() == memory);
				REQUIRE(true == text.CheckJurisdiction());
			}
		}*/

		WHEN("More capacity is reserved") {
			text.Allocate(20);
			THEN("The capacity changes but not the size, memory will move in order to have jurisdiction") {
				REQUIRE(text.GetCount() == 5);
				REQUIRE(text.GetReserved() >= 20);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(text.GetRaw() == memory);
				#endif
				REQUIRE(text.HasAuthority());
			}
		}

		WHEN("More capacity is reserved, via Extend()") {
			auto region = text.Extend(10);
			THEN("The capacity and size change") {
				REQUIRE(text.GetCount() == 15);
				REQUIRE(text.GetReserved() >= 15);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(text.GetRaw() == memory);
				#endif
				REQUIRE(text.HasAuthority());
				REQUIRE(region.GetCount() == 10);
				REQUIRE(region.GetRaw() == text.GetRaw() + 5);
			}
		}

		WHEN("Less capacity is reserved") {
			text.Allocate(2);
			THEN("Capacity is not changed, but count is trimmed; memory will not move, and memory will still be outside jurisdiction") {
				REQUIRE(text.GetCount() == 2);
				REQUIRE(text.GetReserved() >= 5);
				REQUIRE(text.GetRaw() == memory);
				REQUIRE(text.HasAuthority());
			}
		}

		WHEN("Text is cleared") {
			text.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(text.GetCount() == 0);
				REQUIRE(text.GetReserved() >= 5);
				REQUIRE(text.GetRaw() == memory);
				REQUIRE(text.HasAuthority());
				REQUIRE(text.Is<char8_t>());
			}
		}

		WHEN("Text is reset") {
			text.Reset();
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(text.GetCount() == 0);
				REQUIRE(text.GetReserved() == 0);
				REQUIRE_FALSE(text.GetRaw());
				REQUIRE(text.Is<char8_t>());
			}
		}

		WHEN("Text is copied shallowly") {
			Text copy = text;
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(text.GetCount() == copy.GetCount());
				REQUIRE(text.GetReserved() == copy.GetReserved());
				REQUIRE(text.GetRaw() == copy.GetRaw());
				REQUIRE(text.GetType() == copy.GetType());
				REQUIRE(text.HasAuthority());
				REQUIRE(copy.HasAuthority());
				REQUIRE(copy.GetReferences() == 2);
				REQUIRE(text.GetReferences() == 2);
			}
		}

		WHEN("Text is copied deeply") {
			Text copy = text.Clone();
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(text.GetCount() == copy.GetCount());
				REQUIRE(text.GetReserved() == copy.GetReserved());
				REQUIRE(text.GetRaw() != copy.GetRaw());
				REQUIRE(text.GetType() == copy.GetType());
				REQUIRE(text.HasAuthority());
				REQUIRE(copy.HasAuthority());
				REQUIRE(copy.GetReferences() == 1);
				REQUIRE(text.GetReferences() == 1);
			}
		}

		WHEN("Text is reset, then allocated again") {
			text.Reset();
			text += "kurec";
			THEN("Block manager should reuse the memory") {
				REQUIRE(text.GetCount() == 5);
				REQUIRE(text.GetReserved() >= 5);
				REQUIRE(text.HasAuthority());
				REQUIRE(text.Is<char8_t>());
			}
		}

		WHEN("Texts are compared") {
			THEN("The results should match") {
				REQUIRE(text == "test1");
				REQUIRE(text != "Tests");
			}
		}
	}
}
