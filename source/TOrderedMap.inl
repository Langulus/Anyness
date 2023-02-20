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

   /// Copy-constructor                                                       
   ///   @param other - the map to copy                                       
   TEMPLATE()
   MAP()::TOrderedMap(const TOrderedMap& other)
      : Self {::Langulus::Copy(other)} { }

   /// Move-constructor                                                       
   ///   @param other - the map to move                                       
   TEMPLATE()
   MAP()::TOrderedMap(TOrderedMap&& other) noexcept
      : Self {::Langulus::Move(other)} { }

   /// Copy assignment                                                        
   ///   @param rhs - the map to copy                                         
   TEMPLATE()
   MAP()& MAP()::operator = (const TOrderedMap& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the map to move                                         
   TEMPLATE()
   MAP()& MAP()::operator = (TOrderedMap&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Semantic assignment for an ordered map                                 
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the ordered map to use for construction                 
   TEMPLATE()
   template<CT::Semantic S>
   MAP()& MAP()::operator = (S&& rhs) noexcept requires (CT::Exact<TypeOf<S>, Self>) {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) Self {rhs.Forward()};
      return *this;
   }
   
   /// Copy assign a pair                                                     
   ///   @param rhs - the pair to copy                                        
   TEMPLATE()
   MAP()& MAP()::operator = (const Pair& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assign a pair                                                     
   ///   @param rhs - the pair to move                                        
   TEMPLATE()
   MAP()& MAP()::operator = (Pair&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Semantic assignment for a pair                                         
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the pair to use                                         
   TEMPLATE()
   template<CT::Semantic S>
   MAP()& MAP()::operator = (S&& rhs) noexcept requires (CT::Pair<TypeOf<S>>) {
      Clear();
      Insert(S::Nest(rhs.mValue.mKey), S::Nest(rhs.mValue.mValue));
      return *this;
   }
   
   /// Insert a single pair inside table via copy                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Count MAP()::Insert(const K& key, const V& value) {
      return Insert(Langulus::Copy(key), Langulus::Copy(value));
   }

   /// Insert a single pair inside table via key copy and value move          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Count MAP()::Insert(const K& key, V&& value) {
      return Insert(Langulus::Copy(key), Langulus::Move(value));
   }

   /// Insert a single pair inside table via key move and value copy          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Count MAP()::Insert(K&& key, const V& value) {
      return Insert(Langulus::Move(key), Langulus::Copy(value));
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Count MAP()::Insert(K&& key, V&& value) {
      return Insert(Langulus::Move(key), Langulus::Move(value));
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE()
   template<CT::Semantic SK, CT::Semantic SV>
   LANGULUS(ALWAYSINLINE)
   Count MAP()::Insert(SK&& key, SV&& value) noexcept requires (CT::Exact<TypeOf<SK>, K> && CT::Exact<TypeOf<SV>, V>) {
      Allocate(GetCount() + 1);
      Base::InsertInner<true>(
         Base::GetBucket(key.mValue), 
         key.Forward(), 
         value.Forward()
      );
      return 1;
   }
   
   /// Copy-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TEMPLATE()
   MAP()& MAP()::operator << (const TPair<K, V>& rhs) {
      return operator << (Langulus::Copy(rhs));
   }

   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TEMPLATE()
   MAP()& MAP()::operator << (TPair<K, V>&& rhs) {
      return operator << (Langulus::Move(rhs));
   }
   
   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TEMPLATE()
   template<CT::Semantic S>
   MAP()& MAP()::operator << (S&& rhs) noexcept requires (CT::Pair<TypeOf<S>>) {
      Insert(S::Nest(rhs.mValue.mKey), S::Nest(rhs.mValue.mValue));
      return *this;
   }

} // namespace Langulus::Anyness

#undef MAP
#undef TEMPLATE

   
