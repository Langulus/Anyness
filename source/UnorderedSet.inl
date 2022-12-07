///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedSet.hpp"

namespace Langulus::Anyness
{

   /// Copy-construct a set from a disowned set                               
   /// The disowned set's contents will not be referenced                     
   ///   @param other - the set to disown                                     
   constexpr UnorderedSet::UnorderedSet(Disowned<UnorderedSet>&& other) noexcept
      : BlockSet {other.Forward<BlockSet>()} { }

   /// Move-construct a set from an abandoned set                             
   /// The abandoned set will be minimally reset, saving on some instructions 
   ///   @param other - the set to abandon                                    
   constexpr UnorderedSet::UnorderedSet(Abandoned<UnorderedSet>&& other) noexcept
      : BlockSet {other.Forward<BlockSet>()} { }

   /// Clone the set                                                          
   ///   @return the cloned set                                               
   inline UnorderedSet UnorderedSet::Clone() const {
      UnorderedSet cloned;
      static_cast<BlockSet&>(cloned) = BlockSet::Clone();
      return Abandon(cloned);
   }

} // namespace Langulus::Anyness
