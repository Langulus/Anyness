///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md			
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
	const Text serialized {ex};
	return ::std::string {Token {serialized}};
}

SCENARIO("Shared pointer manipulation", "[TPointer]") {
	GIVEN("A templated shared pointer") {
		Ptr<int> pointer;
		Ptr<int> pointer2;

		REQUIRE_FALSE(pointer.Get());
		REQUIRE_FALSE(pointer);
		REQUIRE(pointer == pointer2);

		WHEN("Create an instance") {
			IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

			pointer.New(5);

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE(*pointer == 5);
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer.GetUses() == 1);
			}
		}

		WHEN("Create and copy an instance") {
			IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

			pointer.New(5);
			pointer2 = pointer;

			THEN("Should have exactly two references and jurisdiction") {
				REQUIRE(pointer == pointer2);
				REQUIRE(*pointer == 5);
				REQUIRE(*pointer2 == 5);
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer2.HasAuthority());
				REQUIRE(pointer.GetUses() == 2);
				REQUIRE(pointer2.GetUses() == 2);
			}
		}

		WHEN("Create and move an instance") {
			IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

			pointer.New(5);
			pointer2 = ::std::move(pointer);

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE_FALSE(pointer);
				REQUIRE(pointer2);
				REQUIRE(*pointer2 == 5);
				REQUIRE_FALSE(pointer.HasAuthority());
				REQUIRE(pointer2.HasAuthority());
				REQUIRE(pointer.GetUses() == 0);
			}
		}

		WHEN("Overwrite an instance") {
			IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

			pointer.New(5);
			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				auto backup = pointer.Get();
			#endif
			pointer2.New(6);
			pointer = pointer2;

			THEN("Old memory should be freed, but still be in jurisdiction") {
				REQUIRE(pointer == pointer2);
				REQUIRE(*pointer == 6);
				REQUIRE(*pointer2 == 6);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(pointer.GetType(), backup));
					REQUIRE_FALSE(Allocator::Find(pointer.GetType(), backup));
				#endif
				REQUIRE(pointer2.HasAuthority());
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer.GetUses() == 2);
			}
		}
	}

	GIVEN("A templated shared pointer filled with items created via 'new' statement") {
		Ptr<Any> pointer;

		WHEN("Given an xvalue pointer created via `new` statement") {
			IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

			auto raw = new Any {3};
			const auto rawBackUp = raw;
			pointer = ::std::move(raw);

			THEN("Should have exactly two references and jurisdiction, if NEWDELETE and MANAGED_MEMORY features are enabled") {
				REQUIRE(pointer == rawBackUp);
				REQUIRE(*pointer == *rawBackUp);
				REQUIRE(raw == rawBackUp);
				#if LANGULUS_FEATURE(NEWDELETE)
					REQUIRE(pointer.HasAuthority());
					REQUIRE(pointer.GetReferences() == 2);
				#endif
			}
		}

		WHEN("Given an immediate xvalue pointer created via `new` statement - a very bad practice!") {
			IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

			pointer = new Any {3};

			THEN("Should have exactly two references and jurisdiction, if NEWDELETE and MANAGED_MEMORY features are enabled") {
			#if LANGULUS_FEATURE(NEWDELETE)
				REQUIRE(pointer.HasAuthority());
				REQUIRE(pointer.GetReferences() == 2);
			#endif
			}
		}

		#if LANGULUS_FEATURE(NEWDELETE)
			WHEN("Given an xvalue pointer and then reset") {
				Allocator::CollectGarbage();
				auto raw = new Any {3};
				pointer = ::std::move(raw);
				auto unused = Allocator::Free(pointer.GetType(), raw, 1);
				pointer = nullptr;

				THEN("Should have released the resources") {
					REQUIRE_FALSE(raw->HasAuthority());
					REQUIRE(Allocator::CheckAuthority(pointer.GetType(), raw));
					REQUIRE_FALSE(Allocator::Find(pointer.GetType(), raw));
					REQUIRE_FALSE(pointer.HasAuthority());
				}
			}
		#endif

		WHEN("Given an lvalue pointer") {
			IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

			const auto raw = new Any {4};
			pointer = raw;

			THEN("Should have exactly one reference and jurisdiction") {
				REQUIRE(pointer == raw);
				REQUIRE(*pointer == *raw);
				#if LANGULUS_FEATURE(NEWDELETE)
					REQUIRE(pointer.HasAuthority());
					REQUIRE(pointer.GetReferences() == 2);
				#endif
			}
		}
	}
}
