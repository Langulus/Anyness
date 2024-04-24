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

   /// Implicit cast to constant pair                                         
   TEMPLATE() LANGULUS(INLINED)
   PAIR()::operator TPair<const Deref<K>&, const Deref<V>&>() const noexcept
   requires CT::Reference<K, V> {
      return {mKey, mValue};
   }

   /// Get contained key                                                      
   TEMPLATE() LANGULUS(INLINED)
   Block<K> PAIR()::GetKey() noexcept {
      if constexpr (CT::Reference<K> or CT::Dense<K>) {
         return Block {
            DataState::Member,
            MetaDataOf<Deref<K>>(), 1,
            &mKey
         };
      }
      else return mKey.GetBlock();
   }
   
   TEMPLATE() LANGULUS(INLINED)
   Block<K> PAIR()::GetKey() const noexcept {
      auto block = const_cast<PAIR()*>(this)->GetKey();
      block.MakeConst();
      return block;
   }

   /// Get contained value                                                    
   TEMPLATE() LANGULUS(INLINED)
   Block<V> PAIR()::GetValue() noexcept {
      if constexpr (CT::Reference<V> or CT::Dense<K>) {
         return Block {
            DataState::Member,
            MetaDataOf<Deref<V>>(), 1,
            &mValue
         };
      }
      else return mValue.GetBlock();
   }

   TEMPLATE() LANGULUS(INLINED)
   Block<V> PAIR()::GetValue() const noexcept {
      auto block = const_cast<PAIR()*>(this)->GetValue();
      block.MakeConst();
      return block;
   }

   TEMPLATE() LANGULUS(INLINED)
   Handle<K> PAIR()::GetKeyHandle() {
      if constexpr (CT::Sparse<K> and not CT::Reference<K>)
         return mKey.GetHandle();
      else
         return {mKey};
   }

   TEMPLATE() LANGULUS(INLINED)
   Handle<V> PAIR()::GetValueHandle() {
      if constexpr (CT::Sparse<V> and not CT::Reference<V>)
         return mValue.GetHandle();
      else
         return {mValue};
   }
   
   TEMPLATE() LANGULUS(INLINED)
   Handle<const K> PAIR()::GetKeyHandle() const {
      if constexpr (CT::Sparse<K> and not CT::Reference<K>)
         return mKey.GetHandle();
      else
         return {mKey};
   }

   TEMPLATE() LANGULUS(INLINED)
   Handle<const V> PAIR()::GetValueHandle() const {
      if constexpr (CT::Sparse<V> and not CT::Reference<V>)
         return mValue.GetHandle();
      else
         return {mValue};
   }

   /// Clear anything contained, but don't release memory                     
   TEMPLATE() LANGULUS(INLINED)
   void PAIR()::Clear() {
      if constexpr (CT::Sparse<K> and not CT::Reference<K>)
         mKey.Reset();
      if constexpr (CT::Sparse<V> and not CT::Reference<V>)
         mValue.Reset();
   }

   /// Clear and release memory                                               
   TEMPLATE() LANGULUS(INLINED)
   void PAIR()::Reset() {
      Clear();
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef PAIR

