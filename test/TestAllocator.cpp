///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>
#include <random>

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md			
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
	const Text serialized {ex};
	return ::std::string {Token {serialized}};
}

using timer = Catch::Benchmark::Chronometer;
template<class T>
using some = std::vector<T>;
template<class T>
using uninitialized = Catch::Benchmark::storage_for<T>;

std::random_device rd;
std::mt19937 gen(rd());

using Type1 = uint8_t;
using Type2 = uint16_t;
using Type4 = uint32_t;
using Type8 = uint64_t;

struct TypeBig {
	Type1 t1;
	Type2 t2;
	Type4 t4;
	Type8 t8;
};

struct TypeVeryBig {
	TypeBig t1;
	TypeBig t2;
	TypeBig t4;
	TypeBig t8[5];
};

bool IsAligned(const void* a) noexcept {
	return 0 == (reinterpret_cast<Pointer>(a) & Pointer {Alignment - 1});
}

SCENARIO("Testing CountLeadingZeroes calls", "[allocator]") {
	const Size numbers[] {
		0, 1, 2, 3, 4, 5, 6, 11, 16, 64, 99, 120, 128
	};

	#if LANGULUS(BITNESS) == 32
		const Size results[] {
			32, 31, 30, 30, 29, 29, 29, 28, 27, 25, 25, 25, 24
		};
	#elif LANGULUS(BITNESS) == 64
		const Size results[] {
			64, 63, 62, 62, 61, 61, 61, 60, 59, 57, 57, 57, 56
		};
	#endif

	static_assert(sizeof(numbers) == sizeof(results), "Oops");

	WHEN("CountLeadingZeroes is executed") {
		THEN("Results should be correct") {
			for (unsigned i = 0; i < sizeof(numbers) / sizeof(Size); ++i) {
				REQUIRE(CountLeadingZeroes(numbers[i]) == static_cast<int>(results[i]));
			}
		}
	}
}

SCENARIO("Testing CountTrailingZeroes calls", "[allocator]") {
	const Size numbers[] {
		0, 1, 2, 3, 4, 5, 6, 11, 16, 64, 99, 120, 128
	};

	#if LANGULUS(BITNESS) == 32
		const Size results[] {
			32, 0, 1, 0, 2, 0, 1, 0, 4, 6, 0, 3, 7
		};
	#elif LANGULUS(BITNESS) == 64
		const Size results[] {
			64, 0, 1, 0, 2, 0, 1, 0, 4, 6, 0, 3, 7
		};
	#endif

	static_assert(sizeof(numbers) == sizeof(results), "Oops");

	WHEN("CountTrailingZeroes is executed") {
		THEN("Results should be correct") {
			for (unsigned i = 0; i < sizeof(numbers) / sizeof(Size); ++i) {
				REQUIRE(CountTrailingZeroes(numbers[i]) == static_cast<int>(results[i]));
			}
		}
	}
}

TEMPLATE_TEST_CASE("Testing IsPowerOfTwo calls", "[allocator]", uint8_t, uint16_t, uint32_t, uint64_t) {
	using T = TestType;
	const T numbers[] {
		0, 1, 2, 3, 4, 5, 6, 11, 16, 64, 99, 120, 128
	};
	const bool results[] {
		false, true, true, false, true, false, false, false, true, true, false, false, true
	};
	static_assert(sizeof(numbers)/sizeof(T) == sizeof(results)/sizeof(bool), "Oops");

	WHEN("IsPowerOfTwo is executed") {
		THEN("Results should be correct") {
			for (unsigned i = 0; i < sizeof(numbers) / sizeof(T); ++i) {
				REQUIRE(IsPowerOfTwo(numbers[i]) == results[i]);
			}
		}
	}
}

