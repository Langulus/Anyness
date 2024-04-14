///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Pair.hpp"


namespace Langulus::Anyness
{

   /// Construct pair from any other kind of pair                             
   ///   @param other - the pair to construct with                            
   template<class P> requires CT::Pair<Desem<P>> LANGULUS(INLINED)
   Pair::Pair(P&& other)
      : mKey   {SemanticOf<decltype(other)> {DesemCast(other).mKey}}
      , mValue {SemanticOf<decltype(other)> {DesemCast(other).mValue}} {}

   /// Construct pair manually                                                
   ///   @param key - the key                                                 
   ///   @param value - the value                                             
   template<class K, class V>
   requires CT::UnfoldInsertable<K, V> LANGULUS(INLINED)
   Pair::Pair(K&& key, V&& val)
      : mKey   {Forward<decltype(key)>(key)}
      , mValue {Forward<decltype(val)>(val)} {}

   /// Assign any kind of pair                                                
   ///   @param rhs - the pair to assign                                      
   template<class P> requires CT::Pair<Desem<P>> LANGULUS(INLINED)
   Pair& Pair::operator = (P&& rhs) {
      mKey   = SemanticOf<decltype(rhs)> {DesemCast(rhs).mKey};
      mValue = SemanticOf<decltype(rhs)> {DesemCast(rhs).mValue};
      return *this;
   }

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   LANGULUS(INLINED)
   Hash Pair::GetHash() const {
      return HashOf(mKey, mValue);
   }

   /// Get the type of the contained key                                      
   ///   @return the key type                                                 
   LANGULUS(INLINED)
   DMeta Pair::GetKeyType() const noexcept {
      return mKey.GetType();
   }

   /// Get the type of the contained value                                    
   ///   @return the value type                                               
   LANGULUS(INLINED)
   DMeta Pair::GetValueType() const noexcept {
      return mValue.GetType();
   }

} // namespace Langulus::Anyness
