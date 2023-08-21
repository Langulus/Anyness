///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <Anyness/Any.hpp>

using namespace Langulus;
using namespace Langulus::Anyness;

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

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md        
#define LANGULUS_EXCEPTION_HANDLER \
   CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) { \
      const Text serialized {ex}; \
      return ::std::string {Token {serialized}}; \
   }
