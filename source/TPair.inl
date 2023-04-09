///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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

   /// Initialize manually by reference or pointer                            
   ///   @param key - the key to use                                          
   ///   @param value - the value to use                                      
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr PAIR()::TPair(K key, V value) noexcept requires (!CT::Decayed<K, V>)
      : mKey {key}
      , mValue {value} {}

   /// Initialize manually by a move (noexcept)                               
   ///   @param key - the key to use                                          
   ///   @param value - the value to use                                      
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr PAIR()::TPair(K&& key, V&& value) requires (CT::Decayed<K, V> && CT::MoveMakable<K, V>)
      : mKey {Forward<K>(key)}
      , mValue {Forward<V>(value)} {}

   /// Initialize manually by a shallow-copy                                  
   ///   @param key - the key to use                                          
   ///   @param value - the value to use                                      
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr PAIR()::TPair(const K& key, const V& value) requires (CT::Decayed<K, V> && CT::CopyMakable<K, V>)
      : mKey {key}
      , mValue {value} {}

   /// Swap (noexcept)                                                        
   ///   @param other - the pair to swap with                                 
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr void PAIR()::Swap(TPair& other) noexcept requires CT::SwappableNoexcept<K, V> {
      ::std::swap(mKey, other.mKey);
      ::std::swap(mValue, other.mValue);
   }

   /// Swap                                                                   
   ///   @param other - the pair to swap with                                 
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr void PAIR()::Swap(TPair& other) requires CT::Swappable<K, V> {
      ::std::swap(mKey, other.mKey);
      ::std::swap(mValue, other.mValue);
   }

   /// Comparison                                                             
   ///   @param rhs - pair to compare against                                 
   ///   @return true if pairs match                                          
   TEMPLATE()
   LANGULUS(INLINED)
   bool PAIR()::operator == (const TPair& rhs) const {
      return mKey == rhs.mKey && mValue == rhs.mValue;
   }

   /// Clone the pair                                                         
   ///   @return a cloned pair                                                
   TEMPLATE()
   LANGULUS(INLINED)
   PAIR() PAIR()::Clone() const {
      return {Langulus::Clone(mKey), Langulus::Clone(mValue)};
   }

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   TEMPLATE()
   LANGULUS(INLINED)
   Hash PAIR()::GetHash() const {
      return HashData(mKey, mValue);
   }

   /// Get the type of the contained key                                      
   ///   @return the key type                                                 
   TEMPLATE()
   LANGULUS(INLINED)
   DMeta PAIR()::GetKeyType() const noexcept {
      return MetaData::Of<K>();
   }

   /// Get the type of the contained value                                    
   ///   @return the value type                                               
   TEMPLATE()
   LANGULUS(INLINED)
   DMeta PAIR()::GetValueType() const noexcept {
      return MetaData::Of<V>();
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef PAIR

