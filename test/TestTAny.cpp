///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"
#include <catch2/catch.hpp>

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md			
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
	const Text serialized {ex};
	return ::std::string {Token {serialized}};
}

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

template<class T, class ALT_T>
T CreateElement(const ALT_T& e) {
	T element;
	if constexpr (CT::Sparse<T>) {
		if constexpr (!CT::Same<T, Block>)
			element = new Decay<T> {e};
		else {
			element = new Block {};
			element->Insert(e);
		}
	}
	else if constexpr (CT::Same<T, ALT_T>)
		element = e;
	else {
		if constexpr (!CT::Same<T, Block>)
			element = Decay<T> {e};
		else {
			element = Block {};
			element.Insert(e);
		}
	}

	return element;
}

/// The main test for Any/TAny containers, with all kinds of items, from		
/// sparse to dense, from trivial to complex, from flat to deep					
TEMPLATE_TEST_CASE("Any/TAny", "[any]", 
	(TypePair<TAny<Any>, Any>),
	(TypePair<TAny<int>, int>),
	(TypePair<TAny<Trait>, Trait>),
	(TypePair<TAny<Traits::Count>, Traits::Count>),
	(TypePair<TAny<Text>, Text>),
	//(TypePair<TAny<Block>, Block>),
	(TypePair<TAny<int*>, int*>),
	(TypePair<TAny<Trait*>, Trait*>),
	(TypePair<TAny<Traits::Count*>, Traits::Count*>),
	(TypePair<TAny<Any*>, Any*>),
	(TypePair<TAny<Text*>, Text*>),
	//(TypePair<TAny<Block*>, Block*>),
	(TypePair<Any, int>),
	(TypePair<Any, Trait>),
	(TypePair<Any, Traits::Count>),
	(TypePair<Any, Any>),
	(TypePair<Any, Text>),
	//(TypePair<Any, Block>),
	(TypePair<Any, int*>),
	(TypePair<Any, Trait*>),
	(TypePair<Any, Traits::Count*>),
	(TypePair<Any, Any*>),
	(TypePair<Any, Text*>),
	//(TypePair<Any, Block*>),
	(TypePair<Traits::Name, Text>)
) {
	using T = typename TestType::Container;
	using E = typename TestType::Element;
	//using StdT = std::vector<E>;
	using DenseE = Decay<E>;
		
	E element = CreateElement<E>(555);
	const DenseE& denseValue {DenseCast(element)};
	const DenseE* const sparseValue = {SparseCast(element)};

	const E darray1[5] {
		CreateElement<E>(1),
		CreateElement<E>(2),
		CreateElement<E>(3),
		CreateElement<E>(4),
		CreateElement<E>(5)
	};
	const E darray2[5] {
		CreateElement<E>(6),
		CreateElement<E>(7),
		CreateElement<E>(8),
		CreateElement<E>(9),
		CreateElement<E>(10)
	};

	GIVEN("Default constructed container") {
		T pack;

		THEN("Properties should match") {
			REQUIRE(pack.GetCount() == 0);
			REQUIRE_FALSE(pack.IsConstant());
			REQUIRE_FALSE(pack.IsCompressed());
			REQUIRE_FALSE(pack.IsAbstract());
			REQUIRE_FALSE(pack.IsAllocated());
			REQUIRE(pack.IsDeep() == (CT::Typed<T> && CT::Deep<Decay<E>>));
			REQUIRE_FALSE(pack.IsEncrypted());
			REQUIRE_FALSE(pack.IsFuture());
			REQUIRE_FALSE(pack.IsPast());
			REQUIRE_FALSE(pack.IsMissing());
			REQUIRE_FALSE(pack.IsStatic());
			REQUIRE_FALSE(pack.IsValid());
			REQUIRE(pack.IsNow());
			REQUIRE(pack.IsInvalid());
			REQUIRE(pack.GetRaw() == nullptr);
			REQUIRE(pack.IsEmpty());

			if constexpr (CT::Typed<T>) {
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
				REQUIRE(pack.IsUntyped());
				REQUIRE(pack.GetType() == nullptr);
				REQUIRE(pack.IsDense());
				REQUIRE_FALSE(pack.IsSparse());
				REQUIRE(pack.GetState() == DataState::Default);
			}

			REQUIRE(pack.IsTypeConstrained() == CT::Typed<T>);
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
			if constexpr (CT::Deep<E> && CT::Typed<T>)
				REQUIRE_THROWS(pack = element);
			else
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
					REQUIRE_FALSE(pack.IsEmpty());
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
					REQUIRE_FALSE(pack.IsEmpty());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(*pack.GetRawSparse() == sparseValue);
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
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
			if constexpr (CT::Deep<E> && CT::Typed<T>)
				REQUIRE_THROWS(pack = Move(movable));
			else
				pack = Move(movable);

			THEN("Properties should match") {
				if constexpr (!CT::Deep<E> && CT::Block<E>) {
					REQUIRE(movable.IsEmpty());
					REQUIRE_FALSE(movable.IsAllocated());
					REQUIRE(movable != element);
				}

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
					REQUIRE_FALSE(pack.IsEmpty());
					REQUIRE(pack.GetType() != nullptr);
					REQUIRE(pack.GetRaw() != nullptr);
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
					REQUIRE_FALSE(pack.IsEmpty());
					REQUIRE(pack.GetType() != nullptr);
					REQUIRE(pack.GetRaw() != nullptr);
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
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
			if constexpr (CT::Deep<E> && CT::Typed<T>)
				REQUIRE_THROWS(pack = Disown(element));
			else
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
					REQUIRE_FALSE(pack.IsEmpty());
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
					REQUIRE_FALSE(pack.IsEmpty());
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
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
			if constexpr (CT::Deep<E> && CT::Typed<T>)
				REQUIRE_THROWS(pack = Abandon(movable));
			else
				pack = Abandon(movable);

			THEN("Properties should match") {
				if constexpr (!CT::Deep<E> && CT::Block<E>) {
					REQUIRE_FALSE(movable.IsEmpty());
					REQUIRE(movable.IsAllocated());
					REQUIRE(movable.IsStatic());
				}

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
					REQUIRE_FALSE(pack.IsEmpty());
					REQUIRE(pack.GetType() != nullptr);
					REQUIRE(pack.GetRaw() != nullptr);
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
					REQUIRE_FALSE(pack.IsEmpty());
					REQUIRE(pack.GetType() != nullptr);
					REQUIRE(pack.GetRaw() != nullptr);
				}

				if constexpr (CT::Sparse<E>) {
					REQUIRE(asbytes(pack.GetRawSparse()->mPointer) == asbytes(sparseValue));
					REQUIRE(pack.GetRawSparse()->mEntry == nullptr);
				}

				REQUIRE_THROWS(pack.template As<float>() == 0.0f);
				REQUIRE_THROWS(pack.template As<float*>() == nullptr);
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
				if constexpr (CT::Typed<T>) {
					REQUIRE(pack.GetType()->template Is<E>());
					REQUIRE(pack.GetType()->template Is<DenseE>());
					REQUIRE(pack.IsDense() == CT::Dense<E>);
					REQUIRE(pack.IsSparse() == CT::Sparse<E>);
				}
				else {
					REQUIRE_FALSE(pack.GetType());
					REQUIRE(pack.IsDense());
					REQUIRE_FALSE(pack.IsSparse());
				}

				REQUIRE(pack.IsTypeConstrained() == CT::Typed<T>);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.IsEmpty());
				REQUIRE(pack.GetUses() == 0);
				REQUIRE(pack.IsDeep() == (CT::Typed<T> && CT::Deep<Decay<E>>));
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

	}

	GIVEN("Container constructed by same container copy") {
		if constexpr (CT::Deep<E> && CT::Typed<T>)
			REQUIRE_THROWS(T {element});
		else {
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
	}

	GIVEN("Container constructed by value copy") {
		if constexpr (CT::Deep<E> && CT::Typed<T>)
			REQUIRE_THROWS(T {element});
		else {
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
				pack = T {};

				THEN("Various traits change") {
					if constexpr (CT::Typed<T>) {
						REQUIRE(pack.GetType()->template Is<E>());
					}
					REQUIRE(pack.IsTypeConstrained() == CT::Typed<T>);
					REQUIRE(pack.GetRaw() == nullptr);
					REQUIRE_FALSE(pack.IsAllocated());
					REQUIRE(pack.IsEmpty());
					REQUIRE(pack.GetUses() == 0);
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
					if constexpr (CT::Typed<T>) {
						REQUIRE(pack.GetType()->template Is<E>());
						REQUIRE(pack.GetType()->template Is<DenseE>());
					}
					else {
						REQUIRE(pack.GetType());
					}

					REQUIRE(pack.IsTypeConstrained() == CT::Typed<T>);
					REQUIRE(pack.IsDense() == CT::Dense<E>);
					REQUIRE(pack.IsSparse() == CT::Sparse<E>);
					REQUIRE(pack.GetRaw() != nullptr);
					REQUIRE_FALSE(pack.IsEmpty());
					REQUIRE(pack.GetUses() == (CT::Deep<E> && CT::Same<T, E> ? 2 : 1));
					REQUIRE(pack.IsDeep() == (CT::Deep<Decay<E>> && (CT::Sparse<E> || !CT::Same<T, E>)));
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
	}

	GIVEN("Container constructed by value move") {
		if constexpr (CT::Deep<E> && CT::Typed<T>) {
			E movable = element;
			REQUIRE_THROWS(T {Move(movable)});
		}
		else {
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
					if constexpr (CT::Typed<T>)
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
	}

	GIVEN("Container constructed by disowned value") {
		if constexpr (CT::Deep<E> && CT::Typed<T>)
			REQUIRE_THROWS(T {Disown(element)});
		else {
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
					if constexpr (CT::Typed<T>)
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
	}
	 
	GIVEN("Container constructed by abandoned value") {
		if constexpr (CT::Deep<E> && CT::Typed<T>) {
			E movable = element;
			REQUIRE_THROWS(T {Abandon(movable)});
		}
		else {
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
					if constexpr (CT::Typed<T>)
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
	}
	
	GIVEN("Container with some items") {
		const T pack {};
		const_cast<T&>(pack) << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

		WHEN("Given a preinitialized container with 5 elements") {
			THEN("These properties should be correct") {
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.template Is<E>());
				REQUIRE(pack.GetRaw());
				for (unsigned i = 0; i < pack.GetCount(); ++i)
					REQUIRE(pack[i] == darray1[i]);
				REQUIRE_FALSE(pack.IsConstant());
			}
		}

		WHEN("Shallow-copy more of the same stuff to the back (<<)") {
			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack) << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack.template Is<E>());
				for (unsigned i = 0; i < 5; ++i)
					REQUIRE(pack[i] == darray1[i]);
				for (unsigned i = 5; i < pack.GetCount(); ++i)
					REQUIRE(pack[i] == darray2[i-5]);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					if constexpr (CT::Same<E, int>) {
						REQUIRE(pack.GetRaw() == memory);
					}
				#endif
			}
			
			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TAny::operator << (5 consecutive trivial copies)") (timer meter) {
					some<T> storage(meter.runs());

					meter.measure([&](int i) {
						return storage[i] << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
					});
				};

				BENCHMARK_ADVANCED("std::vector::push_back(5 consecutive trivial copies)") (timer meter) {
					some<StdT> storage(meter.runs());

					meter.measure([&](int i) {
						auto& s = storage[i];
						s.push_back(darray2[0]);
						s.push_back(darray2[1]);
						s.push_back(darray2[2]);
						s.push_back(darray2[3]);
						return s.push_back(darray2[4]);
					});
				};
			#endif
		}

		WHEN("Shallow-copy more of the same stuff to the front (>>)") {
			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack) >> darray2[0] >> darray2[1] >> darray2[2] >> darray2[3] >> darray2[4];

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack.template Is<E>());
				for (unsigned i = 5; i > 0; --i)
					REQUIRE(pack[5 - i] == darray2[i - 1]);
				for (unsigned i = 5; i < pack.GetCount(); ++i)
					REQUIRE(pack[i] == darray1[i-5]);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					if constexpr (CT::Same<E, int>) {
						REQUIRE(pack.GetRaw() == memory);
					}
				#endif
			}
			
			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TAny::operator >> (5 consecutive trivial copies)") (timer meter) {
					some<T> storage(meter.runs());

					meter.measure([&](int i) {
						return storage[i] >> darray2[0] >> darray2[1] >> darray2[2] >> darray2[3] >> darray2[4];
					});
				};

				BENCHMARK_ADVANCED("std::vector::push_front(5 consecutive trivial copies)") (timer meter) {
					some<StdT> storage(meter.runs());

					meter.measure([&](int i) {
						auto& s = storage[i];
						s.push_front(darray2[0]);
						s.push_front(darray2[1]);
						s.push_front(darray2[2]);
						s.push_front(darray2[3]);
						return s.push_front(darray2[4]);
					});
				};
			#endif
		}

		WHEN("Move more of the same stuff to the back (<<)") {
			E darray3[5] {
				CreateElement<E>(6),
				CreateElement<E>(7),
				CreateElement<E>(8),
				CreateElement<E>(9),
				CreateElement<E>(10)
			};

			const E darray3backup[5] {
				darray3[0],
				darray3[1],
				darray3[2],
				darray3[3],
				darray3[4],
			};

			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack) << Move(darray3[0]) << Move(darray3[1]) << Move(darray3[2]) << Move(darray3[3]) << Move(darray3[4]);

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack.template Is<E>());

				for (unsigned i = 0; i < 5; ++i)
					REQUIRE(pack[i] == darray1[i]);

				for (unsigned i = 5; i < pack.GetCount(); ++i)
					REQUIRE(pack[i] == darray3backup[i - 5]);

				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
			}
			
			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TAny::operator << (5 consecutive trivial moves)") (timer meter) {
					some<T> storage(meter.runs());

					meter.measure([&](int i) {
						return storage[i] << Move(darray2[0]) << Move(darray2[1]) << Move(darray2[2]) << Move(darray2[3]) << Move(darray2[4]);
					});
				};

				BENCHMARK_ADVANCED("std::vector::emplace_back(5 consecutive trivial moves)") (timer meter) {
					some<StdT> storage(meter.runs());

					meter.measure([&](int i) {
						auto& s = storage[i];
						s.emplace_back(Move(darray2[0]));
						s.emplace_back(Move(darray2[1]));
						s.emplace_back(Move(darray2[2]));
						s.emplace_back(Move(darray2[3]));
						return s.emplace_back(Move(darray2[4]));
					});
				};
			#endif
		}

		WHEN("Move more of the same stuff to the front (>>)") {
			E darray3[5] {
				CreateElement<E>(6),
				CreateElement<E>(7),
				CreateElement<E>(8),
				CreateElement<E>(9),
				CreateElement<E>(10)
			};

			const E darray3backup[5] {
				darray3[0],
				darray3[1],
				darray3[2],
				darray3[3],
				darray3[4],
			};

			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack) >> Move(darray3[0]) >> Move(darray3[1]) >> Move(darray3[2]) >> Move(darray3[3]) >> Move(darray3[4]);

			THEN("The size and capacity change, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 10);
				REQUIRE(pack.GetReserved() >= 10);
				REQUIRE(pack.template Is<E>());

				for (unsigned i = 5; i > 0; --i)
					REQUIRE(pack[5 - i] == darray3backup[i - 1]);

				for (unsigned i = 5; i < pack.GetCount(); ++i)
					REQUIRE(pack[i] == darray1[i - 5]);

				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
			}
			
			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TAny::operator >> (5 consecutive trivial moves)") (timer meter) {
					some<T> storage(meter.runs());

					meter.measure([&](int i) {
						return storage[i] >> Move(darray2[0]) >> Move(darray2[1]) >> Move(darray2[2]) >> Move(darray2[3]) >> Move(darray2[4]);
					});
				};

				BENCHMARK_ADVANCED("std::vector::emplace_front(5 consecutive trivial moves)") (timer meter) {
					some<StdT> storage(meter.runs());

					meter.measure([&](int i) {
						auto& s = storage[i];
						s.emplace_front(Move(darray2[0]));
						s.emplace_front(Move(darray2[1]));
						s.emplace_front(Move(darray2[2]));
						s.emplace_front(Move(darray2[3]));
						return s.emplace_front(Move(darray2[4]));
					});
				};
			#endif
		}

		WHEN("Insert single item at a specific place by shallow-copy") {
			const auto i666 = CreateElement<E>(666);

			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack).InsertAt(i666, 3);

			THEN("The size changes, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 6);
				REQUIRE(pack.GetReserved() >= 6);
				REQUIRE(pack.template Is<E>());
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack[0] == darray1[0]);
				REQUIRE(pack[1] == darray1[1]);
				REQUIRE(pack[2] == darray1[2]);
				REQUIRE(pack[3] == i666);
				REQUIRE(pack[4] == darray1[3]);
				REQUIRE(pack[5] == darray1[4]);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TAny::Insert(single copy in middle)") (timer meter) {
					some<T> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].InsertAt(&i666, &i666 + 1, 3);
					});
				};

				BENCHMARK_ADVANCED("std::vector::insert(single copy in middle)") (timer meter) {
					some<StdT> storage(meter.runs());
					for (auto&& o : storage)
						o = { darray1[0], darray1[1], darray1[2], darray1[3], darray1[4] };

					meter.measure([&](int i) {
						return storage[i].insert(storage[i].begin() + 3, i666d);
					});
				};
			#endif
		}

		WHEN("Insert single item at a specific place by move") {
			auto i666 = CreateElement<E>(666);
			const auto i666backup = i666;

			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack).InsertAt(Move(i666), 3);

			THEN("The size changes, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 6);
				REQUIRE(pack.GetReserved() >= 6);
				REQUIRE(pack.template Is<E>());
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack[0] == darray1[0]);
				REQUIRE(pack[1] == darray1[1]);
				REQUIRE(pack[2] == darray1[2]);
				REQUIRE(pack[3] == i666backup);
				REQUIRE(pack[4] == darray1[3]);
				REQUIRE(pack[5] == darray1[4]);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TAny::Emplace(single move in middle)") (timer meter) {
					some<T> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].InsertAt(Move(i666d), 3);
					});
				};

				BENCHMARK_ADVANCED("std::vector::insert(single move in middle)") (timer meter) {
					some<StdT> storage(meter.runs());
					for (auto&& o : storage)
						o = { darray1[0], darray1[1], darray1[2], darray1[3], darray1[4] };

					meter.measure([&](int i) {
						return storage[i].insert(storage[i].begin() + 3, Move(i666d));
					});
				};
			#endif
		}

		WHEN("Emplace item at a specific place") {
			auto i666 = CreateElement<E>(666);
			const auto i666backup = i666;

			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack).EmplaceAt(3, Move(i666));

			THEN("The size changes, type will never change, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 6);
				REQUIRE(pack.GetReserved() >= 6);
				REQUIRE(pack.template Is<E>());
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					REQUIRE(pack.GetRaw() == memory);
				#endif
				REQUIRE(pack[0] == darray1[0]);
				REQUIRE(pack[1] == darray1[1]);
				REQUIRE(pack[2] == darray1[2]);
				REQUIRE(pack[3] == i666backup);
				REQUIRE(pack[4] == darray1[3]);
				REQUIRE(pack[5] == darray1[4]);
			}

			#ifdef LANGULUS_STD_BENCHMARK
				BENCHMARK_ADVANCED("Anyness::TAny::Emplace(single move in middle)") (timer meter) {
					some<T> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].EmplaceAt(3, Move(i666d));
					});
				};

				BENCHMARK_ADVANCED("std::vector::insert(single move in middle)") (timer meter) {
					some<StdT> storage(meter.runs());
					for (auto&& o : storage)
						o = { darray1[0], darray1[1], darray1[2], darray1[3], darray1[4] };

					meter.measure([&](int i) {
						return storage[i].insert(storage[i].begin() + 3, Move(i666d));
					});
				};
			#endif
		}

		WHEN("The size is reduced by finding and removing elements, but reserved memory should remain the same on shrinking") {
			const auto memory = pack.GetRaw();
			const auto removed2 = const_cast<T&>(pack).RemoveValue(CreateElement<E>(2));
			const auto removed4 = const_cast<T&>(pack).RemoveValue(CreateElement<E>(4));

			THEN("The size changes but not capacity") {
				REQUIRE(removed2 == 1);
				REQUIRE(removed4 == 1);
				REQUIRE(pack[0] == darray1[0]);
				REQUIRE(pack[1] == darray1[2]);
				REQUIRE(pack[2] == darray1[4]);
				REQUIRE_THROWS(pack[3] == CreateElement<E>(666));
				REQUIRE(pack.GetCount() == 3);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
			}

			#ifdef LANGULUS_STD_BENCHMARK // Last result: 2:1 performance - needs more optimizations in Index handling
				BENCHMARK_ADVANCED("Anyness::TAny::Remove(single element by value)") (timer meter) {
					some<T> storage(meter.runs());
					for (auto&& o : storage)
						o << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];

					meter.measure([&](int i) {
						return storage[i].RemoveValue(2);
					});
				};

				BENCHMARK_ADVANCED("Anyness::vector::erase-remove(single element by value)") (timer meter) {
					some<StdT> storage(meter.runs());
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
			const auto memory = pack.GetRaw();
			const auto removed9 = const_cast<T&>(pack).RemoveValue(9);

			THEN("The size changes but not capacity") {
				REQUIRE(removed9 == 0);
				REQUIRE(pack[0] == darray1[0]);
				REQUIRE(pack[1] == darray1[1]);
				REQUIRE(pack[2] == darray1[2]);
				REQUIRE(pack[3] == darray1[3]);
				REQUIRE(pack[4] == darray1[4]);
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
			}
		}

		WHEN("More capacity is reserved") {
			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				const auto memory = pack.GetRaw();
			#endif

			const_cast<T&>(pack).Allocate(20);

			THEN("The capacity changes but not the size, memory shouldn't move if MANAGED_MEMORY feature is enabled") {
				REQUIRE(pack.GetCount() == 5);
				REQUIRE(pack.GetReserved() >= 20);
				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					if constexpr (CT::POD<E>) {
						// Test works only for POD types, because containers shift entries around
						REQUIRE(pack.GetRaw() == memory);
					}
				#endif
			}
		}

		WHEN("Less capacity is reserved") {
			const auto memory = pack.GetRaw();
			const_cast<T&>(pack).Allocate(2);

			THEN("Capacity remains unchanged, but count is trimmed; memory shouldn't move") {
				REQUIRE(pack.GetCount() == 2);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
			}
		}

		WHEN("Pack is cleared") {
			const auto memory = pack.GetRaw();
			const_cast<T&>(pack).Clear();

			THEN("Size goes to zero, capacity and type are unchanged") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() >= 5);
				REQUIRE(pack.GetRaw() == memory);
				REQUIRE(pack.template Is<E>());
			}
		}

		WHEN("Pack is reset") {
			const_cast<T&>(pack).Reset();

			THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
				REQUIRE(pack.GetCount() == 0);
				REQUIRE(pack.GetReserved() == 0);
				REQUIRE(pack.GetRaw() == nullptr);
				REQUIRE(pack.template Is<E>() == CT::Typed<T>);
			}
		}

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			if constexpr (CT::Same<E, int>) {
				// Works only if E doesn't move entries around
				WHEN("Pack is reset, then immediately allocated again") {
					#include "CollectGarbage.inl"
					const auto memory = pack.GetRaw();
					const_cast<T&>(pack).Reset();
					const_cast<T&>(pack) << CreateElement<E>(6) << CreateElement<E>(7) << CreateElement<E>(8) << CreateElement<E>(9) << CreateElement<E>(10);
					THEN("Block manager should reuse the memory, if MANAGED_MEMORY feature is enabled") {
						REQUIRE(pack.GetRaw() == memory);
					}
				}
			}
		#endif

		WHEN("Pack is shallow-copied") {
			const_cast<T&>(pack).MakeOr();
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
			const_cast<T&>(pack).MakeOr();
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
			T movable = pack;
			movable.MakeOr();
			const T moved = Move(movable);

			THEN("The new pack should keep the state and data") {
				REQUIRE(movable.GetRaw() == nullptr);
				REQUIRE(movable.GetCount() == 0);
				REQUIRE(movable.GetReserved() == 0);
				REQUIRE(movable.IsTypeConstrained() == CT::Typed<T>);
				REQUIRE(pack.GetRaw() == moved.GetRaw());
				REQUIRE(pack.GetCount() == moved.GetCount());
				REQUIRE(pack.GetReserved() == moved.GetReserved());
				REQUIRE(pack.GetState() + DataState::Or == moved.GetState());
				REQUIRE(pack.GetType() == moved.GetType());
			}
		}

		WHEN("Packs are compared") {
			T another_pack1;
			another_pack1 << CreateElement<E>(1) << CreateElement<E>(2) << CreateElement<E>(3) << CreateElement<E>(4) << CreateElement<E>(5);
			T another_pack2;
			another_pack2 << CreateElement<E>(2) << CreateElement<E>(2) << CreateElement<E>(3) << CreateElement<E>(4) << CreateElement<E>(5);
			T another_pack3;
			another_pack3 << CreateElement<E>(1) << CreateElement<E>(2) << CreateElement<E>(3) << CreateElement<E>(4) << CreateElement<E>(5) << CreateElement<E>(6);
			TAny<uint> another_pack4;
			another_pack4 << uint(1) << uint(2) << uint(3) << uint(4) << uint(5);
			Any another_pack5;
			another_pack5 << CreateElement<E>(1) << CreateElement<E>(2) << CreateElement<E>(3) << CreateElement<E>(4) << CreateElement<E>(5);

			THEN("The comparisons should be adequate") {
				REQUIRE(pack == another_pack1);
				REQUIRE(pack != another_pack2);
				REQUIRE(pack != another_pack3);
				REQUIRE(pack != another_pack4);
				REQUIRE(pack == another_pack5);
			}
		}

		WHEN("A forward value-based search is performed on exitent value") {
			const auto found = pack.Find(CreateElement<E>(3));

			THEN("The value's index should be correct") {
				REQUIRE(found);
				REQUIRE(found == 2);
			}
		}

		WHEN("A forward value-based search is performed on non-exitent value") {
			const auto found = pack.Find(CreateElement<E>(8));
			THEN("The function should return IndexNone") {
				REQUIRE(found == IndexNone);
				REQUIRE_FALSE(found);
			}
		}

		WHEN("A backward value-based search is performed on exitent value") {
			const auto found = pack.template Find<true>(CreateElement<E>(3));

			THEN("The new pack should keep the state and data") {
				REQUIRE(found);
				REQUIRE(found == 2);
			}
		}

		WHEN("A backward value-based search is performed on non-exitent value") {
			const auto found = pack.template Find<true>(CreateElement<E>(8));

			THEN("The function should return IndexNone") {
				REQUIRE(found == IndexNone);
				REQUIRE_FALSE(found);
			}
		}

		WHEN("ForEach flat dense element (immutable)") {
			int it = 0;
			pack.ForEach(
				[&](const int& i) {
					REQUIRE(i == it + 1);
					++it;
				},
				[&](const Trait& i) {
					REQUIRE(i == it + 1);
					++it;
				},
				[&](const Any& i) {
					const auto temp = CreateElement<DenseE>(it + 1);
					REQUIRE(i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}

		WHEN("ForEach flat dense element (mutable)") {
			int it = 0;
			const_cast<T&>(pack).ForEach(
				[&](int& i) {
					REQUIRE(i == it + 1);
					++it;
				},
				[&](const Trait& i) {
					REQUIRE(i == it + 1);
					++it;
				},
				[&](const Any& i) {
					const auto temp = CreateElement<DenseE>(it + 1);
					REQUIRE(i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}

		WHEN("ForEach flat sparse element (immutable)") {
			int it = 0;
			pack.ForEach(
				[&](const int* i) {
					REQUIRE(*i == it + 1);
					++it;
				},
				[&](const Trait* i) {
					REQUIRE(*i == it + 1);
					++it;
				},
				[&](const Any* i) {
					const auto temp = CreateElement<DenseE>(it + 1);
					REQUIRE(*i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}

		WHEN("ForEach flat sparse element (mutable)") {
			int it = 0;
			const_cast<T&>(pack).ForEach(
				[&](int* i) {
					REQUIRE(*i == it + 1);
					++it;
				},
				[&](const Trait* i) {
					REQUIRE(*i == it + 1);
					++it;
				},
				[&](const Any* i) {
					const auto temp = CreateElement<DenseE>(it + 1);
					REQUIRE(*i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}

		WHEN("ForEachRev flat dense element (immutable)") {
			int it = 0;
			pack.ForEachRev(
				[&](const int& i) {
					REQUIRE(i == 5 - it);
					++it;
				},
				[&](const Trait& i) {
					REQUIRE(i == 5 - it);
					++it;
				},
				[&](const Any& i) {
					const auto temp = CreateElement<DenseE>(5 - it);
					REQUIRE(i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}

		WHEN("ForEachRev flat dense element (mutable)") {
			int it = 0;
			const_cast<T&>(pack).ForEachRev(
				[&](int& i) {
					REQUIRE(i == 5 - it);
					++it;
				},
				[&](const Trait& i) {
					REQUIRE(i == 5 - it);
					++it;
				},
				[&](const Any& i) {
					const auto temp = CreateElement<DenseE>(5 - it);
					REQUIRE(i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}

		WHEN("ForEachRev flat sparse element (immutable)") {
			int it = 0;
			pack.ForEachRev(
				[&](const int* i) {
					REQUIRE(*i == 5 - it);
					++it;
				},
				[&](const Trait* i) {
					REQUIRE(*i == 5 - it);
					++it;
				},
				[&](const Any* i) {
					const auto temp = CreateElement<DenseE>(5 - it);
					REQUIRE(*i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}

		WHEN("ForEachRev flat sparse element (mutable)") {
			int it = 0;
			const_cast<T&>(pack).ForEachRev(
				[&](int* i) {
					REQUIRE(*i == 5 - it);
					++it;
				},
				[&](const Trait* i) {
					REQUIRE(*i == 5 - it);
					++it;
				},
				[&](const Any* i) {
					const auto temp = CreateElement<DenseE>(5 - it);
					REQUIRE(*i == static_cast<const Any&>(temp));
					++it;
				}
			);

			THEN("The number of iterated elements should be correct") {
				REQUIRE(static_cast<unsigned>(it) == pack.GetCount());
			}
		}
	}

	GIVEN("Two containers with some items") {
		#include "CollectGarbage.inl"

		const T pack1 {};
		const T pack2 {};
		const_cast<T&>(pack1) << darray1[0] << darray1[1] << darray1[2] << darray1[3] << darray1[4];
		const_cast<T&>(pack2) << darray2[0] << darray2[1] << darray2[2] << darray2[3] << darray2[4];
		const T memory1 = pack1;
		const T memory2 = pack2;

		WHEN("Copy-assign pack1 in pack2") {
			const_cast<T&>(pack2) = pack1;

			THEN("memory1 should be referenced, memory2 should be dereferenced") {
				REQUIRE(pack1.GetUses() == 3);
				REQUIRE(pack2.GetUses() == 3);
				REQUIRE(memory2.GetUses() == 1);
				REQUIRE(pack1 == pack2);
				REQUIRE(pack2 == memory1);
				REQUIRE(pack2 != memory2);
				for (int i = 0; i < 5; ++i)
					REQUIRE(pack2[i] == darray1[i]);
			}
		}

		WHEN("Move-assign pack1 in pack2") {
			auto movable = pack1;
			const_cast<T&>(pack2) = Move(movable);

			THEN("memory1 should be overwritten, memory2 should be released") {
				REQUIRE(pack1.GetUses() == 3);
				REQUIRE(pack2.GetUses() == 3);
				REQUIRE(memory2.GetUses() == 1);
				REQUIRE(pack1 == pack2);
				REQUIRE(movable != pack1);
				REQUIRE(movable == T {});
			}
		}

		WHEN("Disown-assign pack1 in pack2") {
			const_cast<T&>(pack2) = Disown(pack1);

			THEN("memory1 should be referenced, memory2 should be dereferenced") {
				REQUIRE(pack1.GetUses() == 2);
				REQUIRE(pack2.GetUses() == 0);
				REQUIRE(memory2.GetUses() == 1);
				REQUIRE(pack1 == pack2);
				REQUIRE(pack2 == memory1);
				REQUIRE(pack2 != memory2);
				REQUIRE(pack2.mEntry == nullptr);
				for (int i = 0; i < 5; ++i)
					REQUIRE(pack2[i] == darray1[i]);
			}
		}

		WHEN("Abandon-assign pack1 in pack2") {
			auto movable = pack1;
			const_cast<T&>(pack2) = Abandon(movable);

			THEN("memory1 should be overwritten, memory2 should be released") {
				REQUIRE(pack1.GetUses() == 3);
				REQUIRE(pack2.GetUses() == 3);
				REQUIRE(memory2.GetUses() == 1);
				REQUIRE(pack1 == pack2);
				REQUIRE(movable.mEntry == nullptr);
			}
		}

		WHEN("Shallow copy pack1 in pack2 and then reset pack1") {
			const_cast<T&>(pack2) = pack1;
			const_cast<T&>(pack1).Reset();

			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetUses() == 2);
				REQUIRE_FALSE(pack1.GetRaw());
				REQUIRE(pack1.GetReserved() == 0);
				REQUIRE(pack2 == memory1);
			}
		}

		WHEN("Deep copy pack1 in pack2") {
			const_cast<T&>(pack2) = pack1.Clone();

			THEN("memory1 should be referenced twice, memory2 should be released") {
				REQUIRE(pack1.GetUses() == 2);
				REQUIRE(pack2.GetUses() == 1);
				REQUIRE(pack1 == pack2);
				REQUIRE(pack2 == memory1);
				REQUIRE(pack2 != memory2);
			}
		}

		WHEN("Deep copy pack1 in pack2, then reset pack1") {
			const_cast<T&>(pack2) = pack1.Clone();
			const T memory3 = pack2;
			const_cast<T&>(pack1).Reset();

			THEN("memory1 should be referenced once, memory2 should be released") {
				REQUIRE_FALSE(pack1.HasAuthority());
				REQUIRE(pack2.GetUses() == 2);
				REQUIRE(memory3.GetUses() == 2);
			}
		}

		WHEN("Concatenate both packs to a third pack") {
			const auto pack3 = pack1 + pack2;

			THEN("The resulting pack must be a combination of the two") {
				for (int i = 0; i < 5; ++i)
					REQUIRE(pack3[i] == darray1[i]);
				for (int i = 5; i < 10; ++i)
					REQUIRE(pack3[i] == darray2[i - 5]);
			}
		}
	}

	if constexpr (CT::Sparse<E>)
		delete element;
}