TEMPLATE_TEST_CASE("Testing Roof2 calls", "[allocator]", uint8_t, uint16_t, uint32_t, uint64_t) {
	using T = TestType;
	const T numbers[] {
		0, 1, 2, 3, 4, 5, 6, 11, 16, 64, 99, 120, 128
	};
	const T results[] {
		0, 1, 2, 4, 4, 8, 8, 16, 16, 64, 128, 128, 128
	};
	static_assert(sizeof(numbers) == sizeof(results), "Oops");

	WHEN("Roof2 is executed") {
		THEN("Results should be correct") {
			for (unsigned i = 0; i < sizeof(numbers) / sizeof(T); ++i) {
				if (numbers[i] <= 128 || sizeof(T) > 1) {
					REQUIRE(Roof2<true>(numbers[i]) == results[i]);
					REQUIRE(Roof2cexpr<true>(numbers[i]) == results[i]);
				}
				else {
					REQUIRE_THROWS_AS(Roof2<true>(numbers[i]), Except::Overflow);
					REQUIRE_THROWS_AS(Roof2cexpr<true>(numbers[i]), Except::Overflow);
				}
			}
		}

		#ifdef LANGULUS_STD_BENCHMARK // Last result: 
			BENCHMARK_ADVANCED("Roof2 with instrinsics") (timer meter) {
				meter.measure([&](int i) {
					return Roof2(static_cast<T>(i % 256));
				});
			};
			BENCHMARK_ADVANCED("Roof2 without intrinsics") (timer meter) {
				meter.measure([&](int i) {
					return Roof2cexpr(static_cast<T>(i % 256));
				});
			};
		#endif
	}
}

SCENARIO("Testing FastLog2 calls", "[allocator]") {
	const Size numbers[] {
		0, 1, 2, 3, 4, 5, 6, 11, 16, 64, 99, 120, 128
	};
	const Size results[] {
		0, 0, 1, 1, 2, 2, 2,  3,  4,  6,  6,   6,   7
	};
	static_assert(sizeof(numbers) == sizeof(results), "Oops");

	WHEN("FastLog2 is executed") {
		THEN("Results should be correct") {
			for (unsigned i = 0; i < sizeof(numbers) / sizeof(Size); ++i) {
				REQUIRE(Anyness::Inner::FastLog2(numbers[i]) == results[i]);
			}
		}
	}
}

TEMPLATE_TEST_CASE("Testing GetAllocationPageOf<T> calls", "[allocator]", Type1, Type2, Type4, Type8, TypeBig, TypeVeryBig) {
	WHEN("GetAllocationPageOf<T> is executed") {
		THEN("Results should be correct") {
			REQUIRE(IsPowerOfTwo(RTTI::GetAllocationPageOf<TestType>()));
			REQUIRE(RTTI::GetAllocationPageOf<TestType>() >= sizeof(TestType));
		}
	}
}

