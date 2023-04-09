///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedMap.hpp"

namespace Langulus::Anyness
{

   /// Default unordered map constructor                                      
   LANGULUS(INLINED)
   constexpr UnorderedMap::UnorderedMap()
      : BlockMap {} {}

   /// Create from a list of pairs                                            
   ///   @tparam P - the pair type                                            
   ///   @param list - list of pairs                                          
   template<CT::Pair P>
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(::std::initializer_list<P> list) {
      mKeys.mType = list.begin()->GetKeyType();
      mValues.mType = list.begin()->GetValueType();

      AllocateFresh(
         Roof2(
            list.size() < MinimalAllocation
               ? MinimalAllocation
               : list.size()
         )
      );
      const auto hashmask = GetReserved() - 1;

      ZeroMemory(mInfo, GetReserved());
      mInfo[GetReserved()] = 1;

      for (auto& it : list) {
         if constexpr (CT::TypedPair<P>) {
            // Insert a statically typed pair                           
            InsertInner<true>(
               GetBucket(it.mKey), 
               Copy(it.mKey),
               Copy(it.mValue)
            );
         }
         else {
            // Insert a dynamically typed pair                          
            InsertInnerUnknown<true>(
               it.mKey.GetHash().mHash & hashmask,
               Copy(it.mKey),
               Copy(it.mValue)
            );
         }
      }
   }

