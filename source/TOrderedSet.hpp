///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedSet.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   /// A highly optimized ordered hashset implementation, using the Robin     
   /// Hood algorithm                                                         
   ///                                                                        
   template<CT::Data T>
   class TOrderedSet : public TUnorderedSet<T> {
   public:
      static constexpr bool Ordered = true;

      using TUnorderedSet<T>::TUnorderedSet;

      TOrderedSet(const TOrderedSet&);
      TOrderedSet(TOrderedSet&&) noexcept;

      constexpr TOrderedSet(Disowned<TOrderedSet>&&) noexcept;
      constexpr TOrderedSet(Abandoned<TOrderedSet>&&) noexcept;

      TOrderedSet& operator = (const TOrderedSet&);
      TOrderedSet& operator = (TOrderedSet&&) noexcept;

      using TUnorderedSet<T>::operator =;

      NOD() TOrderedSet Clone() const;
   };


   /// The default map is always ordered                                      
   template<CT::Data T>
   using TSet = TOrderedSet<T>;

} // namespace Langulus::Anyness

#include "TOrderedSet.inl"
