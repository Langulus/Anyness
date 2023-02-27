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

      UnorderedMap& operator << (CT::Pair auto&&);
      UnorderedMap& operator << (const CT::Pair auto&);

      template<CT::Semantic S>
      UnorderedMap& operator << (S&&);

   protected:
      template<CT::Semantic SK, CT::Semantic SV>
      Count InsertUnknown(SK&&, SV&&);
      template<CT::Semantic SP>
      Count InsertUnknown(SP&&);
   };

} // namespace Langulus::Anyness

#include "UnorderedMap.inl"
