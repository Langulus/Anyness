///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "OrderedMap.hpp"

namespace Langulus::Anyness
{
   
   /// Semantic ordered map construction                                      
   ///   @tparam S - type and semantic (deducible)                            
   ///   @param other - the ordered map to use                                
   template<CT::Semantic S>
   constexpr OrderedMap::OrderedMap(S&& other) noexcept requires (CT::DerivedFrom<TypeOf<S>, OrderedMap>)
      : BlockMap {other.template Forward<BlockMap>()} { }

} // namespace Langulus::Anyness
