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
      UnorderedSet(UnorderedSet&&) noexcept;

      template<CT::NotSemantic T>
      UnorderedSet(::std::initializer_list<T>);

      template<CT::Semantic S>
      UnorderedSet(S&&) noexcept;

      UnorderedSet& operator = (const UnorderedSet&);
      UnorderedSet& operator = (UnorderedSet&&) noexcept;

      template<CT::Semantic S>
      UnorderedSet& operator = (S&&) noexcept;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&&);
      template<CT::Semantic S>
      Count Insert(S&&);

      UnorderedSet& operator << (CT::NotSemantic auto&&);
      UnorderedSet& operator << (const CT::NotSemantic auto&);
      UnorderedSet& operator << (CT::Semantic auto&&);

   protected:
      template<CT::Semantic S>
      Count InsertUnknown(S&&);
   };

} // namespace Langulus::Anyness

#include "UnorderedSet.inl"
