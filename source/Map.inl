///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Map.hpp"
#include "Pair.inl"
#include "blocks/BlockMap/BlockMap-Construct.inl"
#include "blocks/BlockMap/BlockMap-Capsulation.inl"
#include "blocks/BlockMap/BlockMap-Indexing.inl"
#include "blocks/BlockMap/BlockMap-RTTI.inl"
#include "blocks/BlockMap/BlockMap-Compare.inl"
#include "blocks/BlockMap/BlockMap-Memory.inl"
#include "blocks/BlockMap/BlockMap-Insert.inl"
#include "blocks/BlockMap/BlockMap-Remove.inl"
#include "blocks/BlockMap/BlockMap-Iteration.inl"

#define TEMPLATE() template<bool ORDERED>
#define TABLE() Map<ORDERED>


namespace Langulus::Anyness
{

   /// Shallow-copy constructor                                               
   ///   @param other - the container to shallow-copy                         
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::Map(const Map& other)
      : Map {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - the container to move                                 
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::Map(Map&& other)
      : Map {Move(other)} {}

   /// Unfold constructor                                                     
   /// If there's one map argument, it will be absorbed                       
   /// Otherwise, elements are expected to initialize a pair each             
   ///   @param t1 - first element (can be semantic)                          
   ///   @param tail... - the rest of the elements (optional, can be semantic)
   TEMPLATE() template<class T1, class...TAIL>
   requires CT::Inner::UnfoldInsertable<T1, TAIL...>
   LANGULUS(INLINED) TABLE()::Map(T1&& t1, TAIL&&...tail) {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using T = TypeOf<S>;

         if constexpr (CT::Map<T>)
            BlockMap::BlockTransfer<Map>(S::Nest(t1));
         else
            BlockMap::InsertPair<Map>(Forward<T1>(t1));
      }
      else BlockMap::InsertPair<Map>(Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Map destructor                                                         
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::~Map() {
      Free<Map>();
   }

   /// Copy assignment                                                        
   ///   @param rhs - unordered map to shallow-copy                           
   ///   @return a reference to this map                                      
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const Map& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - unordered map to move over                              
   ///   @return a reference to this map                                      
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (Map&& rhs) {
      return operator = (Move(rhs));
   }

   /// Pair/map assignment, semantic or not                                   
   ///   @param rhs - the pair, or map to assign                              
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         // Potentially absorb a container                              
         if (static_cast<const BlockMap*>(this)
          == static_cast<const BlockMap*>(&DesemCast(rhs)))
            return *this;

         BlockMap::Free<Map>();
         new (this) Map {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         BlockMap::Clear<Map>();
         BlockMap::UnfoldInsert<Map>(S::Nest(rhs));
      }

      return *this;
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::begin() noexcept {
      return BlockMap::begin<Map>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::last() noexcept {
      return BlockMap::last<Map>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::begin() const noexcept {
      return BlockMap::begin<Map>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::last() const noexcept {
      return BlockMap::last<Map>();
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEach(auto&& call) const {
      return BlockMap::ForEach<REVERSE, const Map>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEach(auto&& call) {
      return BlockMap::ForEach<REVERSE, Map>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(auto&& call) const {
      return BlockMap::ForEachKeyElement<REVERSE, const Map>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(auto&& call) {
      return BlockMap::ForEachKeyElement<REVERSE, Map>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(auto&& call) const {
      return BlockMap::ForEachValueElement<REVERSE, const Map>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(auto&& call) {
      return BlockMap::ForEachValueElement<REVERSE, Map>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachKey(auto&&...call) const {
      return BlockMap::ForEachKey<REVERSE, const Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachKey(auto&&...call) {
      return BlockMap::ForEachKey<REVERSE, Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachValue(auto&&...call) const {
      return BlockMap::ForEachValue<REVERSE, const Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachValue(auto&&...call) {
      return BlockMap::ForEachValue<REVERSE, Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TABLE()::ForEachKeyDeep(auto&&...call) const {
      return BlockMap::ForEachKeyDeep<REVERSE, SKIP, const Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TABLE()::ForEachKeyDeep(auto&&...call) {
      return BlockMap::ForEachKeyDeep<REVERSE, SKIP, Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TABLE()::ForEachValueDeep(auto&&...call) const {
      return BlockMap::ForEachValueDeep<REVERSE, SKIP, const Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TABLE()::ForEachValueDeep(auto&&...call) {
      return BlockMap::ForEachValueDeep<REVERSE, SKIP, Map>(
         Forward<Deref<decltype(call)>>(call)...);
   }
   
   /// Check if key origin type matches any of the list                       
   ///   @tparam K1, KN... - the list of types to compare against             
   ///   @return true if key type matches at least one of the others          
   TEMPLATE() template<CT::Data K1, CT::Data...KN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsKey() const noexcept {
      return BlockMap::IsKey<Map, K1, KN...>();
   }

   /// Check if key origin type matches another                               
   ///   @param key - the key type to compare against                         
   ///   @return true if key matches the contained key origin type            
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsKey(DMeta key) const noexcept {
      return BlockMap::IsKey<Map>(key);
   }

   /// Check if cv-unqualified key type matches any of the list               
   ///   @tparam K1, KN... - the list of types to compare against             
   ///   @return true if key type matches at least one of the others          
   TEMPLATE() template<CT::Data K1, CT::Data...KN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeySimilar() const noexcept {
      return BlockMap::IsKeySimilar<Map, K1, KN...>();
   }

   /// Check if cv-unqualified key type matches another                       
   ///   @param key - the key type to compare against                         
   ///   @return true if key matches the contained key unqualified type       
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsKeySimilar(DMeta key) const noexcept {
      return BlockMap::IsKeySimilar<Map>(key);
   }

   /// Check if key type exactly matches any of the list                      
   ///   @tparam K1, KN... - the list of types to compare against             
   ///   @return true if key type matches at least one of the others          
   TEMPLATE() template<CT::Data K1, CT::Data...KN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyExact() const noexcept {
      return BlockMap::IsKeyExact<Map, K1, KN...>();
   }

   /// Check if key type exactly matches any of the list                      
   ///   @param key - the key type to compare against                         
   ///   @return true if key matches the contained key unqualified type       
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsKeyExact(DMeta key) const noexcept {
      return BlockMap::IsKeyExact<Map>(key);
   }

   /// Check if value origin type matches any of the list                     
   ///   @tparam V1, VN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data V1, CT::Data...VN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsValue() const noexcept {
      return BlockMap::IsValue<Map, V1, VN...>();
   }

   /// Check if value origin type matches another                             
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsValue(DMeta value) const noexcept {
      return BlockMap::IsValue<Map>(value);
   }

   /// Check if cv-unqualified value type matches any of the list             
   ///   @tparam V1, VN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data V1, CT::Data...VN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueSimilar() const noexcept {
      return BlockMap::IsValueSimilar<Map, V1, VN...>();
   }

   /// Check if cv-unqualified value type matches another                     
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained value unqualified type   
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsValueSimilar(DMeta value) const noexcept {
      return BlockMap::IsValueSimilar<Map>(value);
   }

   /// Check if value type exactly matches any of the list                    
   ///   @tparam V1, VN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data V1, CT::Data...VN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueExact() const noexcept {
      return BlockMap::IsValueExact<Map, V1, VN...>();
   }

   /// Check if value type exactly matches any of the list                    
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained value unqualified type   
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsValueExact(DMeta value) const noexcept {
      return BlockMap::IsValueExact<Map>(value);
   }

   /// Compare this map against another map, type-erased or not               
   ///   @param rhs - map to compare against                                  
   ///   @return true if contents of both maps are the same                   
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (CT::Map auto const& rhs) const {
      return BlockMap::operator == <Map> (rhs);
   }

   /// Compare this map against a pair, type-erased or not                    
   ///   @param rhs - pair to compare against                                 
   ///   @return true this map contains only this exact pair                  
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (CT::Pair auto const& rhs) const {
      return BlockMap::operator == <Map> (rhs);
   }

   /// Hash the contents of map                                               
   ///   @attention hashing is slow, it is recommended to cache the value     
   ///   @return the cache                                                    
   TEMPLATE() LANGULUS(INLINED)
   Hash TABLE()::GetHash() const {
      return BlockMap::GetHash<Map>();
   }

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsKey(const CT::NotSemantic auto& key) const {
      return BlockMap::ContainsKey<Map>(key);
   }

   /// Search for a value inside the table                                    
   ///   @param val - the value to search for                                 
   ///   @return true if value is found, false otherwise                      
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsValue(const CT::NotSemantic auto& val) const {
      return BlockMap::ContainsValue<Map>(val);
   }

   /// Search for a pair inside the table                                     
   ///   @param pair - the pair to search for                                 
   ///   @return true if pair is found, false otherwise                       
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsPair(const CT::Pair auto& pair) const {
      return BlockMap::ContainsPair<Map>(pair);
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TEMPLATE() LANGULUS(INLINED)
   Index TABLE()::Find(const CT::NotSemantic auto& key) const {
      return BlockMap::Find<Map>(key);
   }

   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::FindIt(const CT::NotSemantic auto& key) {
      return BlockMap::FindIt<Map>(key);
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::FindIt(const CT::NotSemantic auto& key) const {
      return BlockMap::FindIt<Map>(key);
   }

   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const CT::NotSemantic auto& key) {
      return BlockMap::At<Map>(key);
   }

   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const CT::NotSemantic auto& key) const {
      return BlockMap::At<Map>(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const CT::NotSemantic auto& key) {
      return BlockMap::operator [] <Map>(key);
   }

   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const CT::NotSemantic auto& key) const {
      return BlockMap::operator [] <Map>(key);
   }

   /// Insert a pair                                                          
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (CT::Inner::UnfoldInsertable auto&& other) {
      BlockMap::UnfoldInsert<Map>(Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Insert a pair                                                          
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator >> (CT::Inner::UnfoldInsertable auto&& other) {
      BlockMap::UnfoldInsert<Map>(Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Erase a pair via key                                                   
   ///   @param key - the key to search for                                   
   ///   @return 1 if key was found and pair was removed                      
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemoveKey(const CT::NotSemantic auto& key) {
      return BlockMap::RemoveKey<Map>(key);
   }

   /// Erase all pairs with a given value                                     
   ///   @param value - the match to search for                               
   ///   @return the number of removed pairs                                  
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemoveValue(const CT::NotSemantic auto& value) {
      return BlockMap::RemoveValue<Map>(value);
   }
     
   /// Erase all pairs matching a pair                                        
   ///   @param value - the match to search for                               
   ///   @return the number of removed pairs                                  
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemovePair(const CT::Pair auto& pair) {
      return BlockMap::RemovePair<Map>(pair);
   }
     
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this map instance         
   ///   @attention assumes that iterator points to a valid entry             
   ///   @param index - the index to remove                                   
   ///   @return the iterator of the previous element, unless index is the    
   ///           first, or at the end already                                 
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::RemoveIt(const Iterator& index) {
      return BlockMap::RemoveIt<Map>(index);
   }

   /// Destroy all contained pairs, but don't deallocate                      
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Clear() {
      return BlockMap::Clear<Map>();
   }

   /// Destroy all contained pairs and deallocate                             
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Reset() {
      return BlockMap::Reset<Map>();
   }

   /// Reduce reserved size, depending on number of contained elements        
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Compact() {
      return BlockMap::Compact<Map>();
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TABLE