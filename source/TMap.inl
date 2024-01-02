///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TMap.hpp"
#include "Map.inl"
#include "TPair.inl"
#include "TAny.inl"
#include "TAny-Iteration.inl"

#define TEMPLATE() template<CT::Data K, CT::Data V, bool ORDERED>
#define TABLE() TMap<K, V, ORDERED>
#define ITERATOR() TABLE()::template TIterator<MUTABLE>


namespace Langulus::Anyness
{

   /// Default construction                                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TABLE()::TMap()
      : Map<ORDERED> {} {
      mKeys.mState = DataState::Typed;
      mValues.mState = DataState::Typed;
      if constexpr (CT::Constant<K>)
         mKeys.MakeConst();
      if constexpr (CT::Constant<V>)
         mValues.MakeConst();
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::TMap(const TMap& other)
      : TMap {Copy(other)} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::TMap(TMap&& other) noexcept
      : TMap {Move(other)} {}

   /// Semantic constructor from any map/pair                                 
   ///   @param other - the semantic and map/pair to initialize with          
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::TMap(CT::Semantic auto&& other)
      : TMap {} {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;
      mKeys.mType = MetaDataOf<K>();
      mValues.mType = MetaDataOf<V>();

      if constexpr (CT::Array<T>) {
         if constexpr (CT::Pair<Deext<T>>) {
            // Construct from an array of pairs                         
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
            BlockTransfer<TUnorderedMap>(other.Forward());
         }
      }
      else if constexpr (CT::Pair<T>) {
         // Construct from any kind of pair                             
         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         constexpr auto hashmask = MinimalAllocation - 1;
         InsertPairInner<false, false>(hashmask, other.Forward());
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
   }
   
   /// Create from a list of elements                                         
   ///   @param head - first element                                          
   ///   @param tail - tail of elements                                       
   TEMPLATE()
   template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
   TABLE()::TMap(T1&& t1, T2&& t2, TAIL&&... tail) {
      mKeys.mType = MetaDataOf<K>();
      mValues.mType = MetaDataOf<V>();

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

   /// Destroys the map and all it's contents                                 
   TEMPLATE()
   TABLE()::~TMap() {
      Free<Self>();
      mValues.mEntry = nullptr;
   }

   /// Move a table                                                           
   ///   @param pair - the table to move                                      
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (TMap&& pair) noexcept {
      return operator = (Move(pair));
   }

   /// Creates a shallow copy of the given table                              
   ///   @param pair - the table to reference                                 
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const TMap& pair) {
      return operator = (Copy(pair));
   }

   /// Semantic assignment for an unordered map                               
   ///   @param rhs - the unordered map to use for construction               
   TEMPLATE()
   TABLE()& TABLE()::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         if (&static_cast<const BlockMap&>(*other) == this)
            return *this;

         Reset<Self>();
         new (this) Self {other.Forward()};
      }
      else if constexpr (CT::Pair<T>) {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Free<Self>();
            new (this) Self {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Clear<Self>();
            InsertPairInner<false, false>(
               GetReserved() - 1, other.Forward());
         }
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");

      return *this;
   }

   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyTypeConstrained() const noexcept {
      return true;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if key type is abstract                                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyAbstract() const noexcept {
      return CT::Abstract<K>;
   }
   
   /// Check if value type is abstract                                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueAbstract() const noexcept {
      return CT::Abstract<V>;
   }
   
   /// Check if key type is default-constructible                             
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyConstructible() const noexcept {
      return CT::Defaultable<K>;
   }
   
   /// Check if value type is default-constructible                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueConstructible() const noexcept {
      return CT::Defaultable<V>;
   }
   
   /// Check if key type is deep                                              
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyDeep() const noexcept {
      return CT::Deep<K>;
   }
   
   /// Check if value type is deep                                            
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueDeep() const noexcept {
      return CT::Deep<V>;
   }