   /// Copy constructor                                                       
   ///   @param other - map to shallow-copy                                   
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(const UnorderedMap& other)
      : UnorderedMap {Langulus::Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - map to move                                           
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(UnorderedMap&& other) noexcept
      : UnorderedMap {Langulus::Move(other)} {}

   /// Semantic constructor from any map/pair                                 
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   template<CT::Semantic S>
   LANGULUS(INLINED)
   UnorderedMap::UnorderedMap(S&& other) noexcept {
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         // Construct from any kind of map                              
         if constexpr (T::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
            mKeys.mType = other.mValue.GetKeyType();
            mValues.mType = other.mValue.GetValueType();

            AllocateFresh(other.mValue.GetReserved());
            const auto hashmask = GetReserved() - 1;

            ZeroMemory(mInfo, GetReserved());
            mInfo[GetReserved()] = 1;

            using TP = typename T::Pair;
            other.mValue.ForEach([this, hashmask](TP& pair) {
               if constexpr (CT::TypedPair<TP>) {
                  // Insert a statically typed pair                     
                  InsertInner<false>(
                     GetBucket(pair.mKey), 
                     S::Nest(pair.mKey), 
                     S::Nest(pair.mValue)
                  );
               }
               else {
                  // Insert a dynamically typed pair                    
                  InsertInnerUnknown<false>(
                     pair.mKey.GetHash().mHash & hashmask,
                     S::Nest(pair.mKey), 
                     S::Nest(pair.mValue)
                  );
               }
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
         mKeys.mType = other.mValue.GetKeyType();
         mValues.mType = other.mValue.GetValueType();

         AllocateFresh(MinimalAllocation);
         constexpr auto hashmask = MinimalAllocation - 1;

         ZeroMemory(mInfo, GetReserved());
         mInfo[GetReserved()] = 1;

         if constexpr (CT::TypedPair<T>) {
            // Insert a statically typed pair                           
            InsertInner<false>(
               GetBucket(other.mValue.mKey),
               S::Nest(other.mValue.mKey),
               S::Nest(other.mValue.mValue)
            );
         }
         else {
            // Insert a dynamically typed pair                          
            InsertInnerUnknown<false>(
               other.mValue.mKey.GetHash().mHash & hashmask,
               S::Nest(other.mValue.mKey),
               S::Nest(other.mValue.mValue)
            );
         }
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
   }

   /// Copy assignment of a pair                                              
   ///   @param rhs - pair to copy-insert                                     
   ///   @return a reference to this map                                      
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (const CT::Pair auto& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assignment of a pair                                              
   ///   @param rhs - pair to move-insert                                     
   ///   @return a reference to this map                                      
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (CT::Pair auto&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Copy assignment                                                        
   ///   @param rhs - unordered map to copy-insert                            
   ///   @return a reference to this map                                      
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (const UnorderedMap& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - unordered map to move-insert                            
   ///   @return a reference to this map                                      
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (UnorderedMap&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Semantic assignment from any map/pair                                  
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   template<CT::Semantic S>
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator = (S&& other) noexcept {
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         if (static_cast<const BlockMap*>(this)
          == static_cast<const BlockMap*>(&other.mValue))
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
            const auto hashmask = GetReserved() - 1;

            if constexpr (CT::TypedPair<T>) {
               // Insert a statically typed pair                        
               InsertInner<false>(
                  GetBucket(other.mValue.mKey),
                  S::Nest(other.mValue.mKey),
                  S::Nest(other.mValue.mValue)
               );
            }
            else {
               // Insert a dynamically typed pair                       
               InsertInnerUnknown<false>(
                  other.mValue.mKey.GetHash().mHash & hashmask,
                  S::Nest(other.mValue.mKey),
                  S::Nest(other.mValue.mValue)
               );
            }
         }
      }
      else LANGULUS_ERROR("Unsupported unordered map assignment");

      return *this;
   }
   
   /// Insert a single pair inside table via copy                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count UnorderedMap::Insert(
      const CT::NotSemantic auto& key, 
      const CT::NotSemantic auto& value
   ) {
      return Insert(Copy(key), Copy(value));
   }

   /// Insert a single pair inside table via key copy and value move          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count UnorderedMap::Insert(
      const CT::NotSemantic auto& key, 
      CT::NotSemantic auto&& value
   ) {
      return Insert(Copy(key), Move(value));
   }

   /// Insert a single pair inside table via key move and value copy          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count UnorderedMap::Insert(
      CT::NotSemantic auto&& key, 
      const CT::NotSemantic auto& value
   ) {
      return Insert(Move(key), Copy(value));
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count UnorderedMap::Insert(
      CT::NotSemantic auto&& key, 
      CT::NotSemantic auto&& value
   ) {
      return Insert(Move(key), Move(value));
   }
   
   /// Semantically insert key and value                                      
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::Semantic SK, CT::Semantic SV>
   Count UnorderedMap::Insert(SK&& key, SV&& value) {
      using K = TypeOf<SK>;
      using V = TypeOf<SV>;

      Mutate<K, V>();
      Allocate(GetCount() + 1);
      InsertInner<true>(GetBucket(key.mValue), key.Forward(), value.Forward());
      return 1;
   }
   
   /// Semantically insert any pair                                           
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::Semantic S>
   Count UnorderedMap::Insert(S&& pair) {
      using T = TypeOf<S>;
      static_assert(CT::Pair<T>, "T must be a pair");

      if constexpr (CT::TypedPair<T>)
         return Insert(S::Nest(pair.mValue.mKey), S::Nest(pair.mValue.mValue));
      else
         return InsertUnknown(S::Nest(pair.mValue.mKey), S::Nest(pair.mValue.mValue));
   }

   /// Copy-insert any pair inside the map                                    
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (const CT::Pair auto& item) {
      return operator << (Copy(item));
   }

   /// Move-insert any pair inside the map                                    
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (CT::Pair auto&& item) {
      return operator << (Move(item));
   }

   /// Semantic insertion of any pair inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   template<CT::Semantic S>
   LANGULUS(INLINED)
   UnorderedMap& UnorderedMap::operator << (S&& item) {
      Insert(item.Forward());
      return *this;
   }
   
   /// Semantically insert a type-erased pair                                 
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted or value was overwritten              
   template<CT::Semantic SK, CT::Semantic SV>
   LANGULUS(INLINED)
   Count UnorderedMap::InsertUnknown(SK&& key, SV&& val) {
      static_assert(CT::Block<TypeOf<SK>>,
         "SK's type must be a block type");
      static_assert(CT::Block<TypeOf<SV>>,
         "SV's type must be a block type");

      Mutate(key.mValue.mType, val.mValue.mType);

      Allocate(GetCount() + 1);
      const auto index = key.mValue.GetHash().mHash & (GetReserved() - 1);
      InsertInnerUnknown<true>(index, key.Forward(), val.Forward());
      return 1;
   }
   
   /// Semantically insert a type-erased pair                                 
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted or value was overwritten              
   template<CT::Semantic SP>
   LANGULUS(INLINED)
   Count UnorderedMap::InsertUnknown(SP&& pair) {
      using T = TypeOf<SP>;
      static_assert(CT::Pair<T> && !CT::TypedPair<T>,
         "SP's type must be type-erased pair type");

      return InsertUnknown(
         SP::Nest(pair.mValue.mKey), 
         SP::Nest(pair.mValue.mValue)
      );
   }
   
   /// Access value by key, or implicitly add the key if not found            
   /// Value will be default-constructed in the latter case                   
   ///   @param key - the key to search for                                   
   ///   @return the corresponding value block                                
   template<CT::NotSemantic K>
   Block UnorderedMap::At(const K& key) {
      const auto found = FindIndex(key);
      if (found != GetReserved())
         return GetValue(found);

      // Key wasn't found, but map is mutable and we can add it         
      Mutate(MetaData::Of<K>(), mValues.mType);

      auto newk = Block::From(key);
      auto newv = Block {mValues.mState, mValues.mType};
      newv.template AllocateMore<true>(1);

      Allocate(GetCount() + 1);
      const auto index = HashData(key).mHash & (GetReserved() - 1);
      InsertInnerUnknown<false>(index, Copy(newk), Abandon(newv));
      return GetValue(index);
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
