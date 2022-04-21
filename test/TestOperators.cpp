#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Testing new/delete operators", "[new][delete]") {
	GIVEN("POD dynamic memory allocated with overriden new operator") {
		auto meta = MetaData::Of<int>();
		auto a = new int[1024];
		int static_memory[64] = {};

		REQUIRE(PCMEMORY.CheckUsage(meta, a));
		REQUIRE(PCMEMORY.CheckJurisdiction(meta, a));
		REQUIRE(PCMEMORY.GetReferences(meta, a) == 1);

		REQUIRE_FALSE(PCMEMORY.CheckUsage(meta, static_memory));
		REQUIRE_FALSE(PCMEMORY.CheckJurisdiction(meta, static_memory));
		REQUIRE(PCMEMORY.GetReferences(meta, static_memory) == 1);

		PCMEMORY.Reference(nullptr, a, 1);

		WHEN("Referencing the dynamic memory") {
			PCMEMORY.Reference(nullptr, a, 1);
			THEN("Reference count increases") {
				REQUIRE(PCMEMORY.CheckUsage(meta, a) == true);
				REQUIRE(PCMEMORY.CheckJurisdiction(meta, a) == true);
				REQUIRE(PCMEMORY.GetReferences(meta, a) == 3);
			}
		}

		WHEN("Dereferencing the dynamic memory") {
			PCMEMORY.Reference(nullptr, a, -1);
			THEN("Reference count decreases, and in this case - memory is freed") {
				REQUIRE(PCMEMORY.CheckJurisdiction(meta, a) == true);
				REQUIRE(PCMEMORY.CheckUsage(meta, a) == true);
				REQUIRE(PCMEMORY.GetReferences(meta, a) == 1);

				PCMEMORY.Reference(nullptr, a, -1);

				REQUIRE(PCMEMORY.CheckJurisdiction(meta, a) == true);
				REQUIRE(PCMEMORY.CheckUsage(meta, a) == false);
				REQUIRE(PCMEMORY.GetReferences(meta, a) == 0);
			}
		}

		WHEN("Using delete operator on the allocated memory") {
			THEN("Block is deallocated, regardless of number of references") {
				delete[] a;
				REQUIRE(PCMEMORY.CheckUsage(meta, a) == false);
				REQUIRE(PCMEMORY.CheckJurisdiction(meta, a) == true);
				REQUIRE(PCMEMORY.GetReferences(meta, a) == 0);
			}
		}

		WHEN("Deallocate and reallocate memory once") {
			const auto initialState = PCMEMORY.GetStats();
			delete[] a;
			a = new int[1024];
			REQUIRE(initialState == PCMEMORY.GetStats());
		}

		WHEN("Deallocate and reallocate memory repeatedly") {
			const auto initialState = PCMEMORY.GetStats();
			for (pcptr i = 0; i < 10000u; ++i) {
				delete[] a;
				a = new int[1024];
			}
			REQUIRE(initialState == PCMEMORY.GetStats());
		}
	}
}

/*TEST_CASE("Fibonacci") {
	BENCHMARK("Fibonacci 20") {
		auto a = new int[1];
		delete[] a;
	};

	BENCHMARK("Fibonacci 25") {
		auto a = new int[100];
		delete[] a;
	};

	BENCHMARK("Fibonacci 30") {
		auto a = new int[1000];
		delete[] a;
	};

	BENCHMARK("Fibonacci 35") {
		auto a = new int[10000];
		delete[] a;
	};
}*/
