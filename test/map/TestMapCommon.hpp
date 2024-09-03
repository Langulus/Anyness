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
#include <Anyness/TMap.hpp>
#include <Anyness/Map.hpp>
#include <unordered_map>
#include "../Common.hpp"


///                                                                           
/// Possible states:                                                          
///   - uninitialized                                                         
///   - default                                                               
template<class E>
void CheckState_Default(const auto&);
///   - invariant                                                             
template<class E>
void CheckState_Invariant(const auto&);
///   - owned-full                                                            
template<class E>
void CheckState_OwnedFull(const auto&);
///   - owned-full-const                                                      
template<class E>
void CheckState_OwnedFullConst(const auto&);
///   - owned-empty                                                           
template<class E>
void CheckState_OwnedEmpty(const auto&);
///   - disowned-full                                                         
template<class E>
void CheckState_DisownedFull(const auto&);
///   - disowned-full-const                                                   
template<class E>
void CheckState_DisownedFullConst(const auto&);
///   - abandoned                                                             
template<class E>
void CheckState_Abandoned(const auto&);



template<class K, class V>
void Helper_TestType(const auto& any) {
   REQUIRE      (any.IsKeyTyped());
   REQUIRE      (any.IsValueTyped());
   REQUIRE_FALSE(any.IsKeyUntyped());
   REQUIRE_FALSE(any.IsValueUntyped());

   REQUIRE      (any.GetKeyType() == MetaDataOf<K>());
   REQUIRE      (any.GetKeyType()->template IsSimilar<const K>());
   REQUIRE      (any.GetKeyType()->template IsExact<K>());
   REQUIRE      (any.GetKeyType()->template Is<K*>());
   REQUIRE      (any.IsKeyDense() == CT::Dense<K>);
   REQUIRE      (any.IsKeySparse() == CT::Sparse<K>);
   REQUIRE      (any.IsKeyDeep() == CT::Deep<Decay<K>>);

   REQUIRE      (any.GetValueType() == MetaDataOf<V>());
   REQUIRE      (any.GetValueType()->template IsSimilar<const V>());
   REQUIRE      (any.GetValueType()->template IsExact<V>());
   REQUIRE      (any.GetValueType()->template Is<V*>());
   REQUIRE      (any.IsValueDense() == CT::Dense<V>);
   REQUIRE      (any.IsValueSparse() == CT::Sparse<V>);
   REQUIRE      (any.IsValueDeep() == CT::Deep<Decay<V>>);
}

template<CT::Map LHS, CT::Map RHS>
void Helper_TestSame(const LHS& lhs, const RHS& rhs) {
   REQUIRE(lhs.GetRaw() == rhs.GetRaw());
   REQUIRE(lhs.IsKeyExact(rhs.GetKeyType()));
   REQUIRE(lhs.IsValueExact(rhs.GetValueType()));
   REQUIRE(lhs == rhs);
   REQUIRE(lhs.IsDeep() == rhs.IsDeep());
   REQUIRE(lhs.IsConstant() == rhs.IsConstant());
   REQUIRE(lhs.GetUnconstrainedState() == rhs.GetUnconstrainedState());
}



template<class K, class V>
void CheckState_Default(const auto& map) {
   using T = Decay<decltype(map)>;

   if constexpr (CT::Typed<T>) {
      static_assert(CT::Exact<typename T::Key, K>);
      static_assert(CT::Exact<typename T::Value, V>);
      Helper_TestType<K, V>(map);
      REQUIRE      (map.GetKeyState() == DataState::Typed);
      REQUIRE      (map.GetValueState() == DataState::Typed);
   }
   else {
      REQUIRE_FALSE(map.IsKeyTyped());
      REQUIRE_FALSE(map.IsValueTyped());
      REQUIRE      (map.IsKeyUntyped());
      REQUIRE      (map.IsValueUntyped());
      REQUIRE      (map.GetKeyType() == nullptr);
      REQUIRE      (map.GetValueType() == nullptr);
      REQUIRE      (map.IsKeyDense());
      REQUIRE      (map.IsValueDense());
      REQUIRE_FALSE(map.IsKeySparse());
      REQUIRE_FALSE(map.IsValueSparse());
      REQUIRE      (map.GetKeyState() == DataState::Default);
      REQUIRE      (map.GetValueState() == DataState::Default);
      REQUIRE_FALSE(map.IsKeyDeep());
      REQUIRE_FALSE(map.IsValueDeep());
   }

   REQUIRE      (map.IsKeyTypeConstrained() == CT::Typed<T>);
   REQUIRE      (map.IsValueTypeConstrained() == CT::Typed<T>);
   REQUIRE_FALSE(map.IsKeyCompressed());
   REQUIRE_FALSE(map.IsValueCompressed());
   REQUIRE      (map.IsKeyConstant() == CT::Constant<K>);
   REQUIRE      (map.IsValueConstant() == CT::Constant<V>);
   REQUIRE_FALSE(map.IsKeyEncrypted());
   REQUIRE_FALSE(map.IsValueEncrypted());
   REQUIRE_FALSE(map.IsKeyMissing());
   REQUIRE_FALSE(map.IsValueMissing());
   REQUIRE_FALSE(map.IsValid());
   REQUIRE      (map.IsInvalid());
   REQUIRE_FALSE(map.IsAllocated());
   REQUIRE_FALSE(map.GetKeys().GetAllocation());
   REQUIRE_FALSE(map.GetVals().GetAllocation());
   REQUIRE      (map.IsEmpty());
   REQUIRE      (map.GetCount() == 0);
   REQUIRE      (map.GetReserved() == 0);
   REQUIRE      (map.GetKeys().GetUses() == 0);
   REQUIRE      (map.GetVals().GetUses() == 0);
   REQUIRE      (map.GetRawKeysMemory() == nullptr);
   REQUIRE      (map.GetRawValsMemory() == nullptr);
   REQUIRE_FALSE(map);
   REQUIRE      (not map);
}