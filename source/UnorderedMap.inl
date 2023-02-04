///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedMap.hpp"

namespace Langulus::Anyness
{

   /// Copy-construct a map from a disowned map                               
   /// The disowned map's contents will not be referenced                     
   ///   @param other - the map to disown                                     
   /*constexpr UnorderedMap::UnorderedMap(Disowned<UnorderedMap>&& other) noexcept
      : BlockMap {other.Forward<BlockMap>()} { }

   /// Move-construct a map from an abandoned map                             
   /// The abandoned map will be minimally reset, saving on some instructions 
   ///   @param other - the map to abandon                                    
   constexpr UnorderedMap::UnorderedMap(Abandoned<UnorderedMap>&& other) noexcept
      : BlockMap {other.Forward<BlockMap>()} { }*/

   /// Clone the map                                                          
   ///   @return the cloned map                                               
   /*inline UnorderedMap UnorderedMap::Clone() const {
      UnorderedMap cloned;
      static_cast<BlockMap&>(cloned) = BlockMap::Clone();
      return Abandon(cloned);
   }*/

} // namespace Langulus::Anyness
