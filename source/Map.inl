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