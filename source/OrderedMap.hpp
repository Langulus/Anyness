///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
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
      static constexpr bool Ownership = true;
      static constexpr bool Ordered = true;

      constexpr OrderedMap() noexcept = default;
      OrderedMap(const OrderedMap&);
      OrderedMap(OrderedMap&&);

      OrderedMap(const CT::NotSemantic auto&);
      OrderedMap(CT::NotSemantic auto&);
      OrderedMap(CT::NotSemantic auto&&);
      OrderedMap(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      OrderedMap(T1&&, T2&&, TAIL&&...);

      ~OrderedMap();

      OrderedMap& operator = (const OrderedMap&);
      OrderedMap& operator = (OrderedMap&&);

      OrderedMap& operator = (const CT::NotSemantic auto&);
      OrderedMap& operator = (CT::NotSemantic auto&);
      OrderedMap& operator = (CT::NotSemantic auto&&);
      OrderedMap& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(auto&&, auto&&);
      Count InsertBlock(auto&&, auto&&);
      Count InsertPair(auto&&);
      Count InsertPairBlock(auto&&);

      OrderedMap& operator << (auto&&);
   };

} // namespace Langulus::Anyness
