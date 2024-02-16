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
#include <Anyness/Any.hpp>
#include <Anyness/TAny.hpp>
#include "../Common.hpp"

template<class T, class E>
decltype(auto) FromHelper() {
   if constexpr (not CT::Typed<T>) {
      if constexpr (CT::TraitBased<T>) {
         if constexpr (CT::Trait<T>)
            return T::template OfType<E>();
         else
            return T::template From<Traits::Count, E>();
      }
      else return T::template From<E>();
   }
   else return T {};
}


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

template<class E>
void Helper_TestType(const auto& any) {
   REQUIRE      (any.IsTyped());
   REQUIRE_FALSE(any.IsUntyped());
   REQUIRE      (any.GetType() == MetaDataOf<E>());
   REQUIRE      (any.GetType()->template IsSimilar<const E>());
   REQUIRE      (any.GetType()->template IsExact<E>());
   REQUIRE      (any.GetType()->template Is<E*>());
   REQUIRE      (any.IsDense() == CT::Dense<E>);
   REQUIRE      (any.IsSparse() == CT::Sparse<E>);
   REQUIRE      (any.IsDeep() == CT::Deep<Decay<E>>);
}

template<CT::BlockBased LHS, CT::BlockBased RHS>
void Helper_TestSame(const LHS& lhs, const RHS& rhs) {
   REQUIRE(lhs.GetRaw() == rhs.GetRaw());
   REQUIRE(lhs.IsExact(rhs.GetType()));
   REQUIRE(lhs == rhs);
   REQUIRE(lhs.IsDeep() == rhs.IsDeep());
   REQUIRE(lhs.IsConstant() == rhs.IsConstant());
   REQUIRE(lhs.GetUnconstrainedState() == rhs.GetUnconstrainedState());
}


///                                                                           
/// Possible actions for each state:                                          
///   - uninitialized                                                         
///      - constexpr-default-initialized                                      
///      - runtime-default-initialized                                        
///      - semantic-initialized from container                                
///      - semantic-initialized from single dense element                     
///      - semantic-initialized from multiple dense elements                  
///      - semantic-initialized from dense element bounded array              

template<class E>
void CheckState_Default(const auto& any) {
   using T = Decay<decltype(any)>;

   if constexpr (CT::Typed<T>) {
      static_assert(CT::Exact<TypeOf<T>, E>);
      Helper_TestType<E>(any);
      REQUIRE      (any.GetState() == DataState::Typed);
   }
   else {
      REQUIRE_FALSE(any.IsTyped());
      REQUIRE      (any.IsUntyped());
      REQUIRE      (any.GetType() == nullptr);
      REQUIRE      (any.IsDense());
      REQUIRE_FALSE(any.IsSparse());
      REQUIRE      (any.GetState() == DataState::Default);
      REQUIRE_FALSE(any.IsDeep());
   }

   REQUIRE      (any.IsTypeConstrained() == CT::Typed<T>);
   REQUIRE_FALSE(any.IsCompressed());
   REQUIRE      (any.IsConstant() == CT::Constant<E>);
   REQUIRE_FALSE(any.IsEncrypted());
   REQUIRE_FALSE(any.IsMissing());
   REQUIRE_FALSE(any.IsOr());
   REQUIRE_FALSE(any.IsStatic());
   REQUIRE_FALSE(any.IsValid());
   REQUIRE      (any.IsInvalid());
   REQUIRE_FALSE(any.IsAllocated());
   REQUIRE_FALSE(any.HasAuthority());
   REQUIRE      (any.IsNow());
   REQUIRE_FALSE(any.IsFuture());
   REQUIRE_FALSE(any.IsPast());
   REQUIRE      (any.IsEmpty());
   REQUIRE      (any.GetCount() == 0);
   REQUIRE      (any.GetReserved() == 0);
   REQUIRE      (any.GetUses() == 0);
   REQUIRE      (any.GetRaw() == nullptr);
   REQUIRE_FALSE(any);
   REQUIRE      (not any);
}

template<class E>
void CheckState_OwnedEmpty(const auto& any) {
   using T = Decay<decltype(any)>;

   Helper_TestType<E>(any);

   REQUIRE      (any.IsTypeConstrained() == CT::Typed<T>);
   REQUIRE_FALSE(any.IsCompressed());
   REQUIRE      (any.IsConstant() == CT::Constant<E>);
   REQUIRE_FALSE(any.IsEncrypted());
   REQUIRE_FALSE(any.IsStatic());
   REQUIRE_FALSE(any.IsValid());
   REQUIRE      (any.IsInvalid());
   REQUIRE      (any.IsAllocated());
   REQUIRE      (any.HasAuthority());
   REQUIRE      (any.IsEmpty());
   REQUIRE      (any.GetCount() == 0);
   REQUIRE      (any.GetReserved() > 0);
   REQUIRE      (any.GetUses() == 1);
   REQUIRE      (any.GetRaw());
   REQUIRE_FALSE(any);
   REQUIRE      (not any);
}

template<class E>
void CheckState_OwnedFull(const auto& any) {
   using T = Decay<decltype(any)>;

   Helper_TestType<E>(any);

   REQUIRE      (any.IsTypeConstrained() == CT::Typed<T>);
   REQUIRE_FALSE(any.IsCompressed());
   REQUIRE      (any.IsConstant() == CT::Constant<E>);
   REQUIRE_FALSE(any.IsEncrypted());
   REQUIRE      (any.IsValid());
   REQUIRE_FALSE(any.IsInvalid());
   REQUIRE_FALSE(any.IsStatic());
   REQUIRE      (any.IsAllocated());
   REQUIRE      (any.HasAuthority());
   REQUIRE_FALSE(any.IsEmpty());
   REQUIRE      (any.GetCount() > 0);
   REQUIRE      (any.GetReserved() > 0);
   REQUIRE      (any.GetUses() > 0);
   REQUIRE      (any.GetRaw());
   REQUIRE      (any);
   REQUIRE_FALSE(not any);
}


template<class E>
void CheckState_Abandoned(const auto& any) {
   REQUIRE_FALSE(any.HasAuthority());
}