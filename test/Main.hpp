///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
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

using uint = unsigned int;

template<class T>
using some = std::vector<T>;

template<class L, class R>
struct TypePair {
   using LHS = L;
   using RHS = R;
};


template<CT::Dense T, class ALT_T>
T CreateElement(const ALT_T& e) {
   T element;
   if constexpr (CT::Same<T, ALT_T>)
      element = e;
   else if constexpr (not CT::Same<T, Block>)
      element = Decay<T> {e};
   else {
      element = Block {};
      element.Insert(e);
   }

   return element;
}

template<CT::Sparse T, class ALT_T>
T CreateElement(const ALT_T& e) {
   T element;
   if constexpr (not CT::Same<T, Block>)
      element = new Decay<T> {e};
   else {
      element = new Block {};
      element->Insert(e);
   }

   return element;
}


template<class C, class K, class V>
struct MapPair {
   using Container = C;
   using Key = K;
   using Value = V;
};

template<class K, class V>
struct MapPair2 {
   using Key = K;
   using Value = V;
};

template<class P, class K, class V, class ALT_K, class ALT_V>
P CreatePair(const ALT_K& key, const ALT_V& value) {
   return P {
      CreateElement<K>(key),
      CreateElement<V>(value)
   };
}

namespace std {
   template<>
   struct hash<Text> {
      size_t operator()(const Text& str) const noexcept {
         return str.GetHash().mHash;
      }
   };
}