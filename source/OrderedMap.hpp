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

      constexpr OrderedMap();
      OrderedMap(const OrderedMap&);
      OrderedMap(OrderedMap&&) noexcept;

      template<CT::Pair P>
      OrderedMap(::std::initializer_list<P>);

      template<CT::Semantic S>
      OrderedMap(S&&) noexcept;

      OrderedMap& operator = (const OrderedMap&);
      OrderedMap& operator = (OrderedMap&&) noexcept;

      OrderedMap& operator = (const CT::Pair auto&);
      OrderedMap& operator = (CT::Pair auto&&) noexcept;

      template<CT::Semantic S>
      OrderedMap& operator = (S&&) noexcept;

      ///                                                                     
      ///   Search                                                            
      ///                                                                     
      template<CT::NotSemantic K>
      NOD() Block At(const K&);
      template<CT::NotSemantic K>
      NOD() Block operator[] (const K&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const CT::NotSemantic auto&, const CT::NotSemantic auto&);
      Count Insert(const CT::NotSemantic auto&, CT::NotSemantic auto&&);
      Count Insert(CT::NotSemantic auto&&, const CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&&, CT::NotSemantic auto&&);
      Count Insert(CT::Pair auto&&);
      Count Insert(const CT::Pair auto&);

      template<CT::Semantic SK, CT::Semantic SV>
      Count Insert(SK&&, SV&&);
      template<CT::Semantic S>
      Count Insert(S&&);

      OrderedMap& operator << (CT::Pair auto&&);
      OrderedMap& operator << (const CT::Pair auto&);

      template<CT::Semantic S>
      OrderedMap& operator << (S&&);

   protected:
      template<CT::Semantic SK, CT::Semantic SV>
      Count InsertUnknown(SK&&, SV&&);
      template<CT::Semantic SP>
      Count InsertUnknown(SP&&);
   };

} // namespace Langulus::Anyness

#include "OrderedMap.inl"
