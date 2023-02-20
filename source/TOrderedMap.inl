///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOrderedMap.hpp"

#define TEMPLATE() template<CT::Data K, CT::Data V>
#define MAP() TOrderedMap<K, V>

namespace Langulus::Anyness
{

   /// Copy-constructor                                                       
   ///   @param other - the map to copy                                       
   TEMPLATE()
   MAP()::TOrderedMap(const TOrderedMap& other)
      : Self {::Langulus::Copy(other)} { }

   /// Move-constructor                                                       
   ///   @param other - the map to move                                       
   TEMPLATE()
   MAP()::TOrderedMap(TOrderedMap&& other) noexcept
      : Self {::Langulus::Move(other)} { }

   /// Copy assignment                                                        
   ///   @param rhs - the map to copy                                         
   TEMPLATE()
   MAP()& MAP()::operator = (const TOrderedMap& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the map to move                                         
   TEMPLATE()
   MAP()& MAP()::operator = (TOrderedMap&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Semantic assignment for an ordered map                                 
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the ordered map to use for construction                 
   TEMPLATE()
   template<CT::Semantic S>
   MAP()& MAP()::operator = (S&& rhs) noexcept requires (CT::Exact<TypeOf<S>, Self>) {
      Base::operator = (rhs.template Forward<Base>(rhs));
      return *this;
   }
   
   /// Copy assign a pair                                                     
   ///   @param rhs - the pair to copy                                        
   TEMPLATE()
   MAP()& MAP()::operator = (const Pair& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assign a pair                                                     
   ///   @param rhs - the pair to move                                        
   TEMPLATE()
   MAP()& MAP()::operator = (Pair&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Semantic assignment for a pair                                         
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the pair to use                                         
   TEMPLATE()
   template<CT::Semantic S>
   MAP()& MAP()::operator = (S&& rhs) noexcept requires (CT::Pair<TypeOf<S>>) {
      TODO();
      return *this;
   }

} // namespace Langulus::Anyness

#undef MAP
#undef TEMPLATE

   
