///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>

using uint = unsigned int;
using timer = Catch::Benchmark::Chronometer;
template<class T>
using some = std::vector<T>;
template<class T>
using uninitialized = Catch::Benchmark::storage_for<T>;

template<class C, class E>
struct TypePair {
	using Container = C;
	using Element = E;
};

Byte* asbytes(void* a) noexcept {
	return reinterpret_cast<Byte*>(a);
}

const Byte* asbytes(const void* a) noexcept {
	return reinterpret_cast<const Byte*>(a);
}

/// Detect if type is TAny, instead of Any, by searching for [] operator		
template<class T>
concept IsStaticallyOptimized = CT::Deep<T> && requires (const T& a) { {a[Offset {0}]}; };

/// Get simple value, no matter if inside container or not							
template<class T, CT::Dense SOURCE>
decltype(auto) Resolve(const SOURCE& s) {
	if constexpr (CT::Same<T, SOURCE>)
		return s;
	else if constexpr (CT::Fundamental<SOURCE>)
		return static_cast<T>(s);
	else
		return s.template As<T>();
}

/// The main test for Any/TAny containers, with all kinds of items, from		
/// sparse to dense, from trivial to complex, from flat to deep					
TEMPLATE_TEST_CASE("Any/TAny", "[any]", 
	(TypePair<TAny<int>, int>),
	(TypePair<TAny<Trait>, Trait>),
	(TypePair<TAny<Traits::Count>, Traits::Count>),
	(TypePair<TAny<Any>, Any>),
	(TypePair<TAny<int*>, int*>),
	(TypePair<TAny<Trait*>, Trait*>),
	(TypePair<TAny<Traits::Count*>, Traits::Count*>),
	(TypePair<TAny<Any*>, Any*>),
	(TypePair<Any, int>),
	(TypePair<Any, Trait>),
	(TypePair<Any, Traits::Count>),
	(TypePair<Any, Any>),
	(TypePair<Any, int*>),
	(TypePair<Any, Trait*>),
	(TypePair<Any, Traits::Count*>),
	(TypePair<Any, Any*>)
) {
	using T = typename TestType::Container;
	using E = typename TestType::Element;
	using StdT = std::vector<E>;
	using DenseE = Decay<E>;
		
	E element;
	if constexpr (CT::Sparse<E>)
		element = new DenseE {555};
	else
		element = 555;

	const DenseE& denseValue {DenseCast(element)};
	const DenseE* const sparseValue = {SparseCast(element)};


	GIVEN("Default constructed container") {
		T pack;

		THEN("Properties should match") {
			REQUIRE(pack.GetCount() == 0);
			REQUIRE_FALSE(pack.IsConstant());
			REQUIRE_FALSE(pack.IsCompressed());
			REQUIRE_FALSE(pack.IsAbstract());
			REQUIRE_FALSE(pack.IsAllocated());
			REQUIRE(pack.IsDeep() == (IsStaticallyOptimized<T> && CT::Deep<Decay<E>>));
			REQUIRE_FALSE(pack.IsEncrypted());
			REQUIRE_FALSE(pack.IsFuture());
			REQUIRE_FALSE(pack.IsPast());
			REQUIRE_FALSE(pack.IsPhased());
			REQUIRE_FALSE(pack.IsMissing());
			REQUIRE_FALSE(pack.IsStatic());
			REQUIRE_FALSE(pack.IsValid());
			REQUIRE(pack.IsNow());
			REQUIRE(pack.IsInvalid());
			REQUIRE(pack.GetRaw() == nullptr);
			REQUIRE(pack.IsEmpty());

			if constexpr (IsStaticallyOptimized<T>) {
				REQUIRE(pack.IsTypeConstrained());
				REQUIRE_FALSE(pack.IsUntyped());
				REQUIRE(pack.GetType() != nullptr);
				REQUIRE(pack.GetType()->template Is<E>());
				REQUIRE(pack.GetType()->template Is<DenseE>());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
				if constexpr (CT::Sparse<E>)
					REQUIRE(pack.GetState() == (DataState::Typed | DataState::Sparse));
				else
					REQUIRE(pack.GetState() == DataState::Typed);
			}
			else {
				REQUIRE_FALSE(pack.IsTypeConstrained());
				REQUIRE(pack.IsUntyped());
				REQUIRE(pack.GetType() == nullptr);
				REQUIRE(pack.IsDense());
				REQUIRE_FALSE(pack.IsSparse());
				REQUIRE(pack.GetState() == DataState::Default);
			}

			REQUIRE(pack.GetRaw() == nullptr);
			REQUIRE(pack.IsEmpty());
			REQUIRE_FALSE(pack.IsAllocated());
			REQUIRE(pack.GetUses() == 0);
		}

		#ifdef LANGULUS_STD_BENCHMARK
			BENCHMARK_ADVANCED("default construction") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<T>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct();
				});
			};

			BENCHMARK_ADVANCED("std::vector::default construction") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<StdT>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct();
				});
			};

			BENCHMARK_ADVANCED("std::any::default construction") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<std::any>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct();
				});
			};
		#endif

		WHEN("Assigned value by copy") {
			pack = element;

			THEN("Properties should match") {
				REQUIRE(pack.GetType() != nullptr);
				if constexpr (CT::Flat<E>) {
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.HasAuthority());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == element.GetUses());
					REQUIRE(pack.GetUses() == 2);
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
					REQUIRE(pack.IsStatic() == element.IsStatic());
					REQUIRE(pack.HasAuthority() == element.HasAuthority());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.template As<DenseE>().IsStatic() == element.IsStatic());
					REQUIRE(pack.template As<DenseE>().IsConstant() == element.IsConstant());
					REQUIRE(pack.template As<DenseE>().HasAuthority() == element.HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 2);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
					REQUIRE(pack.IsDeep());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = {value};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};
			#endif
		}
		
		WHEN("Assigned value by move") {
			auto movable = element;
			pack = Move(movable);

			THEN("Properties should match") {
				if constexpr (CT::Deep<E> && CT::Dense<E>) {
					REQUIRE(movable.IsEmpty());
					REQUIRE_FALSE(movable.IsAllocated());
					REQUIRE(movable != element);
				}
				REQUIRE(pack.GetType() != nullptr);
				REQUIRE(pack.GetRaw() != nullptr);
				if constexpr (CT::Flat<E>) {
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.HasAuthority());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == 2);
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
					REQUIRE(pack.IsStatic() == element.IsStatic());
					REQUIRE(pack.HasAuthority() == element.HasAuthority());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE_FALSE(pack.template As<DenseE>().IsStatic());
					REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
					REQUIRE(pack.template As<DenseE>().HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 2);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.IsDeep());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = {Move(value)};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};
			#endif
		}

		WHEN("Assigned disowned value") {
			pack = Disown(element);

			THEN("Properties should match") {
				if constexpr (CT::Flat<E>) {
					REQUIRE(pack.GetType() != nullptr);
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE(pack.HasAuthority());
					REQUIRE(pack.IsConstant() == CT::Constant<E>);
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetType() == element.GetType());
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == 0);
					REQUIRE(pack.IsStatic());
					REQUIRE_FALSE(pack.HasAuthority());
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template As<DenseE>().GetType() == element.GetType());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.template As<DenseE>().IsStatic());
					REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
					REQUIRE_FALSE(pack.template As<DenseE>().HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 0);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
					REQUIRE(pack.IsDeep());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single disowned value)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Disown(value);
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = {value};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};
			#endif
		}
		
		WHEN("Assigned abandoned value") {
			auto movable = element;
			pack = Abandon(movable);

			THEN("Properties should match") {
				if constexpr (CT::Deep<E> && CT::Dense<E>) {
					REQUIRE_FALSE(movable.IsEmpty());
					REQUIRE(movable.IsAllocated());
					REQUIRE(movable.IsStatic());
				}
				REQUIRE(pack.GetType() != nullptr);
				REQUIRE(pack.GetRaw() != nullptr);
				if constexpr (CT::Flat<E>) {
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.HasAuthority());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE(pack.IsConstant() == CT::Constant<E>);
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == 2);
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
					REQUIRE(pack.IsStatic() == element.IsStatic());
					REQUIRE(pack.HasAuthority() == element.HasAuthority());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE_FALSE(pack.template As<DenseE>().IsStatic());
					REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
					REQUIRE(pack.template As<DenseE>().HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 2);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.IsDeep());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single abandoned value)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Abandon(value);
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = {Move(value)};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};
			#endif
		}

		WHEN("Assigned empty self") {
			pack = pack;

			THEN("Various traits change") {
				if constexpr (IsStaticallyOptimized<T>) {
					REQUIRE(pack.GetType()->template Is<E>());
					REQUIRE(pack.GetType()->template Is<DenseE>());
					REQUIRE(pack.IsDense() == CT::Dense<E>);
					REQUIRE(pack.IsSparse() == CT::Sparse<E>);
					REQUIRE(pack.IsTypeConstrained());
				}
				else {
					REQUIRE_FALSE(pack.GetType());
					REQUIRE_FALSE(pack.IsTypeConstrained());
					REQUIRE(pack.IsDense());
					REQUIRE_FALSE(pack.IsSparse());
				}
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.IsEmpty());
				REQUIRE(pack.GetUses() == 0);
				REQUIRE(pack.IsDeep() == (IsStaticallyOptimized<T> && CT::Deep<Decay<E>>));
				REQUIRE_FALSE(pack.IsAllocated());
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};
			#endif
		}

		WHEN("Assigned full self") {
			pack = element;
			pack = pack;

			THEN("Various traits change") {
				REQUIRE(pack.IsTypeConstrained() == IsStaticallyOptimized<T>);
				if constexpr (IsStaticallyOptimized<T>) {
					REQUIRE(pack.GetType()->template Is<E>());
					REQUIRE(pack.GetType()->template Is<DenseE>());
				}
				else {
					REQUIRE(pack.GetType());
				}
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.GetUses() == (CT::Deep<E> && CT::Same<T,E> ? 2 : 1));
				REQUIRE(pack.IsDeep() == (CT::Deep<Decay<E>> && (CT::Sparse<E> || !CT::Same<T,E>)));
				REQUIRE(pack.IsAllocated());
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs());
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};
			#endif
		}
	}

	GIVEN("Container constructed by same container copy") {
		const T source {element};
		T pack {source};

		THEN("Properties should match") {
			REQUIRE(pack.GetType() != nullptr);
			if constexpr (CT::Flat<E>) {
				REQUIRE(pack.template Is<DenseE>());
				REQUIRE(pack.template Is<DenseE*>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE(pack.GetUses() == 2);
				REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
			}
			else if constexpr (CT::Same<E, T>) {
				REQUIRE(pack.Is(element.GetType()));
				REQUIRE(pack == source);
				REQUIRE(pack == element);
				REQUIRE(pack.GetUses() == 3);
				REQUIRE(pack.IsDeep() == element.IsDeep());
			}
			else {
				REQUIRE(pack.template Is<typename T::Type>());
				REQUIRE(pack == source);
				REQUIRE(pack != element);
				REQUIRE(pack.GetUses() == 2);
				REQUIRE(pack.IsDeep());
			}
			REQUIRE(pack.GetRaw() != nullptr);
			REQUIRE_THROWS(pack.template As<float>() == 0.0f);
			REQUIRE_FALSE(pack.IsEmpty());
			REQUIRE(pack.IsDense() == CT::Dense<E>);
			REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			REQUIRE_FALSE(pack.IsStatic());
			REQUIRE_FALSE(pack.IsConstant());
			REQUIRE(pack.HasAuthority());
			REQUIRE_THROWS(pack.template As<float*>() == nullptr);
		}

		#ifdef LANGULUS_STD_BENCHMARK
			BENCHMARK_ADVANCED("construction (single container copy)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<T>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(source);
				});
			};

			BENCHMARK_ADVANCED("std::vector::construction (single container copy)") (timer meter) {
				#include "CollectGarbage.inl"
				StdT source {1, 555};
				some<uninitialized<StdT>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(source);
				});
			};

			BENCHMARK_ADVANCED("std::any::construction (single container copy)") (timer meter) {
				#include "CollectGarbage.inl"
				std::any source {555};
				some<uninitialized<std::any>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(source);
				});
			};
		#endif
	}

	GIVEN("Container constructed by value copy") {
		T pack {element};

		THEN("Properties should match") {
			REQUIRE(pack.GetRaw() != nullptr);
			REQUIRE(pack.GetType() != nullptr);
			if constexpr (CT::Flat<E>) {
				REQUIRE(pack.template Is<DenseE>());
				REQUIRE(pack.template Is<DenseE*>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
			}
			else if constexpr (CT::Same<E, T>) {
				REQUIRE(pack.Is(element.GetType()));
				REQUIRE(pack == element);
				REQUIRE(pack.GetUses() == 2);
				REQUIRE(pack.IsDeep() == element.IsDeep());
			}
			else {
				REQUIRE(pack.template Is<typename T::Type>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE(pack != element);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(pack.IsDeep());
			}
			REQUIRE_THROWS(pack.template As<float>() == 0.0f);
			REQUIRE_THROWS(pack.template As<float*>() == nullptr);
			REQUIRE_FALSE(pack.IsEmpty());
			REQUIRE(pack.IsDense() == CT::Dense<E>);
			REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			REQUIRE_FALSE(pack.IsStatic());
			REQUIRE_FALSE(pack.IsConstant());
			REQUIRE(pack.HasAuthority());
		}

		#ifdef LANGULUS_STD_BENCHMARK
			BENCHMARK_ADVANCED("construction (single value copy)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<T>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(value);
				});
			};

			BENCHMARK_ADVANCED("std::vector::construction (single value copy)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<StdT>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(1, value);
				});
			};

			BENCHMARK_ADVANCED("std::any::construction (single value copy)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<std::any>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(value);
				});
			};
		#endif

		WHEN("Assigned compatible value by copy") {
			pack = element;

			THEN("Properties should match") {
				REQUIRE(pack.GetType() != nullptr);
				if constexpr (CT::Flat<E>) {
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.HasAuthority());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == element.GetUses());
					REQUIRE(pack.GetUses() == 2);
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
					REQUIRE(pack.IsStatic() == element.IsStatic());
					REQUIRE(pack.HasAuthority() == element.HasAuthority());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.template As<DenseE>().IsStatic() == element.IsStatic());
					REQUIRE(pack.template As<DenseE>().IsConstant() == element.IsConstant());
					REQUIRE(pack.template As<DenseE>().HasAuthority() == element.HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 2);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
					REQUIRE(pack.IsDeep());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = {value};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};
			#endif
		}
		
		WHEN("Assigned compatible value by move") {
			auto movable = element;
			pack = Move(movable);

			THEN("Properties should match") {
				if constexpr (CT::Deep<E> && CT::Dense<E>) {
					REQUIRE(movable.IsEmpty());
					REQUIRE_FALSE(movable.IsAllocated());
					REQUIRE(movable != element);
				}
				REQUIRE(pack.GetType() != nullptr);
				REQUIRE(pack.GetRaw() != nullptr);
				if constexpr (CT::Flat<E>) {
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.HasAuthority());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == 2);
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
					REQUIRE(pack.IsStatic() == element.IsStatic());
					REQUIRE(pack.HasAuthority() == element.HasAuthority());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE_FALSE(pack.template As<DenseE>().IsStatic());
					REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
					REQUIRE(pack.template As<DenseE>().HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 2);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.IsDeep());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = {Move(value)};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};
			#endif
		}

		WHEN("Assigned compatible disowned value") {
			pack = Disown(element);

			THEN("Properties should match") {
				if constexpr (CT::Flat<E>) {
					REQUIRE(pack.GetType() != nullptr);
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE(pack.HasAuthority());
					REQUIRE(pack.IsConstant() == CT::Constant<E>);
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetType() == element.GetType());
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == 0);
					REQUIRE(pack.IsStatic());
					REQUIRE_FALSE(pack.HasAuthority());
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template As<DenseE>().GetType() == element.GetType());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.template As<DenseE>().IsStatic());
					REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
					REQUIRE_FALSE(pack.template As<DenseE>().HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 0);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
					REQUIRE(pack.IsDeep());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single disowned value)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = Disown(value);
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = {value};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value copy)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = value;
					});
				};
			#endif
		}
		
		WHEN("Assigned compatible abandoned value") {
			auto movable = element;
			pack = Abandon(movable);

			THEN("Properties should match") {
				if constexpr (CT::Deep<E> && CT::Dense<E>) {
					REQUIRE_FALSE(movable.IsEmpty());
					REQUIRE(movable.IsAllocated());
					REQUIRE(movable.IsStatic());
				}
				REQUIRE(pack.GetType() != nullptr);
				REQUIRE(pack.GetRaw() != nullptr);
				if constexpr (CT::Flat<E>) {
					if constexpr (CT::Sparse<E>)
						REQUIRE(&pack.template As<DenseE>() == sparseValue);
					REQUIRE(pack.template Is<DenseE>());
					REQUIRE(pack.template Is<DenseE*>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.HasAuthority());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE(pack.IsConstant() == CT::Constant<E>);
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				else if constexpr (CT::Same<E, T>) {
					REQUIRE(pack.GetRaw() == element.GetRaw());
					REQUIRE(pack.Is(element.GetType()));
					REQUIRE(pack == element);
					REQUIRE(pack.GetUses() == 2);
					REQUIRE(pack.IsDeep() == element.IsDeep());
					REQUIRE(pack.IsConstant() == element.IsConstant());
					REQUIRE(pack.IsStatic() == element.IsStatic());
					REQUIRE(pack.HasAuthority() == element.HasAuthority());
				}
				else {
					REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
					REQUIRE(pack.template Is<typename T::Type>());
					REQUIRE(pack.template As<DenseE>() == denseValue);
					REQUIRE(*pack.template As<DenseE*>() == denseValue);
					REQUIRE_FALSE(pack.template As<DenseE>().IsStatic());
					REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
					REQUIRE(pack.template As<DenseE>().HasAuthority());
					REQUIRE(pack.template As<DenseE>().GetUses() == 2);
					REQUIRE(pack.template As<DenseE>() == element);
					REQUIRE(pack != element);
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.IsDeep());
					REQUIRE_FALSE(pack.IsStatic());
					REQUIRE_FALSE(pack.IsConstant());
					REQUIRE(pack.HasAuthority());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (single abandoned value)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = Abandon(value);
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = {Move(value)};
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (single value move)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = Move(value);
					});
				};
			#endif
		}

		WHEN("Assigned compatible empty self") {
			pack = pack;

			THEN("Various traits change") {
				if constexpr (CT::Same<T, E> && CT::Dense<E>) {
					REQUIRE(pack.GetType()->template Is<int>());
					REQUIRE(pack.GetType()->template Is<int*>());
					REQUIRE(pack.GetUses() == 2);
					REQUIRE_FALSE(pack.IsDeep());
				}
				else {
					REQUIRE(pack.GetType()->template Is<E>());
					REQUIRE(pack.GetType()->template Is<DenseE>());
					REQUIRE(pack.GetUses() == 1);
					REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
				}
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
				REQUIRE(pack.IsTypeConstrained() == IsStaticallyOptimized<T>);
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE(pack.IsAllocated());
				REQUIRE_FALSE(pack.IsEmpty());
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};
			#endif
		}

		WHEN("Assigned compatible full self") {
			pack = element;
			pack = pack;

			THEN("Various traits change") {
				REQUIRE(pack.IsTypeConstrained() == IsStaticallyOptimized<T>);
				if constexpr (IsStaticallyOptimized<T>) {
					REQUIRE(pack.GetType()->template Is<E>());
					REQUIRE(pack.GetType()->template Is<DenseE>());
				}
				else {
					REQUIRE(pack.GetType());
				}
				REQUIRE(pack.IsDense() == CT::Dense<E>);
				REQUIRE(pack.IsSparse() == CT::Sparse<E>);
				REQUIRE(pack.GetRaw() != nullptr);
				REQUIRE_FALSE(pack.IsEmpty());
				REQUIRE(pack.GetUses() == (CT::Deep<E> && CT::Same<T,E> ? 2 : 1));
				REQUIRE(pack.IsDeep() == (CT::Deep<Decay<E>> && (CT::Sparse<E> || !CT::Same<T,E>)));
				REQUIRE(pack.IsAllocated());
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<T> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::vector::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<StdT> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};

				BENCHMARK_ADVANCED("std::any::operator = (self)") (timer meter) {
					#include "CollectGarbage.inl"
					some<std::any> storage(meter.runs(), element);
					meter.measure([&](int i) {
						return storage[i] = storage[i];
					});
				};
			#endif
		}
	}

	GIVEN("Container constructed by value move") {
		E movable = element;
		T pack {Move(movable)};

		THEN("Properties should match") {
			if constexpr (CT::Deep<E> && CT::Dense<E>) {
				REQUIRE(movable.IsEmpty());
				REQUIRE_FALSE(movable.IsAllocated());
				REQUIRE(movable != element);
			}
			REQUIRE(pack.GetType() != nullptr);
			REQUIRE(pack.GetRaw() != nullptr);
			if constexpr (CT::Flat<E>) {
				if constexpr (CT::Sparse<E>)
					REQUIRE(&pack.template As<DenseE>() == sparseValue);
				REQUIRE(pack.template Is<DenseE>());
				REQUIRE(pack.template Is<DenseE*>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(pack.HasAuthority());
				REQUIRE_FALSE(pack.IsStatic());
				REQUIRE_FALSE(pack.IsConstant());
				REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
			}
			else if constexpr (CT::Same<E, T>) {
				REQUIRE(pack.GetRaw() == element.GetRaw());
				REQUIRE(pack.Is(element.GetType()));
				REQUIRE(pack == element);
				REQUIRE(pack.GetUses() == 2);
				REQUIRE(pack.IsDeep() == element.IsDeep());
				REQUIRE(pack.IsConstant() == element.IsConstant());
				REQUIRE(pack.IsStatic() == element.IsStatic());
				REQUIRE(pack.HasAuthority() == element.HasAuthority());
			}
			else {
				REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
				REQUIRE(pack.template Is<typename T::Type>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE_FALSE(pack.template As<DenseE>().IsStatic());
				REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
				REQUIRE(pack.template As<DenseE>().HasAuthority());
				REQUIRE(pack.template As<DenseE>().GetUses() == 2);
				REQUIRE(pack.template As<DenseE>() == element);
				REQUIRE(pack != element);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(pack.IsDeep());
				REQUIRE_FALSE(pack.IsStatic());
				REQUIRE_FALSE(pack.IsConstant());
				REQUIRE(pack.HasAuthority());
			}

			if constexpr (CT::Sparse<E>) {
				REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
				REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
			}

			REQUIRE_THROWS(pack.template As<float>() == 0.0f);
			REQUIRE_THROWS(pack.template As<float*>() == nullptr);
			REQUIRE_FALSE(pack.IsEmpty());
			REQUIRE(pack.IsDense() == CT::Dense<E>);
			REQUIRE(pack.IsSparse() == CT::Sparse<E>);
		}

		#ifdef LANGULUS_STD_BENCHMARK
			BENCHMARK_ADVANCED("construction (single value move)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<T>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(Move(value));
				});
			};

			BENCHMARK_ADVANCED("std::vector::construction (single value move)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<StdT>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(1, Move(value));
				});
			};

			BENCHMARK_ADVANCED("std::any::construction (single value move)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<std::any>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(Move(value));
				});
			};
		#endif
	}

	GIVEN("Container constructed by disowned value") {
		T pack {Disown(element)};

		THEN("Properties should match") {
			REQUIRE(pack.GetType() != nullptr);
			if constexpr (CT::Flat<E>) {
				if constexpr (CT::Sparse<E>)
					REQUIRE(&pack.template As<DenseE>() == sparseValue);
				REQUIRE(pack.template Is<DenseE>());
				REQUIRE(pack.template Is<DenseE*>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE_FALSE(pack.IsStatic());
				REQUIRE(pack.HasAuthority());
				REQUIRE(pack.IsConstant() == CT::Constant<E>);
				REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
			}
			else if constexpr (CT::Same<E, T>) {
				REQUIRE(pack.GetRaw() == element.GetRaw());
				REQUIRE(pack.Is(element.GetType()));
				REQUIRE(pack == element);
				REQUIRE(pack.GetUses() == 0);
				REQUIRE(pack.IsStatic());
				REQUIRE_FALSE(pack.HasAuthority());
				REQUIRE(pack.IsDeep() == element.IsDeep());
				REQUIRE(pack.IsConstant() == element.IsConstant());
			}
			else {
				REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
				REQUIRE(pack.template Is<typename T::Type>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE(pack.template As<DenseE>().IsStatic());
				REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
				REQUIRE_FALSE(pack.template As<DenseE>().HasAuthority());
				REQUIRE(pack.template As<DenseE>().GetUses() == 0);
				REQUIRE(pack.template As<DenseE>() == element);
				REQUIRE(pack != element);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE_FALSE(pack.IsStatic());
				REQUIRE_FALSE(pack.IsConstant());
				REQUIRE(pack.HasAuthority());
				REQUIRE(pack.IsDeep());
			}

			if constexpr (CT::Sparse<E>) {
				REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
				REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
			}

			REQUIRE_THROWS(pack.template As<float>() == 0.0f);
			REQUIRE_THROWS(pack.template As<float*>() == nullptr);
			REQUIRE_FALSE(pack.IsEmpty());
			REQUIRE(pack.IsDense() == CT::Dense<E>);
			REQUIRE(pack.IsSparse() == CT::Sparse<E>);
		}

		#ifdef LANGULUS_STD_BENCHMARK
			BENCHMARK_ADVANCED("construction (single disowned value)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<T>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(Disowned(value));
				});
			};

			BENCHMARK_ADVANCED("std::vector::construction (single value copy)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<StdT>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(1, value);
				});
			};

			BENCHMARK_ADVANCED("std::any::construction (single value copy)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<std::any>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(value);
				});
			};
		#endif
	}
	 
	GIVEN("Container constructed by abandoned value") {
		E movable = element;
		T pack {Abandon(movable)};

		THEN("Properties should match") {
			if constexpr (CT::Deep<E> && CT::Dense<E>) {
				REQUIRE_FALSE(movable.IsEmpty());
				REQUIRE(movable.IsAllocated());
				REQUIRE(movable.IsStatic());
			}
			REQUIRE(pack.GetType() != nullptr);
			REQUIRE(pack.GetRaw() != nullptr);
			if constexpr (CT::Flat<E>) {
				if constexpr (CT::Sparse<E>)
					REQUIRE(&pack.template As<DenseE>() == sparseValue);
				REQUIRE(pack.template Is<DenseE>());
				REQUIRE(pack.template Is<DenseE*>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(pack.HasAuthority());
				REQUIRE_FALSE(pack.IsStatic());
				REQUIRE(pack.IsConstant() == CT::Constant<E>);
				REQUIRE(pack.IsDeep() == CT::Deep<Decay<E>>);
			}
			else if constexpr (CT::Same<E, T>) {
				REQUIRE(pack.GetRaw() == element.GetRaw());
				REQUIRE(pack.Is(element.GetType()));
				REQUIRE(pack == element);
				REQUIRE(pack.GetUses() == 2);
				REQUIRE(pack.IsDeep() == element.IsDeep());
				REQUIRE(pack.IsConstant() == element.IsConstant());
				REQUIRE(pack.IsStatic() == element.IsStatic());
				REQUIRE(pack.HasAuthority() == element.HasAuthority());
			}
			else {
				REQUIRE(pack.template As<DenseE>().GetRaw() == sparseValue->GetRaw());
				REQUIRE(pack.template Is<typename T::Type>());
				REQUIRE(pack.template As<DenseE>() == denseValue);
				REQUIRE(*pack.template As<DenseE*>() == denseValue);
				REQUIRE_FALSE(pack.template As<DenseE>().IsStatic());
				REQUIRE_FALSE(pack.template As<DenseE>().IsConstant());
				REQUIRE(pack.template As<DenseE>().HasAuthority());
				REQUIRE(pack.template As<DenseE>().GetUses() == 2);
				REQUIRE(pack.template As<DenseE>() == element);
				REQUIRE(pack != element);
				REQUIRE(pack.GetUses() == 1);
				REQUIRE(pack.IsDeep());
				REQUIRE_FALSE(pack.IsStatic());
				REQUIRE_FALSE(pack.IsConstant());
				REQUIRE(pack.HasAuthority());
			}

			if constexpr (CT::Sparse<E>) {
				REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
				REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
			}

			REQUIRE_THROWS(pack.template As<float>() == 0.0f);
			REQUIRE_THROWS(pack.template As<float*>() == nullptr);
			REQUIRE_FALSE(pack.IsEmpty());
			REQUIRE(pack.IsDense() == CT::Dense<E>);
			REQUIRE(pack.IsSparse() == CT::Sparse<E>);
		}

		#ifdef LANGULUS_STD_BENCHMARK
			BENCHMARK_ADVANCED("construction (single abandoned value)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<T>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(Abandon(value));
				});
			};

			BENCHMARK_ADVANCED("std::vector::construction (single value move)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<StdT>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(1, Move(value));
				});
			};

			BENCHMARK_ADVANCED("std::any::construction (single value move)") (timer meter) {
				#include "CollectGarbage.inl"
				some<uninitialized<std::any>> storage(meter.runs());
				meter.measure([&](int i) {
					return storage[i].construct(Move(value));
				});
			};
		#endif
	}
	
	GIVEN("Container with some POD items") {
		// Arrays are dynamic to avoid constexprification
		int* darray1 = nullptr;
		darray1 = new int[5] {1, 2, 3, 4, 5};
		int* darray2 = nullptr;
		darray2 = new int[5] {6, 7, 8, 9, 10};

		TAny<int> pack;
		pack << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
		auto memory = pack.GetRaw();

		REQUIRE(pack.GetCount() == 5);
		REQUIRE(pack.GetReserved() >= 5);
		REQUIRE(pack.template Is<int>());
		REQUIRE(pack.GetRaw());
		REQUIRE(pack[0] == 1);
		REQUIRE(pack[1] == 2);
		REQUIRE(pack[2] == 3);
		REQUIRE(pack[3] == 4);
		REQUIRE(pack[4] == 5);
		REQUIRE_FALSE(pack.IsConstant());

		WHEN("Shallow-copy more of the same stuff") {
			pack << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack[0] == 1);
				REQUIRE(pack[1] == 2);
				REQUIRE(pack[2] == 3);
				REQUIRE(pack[3] == 4);
				REQUIRE(pack[4] == 5);
				REQUIRE(pack[5] == 6);
				REQUIRE(pack[6] == 7);
				REQUIRE(pack[7] == 8);
				REQUIRE(pack[8] == 9);
				REQUIRE(pack[9] == 10);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack.Is<int>());
			}
			
			#ifdef LANGULUS_STD_BENCHMARK // Last result: 1:2 performance
				BENCHMARK_ADVANCED("Anyness::TAny::operator << (5 consecutive trivial copies)") (Catch::Benchmark::Chronometer meter) {
					std::vector<TAny<int>> storage(meter.runs());

					meter.measure([&](int i) {
						return storage[i] << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
					});
				};

				BENCHMARK_ADVANCED("std::vector::push_back(5 consecutive trivial copies)") (Catch::Benchmark::Chronometer meter) {
					std::vector<std::vector<int>> storage(meter.runs());

					meter.measure([&](int i) {
						storage[i].push_back(darray2[0]);
						storage[i].push_back(darray2[1]);
						storage[i].push_back(darray2[2]);
						storage[i].push_back(darray2[3]);
						return storage[i].push_back(darray2[4]);
					});
				};
			#endif
		}

		WHEN("Move more of the same stuff") {
			pack << Move(darray2[0]) << Move(darray2[1]) << Move(darray2[2]) << Move(darray2[3]) << Move(darray2[4]);

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack[0] == 1);
				REQUIRE(pack[1] == 2);
				REQUIRE(pack[2] == 3);
				REQUIRE(pack[3] == 4);
				REQUIRE(pack[4] == 5);
				REQUIRE(pack[5] == 6);
				REQUIRE(pack[6] == 7);
				REQUIRE(pack[7] == 8);
				REQUIRE(pack[8] == 9);
				REQUIRE(pack[9] == 10);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack.Is<int>());
			}
			
			#ifdef LANGULUS_STD_BENCHMARK // Last result: 1:2 performance
				BENCHMARK_ADVANCED("Anyness::TAny::operator << (5 consecutive trivial moves)") (Catch::Benchmark::Chronometer meter) {
					std::vector<TAny<int>> storage(meter.runs());

					meter.measure([&](int i) {
						return storage[i] << Move(darray2[0]) << Move(darray2[1]) << Move(darray2[2]) << Move(darray2[3]) << Move(darray2[4]);
					});
				};

				BENCHMARK_ADVANCED("std::vector::emplace_back(5 consecutive trivial moves)") (Catch::Benchmark::Chronometer meter) {
					std::vector<std::vector<int>> storage(meter.runs());

					meter.measure([&](int i) {
						storage[i].emplace_back(Move(darray2[0]));
						storage[i].emplace_back(Move(darray2[1]));
						storage[i].emplace_back(Move(darray2[2]));
						storage[i].emplace_back(Move(darray2[3]));
						return storage[i].emplace_back(Move(darray2[4]));
					});
				};
			#endif
		}

		WHEN("Insert more trivial items at a specific place by shallow-copy") {
			int* i666 = new int {666};
			int& i666d = *i666;
			pack.InsertAt(i666, i666 + 1, 3);

			THEN("The size changes, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
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

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 1:2 performance
				BENCHMARK_ADVANCED("Anyness::TAny::Insert(single copy in middle)") (Catch::Benchmark::Chronometer meter) {
					std::vector<TAny<int>> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].Insert(i666, 1, 3);
					});
				};

				BENCHMARK_ADVANCED("std::vector::insert(single copy in middle)") (Catch::Benchmark::Chronometer meter) {
					std::vector<std::vector<int>> storage(meter.runs());
					for (auto&& o : storage)
						o = { darray1[0], darray1[1], darray1[2], darray1[3], darray1[4] };

					meter.measure([&](int i) {
						return storage[i].insert(storage[i].begin() + 3, i666d);
					});
				};
			#endif
		}

		WHEN("Insert more trivial items at a specific place by move") {
			int* i666 = new int {666};
			int& i666d = *i666;
			pack.InsertAt(Move(*i666), 3);

			THEN("The size changes, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
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

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 1:2 performance
				BENCHMARK_ADVANCED("Anyness::TAny::Emplace(single move in middle)") (Catch::Benchmark::Chronometer meter) {
					std::vector<TAny<int>> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].Insert(Move(i666d), 3);
					});
				};

				BENCHMARK_ADVANCED("std::vector::insert(single move in middle)") (Catch::Benchmark::Chronometer meter) {
					std::vector<std::vector<int>> storage(meter.runs());
					for (auto&& o : storage)
						o = { darray1[0], darray1[1], darray1[2], darray1[3], darray1[4] };

					meter.measure([&](int i) {
						return storage[i].insert(storage[i].begin() + 3, Move(i666d));
					});
				};
			#endif
		}

		WHEN("The size is reduced by finding and removing elements, but reserved memory should remain the same on shrinking") {
			const auto removed2 = pack.RemoveValue(2);
			const auto removed4 = pack.RemoveValue(4);

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

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 2:1 performance - needs more optimizations in Index handling
				BENCHMARK_ADVANCED("Anyness::TAny::Remove(single element by value)") (Catch::Benchmark::Chronometer meter) {
					std::vector<TAny<int>> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].RemoveValue(2);
					});
				};

				BENCHMARK_ADVANCED("Anyness::vector::erase-remove(single element by value)") (Catch::Benchmark::Chronometer meter) {
					std::vector<std::vector<int>> storage(meter.runs());
					for (auto&& o : storage)
						o = { darray1[0], darray1[1], darray1[2], darray1[3], darray1[4] };

					meter.measure([&](int i) {
						// Erase-remove idiom											
						return storage[i].erase(std::remove(storage[i].begin(), storage[i].end(), 2), storage[i].end());
					});
				};
			#endif
		}

		WHEN("Removing non-available elements") {
			const auto removed9 = pack.RemoveValue(9);
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

	GIVEN("Two containers with some POD items") {
		#include "CollectGarbage.inl"

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
				REQUIRE(pack1.GetUses() == 2);
				REQUIRE(pack2.GetUses() == 2);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}

		WHEN("Shallow copy pack1 in pack2 and then reset pack1") {
			pack2 = pack1;
			pack1.Reset();
			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetUses() == 1);
				REQUIRE_FALSE(pack1.GetRaw());
				REQUIRE(pack1.GetReserved() == 0);
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}

		WHEN("Deep copy pack1 in pack2") {
			pack2 = pack1.Clone();
			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetUses() == 1);
				REQUIRE(pack2.GetUses() == 1);
				REQUIRE(static_cast<Block&>(pack1) == static_cast<Block&>(pack2));
				REQUIRE(static_cast<Block&>(pack2) == memory1);
				REQUIRE(static_cast<Block&>(pack2) != memory2);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}

		WHEN("Deep copy pack1 in pack2, then reset pack1") {
			pack2 = pack1.Clone();
			const auto memory3 = static_cast<Block>(pack2);
			pack1.Reset();
			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetUses() == 1);
				REQUIRE(memory3.GetUses() == 1);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE_FALSE(Allocator::Find(memory1.GetType(), memory1.GetRaw()));
					REQUIRE_FALSE(Allocator::Find(memory2.GetType(), memory2.GetRaw()));
				#endif
			}
		}
	}

	if constexpr (CT::Sparse<E>)
		delete element;
}
