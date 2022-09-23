///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#define LANGULUS_EXPOSE_PRIVATE_HEADERS
#include <LangulusAnyness.hpp>

using namespace Langulus;
using namespace Langulus::Anyness;

using Anyness::Inner::Allocator;
using Anyness::Inner::Allocation;
using Anyness::Inner::Pool;

//#define LANGULUS_STD_BENCHMARK

//#ifdef LANGULUS_STD_BENCHMARK
#define CATCH_CONFIG_ENABLE_BENCHMARKING
//#endif

inline Byte* asbytes(void* a) noexcept {
	return reinterpret_cast<Byte*>(a);
}

inline const Byte* asbytes(const void* a) noexcept {
	return reinterpret_cast<const Byte*>(a);
}
