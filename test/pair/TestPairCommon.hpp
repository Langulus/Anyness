///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           

/// INTENTIONALLY NOT GUARDED                                                 
/// Include this file once in each cpp file, after all other headers          
#include <Anyness/Text.hpp>
#include <Anyness/Trait.hpp>
#include <Anyness/Pair.hpp>
#include <Anyness/TPair.hpp>
#include <unordered_map>
#include "../many/TestManyCommon.hpp"


///                                                                           
/// Possible states:                                                          
///   - uninitialized                                                         
///   - default                                                               
template<class K, class V>
void Pair_CheckState_Default(const auto&);
///   - invariant                                                             
template<class K, class V>
void Pair_CheckState_Invariant(const auto&);
///   - owned-full                                                            
template<class K, class V>
void Pair_CheckState_OwnedFull(const auto&);
///   - owned-full-const                                                      
template<class K, class V>
void Pair_CheckState_OwnedFullConst(const auto&);
///   - owned-empty                                                           
template<class K, class V>
void Pair_CheckState_OwnedEmpty(const auto&);
///   - disowned-full                                                         
template<class K, class V>
void Pair_CheckState_DisownedFull(const auto&);
///   - disowned-full-const                                                   
template<class K, class V>
void Pair_CheckState_DisownedFullConst(const auto&);
///   - abandoned                                                             
template<class K, class V>
void Pair_CheckState_Abandoned(const auto&);


template<class K, class V>
void Pair_CheckState_Default(const auto& pair) {
   if constexpr (CT::Typed<decltype(pair)>) {
      Any_CheckState_DisownedFullConst<K>(pair.GetKeyBlock());
      Any_CheckState_DisownedFullConst<V>(pair.GetValueBlock());
   }
   else {
      Any_CheckState_Default<K>(pair.GetKeyBlock());
      Any_CheckState_Default<V>(pair.GetValueBlock());
   }
}

template<class K, class V>
void Pair_CheckState_OwnedFull(const auto& pair) {
   if constexpr (CT::Typed<decltype(pair)>) {
      Any_CheckState_DisownedFullConst<K>(pair.GetKeyBlock());
      Any_CheckState_DisownedFullConst<V>(pair.GetValueBlock());
   }
   else {
      Any_CheckState_OwnedFull<K>(pair.GetKeyBlock());
      Any_CheckState_OwnedFull<V>(pair.GetValueBlock());
   }
}

template<class K, class V>
void Pair_CheckState_OwnedEmpty(const auto& pair) {
   if constexpr (CT::Typed<decltype(pair)>) {
      Any_CheckState_DisownedFullConst<K>(pair.GetKeyBlock());
      Any_CheckState_DisownedFullConst<V>(pair.GetValueBlock());
   }
   else {
      Any_CheckState_OwnedEmpty<K>(pair.GetKeyBlock());
      Any_CheckState_OwnedEmpty<V>(pair.GetValueBlock());
   }
}