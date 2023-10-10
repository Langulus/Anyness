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
   ///   Type-erased unordered map                                            
   ///                                                                        
   class UnorderedMap : public BlockMap {
   public:
      static constexpr bool Ownership = true;

      constexpr UnorderedMap() noexcept = default;
      UnorderedMap(const UnorderedMap&);
      UnorderedMap(UnorderedMap&&);

      UnorderedMap(const CT::NotSemantic auto&);
      UnorderedMap(CT::NotSemantic auto&);
      UnorderedMap(CT::NotSemantic auto&&);
      UnorderedMap(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      UnorderedMap(T1&&, T2&&, TAIL&&...);

      ~UnorderedMap();

      UnorderedMap& operator = (const UnorderedMap&);
      UnorderedMap& operator = (UnorderedMap&&);

      UnorderedMap& operator = (const CT::NotSemantic auto&);
      UnorderedMap& operator = (CT::NotSemantic auto&);
      UnorderedMap& operator = (CT::NotSemantic auto&&);
      UnorderedMap& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      UnorderedMap& operator << (auto&&);
   };

} // namespace Langulus::Anyness
