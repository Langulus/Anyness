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

   /// Default pair constructor                                               
   LANGULUS(ALWAYSINLINE)
   constexpr Pair::Pair() {}

   /// Pair constructor by shallow-copy                                       
   ///   @tparam K - type of key (deducible)                                  
   ///   @tparam V - type of value (deducible)                                
   ///   @param key - the key to copy                                         
   ///   @param value - the value to copy                                     
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Pair::Pair(const K& key, const V& val)
      : Pair {Langulus::Copy(key), Langulus::Copy(val)} {}

   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Pair::Pair(const K& key, V&& val)
      : Pair {Langulus::Copy(key), Langulus::Move(val)} {}

   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Pair::Pair(K&& key, const V& val)
      : Pair {Langulus::Move(key), Langulus::Copy(val)} {}

   /// Pair constructor by move                                               
   ///   @tparam K - type of key (deducible)                                  
   ///   @tparam V - type of value (deducible)                                
   ///   @param key - the key to move                                         
   ///   @param value - the value to move                                     
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Pair::Pair(K&& key, V&& val)
      : Pair {Langulus::Move(key), Langulus::Move(val)} {}

   /// Pair semantic constructor                                              
   ///   @tparam SK - type and semantic of key (deducible)                    
   ///   @tparam SV - type and semantic of value (deducible)                  
   ///   @param key - the key                                                 
   ///   @param value - the value                                             
   template<CT::Semantic SK, CT::Semantic SV>
   LANGULUS(ALWAYSINLINE)
   Pair::Pair(SK&& key, SV&& val)
      : mKey {key.Forward()}
      , mValue {val.Forward()} {}

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   LANGULUS(ALWAYSINLINE)
   Hash Pair::GetHash() const {
      return HashData(mKey, mValue);
   }

   /// Get the type of the contained key                                      
   ///   @return the key type                                                 
   LANGULUS(ALWAYSINLINE)
   DMeta Pair::GetKeyType() const noexcept {
      return mKey.GetType();
   }

   /// Get the type of the contained value                                    
   ///   @return the value type                                               
   LANGULUS(ALWAYSINLINE)
   DMeta Pair::GetValueType() const noexcept {
      return mValue.GetType();
   }

} // namespace Langulus::Anyness
