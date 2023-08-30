///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(const CT::NotSemantic auto& other)
      : UnorderedMap {Copy(other)} {}
   
   /// Copy constructor from any map/pair                                     
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(CT::NotSemantic auto& other)
      : UnorderedMap {Copy(other)} {}
   
   /// Move constructor from any map/pair                                     
   ///   @tparam S - semantic and type (deducible)                            
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

      if constexpr (CT::Map<T>) {
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
               InsertPairInner<UnorderedMap>(hashmask, S::Nest(pair));
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
         InsertPairInner<UnorderedMap>(hashmask, other.Forward());
      }
      else if constexpr (CT::Array<T>) {
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
               InsertPairInner<UnorderedMap>(hashmask, S::Nest(pair));
         }
         else LANGULUS_ERROR("Unsupported semantic array constructor");

         //TODO perhaps constructor from map array, by merging them?
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
   }
   
   /// Create from a list of pairs                                            
   ///   @param head - first pair                                             
   ///   @param tail - tail of pairs                                          
   template<CT::Data HEAD, CT::Data... TAIL>
   UnorderedMap::UnorderedMap(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1) {
      if constexpr (CT::Semantic<HEAD>) {
         if constexpr (CT::Pair<TypeOf<HEAD>>) {
            mKeys.mType = head->GetKeyType();
            mValues.mType = head->GetValueType();
         }
         else LANGULUS_ERROR("Type inside semantic is not a Pair");
      }
      else {
         if constexpr (CT::Pair<HEAD>) {
            mKeys.mType = head.GetKeyType();
            mValues.mType = head.GetValueType();
         }
         else LANGULUS_ERROR("Type is not a Pair");
      }

      constexpr auto capacity = Roof2(
         sizeof...(TAIL) + 1 < MinimalAllocation
            ? MinimalAllocation
            : sizeof...(TAIL) + 1
      );

      AllocateFresh(capacity);
      ZeroMemory(mInfo, capacity);
      mInfo[capacity] = 1;

      Insert(Forward<HEAD>(head));
      (Insert(Forward<TAIL>(tail)), ...);
   }

   /// Map destructor                                                         
   LANGULUS(INLINED)
   UnorderedMap::~UnorderedMap() {
      Free();
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
         if (static_cast<const BlockMap*>(this)
          == static_cast<const BlockMap*>(&*other))
            return *this;

         Free();
         new (this) UnorderedMap {other.Forward()};
      }
      else if constexpr (CT::Pair<T>) {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Free();
            new (this) UnorderedMap {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Clear();
            InsertPairInner<UnorderedMap>(GetReserved() - 1, other.Forward());
         }
      }
      else LANGULUS_ERROR("Unsupported unordered map assignment");

      return *this;
   }
   
   Count UnorderedMap::Insert(const CT::NotSemantic auto& k, const CT::NotSemantic auto& v) {
      return Insert(Copy(k), Copy(v));
   }

   Count UnorderedMap::Insert(const CT::NotSemantic auto& k, CT::NotSemantic auto& v) {
      return Insert(Copy(k), Copy(v));
   }

   Count UnorderedMap::Insert(const CT::NotSemantic auto& k, CT::NotSemantic auto&& v) {
      return Insert(Copy(k), Move(v));
   }

   Count UnorderedMap::Insert(const CT::NotSemantic auto& k, CT::Semantic auto&& v) {
      return Insert(Copy(k), v.Forward());
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto& k, const CT::NotSemantic auto& v) {
      return Insert(Copy(k), Copy(v));
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto& k, CT::NotSemantic auto& v) {
      return Insert(Copy(k), Copy(v));
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto& k, CT::NotSemantic auto&& v) {
      return Insert(Copy(k), Move(v));
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto& k, CT::Semantic auto&& v) {
      return Insert(Copy(k), v.Forward());
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto&& k, const CT::NotSemantic auto& v) {
      return Insert(Move(k), Copy(v));
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto&& k, CT::NotSemantic auto& v) {
      return Insert(Move(k), Copy(v));
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto&& k, CT::NotSemantic auto&& v) {
      return Insert(Move(k), Move(v));
   }

   Count UnorderedMap::Insert(CT::NotSemantic auto&& k, CT::Semantic auto&& v) {
      return Insert(Move(k), v.Forward());
   }

   Count UnorderedMap::Insert(CT::Semantic auto&& k, const CT::NotSemantic auto& v) {
      return Insert(k.Forward(), Copy(v));
   }

   Count UnorderedMap::Insert(CT::Semantic auto&& k, CT::NotSemantic auto& v) {
      return Insert(k.Forward(), Copy(v));
   }

   Count UnorderedMap::Insert(CT::Semantic auto&& k, CT::NotSemantic auto&& v) {
      return Insert(k.Forward(), Move(v));
   }
   
   /// Semantically insert key and value                                      
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted, zero otherwise                       
   Count UnorderedMap::Insert(CT::Semantic auto&& key, CT::Semantic auto&& value) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(value)>;
      using K = TypeOf<SK>;
      using V = TypeOf<SV>;

      Mutate<K, V>();
      Reserve(GetCount() + 1);
      InsertInner<true>(
         GetBucket(GetReserved() - 1, *key), 
         key.Forward(), value.Forward()
      );
      return 1;
   }
   
   /// Semantically insert any pair                                           
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   Count UnorderedMap::Insert(CT::Semantic auto&& pair) {
      using S = Decay<decltype(pair)>;
      using T = TypeOf<S>;
      static_assert(CT::Pair<T>, "T must be a pair");

      if constexpr (CT::TypedPair<T>)
         return Insert(S::Nest(pair->mKey), S::Nest(pair->mValue));
      else
         return InsertUnknown(S::Nest(pair->mKey), S::Nest(pair->mValue));
   }

   /// Copy-insert any pair inside the map                                    
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (const CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }

   /// Copy-insert any pair inside the map                                    
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }

   /// Move-insert any pair inside the map                                    
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (CT::NotSemantic auto&& item) {
      return operator << (Move(item));
   }

   /// Semantic insertion of any pair inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (CT::Semantic auto&& item) {
      Insert(item.Forward());
      return *this;
   }
   
   /// Semantically insert a type-erased pair                                 
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted or value was overwritten              
   LANGULUS(INLINED)
   Count UnorderedMap::InsertUnknown(CT::Semantic auto&& key, CT::Semantic auto&& val) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(val)>;

      static_assert(CT::Exact<TypeOf<SK>, Block>,
         "SK type must be exactly Block (build-time optimization)");
      static_assert(CT::Exact<TypeOf<SV>, Block>,
         "SV type must be exactly Block (build-time optimization)");

      Mutate(key->mType, val->mType);
      Reserve(GetCount() + 1);
      InsertInnerUnknown<true>(
         GetBucketUnknown(GetReserved() - 1, *key),
         key.Forward(), val.Forward()
      );
      return 1;
   }
   
   /// Semantically insert a type-erased pair                                 
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted or value was overwritten              
   LANGULUS(INLINED)
   Count UnorderedMap::InsertUnknown(CT::Semantic auto&& pair) {
      using S = Decay<decltype(pair)>;
      using T = TypeOf<S>;
      static_assert(CT::Pair<T> && !CT::TypedPair<T>,
         "SP's type must be type-erased pair type");

      return InsertUnknown(S::Nest(pair->mKey), S::Nest(pair->mValue));
   }
   
   /// Access value by key, or implicitly add the key if not found            
   /// Value will be default-constructed in the latter case                   
   ///   @param key - the key to search for                                   
   ///   @return the corresponding value block                                
   template<CT::NotSemantic K>
   Block UnorderedMap::At(const K& key) {
      const auto found = FindIndex<UnorderedMap>(key);
      if (found < GetReserved())
         return GetValueInner(found);

      // Key wasn't found, but map is mutable and we can add it         
      Mutate(MetaData::Of<K>(), mValues.mType);

      Any newk {key};
      Any newv = Any::FromMeta(mValues.mType, mValues.mState);
      newv.template AllocateMore<true>(1);
      Reserve(GetCount() + 1);

      // Insert the new pair                                            
      const auto insertedAt = InsertInnerUnknown<false>(
         GetBucket(GetReserved() - 1, key),
         Abandon(Forward<Block>(newk)), 
         Abandon(Forward<Block>(newv))
      );
      return GetValueInner(insertedAt);
   }

   /// Access value by key, or implicitly add the key if not found            
   /// Value will be default-constructed in the latter case                   
   ///   @param key - the key to find                                         
   ///   @return the corresponding value block                                
   template<CT::NotSemantic K>
   LANGULUS(INLINED)
   Block UnorderedMap::operator[] (const K& key) {
      return At(key);
   }

} // namespace Langulus::Anyness
