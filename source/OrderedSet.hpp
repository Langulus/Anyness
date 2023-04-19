///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "blocks/BlockSet.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased ordered set                                              
   ///                                                                        
   class OrderedSet : public BlockSet {
   public:
      static constexpr bool Ordered = true;

      constexpr OrderedSet();
      OrderedSet(const OrderedSet&);
      OrderedSet(OrderedSet&&) noexcept;

      template<CT::NotSemantic T>
      OrderedSet(::std::initializer_list<T>);

      template<CT::Semantic S>
      OrderedSet(S&&) noexcept;

      OrderedSet& operator = (const OrderedSet&);
      OrderedSet& operator = (OrderedSet&&) noexcept;

      template<CT::Semantic S>
      OrderedSet& operator = (S&&) noexcept;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&&);
      template<CT::Semantic S>
      Count Insert(S&&);

      OrderedSet& operator << (CT::NotSemantic auto&&);
      OrderedSet& operator << (const CT::NotSemantic auto&);
      OrderedSet& operator << (CT::Semantic auto&&);

   protected:
      template<CT::Semantic S>
      Count InsertUnknown(S&&);
   };

} // namespace Langulus::Anyness

#include "OrderedSet.inl"
