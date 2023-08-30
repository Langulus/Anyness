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
      static constexpr bool Ownership = true;
      static constexpr bool Ordered = false;

      constexpr UnorderedMap() noexcept = default;
      UnorderedMap(const UnorderedMap&);
      UnorderedMap(UnorderedMap&&);

      UnorderedMap(const CT::NotSemantic auto&);
      UnorderedMap(CT::NotSemantic auto&);
      UnorderedMap(CT::NotSemantic auto&&);
      UnorderedMap(CT::Semantic auto&&);

      template<CT::Data HEAD, CT::Data... TAIL>
      UnorderedMap(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      ~UnorderedMap();

      UnorderedMap& operator = (const UnorderedMap&);
      UnorderedMap& operator = (UnorderedMap&&);

      UnorderedMap& operator = (const CT::NotSemantic auto&);
      UnorderedMap& operator = (CT::NotSemantic auto&);
      UnorderedMap& operator = (CT::NotSemantic auto&&);
      UnorderedMap& operator = (CT::Semantic auto&&);

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
      Count Insert(const CT::NotSemantic auto&,  const CT::NotSemantic auto&);
      Count Insert(const CT::NotSemantic auto&,        CT::NotSemantic auto&);
      Count Insert(const CT::NotSemantic auto&,        CT::NotSemantic auto&&);
      Count Insert(const CT::NotSemantic auto&,        CT::Semantic    auto&&);

      Count Insert(      CT::NotSemantic auto&,  const CT::NotSemantic auto&);
      Count Insert(      CT::NotSemantic auto&,        CT::NotSemantic auto&);
      Count Insert(      CT::NotSemantic auto&,        CT::NotSemantic auto&&);
      Count Insert(      CT::NotSemantic auto&,        CT::Semantic    auto&&);

      Count Insert(      CT::NotSemantic auto&&, const CT::NotSemantic auto&);
      Count Insert(      CT::NotSemantic auto&&,       CT::NotSemantic auto&);
      Count Insert(      CT::NotSemantic auto&&,       CT::NotSemantic auto&&);
      Count Insert(      CT::NotSemantic auto&&,       CT::Semantic    auto&&);

      Count Insert(      CT::Semantic    auto&&, const CT::NotSemantic auto&);
      Count Insert(      CT::Semantic    auto&&,       CT::NotSemantic auto&);
      Count Insert(      CT::Semantic    auto&&,       CT::NotSemantic auto&&);
      Count Insert(      CT::Semantic    auto&&,       CT::Semantic    auto&&);

      Count Insert(const CT::NotSemantic auto&);
      Count Insert(      CT::NotSemantic auto&);
      Count Insert(      CT::NotSemantic auto&&);
      Count Insert(      CT::Semantic    auto&&);

      UnorderedMap& operator << (const CT::NotSemantic auto&);
      UnorderedMap& operator << (      CT::NotSemantic auto&);
      UnorderedMap& operator << (      CT::NotSemantic auto&&);
      UnorderedMap& operator << (      CT::Semantic    auto&&);

   protected:
      Count InsertUnknown(CT::Semantic auto&&, CT::Semantic auto&&);
      Count InsertUnknown(CT::Semantic auto&&);
   };

} // namespace Langulus::Anyness
