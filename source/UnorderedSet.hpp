///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased unordered set                                            
   ///                                                                        
   class UnorderedSet : public BlockSet {
   public:
      static constexpr bool Ordered = false;

      using BlockSet::BlockSet;
      constexpr UnorderedSet(Disowned<UnorderedSet>&&) noexcept;
      constexpr UnorderedSet(Abandoned<UnorderedSet>&&) noexcept;

      using BlockSet::operator =;

      NOD() UnorderedSet Clone() const;
   };

} // namespace Langulus::Anyness

#include "UnorderedSet.inl"
