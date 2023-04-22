///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedMap.hpp"

#define TABLE_TEMPLATE() template<CT::Data K, CT::Data V>
#define TABLE() TUnorderedMap<K, V>
#define ITERATOR() TABLE()::template TIterator<MUTABLE>

namespace Langulus::Anyness
{

   /// Default construction                                                   
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr TABLE()::TUnorderedMap()
      : UnorderedMap {} {
      mKeys.mState = DataState::Typed;
      mValues.mState = DataState::Typed;
      if constexpr (CT::Constant<K>)
         mKeys.MakeConst();
      if constexpr (CT::Constant<V>)
         mValues.MakeConst();
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedMap(const TUnorderedMap& other)
      : TUnorderedMap {Copy(other)} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedMap(TUnorderedMap&& other) noexcept
      : TUnorderedMap {Move(other)} {}

   /// Copy construction from any map/pair                                    
   ///   @param other - the map/pair to initialize with                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedMap(const CT::NotSemantic auto& other)
      : TUnorderedMap {Copy(other)} {}

   /// Copy construction from any map/pair                                    
   ///   @param other - the map/pair to initialize with                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedMap(CT::NotSemantic auto& other)
      : TUnorderedMap {Copy(other)} {}
   
   /// Move construction from any map/pair                                    
   ///   @param other - the map/pair to initialize with                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedMap(CT::NotSemantic auto&& other)
      : TUnorderedMap {Move(other)} {}

   /// Semantic constructor from any map/pair                                 
   ///   @param other - the semantic and map/pair to initialize with          
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedMap(CT::Semantic auto&& other)
      : TUnorderedMap {} {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         // Construct from any kind of map                              
         if constexpr (T::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
            mKeys.mType = MetaData::Of<K>();
            mValues.mType = MetaData::Of<V>();

            AllocateFresh(other.mValue.GetReserved());

            ZeroMemory(mInfo, GetReserved());
            mInfo[GetReserved()] = 1;

            other.mValue.ForEach(
               [this](const typename T::Pair& pair) {
                  InsertUnknown(S::Nest(pair));
               }
            );
         }
         else {
            // We can directly interface map, because it is unordered   
            // and uses the same bucketing approach                     
            BlockTransfer<TUnorderedMap>(other.Forward());
         }
      }
      else if constexpr (CT::Pair<T>) {
         // Construct from any kind of pair                             
         mKeys.mType = MetaData::Of<K>();
         mValues.mType = MetaData::Of<V>();

         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         // Insert a statically typed element                           
         InsertInner<false>(
            GetBucket(MinimalAllocation - 1, other.mValue.mKey),
            S::Nest(other.mValue.mKey),
            S::Nest(other.mValue.mValue)
         );
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
   }
   
   /// Create from a list of elements                                         
   ///   @param head - first element                                          
   ///   @param tail - tail of elements                                       
   TABLE_TEMPLATE()
   template<CT::Data HEAD, CT::Data... TAIL>
   TABLE()::TUnorderedMap(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1) {
      mKeys.mType = MetaData::Of<K>();
      mValues.mType = MetaData::Of<V>();

      constexpr auto capacity = Roof2(
         sizeof...(TAIL) + 1 < MinimalAllocation
            ? MinimalAllocation
            : sizeof...(TAIL) + 1
      );

      AllocateFresh(capacity);
      ZeroMemory(mInfo, capacity);
      mInfo[capacity] = 1;
      Inner::NestedSemanticInsertion(
         *this, Forward<HEAD>(head), Forward<TAIL>(tail)...
      );
   }

   /// Destroys the map and all it's contents                                 
   TABLE_TEMPLATE()
   TABLE()::~TUnorderedMap() {
      if (!mValues.mEntry)
         return;

      if (mValues.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         // This is a statically-optimized equivalent                   
         ClearInner();

         // Deallocate stuff                                            
         Allocator::Deallocate(mKeys.mEntry);
         Allocator::Deallocate(mValues.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref values     
         // Notice we don't dereference keys, we use only value's refs  
         // to save on some redundancy                                  
         mValues.mEntry->Free();
      }

      mValues.mEntry = nullptr;
   }

   /// Checks if both tables contain the same entries                         
   /// Order is irrelevant                                                    
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   TABLE_TEMPLATE()
   bool TABLE()::operator == (const TUnorderedMap& other) const {
      if (other.GetCount() != GetCount())
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         const auto lhs = info - GetInfo();
         if (!*(info++))
            continue;

         const auto rhs = other.FindIndex(GetRawKey(lhs));
         if (rhs == other.GetReserved() || GetValue(lhs) != other.GetValue(rhs))
            return false;
      }

      return true;
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (TUnorderedMap&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const TUnorderedMap& rhs) {
      return operator = (Langulus::Copy(rhs));
   }
   
   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }
   
   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }

