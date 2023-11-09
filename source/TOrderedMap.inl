///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOrderedMap.hpp"
#include "TUnorderedMap.inl"

#define TABLE_TEMPLATE() template<CT::Data K, CT::Data V>
#define TABLE() TOrderedMap<K, V>


namespace Langulus::Anyness
{

   /// Default construction                                                   
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr TABLE()::TOrderedMap()
      : Base {} {}

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TOrderedMap(const TOrderedMap& other)
      : TOrderedMap {Copy(other)} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TOrderedMap(TOrderedMap&& other) noexcept
      : TOrderedMap {Move(other)} {}

   /// Copy construction from any map/pair                                    
   ///   @param other - the map/pair to initialize with                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TOrderedMap(const CT::NotSemantic auto& other)
      : TOrderedMap {Copy(other)} {}

   /// Copy construction from any map/pair                                    
   ///   @param other - the map/pair to initialize with                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TOrderedMap(CT::NotSemantic auto& other)
      : TOrderedMap {Copy(other)} {}
   
   /// Move construction from any map/pair                                    
   ///   @param other - the map/pair to initialize with                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TOrderedMap(CT::NotSemantic auto&& other)
      : TOrderedMap {Move(other)} {}

   /// Semantic constructor from any map/pair                                 
   ///   @param other - the semantic and map/pair to initialize with          
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TOrderedMap(CT::Semantic auto&& other)
      : Base {} {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;
      mKeys.mType   = MetaData::Of<K>();
      mValues.mType = MetaData::Of<V>();

      if constexpr (CT::Array<T>) {
         if constexpr (CT::Pair<Deext<T>>) {
            // Construct from an array of pairs                         
            constexpr auto reserved = Roof2(ExtentOf<T>);
            Base::AllocateFresh(reserved);
            ZeroMemory(mInfo, reserved);
            mInfo[reserved] = 1;

            constexpr auto hashmask = reserved - 1;
            for (auto& pair : *other) {
               Base::template InsertPairInner<true, true>(
                  hashmask, S::Nest(pair));
            }
         }
         else LANGULUS_ERROR("Unsupported semantic array constructor");

         //TODO perhaps constructor from map array, by merging them?
      }
      else if constexpr (CT::Map<T>) {
         // Construct from any kind of map                              
         if constexpr (not T::Ordered) {
            // We have to reinsert everything, because source is        
            // not ordered and uses a different bucketing approach      
            const auto reserved = other->GetReserved();
            Base::AllocateFresh(reserved);
            ZeroMemory(mInfo, reserved);
            mInfo[reserved] = 1;

            const auto hashmask = reserved - 1;
            using TP = typename T::Pair;
            other->ForEach([this, hashmask](TP& pair) {
               Base::template InsertPairInner<false, true>
                  (hashmask, S::Nest(pair));
            });
         }
         else {
            // We can directly interface map, because it is unordered   
            // and uses the same bucketing approach                     
            Base::template BlockTransfer<Self>(other.Forward());
         }
      }
      else if constexpr (CT::Pair<T>) {
         // Construct from any kind of pair                             
         Base::AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         constexpr auto hashmask = MinimalAllocation - 1;
         Base::template InsertPairInner<false, true>(
            hashmask, other.Forward());
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
   }
   
   /// Create from a list of elements                                         
   ///   @param head - first element                                          
   ///   @param tail - tail of elements                                       
   TABLE_TEMPLATE()
   template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
   TABLE()::TOrderedMap(T1&& t1, T2&& t2, TAIL&&... tail) {
      mKeys.mType = MetaData::Of<K>();
      mValues.mType = MetaData::Of<V>();

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

   /// Move a table                                                           
   ///   @param pair - the table to move                                      
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (TOrderedMap&& pair) noexcept {
      return operator = (Move(pair));
   }

   /// Creates a shallow copy of the given table                              
   ///   @param pair - the table to reference                                 
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const TOrderedMap& pair) {
      return operator = (Copy(pair));
   }
   
   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const CT::NotSemantic auto& pair) {
      return operator = (Copy(pair));
   }
   
   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::NotSemantic auto& pair) {
      return operator = (Copy(pair));
   }

   /// Emplace a single pair into a cleared map                               
   ///   @param pair - the pair to emplace                                    
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::NotSemantic auto&& pair) {
      return operator = (Move(pair));
   }

   /// Semantic assignment for an unordered map                               
   ///   @param rhs - the unordered map to use for construction               
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         if (&static_cast<const BlockMap&>(*other) == this)
            return *this;

         Base::template Reset<Self>();
         new (this) Self {other.Forward()};
      }
      else if constexpr (CT::Pair<T>) {
         if (Base::GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Base::template Free<Self>();
            new (this) Self {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Base::template Clear<Self>();
            Base::template InsertPairInner<false, true>(
               Base::GetReserved() - 1,
               other.Forward()
            );
         }
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");

      return *this;
   }


   ///                                                                        
   /// All possible ways a key and value could be inserted to the map         
   ///                                                                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(const K& k, const V& v) {
      return Insert(Copy(k), Copy(v));
   }

   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(const K& k, V&& v) {
      return Insert(Copy(k), Move(v));
   }

   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(const K& k, CT::Semantic auto&& v) {
      return Insert(Copy(k), v.Forward());
   }

   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(K&& k, const V& v) {
      return Insert(Move(k), Copy(v));
   }

   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(K&& k, V&& v) {
      return Insert(Move(k), Move(v));
   }

   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(K&& k, CT::Semantic auto&& v) {
      return Insert(Move(k), v.Forward());
   }

   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(CT::Semantic auto&& k, const V& v) {
      return Insert(k.Forward(), Copy(v));
   }

   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(CT::Semantic auto&& k, V&& v) {
      return Insert(k.Forward(), Move(v));
   }
   
   /// Semantically insert key and value                                      
   ///   @param key - the key to insert                                       
   ///   @param val - the value to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(CT::Semantic auto&& key, CT::Semantic auto&& val) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(val)>;

      Base::template Mutate<TypeOf<SK>, TypeOf<SV>>();
      Base::template Reserve<Self>(Base::GetCount() + 1);
      Base::template InsertInner<true, true>(
         Base::GetBucket(Base::GetReserved() - 1, *key),
         key.Forward(), val.Forward()
      );
      return 1;
   }
   
   /// Semantically insert a type-erased pair                                 
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted or value was overwritten              
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertBlock(CT::Semantic auto&& key, CT::Semantic auto&& val) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(val)>;

      static_assert(CT::Exact<TypeOf<SK>, Block>,
         "SK type must be exactly Block (build-time optimization)");
      static_assert(CT::Exact<TypeOf<SV>, Block>,
         "SV type must be exactly Block (build-time optimization)");

      Base::Mutate(key->mType, val->mType);
      Base::template Reserve<Self>(Base::GetCount() + 1);
      Base::template InsertInnerUnknown<true, true>(
         Base::GetBucketUnknown(Base::GetReserved() - 1, *key),
         key.Forward(), val.Forward()
      );
      return 1;
   }
   
   /// Copy-insert any pair                                                   
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertPair(const CT::Pair auto& pair) {
      return InsertPair(Copy(pair));
   }

   /// Move-insert any pair                                                   
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertPair(CT::Pair auto&& pair) {
      return InsertPair(Move(pair));
   }

   /// Semantically insert any pair                                           
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertPair(CT::Semantic auto&& pair) {
      using S = Decay<decltype(pair)>;
      using T = TypeOf<S>;
      static_assert(CT::Pair<T>, "T must be a pair");

      if constexpr (CT::TypedPair<T>)
         return Insert(S::Nest(pair->mKey), S::Nest(pair->mValue));
      else
         return InsertUnknown(S::Nest(pair->mKey), S::Nest(pair->mValue));
   }

   /// Semantically insert a type-erased pair                                 
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted or value was overwritten              
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertPairBlock(CT::Semantic auto&& pair) {
      using S = Decay<decltype(pair)>;
      using T = TypeOf<S>;
      static_assert(CT::Pair<T> and not CT::TypedPair<T>,
         "SP's type must be type-erased pair type");

      return InsertUnknown(S::Nest(pair->mKey), S::Nest(pair->mValue));
   }

   /// Copy-insert any pair inside the map                                    
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (const CT::Pair auto& pair) {
      return operator << (Copy(pair));
   }

   /// Move-insert any pair inside the map                                    
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (CT::Pair auto&& pair) {
      return operator << (Move(pair));
   }

   /// Semantic insertion of any pair inside the map                          
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (CT::Semantic auto&& pair) {
      InsertPair(pair.Forward());
      return *this;
   }

   /// Combine the contents of two maps (destructively)                       
   ///   @param rhs - the map to concatenate                                  
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator += (const TABLE()& rhs) {
      for (auto pair : rhs) {
         auto found = Find(pair.mKey);
         if (found) {
            if constexpr (requires (V& lhs) { lhs += rhs; })
               GetValue(found) += pair.mValue;
            else
               LANGULUS_THROW(Concat, "No concat operator available");
         }
         else Insert(pair.mKey, pair.mValue);
      }
      return *this;
   }


   ///                                                                        
   ///   Comparison                                                           
   ///                                                                        

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsKey(const K& key) const {
      return Base::template FindInner<Self>(key) != InvalidOffset;
   }

   /// Search for a value inside the table                                    
   ///   @param value - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   TABLE_TEMPLATE()
   bool TABLE()::ContainsValue(const V& match) const
   requires CT::Inner::Comparable<V> {
      if (Base::IsEmpty())
         return false;

      auto value = &Base::GetRawValue(0);
      auto info = Base::GetInfo();
      const auto infoEnd = Base::GetInfoEnd();

      while (info != infoEnd) {
         if (*info and *value == match)
            return true;

         ++value; ++info;
      }

      return false;
   }

   /// Search for a pair inside the table                                     
   ///   @param pair - the pair to search for                                 
   ///   @return true if pair is found, false otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsPair(const Pair& pair) const
   requires CT::Inner::Comparable<V> {
      const auto found = Base::template FindInner<Self>(pair.mKey);
      return found != InvalidOffset and GetValue(found) == pair.mValue;
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Index TABLE()::Find(const K& key) const {
      const auto found = Base::template FindInner<Self>(key);
      return found != InvalidOffset ? Index {found} : IndexNone;
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::FindIt(const K& key) {
      const auto found = Base::template FindInner<Self>(key);
      if (found == InvalidOffset)
         return Base::end();

      return {
         Base::GetInfo() + found, Base::GetInfoEnd(),
         &Base::GetRawKey(found),
         &Base::GetRawValue(found)
      };
   }
      
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::FindIt(const K& key) const {
      return const_cast<Self*>(this)->FindIt(key);
   }
   
   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const K& key) {
      const auto found = Base::template FindInner<Self>(key);
      LANGULUS_ASSERT(found != InvalidOffset, OutOfRange, "Key not found");
      return Base::GetRawValue(found);
   }
   
   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const K& key) const {
      return const_cast<Self*>(this)->At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const K& key) {
      return At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const K& key) const {
      return At(key);
   }
   
   /// Erase a pair via key                                                   
   ///   @param key - the key to search for                                   
   ///   @return 1 if key was found and pair was removed                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemoveKey(const K& key) {
      return BlockMap::template RemoveKey<Self>(key);
   }

   /// Erase all pairs with a given value                                     
   ///   @param value - the match to search for                               
   ///   @return the number of removed pairs                                  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemoveValue(const V& value) {
      return BlockMap::template RemoveValue<Self>(value);
   }

} // namespace Langulus::Anyness

#undef TABLE_TEMPLATE
#undef TABLE
