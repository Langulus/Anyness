#include "TestMain.hpp"
#include <catch2/catch.hpp>

using uint = unsigned int;

SCENARIO("TAny", "[containers]") {
	GIVEN("A TAny instance") {
		int value = 555;
		auto meta = MetaData::Of<int>();
		TAny<int> pack;

		REQUIRE(meta);
		REQUIRE(pack.GetType() == meta);
		REQUIRE(pack.IsTypeConstrained());
		REQUIRE(pack.GetRaw() == nullptr);
		REQUIRE(pack.IsEmpty());
		REQUIRE_FALSE(pack.IsAllocated());

		WHEN("Given a POD value by copy") {
			pack = value;
			THEN("Various traits change") {
				REQUIRE(pack.GetType() == meta);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == value);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(*pack.As<int*>() == value);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK("Anyness::TAny::operator = (single trivial copy)") {
					TAny<int> myPack;
					myPack = value;
					return myPack.GetCount();		// prevent stuff being optimized-out
				};
				BENCHMARK("std::vector::push_back(single trivial copy)") {
					std::vector<int> stdPack;
					stdPack.push_back(value);
					return stdPack.size();	// prevent stuff being optimized-out
				};
			#endif
		}
		
		WHEN("Given a POD value by move") {
			pack = Move(value);
			THEN("Various traits change") {
				REQUIRE(pack.GetType() == meta);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == value);
				REQUIRE_THROWS(pack.As<float>() == float(value));
				REQUIRE(*pack.As<int*>() == value);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK("Anyness::TAny::operator = (single trivial move)") {
					TAny<int> myPack;
					myPack = Move(value);
					return myPack.GetCount();		// prevent stuff being optimized-out
				};
				BENCHMARK("std::vector::emplace_back(single trivial move)") {
					std::vector<int> stdPack;
					stdPack.emplace_back(Move(value));
					return stdPack.size();	// prevent stuff being optimized-out
				};
			#endif
		}
	}
	
	GIVEN("A templated Any with some POD items") {
		TAny<int> pack;
		pack << int(1) << int(2) << int(3) << int(4) << int(5);
		auto memory = pack.GetRaw();

		REQUIRE(pack.GetCount() == 5);
		REQUIRE(pack.GetReserved() >= 5);
		REQUIRE(pack.Is<int>());
		REQUIRE(pack.GetRaw());
		REQUIRE(pack[0] == 1);
		REQUIRE(pack[1] == 2);
		REQUIRE(pack[2] == 3);
		REQUIRE(pack[3] == 4);
		REQUIRE(pack[4] == 5);
		REQUIRE_FALSE(pack.IsConstant());

		WHEN("Push more of the same stuff") {
			pack << int(6) << int(7) << int(8) << int(9) << int(10);
			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack.Is<int>());
			}
			
			//#ifdef LANGULUS_STD_BENCHMARK
				TAny<int> myPack;
				std::vector<int> stdPack;
				BENCHMARK("Anyness::TAny::operator << (5 trivial moves)") {
					myPack << int(1) << int(2) << int(3) << int(4) << int(5);
					return myPack.GetCount();		// prevent stuff being optimized-out
				};
				BENCHMARK("std::vector::emplace_back(5 trivial moves)") {
					stdPack.emplace_back(1);
					stdPack.emplace_back(2);
					stdPack.emplace_back(3);
					stdPack.emplace_back(4);
					stdPack.emplace_back(5);
					return stdPack.size();	// prevent stuff being optimized-out
				};
			//#endif
		}

		WHEN("Insert more items at a specific place") {
			int i666 = 666;
			pack.Insert(&i666, 1, 3);
			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 6);
				REQUIRE(pack.GetReserved() >= 6);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack.Is<int>());
				REQUIRE(pack[0] == 1);
				REQUIRE(pack[1] == 2);
				REQUIRE(pack[2] == 3);
				REQUIRE(pack[3] == 666);
				REQUIRE(pack[4] == 4);
				REQUIRE(pack[5] == 5);
			}
		}

		WHEN("The size is reduced by removing elements, but allocated memory should remain the same") {
			const auto removed2 = pack.Remove(int(2));
			const auto removed4 = pack.Remove(int(4));
			THEN("The size changes but not capacity") {
				REQUIRE(removed2 == 1);
				REQUIRE(removed4 == 1);
				REQUIRE(pack[0] == 1);
				REQUIRE(pack[1] == 3);
				REQUIRE(pack[2] == 5);
				SAFETY(REQUIRE_THROWS(pack[3] == 666));
				REQUIRE(pack.GetCount() == 3);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
			}
		}

		WHEN("Removing non-available elements") {
			const auto removed9 = pack.Remove(int(9));
			THEN("The size changes but not capacity") {
				REQUIRE(removed9 == 0);
				REQUIRE(pack[0] == 1);
				REQUIRE(pack[1] == 2);
				REQUIRE(pack[2] == 3);
				REQUIRE(pack[3] == 4);
				REQUIRE(pack[4] == 5);
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
			}
		}

		WHEN("More capacity is reserved") {
			pack.Allocate(20);
			THEN("The capacity changes but not the size, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 20);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
			}
		}

		WHEN("Less capacity is reserved") {
			pack.Allocate(2);
			THEN("Capacity remains unchanged, but count is trimmed; memory shouldn't move") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
			}
		}

		WHEN("Pack is cleared") {
			pack.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.Is<int>());
			}
		}

		WHEN("Pack is reset") {
			pack.Reset();
			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.Is<int>());
			}
		}

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			WHEN("Pack is reset, then immediately allocated again") {
				pack.Reset();
				pack << int(6) << int(7) << int(8) << int(9) << int(10);
				THEN("Block manager should reuse the memory, if MANAGED_MEMORY feature is enabled") {
					REQUIRE(pack.GetRaw() == memory);
				}
			}
		#endif

		WHEN("Pack is shallow-copied") {
			pack.MakeOr();
			auto copy = pack;
			THEN("The new pack should keep the state and data") {
				REQUIRE(copy.GetRaw() == pack.GetRaw());
				REQUIRE(copy.GetCount() == pack.GetCount());
				REQUIRE(copy.GetReserved() == pack.GetReserved());
				REQUIRE(copy.GetState() == pack.GetState());
				REQUIRE(copy.GetType() == pack.GetType());
				REQUIRE(copy.GetReferences() == 2);
			}
		}

		WHEN("Pack is cloned") {
			pack.MakeOr();
			auto clone = pack.Clone();
			THEN("The new pack should keep the state and data") {
				REQUIRE(clone.GetRaw() != pack.GetRaw());
				REQUIRE(clone.GetCount() == pack.GetCount());
				REQUIRE(clone.GetReserved() >= clone.GetCount());
				REQUIRE(clone.GetState() == pack.GetState());
				REQUIRE(clone.GetType() == pack.GetType());
				REQUIRE(clone.GetReferences() == 1);
				REQUIRE(pack.GetReferences() == 1);
			}
		}

		WHEN("Pack is moved") {
			pack.MakeOr();
			TAny<int> moved = Move(pack);
			THEN("The new pack should keep the state and data") {
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.IsTypeConstrained());
				REQUIRE(pack.GetType() == moved.GetType());
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
				REQUIRE(pack1.GetReferences() == 2);
				REQUIRE(pack2.GetReferences() == 2);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
			}
		}

		WHEN("Shallow copy pack1 in pack2 and then reset pack1") {
			pack2 = pack1;
			pack1.Reset();
			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetReferences() == 1);
				REQUIRE_FALSE(pack1.GetRaw());
				REQUIRE(pack1.GetReserved() == 0);
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
			}
		}

		WHEN("Deep copy pack1 in pack2") {
			pack2 = pack1.Clone();
			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetReferences() == 1);
				REQUIRE(pack2.GetReferences() == 1);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(static_cast<Block&>(pack2) != memory2);
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
			}
		}

		WHEN("Deep copy pack1 in pack2, then reset pack1") {
			pack2 = pack1.Clone();
			const auto memory3 = static_cast<Block>(pack2);
			pack1.Reset();
			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE_FALSE(Allocator::Find(memory1.GetType(), memory1.GetRaw()));
				REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				REQUIRE(pack2.GetReferences() == 1);
				REQUIRE(memory3.GetReferences() == 1);
			}
		}
	}
}
