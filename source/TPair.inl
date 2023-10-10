///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TPair.hpp"

#define TEMPLATE() template<CT::Data K, CT::Data V>
#define PAIR() TPair<K, V>


namespace Langulus::Anyness
{

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(K key, V val) requires (not CT::Decayed<K, V>)
      : mKey {key}
      , mValue {val} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(const K& key, const V& val)
   requires (CT::Decayed<K, V> and CT::CopyMakable<K, V>)
      : TPair {Copy(key), Copy(val)} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(const K& key, V&& val)
   requires (CT::Decayed<K, V> and CT::CopyMakable<K> and CT::MoveMakable<V>)
      : TPair {Copy(key), Move(val)} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(const K& key, CT::Semantic auto&& val)
   requires (CT::Decayed<K, V> and CT::CopyMakable<K>)
      : TPair {Copy(key), val.Forward()} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(K&& key, const V& val)
   requires (CT::Decayed<K, V> and CT::MoveMakable<K> and CT::CopyMakable<V>)
      : TPair {Move(key), Copy(val)} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(K&& key, V&& val)
   requires (CT::Decayed<K, V> and CT::MoveMakable<K, V>)
      : TPair {Move(key), Move(val)} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(K&& key, CT::Semantic auto&& val)
   requires (CT::Decayed<K, V> and CT::MoveMakable<K>)
      : TPair {Move(key), val.Forward()} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(CT::Semantic auto&& key, const V& val)
   requires (CT::Decayed<K, V> and CT::CopyMakable<V>)
      : TPair {key.Forward(), Copy(val)} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(CT::Semantic auto&& key, V&& val)
   requires (CT::Decayed<K, V> and CT::MoveMakable<V>)
      : TPair {key.Forward(), Move(val)} {}

   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(CT::Semantic auto&& key, CT::Semantic auto&& val) requires CT::Decayed<K, V>
      : mKey {key.Forward()}
      , mValue {val.Forward()} {}

   /// Swap (noexcept)                                                        
   ///   @param other - the pair to swap with                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr void PAIR()::Swap(TPair& other) noexcept
   requires CT::SwappableNoexcept<K, V> {
      ::std::swap(mKey, other.mKey);
      ::std::swap(mValue, other.mValue);
   }

   /// Swap                                                                   
   ///   @param other - the pair to swap with                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr void PAIR()::Swap(TPair& other) requires CT::Swappable<K, V> {
      ::std::swap(mKey, other.mKey);
      ::std::swap(mValue, other.mValue);
   }

   /// Comparison                                                             
   ///   @param rhs - pair to compare against                                 
   ///   @return true if pairs match                                          
   TEMPLATE() LANGULUS(INLINED)
   bool PAIR()::operator == (const TPair& rhs) const {
      return mKey == rhs.mKey and mValue == rhs.mValue;
   }

   /// Clone the pair                                                         
   ///   @return a cloned pair                                                
   TEMPLATE() LANGULUS(INLINED)
   PAIR() PAIR()::Clone() const {
      return {Langulus::Clone(mKey), Langulus::Clone(mValue)};
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr PAIR()::operator TPair<const Deref<K>&, const Deref<V>&>() const noexcept {
      return {mKey, mValue};
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr PAIR()::operator TPair<Deref<K>&, Deref<V>&>() const noexcept
   requires CT::Mutable<K, V> {
      return {mKey, mValue};
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr PAIR()::operator TPair<const Deref<K>&, Deref<V>&>() const noexcept
   requires CT::Mutable<V> {
      return {mKey, mValue};
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr PAIR()::operator TPair<Deref<K>&, const Deref<V>&>() const noexcept
   requires CT::Mutable<K> {
      return {mKey, mValue};
   }

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   TEMPLATE() LANGULUS(INLINED)
   Hash PAIR()::GetHash() const {
      return HashOf(mKey, mValue);
   }

   /// Get the type of the contained key                                      
   ///   @return the key type                                                 
   TEMPLATE() LANGULUS(INLINED)
   DMeta PAIR()::GetKeyType() const noexcept {
      return MetaData::Of<K>();
   }

   /// Get the type of the contained value                                    
   ///   @return the value type                                               
   TEMPLATE() LANGULUS(INLINED)
   DMeta PAIR()::GetValueType() const noexcept {
      return MetaData::Of<V>();
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef PAIR

