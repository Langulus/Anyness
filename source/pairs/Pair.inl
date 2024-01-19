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
#include "../many/Any.inl"


namespace Langulus::Anyness
{

   /// Construct pair from any other kind of pair                             
   ///   @param other - the pair to construct with                            
   template<class P> requires CT::Pair<Desem<P>> LANGULUS(INLINED)
   Pair::Pair(P&& other)
      : mKey   {SemanticOf<P> {DesemCast(other).mKey}}
      , mValue {SemanticOf<P> {DesemCast(other).mValue}} {}

   /// Construct pair manually                                                
   ///   @param key - the key                                                 
   ///   @param value - the value                                             
   template<class K, class V>
   requires CT::Inner::UnfoldInsertable<K, V> LANGULUS(INLINED)
   Pair::Pair(K&& key, V&& value)
      : mKey   {Forward<K>(key)}
      , mValue {Forward<V>(value)} {}

   /// Assign any kind of pair                                                
   ///   @param rhs - the pair to assign                                      
   template<class P> requires CT::Pair<Desem<P>> LANGULUS(INLINED)
   Pair& Pair::operator = (P&& rhs) {
      mKey   = SemanticOf<P> {DesemCast(rhs).mKey};
      mValue = SemanticOf<P> {DesemCast(rhs).mValue};
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