   /// Emplace a single pair into a cleared map                               
   ///   @param pair - the pair to emplace                                    
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::NotSemantic auto&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantic assignment for an unordered map                               
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the unordered map to use for construction               
   TABLE_TEMPLATE()
   template<CT::Semantic S>
   TABLE()& TABLE()::operator = (S&& rhs) {
      using ST = TypeOf<S>;

      if constexpr (CT::Map<ST>) {
         if (&static_cast<const BlockMap&>(rhs.mValue) == this)
            return *this;

         Reset();
         new (this) Self {rhs.Forward()};
      }
      else if constexpr (CT::Pair<ST>) {
         Clear();
         Insert(S::Nest(rhs.mValue.mKey), S::Nest(rhs.mValue.mValue));
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");

      return *this;
   }

   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyTypeConstrained() const noexcept {
      return true;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if key type is abstract                                          
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyAbstract() const noexcept {
      return CT::Abstract<K>;
   }
   
   /// Check if value type is abstract                                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueAbstract() const noexcept {
      return CT::Abstract<V>;
   }
   
   /// Check if key type is default-constructible                             
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyConstructible() const noexcept {
      return CT::Defaultable<K>;
   }
   
   /// Check if value type is default-constructible                           
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueConstructible() const noexcept {
      return CT::Defaultable<V>;
   }
   
   /// Check if key type is deep                                              
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyDeep() const noexcept {
      return CT::Deep<K>;
   }
   
   /// Check if value type is deep                                            
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueDeep() const noexcept {
      return CT::Deep<V>;
   }

   /// Check if the key type is a pointer                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeySparse() const noexcept {
      return CT::Sparse<K>;
   }
   
   /// Check if the value type is a pointer                                   
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueSparse() const noexcept {
      return CT::Sparse<V>;
   }

   /// Check if the key type is not a pointer                                 
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsKeyDense() const noexcept {
      return CT::Dense<K>;
   }

   /// Check if the value type is not a pointer                               
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsValueDense() const noexcept {
      return CT::Dense<V>;
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr Size TABLE()::GetKeyStride() const noexcept {
      return sizeof(K); 
   }
   
   /// Get the size of a single value, in bytes                               
   ///   @return the number of bytes a single value contains                  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr Size TABLE()::GetValueStride() const noexcept {
      return sizeof(V); 
   }

   /// Get a raw key entry (const)                                            
   ///   @param index - the key index                                         
   ///   @return a constant reference to the element                          
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr const K& TABLE()::GetRawKey(Offset index) const noexcept {
      return GetKeys().GetRaw()[index];
   }

   /// Get a raw key entry                                                    
   ///   @param index - the key index                                         
   ///   @return a mutable reference to the element                           
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr K& TABLE()::GetRawKey(Offset index) noexcept {
      return GetKeys().GetRaw()[index];
   }

   /// Get a handle to a key                                                  
   ///   @param index - the key index                                         
   ///   @return the handle                                                   
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr Handle<K> TABLE()::GetKeyHandle(Offset index) noexcept {
      return GetKeys().GetHandle(index);
   }

   /// Get a raw value entry (const)                                          
   ///   @param index - the value index                                       
   ///   @return a constant reference to the element                          
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr const V& TABLE()::GetRawValue(Offset index) const noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a raw value entry                                                  
   ///   @param index - the value index                                       
   ///   @return a mutable reference to the element                           
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr V& TABLE()::GetRawValue(Offset index) noexcept {
      return GetValues().GetRaw()[index];
   }
   
   /// Get a handle to a value                                                
   ///   @param index - the value index                                       
   ///   @return the handle                                                   
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr Handle<V> TABLE()::GetValueHandle(Offset index) noexcept {
      return GetValues().GetHandle(index);
   }

