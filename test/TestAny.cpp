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

using uint = unsigned int;

SCENARIO("Any", "[containers]") {

	GIVEN("An Any instance") {
		int value = 555;
		auto meta = MetaData::Of<int>();

		REQUIRE(meta);

		WHEN("Given a default-constructed Any") {
			Any pack;

			THEN("Various traits change") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetType() == nullptr);
				REQUIRE(pack.IsUntyped());
				REQUIRE_FALSE(pack.IsTypeConstrained());
				REQUIRE_FALSE(pack.IsConstant());
				REQUIRE_FALSE(pack.IsCompressed());
				REQUIRE_FALSE(pack.IsAbstract());
				REQUIRE_FALSE(pack.IsAllocated());
				REQUIRE_FALSE(pack.IsDeep());
				REQUIRE_FALSE(pack.IsEncrypted());
				REQUIRE_FALSE(pack.IsFuture());
				REQUIRE_FALSE(pack.IsPast());
				REQUIRE_FALSE(pack.IsPhased());
				REQUIRE_FALSE(pack.IsMissing());
				REQUIRE_FALSE(pack.IsSparse());
				REQUIRE_FALSE(pack.IsStatic());
				REQUIRE_FALSE(pack.IsValid());
				REQUIRE(pack.IsNow());
				REQUIRE(pack.IsInvalid());
				REQUIRE(pack.IsDense());
				REQUIRE(pack.GetState() == DataState::Default);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.IsEmpty());
			}

			#ifdef LANGULUS_STD_BENCHMARK
				// Anyness::Any default construction is about 30% faster than std::any's on MSVC
				BENCHMARK_ADVANCED("Anyness::Any::default construction") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Catch::Benchmark::storage_for<Any>> storage(meter.runs());
					meter.measure([&](int i) { return storage[i].construct(); });
				};

				BENCHMARK_ADVANCED("std::any::default construction") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Catch::Benchmark::storage_for<std::any>> storage(meter.runs());
					meter.measure([&](int i) { return storage[i].construct(); });
				};

				// Anyness::Any default construction is twice as slow than std::vector's, due to
				// it having more members. Maybe compress members a bit to save some cycles?
				// Otherwise, keep in mind that speed and flexibility are the main Anyness mission
				BENCHMARK_ADVANCED("std::vector::default construction") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Catch::Benchmark::storage_for<std::vector<int>>> storage(meter.runs());
					meter.measure([&](int i) { return storage[i].construct(); });
				};
			#endif
		}

		WHEN("Given a POD value by copy") {
			Allocator::CollectGarbage();
			Any pack;
			pack = value;

			THEN("Various traits change") {
				REQUIRE(pack.GetCount() == 1);
				REQUIRE(pack.GetType() == meta);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == value);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(*pack.As<int*>() == value);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 8:1 performance - needs optimization
				BENCHMARK_ADVANCED("Anyness::Any::operator = (single trivial copy)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single trivial copy)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};
			#endif
		}

		WHEN("Given a dense Trait") {
			Allocator::CollectGarbage();
			Any pack;
			pack = Traits::Count(5);

			THEN("Various traits change") {
				REQUIRE(pack.GetCount() == 1);
				REQUIRE(pack.Is<Traits::Count>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE_FALSE(pack.IsDeep());
				REQUIRE(pack.As<Traits::Count>() == Traits::Count(5));
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 1:1 performance
				BENCHMARK_ADVANCED("Anyness::Any::operator = (Traits::Count(5))") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Traits::Count(5);
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (Traits::Count(5))") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Traits::Count(5);
					});
				};
			#endif
		}

		WHEN("Given a POD value by move") {
			Allocator::CollectGarbage();
			Any pack;
			pack = Move(value);

			THEN("Various traits change") {
				REQUIRE(pack.GetType() == meta);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == value);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(*pack.As<int*>() == value);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 8:1 performance - needs optimization
				BENCHMARK_ADVANCED("Anyness::Any::operator = (single trivial move)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single trivial move)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};
			#endif
		}

		WHEN("Given a sparse value") {
			Allocator::CollectGarbage();
			int* original_int = new int(value);
			Any pack;
			pack = original_int;

			THEN("Various traits change") {
				REQUIRE(pack.GetType() == meta);
				REQUIRE(pack.IsSparse());
				REQUIRE(pack.Is<int*>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == value);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(*pack.As<int*>() == value);
				REQUIRE(pack.As<int*>() == original_int);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				#if LANGULUS_FEATURE(NEWDELETE) && LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(meta, original_int));
					REQUIRE(Allocator::GetReferences(meta, original_int) == 2);
				#else
					REQUIRE_FALSE(Allocator::CheckAuthority(meta, original_int));
					REQUIRE(Allocator::GetReferences(meta, original_int) == 0);
				#endif
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 9:1 performance - needs optimization
				BENCHMARK_ADVANCED("Anyness::Any::operator = (single pointer copy)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single pointer copy)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};
			#endif
		}

		WHEN("Given a sparse value by move") {
			Allocator::CollectGarbage();
			int* original_int = new int(value);
			int* original_int_backup = original_int;
			Any pack;
			pack = Move(original_int);

			THEN("Various traits change, pointer remains valid") {
				REQUIRE(original_int == original_int_backup);
				REQUIRE(pack.IsSparse());
				REQUIRE(pack.GetType() == meta);
				REQUIRE(pack.Is<int*>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.As<int>() == value);
				REQUIRE_THROWS(pack.As<float>() == float(value));
				REQUIRE(*pack.As<int*>() == value);
				REQUIRE(pack.As<int*>() == original_int_backup);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				#if LANGULUS_FEATURE(NEWDELETE) && LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(meta, original_int_backup));
					REQUIRE(Allocator::GetReferences(meta, original_int_backup) == 2);
				#else
					REQUIRE_FALSE(Allocator::CheckAuthority(meta, original_int_backup));
					REQUIRE(Allocator::GetReferences(meta, original_int_backup) == 0);
				#endif
				REQUIRE(pack.GetUses() == 1);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 9:1 performance - needs optimization
				BENCHMARK_ADVANCED("Anyness::Any::operator = (single pointer move)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single pointer move)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};
			#endif
		}

		WHEN("Shallow-copying Any") {
			Allocator::CollectGarbage();
			int* original_int = new int(value);
			Any pack;
			pack = original_int;
			Any another_pack = pack;

			THEN("Various traits change") {
				REQUIRE(another_pack == pack);
				REQUIRE(another_pack.GetType() == meta);
				REQUIRE(another_pack.IsSparse());
				REQUIRE(another_pack.Is<int*>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.As<int>() == value);
				REQUIRE_THROWS(another_pack.As<float>() == float(value));
				REQUIRE(*another_pack.As<int*>() == value);
				REQUIRE(another_pack.As<int*>() == original_int);
				REQUIRE_THROWS(another_pack.As<float*>() == nullptr);
				#if LANGULUS_FEATURE(NEWDELETE) && LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(meta, original_int));
					REQUIRE(Allocator::GetReferences(meta, original_int) == 2);
				#else
					REQUIRE_FALSE(Allocator::CheckAuthority(meta, original_int));
					REQUIRE(Allocator::GetReferences(meta, original_int) == 0);
				#endif
				REQUIRE(pack.GetUses() == another_pack.GetUses());
				REQUIRE(pack.GetUses() == 2);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 2:1 performance - needs optimization
				BENCHMARK_ADVANCED("Anyness::Any::operator = (shallow-copied Any)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> source(meter.runs());
					for (auto& i : source)
						i = original_int;

					std::vector<Any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = source[i];
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (shallow-copied std::any)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> source(meter.runs());
					for (auto& i : source)
						i = original_int;

					std::vector<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = source[i];
					});
				};
			#endif
		}

		WHEN("Moving Any") {
			Allocator::CollectGarbage();
			int* original_int = new int(value);
			Any pack;
			pack = original_int;
			Any another_pack = Move(pack);

			THEN("Entire container is moved to the new place without referencing anything") {
				REQUIRE(pack.GetType() == nullptr);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);

				REQUIRE(another_pack.GetType() == meta);
				REQUIRE(another_pack.Is<int*>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.As<int>() == value);
				REQUIRE_THROWS(another_pack.As<float>() == 0.0f);
				REQUIRE(*another_pack.As<int*>() == value);
				REQUIRE(another_pack.As<int*>() == original_int);
				REQUIRE_THROWS(another_pack.As<float*>() == nullptr);
				#if LANGULUS_FEATURE(NEWDELETE) && LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(meta, original_int));
					REQUIRE(Allocator::GetReferences(meta, original_int) == 2);
				#else
					REQUIRE_FALSE(Allocator::CheckAuthority(meta, original_int));
					REQUIRE(Allocator::GetReferences(meta, original_int) == 0);
				#endif
				REQUIRE(another_pack.GetUses() == 1);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 6:1 performance - needs optimization
				Allocator::CollectGarbage();
				BENCHMARK_ADVANCED("Anyness::Any::operator = (moved Any)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> source(meter.runs());
					for (auto& i : source)
						i = original_int;

					std::vector<Any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(source[i]);
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (moved std::any)") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> source(meter.runs());
					for (auto& i : source)
						i = original_int;

					std::vector<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(source[i]);
					});
				};
			#endif
		}

		WHEN("Constructing via a Block filled with dense items") {
			Allocator::CollectGarbage();
			Any pack = value;
			Any another_pack {static_cast<Block&>(pack)};

			THEN("Block will be referenced") {
				REQUIRE(another_pack.GetType() == meta);
				REQUIRE(another_pack.Is<int>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.IsDense());
				REQUIRE(another_pack.As<int>() == value);
				REQUIRE(another_pack.HasAuthority());
				REQUIRE(pack.GetUses() == another_pack.GetUses());
				REQUIRE(pack.GetUses() == 2);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 2:1 performance - needs optimization
				BENCHMARK_ADVANCED("Anyness::Any::construct via dense Block shallow-copy") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> source(meter.runs());
					for (auto& i : source)
						i = value;

					std::vector<Catch::Benchmark::storage_for<Any>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct(static_cast<Block&>(source[i]));
					});
				};

				BENCHMARK_ADVANCED("std::any::construct via std::any copy") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> source(meter.runs());
					for (auto& i : source)
						i = value;

					std::vector<Catch::Benchmark::storage_for<std::any>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct(source[i]);
					});
				};
			#endif
		}

		WHEN("Constructing via a Block filled with sparse items") {
			Allocator::CollectGarbage();
			Any pack = new int {value};
			Any another_pack {static_cast<Block&>(pack)};

			THEN("Block will be referenced") {
				REQUIRE(another_pack.GetType() == meta);
				REQUIRE(another_pack.Is<int>());
				REQUIRE(another_pack.GetRaw() != nullptr);
				REQUIRE(another_pack.IsSparse());
				REQUIRE(another_pack.As<int>() == value);
				REQUIRE(another_pack.HasAuthority());
				REQUIRE(pack.GetUses() == another_pack.GetUses());
				REQUIRE(pack.GetUses() == 2);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 2:1 performance - needs optimization
				BENCHMARK_ADVANCED("Anyness::Any::construct via sparse Block shallow-copy") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<Any> source(meter.runs());
					for (auto& i : source)
						i = value;

					std::vector<Catch::Benchmark::storage_for<Any>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct(static_cast<Block&>(source[i]));
					});
				};

				BENCHMARK_ADVANCED("std::any::construct via std::any copy") (Catch::Benchmark::Chronometer meter) {
					Allocator::CollectGarbage();
					std::vector<std::any> source(meter.runs());
					for (auto& i : source)
						i = value;

					std::vector<Catch::Benchmark::storage_for<std::any>> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i].construct(source[i]);
					});
				};
			#endif
		}

		WHEN("Given a sparse value and then reset") {
			Allocator::CollectGarbage();
			int* original_int = new int(value);
			Any pack;
			pack = original_int;
			pack.Reset();
			THEN("Various traits change") {
				REQUIRE(pack.GetType() == nullptr);
				REQUIRE(pack.GetRaw() == nullptr);
				#if LANGULUS_FEATURE(NEWDELETE) && LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(meta, original_int));
				#else
					REQUIRE_FALSE(Allocator::CheckAuthority(meta, original_int));
				#endif
				REQUIRE(Allocator::GetReferences(meta, original_int) == 0);
			}
		}

		WHEN("Given static text") {
			Allocator::CollectGarbage();
			Text original_pct = u8"Lorep Ipsum";
			Any pack;
			pack = original_pct;
			THEN("Various traits change") {
				REQUIRE(pack.GetType() == MetaData::Of<Text>());
				REQUIRE(pack.Is<Text>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE_THROWS(pack.As<int>() == 0);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(pack.As<Text>() == original_pct);
				REQUIRE_THROWS(pack.As<int*>() == nullptr);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				REQUIRE(*pack.As<Text*>() == original_pct);
				REQUIRE(pack.As<Text*>()->HasAuthority());
				REQUIRE(pack.As<Text*>()->GetUses() == 2);
			}
		}

		WHEN("Given dynamic text") {
			Allocator::CollectGarbage();
			Text original_pct = u8"Lorep Ipsum";
			Any pack;
			pack = original_pct.Clone();
			THEN("Various traits change") {
				REQUIRE(pack.GetType() == MetaData::Of<Text>());
				REQUIRE(pack.Is<Text>());
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE_THROWS(pack.As<int>() == 0);
				REQUIRE_THROWS(pack.As<float>() == 0.0f);
				REQUIRE(pack.As<Text>() == original_pct);
				REQUIRE_THROWS(pack.As<int*>() == nullptr);
				REQUIRE_THROWS(pack.As<float*>() == nullptr);
				REQUIRE(*pack.As<Text*>() == original_pct);
				REQUIRE(pack.As<Text*>()->HasAuthority());
				REQUIRE(pack.As<Text*>()->GetUses() == 1);
			}
		}

		WHEN("Given dynamic text, which is later referenced multiple times") {
			Allocator::CollectGarbage();
			Text original_pct = u8"Lorep Ipsum";
			Any pack;
			pack = original_pct.Clone();
			Any pack2(pack);
			Any pack3(pack2);
			Any pack4(pack3);

			THEN("Various traits change") {
				REQUIRE(pack4.GetType() == MetaData::Of<Text>());
				REQUIRE(pack4.Is<Text>());
				REQUIRE(pack4.GetRaw() != nullptr);
				REQUIRE_THROWS(pack4.As<int>() == 0);
				REQUIRE_THROWS(pack4.As<float>() == 0.0f);
				REQUIRE(pack4.As<Text>() == original_pct);
				REQUIRE_THROWS(pack4.As<int*>() == nullptr);
				REQUIRE_THROWS(pack4.As<float*>() == nullptr);
				REQUIRE(*pack4.As<Text*>() == original_pct);
				REQUIRE(pack4.As<Text*>()->HasAuthority());
				REQUIRE(pack.GetUses() == 4);
				REQUIRE(pack2.GetUses() == 4);
				REQUIRE(pack3.GetUses() == 4);
				REQUIRE(pack4.GetUses() == 4);
				REQUIRE(pack4.As<Text*>()->GetUses() == 1);
			}
		}

		WHEN("Given dynamic text, which is later referenced multiple times, and then dereferenced") {
			Allocator::CollectGarbage();
			Text original_pct = u8"Lorep Ipsum";
			Any pack;
			pack = original_pct.Clone();
			Any pack2(pack);
			Any pack3(pack2);
			Any pack4(pack3);
			pack.Reset();
			pack3.Reset();

			THEN("Various traits change") {
				REQUIRE(pack4.GetType() == MetaData::Of<Text>());
				REQUIRE(pack4.Is<Text>());
				REQUIRE(pack4.GetRaw());
				REQUIRE_THROWS(pack4.As<int>() == 0);
				REQUIRE_THROWS(pack4.As<float>() == 0.0f);
				REQUIRE(pack4.As<Text>() == original_pct);
				REQUIRE_THROWS(pack4.As<int*>() == nullptr);
				REQUIRE_THROWS(pack4.As<float*>() == nullptr);
				REQUIRE(*pack4.As<Text*>() == original_pct);
				REQUIRE(pack4.As<Text*>()->HasAuthority());
				REQUIRE(pack.GetUses() == 0);
				REQUIRE(pack2.GetUses() == 2);
				REQUIRE(pack3.GetUses() == 0);
				REQUIRE(pack4.GetUses() == 2);
				REQUIRE(pack4.As<Text*>()->GetUses() == 1);

				REQUIRE(pack.GetType() == nullptr);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack3.GetType() == nullptr);
				REQUIRE(pack3.GetRaw() == nullptr);
			}
		}
	}

	GIVEN("A universal Any with some POD items") {
		Allocator::CollectGarbage();
		Any pack;
		pack << int(1) << int(2) << int(3) << int(4) << int(5);
		auto memory = pack.GetRaw();

		REQUIRE(pack.GetCount() == 5);
		REQUIRE(pack.GetReserved() >= 5);
		REQUIRE(pack.Is<int>());
		REQUIRE(pack.GetRaw());

		WHEN("Push more stuff") {
			pack << int(6) << int(7) << int(8) << int(9) << int(10);
			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetUses() == 1);
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
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.GetUses() == 1);
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
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.GetUses() == 0);
				REQUIRE(pack.GetState() == DataState::Default);
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
				REQUIRE(pack.GetUses() == 1);
			}
		}

		WHEN("Less capacity is reserved") {
			pack.Allocate(2);
			THEN("Neither size nor capacity are changed") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.GetUses() == 1);
			}
		}

		WHEN("Pack is cleared") {
			pack.Clear();
			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.Is<int>());
				REQUIRE(pack.GetUses() == 1);
			}
		}

		WHEN("Pack is reset") {
			pack.Reset();
			THEN("Size and capacity goes to zero, type is reset to udAny") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.GetType() == nullptr);
				REQUIRE(pack.GetUses() == 0);
			}
		}

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			WHEN("Pack is reset, then immediately allocated again") {
				pack.Reset();
				pack << int(6) << int(7) << int(8) << int(9) << int(10);
				THEN("Block manager should reuse the memory if MANAGED_MEMORY feature is enabled") {
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
				REQUIRE(copy.GetUses() == 2);
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
				REQUIRE(clone.GetUses() == 1);
				REQUIRE(pack.GetUses() == 1);
			}
		}

		WHEN("Pack is moved") {
			pack.MakeOr();
			Any moved = Move(pack);
			THEN("The new pack should keep the state and data") {
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetState() == DataState::Default);
				REQUIRE(pack.GetType() == nullptr);
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
		Allocator::CollectGarbage();
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
			THEN("The entire container is reset") {
				REQUIRE(pack.IsEmpty());
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.Is<Any>());
				REQUIRE(pack.IsTypeConstrained());
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.GetUses() == 0);
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
			auto result = subpack1.SmartPush<true, false>(u8'?');
			THEN("The pack must remain unchanged") {
				REQUIRE(result == 0);
				REQUIRE(subpack1.GetCount() == 5);
			}
		}

		WHEN("Smart pushing with retainment") {
			Any deepened;
			deepened << int(1) << int(2) << int(3) << int(4) << int(5);
			auto result = deepened.SmartPush<false, true>(u8'?');
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
			auto pushed = Any::FromMeta(nullptr, DataState {DataState::Phased | DataState::Missing});
			auto result = deepened.SmartPush<true, true>(pushed);
			THEN("The pack must get deeper and contain it") {
				REQUIRE(result == 1);
				REQUIRE(deepened.IsDeep());
				REQUIRE(deepened.GetCount() == 2);
				REQUIRE(deepened.As<Any>(0).GetCount() == 5);
				REQUIRE(deepened.As<Any>(1).GetCount() == 0);
				REQUIRE(deepened.As<Any>(1).GetState() == DataState::Phased + DataState::Missing);
			}
		}

		WHEN("Smart pushing an empty container (but not stateless) with retainment to another empty container") {
			auto pushed = Any::FromMeta(nullptr, DataState {DataState::Phased | DataState::Missing});
			auto pushed2 = Any::FromMeta(nullptr, DataState {});
			auto result = pushed2.SmartPush<true, true>(pushed);
			THEN("The pack must get deeper and contain it") {
				REQUIRE(result == 1);
				REQUIRE(pushed2.GetCount() == 0);
				REQUIRE(pushed2.GetState() == DataState::Phased + DataState::Missing);
			}
		}

		WHEN("Smart pushing to an empty container (concat & retain enabled)") {
			Any pushed;
			auto result = pushed.SmartPush<true, true>(pack);
			THEN("The empty container becomes the pushed container") {
				REQUIRE(pushed == pack);
				REQUIRE(result == 1);
			}
		}

		WHEN("Smart pushing to a different container with retain enabled") {
			Any pushed;
			pushed << 666;
			pushed.MakeOr();
			auto result = pushed.SmartPush<true, true>(u8'?');
			THEN("Must not duplicate state of deepened container") {
				REQUIRE(result == 1);
				REQUIRE(!pushed.IsOr());
				REQUIRE(pushed.As<Any>(0).IsOr());
				REQUIRE(!pushed.As<Any>(1).IsOr());
			}
		}
	}

	GIVEN("A universal Any with some deep items for the purpose of optimization") {
		Allocator::CollectGarbage();
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

	GIVEN("A universal Any with some deep items, and their Blocks coalesced") {
		Allocator::CollectGarbage();
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
			Base base;
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
