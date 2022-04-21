#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Any", "[containers]") {
	GIVEN("An Any instance") {
		int original_value = 555;
		auto meta = MetaData::Of<int>();
		auto metas = MetaData::Of<int*>();
		Text original_pct = "Lorep Ipsum";
		Any pack;

		REQUIRE(meta);
		REQUIRE(metas);
		REQUIRE_FALSE(pack.GetType());
		REQUIRE_FALSE(pack.GetRaw());

		WHEN("Using new statements") {
			int* original_int = new int(original_value);

			THEN("We should have jurisdiction over that memory") {
				REQUIRE(original_int != nullptr);
				REQUIRE(PCMEMORY.CheckJurisdiction(metas, original_int));
				REQUIRE(PCMEMORY.GetReferences(metas, original_int) == 1);
			}
		}

		WHEN("Given a POD value by copy") {
			pack = original_value;
			THEN("Various traits change") {
				REQUIRE(pack.GetMeta() == meta);
				REQUIRE(pack.GetDataID().Is<int>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == original_value);
				REQUIRE_THROWS(pack.As<float>() == float(original_value));
				REQUIRE(*pack.As<int*>() == original_value);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
			}
		}

		WHEN("Given a dense Trait") {
			pack = Trait::From<Traits::Count>(5);
			THEN("Various traits change") {
				REQUIRE(pack.GetMeta() == MetaData::Of<Trait>());
				REQUIRE(pack.GetDataID().Is<Trait>());
				REQUIRE(pack.GetRaw() != nullptr);
			}
		}

		WHEN("Given a POD value by move") {
			pack = pcMove(original_value);
			THEN("Various traits change") {
				REQUIRE(pack.GetMeta() == meta);
				REQUIRE(pack.GetDataID().Is<int>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == original_value);
				REQUIRE_THROWS(pack.As<float>() == float(original_value));
				REQUIRE(*pack.As<int*>() == original_value);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
			}
		}

		WHEN("Given a sparse value") {
			int* original_int = new int(original_value);
			pack = original_int;

			THEN("Various traits change") {
				REQUIRE(pack.GetMeta() == meta->GetSparseMeta());
				REQUIRE(pack.GetDataID().Is<int*>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == original_value);
				REQUIRE_THROWS(pack.As<float>() == float(original_value));
				REQUIRE(*pack.As<int*>() == original_value);
				REQUIRE(pack.As<int*>() == original_int);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				REQUIRE(PCMEMORY.CheckJurisdiction(metas, original_int));
				REQUIRE(PCMEMORY.GetReferences(metas, original_int) == 2);
			}
		}

		WHEN("Given a sparse value by move") {
			int* original_int = new int(original_value);
			int* original_int_backup = original_int;
			pack = pcMove(original_int);

			THEN("Various traits change, pointer remains valid") {
				REQUIRE(original_int == original_int_backup);
				REQUIRE(pack.GetMeta() == meta->GetSparseMeta());
				REQUIRE(pack.GetDataID().Is<int*>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == original_value);
				REQUIRE_THROWS(pack.As<float>() == float(original_value));
				REQUIRE(*pack.As<int*>() == original_value);
				REQUIRE(pack.As<int*>() == original_int_backup);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				REQUIRE(PCMEMORY.CheckJurisdiction(metas, original_int_backup));
				REQUIRE(PCMEMORY.GetReferences(metas, original_int_backup) == 2);
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}

		WHEN("Given a sparse value and then copied from Any") {
			int* original_int = new int(original_value);
			pack = original_int;
			Any another_pack = pack;

			THEN("Various traits change") {
				REQUIRE(another_pack.GetMeta() == meta->GetSparseMeta());
				REQUIRE(another_pack.GetDataID().Is<int*>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.As<int>() == original_value);
				REQUIRE_THROWS(another_pack.As<float>() == float(original_value));
				REQUIRE(*another_pack.As<int*>() == original_value);
				REQUIRE(another_pack.As<int*>() == original_int);
				REQUIRE_THROWS(another_pack.As<float*>() == nullptr);
				REQUIRE(PCMEMORY.CheckJurisdiction(metas, original_int));
				REQUIRE(PCMEMORY.GetReferences(metas, original_int) == 2);
				REQUIRE(pack.GetBlockReferences() == another_pack.GetBlockReferences());
				REQUIRE(pack.GetBlockReferences() == 2);
			}
		}

		WHEN("Given a sparse value and then moved from Any") {
			int* original_int = new int(original_value);
			pack = original_int;
			Any another_pack = pcMove(pack);

			THEN("Entire container is moved to the new place without referencing anything") {
				REQUIRE(pack.GetMeta() == nullptr);
				REQUIRE(pack.GetDataID() == udAny);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);

				REQUIRE(another_pack.GetMeta() == meta->GetSparseMeta());
				REQUIRE(another_pack.GetDataID().Is<int*>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.As<int>() == original_value);
				REQUIRE_THROWS(another_pack.As<float>() == float(original_value));
				REQUIRE(*another_pack.As<int*>() == original_value);
				REQUIRE(another_pack.As<int*>() == original_int);
				REQUIRE_THROWS(another_pack.As<float*>() == nullptr);
				REQUIRE(PCMEMORY.CheckJurisdiction(metas, original_int));
				REQUIRE(PCMEMORY.GetReferences(metas, original_int) == 2);
				REQUIRE(another_pack.GetBlockReferences() == 1);
			}
		}

		WHEN("Given a sparse value and then copied from Block") {
			int* original_int = new int(original_value);
			pack = original_int;
			Any another_pack = static_cast<Block&>(pack);

			THEN("Various traits change") {
				REQUIRE(another_pack.GetMeta() == meta->GetSparseMeta());
				REQUIRE(another_pack.GetDataID().Is<int*>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.As<int>() == original_value);
				REQUIRE_THROWS(another_pack.As<float>() == float(original_value));
				REQUIRE(*another_pack.As<int*>() == original_value);
				REQUIRE(another_pack.As<int*>() == original_int);
				REQUIRE_THROWS(another_pack.As<float*>() == nullptr);
				REQUIRE(PCMEMORY.CheckJurisdiction(metas, original_int));
				REQUIRE(PCMEMORY.GetReferences(metas, original_int) == 2);
				REQUIRE(pack.GetBlockReferences() == another_pack.GetBlockReferences());
				REQUIRE(pack.GetBlockReferences() == 2);
			}
		}

		WHEN("Given a sparse value and then moved from Block") {
			int* original_int = new int(original_value);
			pack = original_int;
			Any another_pack = pcMove(static_cast<Block&>(pack));

			THEN("Moving a block intro container doesn't reset source block to avoid memory leaks") {
				REQUIRE(pack.GetMeta() == meta->GetSparseMeta());
				REQUIRE(pack.GetDataID().Is<int*>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == original_value);
				REQUIRE_THROWS(pack.As<float>() == float(original_value));
				REQUIRE(*pack.As<int*>() == original_value);
				REQUIRE(pack.As<int*>() == original_int);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				REQUIRE(pack.GetBlockReferences() == 2);

				REQUIRE(another_pack.GetBlockReferences() == 2);
				REQUIRE(another_pack.GetMeta() == meta->GetSparseMeta());
				REQUIRE(another_pack.GetDataID().Is<int*>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.As<int>() == original_value);
				REQUIRE_THROWS(another_pack.As<float>() == float(original_value));
				REQUIRE(*another_pack.As<int*>() == original_value);
				REQUIRE(another_pack.As<int*>() == original_int);
				REQUIRE_THROWS(another_pack.As<float*>() == nullptr);
				REQUIRE(another_pack.GetBlockReferences() == 2);

				REQUIRE(PCMEMORY.CheckJurisdiction(metas, original_int));
				REQUIRE(PCMEMORY.GetReferences(metas, original_int) == 2);
			}
		}

		WHEN("Given a sparse value and then reset") {
			int* original_int = new int(original_value);
			pack = original_int;
			pack.Reset();
			THEN("Various traits change") {
				REQUIRE(pack.GetMeta() == nullptr);
				REQUIRE(pack.GetDataID() == udAny);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(PCMEMORY.CheckJurisdiction(meta, original_int));
				REQUIRE(PCMEMORY.GetReferences(meta, original_int) == 1);
			}
		}

		WHEN("Given static text") {
			pack = original_pct;
			THEN("Various traits change") {
				REQUIRE(pack.GetMeta() == DataID::Reflect<Text>());
				REQUIRE(pack.GetDataID().Is<Text>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE_THROWS(pack.As<int>() == 0);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(pack.As<Text>() == original_pct);
				REQUIRE_THROWS(pack.As<int*>() == nullptr);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				REQUIRE(*pack.As<Text*>() == original_pct);
				REQUIRE(pack.As<Text*>()->CheckJurisdiction() == true);
				REQUIRE(pack.As<Text*>()->GetBlockReferences() == 2);
			}
		}

		WHEN("Given dynamic text") {
			pack = original_pct.Clone();
			THEN("Various traits change") {
				REQUIRE(pack.GetMeta() == DataID::Reflect<Text>());
				REQUIRE(pack.GetDataID().Is<Text>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE_THROWS(pack.As<int>() == 0);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(pack.As<Text>() == original_pct);
				REQUIRE_THROWS(pack.As<int*>() == nullptr);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				REQUIRE(*pack.As<Text*>() == original_pct);
				REQUIRE(pack.As<Text*>()->CheckJurisdiction() == true);
				REQUIRE(pack.As<Text*>()->GetBlockReferences() == 1);
			}
		}

		WHEN("Given dynamic text, which is later referenced multiple times") {
			pack = original_pct.Clone();
			Any pack2(pack);
			Any pack3(pack2);
			Any pack4(pack3);

			THEN("Various traits change") {
				REQUIRE(pack4.GetMeta() == DataID::Reflect<Text>());
				REQUIRE(pack4.GetDataID().Is<Text>());
				REQUIRE(pack4.GetRaw() != nullptr);
				REQUIRE_THROWS(pack4.As<int>() == 0);
				REQUIRE_THROWS(pack4.As<float>() == 0.0f);
				REQUIRE(pack4.As<Text>() == original_pct);
				REQUIRE_THROWS(pack4.As<int*>() == nullptr);
				REQUIRE_THROWS(pack4.As<float*>() == nullptr);
				REQUIRE(*pack4.As<Text*>() == original_pct);
				REQUIRE(pack4.As<Text*>()->CheckJurisdiction() == true);
				REQUIRE(pack.GetBlockReferences() == 4);
				REQUIRE(pack2.GetBlockReferences() == 4);
				REQUIRE(pack3.GetBlockReferences() == 4);
				REQUIRE(pack4.GetBlockReferences() == 4);
				REQUIRE(pack4.As<Text*>()->GetBlockReferences() == 1);
			}
		}

		WHEN("Given dynamic text, which is later referenced multiple times, and then dereferenced") {
			pack = original_pct.Clone();
			Any pack2(pack);
			Any pack3(pack2);
			Any pack4(pack3);
			pack.Reset();
			pack3.Reset();

			THEN("Various traits change") {
				REQUIRE(pack4.GetMeta() == DataID::Reflect<Text>());
				REQUIRE(pack4.GetDataID().Is<Text>());
				REQUIRE(pack4.GetRaw() != nullptr);
				REQUIRE_THROWS(pack4.As<int>() == 0);
				REQUIRE_THROWS(pack4.As<float>() == 0.0f);
				REQUIRE(pack4.As<Text>() == original_pct);
				REQUIRE_THROWS(pack4.As<int*>() == nullptr);
				REQUIRE_THROWS(pack4.As<float*>() == nullptr);
				REQUIRE(*pack4.As<Text*>() == original_pct);
				REQUIRE(pack4.As<Text*>()->CheckJurisdiction() == true);
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(pack2.GetBlockReferences() == 2);
				REQUIRE(pack3.GetBlockReferences() == 1);
				REQUIRE(pack4.GetBlockReferences() == 2);
				REQUIRE(pack4.As<Text*>()->GetBlockReferences() == 1);

				REQUIRE(pack.GetMeta() == nullptr);
				REQUIRE(pack.GetDataID() == udAny);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack3.GetMeta() == nullptr);
				REQUIRE(pack3.GetDataID() == udAny);
				REQUIRE(pack3.GetRaw() == nullptr);
			}
		}
	}

	GIVEN("A templated Any with some POD items") {
		TAny<int> pack;
		pack << int(1) << int(2) << int(3) << int(4) << int(5);
		auto memory = pack.GetBytes();

		REQUIRE(pack.GetCount() == 5);
		REQUIRE(pack.GetReserved() >= 5);
		REQUIRE(pack.Is<int>());
		REQUIRE(pack.GetBytes() != nullptr);
		REQUIRE(pack[0] == 1);
		REQUIRE(pack[1] == 2);
		REQUIRE(pack[2] == 3);
		REQUIRE(pack[3] == 4);
		REQUIRE(pack[4] == 5);

		WHEN("Push more of the same stuff") {
			pack << int(6) << int(7) << int(8) << int(9) << int(10);
			THEN("The size and capacity change, type will never change, memory shouldn't move") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<int>());
			}
		}

		WHEN("Insert more items at a specific place") {
			int i666 = 666;
			pack.Insert(&i666, 1, 3);
			THEN("The size and capacity change, type will never change, memory shouldn't move") {
				REQUIRE(pack.GetCount() == 6);
				REQUIRE(pack.GetReserved() >= 6);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack[0] == 1);
				REQUIRE(pack[1] == 2);
				REQUIRE(pack[2] == 3);
				REQUIRE(pack[3] == 666);
				REQUIRE(pack[4] == 4);
				REQUIRE(pack[5] == 5);
			}
		}

		WHEN("The size is reduced by removing elements") {
			pack.Remove(int(2));
			pack.Remove(int(4));
			THEN("The size changes but not capacity") {
				REQUIRE(pack[0] == 1);
				REQUIRE(pack[1] == 3);
				REQUIRE(pack[2] == 5);
				REQUIRE(pack.GetCount() == 3);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
			}
		}

		WHEN("More capacity is reserved") {
			pack.Allocate(20);
			THEN("The capacity changes but not the size, memory shouldn't move") {
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 20);
				REQUIRE(pack.GetBytes() == memory);
			}
		}

		WHEN("Less capacity is reserved") {
			pack.Allocate(2);
			THEN("Capacity remains unchanged, but count is trimmed") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
			}
		}

		WHEN("Pack is cleared") {
			pack.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<int>());
			}
		}

		WHEN("Pack is reset") {
			pack.Reset();
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetBytes() == nullptr);
				REQUIRE(pack.Is<int>());
			}
		}

		WHEN("Pack is reset, then allocated again") {
			pack.Reset();
			pack << int(6) << int(7) << int(8) << int(9) << int(10);
			THEN("Block manager should reuse the memory") {
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<int>());
			}
		}

		WHEN("Pack is shallow-copied") {
			pack.ToggleState(DState::Or, true);
			auto copy = pack;
			THEN("The new pack should keep the state and data") {
				REQUIRE(copy.GetBytes() == pack.GetBytes());
				REQUIRE(copy.GetCount() == pack.GetCount());
				REQUIRE(copy.GetReserved() == pack.GetReserved());
				REQUIRE(copy.GetState() == pack.GetState());
				REQUIRE(copy.GetDataID() == pack.GetDataID());
				REQUIRE(copy.GetBlockReferences() == 2);
			}
		}

		WHEN("Pack is cloned") {
			pack.ToggleState(DState::Or, true);
			auto clone = pack.Clone();
			THEN("The new pack should keep the state and data") {
				REQUIRE(clone.GetBytes() != pack.GetBytes());
				REQUIRE(clone.GetCount() == pack.GetCount());
				REQUIRE(clone.GetReserved() >= clone.GetCount());
				REQUIRE(clone.GetState() == pack.GetState());
				REQUIRE(clone.GetDataID() == pack.GetDataID());
				REQUIRE(clone.GetBlockReferences() == 1);
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}

		WHEN("Pack is moved") {
			pack.ToggleState(DState::Or, true);
			TAny<int> moved = pcMove(pack);
			THEN("The new pack should keep the state and data") {
				REQUIRE(pack.GetBytes() == nullptr);
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetState() == DState::Typed);
				REQUIRE(pack.GetDataID() == moved.GetDataID());
			}
		}

		WHEN("Packs are compared") {
			TAny<int> another_pack1;
			another_pack1 << int(1) << int(2) << int(3) << int(4) << int(5);
			TAny<int> another_pack2;
			another_pack2 << int(2) << int(2) << int(3) << int(4) << int(5);
			TAny<int> another_pack3;
			another_pack3 << int(1) << int(2) << int(3) << int(4) << int(5) << int(6);
			TAny<uint> another_pack4;
			another_pack4 << uint(1) << uint(2) << uint(3) << uint(4) << uint(5);
			Any another_pack5;
			another_pack5 << int(1) << int(2) << int(3) << int(4) << int(5);
			THEN("The comparisons should be adequate") {
				REQUIRE(pack == another_pack1);
				REQUIRE(pack != another_pack2);
				REQUIRE(pack != another_pack3);
				REQUIRE(pack != another_pack4);
				REQUIRE(pack == another_pack5);
			}
		}
	}

	GIVEN("A universal Any with some POD items") {
		Any pack;
		pack << int(1) << int(2) << int(3) << int(4) << int(5);
		auto memory = pack.GetBytes();

		REQUIRE(pack.GetCount() == 5);
		REQUIRE(pack.GetReserved() >= 5);
		REQUIRE(pack.Is<int>());
		REQUIRE(pack.GetBytes() != nullptr);

		WHEN("Push more stuff") {
			pack << int(6) << int(7) << int(8) << int(9) << int(10);
			THEN("The size and capacity change, type will never change, memory shouldn't move") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}
		WHEN("The size is reduced") {
			pack.RemoveIndex(pack.Find(int(2)));
			pack.RemoveIndex(pack.Find(int(4)));
			THEN("The size changes but not capacity") {
				REQUIRE(pack.As<int>(0) == 1);
				REQUIRE(pack.As<int>(1) == 3);
				REQUIRE(pack.As<int>(2) == 5);
				REQUIRE(pack.GetCount() == 3);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}
		WHEN("The size is reduced to zero") {
			pack.RemoveIndex(pack.Find(int(2)));
			pack.RemoveIndex(pack.Find(int(4)));
			pack.RemoveIndex(pack.Find(int(1)));
			pack.RemoveIndex(pack.Find(int(3)));
			pack.RemoveIndex(pack.Find(int(5)));
			THEN("The container should be fully reset") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetBytes() == nullptr);
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(pack.GetState() == DState::Default);
			}
		}
		WHEN("More capacity is reserved") {
			pack.Allocate(20);
			THEN("The capacity changes but not the size, memory shouldn't move") {
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 20);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}
		WHEN("Less capacity is reserved") {
			pack.Allocate(2);
			THEN("Neither size nor capacity are changed") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}
		WHEN("Pack is cleared") {
			pack.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}
		WHEN("Pack is reset") {
			pack.Reset();
			THEN("Size and capacity goes to zero, type is reset to udAny") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetBytes() == nullptr);
				REQUIRE(pack.GetDataID() == udAny);
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}
		WHEN("Pack is reset, then allocated again") {
			pack.Reset();
			pack << int(6) << int(7) << int(8) << int(9) << int(10);
			THEN("Block manager should reuse the memory") {
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<int>());
			}
		}
		WHEN("Pack is shallow-copied") {
			pack.ToggleState(DState::Or, true);
			auto copy = pack;
			THEN("The new pack should keep the state and data") {
				REQUIRE(copy.GetBytes() == pack.GetBytes());
				REQUIRE(copy.GetCount() == pack.GetCount());
				REQUIRE(copy.GetReserved() == pack.GetReserved());
				REQUIRE(copy.GetState() == pack.GetState());
				REQUIRE(copy.GetDataID() == pack.GetDataID());
				REQUIRE(copy.GetBlockReferences() == 2);
			}
		}
		WHEN("Pack is cloned") {
			pack.ToggleState(DState::Or, true);
			auto clone = pack.Clone();
			THEN("The new pack should keep the state and data") {
				REQUIRE(clone.GetBytes() != pack.GetBytes());
				REQUIRE(clone.GetCount() == pack.GetCount());
				REQUIRE(clone.GetReserved() >= clone.GetCount());
				REQUIRE(clone.GetState() == pack.GetState());
				REQUIRE(clone.GetDataID() == pack.GetDataID());
				REQUIRE(clone.GetBlockReferences() == 1);
				REQUIRE(pack.GetBlockReferences() == 1);
			}
		}
		WHEN("Pack is moved") {
			pack.ToggleState(DState::Or, true);
			Any moved = pcMove(pack);
			THEN("The new pack should keep the state and data") {
				REQUIRE(pack.GetBytes() == nullptr);
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetState() == DState::Default);
				REQUIRE(pack.GetDataID() == udAny);
			}
		}

		WHEN("Packs can be compared") {
			TAny<int> another_pack1;
			another_pack1 << int(1) << int(2) << int(3) << int(4) << int(5);
			TAny<int> another_pack2;
			another_pack2 << int(2) << int(2) << int(3) << int(4) << int(5);
			TAny<int> another_pack3;
			another_pack3 << int(1) << int(2) << int(3) << int(4) << int(5) << int(6);
			TAny<uint> another_pack4;
			another_pack4 << uint(1) << uint(2) << uint(3) << uint(4) << uint(5);
			Any another_pack5;
			another_pack5 << int(1) << int(2) << int(3) << int(4) << int(5);
			THEN("The comparisons should be adequate") {
				REQUIRE(pack == another_pack1);
				REQUIRE(pack != another_pack2);
				REQUIRE(pack != another_pack3);
				REQUIRE(pack != another_pack4);
				REQUIRE(pack == another_pack5);
			}
		}
	}

	GIVEN("A universal Any with some deep items") {
		Any pack;
		Any subpack1;
		Any subpack2;
		Any subpack3;
		subpack1 << int(1) << int(2) << int(3) << int(4) << int(5);
		subpack2 << int(6) << int(7) << int(8) << int(9) << int(10);
		subpack3 << subpack1 << subpack2;
		pack << subpack1 << subpack2 << subpack3;
		pack.MakeTypeConstrained();

		auto memory = pack.GetBytes();
		//auto& submemory4 = subpack3.As<Any>(0);
		//auto& submemory5 = subpack3.As<Any>(1);

		REQUIRE(pack.GetCount() == 3);
		REQUIRE(pack.GetReserved() >= 3);
		REQUIRE(pack.Is<Any>());
		REQUIRE(pack.GetBytes() != nullptr);

		WHEN("Push more stuff") {
			REQUIRE_THROWS(pack << int(6) << int(7) << int(8) << int(9) << int(10));
			THEN("Pack is already full with more packs, so nothing should happen") {
				REQUIRE(pack.GetCount() == 3);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetBytes() != nullptr);
			}
		}

		WHEN("Element 0 is removed") {
			const auto refsBefore = pack.GetBlockReferences();
			pack.RemoveIndex(0);
			THEN("The size changes but not capacity") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.As<Any>(0) == subpack2);
				REQUIRE(pack.As<Any>(1) == subpack3);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.GetBlockReferences() == refsBefore);
				REQUIRE(subpack1.GetBlockReferences() == 2);
				REQUIRE(subpack2.GetBlockReferences() == 3);
				REQUIRE(subpack3.GetBlockReferences() == 2);
			}
		}

		WHEN("Element 1 is removed") {
			const auto refsBefore = pack.GetBlockReferences();
			pack.RemoveIndex(1);
			THEN("The size changes but not capacity") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.As<Any>(0) == subpack1);
				REQUIRE(pack.As<Any>(1) == subpack3);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.GetBlockReferences() == refsBefore);
				REQUIRE(subpack1.GetBlockReferences() == 3);
				REQUIRE(subpack2.GetBlockReferences() == 2);
				REQUIRE(subpack3.GetBlockReferences() == 2);
			}
		}

		WHEN("Element 2 is removed") {
			const auto refsBefore = pack.GetBlockReferences();
			pack.RemoveIndex(2);
			THEN("The size changes but not capacity") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.As<Any>(0) == subpack1);
				REQUIRE(pack.As<Any>(1) == subpack2);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.GetBlockReferences() == refsBefore);
				REQUIRE(subpack1.GetBlockReferences() == 3);
				REQUIRE(subpack2.GetBlockReferences() == 3);
				REQUIRE(subpack3.GetBlockReferences() == 1);
			}
		}

		WHEN("All element are removed one by one") {
			pack.RemoveIndex(0);
			pack.RemoveIndex(0);
			pack.RemoveIndex(0);
			THEN("The entire container is reset") {
				REQUIRE(pack.IsEmpty());
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.IsTypeConstrained());
				REQUIRE(pack.GetBytes() == nullptr);
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(subpack1.GetBlockReferences() == 2);
				REQUIRE(subpack2.GetBlockReferences() == 2);
				REQUIRE(subpack3.GetBlockReferences() == 1);
			}
		}

		WHEN("The size is reduced, by finding and removing") {
			pack.RemoveIndex(pack.Find(subpack1));
			THEN("The size changes but not capacity") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.As<Any>(0) == subpack2);
				REQUIRE(pack.As<Any>(1) == subpack3);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetBytes() != nullptr);
			}
		}

		WHEN("Pack is cleared") {
			pack.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.GetBytes() == memory);
				REQUIRE(pack.Is<Any>());
			}
		}

		WHEN("Pack is reset") {
			pack.Reset();
			THEN("Size and capacity goes to zero, type is reset to udAny") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetBytes() == nullptr);
				REQUIRE(pack.GetDataID().Is<Any>());
				REQUIRE(pack.GetState() == DState::Typed);
			}
		}

		WHEN("Pack is shallow-copied") {
			pack.As<Any>(2).As<Any>(1).ToggleState(DState::Or, true);
			pack.As<Any>(0).ToggleState(DState::Or, true);
			auto copy = pack;
			THEN("The new pack should keep the state and data") {
				REQUIRE(copy.GetBytes() == pack.GetBytes());
				REQUIRE(copy.GetCount() == pack.GetCount());
				REQUIRE(copy.GetReserved() == pack.GetReserved());
				REQUIRE(copy.GetState() == pack.GetState());
				REQUIRE(copy.GetDataID() == pack.GetDataID());
				REQUIRE(copy.GetBlockReferences() == 2);
				REQUIRE(copy.As<Any>(0).GetBytes() == subpack1.GetBytes());
				REQUIRE(copy.As<Any>(0).GetState() == DState::Or);
				REQUIRE(copy.As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(copy.As<Any>(0).GetBlockReferences() == 3);
				REQUIRE(copy.As<Any>(1).GetBytes() == subpack2.GetBytes());
				REQUIRE(copy.As<Any>(1).GetState() == DState::Default);
				REQUIRE(copy.As<Any>(1).GetCount() == subpack2.GetCount());
				REQUIRE(copy.As<Any>(1).GetBlockReferences() == 3);
				REQUIRE(copy.As<Any>(2).GetBytes() == subpack3.GetBytes());
				REQUIRE(copy.As<Any>(2).GetState() == DState::Default);
				REQUIRE(copy.As<Any>(2).GetCount() == subpack3.GetCount());
				REQUIRE(copy.As<Any>(2).GetBlockReferences() == 2);
				REQUIRE(copy.As<Any>(2).As<Any>(0).GetBytes() == subpack1.GetBytes());
				REQUIRE(copy.As<Any>(2).As<Any>(0).GetState() == DState::Default);
				REQUIRE(copy.As<Any>(2).As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(copy.As<Any>(2).As<Any>(1).GetBytes() == subpack2.GetBytes());
				REQUIRE(copy.As<Any>(2).As<Any>(1).GetState() == DState::Or);
				REQUIRE(copy.As<Any>(2).As<Any>(1).GetCount() == subpack2.GetCount());
			}
		}

		WHEN("Pack is cloned") {
			pack.As<Any>(2).As<Any>(1).ToggleState(DState::Or, true);
			pack.As<Any>(0).ToggleState(DState::Or, true);
			auto clone = pack.Clone();
			THEN("The new pack should keep the state and data") {
				REQUIRE(clone.GetBytes() != pack.GetBytes());
				REQUIRE(clone.GetCount() == pack.GetCount());
				REQUIRE(clone.GetReserved() >= clone.GetCount());
				REQUIRE(clone.GetState() == pack.GetUnconstrainedState());
				REQUIRE(clone.GetDataID() == pack.GetDataID());
				REQUIRE(clone.GetBlockReferences() == 1);
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(clone.As<Any>(0).GetBytes() != subpack1.GetBytes());
				REQUIRE(clone.As<Any>(0).GetState() == DState::Or);
				REQUIRE(clone.As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(clone.As<Any>(0).GetBlockReferences() == 1);
				REQUIRE(pack.As<Any>(0).GetBlockReferences() == 3);
				REQUIRE(clone.As<Any>(1).GetBytes() != subpack2.GetBytes());
				REQUIRE(clone.As<Any>(1).GetState() == DState::Default);
				REQUIRE(clone.As<Any>(1).GetCount() == subpack2.GetCount());
				REQUIRE(clone.As<Any>(1).GetBlockReferences() == 1);
				REQUIRE(pack.As<Any>(1).GetBlockReferences() == 3);
				REQUIRE(clone.As<Any>(2).GetBytes() != subpack3.GetBytes());
				REQUIRE(clone.As<Any>(2).GetState() == DState::Default);
				REQUIRE(clone.As<Any>(2).GetCount() == subpack3.GetCount());
				REQUIRE(clone.As<Any>(2).GetBlockReferences() == 1);
				REQUIRE(pack.As<Any>(2).GetBlockReferences() == 2);
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetBytes() != subpack1.GetBytes());
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetState() == DState::Default);
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetBlockReferences() == 1);
				REQUIRE(pack.As<Any>(2).As<Any>(0).GetBlockReferences() == 3);
				REQUIRE(clone.As<Any>(2).As<Any>(1).GetBytes() != subpack2.GetBytes());
				REQUIRE(clone.As<Any>(2).As<Any>(1).GetState() == DState::Or);
				REQUIRE(clone.As<Any>(2).As<Any>(1).GetCount() == subpack2.GetCount());
				REQUIRE(clone.As<Any>(2).As<Any>(1).GetBlockReferences() == 1);
				REQUIRE(pack.As<Any>(2).As<Any>(1).GetBlockReferences() == 3);
			}
		}

		WHEN("Smart pushing different type without retainment") {
			auto result = subpack1.SmartPush(Any(char8('?')), {}, true, false);
			THEN("The pack must remain unchanged") {
				REQUIRE(result == 0);
				REQUIRE(subpack1.GetCount() == 5);
			}
		}

		WHEN("Smart pushing with retainment") {
			Any deepened;
			deepened << int(1) << int(2) << int(3) << int(4) << int(5);
			auto result = deepened.SmartPush(Any(char8('?')), {}, false, true);
			THEN("The pack must get deeper and contain it") {
				REQUIRE(result == 1);
				REQUIRE(deepened.IsDeep());
				REQUIRE(deepened.GetCount() == 2);
				REQUIRE(deepened.As<Any>(0).GetCount() == 5);
				REQUIRE(deepened.As<Any>(1).GetCount() == 1);
			}
		}

		WHEN("Smart pushing an empty container (but not stateless) with retainment") {
			Any deepened;
			deepened << int(1) << int(2) << int(3) << int(4) << int(5);
			auto pushed = Any::From(nullptr, DState::Polar | DState::Missing);
			auto result = deepened.SmartPush(pushed, {}, true, true);
			THEN("The pack must get deeper and contain it") {
				REQUIRE(result == 1);
				REQUIRE(deepened.IsDeep());
				REQUIRE(deepened.GetCount() == 2);
				REQUIRE(deepened.As<Any>(0).GetCount() == 5);
				REQUIRE(deepened.As<Any>(1).GetCount() == 0);
				REQUIRE(deepened.As<Any>(1).GetState() == DState::Polar + DState::Missing);
			}
		}

		WHEN("Smart pushing an empty container (but not stateless) with retainment to another empty container") {
			auto pushed = Any::From(nullptr, DState::Polar | DState::Missing);
			auto pushed2 = Any::From(nullptr, DState::Default);
			auto result = pushed2.SmartPush(pushed, {}, true, true);
			THEN("The pack must get deeper and contain it") {
				REQUIRE(result == 1);
				REQUIRE(pushed2.GetCount() == 0);
				REQUIRE(pushed2.GetState() == DState::Polar + DState::Missing);
			}
		}

		WHEN("Smart pushing to an empty container (concat & retain enabled)") {
			Any pushed;
			auto result = pushed.SmartPush(pack, {}, true, true);
			THEN("The empty container becomes the pushed container") {
				REQUIRE(pushed == pack);
				REQUIRE(result == 1);
			}
		}

		WHEN("Smart pushing to a different container with retain enabled") {
			Any pushed;
			pushed << 666;
			pushed.ToggleState(DState::Or, true);
			auto result = pushed.SmartPush(Any(char8('?')), {}, true, true);
			THEN("Must not duplicate state of deepened container") {
				REQUIRE(result == 1);
				REQUIRE(!pushed.IsOr());
				REQUIRE(pushed.As<Any>(0).IsOr());
				REQUIRE(!pushed.As<Any>(1).IsOr());
			}
		}
	}

	GIVEN("Two templated Any with some POD items") {
		TAny<int> pack1;
		TAny<int> pack2;
		pack1 << int(1) << int(2) << int(3) << int(4) << int(5);
		pack2 << int(6) << int(7) << int(8) << int(9) << int(10);
		const auto memory1 = static_cast<Block>(pack1);
		const auto memory2 = static_cast<Block>(pack2);

		REQUIRE(memory1 != memory2);

		WHEN("Shallow copy pack1 in pack2") {
			pack2 = pack1;
			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetBlockReferences() == 2);
				REQUIRE(pack2.GetBlockReferences() == 2);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(memory2.GetBlockReferences() == 0);
			}
		}

		WHEN("Shallow copy pack1 in pack2 and then reset pack1") {
			pack2 = pack1;
			pack1.Reset();
			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE(pack1.CheckJurisdiction() == false);
				REQUIRE(pack2.GetBlockReferences() == 1);
				REQUIRE(pack1.GetRaw() == nullptr);
				REQUIRE(pack1.GetReserved() == 0);
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(memory2.GetBlockReferences() == 0);
			}
		}

		WHEN("Deep copy pack1 in pack2") {
			pack2 = pack1.Clone();
			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetBlockReferences() == 1);
				REQUIRE(pack2.GetBlockReferences() == 1);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(static_cast<Block&>(pack2) != memory2);
				REQUIRE(memory2.GetBlockReferences() == 0);
			}
		}

		WHEN("Deep copy pack1 in pack2, then reset pack1") {
			pack2 = pack1.Clone();
			const auto memory3 = static_cast<Block>(pack2);
			pack1.Reset();
			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE(pack1.CheckJurisdiction() == false);
				REQUIRE(pack2.GetBlockReferences() == 1);
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(static_cast<Block&>(pack2) != memory2);
				REQUIRE(memory1.CheckJurisdiction() == false);
				REQUIRE(memory2.CheckJurisdiction() == false);
				REQUIRE(memory3.GetBlockReferences() == 1);
			}
		}
	}

	GIVEN("A universal Any with some deep items") {
		Any pack;
		Any subpack1;
		Any subpack2;
		Any subpack3;
		subpack1 << int(1) << int(2) << int(3) << int(4) << int(5);
		subpack2 << int(6) << int(7) << int(8) << int(9) << int(10);
		subpack3 << subpack1;
		subpack3.MakeOr();
		pack << subpack1 << subpack2 << subpack3;

		WHEN("The container is optimized") {
			pack.Optimize();

			THEN("Some subpacks should be optimized-out") {
				REQUIRE(pack.GetCount() == 3);
				REQUIRE(pack.As<Any>(0) == subpack1);
				REQUIRE(pack.As<Any>(1) == subpack2);
				REQUIRE(pack.As<Any>(2) == subpack1);
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(subpack1.GetBlockReferences() == 3);
				REQUIRE(subpack2.GetBlockReferences() == 2);
				REQUIRE(subpack3.GetBlockReferences() == 1);
			}
		}
	}

	GIVEN("A universal Any with some deep items, and their Blocks coalesced") {
		Any pack;
		Any subpack1;
		Any subpack2;
		Any subpack3;
		subpack1 << int(1) << int(2) << int(3) << int(4) << int(5);
		subpack2 << int(6) << int(7) << int(8) << int(9) << int(10);
		subpack3 << subpack1;
		subpack3.MakeOr();
		pack << subpack1 << subpack2 << subpack3;

		auto baseRange = Any::From<Block>();
		baseRange.Allocate(3);

		for (pcptr e = 0; e < pack.GetCount(); ++e) {
			auto element = pack.GetElement(e);
			LinkedBase base;
			REQUIRE(element.GetMeta()->GetBase<Block>(0, base));
			auto baseBlock = element.GetBaseMemory(MetaData::Of<Block>(), base);
			baseRange.InsertBlock(baseBlock);
		}

		WHEN("The Block bases from the subpacks are coalesced in a single container") {
			THEN("Contents should be referenced despite Block having no referencing logic in its reflected copy-operator") {
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(subpack1.GetBlockReferences() == 4);
				REQUIRE(subpack2.GetBlockReferences() == 3);
				REQUIRE(subpack3.GetBlockReferences() == 3);
			}
		}

		WHEN("The coalesced Block bases are freed") {
			baseRange.Reset();

			THEN("Contents should be dereferenced despite Block having no referencing logic in its reflected destructor") {
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(subpack1.GetBlockReferences() == 3);
				REQUIRE(subpack2.GetBlockReferences() == 2);
				REQUIRE(subpack3.GetBlockReferences() == 2);
			}
		}

		WHEN("The master pack is freed") {
			pack.Reset();

			THEN("Contents should be dereferenced") {
				REQUIRE(pack.GetBlockReferences() == 1);
				REQUIRE(subpack1.GetBlockReferences() == 3);
				REQUIRE(subpack2.GetBlockReferences() == 2);
				REQUIRE(subpack3.GetBlockReferences() == 2);
			}
		}
	}
}