   /// Get the size of all pairs, in bytes                                    
   ///   @return the total amount of initialized bytes                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   constexpr Size TABLE()::GetByteSize() const noexcept {
      return sizeof(Pair) * GetCount(); 
   }

   /// Get the key meta data                                                  
   /// Also implicitly initializes the internal key type                      
   ///   @attention this shouldn't be called on static initialization time    
   ///   @return the meta definition of the key type                          
   TABLE_TEMPLATE() LANGULUS(INLINED)
   DMeta TABLE()::GetKeyType() const {
      mKeys.mType = MetaData::Of<K>();
      return mKeys.mType;
   }

   /// Get the value meta data                                                
   /// Also implicitly initializes the internal key type                      
   ///   @attention this shouldn't be called on static initialization time    
   ///   @return the meta definition of the value type                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   DMeta TABLE()::GetValueType() const {
      mValues.mType = MetaData::Of<V>();
      return mValues.mType;
   }

   /// Check if key type exactly matches another                              
   TABLE_TEMPLATE()
   template<class ALT_K>
   LANGULUS(INLINED)
   constexpr bool TABLE()::KeyIs() const noexcept {
      return CT::Same<K, ALT_K>;
   }

   /// Check if value type exactly matches another                            
   TABLE_TEMPLATE()
   template<class ALT_V>
   LANGULUS(INLINED)
   constexpr bool TABLE()::ValueIs() const noexcept {
      return CT::Same<V, ALT_V>;
   }

   /// Copy-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (const TPair<K, V>& rhs) {
      return operator << (Langulus::Copy(rhs));
   }

   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (TPair<K, V>&& rhs) {
      return operator << (Langulus::Move(rhs));
   }
   
   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (S&& rhs) noexcept requires (CT::Pair<TypeOf<S>>) {
      Insert(S::Nest(rhs.mValue.mKey), S::Nest(rhs.mValue.mValue));
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
   TABLE_TEMPLATE() LANGULUS(INLINED)
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
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Size TABLE()::RequestValuesSize(const Count count) noexcept {
      Size valueByteSize = count * sizeof(V);
      if constexpr (CT::Sparse<V>)
         valueByteSize *= 2;
      return valueByteSize;
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   TABLE_TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Reserve(const Count& count) {
      AllocateInner(
         Roof2(count < MinimalAllocation ? MinimalAllocation : count)
      );
   }

   /// Allocate a fresh set keys and values (for internal use only)           
   ///   @attention doesn't initialize anything, but the memory state         
   ///   @attention doesn't modify count, doesn't set info sentinel           
   ///   @attention assumes count is a power-of-two                           
   ///   @param count - the new number of pairs                               
   TABLE_TEMPLATE()
   void TABLE()::AllocateFresh(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      mKeys.mEntry = Allocator::Allocate(keyAndInfoSize);
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      const auto valueByteSize = RequestValuesSize(count);
      mValues.mEntry = Allocator::Allocate(valueByteSize);

      if (!mValues.mEntry) {
         Allocator::Deallocate(mKeys.mEntry);
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate, "Out of memory");
      }

      mValues.mRaw = mValues.mEntry->GetBlockStart();
      mKeys.mReserved = mValues.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = mKeys.mEntry->GetBlockStart();
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
   }

   /// Allocate or reallocate key and info array                              
   ///   @attention assumes count is a power-of-two                           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   TABLE_TEMPLATE()
   template<bool REUSE>
   void TABLE()::AllocateData(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      auto oldInfo = mInfo;
      const auto oldCount = GetReserved();
      const auto oldInfoEnd = oldInfo + oldCount;

      // Allocate new keys                                              
      const Block oldKeys {mKeys};
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      if constexpr (REUSE)
         mKeys.mEntry = Allocator::Reallocate(keyAndInfoSize, mKeys.mEntry);
      else {
         mKeys.mType = MetaData::Of<K>();
         mKeys.mEntry = Allocator::Allocate(keyAndInfoSize);
      }

      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      // Allocate new values                                            
      const Block oldVals {mValues};
      const auto valueByteSize = RequestValuesSize(count);
      if constexpr (REUSE)
         mValues.mEntry = Allocator::Reallocate(valueByteSize, mValues.mEntry);
      else {
         mValues.mType = MetaData::Of<V>();
         mValues.mEntry = Allocator::Allocate(valueByteSize);
      }

      if (!mValues.mEntry) {
         Allocator::Deallocate(mKeys.mEntry);
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate, "Out of memory");
      }

      mValues.mRaw = mValues.mEntry->GetBlockStart();
      mKeys.mReserved = mValues.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = mKeys.mEntry->GetBlockStart();
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
      // Set the sentinel                                               
      mInfo[count] = 1;

      // Zero or move the info array                                    
      if constexpr (REUSE) {
         // Check if keys were reused                                   
         if (mKeys.mEntry == oldKeys.mEntry) {
            // Keys were reused, but info always moves (null the rest)  
            MoveMemory(mInfo, oldInfo, oldCount);
            ZeroMemory(mInfo + oldCount, count - oldCount);

            // Data was reused, but entries always move if sparse keys  
            if constexpr (CT::Sparse<K>) {
               MoveMemory(
                  mKeys.mRawSparse + count,
                  mKeys.mRawSparse + oldCount,
                  oldCount
               );
            };

            if (mValues.mEntry == oldVals.mEntry) {
               // Both keys and values remain in the same place         
               // Data was reused, but entries always move if sparse val
               if constexpr (CT::Sparse<V>) {
                  MoveMemory(
                     mValues.mRawSparse + count,
                     mValues.mRawSparse + oldCount,
                     oldCount
                  );
               };

               Rehash(count, oldCount);
               return;
            }
         }
         else ZeroMemory(mInfo, count);
      }
      else ZeroMemory(mInfo, count);

      if (oldVals.IsEmpty()) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      mValues.mCount = 0;
      auto key = oldKeys.GetHandle<K>(0);
      auto val = oldVals.GetHandle<V>(0);
      const auto hashmask = GetReserved() - 1;
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            const auto index = HashData(key.Get()).mHash & hashmask;
            InsertInner<false>(index, Abandon(key), Abandon(val));
            key.Destroy();
            val.Destroy();
         }
         
         ++oldInfo;
         ++key;
         ++val;
      }

      // Free the old allocations                                       
      if constexpr (REUSE) {
         // When reusing, keys and values can potentially remain same   
         // Avoid deallocating them if that's the case                  
         if (oldVals.mEntry != mValues.mEntry)
            Allocator::Deallocate(oldVals.mEntry);
         if (oldKeys.mEntry != mKeys.mEntry)
            Allocator::Deallocate(oldKeys.mEntry);
      }
      else if (oldVals.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         // (keys are always present, if values are present)            
         if (oldVals.mEntry->GetUses() > 1)
            oldVals.mEntry->Free();
         else {
            Allocator::Deallocate(oldVals.mEntry);
            Allocator::Deallocate(oldKeys.mEntry);
         }
      }
   }

   /// Rehashes each key and reinserts pair                                   
   ///   @attention assumes counts are a power-of-two number                  
   ///   @param count - the new number of pairs                               
   ///   @param oldCount - the old number of pairs                            
   TABLE_TEMPLATE()
   void TABLE()::Rehash(const Count& count, const Count& oldCount) {
      LANGULUS_ASSUME(DevAssumes, 
         IsPowerOfTwo(count) && IsPowerOfTwo(oldCount),
         "A count is not a power-of-two"
      );

      auto oldKey = GetKeyHandle(0);
      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = count - 1;

      // For each old existing key...                                   
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            const Offset newIndex = HashData(oldKey.Get()).mHash & hashmask;
            if (oldIndex != newIndex) {
               // Immediately move the old pair to the swapper          
               auto oldValue = GetValueHandle(oldIndex);
               HandleLocal<K> keyswap {Abandon(oldKey)};
               HandleLocal<V> valswap {Abandon(oldValue)};

               // Destroy the key, info and value                       
               oldKey.Destroy();
               oldValue.Destroy();
               *oldInfo = 0;
               --mValues.mCount;

               InsertInner<false>(
                  newIndex, Abandon(keyswap), Abandon(valswap)
               );
               /*if (oldIndex == InsertInner<false>(
                  newIndex, Abandon(keyswap), Abandon(valswap))) {
                  continue;
               }*/
            }
         }

         ++oldKey;
         ++oldInfo;
      }
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - number of pairs to allocate                           
   TABLE_TEMPLATE()
   void TABLE()::AllocateInner(const Count& count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() && GetUses() == 1)
         AllocateData<true>(count);
      else
         AllocateData<false>(count);
   }

   /// Inner insertion function                                               
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param key - key to move in                                          
   ///   @param val - value to move in                                        
   ///   @return the index at which item was inserted                         
   TABLE_TEMPLATE()
   template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
   Offset TABLE()::InsertInner(const Offset& start, SK&& key, SV&& val) {
      HandleLocal<K> keyswap {key.Forward()};
      HandleLocal<V> valswap {val.Forward()};

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto& candidate = GetRawKey(index);
            if (keyswap.Compare(candidate)) {
               // Neat, the key already exists - just reassign          
               GetValueHandle(index).Assign(Abandon(valswap));
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetKeyHandle(index).Swap(keyswap);
            GetValueHandle(index).Swap(valswap);
            ::std::swap(attempts, *psl);
         }

         ++attempts;

         if (psl < pslEnd - 1)
            ++psl;
         else
            psl = GetInfo();
      } 

      // If reached, empty slot reached, so put the pair there          
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      const auto index = psl - GetInfo();
      GetKeyHandle(index).New(Abandon(keyswap));
      GetValueHandle(index).New(Abandon(valswap));
      *psl = attempts;
      ++mValues.mCount;
      return index;
   }

   /// Insert a single pair inside table via copy                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(const K& key, const V& value) {
      return Insert(Copy(key), Copy(value));
   }

   /// Insert a single pair inside table via key copy and value move          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(const K& key, V&& value) {
      return Insert(Copy(key), Move(value));
   }

   /// Insert a single pair inside table via key move and value copy          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(K&& key, const V& value) {
      return Insert(Move(key), Copy(value));
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(K&& key, V&& value) {
      return Insert(Move(key), Move(value));
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   template<CT::Semantic SK, CT::Semantic SV>
   LANGULUS(INLINED)
   Count TABLE()::Insert(SK&& key, SV&& value) noexcept requires (CT::Exact<TypeOf<SK>, K> && CT::Exact<TypeOf<SV>, V>) {
      Reserve(GetCount() + 1);
      InsertInner<true>(
         GetBucket(GetReserved() - 1, key.mValue), 
         key.Forward(), value.Forward()
      );
      return 1;
   }

   /// Destroy everything valid inside the map                                
   TABLE_TEMPLATE()
   void TABLE()::ClearInner() {
      auto inf = GetInfo();
      const auto infEnd = GetInfoEnd();
      while (inf != infEnd) {
         if (*inf) {
            const auto offset = inf - GetInfo();
            GetKeyHandle(offset).Destroy();
            GetValueHandle(offset).Destroy();
         }

         ++inf;
      }
   }

   /// Clears all data, but doesn't deallocate, and retains state             
   TABLE_TEMPLATE()
   void TABLE()::Clear() {
      if (IsEmpty())
         return;

      if (mValues.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // Clear all info to zero                                      
         ZeroMemory(mInfo, GetReserved());
         mValues.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         // Notice keys are not dereferenced, we use only value refs    
         // to save on some redundancy                                  
         mInfo = nullptr;
         mValues.mEntry->Free();
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }
   }

   /// Clears all data, state, and deallocates                                
   TABLE_TEMPLATE()
   void TABLE()::Reset() {
      if (!mValues.mEntry)
         return;

      if (mValues.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // No point in resetting info, we'll be deallocating it        
         Allocator::Deallocate(mKeys.mEntry);
         Allocator::Deallocate(mValues.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref values     
         // Notice keys are not dereferenced, we use only value refs    
         // to save on some redundancy                                  
         mValues.mEntry->Free();
      }

      mInfo = nullptr;
      mKeys.ResetState();
      mKeys.ResetMemory();
      mValues.ResetState();
      mValues.ResetMemory();
   }

   /// Safely erases element at a specific index                              
   ///   @param index - the index to remove                                   
   ///   @return 1 if something was removed, or zero of index not found       
   TABLE_TEMPLATE()
   Count TABLE()::RemoveIndex(const Index& index) {
      const auto offset = index.GetOffset();
      if (offset >= GetReserved() || 0 == mInfo[offset])
         return 0;

      RemoveIndex(offset);
      return 1;
   }
   
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this map instance         
   ///   @attention assumes that iterator points to a valid entry             
   ///   @param index - the index to remove                                   
   ///   @return the iterator of the previous element, unless index is the    
   ///           first, or at the end already                                 
   TABLE_TEMPLATE()
   typename TABLE()::Iterator TABLE()::RemoveIndex(const Iterator& index) {
      const auto sentinel = GetReserved();
      auto offset = static_cast<Offset>(index.mInfo - mInfo);
      if (offset >= sentinel)
         return end();

      RemoveIndex(offset--); //TODO what if map shrinks, offset might become invalid? Doesn't shrink for now
      
      while (offset < sentinel && 0 == mInfo[offset])
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
   
   /// Erases element at a specific index                                     
   ///   @attention assumes that index points to a valid entry                
   ///   @param index - the index to remove                                   
   TABLE_TEMPLATE()
   void TABLE()::RemoveIndex(const Offset& index) SAFETY_NOEXCEPT() {
      auto psl = GetInfo() + index;
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid pair");

      const auto pslEnd = GetInfoEnd();
      auto key = GetKeyHandle(index);
      auto val = GetValueHandle(index);

      // Destroy the key, info and value at the start                   
      (key++).Destroy();
      (val++).Destroy();
      *(psl++) = 0;

      // And shift backwards, until a zero or 1 is reached              
      // That way we move every entry that is far from its start        
      // closer to it. Moving is costly, unless you use pointers        
      try_again:
      while (*psl > 1) {
         psl[-1] = (*psl) - 1;

         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wplacement-new"
         #endif

         (key - 1).New(Abandon(key));
         (val - 1).New(Abandon(val));

         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic pop
         #endif

         (key++).Destroy();
         (val++).Destroy();
         *(psl++) = 0;
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd && *GetInfo() > 1) {
         psl = GetInfo();
         key = GetKeyHandle(0);
         val = GetValueHandle(0);

         // Shift first entry to the back                               
         const auto last = mValues.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         GetKeyHandle(last).New(Abandon(key));
         GetValueHandle(last).New(Abandon(val));

         (key++).Destroy();
         (val++).Destroy();
         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mValues.mCount;
   }

   /// Insert a single value or key, either sparse or dense                   
   ///   @tparam T - the type to add, either key or value (deducible)         
   ///   @param element - the address of the element to remove                
   TABLE_TEMPLATE()
   template<class T>
   void TABLE()::Overwrite(T&& from, T& to) noexcept {
      // Remove the old entry                                           
      RemoveInner(&to);

      // Reconstruct the new one in place                               
      new (&to) T {Forward<T>(from)};
   }

   /// Erase a pair via key                                                   
   ///   @param key - the key to search for                                   
   ///   @return the number of removed pairs                                  
   TABLE_TEMPLATE()
   Count TABLE()::RemoveKey(const K& match) {
      // Get the starting index based on the key hash                   
      const auto start = GetBucket(GetReserved() - 1, match);
      auto key = &GetRawKey(start);
      auto info = GetInfo() + start;
      const auto infoEnd = GetInfoEnd();

      while (info != infoEnd) {
         if (*info && *key == match) {
            // Found it                                                 
            RemoveIndex(info - GetInfo());
            return 1;
         }

         ++key; ++info;
      }
      
      // No such key was found                                          
      return 0;
   }

   /// Erase all pairs with a given value                                     
   ///   @param value - the value to search for                               
   ///   @return the number of removed pairs                                  
   TABLE_TEMPLATE()
   Count TABLE()::RemoveValue(const V& match) {
      Count removed {};
      auto value = &GetRawValue(0);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();

      while (info != infoEnd) {
         if (*info && *value == match) {
            // Found it, but there may be more                          
            RemoveIndex(info - GetInfo());
            ++removed;
         }

         ++value; ++info;
      }

      return removed;
   }

   /// If possible reallocates the map to a smaller one                       
   TABLE_TEMPLATE()
   void TABLE()::Compact() {
      TODO();
   }


   ///                                                                        
   ///   SEARCH                                                               
   ///                                                                        

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   TABLE_TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsKey(const K& key) const {
      if (IsEmpty())
         return false;
      return FindIndex(key) != GetReserved();
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Index TABLE()::FindKeyIndex(const K& key) const {
      const auto offset = FindIndex(key);
      return offset != GetReserved() ? Index {offset} : IndexNone;
   }

   /// Search for a value inside the table                                    
   ///   @param value - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   TABLE_TEMPLATE()
   bool TABLE()::ContainsValue(const V& match) const {
      if (IsEmpty())
         return false;

      auto value = &GetRawValue(0);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();

      while (info != infoEnd) {
         if (*info && *value == match)
            return true;

         ++value; ++info;
      }

      return false;
   }

   /// Search for a pair inside the table                                     
   ///   @param pair - the pair to search for                                 
   ///   @return true if pair is found, false otherwise                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::ContainsPair(const Pair& pair) const {
      const auto found = FindIndex(pair.mKey);
      return found != GetReserved() && GetValue(found) == pair.mValue;
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   const TAny<K>& TABLE()::GetKeys() const noexcept {
      return BlockMap::GetKeys<K>();
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TAny<K>& TABLE()::GetKeys() noexcept {
      return BlockMap::GetKeys<K>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   const TAny<V>& TABLE()::GetValues() const noexcept {
      return BlockMap::GetValues<V>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   TAny<V>& TABLE()::GetValues() noexcept {
      return BlockMap::GetValues<V>();
   }

   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const K& key) {
      const auto found = FindIndex(key);
      if (found == GetReserved()) {
         // Key wasn't found, but map is mutable and we can add it      
         if constexpr (CT::Defaultable<V>) {
            // Defaultable value, so adding the key is acceptable       
            Insert(key, V {});
            return GetRawValue(FindIndex(key));
         }
         else LANGULUS_THROW(Construct,
            "Can't implicitly create key"
            " - value is not default-constructible");
      }

      return GetRawValue(found);
   }

   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::At(const K& key) const {
      const auto found = FindIndex(key);
      LANGULUS_ASSERT(found != GetReserved(), OutOfRange, "Key not found");
      return GetRawValue(found);
   }

   /// Get a key at an index                                                  
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the constant key reference                                   
   TABLE_TEMPLATE() LANGULUS(INLINED)
   const K& TABLE()::GetKey(const CT::Index auto& index) const {
      const auto idx = GetKeys().template SimplifyIndex<K, false>(index);
      if (!mInfo[idx])
         LANGULUS_THROW(OutOfRange, "No pair at given index");
      return GetKeys().GetRaw()[idx];
   }

   /// Get a key at an index                                                  
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TABLE_TEMPLATE() LANGULUS(INLINED)
   K& TABLE()::GetKey(const CT::Index auto& index) {
      const auto idx = GetKeys().template SimplifyIndex<K, false>(index);
      if (!mInfo[idx])
         LANGULUS_THROW(OutOfRange, "No pair at given index");
      return GetKeys().GetRaw()[idx];
   }

   /// Get a value at an index                                                
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the constant value reference                                 
   TABLE_TEMPLATE() LANGULUS(INLINED)
   const V& TABLE()::GetValue(const CT::Index auto& index) const {
      const auto idx = GetValues().template SimplifyIndex<V, false>(index);
      if (!mInfo[idx])
         LANGULUS_THROW(OutOfRange, "No pair at given index");
      return GetValues().GetRaw()[idx];
   }

   /// Get a value at an index                                                
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable value reference                                  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   V& TABLE()::GetValue(const CT::Index auto& index) {
      const auto idx = GetValues().template SimplifyIndex<V, false>(index);
      if (!mInfo[idx])
         LANGULUS_THROW(OutOfRange, "No pair at given index");
      return GetValues().GetRaw()[idx];
   }

   /// Get a pair at an index                                                 
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the constant pair reference                                  
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::PairConstRef TABLE()::GetPair(const CT::Index auto& i) const {
      const auto idx = GetValues().template SimplifyIndex<V, false>(i);
      if (!mInfo[idx])
         LANGULUS_THROW(OutOfRange, "No pair at given index");
      return {GetKey(idx), GetValue(idx)};
   }

   /// Get a pair at an index                                                 
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable pair reference                                   
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::PairRef TABLE()::GetPair(const CT::Index auto& i) {
      const auto idx = GetValues().template SimplifyIndex<V, false>(i);
      return {GetKey(idx), GetValue(idx)};
   }

   /// Find the index of a pair by key                                        
   ///   @param key - the key to search for                                   
   ///   @return the index                                                    
   TABLE_TEMPLATE()
   Offset TABLE()::FindIndex(const K& key) const {
      if (IsEmpty())
         return GetReserved();

      // Get the starting index based on the key hash                   
      // Since reserved elements are always power-of-two, we use them   
      // as a mask to the hash, to extract the relevant bucket          
      const auto start = GetBucket(GetReserved() - 1, key);
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd() - 1;
      auto candidate = &GetRawKey(start);

      Count attempts{};
      while (*psl > attempts) {
         if (*candidate != key) {
            // There might be more keys to the right, check them        
            if (psl == pslEnd) UNLIKELY() {
               // By 'to the right' I also mean looped back to start    
               psl = GetInfo();
               candidate = &GetRawKey(0);
            }
            else LIKELY() {
               ++psl;
               ++candidate;
            }

            ++attempts;
            continue;
         }

         // Found                                                       
         return psl - GetInfo();
      }

      // Nothing found, return end offset                               
      return GetReserved();
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const K& key) const {
      return At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const K& key) {
      return At(key);
   }



   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (!*info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::end() noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr, nullptr};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TABLE_TEMPLATE()
   typename TABLE()::ConstIterator TABLE()::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (!*info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         &GetRawKey(offset),
         &GetRawValue(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TABLE_TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr, nullptr};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TABLE_TEMPLATE()
   typename TABLE()::ConstIterator TABLE()::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);

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
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::Last() {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);
      return GetPair(static_cast<Offset>(info - GetInfo()));
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TABLE_TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::Last() const {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);
      return GetPair(static_cast<Offset>(info - GetInfo()));
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(TFunctor<bool(const Block&)>&& f) const {
      Offset i {};
      return GetKeys().ForEachElement([&](const Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all keys inside the map, and perform f() on them (mutable)     
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(TFunctor<bool(Block&)>&& f) {
      Offset i {};
      return GetKeys().ForEachElement([&](Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all keys inside the map, and perform f() on them               
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(TFunctor<void(const Block&)>&& f) const {
      Offset i {};
      return GetKeys().ForEachElement([&](const Block& element) {
         if (mInfo[i++])
            f(element);
      });
   }

   /// Iterate all keys inside the map, and perform f() on them (mutable)     
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachKeyElement(TFunctor<void(Block&)>&& f) {
      Offset i {};
      return GetKeys().ForEachElement([&](Block& element) {
         if (mInfo[i++])
            f(element);
      });
   }

   /// Iterate all values inside the map, and perform f() on them             
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each value block                 
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(TFunctor<bool(const Block&)>&& f) const {
      Offset i {};
      return GetValues().ForEachElement([&](const Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all values inside the map, and perform f() on them             
   ///   @param f - the function to call for each values block                
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(TFunctor<bool(Block&)>&& f) {
      Offset i {};
      return GetValues().ForEachElement([&](Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all values inside the map, and perform f() on them             
   ///   @param f - the function to call for each values block                
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(TFunctor<void(const Block&)>&& f) const {
      Offset i {};
      return GetValues().ForEachElement([&](const Block& element) {
         if (mInfo[i++])
            f(element);
      });
   }

   /// Iterate all values inside the map, and perform f() on them (mutable)   
   ///   @param f - the function to call for each values block                
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::ForEachValueElement(TFunctor<void(Block&)>&& f) {
      Offset i {};
      return GetValues().ForEachElement([&](Block& element) {
         if (mInfo[i++])
            f(element);
      });
   }


   ///                                                                        
   ///   Unordered map iterator                                               
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   TABLE_TEMPLATE()
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

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename ITERATOR()& TABLE()::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (!*++mInfo);
      const auto offset = mInfo - previous;
      mKey += offset;
      mValue += offset;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TABLE_TEMPLATE()
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
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   bool TABLE()::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairRef TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return {*const_cast<K*>(mKey), *const_cast<V*>(mValue)};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairConstRef TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return {*mKey, *mValue};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairRef TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return **this;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename TABLE()::PairConstRef TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return **this;
   }

} // namespace Langulus::Anyness

#undef ITERATOR
#undef TABLE_TEMPLATE
#undef TABLE
