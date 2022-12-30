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
   ///   Type-erased unordered map                                            
   ///                                                                        
   class UnorderedMap : public BlockMap {
   public:
      static constexpr bool Ordered = false;

      using BlockMap::BlockMap;

      constexpr UnorderedMap(Disowned<UnorderedMap>&&) noexcept;
      constexpr UnorderedMap(Abandoned<UnorderedMap>&&) noexcept;

      using BlockMap::operator =;

      NOD() UnorderedMap Clone() const;
   };

} // namespace Langulus::Anyness

#include "UnorderedMap.inl"
