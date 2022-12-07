///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOrderedSet.hpp"

#define TEMPLATE() template<CT::Data T>
#define MAP() TOrderedSet<T>

namespace Langulus::Anyness
{

   /// Copy-construct a map                                                   
   ///   @param other - the map to copy                                       
   TEMPLATE()
   MAP()::TOrderedSet(const TOrderedSet& other)
      : TUnorderedSet<T> {static_cast<const TUnorderedSet<T>&>(other)} { }

   /// Move-construct a map                                                   
   ///   @param other - the map to move                                       
   TEMPLATE()
   MAP()::TOrderedSet(TOrderedSet&& other) noexcept
      : TUnorderedSet<T> {Forward<TUnorderedSet<T>>(other)} { }
   
   /// Copy-construct a map from a disowned map                               
   /// The disowned map's contents will not be referenced                     
   ///   @param other - the map to disown                                     
   TEMPLATE()
   constexpr MAP()::TOrderedSet(Disowned<TOrderedSet>&& other) noexcept
      : TUnorderedSet<T> {other.template Forward<TUnorderedSet<T>>()} { }

   /// Move-construct a map from an abandoned map                             
   /// The abandoned map will be minimally reset, saving on some instructions 
   ///   @param other - the map to abandon                                    
   TEMPLATE()
   constexpr MAP()::TOrderedSet(Abandoned<TOrderedSet>&& other) noexcept
      : TUnorderedSet<T> {other.template Forward<TUnorderedSet<T>>()} { }

   /// Clone the map                                                          
   ///   @return the cloned map                                               
   TEMPLATE()
   MAP() MAP()::Clone() const {
      TOrderedSet<T> cloned;
      static_cast<TUnorderedSet<T>&>(cloned) = TUnorderedSet<T>::Clone();
      return Abandon(cloned);
   }

   /// Copy assignment                                                        
   ///   @param rhs - the map to copy                                         
   TEMPLATE()
   MAP()& MAP()::operator = (const TOrderedSet& rhs) {
      TUnorderedSet<T>::operator = (static_cast<const TUnorderedSet<T>&>(rhs));
      return *this;
   }

   /// Move assignment                                                        
   ///   @param rhs - the map to move                                         
   TEMPLATE()
   MAP()& MAP()::operator = (TOrderedSet&& rhs) noexcept {
      TUnorderedSet<T>::operator = (Forward<TUnorderedSet<T>>(rhs));
      return *this;
   }

} // namespace Langulus::Anyness

#undef MAP
#undef TEMPLATE

   
