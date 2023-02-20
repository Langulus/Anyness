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

      //TODO defined in header due to MSVC compiler bug (02/2023)       
      // Might be fixed in the future                                   
      /*template<CT::Semantic S>
      constexpr UnorderedMap(S&& other) noexcept requires (CT::DerivedFrom<TypeOf<S>, UnorderedMap>)
         : BlockMap {other.template Forward<BlockMap>()} { }*/

      using BlockMap::operator =;
   };

} // namespace Langulus::Anyness

#include "UnorderedMap.inl"