#if LANGULUS_FEATURE(MANAGED_MEMORY)
SCENARIO("Testing pool functions", "[allocator]") {
	GIVEN("A pool") {
		Pool* pool = nullptr;

		WHEN("Default pool size is allocated on the pool") {
			pool = Allocator::AllocatePool(Pool::DefaultPoolSize);
			REQUIRE(pool);

			const auto originPtr = pool->GetPoolStart<Byte>();
			const auto smallest = pool->GetMinAllocation();
			const auto origin = reinterpret_cast<Pointer>(originPtr);
			const auto full = pool->GetAllocatedByBackend();
			const auto half = full / 2;
			const auto quarter = half / 2;

			THEN("Requirements should be met") {
				REQUIRE(IsPowerOfTwo(pool->GetAllocatedByBackend()));
				REQUIRE(IsPowerOfTwo(pool->GetMinAllocation()));
				REQUIRE(IsPowerOfTwo(pool->GetMaxEntries()));
				REQUIRE(IsAligned(pool->GetPoolStart()));
				REQUIRE(pool->GetAllocatedByBackend() == Pool::DefaultPoolSize);
				REQUIRE(reinterpret_cast<Pointer>(pool->AllocationFromIndex(0)) == origin);
				REQUIRE(reinterpret_cast<Pointer>(pool->AllocationFromIndex(1)) == origin + half);
				REQUIRE(reinterpret_cast<Pointer>(pool->AllocationFromIndex(2)) == origin + quarter);
				REQUIRE(reinterpret_cast<Pointer>(pool->AllocationFromIndex(3)) == origin + quarter + half);
				REQUIRE(pool->ThresholdFromIndex(1) == half);
				REQUIRE(pool->ThresholdFromIndex(2) == quarter);
				REQUIRE(pool->ThresholdFromIndex(3) == quarter);
				REQUIRE(pool->ThresholdFromIndex(4) == quarter / 2);
				REQUIRE(pool->ThresholdFromIndex(5) == quarter / 2);
				REQUIRE(pool->ThresholdFromIndex(6) == quarter / 2);
				REQUIRE(pool->ThresholdFromIndex(7) == quarter / 2);
				REQUIRE(pool->ThresholdFromIndex(8) == quarter / 4);
				REQUIRE(pool->ThresholdFromIndex(pool->GetMaxEntries() - 1) == smallest);
				REQUIRE(pool->ThresholdFromIndex(pool->GetMaxEntries()) == smallest / 2);
				REQUIRE(pool->CanContain(1));
				REQUIRE(pool->CanContain(Alignment));
				REQUIRE(pool->CanContain(smallest));
				REQUIRE(pool->CanContain(half));
				REQUIRE(pool->CanContain(full));
				REQUIRE_FALSE(pool->CanContain(full + 1));
				REQUIRE(pool->GetAllocatedByFrontend() == 0);
				REQUIRE(pool->GetMaxEntries() == full / smallest);
				REQUIRE(pool->Contains(originPtr));
				REQUIRE(pool->Contains(originPtr + half));
				REQUIRE(pool->Contains(originPtr + half * 2 - 1));
				REQUIRE_FALSE(pool->Contains(originPtr + half * 2));
				REQUIRE_FALSE(pool->Contains(nullptr));
				REQUIRE_FALSE(pool->IsInUse());
			}

			Allocator::DeallocatePool(pool);
		}

		WHEN("A small entry is allocated inside a new default-sized pool") {
			pool = Allocator::AllocatePool(Pool::DefaultPoolSize);
			REQUIRE(pool);

			auto entry = pool->Allocate(5);
			const auto full = pool->GetAllocatedByBackend();
			const auto smallest = pool->GetMinAllocation();

			THEN("Requirements should be met") {
				REQUIRE(pool->GetAllocatedByFrontend() == entry->GetTotalSize());
				REQUIRE(pool->GetMaxEntries() == full / smallest);
				REQUIRE(pool->Contains(entry));
				REQUIRE(pool->IsInUse());
			}

			Allocator::DeallocatePool(pool);
		}

		WHEN("A small entry is allocated inside a new huge pool") {
			if constexpr (Bitness == 32)
				pool = Allocator::AllocatePool(Pool::DefaultPoolSize * 1024);
			else
				pool = Allocator::AllocatePool(Pool::DefaultPoolSize * 1024 * 4);

			REQUIRE(pool);

			auto entry = pool->Allocate(5);
			const auto full = pool->GetAllocatedByBackend();
			const auto smallest = pool->GetMinAllocation();

			THEN("Requirements should be met") {
				REQUIRE(pool->GetAllocatedByFrontend() == entry->GetTotalSize());
				REQUIRE(pool->GetMaxEntries() == full / smallest);
				REQUIRE(pool->Contains(entry));
				REQUIRE(pool->IsInUse());
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 
				BENCHMARK_ADVANCED("Pool::Allocate(5)") (timer meter) {
					std::vector<Allocation*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = pool->Allocate(5);
					});
					
					for (auto& i : storage) {
						if (i)
							pool->Deallocate(i);
						else {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because the pool got full - use a bigger pool");
						}
					}
				};

				BENCHMARK_ADVANCED("std::malloc(5)") (timer meter) {
					std::vector<void*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = ::std::malloc(5);
					});

					for (auto& i : storage) {
						if (i)
							::std::free(i);
						else {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because malloc returned a zero");
						}
					}
				};

				BENCHMARK_ADVANCED("Pool::Allocate(32)") (timer meter) {
					std::vector<Allocation*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = pool->Allocate(32);
					});

					for (auto& i : storage) {
						if (i)
							pool->Deallocate(i);
						else {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because the pool got full - use a bigger pool");
						}
					}
				};

				BENCHMARK_ADVANCED("std::malloc(32)") (timer meter) {
					std::vector<void*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = ::std::malloc(32);
					});

					for (auto& i : storage) {
						if (i)
							::std::free(i);
						else {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because malloc returned a zero");
						}
					}
				};

				BENCHMARK_ADVANCED("Pool::Reallocate(32 -> 5)") (timer meter) {
					std::vector<Allocation*> storage(meter.runs());
					for (auto& i : storage) {
						i = pool->Allocate(32);
						if (!i) {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because the pool got full - use a bigger pool");
						}
					}

					meter.measure([&](int i) {
						const auto r = pool->Reallocate(storage[i], 5);
						if (r)
							storage[i] = storage[i];
						return r;
					});

					for (auto& i : storage)
						pool->Deallocate(i);
				};

				BENCHMARK_ADVANCED("std::realloc(32 -> 5)") (timer meter) {
					std::vector<void*> storage(meter.runs());
					for (auto& i : storage) {
						i = ::std::malloc(32);
						if (!i) {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because malloc returned a zero");
						}
					}

					meter.measure([&](int i) {
						const auto r = ::std::realloc(storage[i], 5);
						if (r)
							storage[i] = r;
						return r;
					});

					for (auto& i : storage)
						::std::free(i);
				};

				BENCHMARK_ADVANCED("Pool::Reallocate(5 -> 32)") (timer meter) {
					std::vector<Allocation*> storage(meter.runs());
					for (auto& i : storage) {
						i = pool->Allocate(5);
						if (!i) {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because the pool got full - use a bigger pool");
						}
					}

					meter.measure([&](int i) {
						const auto r = pool->Reallocate(storage[i], 32);
						if (r)
							storage[i] = storage[i];
						return r;
					});

					for (auto& i : storage)
						pool->Deallocate(i);
				};

				BENCHMARK_ADVANCED("std::realloc(5 -> 32)") (timer meter) {
					std::vector<void*> storage(meter.runs());
					for (auto& i : storage) {
						i = ::std::malloc(5);
						if (!i) {
							LANGULUS_THROW(Deallocate,
								"The test is invalid, because malloc returned a zero");
						}
					}

					meter.measure([&](int i) {
						const auto r = ::std::realloc(storage[i], 32);
						if (r)
							storage[i] = r;
						return r;
					});

					for (auto& i : storage)
						::std::free(i);
				};
			#endif

			Allocator::DeallocatePool(pool);
		}

		WHEN("A new default-sized pool is filled with all possible small entries") {
			pool = Allocator::AllocatePool(Pool::DefaultPoolSize);
			REQUIRE(pool);

			// Fill up
			for (Count i = 0; i < pool->GetMaxEntries(); ++i) {
				auto entry = pool->Allocate(5);
				REQUIRE(entry);
				entry->Keep(i);

				// Fill the entire entry to check for heap corruptions
				for (Size i2 = 0; i2 < entry->GetAllocatedSize(); ++i2) {
					entry->GetBlockStart()[i2] = {};
				}
			}

			// Add more
			for (int i = 0; i < 5; ++i) {
				auto entry = pool->Allocate(5);
				REQUIRE(entry == nullptr);
			}

			const auto full = pool->GetAllocatedByBackend();
			const auto smallest = pool->GetMinAllocation();

			THEN("Requirements should be met") {
				REQUIRE(pool->GetAllocatedByFrontend() == pool->GetMaxEntries() * Allocation::GetNewAllocationSize(5));
				REQUIRE(pool->GetMaxEntries() == full / smallest);
				for (Count i = 0; i < pool->GetMaxEntries(); ++i) {
					auto entry = pool->AllocationFromIndex(i);
					REQUIRE(pool->Contains(entry));
					REQUIRE(entry->GetUses() == 1 + i);
				}
			}

			Allocator::DeallocatePool(pool);
		}

		WHEN("An entry larger than the minimum is allocated inside a new default-sized pool") {
			pool = Allocator::AllocatePool(Pool::DefaultPoolSize);
			auto entry = pool->Allocate(Allocation::GetMinAllocation());

			THEN("Requirements should be met") {
				REQUIRE(entry);
				REQUIRE(pool->GetAllocatedByFrontend() == entry->GetTotalSize());
				REQUIRE(pool->GetMinAllocation() == Roof2(entry->GetTotalSize()));
				REQUIRE(pool->GetMaxEntries() == pool->GetAllocatedByBackend() / pool->GetMinAllocation());
				REQUIRE(pool->Contains(entry));
				REQUIRE(pool->IsInUse());
			}

			Allocator::DeallocatePool(pool);
		}

		WHEN("An entry larger than the pool itself is allocated inside a new default-sized pool") {
			pool = Allocator::AllocatePool(Pool::DefaultPoolSize);
			auto entry = pool->Allocate(Pool::DefaultPoolSize * 2);

			THEN("The resulting allocation should be invalid") {
				REQUIRE(entry == nullptr);
				REQUIRE(pool->GetAllocatedByFrontend() == 0);
				REQUIRE_FALSE(pool->IsInUse());
			}

			Allocator::DeallocatePool(pool);
		}
	}
}
#endif

