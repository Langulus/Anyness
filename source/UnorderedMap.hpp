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

      constexpr UnorderedMap();
      UnorderedMap(const UnorderedMap&);
      UnorderedMap(UnorderedMap&&) noexcept;

      template<CT::Pair P>
      UnorderedMap(::std::initializer_list<P>);

      template<CT::Semantic S>
      UnorderedMap(S&&) noexcept;

      UnorderedMap& operator = (const UnorderedMap&);
      UnorderedMap& operator = (UnorderedMap&&) noexcept;

      UnorderedMap& operator = (const CT::Pair auto&);
      UnorderedMap& operator = (CT::Pair auto&&) noexcept;

      template<CT::Semantic S>
      UnorderedMap& operator = (S&&) noexcept;
   };

} // namespace Langulus::Anyness

#include "UnorderedMap.inl"
