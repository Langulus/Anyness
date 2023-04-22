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
   ///   Type-erased unordered set                                            
   ///                                                                        
   class UnorderedSet : public BlockSet {
   public:
      static constexpr bool Ordered = false;

      constexpr UnorderedSet();
      UnorderedSet(const UnorderedSet&);
      UnorderedSet(UnorderedSet&&);

      UnorderedSet(const CT::NotSemantic auto&);
      UnorderedSet(CT::NotSemantic auto&);
      UnorderedSet(CT::NotSemantic auto&&);
      UnorderedSet(CT::Semantic auto&&);

      template<CT::Data HEAD, CT::Data... TAIL>
      UnorderedSet(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      UnorderedSet& operator = (const UnorderedSet&);
      UnorderedSet& operator = (UnorderedSet&&);

      UnorderedSet& operator = (const CT::NotSemantic auto&);
      UnorderedSet& operator = (CT::NotSemantic auto&);
      UnorderedSet& operator = (CT::NotSemantic auto&&);
      UnorderedSet& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&&);
      Count Insert(CT::Semantic auto&&);

      UnorderedSet& operator << (const CT::NotSemantic auto&);
      UnorderedSet& operator << (CT::NotSemantic auto&);
      UnorderedSet& operator << (CT::NotSemantic auto&&);
      UnorderedSet& operator << (CT::Semantic auto&&);

   protected:
      Count InsertUnknown(CT::Semantic auto&&);
   };

} // namespace Langulus::Anyness

#include "UnorderedSet.inl"
