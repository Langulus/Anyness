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

   /// Copy-construct a map from a disowned map                               
   /// The disowned map's contents will not be referenced                     
   ///   @param other - the map to disown                                     
   inline OrderedMap::OrderedMap(Disowned<OrderedMap>&& other) noexcept
      : BlockMap {other.Forward<BlockMap>()} { }

   /// Move-construct a map from an abandoned map                             
   /// The abandoned map will be minimally reset, saving on some instructions 
   ///   @param other - the map to abandon                                    
   inline OrderedMap::OrderedMap(Abandoned<OrderedMap>&& other) noexcept
      : BlockMap {other.Forward<BlockMap>()} { }

   /// Clone the map                                                          
   ///   @return the cloned map                                               
   inline OrderedMap OrderedMap::Clone() const {
      OrderedMap cloned;
      static_cast<BlockMap&>(cloned) = BlockMap::Clone();
      return Abandon(cloned);
   }

} // namespace Langulus::Anyness
