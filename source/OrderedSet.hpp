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
      static constexpr bool Ownership = true;

      using BlockSet::BlockSet;

      OrderedSet(Disowned<OrderedSet>&&) noexcept;
      OrderedSet(Abandoned<OrderedSet>&&) noexcept;

      using BlockSet::operator =;

      NOD() OrderedSet Clone() const;
   };

} // namespace Langulus::Anyness

#include "OrderedSet.inl"
