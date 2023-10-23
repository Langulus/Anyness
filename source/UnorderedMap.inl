///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedMap.hpp"
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


namespace Langulus::Anyness
{

   /// Copy constructor                                                       
   ///   @param other - map to shallow-copy                                   
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(const UnorderedMap& other)
      : UnorderedMap {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - map to move                                           
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(UnorderedMap&& other)
      : UnorderedMap {Move(other)} {}

   /// Copy constructor from any map/pair                                     
   ///   @param other - the semantic type                                     
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(const CT::NotSemantic auto& other)
      : UnorderedMap {Copy(other)} {}
   
   /// Copy constructor from any map/pair                                     
   ///   @param other - the semantic type                                     
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(CT::NotSemantic auto& other)
      : UnorderedMap {Copy(other)} {}
   
   /// Move constructor from any map/pair                                     
   ///   @param other - the semantic type                                     
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(CT::NotSemantic auto&& other)
      : UnorderedMap {Move(other)} {}

   /// Semantic constructor from any map/pair                                 
   ///   @param other - the semantic type and map/pair to initialize with     
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(CT::Semantic auto&& other) {
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
   
   /// Create from a list of pairs                                            
   ///   @param head - first pair                                             
   ///   @param tail - tail of pairs                                          
   template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
   UnorderedMap::UnorderedMap(T1&& t1, T2&& t2, TAIL&&... tail) {
      if constexpr (CT::Semantic<T1>) {
         if constexpr (CT::Pair<TypeOf<T1>>) {
            mKeys.mType = t1->GetKeyType();
            mValues.mType = t1->GetValueType();
         }
         else LANGULUS_ERROR("T inside semantic is not a Pair");
      }
      else {
         if constexpr (CT::Pair<T1>) {
            mKeys.mType = t1.GetKeyType();
            mValues.mType = t1.GetValueType();
         }
         else LANGULUS_ERROR("T is not a Pair");
      }

      constexpr auto capacity = Roof2(
         sizeof...(TAIL) + 2 < MinimalAllocation
            ? MinimalAllocation
            : sizeof...(TAIL) + 2
      );

      AllocateFresh(capacity);
      ZeroMemory(mInfo, capacity);
      mInfo[capacity] = 1;

      Insert(Forward<T1>(t1));
      Insert(Forward<T2>(t2));
      (Insert(Forward<TAIL>(tail)), ...);
   }

   /// Map destructor                                                         
   LANGULUS(INLINED)
   UnorderedMap::~UnorderedMap() {
      Free<UnorderedMap>();
   }

   /// Copy assignment                                                        
   ///   @param rhs - unordered map to shallow-copy                           
   ///   @return a reference to this map                                      
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (const UnorderedMap& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - unordered map to move over                              
   ///   @return a reference to this map                                      
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (UnorderedMap&& rhs) {
      return operator = (Move(rhs));
   }

   /// Copy assignment from any map/pair                                      
   ///   @param rhs - the semantic type and map/pair to assign                
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (const CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }
   
   /// Copy assignment from any map/pair                                      
   ///   @param rhs - the semantic type and map/pair to assign                
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }
   
   /// Move assignment from any map/pair                                      
   ///   @param rhs - the semantic type and map/pair to assign                
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (CT::NotSemantic auto&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantic assignment from any map/pair                                  
   ///   @param other - the semantic type and map/pair to assign              
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         // Assign any kind of map                                      
         if (static_cast<const BlockMap*>(this)
          == static_cast<const BlockMap*>(&*other))
            return *this;

         Free<UnorderedMap>();
         new (this) UnorderedMap {other.Forward()};
      }
      else if constexpr (CT::Pair<T>) {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Free<UnorderedMap>();
            new (this) UnorderedMap {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Clear<UnorderedMap>();
            InsertPairInner<false, false>(
               GetReserved() - 1, other.Forward());
         }
      }
      else LANGULUS_ERROR("Unsupported unordered map assignment");

      return *this;
   }

   /// Insert any kind of pair                                                
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (auto&& pair) {
      InsertPair(Forward<decltype(pair)>(pair));
      return *this;
   }

} // namespace Langulus::Anyness
