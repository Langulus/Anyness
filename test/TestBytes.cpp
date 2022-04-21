#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Byte manipulation", "[bytes]") {
	GIVEN("An empty byte container") {
		Bytes data;

		WHEN("More capacity is reserved, via Extend()") {
			data.Allocate(500);
			auto memory = data.GetBytes();

			REQUIRE(data.IsEmpty());
			REQUIRE(data.GetCount() == 0);
			REQUIRE(data.GetReserved() >= 500);

			auto region = data.Extend(10);
			THEN("The capacity and size change") {
				REQUIRE(data.GetCount() == 10);
				REQUIRE(data.GetReserved() >= 500);
				REQUIRE(data.GetBytes() == memory);
				REQUIRE(data.CheckJurisdiction());

				REQUIRE(region.GetCount() == 10);
				REQUIRE(region.GetRaw() == memory);
			}
		}
	}

	GIVEN("A filled byte container") {
		const int randomStuff[] = { 1, 2, 3, 4, 5 };
		Bytes data{ randomStuff, sizeof(randomStuff) };
		auto memory = data.GetBytes();

		REQUIRE(data.GetCount() == 5 * sizeof(int));
		REQUIRE(data.GetReserved() >= 5 * sizeof(int));
		REQUIRE(data.Is<pcbyte>());
		REQUIRE(data.GetBytes() != nullptr);
		REQUIRE(data.CheckJurisdiction());

		WHEN("Add more bytes") {
			const int moreRandomStuff[] = { 1, 2, 3 };
			data += Bytes{ moreRandomStuff, 3 * sizeof(int) };
			THEN("The size and capacity change, type will never change") {
				REQUIRE(data.GetCount() == 8 * sizeof(int));
				REQUIRE(data.GetReserved() >= 8 * sizeof(int));
				REQUIRE(data.GetBytes() == memory);
				REQUIRE(data.CheckJurisdiction());
				REQUIRE(data.Is<pcbyte>());
			}
		}

		WHEN("More byte capacity is reserved") {
			data.Allocate(40);
			THEN("The capacity changes but not the size, memory will move in order to have jurisdiction") {
				REQUIRE(data.GetCount() == 5 * sizeof(int));
				REQUIRE(data.GetReserved() >= 40);
				REQUIRE(data.GetBytes() == memory);
				REQUIRE(data.CheckJurisdiction());
			}
		}

		WHEN("More byte capacity is reserved, via Extend()") {
			auto region = data.Extend(10);
			THEN("The capacity and size change") {
				REQUIRE(data.GetCount() == 5 * sizeof(int) + 10);
				REQUIRE(data.GetReserved() >= 5 * sizeof(int) + 10);
				REQUIRE(data.GetBytes() == memory);
				REQUIRE(data.CheckJurisdiction());

				REQUIRE(region.GetCount() == 10);
				REQUIRE(region.GetRaw() == memory + 5 * sizeof(int));
			}
		}

		WHEN("Less capacity is reserved") {
			data.Allocate(2);
			THEN("Capacity is not changed, but count is trimmed; memory will not move, and memory will still be outside jurisdiction") {
				REQUIRE(data.GetCount() == 2);
				REQUIRE(data.GetReserved() >= 5);
				REQUIRE(data.GetBytes() == memory);
				REQUIRE(data.CheckJurisdiction());
			}
		}

		WHEN("Bytes are cleared") {
			data.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(data.GetCount() == 0);
				REQUIRE(data.GetReserved() >= 5);
				REQUIRE(data.GetBytes() == memory);
				REQUIRE(data.CheckJurisdiction());
				REQUIRE(data.Is<pcbyte>());
			}
		}

		WHEN("Bytes are reset") {
			data.Reset();
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(data.GetCount() == 0);
				REQUIRE(data.GetReserved() == 0);
				REQUIRE(data.GetBytes() == nullptr);
				REQUIRE(data.Is<pcbyte>());
			}
		}

		WHEN("Bytes are copied shallowly") {
			Bytes copy = data;
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(data.GetCount() == copy.GetCount());
				REQUIRE(data.GetReserved() == copy.GetReserved());
				REQUIRE(data.GetBytes() == copy.GetBytes());
				REQUIRE(data.GetDataID() == copy.GetDataID());
				REQUIRE(data.CheckJurisdiction());
				REQUIRE(copy.CheckJurisdiction());
				REQUIRE(copy.GetBlockReferences() == 2);
				REQUIRE(data.GetBlockReferences() == 2);
			}
		}

		WHEN("Bytes are cloned") {
			Bytes copy = data.Clone();
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(data.GetCount() == copy.GetCount());
				REQUIRE(data.GetReserved() == copy.GetReserved());
				REQUIRE(data.GetBytes() != copy.GetBytes());
				REQUIRE(data.GetDataID() == copy.GetDataID());
				REQUIRE(data.CheckJurisdiction());
				REQUIRE(copy.CheckJurisdiction());
				REQUIRE(copy.GetBlockReferences() == 1);
				REQUIRE(data.GetBlockReferences() == 1);
			}
		}

		WHEN("Bytes are reset, then allocated again") {
			const int randomStuff2[] = { 4, 5, 6, 7, 8, 9 };
			data.Reset();
			data += Bytes{ randomStuff2, sizeof(randomStuff2) };
			THEN("Block manager should reuse the memory") {
				REQUIRE(data.GetCount() == sizeof(int) * 6);
				REQUIRE(data.GetReserved() >= sizeof(int) * 6);
				REQUIRE(data.GetBytes() != memory);
				REQUIRE(data.CheckJurisdiction());
				REQUIRE(data.Is<pcbyte>());
			}
		}

		WHEN("Bytes are compared") {
			THEN("The results should match") {
				const int randomStuff2[] = { 4, 5, 6, 7, 8, 9 };
				REQUIRE(data == Bytes{ randomStuff, sizeof(randomStuff) });
				REQUIRE(data != Bytes{ randomStuff2, sizeof(randomStuff2) });
			}
		}
	}
}