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
      : mKey   {SemanticOf<decltype(other)>::Nest(DesemCast(other).mKey)}
      , mValue {SemanticOf<decltype(other)>::Nest(DesemCast(other).mValue)} {}

   /// Construct pair manually                                                
   ///   @param key - the key                                                 
   ///   @param value - the value                                             
   template<class K, class V>
   requires CT::UnfoldInsertable<K, V> LANGULUS(INLINED)
   Pair::Pair(K&& key, V&& val)
      : mKey   {Forward<K>(key)}
      , mValue {Forward<V>(val)} {}

   /// Assign any kind of pair                                                
   ///   @param rhs - the pair to assign                                      
   template<class P> requires CT::Pair<Desem<P>> LANGULUS(INLINED)
   Pair& Pair::operator = (P&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      mKey   = S::Nest(DesemCast(rhs).mKey);
      mValue = S::Nest(DesemCast(rhs).mValue);
      return *this;
   }

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   LANGULUS(INLINED)
   Hash Pair::GetHash() const {
      return HashOf(mKey, mValue);
   }

   /// Get the contained key                                                  
   ///   @return a reference to the contained key                             
   LANGULUS(INLINED)
   Many const& Pair::GetKey() const noexcept {
      return mKey;
   }

   LANGULUS(INLINED)
   Many& Pair::GetKey() noexcept {
      return mKey;
   }

   /// Get the contained value                                                
   ///   @return a reference to the contained key                             
   LANGULUS(INLINED)
   Many const& Pair::GetValue() const noexcept {
      return mValue;
   }

   LANGULUS(INLINED)
   Many& Pair::GetValue() noexcept {
      return mValue;
   }

   LANGULUS(INLINED)
   bool Pair::operator == (CT::Pair auto const& rhs) const {
      return mKey == rhs.mKey and mValue == rhs.mValue;
   }

   /// Clear anything contained, but don't release memory                     
   LANGULUS(INLINED)
   void Pair::Clear() {
      mKey.Clear();
      mValue.Clear();
   }

   /// Clear and release memory                                               
   LANGULUS(INLINED)
   void Pair::Reset() {
      mKey.Reset();
      mValue.Reset();
   }

} // namespace Langulus::Anyness
