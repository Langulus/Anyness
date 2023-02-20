///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "blocks/BlockMap.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased ordered map                                              
   ///                                                                        
   class OrderedMap : public BlockMap {
   public:
      static constexpr bool Ordered = true;
      static constexpr bool Ownership = true;

      using BlockMap::BlockMap;

      template<CT::Semantic S>
      constexpr OrderedMap(S&&) noexcept requires (CT::DerivedFrom<TypeOf<S>, OrderedMap>);

      using BlockMap::operator =;
   };

} // namespace Langulus::Anyness

#include "OrderedMap.inl"
