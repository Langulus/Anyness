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

#define TEMPLATE() template<class K, class V>
#define PAIR() TPair<K, V>


namespace Langulus::Anyness
{

   /// Semantic constructor from any other pair (if K and V aren't references)
   ///   @param pair - the pair to use for initialization                     
   TEMPLATE() template<class P>
   requires CT::PairMakable<K, V, P> LANGULUS(INLINED)
   PAIR()::TPair(P&& pair)
      : mKey   {SemanticOf<decltype(pair)>::Nest(DesemCast(pair).mKey)}
      , mValue {SemanticOf<decltype(pair)>::Nest(DesemCast(pair).mValue)} {}

   /// Semantic constructor from key and value, if K and V aren't references, 
   /// not aggregate types                                                    
   ///   @param key - the key                                                 
   ///   @param val - the value                                               
   TEMPLATE() template<class K1, class V1>
   requires (CT::MakableFrom<K, K1> and CT::MakableFrom<V, V1>
   and CT::NotReference<K, V>) LANGULUS(INLINED)
   PAIR()::TPair(K1&& key, V1&& val)
      : mKey   {SemanticOf<decltype(key)>::Nest(key)}
      , mValue {SemanticOf<decltype(val)>::Nest(val)} {}

   /// Semantic constructor from key and value (if K and V are references)    
   ///   @param key - the key                                                 
   ///   @param val - the value                                               
   TEMPLATE() LANGULUS(INLINED)
   PAIR()::TPair(K&& key, V&& val) noexcept requires CT::Reference<K, V>
      : mKey   {Forward<K>(key)}
      , mValue {Forward<V>(val)} {}

   /// Semantic assignment from any other pair (if K and V aren't references) 
   ///   @param pair - the pair to assign                                     
   TEMPLATE() template<class P>
   requires CT::PairAssignable<K, V, P> LANGULUS(INLINED)
   PAIR()& PAIR()::operator = (P&& pair) {
      using S = SemanticOf<decltype(pair)>;
      mKey   = S::Nest(DesemCast(pair).mKey);
      mValue = S::Nest(DesemCast(pair).mValue);
      return *this;
   }

   /// Comparison                                                             
   ///   @param rhs - pair to compare against                                 
   ///   @return true if pairs match                                          
   TEMPLATE() template<class P>
   requires CT::PairComparable<K, V, P> LANGULUS(INLINED)
   bool PAIR()::operator == (const P& rhs) const {
      return mKey == rhs.mKey and mValue == rhs.mValue;
   }

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   TEMPLATE() LANGULUS(INLINED)
   Hash PAIR()::GetHash() const requires CT::Hashable<K, V> {
      return HashOf(mKey, mValue);
   }

   /// Get the type of the contained key                                      
   ///   @return the key type                                                 
   TEMPLATE() LANGULUS(INLINED)
   DMeta PAIR()::GetKeyType() const noexcept {
      return MetaDataOf<K>();
   }

   /// Get the type of the contained value                                    
   ///   @return the value type                                               
   TEMPLATE() LANGULUS(INLINED)
   DMeta PAIR()::GetValueType() const noexcept {
      return MetaDataOf<V>();
   }

   /// Implicit cast to constant pair                                         
   TEMPLATE() LANGULUS(INLINED)
   PAIR()::operator TPair<const Deref<K>&, const Deref<V>&>() const noexcept
   requires CT::Reference<K, V> {
      return {mKey, mValue};
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef PAIR