SCENARIO("Testing allocator functions", "[allocator]") {
	GIVEN("An allocation") {
		Allocation* entry = nullptr;

		WHEN("Memory is allocated on the heap") {
			#include "CollectGarbage.inl"

			entry = Allocator::Allocate(512);

			THEN("Requirements should be met") {
				REQUIRE(entry);
				REQUIRE(entry->GetBlockStart() != nullptr);
				REQUIRE(entry->GetBlockStart() != reinterpret_cast<Byte*>(entry));
				REQUIRE(reinterpret_cast<Pointer>(entry) % Alignment == 0);
				REQUIRE(reinterpret_cast<Pointer>(entry->GetBlockStart()) % Alignment == 0);
				REQUIRE(entry->GetAllocatedSize() >= 512);
				REQUIRE(entry->GetBlockEnd() == entry->GetBlockStart() + entry->GetAllocatedSize());
				REQUIRE(entry->GetSize() % Alignment == 0);
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

			Allocator::Deallocate(entry);

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 
				#include "CollectGarbage.inl"

				BENCHMARK_ADVANCED("Allocator::Allocate(5)") (timer meter) {
					std::vector<Allocation*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Allocator::Allocate(5);
					});

					for (auto& i : storage) {
						if (i)
							Allocator::Deallocate(i);
						else
							LANGULUS_THROW(Deallocate, "The test is invalid, because memory got full");
					}
				};

				#include "CollectGarbage.inl"

				BENCHMARK_ADVANCED("malloc(5)") (timer meter) {
					std::vector<void*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = ::std::malloc(5);
					});

					for (auto& i : storage) {
						if (i)
							::std::free(i);
						else
							LANGULUS_THROW(Deallocate, "The test is invalid, because memory got full");
					}
				};

				#include "CollectGarbage.inl"

				BENCHMARK_ADVANCED("Allocator::Allocate(512)") (timer meter) {
					std::vector<Allocation*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Allocator::Allocate(512);
					});

					for (auto& i : storage) {
						if (i)
							Allocator::Deallocate(i);
						else
							LANGULUS_THROW(Deallocate, "The test is invalid, because memory got full");
					}
				};

				#include "CollectGarbage.inl"

				BENCHMARK_ADVANCED("malloc(512)") (timer meter) {
					std::vector<void*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = ::std::malloc(512);
					});

					for (auto& i : storage) {
						if (i)
							::std::free(i);
						else
							LANGULUS_THROW(Deallocate, "The test is invalid, because memory got full");
					}
				};

				#include "CollectGarbage.inl"

				BENCHMARK_ADVANCED("Allocator::Allocate(Pool::DefaultPoolSize)") (timer meter) {
					std::vector<Allocation*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Allocator::Allocate(1024*1024);
					});

					for (auto& i : storage) {
						if (i)
							Allocator::Deallocate(i);
						else
							LANGULUS_THROW(Deallocate, "The test is invalid, because memory got full");
					}
				};

				#include "CollectGarbage.inl"

				BENCHMARK_ADVANCED("malloc(Pool::DefaultPoolSize)") (timer meter) {
					std::vector<void*> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = ::std::malloc(1024 * 1024);
					});

					for (auto& i : storage) {
						if (i)
							::std::free(i);
						else
							LANGULUS_THROW(Deallocate, "The test is invalid, because memory got full");
					}
				};
			#endif
		}

		WHEN("Referenced once") {
			#include "CollectGarbage.inl"

			entry = Allocator::Allocate(512);
			REQUIRE(entry);
			entry->Keep();

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 2);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(nullptr, entry));
					REQUIRE(Allocator::Find(nullptr, entry->GetBlockStart()));
					REQUIRE_FALSE(Allocator::Find(nullptr, entry));
				#endif
			}

			SAFETY(REQUIRE_THROWS(Allocator::Deallocate(entry)));
			entry->Free();
			Allocator::Deallocate(entry);
		}

		WHEN("Referenced multiple times") {
			#include "CollectGarbage.inl"

			entry = Allocator::Allocate(512);
			REQUIRE(entry);
			entry->Keep(5);

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 6);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(nullptr, entry));
					REQUIRE(Allocator::Find(nullptr, entry->GetBlockStart()));
					REQUIRE_FALSE(Allocator::Find(nullptr, entry));
				#endif
			}

			SAFETY(REQUIRE_THROWS(Allocator::Deallocate(entry)));
			entry->Free(5);
			Allocator::Deallocate(entry);
		}

		WHEN("Dereferenced once without deletion") {
			#include "CollectGarbage.inl"

			entry = Allocator::Allocate(512);
			REQUIRE(entry);
			entry->Keep();
			entry->Free();

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 1);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(nullptr, entry));
					REQUIRE(Allocator::Find(nullptr, entry->GetBlockStart()));
					REQUIRE_FALSE(Allocator::Find(nullptr, entry));
				#endif
			}

			Allocator::Deallocate(entry);
		}

		WHEN("Dereferenced multiple times without deletion") {
			#include "CollectGarbage.inl"

			entry = Allocator::Allocate(512);
			REQUIRE(entry);
			entry->Keep(5);
			entry->Free(4);

			THEN("Requirements should be met") {
				REQUIRE(entry->GetUses() == 2);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(Allocator::CheckAuthority(nullptr, entry));
					REQUIRE(Allocator::Find(nullptr, entry->GetBlockStart()));
					REQUIRE_FALSE(Allocator::Find(nullptr, entry));
				#endif
			}

			SAFETY(REQUIRE_THROWS(Allocator::Deallocate(entry)));
			entry->Free(1);
			Allocator::Deallocate(entry);
		}

		WHEN("Dereferenced once with deletion") {
			#include "CollectGarbage.inl"

			entry = Allocator::Allocate(512);
			REQUIRE(entry);
			Allocator::Deallocate(entry);

			THEN("We shouldn't be able to access the memory any longer, but it is still under jurisdiction") {
			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				REQUIRE(Allocator::CheckAuthority(nullptr, entry));
				REQUIRE_FALSE(Allocator::Find(nullptr, entry->GetBlockStart()));
				REQUIRE_FALSE(Allocator::Find(nullptr, entry));
			#endif
			}
		}

		WHEN("Dereferenced multiple times with deletion") {
			#include "CollectGarbage.inl"

			entry = Allocator::Allocate(512);
			REQUIRE(entry);
			entry->Keep(5);

			SAFETY(REQUIRE_THROWS(Allocator::Deallocate(entry)));
			entry->Free(5);
			Allocator::Deallocate(entry);

			THEN("We shouldn't be able to access the memory any longer, but it is still under jurisdiction") {
			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				REQUIRE(Allocator::CheckAuthority(nullptr, entry));
				REQUIRE_FALSE(Allocator::Find(nullptr, entry->GetBlockStart()));
				REQUIRE_FALSE(Allocator::Find(nullptr, entry));
			#endif
			}
		}
	}
}
