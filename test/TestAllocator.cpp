///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE("Testing Roof2 calls with small numbers", "[allocator]", uint8_t, uint16_t, uint32_t, uint64_t) {
	using T = TestType;
	const T numbers[] {
		0, 1, 2, 3, 4, 5, 6, 11, 16, 64, 99, 120, 200
	};
	const T results[] {
		0, 1, 2, 4, 4, 8, 8, 16, 16, 64, 128, 128, 0
	};
	static_assert(sizeof(numbers) == sizeof(results), "Oops");

	WHEN("Roof2 is executed") {
		THEN("Results should be correct") {
			for (int i = 0; i < sizeof(numbers) / sizeof(T); ++i) {
				if (numbers[i] <= 128 || sizeof(T) > 1)
					REQUIRE(Roof2<true>(numbers[i]) == results[i]);
				else
					REQUIRE_THROWS_AS(Roof2<true>(numbers[i]), Except::Overflow);
			}
		}
	}
}

SCENARIO("Testing allocator functions", "[allocator]") {
	GIVEN("An allocation") {
		Allocation* entry = nullptr;

		WHEN("Memory is allocated on the heap") {
			entry = Allocator::Allocate(512);

			THEN("Requirements should be met") {
				REQUIRE(entry->GetBlockStart() != nullptr);
				REQUIRE(entry->GetBlockStart() != reinterpret_cast<Byte*>(entry));
				REQUIRE(reinterpret_cast<Pointer>(entry) % Pointer {LANGULUS(ALIGN)} == 0);
				REQUIRE(reinterpret_cast<Pointer>(entry->GetBlockStart()) % Pointer {LANGULUS(ALIGN)} == 0);
				REQUIRE(entry->GetAllocatedSize() >= 512);
				REQUIRE(entry->GetBlockEnd() == entry->GetBlockStart() + entry->GetAllocatedSize());
				REQUIRE(entry->GetSize() % Size {LANGULUS(ALIGN)} == 0);
				REQUIRE(entry->GetBlockStart() == reinterpret_cast<Byte*>(entry) + entry->GetSize());
				REQUIRE(entry->GetUses() == 1);
				for (Size i = 0; i < 512; ++i) {
					auto p = entry->GetBlockStart() + i;
					REQUIRE(entry->Contains(p));
				}
				for (Size i = 512; i < 513; ++i) {
					auto p = entry->GetBlockStart() + i;
					REQUIRE_FALSE(entry->Contains(p));
				}
			}
		}

		WHEN("Referenced once") {
			entry = Allocator::Allocate(512);
			entry->Keep();

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 2);
			}
		}

		WHEN("Referenced multiple times") {
			entry = Allocator::Allocate(512);
			entry->Keep(5);

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 6);
			}
		}

		WHEN("Dereferenced once without deletion") {
			entry = Allocator::Allocate(512);
			entry->Keep();
			entry->Free<false>();

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 1);
			}
		}

		WHEN("Dereferenced multiple times without deletion") {
			entry = Allocator::Allocate(512);
			entry->Keep(5);
			entry->Free<false>(4);

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 2);
			}
		}

		WHEN("Dereferenced once with deletion") {
			entry = Allocator::Allocate(512);
			entry->Free<true>();

			THEN("We shouldn't be able to access the memory any longer") {
				REQUIRE_FALSE(Allocator::CheckAuthority(nullptr, entry));
			}
		}

		WHEN("Dereferenced multiple times with deletion") {
			entry = Allocator::Allocate(512);
			entry->Keep(5);
			entry->Free<true>(6);

			THEN("We shouldn't be able to access the memory any longer") {
				REQUIRE_FALSE(Allocator::CheckAuthority(nullptr, entry));
			}
		}
	}
}
