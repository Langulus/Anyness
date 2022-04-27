#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Testing new/delete operators", "[new][delete]") {
	GIVEN("POD dynamic memory allocated with overriden new operator") {
		auto meta = MetaData::Of<int>();
		auto a = new int[1024];
		int static_memory[64] = {};

		REQUIRE(Allocator::Find(meta, a));
		REQUIRE(Allocator::CheckAuthority(meta, a));
		REQUIRE(Allocator::GetReferences(meta, a) == 1);

		REQUIRE_FALSE(Allocator::Find(meta, static_memory));
		REQUIRE_FALSE(Allocator::CheckAuthority(meta, static_memory));
		REQUIRE(Allocator::GetReferences(meta, static_memory) == 1);

		Allocator::Keep(nullptr, a, 1);

		WHEN("Referencing the dynamic memory") {
			Allocator::Keep(nullptr, a, 1);
			THEN("Reference count increases") {
				REQUIRE(Allocator::Find(meta, a));
				REQUIRE(Allocator::CheckAuthority(meta, a));
				REQUIRE(Allocator::GetReferences(meta, a) == 3);
			}
		}

		WHEN("Dereferencing the dynamic memory") {
			auto unused = Allocator::Free(nullptr, a, -1);
			THEN("Reference count decreases, and in this case - memory is freed") {
				REQUIRE(Allocator::CheckAuthority(meta, a));
				REQUIRE(Allocator::Find(meta, a));
				REQUIRE(Allocator::GetReferences(meta, a) == 1);

				auto unused2 = Allocator::Free(nullptr, a, -1);

				REQUIRE(Allocator::CheckAuthority(meta, a));
				REQUIRE_FALSE(Allocator::Find(meta, a));
				REQUIRE(Allocator::GetReferences(meta, a) == 0);
			}
		}

		WHEN("Using delete operator on the allocated memory") {
			THEN("Block is deallocated, regardless of number of references") {
				delete[] a;
				REQUIRE_FALSE(Allocator::Find(meta, a));
				REQUIRE(Allocator::CheckAuthority(meta, a));
				REQUIRE(Allocator::GetReferences(meta, a) == 0);
			}
		}

		WHEN("Deallocate and reallocate memory once") {
			const auto initialState = Allocator::GetStatistics();
			delete[] a;
			a = new int[1024];
			REQUIRE(initialState == Allocator::GetStatistics());
		}

		WHEN("Deallocate and reallocate memory repeatedly") {
			const auto initialState = Allocator::GetStatistics();
			for (Count i = 0; i < 10000u; ++i) {
				delete[] a;
				a = new int[1024];
			}
			REQUIRE(initialState == Allocator::GetStatistics());
		}
	}
}
