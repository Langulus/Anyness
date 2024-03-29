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
   ///   Type-erased unordered set                                            
   ///                                                                        
   class UnorderedSet : public BlockSet {
   public:
      static constexpr bool Ownership = true;
      static constexpr bool Ordered = false;

      constexpr UnorderedSet() noexcept = default;
      UnorderedSet(const UnorderedSet&);
      UnorderedSet(UnorderedSet&&);

      UnorderedSet(const CT::NotSemantic auto&);
      UnorderedSet(CT::NotSemantic auto&);
      UnorderedSet(CT::NotSemantic auto&&);
      UnorderedSet(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      UnorderedSet(T1&&, T2&&, TAIL&&...);

      ~UnorderedSet();

      UnorderedSet& operator = (const UnorderedSet&);
      UnorderedSet& operator = (UnorderedSet&&);

      UnorderedSet& operator = (const CT::NotSemantic auto&);
      UnorderedSet& operator = (CT::NotSemantic auto&);
      UnorderedSet& operator = (CT::NotSemantic auto&&);
      UnorderedSet& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      UnorderedSet& operator << (auto&&);
   };

} // namespace Langulus::Anyness