   /// Check if the key type is a pointer                                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeySparse() const noexcept {
      return CT::Sparse<K>;
   }
   
   /// Check if the value type is a pointer                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueSparse() const noexcept {
      return CT::Sparse<V>;
   }

   /// Check if the key type is not a pointer                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyDense() const noexcept {
      return CT::Dense<K>;
   }

   /// Check if the value type is not a pointer                               
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueDense() const noexcept {
      return CT::Dense<V>;
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TABLE()::GetKeyStride() const noexcept {
      return sizeof(K); 
   }
   
   /// Get the size of a single value, in bytes                               
   ///   @return the number of bytes a single value contains                  
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TABLE()::GetValueStride() const noexcept {
      return sizeof(V); 
   }

   /// Get a raw key entry (const)                                            
   ///   @param index - the key index                                         
   ///   @return a constant reference to the element                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr const K& TABLE()::GetRawKey(Offset index) const noexcept {
      return GetKeys().GetRaw()[index];
   }

   /// Get a raw key entry                                                    
   ///   @param index - the key index                                         
   ///   @return a mutable reference to the element                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr K& TABLE()::GetRawKey(Offset index) noexcept {
      return GetKeys().GetRaw()[index];
   }

   /// Get a handle to a key                                                  
   ///   @param index - the key index                                         
   ///   @return the handle                                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr Handle<K> TABLE()::GetKeyHandle(Offset index) noexcept {
      return GetKeys().GetHandle(index);
   }

   /// Get a raw value entry (const)                                          
   ///   @param index - the value index                                       
   ///   @return a constant reference to the element                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr const V& TABLE()::GetRawValue(Offset index) const noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a raw value entry                                                  
   ///   @param index - the value index                                       
   ///   @return a mutable reference to the element                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr V& TABLE()::GetRawValue(Offset index) noexcept {
      return GetValues().GetRaw()[index];
   }
   
   /// Get a handle to a value                                                
   ///   @param index - the value index                                       
   ///   @return the handle                                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr Handle<V> TABLE()::GetValueHandle(Offset index) noexcept {
      return GetValues().GetHandle(index);
   }

   /// Get the size of all pairs, in bytes                                    
   ///   @return the total amount of initialized bytes                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TABLE()::GetBytesize() const noexcept {
      return sizeof(Pair) * GetCount(); 
   }

   /// Get the key meta data                                                  
   /// Also implicitly initializes the internal key type                      
   ///   @attention this shouldn't be called on static initialization time    
   ///   @return the meta definition of the key type                          
   TEMPLATE() LANGULUS(INLINED)
   DMeta TABLE()::GetKeyType() const {
      mKeys.mType = MetaDataOf<K>();
      return mKeys.mType;
   }

   /// Get the value meta data                                                
   /// Also implicitly initializes the internal key type                      
   ///   @attention this shouldn't be called on static initialization time    
   ///   @return the meta definition of the value type                        
   TEMPLATE() LANGULUS(INLINED)
   DMeta TABLE()::GetValueType() const {
      mValues.mType = MetaDataOf<V>();
      return mValues.mType;
   }

   /// Check if key origin type matches any of the list                       
   ///   @tparam K1, KN... - the list of types to compare against             
   ///   @return true if key type matches at least one of the others          
   TEMPLATE()
   template<CT::Data K1, CT::Data... KN>
   LANGULUS(INLINED)
   constexpr bool TABLE()::KeyIs() const noexcept {
      return CT::SameAsOneOf<K, K1, KN...>;
   }

   /// Check if key origin type matches another                               
   ///   @param key - the key type to compare against                         
   ///   @return true if key matches the contained key origin type            
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::KeyIs(DMeta key) const noexcept {
      return GetKeyType()->Is(key);
   }

   /// Check if cv-unqualified key type matches any of the list               
   ///   @tparam K1, KN... - the list of types to compare against             
   ///   @return true if key type matches at least one of the others          
   TEMPLATE()
   template<CT::Data K1, CT::Data... KN>
   LANGULUS(INLINED)
   constexpr bool TABLE()::KeyIsSimilar() const noexcept {
      return CT::SimilarAsOneOf<K, K1, KN...>;
   }

   /// Check if cv-unqualified key type matches another                       
   ///   @param key - the key type to compare against                         
   ///   @return true if key matches the contained key unqualified type       
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::KeyIsSimilar(DMeta key) const noexcept {
      return GetKeyType()->IsSimilar(key);
   }

   /// Check if key type exactly matches any of the list                      
   ///   @tparam K1, KN... - the list of types to compare against             
   ///   @return true if key type matches at least one of the others          
   TEMPLATE()
   template<CT::Data K1, CT::Data... KN>
   LANGULUS(INLINED)
   constexpr bool TABLE()::KeyIsExact() const noexcept {
      return CT::ExactAsOneOf<K, K1, KN...>;
   }

   /// Check if key type exactly matches any of the list                      
   ///   @param key - the key type to compare against                         
   ///   @return true if key matches the contained key unqualified type       
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::KeyIsExact(DMeta key) const noexcept {
      return GetKeyType()->IsExact(key);
   }

   /// Check if value origin type matches any of the list                     
   ///   @tparam V1, VN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE()
   template<CT::Data V1, CT::Data... VN>
   LANGULUS(INLINED)
   constexpr bool TABLE()::ValueIs() const noexcept {
      return CT::SameAsOneOf<V, V1, VN...>;
   }

   /// Check if value origin type matches another                             
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::ValueIs(DMeta value) const noexcept {
      return GetValueType()->Is(value);
   }

   /// Check if cv-unqualified value type matches any of the list             
   ///   @tparam V1, VN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE()
   template<CT::Data V1, CT::Data... VN>
   LANGULUS(INLINED)
   constexpr bool TABLE()::ValueIsSimilar() const noexcept {
      return CT::SimilarAsOneOf<V, V1, VN...>;
   }

   /// Check if cv-unqualified value type matches another                     
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained value unqualified type   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::ValueIsSimilar(DMeta value) const noexcept {
      return GetValueType()->IsSimilar(value);
   }

   /// Check if value type exactly matches any of the list                    
   ///   @tparam V1, VN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE()
   template<CT::Data V1, CT::Data... VN>
   LANGULUS(INLINED)
   constexpr bool TABLE()::ValueIsExact() const noexcept {
      return CT::ExactAsOneOf<V, V1, VN...>;
   }

   /// Check if value type exactly matches any of the list                    
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained value unqualified type   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::ValueIsExact(DMeta value) const noexcept {
      return GetValueType()->IsExact(value);
   }

   /// Checks type compatibility and sets type for the map                    
   /// Does only compile-time checks for this templated variant               
   ///   @tparam ALT_K - the key type to check                                
   ///   @tparam ALT_V - the value type to check                              
   TEMPLATE()
   template<CT::NotSemantic ALT_K, CT::NotSemantic ALT_V>
   LANGULUS(INLINED)
   constexpr void TABLE()::Mutate() noexcept {
      static_assert(CT::Similar<K, ALT_K>,
         "Can't mutate to incompatible key");
      static_assert(CT::Similar<V, ALT_V>,
         "Can't mutate to incompatible value");

      // Since this is not a type-erased map, GetType() will also set it
      (void) GetKeyType();
      (void) GetValueType();
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @param key - the key type                                            
   ///   @param value - the value type                                        
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Mutate(DMeta key, DMeta value) {
      // Types known at compile-time, so check compatibility            
      // Since this is not a type-erased map, GetType() will also set it
      LANGULUS_ASSERT(GetKeyType()->IsSimilar(key), Mutate,
         "Can't mutate to incompatible key");
      LANGULUS_ASSERT(GetValueType()->IsSimilar(value), Mutate,
         "Can't mutate to incompatible value");
   }
   
   /// Semantically insert key and value                                      
   ///   @param key - the key to insert                                       
   ///   @param val - the value to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(CT::Semantic auto&& key, CT::Semantic auto&& val) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(val)>;

      Mutate<TypeOf<SK>, TypeOf<SV>>();
      Reserve(GetCount() + 1);
      InsertInner<true, false>(
         GetBucket(GetReserved() - 1, *key),
         key.Forward(), val.Forward()
      );
      return 1;
   }
   
   /// Semantically insert a type-erased pair                                 
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted or value was overwritten              
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertBlock(CT::Semantic auto&& key, CT::Semantic auto&& val) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(val)>;

      static_assert(CT::Exact<TypeOf<SK>, Block>,
         "SK type must be exactly Block (build-time optimization)");
      static_assert(CT::Exact<TypeOf<SV>, Block>,
         "SV type must be exactly Block (build-time optimization)");

      Mutate(key->mType, val->mType);
      Reserve(GetCount() + 1);
      InsertInnerUnknown<true, false>(
         GetBucketUnknown(GetReserved() - 1, *key),
         key.Forward(), val.Forward()
      );
      return 1;
   }

   /// Semantically insert any pair                                           
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE() LANGULUS(INLINED)
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
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertPairBlock(CT::Semantic auto&& pair) {
      using S = Decay<decltype(pair)>;
      using T = TypeOf<S>;
      static_assert(CT::Pair<T> and not CT::TypedPair<T>,
         "SP's type must be type-erased pair type");

      return InsertUnknown(S::Nest(pair->mKey), S::Nest(pair->mValue));
   }

   /// Semantic insertion of any pair inside the map                          
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (CT::Semantic auto&& pair) {
      InsertPair(pair.Forward());
      return *this;
   }

   /// Combine the contents of two maps (destructively)                       
   ///   @param rhs - the map to concatenate                                  
   ///   @return a reference to this table for chaining                       
   TEMPLATE() LANGULUS(INLINED)
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

   /// Request a new size of keys and info via the value container            
   /// The memory layout is:                                                  
   ///   [keys for each bucket]                                               
   ///         [padding for alignment]                                        
   ///               [info for each bucket]                                   
   ///                     [one sentinel byte for terminating loops]          
   ///   @param infoStart - [out] the offset at which info bytes start        
   ///   @return the requested byte size                                      
   TEMPLATE() LANGULUS(INLINED)
   Size TABLE()::RequestKeyAndInfoSize(const Count count, Offset& infoStart) noexcept {
      Size keymemory = count * sizeof(K);
      if constexpr (CT::Sparse<K>)
         keymemory *= 2;
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + count + 1;
   }

   /// Request a new size of value container                                  
   ///   @attention assumes value type has been set                           
   ///   @param count - number of values to allocate                          
   ///   @return the requested byte size                                      
   TEMPLATE() LANGULUS(INLINED)
   Size TABLE()::RequestValuesSize(const Count count) noexcept {
      Size valueByteSize = count * sizeof(V);
      if constexpr (CT::Sparse<V>)
         valueByteSize *= 2;
      return valueByteSize;
   }

   /// Erase a pair via key                                                   
   ///   @param key - the key to search for                                   
   ///   @return 1 if key was found and pair was removed                      
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemoveKey(const K& key) {
      return BlockMap::template RemoveKey<TABLE()>(key);
   }

   /// Erase all pairs with a given value                                     
   ///   @param value - the match to search for                               
   ///   @return the number of removed pairs                                  
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemoveValue(const V& value) {
      return BlockMap::template RemoveValue<TABLE()>(value);
   }
     
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this map instance         
   ///   @attention assumes that iterator points to a valid entry             
   ///   @param index - the index to remove                                   
   ///   @return the iterator of the previous element, unless index is the    
   ///           first, or at the end already                                 
   TEMPLATE()
   typename TABLE()::Iterator TABLE()::RemoveIt(const Iterator& index) {
      const auto sentinel = GetReserved();
      auto offset = static_cast<Offset>(index.mInfo - mInfo);
      if (offset >= sentinel)
         return end();

      RemoveInner<K, V>(offset--); //TODO what if map shrinks, offset might become invalid? Doesn't shrink for now
      
      while (offset < sentinel and not mInfo[offset])
         --offset;

      if (offset >= sentinel)
         offset = 0;

      return {
         mInfo + offset, 
         index.mSentinel,
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }


   ///                                                                        
   ///   Comparison                                                           
   ///                                                                        
   
   /// Checks if both tables contain the same entries                         
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (const TMap& other) const requires CT::Inner::Comparable<V> {
      if (other.GetCount() != GetCount())
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            const auto lhs = info - GetInfo();
            const auto rhs = other.template FindInner<TABLE()>(GetRawKey(lhs));
            if (rhs == InvalidOffset or GetRawValue(lhs) != other.GetRawValue(rhs))
               return false;
         }

         ++info;
      }

      return true;
   }

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsKey(const K& key) const {
      return FindInner<TABLE()>(key) != InvalidOffset;
   }

   /// Search for a value inside the table                                    
   ///   @param value - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   TEMPLATE()
   bool TABLE()::ContainsValue(const V& match) const requires CT::Inner::Comparable<V> {
      if (IsEmpty())
         return false;

      auto value = &GetRawValue(0);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();

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
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsPair(const Pair& pair) const requires CT::Inner::Comparable<V> {
      const auto found = FindInner<TABLE()>(pair.mKey);
      return found != InvalidOffset and GetValue(found) == pair.mValue;
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TEMPLATE() LANGULUS(INLINED)
   Index TABLE()::Find(const K& key) const {
      const auto offset = FindInner<TABLE()>(key);
      return offset != InvalidOffset ? Index {offset} : IndexNone;
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::FindIt(const K& key) {
      const auto found = FindInner<TABLE()>(key);
      if (found == InvalidOffset)
         return end();

      return {
         GetInfo() + found, GetInfoEnd(),
         &GetRawKey(found),
         &GetRawValue(found)
      };
   }
      
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::FindIt(const K& key) const {
      return const_cast<TABLE()*>(this)->FindIt(key);
   }
   
   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const K& key) {
      const auto found = FindInner<TABLE()>(key);
      LANGULUS_ASSERT(found != InvalidOffset, OutOfRange, "Key not found");
      return GetRawValue(found);
   }
   
   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const K& key) const {
      return const_cast<TABLE()*>(this)->At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const K& key) {
      return At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const K& key) const {
      return At(key);
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   const TAny<K>& TABLE()::GetKeys() const noexcept {
      return BlockMap::GetKeys<K>();
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   TAny<K>& TABLE()::GetKeys() noexcept {
      return BlockMap::GetKeys<K>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   const TAny<V>& TABLE()::GetValues() const noexcept {
      return BlockMap::GetValues<V>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   TAny<V>& TABLE()::GetValues() noexcept {
      return BlockMap::GetValues<V>();
   }

   /// Get a key at an index                                                  
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param index - the index                                             
   ///   @return the mutable key reference                                    
   TEMPLATE() LANGULUS(INLINED)
   K& TABLE()::GetKey(const CT::Index auto& index) {
      if (IsEmpty())
         LANGULUS_OOPS(OutOfRange, "Map is empty");

      const auto idx = GetKeys().template SimplifyIndex<K, false>(index);
      if (not mInfo[idx])
         LANGULUS_OOPS(OutOfRange, "No pair at given index");
      return GetKeys().GetRaw()[idx];
   }

   /// Get a key at an index                                                  
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param index - the index                                             
   ///   @return the constant key reference                                   
   TEMPLATE() LANGULUS(INLINED)
   const K& TABLE()::GetKey(const CT::Index auto& index) const {
      return const_cast<TABLE()*>(this)->GetKey(index);
   }

   /// Get a value at an index                                                
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable value reference                                  
   TEMPLATE() LANGULUS(INLINED)
   V& TABLE()::GetValue(const CT::Index auto& index) {
      if (IsEmpty())
         LANGULUS_OOPS(OutOfRange, "Map is empty");

      const auto idx = GetValues().template SimplifyIndex<V, false>(index);
      if (not mInfo[idx])
         LANGULUS_OOPS(OutOfRange, "No pair at given index");
      return GetValues().GetRaw()[idx];
   }

   /// Get a value at an index                                                
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param index - the index                                             
   ///   @return the constant value reference                                 
   TEMPLATE() LANGULUS(INLINED)
   const V& TABLE()::GetValue(const CT::Index auto& index) const {
      return const_cast<TABLE()*>(this)->GetValue(index);
   }

   /// Get a pair at an index                                                 
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable pair reference                                   
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::PairRef TABLE()::GetPair(const CT::Index auto& i) {
      if (IsEmpty())
         LANGULUS_OOPS(OutOfRange, "Map is empty");

      const auto idx = GetValues().template SimplifyIndex<V, false>(i);
      return {GetKey(idx), GetValue(idx)};
   }

   /// Get a pair at an index                                                 
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the constant pair reference                                  
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::PairConstRef TABLE()::GetPair(const CT::Index auto& i) const {
      return const_cast<TABLE()*>(this)->GetPair(i);
   }
   

   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::end() noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr, nullptr};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TEMPLATE()
   typename TABLE()::ConstIterator TABLE()::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr, nullptr};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TEMPLATE()
   typename TABLE()::ConstIterator TABLE()::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }
   
   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::Last() {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);
      return GetPair(static_cast<Offset>(info - GetInfo()));
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::Last() const {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);
      return GetPair(static_cast<Offset>(info - GetInfo()));
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TEMPLATE()
   template<class F>
   LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(F&& f) const {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      static_assert(CT::Block<A>, "Function argument must be a block type");

      Offset i {};
      if constexpr (not CT::Void<R>) {
         return GetKeys().ForEachElement([&](const Block& element) {
            return mInfo[i++] ? f(element) : Flow::Continue;
         });
      }
      else {
         return GetKeys().ForEachElement([&](const Block& element) {
            if (mInfo[i++])
               f(element);
         });
      }
   }

   /// Iterate all keys inside the map, and perform f() on them (mutable)     
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TEMPLATE()
   template<class F>
   LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(F&& f) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      static_assert(CT::Block<A>, "Function argument must be a block type");

      Offset i {};
      if constexpr (not CT::Void<R>) {
         return GetKeys().ForEachElement([&](const Block& element) {
            return mInfo[i++] ? f(element) : Flow::Continue;
         });
      }
      else {
         return GetKeys().ForEachElement([&](const Block& element) {
            if (mInfo[i++])
               f(element);
         });
      }
   }
   
   /// Iterate all values inside the map, and perform f() on them             
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each value block                 
   ///   @return the number of successful f() executions                      
   TEMPLATE()
   template<class F>
   LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(F&& f) const {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      static_assert(CT::Block<A>, "Function argument must be a block type");

      Offset i {};
      if constexpr (not CT::Void<R>) {
         return GetValues().ForEachElement([&](const Block& element) {
            return mInfo[i++] ? f(element) : Flow::Continue;
         });
      }
      else {
         return GetValues().ForEachElement([&](const Block& element) {
            if (mInfo[i++])
               f(element);
         });
      }
   }

   /// Iterate all values inside the map, and perform f() on them (mutable)   
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each value block                 
   ///   @return the number of successful f() executions                      
   TEMPLATE()
   template<class F>
   LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(F&& f) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      static_assert(CT::Block<A>, "Function argument must be a block type");

      Offset i {};
      if constexpr (not CT::Void<R>) {
         return GetValues().ForEachElement([&](const Block& element) {
            return mInfo[i++] ? f(element) : Flow::Continue;
         });
      }
      else {
         return GetValues().ForEachElement([&](const Block& element) {
            if (mInfo[i++])
               f(element);
         });
      }
   }


   ///                                                                        
   ///   Unordered map iterator                                               
   ///                                                                        

   /// Construct from a mutable iterator                                      
   ///   @param other - the mutable iterator                                  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   TABLE()::TIterator<MUTABLE>::TIterator(const TIterator<true>& other) noexcept
      : mInfo {other.mInfo}
      , mSentinel {other.mSentinel}
      , mKey {other.mKey}
      , mValue {other.mValue} {}

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   TABLE()::TIterator<MUTABLE>::TIterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const K* key, 
      const V* value
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
      , mKey {key}
      , mValue {value} {}

   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   TABLE()::TIterator<MUTABLE>& TABLE()::TIterator<MUTABLE>::operator = (const TABLE()::TIterator<MUTABLE>& rhs) noexcept {
      mInfo = rhs.mInfo;
      mSentinel = rhs.mSentinel;
      mKey = rhs.mKey;
      mValue = rhs.mValue;
      return *this;
   }

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename ITERATOR()& TABLE()::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (not *++mInfo);
      const auto offset = mInfo - previous;
      mKey += offset;
      mValue += offset;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename ITERATOR() TABLE()::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   bool TABLE()::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairRef TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return {*const_cast<K*>(mKey), *const_cast<V*>(mValue)};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairConstRef TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (not MUTABLE) {
      return {*mKey, *mValue};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairRef TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return **this;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairConstRef TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (not MUTABLE) {
      return **this;
   }

   /// Explicit bool operator, to check if iterator is valid                  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr TABLE()::TIterator<MUTABLE>::operator bool() const noexcept {
      return mInfo != mSentinel;
   }

} // namespace Langulus::Anyness

#undef ITERATOR
#undef TEMPLATE
#undef TABLE
