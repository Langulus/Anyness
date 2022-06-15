///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>

SCENARIO("Text containers", "[text]") {
	GIVEN("Default text container") {
		#include "CollectGarbage.inl"

		Text text;

		WHEN("Nothing is done") {
			THEN("The capacity and size change") {
				REQUIRE_FALSE(text.IsAbstract());
				REQUIRE_FALSE(text.IsCompressed());
				REQUIRE_FALSE(text.IsConstant());
				REQUIRE_FALSE(text.IsDeep());
				REQUIRE_FALSE(text.IsSparse());
				REQUIRE_FALSE(text.IsEncrypted());
				REQUIRE_FALSE(text.IsFuture());
				REQUIRE_FALSE(text.IsPast());
				REQUIRE_FALSE(text.IsPhased());
				REQUIRE_FALSE(text.IsMissing());
				REQUIRE_FALSE(text.IsOr());
				REQUIRE_FALSE(text.IsStatic());
				REQUIRE_FALSE(text.IsUntyped());
				REQUIRE_FALSE(text.IsValid());
				REQUIRE_FALSE(text.IsAllocated());
				REQUIRE_FALSE(text.HasAuthority());
				REQUIRE(text.IsTypeConstrained());
				REQUIRE(text.GetType() == MetaData::Of<char8_t>());
				REQUIRE(text.Is<char8_t>());
				REQUIRE(text.IsNow());
				REQUIRE(text.IsInvalid());
				REQUIRE(text.IsDense());
				REQUIRE(text.IsConstructible());
				REQUIRE(text.IsEmpty());
				REQUIRE(text.GetCount() == 0);
				REQUIRE(text.GetReserved() == 0);
				REQUIRE(text.GetUses() == 0);
				REQUIRE(text == u8"");
				REQUIRE(text == "");
				REQUIRE(text == nullptr);
				REQUIRE_FALSE(text == u8"no match");
				REQUIRE_FALSE(text == "no match");
			}
		}

		WHEN("Capacity is reserved") {
			text.Allocate(500);

			THEN("The capacity and size change") {
				REQUIRE_FALSE(text.IsAbstract());
				REQUIRE_FALSE(text.IsCompressed());
				REQUIRE_FALSE(text.IsConstant());
				REQUIRE_FALSE(text.IsDeep());
				REQUIRE_FALSE(text.IsSparse());
				REQUIRE_FALSE(text.IsEncrypted());
				REQUIRE_FALSE(text.IsFuture());
				REQUIRE_FALSE(text.IsPast());
				REQUIRE_FALSE(text.IsPhased());
				REQUIRE_FALSE(text.IsMissing());
				REQUIRE_FALSE(text.IsOr());
				REQUIRE_FALSE(text.IsStatic());
				REQUIRE_FALSE(text.IsUntyped());
				REQUIRE_FALSE(text.IsValid());
				REQUIRE(text.IsTypeConstrained());
				REQUIRE(text.GetType() == MetaData::Of<char8_t>());
				REQUIRE(text.Is<char8_t>());
				REQUIRE(text.IsNow());
				REQUIRE(text.IsInvalid());
				REQUIRE(text.IsDense());
				REQUIRE(text.IsConstructible());
				REQUIRE(text.IsEmpty());
				REQUIRE(text.IsAllocated());
				REQUIRE(text.GetCount() == 0);
				REQUIRE(text.GetReserved() >= 500);
				REQUIRE(text.GetUses() == 1);
				REQUIRE(text.HasAuthority());
				REQUIRE(text == u8"");
				REQUIRE(text == "");
				REQUIRE_FALSE(text == nullptr);
				REQUIRE_FALSE(text == u8"no match");
				REQUIRE_FALSE(text == "no match");
			}
		}

		WHEN("Assigned to itself") {
			text = text;

			THEN("The capacity and size change") {
				REQUIRE_FALSE(text.IsAbstract());
				REQUIRE_FALSE(text.IsCompressed());
				REQUIRE_FALSE(text.IsConstant());
				REQUIRE_FALSE(text.IsDeep());
				REQUIRE_FALSE(text.IsSparse());
				REQUIRE_FALSE(text.IsEncrypted());
				REQUIRE_FALSE(text.IsFuture());
				REQUIRE_FALSE(text.IsPast());
				REQUIRE_FALSE(text.IsPhased());
				REQUIRE_FALSE(text.IsMissing());
				REQUIRE_FALSE(text.IsOr());
				REQUIRE_FALSE(text.IsStatic());
				REQUIRE_FALSE(text.IsUntyped());
				REQUIRE_FALSE(text.IsValid());
				REQUIRE_FALSE(text.IsAllocated());
				REQUIRE_FALSE(text.HasAuthority());
				REQUIRE(text.IsTypeConstrained());
				REQUIRE(text.GetType() == MetaData::Of<char8_t>());
				REQUIRE(text.Is<char8_t>());
				REQUIRE(text.IsNow());
				REQUIRE(text.IsInvalid());
				REQUIRE(text.IsDense());
				REQUIRE(text.IsConstructible());
				REQUIRE(text.IsEmpty());
				REQUIRE(text.GetCount() == 0);
				REQUIRE(text.GetReserved() == 0);
				REQUIRE(text.GetUses() == 0);
				REQUIRE(text == u8"");
				REQUIRE(text == "");
				REQUIRE(text == nullptr);
				REQUIRE_FALSE(text == u8"no match");
				REQUIRE_FALSE(text == "no match");
			}
		}
	}

	GIVEN("Uninitialized text container") {
		#include "CollectGarbage.inl"

		Text* text;

		WHEN("Constructed with a null-terminated literal") {
			text = new Text {"test1"};

			THEN("The capacity and size change") {
				REQUIRE_FALSE((*text).IsAbstract());
				REQUIRE_FALSE((*text).IsCompressed());
				REQUIRE_FALSE((*text).IsConstant());
				REQUIRE_FALSE((*text).IsDeep());
				REQUIRE_FALSE((*text).IsSparse());
				REQUIRE_FALSE((*text).IsEncrypted());
				REQUIRE_FALSE((*text).IsFuture());
				REQUIRE_FALSE((*text).IsPast());
				REQUIRE_FALSE((*text).IsPhased());
				REQUIRE_FALSE((*text).IsMissing());
				REQUIRE_FALSE((*text).IsOr());
				REQUIRE_FALSE((*text).IsUntyped());
				REQUIRE_FALSE((*text).IsInvalid());
				REQUIRE_FALSE((*text).IsStatic());
				REQUIRE((*text).IsTypeConstrained());
				REQUIRE((*text).IsDense());
				REQUIRE((*text).IsValid());
				REQUIRE((*text).IsAllocated());
				REQUIRE((*text).GetCount() == 5);
				REQUIRE((*text).GetReserved() >= 5);
				REQUIRE((*text).Is<char8_t>());
				REQUIRE((*text).GetRaw());
				REQUIRE((*text).HasAuthority());
				REQUIRE((*text) == u8"test1");
				REQUIRE((*text) == "test1");
				REQUIRE((*text)[0] == 't');
				REQUIRE((*text)[1] == 'e');
				REQUIRE((*text)[2] == 's');
				REQUIRE((*text)[3] == 't');
				REQUIRE((*text)[4] == '1');
			}

			delete text;
		}

		WHEN("Constructed with a count-terminated literal") {
			text = new Text {"test1", 5};

			THEN("The capacity and size change") {
				REQUIRE((*text).GetCount() == 5);
				REQUIRE((*text).GetReserved() >= 5);
				REQUIRE((*text).Is<char8_t>());
				REQUIRE((*text).GetRaw());
				REQUIRE((*text).HasAuthority());
				REQUIRE((*text) == u8"test1");
				REQUIRE((*text) == "test1");
				REQUIRE((*text)[0] == 't');
				REQUIRE((*text)[1] == 'e');
				REQUIRE((*text)[2] == 's');
				REQUIRE((*text)[3] == 't');
				REQUIRE((*text)[4] == '1');
			}

			delete text;
		}

		WHEN("Constructed with a c-array") {
			char8_t test1[] = u8"test1";
			text = new Text {test1};

			THEN("The capacity and size change") {
				REQUIRE((*text).GetCount() == 5);
				REQUIRE((*text).GetReserved() >= 5);
				REQUIRE((*text).Is<char8_t>());
				REQUIRE((*text).GetRaw());
				REQUIRE((*text).HasAuthority());
				REQUIRE((*text) == u8"test1");
				REQUIRE((*text) == "test1");
				REQUIRE((*text)[0] == 't');
				REQUIRE((*text)[1] == 'e');
				REQUIRE((*text)[2] == 's');
				REQUIRE((*text)[3] == 't');
				REQUIRE((*text)[4] == '1');
			}

			delete text;
		}

		WHEN("Constructed with a single character") {
			text = new Text {u8'?'};

			THEN("The capacity and size change") {
				REQUIRE((*text).GetCount() == 1);
				REQUIRE((*text).GetReserved() >= 1);
				REQUIRE((*text).Is<char8_t>());
				REQUIRE((*text).GetRaw());
				REQUIRE((*text).HasAuthority());
				REQUIRE((*text) == u8"?");
				REQUIRE((*text)[0] == '?');
			}

			delete text;
		}
	}

	GIVEN("Reserved text container") {
		#include "CollectGarbage.inl"

		Text text;
		text.Allocate(500);
		auto memory = text.GetRaw();

		WHEN("Text is extended") {
			auto region = text.Extend(10);

			THEN("The count change") {
				REQUIRE(text.GetCount() == 10);
				REQUIRE(text.GetReserved() >= 500);
				REQUIRE(text.GetRaw() == memory);
				REQUIRE(text.HasAuthority());
				REQUIRE(region.GetCount() == 10);
				REQUIRE(region.GetRaw() == memory);
			}
		}

		WHEN("Text is concatenated") {
			text += "test";

			THEN("The count change") {
				REQUIRE(text.GetCount() == 4);
				REQUIRE(text.GetReserved() >= 500);
				REQUIRE(text.GetRaw() == memory);
				REQUIRE(text.HasAuthority());
				REQUIRE(text == u8"test");
				REQUIRE(text == "test");
			}
		}

		WHEN("Text is cleared") {
			text += "test";
			text.Clear();

			THEN("Nothing should change") {
				REQUIRE(text.GetCount() == 0);
				REQUIRE(text.GetReserved() >= 500);
				REQUIRE(text.GetRaw() == memory);
				REQUIRE(text.HasAuthority());
				REQUIRE(text != u8"test");
				REQUIRE(text != "test");
			}
		}

		WHEN("Text is reset") {
			text += "test";
			text.Reset();

			THEN("Memory is released") {
				REQUIRE(text.GetCount() == 0);
				REQUIRE(text.GetReserved() == 0);
				REQUIRE(text.GetRaw() == nullptr);
				REQUIRE(text.GetType() == MetaData::Of<char8_t>());
				REQUIRE_FALSE(text.HasAuthority());
				REQUIRE(text != u8"test");
				REQUIRE(text != "test");
			}
		}
	}

	GIVEN("Full text container") {
		#include "CollectGarbage.inl"

		Text text {"test1"};
		auto memory = text.GetRaw();

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
			THEN("The new container shall contain the same data, in the same memory block; data will only be referenced") {
				REQUIRE(text.GetCount() == copy.GetCount());
				REQUIRE(text.GetReserved() == copy.GetReserved());
				REQUIRE(text.GetRaw() == copy.GetRaw());
				REQUIRE(text.GetType() == copy.GetType());
				REQUIRE(text.HasAuthority());
				REQUIRE(copy.HasAuthority());
				REQUIRE(copy.GetUses() == 2);
				REQUIRE(text.GetUses() == 2);
			}
		}

		WHEN("Text is cloned (deep copy)") {
			Text copy = text.Clone();
			THEN("The new container shall contain the same data, but in separate memory block") {
				REQUIRE(text.GetCount() == copy.GetCount());
				REQUIRE(text.GetReserved() >= copy.GetReserved());
				REQUIRE(text.GetRaw() != copy.GetRaw());
				REQUIRE(text.GetType() == copy.GetType());
				REQUIRE(text.HasAuthority());
				REQUIRE(copy.HasAuthority());
				REQUIRE(copy.GetUses() == 1);
				REQUIRE(text.GetUses() == 1);
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

TEMPLATE_TEST_CASE("Unsigned number stringification", "[text]", uint8_t, uint16_t, uint32_t, uint64_t) {
	Text* text;

	WHEN("Constructed with a number") {
		text = new Text{ TestType{66} };

		THEN("The capacity and size change") {
			REQUIRE((*text).GetCount() == 2);
			REQUIRE((*text).GetReserved() >= 2);
			REQUIRE((*text).Is<char8_t>());
			REQUIRE((*text).GetRaw());
			REQUIRE((*text).HasAuthority());
			REQUIRE((*text) == "66");
		}

		delete text;
	}
}

TEMPLATE_TEST_CASE("Signed number stringification", "[text]", int8_t, int16_t, int32_t, int64_t) {
	Text* text;

	WHEN("Constructed with a number") {
		text = new Text{ TestType{-66} };

		THEN("The capacity and size change") {
			REQUIRE((*text).GetCount() == 3);
			REQUIRE((*text).GetReserved() >= 3);
			REQUIRE((*text).Is<char8_t>());
			REQUIRE((*text).GetRaw());
			REQUIRE((*text).HasAuthority());
			REQUIRE((*text) == "-66");
		}

		delete text;
	}
}