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
   constexpr TABLE()::TMap() : Map<ORDERED> {} {
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
   
   /// Create from a list of pairs, each of them can be semantic or not,      
   /// an array, as well as any other kinds of maps                           
   ///   @param t1 - first element                                            
   ///   @param tail - tail of elements (optional)                            
   TEMPLATE() template<class T1, class...TAIL>
   requires CT::DeepMapMakable<K, V, T1, TAIL...> LANGULUS(INLINED)
   TABLE()::TMap(T1&& t1, TAIL&&... tail) {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using ST = TypeOf<S>;

         if constexpr (CT::Map<ST>) {
            if constexpr (CT::Typed<ST>) {
               // Not type-erased map, do compile-time type checks      
               using STT = TypeOf<ST>;
               if constexpr (CT::Similar<Pair, STT>) {
                  // Type is binary compatible, just transfer           
                  BlockTransfer<TMap>(S::Nest(t1));
               }
               else InsertPair(Forward<T1>(t1));
            }
            else {
               // Type-erased map, do run-time type checks              
               if (mKeys.mType   == DesemCast(t1).GetKeyType()
               and mValues.mType == DesemCast(t1).GetValueType()) {
                  // If types are exactly the same, it is safe to       
                  // absorb the map, essentially converting a type-     
                  // erased Map back to its TMap equivalent             
                  BlockTransfer<TMap>(S::Nest(t1));
               }
               else InsertPair(Forward<T1>(t1));
            }
         }
         else InsertPair(Forward<T1>(t1));
      }
      else InsertPair(Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Destroys the map and all it's contents                                 
   TEMPLATE()
   TABLE()::~TMap() {
      BlockMap::Free<TMap>();
   }

   /// Move a table                                                           
   ///   @param pair - the table to move                                      
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (TMap&& pair) {
      return operator = (Move(pair));
   }

   /// Creates a shallow copy of the given table                              
   ///   @param pair - the table to reference                                 
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const TMap& pair) {
      return operator = (Copy(pair));
   }
   
   /// Generic assignment                                                     
   ///   @param rhs - the element/array/container to assign                   
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepMapAssignable<K, V, T1> LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (T1&& rhs) {
      using S = SemanticOf<T1>;
      using ST = TypeOf<S>;
       
      if constexpr (CT::Map<ST>) {
         // Potentially absorb the container                            
         if (static_cast<const BlockMap*>(this)
          == static_cast<const BlockMap*>(&DesemCast(rhs)))
            return *this;

         BlockMap::Free<TMap>();
         new (this) TMap {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         BlockMap::ClearInner<TMap>();
         UnfoldInsert<TMap>(S::Nest(rhs));
      }

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

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @param key - the key type                                            
   ///   @param value - the value type                                        
   TEMPLATE() template<CT::NotSemantic K1, CT::NotSemantic V1>
   LANGULUS(INLINED) void TABLE()::Mutate() noexcept {
      return BlockMap::Mutate<TMap, K1, V1>();
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @param key - the key type                                            
   ///   @param value - the value type                                        
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Mutate(DMeta key, DMeta value) {
      return BlockMap::Mutate<TMap>(key, value);
   }

   /// Reserve a new table size                                               
   ///   @param count - the number of elements to reserve                     
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Reserve(Count count) {
      return BlockMap::Reserve<TMap>(count);
   }

   /// Insert pair, by manually providing key and value, semantically or not  
   ///   @param key - the key to insert                                       
   ///   @param val - the value to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   TEMPLATE() template<class K1, class V1>
   requires (CT::Inner::MakableFrom<K, K1> and CT::Inner::MakableFrom<V, V1>)
   LANGULUS(INLINED) Count TABLE()::Insert(K1&& key, V1&& val) {
      using SK = SemanticOf<decltype(key)>;
      using SV = SemanticOf<decltype(val)>;

      Mutate<TypeOf<SK>, TypeOf<SV>>();
      Reserve(GetCount() + 1);
      InsertInner<TMap, true>(
         GetBucket(GetReserved() - 1, DesemCast(key)),
         SK::Nest(key), SV::Nest(val)
      );
      return 1;
   }
   
   /// Insert pair(s) by manually providing key and value blocks              
   ///   @attention only the overlapping elements will be inserted            
   ///   @param key - the key block to insert                                 
   ///   @param val - the value block to insert                               
   ///   @return the number of inserted pairs                                 
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertBlock(auto&& key, auto&& val) {
      using SK = SemanticOf<decltype(key)>;
      using SV = SemanticOf<decltype(val)>;
      using KB = TypeOf<SK>;
      using VB = TypeOf<SV>;
      static_assert(CT::Block<KB>, "Key type must be a block type");
      static_assert(CT::Block<VB>, "Value type must be a block type");

      if constexpr (not CT::Typed<KB> or not CT::Typed<VB>) {
         // Run-time type check required                                
         Mutate(DesemCast(key).GetType(),
                DesemCast(val).GetType());
      }
      else {
         // Compile-time type check                                     
         static_assert(CT::Inner::MakableFrom<K, TypeOf<KB>>,
            "Key is not constructible from the provided key block");
         static_assert(CT::Inner::MakableFrom<V, TypeOf<VB>>,
            "Value is not constructible from the provided value block");
      }

      const auto count = ::std::min(DesemCast(key).GetCount(),
                                    DesemCast(val).GetCount());
      Reserve(GetCount() + count);

      for (Offset i = 0; i < count; ++i) {
         if constexpr (not CT::Typed<KB> or not CT::Typed<VB>) {
            // Type-erased insertion                                    
            auto keyBlock = DesemCast(key).GetElement(i);
            InsertInnerUnknown<TMap, true>(
               GetBucketUnknown(GetReserved() - 1, keyBlock),
               SK::Nest(keyBlock),
               SV::Nest(DesemCast(val).GetElement(i))
            );
         }
         else {
            // Static type insertion                                    
            auto& keyRef = DesemCast(key)[i];
            InsertInner<TMap, true>(
               GetBucket(GetReserved() - 1, keyRef),
               SK::Nest(keyRef),
               SV::Nest(DesemCast(val)[i])
            );
         }
      }
      return count;
   }

   /// Unfold-insert pairs, semantically or not                               
   ///   @param t1, tail... - pairs, or arrays of pairs, to insert            
   ///   @return the number of inserted pairs                                 
   TEMPLATE() template<class T1, class...TAIL>
   requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1, TAIL...>
   LANGULUS(INLINED) Count TABLE()::InsertPair(T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
        inserted += UnfoldInsert(Forward<T1>(t1));
      ((inserted += UnfoldInsert(Forward<TAIL>(tail))), ...);
      return inserted;
   }

   /// Insert a block of pair(s)                                              
   ///   @param pairs - the block of pairs, semantic or not                   
   ///   @return the number of inserted pairs                                 
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::InsertPairBlock(auto&& pairs) {
      using S = SemanticOf<decltype(pairs)>;
      using SB = TypeOf<S>;
      static_assert(CT::Block<SB>, "Pairs must be inside a block type");
      Count inserted = 0;

      if constexpr (not CT::Typed<SB>) {
         // Insert pairs from a type-erased block...                    
         if (DesemCast(pairs).template CastsTo<Anyness::Pair>()) {
            // ...as type-erased pairs                                  
            DesemCast(pairs).ForEach([&](const Anyness::Pair& p) {
               inserted += InsertPair(S::Nest(const_cast<Anyness::Pair&>(p)));
            });
         }
         else if (DesemCast(pairs).template CastsTo<Pair>()) {
            // ...as exactly typed pairs                                
            DesemCast(pairs).ForEach([&](const Pair& p) {
               inserted += InsertPair(S::Nest(const_cast<Pair&>(p)));
            });
         }
      }
      else if constexpr (CT::Pair<TypeOf<SB>>) {
         // Insert pairs from a statically-typed block                  
         for (auto& p : DesemCast(pairs))
            inserted += InsertPair(S::Nest(p));
      }

      return inserted;
   }

   /// Insert pair                                                            
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1> LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (T1&& pair) {
      InsertPair(pair.Forward());
      return *this;
   }

   /// Insert pair                                                            
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1> LANGULUS(INLINED)
   TABLE()& TABLE()::operator >> (T1&& pair) {
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
      return BlockMap::template RemoveKey<TMap>(key);
   }

   /// Erase all pairs with a given value                                     
   ///   @param value - the match to search for                               
   ///   @return the number of removed pairs                                  
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::RemoveValue(const V& value) {
      return BlockMap::template RemoveValue<TMap>(value);
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

   /// Destroy all contained pairs, but don't deallocate                      
   TEMPLATE()
   void TABLE()::Clear() {
      return BlockMap::Clear<TMap>();
   }

   /// Destroy all contained pairs and deallocate                             
   TEMPLATE()
   void TABLE()::Reset() {
      return BlockMap::Reset<TMap>();
   }

   /// Reduce reserved size, depending on number of contained elements        
   TEMPLATE()
   void TABLE()::Compact() {
      return BlockMap::Compact<TMap>();
   }


   ///                                                                        
   ///   Comparison                                                           
   ///                                                                        
   
   /// Compare this map against another map, type-erased or not               
   ///   @param rhs - map to compare against                                  
   ///   @return true if contents of both maps are the same                   
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (CT::Map auto const& rhs) const
   requires CT::Inner::Comparable<V> {
      return BlockMap::operator == <TMap> (rhs);
   }

   /// Compare this map against a pair, type-erased or not                    
   ///   @param rhs - pair to compare against                                 
   ///   @return true this map contains only this exact pair                  
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (CT::Pair auto const& rhs) const
   requires CT::Inner::Comparable<V> {
      return BlockMap::operator == <TMap> (rhs);
   }

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   bool TABLE()::ContainsKey(K1 const& key) const {
      return BlockMap::ContainsKey<TMap>(key);
   }

   /// Search for a value inside the table                                    
   ///   @param match - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   TEMPLATE() template<CT::NotSemantic V1>
   requires ::std::equality_comparable_with<V, V1> LANGULUS(INLINED)
   bool TABLE()::ContainsValue(V1 const& match) const {
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
   TEMPLATE() template<CT::Pair P>
   requires ::std::equality_comparable_with<TPair<K, V>, P>
   LANGULUS(INLINED) bool TABLE()::ContainsPair(P const& pair) const {
      return BlockMap::ContainsPair<TMap>(pair);
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   Index TABLE()::Find(K1 const& key) const {
      return BlockMap::Find<TMap>(key);
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   auto TABLE()::FindIt(K1 const& key) {
      return BlockMap::FindIt<TMap>(key);
   }
      
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   auto TABLE()::FindIt(K1 const& key) const {
      return BlockMap::FindIt<TMap>(key);
   }
   
   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   decltype(auto) TABLE()::At(K1 const& key) {
      const auto found = FindInner<TMap>(key);
      LANGULUS_ASSERT(found != InvalidOffset, OutOfRange, "Key not found");
      return GetRawValue(found);
   }
   
   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   decltype(auto) TABLE()::At(K1 const& key) const {
      return const_cast<TABLE()*>(this)->At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (K1 const& key) {
      return At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TEMPLATE() template<CT::NotSemantic K1>
   requires ::std::equality_comparable_with<K, K1> LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (K1 const& key) const {
      return At(key);
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   const TAny<K>& TABLE()::GetKeys() const noexcept {
      return BlockMap::GetKeys<TMap>();
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   TAny<K>& TABLE()::GetKeys() noexcept {
      return BlockMap::GetKeys<TMap>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   const TAny<V>& TABLE()::GetValues() const noexcept {
      return BlockMap::GetValues<TMap>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   TAny<V>& TABLE()::GetValues() noexcept {
      return BlockMap::GetValues<TMap>();
   }

   /// Get a key at an index                                                  
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param index - the index                                             
   ///   @return the mutable key reference                                    
   TEMPLATE() LANGULUS(INLINED)
   K& TABLE()::GetKey(CT::Index auto index) {
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
   const K& TABLE()::GetKey(CT::Index auto index) const {
      return const_cast<TABLE()*>(this)->GetKey(index);
   }

   /// Get a value at an index                                                
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable value reference                                  
   TEMPLATE() LANGULUS(INLINED)
   V& TABLE()::GetValue(CT::Index auto index) {
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
   const V& TABLE()::GetValue(CT::Index auto index) const {
      return const_cast<TABLE()*>(this)->GetValue(index);
   }

   /// Get a pair at an index                                                 
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable pair reference                                   
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::PairRef TABLE()::GetPair(CT::Index auto i) {
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
   typename TABLE()::PairConstRef TABLE()::GetPair(CT::Index auto i) const {
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
      while (not *info)
         ++info;

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
      while (info >= GetInfo() and not *--info)
         ;

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
      while (not *info)
         ++info;

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
      while (info >= GetInfo() and not *--info)
         ;

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
      while (info >= GetInfo() and not *--info)
         ;
      return GetPair(static_cast<Offset>(info - GetInfo()));
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::Last() const {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info)
         ;
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
      while (not *++mInfo)
         ;
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
