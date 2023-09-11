///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
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
      static constexpr bool Ownership = true;
      static constexpr bool Ordered = true;

      constexpr OrderedSet() noexcept = default;
      OrderedSet(const OrderedSet&);
      OrderedSet(OrderedSet&&);

      OrderedSet(const CT::NotSemantic auto&);
      OrderedSet(CT::NotSemantic auto&);
      OrderedSet(CT::NotSemantic auto&&);
      OrderedSet(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      OrderedSet(T1&&, T2&&, TAIL&&...);

      ~OrderedSet();

      OrderedSet& operator = (const OrderedSet&);
      OrderedSet& operator = (OrderedSet&&);

      OrderedSet& operator = (const CT::NotSemantic auto&);
      OrderedSet& operator = (CT::NotSemantic auto&);
      OrderedSet& operator = (CT::NotSemantic auto&&);
      OrderedSet& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&&);
      Count Insert(CT::Semantic auto&&);

      OrderedSet& operator << (const CT::NotSemantic auto&);
      OrderedSet& operator << (CT::NotSemantic auto&);
      OrderedSet& operator << (CT::NotSemantic auto&&);
      OrderedSet& operator << (CT::Semantic auto&&);

   protected:
      Count InsertUnknown(CT::Semantic auto&&);
   };

} // namespace Langulus::Anyness
