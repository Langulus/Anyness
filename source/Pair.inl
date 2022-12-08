///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Pair.hpp"

namespace Langulus::Anyness
{

   /// Pair constructor by shallow-copy                                       
   ///   @tparam K - type of key (deducible)                                  
   ///   @tparam V - type of value (deducible)                                
   ///   @param key - the key to copy                                         
   ///   @param value - the value to copy                                     
   template<class K, class V>
   Pair::Pair(const K& key, const V& value)
      : mKey {key}
      , mValue {value} {}

   /// Pair constructor by move                                               
   ///   @tparam K - type of key (deducible)                                  
   ///   @tparam V - type of value (deducible)                                
   ///   @param key - the key to move                                         
   ///   @param value - the value to move                                     
   template<class K, class V>
   Pair::Pair(K&& key, V&& value)
      : mKey {Forward<K>(key)}
      , mValue {Forward<V>(value)} {}

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   inline Hash Pair::GetHash() const {
      return HashData(mKey.GetHash(), mValue.GetHash());
   }

} // namespace Langulus::Anyness
