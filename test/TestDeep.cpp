///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>
#include <any>
#include <vector>

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md			
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
	const Text serialized {ex};
	return ::std::string {Token {serialized}};
}

using uint = unsigned int;

SCENARIO("Deep containers", "[any]") {
	GIVEN("Any with some deep items") {
		#include "CollectGarbage.inl"

		Any pack;
		Any subpack1;
		Any subpack2;
		Any subpack3;
		subpack1 << int(1) << int(2) << int(3) << int(4) << int(5);
		subpack2 << int(6) << int(7) << int(8) << int(9) << int(10);
		subpack3 << subpack1 << subpack2;
		pack << subpack1 << subpack2 << subpack3;
		pack.MakeTypeConstrained();

		auto memory = pack.GetRaw();
		//auto& submemory4 = subpack3.As<Any>(0);
		//auto& submemory5 = subpack3.As<Any>(1);

		REQUIRE(pack.GetCount() == 3);
		REQUIRE(pack.GetReserved() >= 3);
		REQUIRE(pack.Is<Any>());
		REQUIRE(pack.GetRaw());

		WHEN("Push more stuff") {
			REQUIRE_THROWS_AS(pack << int(6), Except::Mutate);
			THEN("Pack is already full with more packs, so nothing should happen") {
				REQUIRE(pack.GetCount() == 3);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetRaw());
			}
		}

		WHEN("Element 0 is removed") {
			const auto refsBefore = pack.GetUses();
			pack.RemoveIndex(0);
			THEN("The size changes but not capacity") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.As<Any>(0) == subpack2);
				REQUIRE(pack.As<Any>(1) == subpack3);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.GetUses() == refsBefore);
				REQUIRE(subpack1.GetUses() == 2);
				REQUIRE(subpack2.GetUses() == 3);
				REQUIRE(subpack3.GetUses() == 2);
			}
		}

		WHEN("Element 1 is removed") {
			const auto refsBefore = pack.GetUses();
			pack.RemoveIndex(1);
			THEN("The size changes but not capacity") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.As<Any>(0) == subpack1);
				REQUIRE(pack.As<Any>(1) == subpack3);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.GetUses() == refsBefore);
				REQUIRE(subpack1.GetUses() == 3);
				REQUIRE(subpack2.GetUses() == 2);
				REQUIRE(subpack3.GetUses() == 2);
			}
		}

		WHEN("Element 2 is removed") {
			const auto refsBefore = pack.GetUses();
			pack.RemoveIndex(2);
			THEN("The size changes but not capacity") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.As<Any>(0) == subpack1);
				REQUIRE(pack.As<Any>(1) == subpack2);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.GetUses() == refsBefore);
				REQUIRE(subpack1.GetUses() == 3);
				REQUIRE(subpack2.GetUses() == 3);
				REQUIRE(subpack3.GetUses() == 1);
			}
		}

		WHEN("All element are removed one by one") {
			pack.RemoveIndex(0);
			pack.RemoveIndex(0);
			pack.RemoveIndex(0);
			THEN("The entire container is cleared, but memory remains in use") {
				REQUIRE(pack.IsEmpty());
				REQUIRE(pack.GetReserved() > 0);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.IsTypeConstrained());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.GetUses() > 0);
				REQUIRE(subpack1.GetUses() == 2);
				REQUIRE(subpack2.GetUses() == 2);
				REQUIRE(subpack3.GetUses() == 1);
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
				REQUIRE(pack.GetRaw() != nullptr);
			}
		}

		WHEN("Pack is cleared") {
			pack.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() >= 3);
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.Is<Any>());
			}
		}

		WHEN("Pack is reset") {
			pack.Reset();
			THEN("Size and capacity goes to zero, type is reset to udAny") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.IsTypeConstrained());
			}
		}

		WHEN("Pack is shallow-copied") {
			pack.As<Any>(2).As<Any>(1).MakeOr();
			pack.As<Any>(0).MakeOr();
			auto copy = pack;
			THEN("The new pack should keep the state and data") {
				REQUIRE(copy.GetRaw() == pack.GetRaw());
				REQUIRE(copy.GetCount() == pack.GetCount());
				REQUIRE(copy.GetReserved() == pack.GetReserved());
				REQUIRE(copy.GetState() == pack.GetState());
				REQUIRE(copy.GetType() == pack.GetType());
				REQUIRE(copy.GetUses() == 2);
				REQUIRE(copy.As<Any>(0).GetRaw() == subpack1.GetRaw());
				REQUIRE(copy.As<Any>(0).IsOr());
				REQUIRE(copy.As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(copy.As<Any>(0).GetUses() == 3);
				REQUIRE(copy.As<Any>(1).GetRaw() == subpack2.GetRaw());
				REQUIRE(copy.As<Any>(1).GetState() == DataState::Default);
				REQUIRE(copy.As<Any>(1).GetCount() == subpack2.GetCount());
				REQUIRE(copy.As<Any>(1).GetUses() == 3);
				REQUIRE(copy.As<Any>(2).GetRaw() == subpack3.GetRaw());
				REQUIRE(copy.As<Any>(2).GetState() == DataState::Default);
				REQUIRE(copy.As<Any>(2).GetCount() == subpack3.GetCount());
				REQUIRE(copy.As<Any>(2).GetUses() == 2);
				REQUIRE(copy.As<Any>(2).As<Any>(0).GetRaw() == subpack1.GetRaw());
				REQUIRE(copy.As<Any>(2).As<Any>(0).GetState() == DataState::Default);
				REQUIRE(copy.As<Any>(2).As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(copy.As<Any>(2).As<Any>(1).GetRaw() == subpack2.GetRaw());
				REQUIRE(copy.As<Any>(2).As<Any>(1).IsOr());
				REQUIRE(copy.As<Any>(2).As<Any>(1).GetCount() == subpack2.GetCount());
			}
		}

		WHEN("Pack is cloned") {
			pack.As<Any>(2).As<Any>(1).MakeOr();
			pack.As<Any>(0).MakeOr();
			auto clone = pack.Clone();
			THEN("The new pack should keep the state and data") {
				REQUIRE(clone.GetRaw() != pack.GetRaw());
				REQUIRE(clone.GetCount() == pack.GetCount());
				REQUIRE(clone.GetReserved() >= clone.GetCount());
				REQUIRE(clone.GetState() == pack.GetUnconstrainedState());
				REQUIRE(clone.GetType() == pack.GetType());
				REQUIRE(clone.GetUses() == 1);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(clone.As<Any>(0).GetRaw() != subpack1.GetRaw());
				REQUIRE(clone.As<Any>(0).IsOr());
				REQUIRE(clone.As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(clone.As<Any>(0).GetUses() == 1);
				REQUIRE(pack.As<Any>(0).GetUses() == 3);
				REQUIRE(clone.As<Any>(1).GetRaw() != subpack2.GetRaw());
				REQUIRE(clone.As<Any>(1).GetState() == DataState::Default);
				REQUIRE(clone.As<Any>(1).GetCount() == subpack2.GetCount());
				REQUIRE(clone.As<Any>(1).GetUses() == 1);
				REQUIRE(pack.As<Any>(1).GetUses() == 3);
				REQUIRE(clone.As<Any>(2).GetRaw() != subpack3.GetRaw());
				REQUIRE(clone.As<Any>(2).GetState() == DataState::Default);
				REQUIRE(clone.As<Any>(2).GetCount() == subpack3.GetCount());
				REQUIRE(clone.As<Any>(2).GetUses() == 1);
				REQUIRE(pack.As<Any>(2).GetUses() == 2);
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetRaw() != subpack1.GetRaw());
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetState() == DataState::Default);
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetCount() == subpack1.GetCount());
				REQUIRE(clone.As<Any>(2).As<Any>(0).GetUses() == 1);
				REQUIRE(pack.As<Any>(2).As<Any>(0).GetUses() == 3);
				REQUIRE(clone.As<Any>(2).As<Any>(1).GetRaw() != subpack2.GetRaw());
				REQUIRE(clone.As<Any>(2).As<Any>(1).IsOr());
				REQUIRE(clone.As<Any>(2).As<Any>(1).GetCount() == subpack2.GetCount());
				REQUIRE(clone.As<Any>(2).As<Any>(1).GetUses() == 1);
				REQUIRE(pack.As<Any>(2).As<Any>(1).GetUses() == 3);
			}
		}

		WHEN("Smart pushing different type without retainment") {
			auto result = subpack1.SmartPush<IndexBack, true, false>('?');
			THEN("The pack must remain unchanged") {
				REQUIRE(result == 0);
				REQUIRE(subpack1.GetCount() == 5);
			}
		}

		WHEN("Smart pushing with retainment") {
			Any deepened;
			deepened << int(1) << int(2) << int(3) << int(4) << int(5);
			auto result = deepened.SmartPush<IndexBack, false, true>('?');
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
			auto pushed = Any::FromMeta(nullptr, DataState::Missing);
			auto result = deepened.SmartPush<IndexBack, true, true>(pushed);
			THEN("The pack must get deeper and contain it") {
				REQUIRE(result == 1);
				REQUIRE(deepened.IsDeep());
				REQUIRE(deepened.GetCount() == 2);
				REQUIRE(deepened.As<Any>(0).GetCount() == 5);
				REQUIRE(deepened.As<Any>(1).GetCount() == 0);
				REQUIRE(deepened.As<Any>(1).GetState() == DataState::Missing);
			}
		}

		WHEN("Smart pushing an empty container (but not stateless) with retainment to another empty container") {
			auto pushed = Any::FromMeta(nullptr, DataState::Missing);
			auto pushed2 = Any::FromMeta(nullptr, DataState {});
			auto result = pushed2.SmartPush<IndexBack, true, true>(pushed);
			THEN("The pack must get deeper and contain it") {
				REQUIRE(result == 1);
				REQUIRE(pushed2.GetCount() == 0);
				REQUIRE(pushed2.GetState() == DataState::Missing);
			}
		}

		WHEN("Smart pushing to an empty container (concat & retain enabled)") {
			Any pushed;
			auto result = pushed.SmartPush<IndexBack, true, true>(pack);
			THEN("The empty container becomes the pushed container") {
				REQUIRE(pushed == pack);
				REQUIRE(result == 1);
			}
		}

		WHEN("Smart pushing to a different container with retain enabled") {
			Any pushed;
			pushed << 666;
			pushed.MakeOr();
			auto result = pushed.SmartPush<IndexBack, true, true>('?');
			THEN("State should be moved to the top") {
				REQUIRE(result == 1);
				REQUIRE(pushed.IsOr());
				REQUIRE(!pushed.As<Any>(0).IsOr());
				REQUIRE(!pushed.As<Any>(1).IsOr());
			}
		}
	}

	GIVEN("Any with some deep items for the purpose of optimization") {
		#include "CollectGarbage.inl"

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
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(subpack1.GetUses() == 3);
				REQUIRE(subpack2.GetUses() == 2);
				REQUIRE(subpack3.GetUses() == 1);
			}
		}
	}

	GIVEN("Any with some deep items, and their Blocks coalesced") {
		#include "CollectGarbage.inl"

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

		for (Count e = 0; e < pack.GetCount(); ++e) {
			auto element = pack.GetElement(e);
			RTTI::Base base;
			REQUIRE(element.GetType()->GetBase<Block>(0, base));
			auto baseBlock = element.GetBaseMemory(MetaData::Of<Block>(), base);
			baseRange.InsertBlock(baseBlock);
		}

		WHEN("The Block bases from the subpacks are coalesced in a single container") {
			THEN("Contents should be referenced despite Block having no referencing logic in its reflected copy-operator") {
				// but why??? rethink this functionality, it doesn't make any sense. sounds like a corner case that got generally fixed for some reason
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(subpack1.GetUses() == 3); //4 if that functionality is added
				REQUIRE(subpack2.GetUses() == 2); //3 if that functionality is added
				REQUIRE(subpack3.GetUses() == 2); //3 if that functionality is added
			}
		}

		WHEN("The coalesced Block bases are freed") {
			baseRange.Reset();

			THEN("Contents should be dereferenced despite Block having no referencing logic in its reflected destructor") {
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(subpack1.GetUses() == 3);
				REQUIRE(subpack2.GetUses() == 2);
				REQUIRE(subpack3.GetUses() == 2);
			}
		}

		WHEN("The master pack is freed") {
			pack.Reset();

			THEN("Contents should be dereferenced") {
				REQUIRE(pack.GetUses() == 0);
				REQUIRE(subpack1.GetUses() == 2); // 3 if that functionality is added
				REQUIRE(subpack2.GetUses() == 1); // 2 if that functionality is added
				REQUIRE(subpack3.GetUses() == 1); // 2 if that functionality is added
			}
		}
	}
}
