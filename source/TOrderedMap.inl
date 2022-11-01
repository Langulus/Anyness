///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOrderedMap.hpp"

#define TEMPLATE() template<CT::Data K, CT::Data V>
#define MAP() TOrderedMap<K, V>

namespace Langulus::Anyness
{

   /// Copy-construct a map                                                   
   ///   @param other - the map to copy                                       
   TEMPLATE()
   MAP()::TOrderedMap(const TOrderedMap& other)
      : TUnorderedMap<K, V> {static_cast<const TUnorderedMap<K, V>&>(other)} { }

   /// Move-construct a map                                                   
   ///   @param other - the map to move                                       
   TEMPLATE()
   MAP()::TOrderedMap(TOrderedMap&& other) noexcept
      : TUnorderedMap<K, V> {Forward<TUnorderedMap<K, V>>(other)} { }
   
   /// Copy-construct a map from a disowned map                               
   /// The disowned map's contents will not be referenced                     
   ///   @param other - the map to disown                                     
   TEMPLATE()
   MAP()::TOrderedMap(Disowned<TOrderedMap>&& other) noexcept
      : TUnorderedMap<K, V> {other.template Forward<TUnorderedMap<K, V>>()} { }

   /// Move-construct a map from an abandoned map                             
   /// The abandoned map will be minimally reset, saving on some instructions 
   ///   @param other - the map to abandon                                    
   TEMPLATE()
   MAP()::TOrderedMap(Abandoned<TOrderedMap>&& other) noexcept
      : TUnorderedMap<K, V> {other.template Forward<TUnorderedMap<K, V>>()} { }

   /// Clone the map                                                          
   ///   @return the cloned map                                               
   TEMPLATE()
   MAP() MAP()::Clone() const {
      TOrderedMap<K, V> cloned;
      static_cast<TUnorderedMap<K, V>&>(cloned) = TUnorderedMap<K, V>::Clone();
      return Abandon(cloned);
   }

   /// Copy assignment                                                        
   ///   @param rhs - the map to copy                                         
   TEMPLATE()
   MAP()& MAP()::operator = (const TOrderedMap& rhs) {
      TUnorderedMap<K, V>::operator = (static_cast<const TUnorderedMap<K, V>&>(rhs));
      return *this;
   }

   /// Move assignment                                                        
   ///   @param rhs - the map to move                                         
   TEMPLATE()
   MAP()& MAP()::operator = (TOrderedMap&& rhs) noexcept {
      TUnorderedMap<K, V>::operator = (Forward<TUnorderedMap<K, V>>(rhs));
      return *this;
   }

} // namespace Langulus::Anyness

#undef MAP
#undef TEMPLATE

   
