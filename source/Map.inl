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
            BlockTransfer<Map>(S::Nest(t1));
         else
            InsertPair<Map>(Forward<T1>(t1));
      }
      else InsertPair<Map>(Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Semantic constructor from any map/pair                                 
   ///   @param other - the semantic type and map/pair to initialize with     
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::Map(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Array<T>) {
         if constexpr (CT::Pair<Deext<T>>) {
            // Construct from an array of pairs                         
            mKeys.mType = (*other)[0].GetKeyType();
            mValues.mType = (*other)[0].GetValueType();

            constexpr auto reserved = Roof2(ExtentOf<T>);
            AllocateFresh(reserved);
            ZeroMemory(mInfo, reserved);
            mInfo[reserved] = 1;

            constexpr auto hashmask = reserved - 1;
            for (auto& pair : *other)
               InsertPairInner<true, false>(hashmask, S::Nest(pair));
         }
         else LANGULUS_ERROR("Unsupported semantic array constructor");

         //TODO perhaps constructor from map array, by merging them?
      }
      else if constexpr (CT::Map<T>) {
         // Construct from any kind of map                              
         if constexpr (T::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
            mKeys.mType = other->GetKeyType();
            mValues.mType = other->GetValueType();

            AllocateFresh(other->GetReserved());
            ZeroMemory(mInfo, GetReserved());
            mInfo[GetReserved()] = 1;

            const auto hashmask = GetReserved() - 1;
            using TP = typename T::Pair;
            other->ForEach([this, hashmask](TP& pair) {
               InsertPairInner<false, false>(hashmask, S::Nest(pair));
            });
         }
         else {
            // We can directly interface map, because it is unordered   
            // and uses the same bucketing approach                     
            BlockTransfer<UnorderedMap>(other.Forward());
         }
      }
      else if constexpr (CT::Pair<T>) {
         // Construct from any kind of pair                             
         mKeys.mType = other->GetKeyType();
         mValues.mType = other->GetValueType();

         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         constexpr auto hashmask = MinimalAllocation - 1;
         InsertPairInner<false, false>(hashmask, other.Forward());
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
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

         Free<Map>();
         new (this) Map {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Clear();
         UnfoldInsert<Map>(S::Nest(rhs));
      }

      return *this;
   }

   /// Insert a pair                                                          
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (CT::Inner::UnfoldInsertable auto&& other) {
      InsertPair<Map>(Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Insert a pair                                                          
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator >> (CT::Inner::UnfoldInsertable auto&& other) {
      InsertPair<Map>(Forward<decltype(other)>(other));
      return *this;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TABLE