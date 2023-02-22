///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockMap.hpp"

namespace Langulus::Anyness
{

   /// Manual construction via an initializer list                            
   ///   @param initlist - the initializer list to forward                    
   template<CT::Data K, CT::Data V>
   BlockMap::BlockMap(::std::initializer_list<TPair<K, V>> initlist)
      : BlockMap {} {
      Mutate<K, V>();
      Allocate(initlist.size());
      for (auto& it : initlist)
         Insert(*it);
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   LANGULUS(ALWAYSINLINE)
   BlockMap::BlockMap(const BlockMap& other)
      : mInfo {other.mInfo}
      , mKeys {other.mKeys}
      , mValues {other.mValues} {
      mValues.Keep();
   }

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   LANGULUS(ALWAYSINLINE)
   BlockMap::BlockMap(BlockMap&& other) noexcept
      : mInfo {other.mInfo}
      , mKeys {other.mKeys}
      , mValues {other.mValues} {
      other.mValues.ResetMemory();
      other.mValues.ResetState();
   }

   /// Semantic copy (block has no ownership, so always just shallow copy)    
   ///   @tparam S - the semantic to use (irrelevant)                         
   ///   @param other - the block to shallow-copy                             
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr BlockMap::BlockMap(S&& other) noexcept requires (CT::Map<TypeOf<S>>)
      : mInfo {other.mValue.mInfo}
      , mKeys {other.mValue.mKeys}
      , mValues {other.mValue.mValues} {
      if constexpr (S::Move && !S::Keep)
         other.mValue.mValues.mEntry = nullptr;
      else if constexpr (!S::Move && !S::Keep)
         mKeys.mEntry = mValues.mEntry = nullptr;
   }

   /// Destroys the map and all it's contents                                 
   inline BlockMap::~BlockMap() {
      if (!mValues.mEntry)
         return;

      if (mValues.mEntry->GetUses() == 1) {
         if (!IsEmpty()) {
            // Remove all used keys and values, they're used only here  
            ClearInner();
         }

         // Deallocate stuff                                            
         Allocator::Deallocate(mKeys.mEntry);
         Allocator::Deallocate(mValues.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref values     
         // Notice how we don't dereference keys, since we use only the 
         // values' references to save on some redundancy               
         mValues.mEntry->Free();
      }
   }

   /// Checks if both tables contain the same entries                         
   /// Order is irrelevant                                                    
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   inline bool BlockMap::operator == (const BlockMap& other) const {
      if (other.GetCount() != GetCount())
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         const auto lhs = info - GetInfo();
         if (!*(info++))
            continue;

         const auto rhs = other.FindIndexUnknown(GetKey(lhs));
         if (rhs == other.GetReserved() || GetValue(lhs) != other.GetValue(rhs))
            return false;
      }

      return true;
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   inline BlockMap& BlockMap::operator = (BlockMap&& rhs) noexcept {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) BlockMap {Forward<BlockMap>(rhs)};
      return *this;
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   inline BlockMap& BlockMap::operator = (const BlockMap& rhs) {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) BlockMap {rhs};
      return *this;
   }

   /// Emplace a single pair into a cleared map                               
   ///   @param pair - the pair to emplace                                    
   ///   @return a reference to this table                                    
   template<CT::Data K, CT::Data V>
   BlockMap& BlockMap::operator = (TPair<K, V>&& pair) noexcept {
      Clear();
      Insert(Move(pair.mKey), Move(pair.mValue));
      return *this;
   }

   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   template<CT::Data K, CT::Data V>
   BlockMap& BlockMap::operator = (const TPair<K, V>& pair) {
      Clear();
      Insert(pair.mKey, pair.mValue);
      return *this;
   }

   /// Clone the table                                                        
   ///   @return the new table                                                
   inline BlockMap BlockMap::Clone() const {
      if (IsEmpty())
         return {};

      BlockMap result {Disown(*this)};

      // Allocate keys and info                                         
      result.mKeys.mEntry = Allocator::Allocate(mKeys.mEntry->GetAllocatedSize());
      LANGULUS_ASSERT(result.mKeys.mEntry, Allocate, "Out of memory");

      // Allocate values                                                
      result.mValues.mEntry = Allocator::Allocate(mValues.mEntry->GetAllocatedSize());
      if (!result.mValues.mEntry) {
         Allocator::Deallocate(result.mKeys.mEntry);
         result.mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate, "Out of memory");
      }

      // Clone the info bytes (including sentinel)                      
      result.mKeys.mRaw = result.mKeys.mEntry->GetBlockStart();
      result.mValues.mRaw = result.mValues.mEntry->GetBlockStart();
      result.mInfo = reinterpret_cast<InfoType*>(result.mKeys.mRaw)
         + (mInfo - reinterpret_cast<const InfoType*>(mKeys.mRaw));
      ::std::memcpy(result.mInfo, mInfo, GetReserved() + 1);

      // Clone the rest                                                 
      auto info = GetInfo();
      const auto infoEnd = info + GetReserved();
      auto fromKey = GetKey(0);
      auto toKey = result.GetKey(0);
      auto fromVal = GetValue(0);
      auto toVal = result.GetValue(0);
      while (info != infoEnd) {
         if (*info) {
            toKey.CallUnknownSemanticConstructors(1, Langulus::Clone(fromKey));
            toVal.CallUnknownSemanticConstructors(1, Langulus::Clone(fromVal));
         }

         ++info;
         toKey.Next();
         fromKey.Next();
         toVal.Next();
         fromVal.Next();
      }

      return Abandon(result);
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   constexpr bool BlockMap::IsKeyUntyped() const noexcept {
      return mKeys.IsUntyped();
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   constexpr bool BlockMap::IsValueUntyped() const noexcept {
      return mValues.IsUntyped();
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   constexpr bool BlockMap::IsKeyTypeConstrained() const noexcept {
      return mKeys.IsTypeConstrained();;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   constexpr bool BlockMap::IsValueTypeConstrained() const noexcept {
      return mValues.IsTypeConstrained();;
   }
   
   /// Check if key type is abstract                                          
   constexpr bool BlockMap::IsKeyAbstract() const noexcept {
      return mKeys.IsAbstract() && mKeys.IsDense();
   }
   
   /// Check if value type is abstract                                        
   constexpr bool BlockMap::IsValueAbstract() const noexcept {
      return mValues.IsAbstract() && mKeys.IsDense();
   }
   
   /// Check if key type is default-constructible                             
   constexpr bool BlockMap::IsKeyConstructible() const noexcept {
      return mKeys.IsDefaultable();
   }
   
   /// Check if value type is default-constructible                           
   constexpr bool BlockMap::IsValueConstructible() const noexcept {
      return mValues.IsDefaultable();
   }
   
   /// Check if key type is deep                                              
   constexpr bool BlockMap::IsKeyDeep() const noexcept {
      return mKeys.IsDeep();
   }
   
   /// Check if value type is deep                                            
   constexpr bool BlockMap::IsValueDeep() const noexcept {
      return mValues.IsDeep();
   }

   /// Check if the key type is a pointer                                     
   constexpr bool BlockMap::IsKeySparse() const noexcept {
      return mKeys.IsSparse();
   }
   
   /// Check if the value type is a pointer                                   
   constexpr bool BlockMap::IsValueSparse() const noexcept {
      return mValues.IsSparse();
   }

   /// Check if the key type is not a pointer                                 
   constexpr bool BlockMap::IsKeyDense() const noexcept {
      return mKeys.IsDense();
   }

   /// Check if the value type is not a pointer                               
   constexpr bool BlockMap::IsValueDense() const noexcept {
      return mValues.IsDense();
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   constexpr Size BlockMap::GetKeyStride() const noexcept {
      return mKeys.GetStride();
   }
   
   /// Get the size of a single value, in bytes                               
   ///   @return the number of bytes a single value contains                  
   constexpr Size BlockMap::GetValueStride() const noexcept {
      return mValues.GetStride();
   }

   /// Get a raw key entry (const)                                            
   ///   @param index - the key index                                         
   ///   @return a constant reference to the element                          
   template<CT::Data K>
   constexpr const K& BlockMap::GetRawKey(Offset index) const noexcept {
      return GetKeys<K>().GetRaw()[index];
   }

   /// Get a raw key entry                                                    
   ///   @param index - the key index                                         
   ///   @return a mutable reference to the element                           
   template<CT::Data K>
   constexpr K& BlockMap::GetRawKey(Offset index) noexcept {
      return GetKeys<K>().GetRaw()[index];
   }

   /// Get a key handle if sparse, or a key pointer                           
   ///   @param index - the key index                                         
   ///   @return the handle                                                   
   template<CT::Data K>
   constexpr decltype(auto) BlockMap::GetKeyHandle(Offset index) noexcept {
      return GetKeys<K>().GetHandle(index);
   }

   /// Get a raw value entry (const)                                          
   ///   @param index - the value index                                       
   ///   @return a constant reference to the element                          
   template<CT::Data V>
   constexpr const V& BlockMap::GetRawValue(Offset index) const noexcept {
      return GetValues<V>().GetRaw()[index];
   }

   /// Get a raw value entry                                                  
   ///   @param index - the value index                                       
   ///   @return a mutable reference to the element                           
   template<CT::Data V>
   constexpr V& BlockMap::GetRawValue(Offset index) noexcept {
      return GetValues<V>().GetRaw()[index];
   }
   
   /// Get a value handle if sparse, or a key pointer                         
   ///   @param index - the value index                                       
   ///   @return the handle                                                   
   template<CT::Data V>
   constexpr decltype(auto) BlockMap::GetValueHandle(Offset index) noexcept {
      return GetValues<V>().GetHandle(index);
   }

   #ifdef LANGULUS_ENABLE_TESTING
      /// Get raw key memory pointer, used only in testing                    
      ///   @return the pointer                                               
      constexpr const void* BlockMap::GetRawKeysMemory() const noexcept {
         return mKeys.mRaw;
      }

      /// Get raw value memory pointer, used only in testing                  
      ///   @return the pointer                                               
      constexpr const void* BlockMap::GetRawValuesMemory() const noexcept {
         return mValues.mRaw;
      }
   #endif

   /// Get the size of all pairs, in bytes                                    
   ///   @return the total amount of initialized bytes                        
   constexpr Size BlockMap::GetByteSize() const noexcept {
      return sizeof(Pair) * GetCount(); 
   }

   /// Get the key meta data                                                  
   inline DMeta BlockMap::GetKeyType() const noexcept {
      return mKeys.GetType();
   }

   /// Get the value meta data                                                
   inline DMeta BlockMap::GetValueType() const noexcept {
      return mValues.GetType();
   }

   /// Check if key type exactly matches another                              
   template<class ALT_K>
   constexpr bool BlockMap::KeyIs() const noexcept {
      return mKeys.Is<ALT_K>();
   }

   /// Check if value type exactly matches another                            
   template<class ALT_V>
   constexpr bool BlockMap::ValueIs() const noexcept {
      return mValues.Is<ALT_V>();
   }

   /// Request a new size of keys and info via the value container            
   /// The memory layout is:                                                  
   ///   [keys for each bucket, including entries, if sparse]                 
   ///         [padding for alignment]                                        
   ///               [info for each bucket]                                   
   ///                     [one sentinel byte for terminating loops]          
   ///   @attention assumes key type has been set                             
   ///   @param count - number of keys to allocate                            
   ///   @param infoStart - [out] the offset at which info bytes start        
   ///   @return the requested byte size                                      
   LANGULUS(ALWAYSINLINE)
   Size BlockMap::RequestKeyAndInfoSize(const Count count, Offset& infoStart) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mKeys.mType, "Key type was not set");
      auto keymemory = count * mKeys.mType->mSize;
      IF_LANGULUS_MANAGED_MEMORY(
         if (mKeys.mType->mIsSparse)
            keymemory *= 2;
      );
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + count + 1;
   }

   /// Request a new size of value container                                  
   ///   @attention assumes value type has been set                           
   ///   @param count - number of values to allocate                          
   ///   @return the requested byte size                                      
   LANGULUS(ALWAYSINLINE)
   Size BlockMap::RequestValuesSize(const Count count) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mValues.mType, "Value type was not set");
      auto valueByteSize = count * mValues.mType->mSize;
      IF_LANGULUS_MANAGED_MEMORY(
         if (mValues.mType->mIsSparse)
            valueByteSize *= 2;
      );
      return valueByteSize;
   }

   /// Get the info array (const)                                             
   ///   @return a pointer to the first element inside the info array         
   inline const BlockMap::InfoType* BlockMap::GetInfo() const noexcept {
      return mInfo;
   }

   /// Get the info array                                                     
   ///   @return a pointer to the first element inside the info array         
   inline BlockMap::InfoType* BlockMap::GetInfo() noexcept {
      return mInfo;
   }

   /// Get the end of the info array                                          
   ///   @return a pointer to the first element inside the info array         
   inline const BlockMap::InfoType* BlockMap::GetInfoEnd() const noexcept {
      return mInfo + GetReserved();
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @tparam K - the key type                                             
   ///   @tparam V - the value type                                           
   template<CT::NotSemantic K, CT::NotSemantic V>
   void BlockMap::Mutate() {
      Mutate(MetaData::Of<K>(), MetaData::Of<V>());
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @param key - the key type                                            
   ///   @param value - the value type                                        
   inline void BlockMap::Mutate(DMeta key, DMeta value) {
      if (!mKeys.mType) {
         // Set a fresh key type                                        
         mKeys.mType = key;
      }
      else {
         // Key type already set, so check compatibility                
         LANGULUS_ASSERT(mKeys.IsExact(key), Mutate,
            "Attempting to mutate type-erased unordered map's key type"
         );
      }

      if (!mValues.mType) {
         // Set a fresh value type                                      
         mValues.mType = value;
      }
      else {
         // Value type already set, so check compatibility              
         LANGULUS_ASSERT(mValues.IsExact(value), Mutate,
            "Attempting to mutate type-erased unordered map's value type"
         );
      }
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   inline void BlockMap::Allocate(const Count& count) {
      AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
   }

   /// Allocate or reallocate key, value, and info array                      
   ///   @attention assumes count is a power-of-two                           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   template<bool REUSE>
   void BlockMap::AllocateData(const Count& count) {
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
      else
         mKeys.mEntry = Allocator::Allocate(keyAndInfoSize);

      LANGULUS_ASSERT(mKeys.mEntry, Allocate,
         "Out of memory on allocating/reallocating keys");

      // Allocate new values                                            
      const Block oldValues {mValues};
      const auto valueByteSize = RequestValuesSize(count);
      if constexpr (REUSE)
         mValues.mEntry = Allocator::Reallocate(valueByteSize, mValues.mEntry);
      else
         mValues.mEntry = Allocator::Allocate(valueByteSize);

      if (!mValues.mEntry) {
         Allocator::Deallocate(mKeys.mEntry);
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate,
            "Out of memory on allocating/reallocating values");
      }

      mValues.mRaw = mValues.mEntry->GetBlockStart();

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = mKeys.mEntry->GetBlockStart();
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
      // Set the sentinel                                               
      mInfo[count] = 1;

      // Zero or move the info array                                    
      if constexpr (REUSE) {
         // Check if keys were reused                                   
         if (mKeys.mEntry == oldKeys.mEntry) {
            // Data was reused, but info always moves (null the rest)   
            ::std::memmove(mInfo, oldInfo, oldCount);
            ::std::memset(mInfo + oldCount, 0, count - oldCount);

            if (mValues.mEntry == oldValues.mEntry) {
               // Both keys and values remain in the same place         
               Rehash(count, oldCount);

               // Make sure reserved count is changed after rehash,     
               // because entries remain at their old places initially  
               mKeys.mReserved = mValues.mReserved = count;
               return;
            }
         }
         else ::std::memset(mInfo, 0, count);
      }
      else ::std::memset(mInfo, 0, count);

      mKeys.mReserved = mValues.mReserved = count;

      if (oldValues.IsEmpty()) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      mValues.mCount = 0;
      auto key = oldKeys.GetElement();
      auto value = oldValues.GetElement();
      const auto hashmask = count - 1;
      while (oldInfo != oldInfoEnd) {
         if (!*(oldInfo++)) {
            key.Next();
            value.Next();
            continue;
         }

         InsertInnerUnknown<false>(
            key.GetHash().mHash & hashmask, 
            Abandon(key), 
            Abandon(value)
         );

         if (!key.IsEmpty())
            key.CallUnknownDestructors();
         else
            key.mCount = 1;

         if (!value.IsEmpty())
            value.CallUnknownDestructors();
         else
            value.mCount = 1;

         key.Next();
         value.Next();
      }

      // Free the old allocations                                       
      if constexpr (REUSE) {
         // When reusing, keys and values can potentially remain same   
         // Avoid deallocating them if that's the case                  
         if (oldValues.mEntry != mValues.mEntry)
            Allocator::Deallocate(oldValues.mEntry);
         if (oldKeys.mEntry != mKeys.mEntry)
            Allocator::Deallocate(oldKeys.mEntry);
      }
      else if (oldValues.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         // (keys are always present, if values are present)            
         if (oldValues.mEntry->GetUses() > 1)
            oldValues.mEntry->Free();
         else {
            Allocator::Deallocate(oldValues.mEntry);
            Allocator::Deallocate(oldKeys.mEntry);
         }
      }
   }

   /// Rehashes and reinserts each pair in the same block                     
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param count - the new number of pairs                               
   ///   @param oldCount - the old number of pairs                            
   inline void BlockMap::Rehash(const Count& count, const Count& oldCount) {
      LANGULUS_ASSUME(DevAssumes, count > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = count - 1;

      // Prepare a set of preallocated swappers                         
      Block keyswap {mKeys.GetState(), GetKeyType()};
      Block valswap {mValues.GetState(), GetValueType()};
      keyswap.AllocateFresh(keyswap.RequestSize(1));
      valswap.AllocateFresh(valswap.RequestSize(1));

      // For each old existing key...                                   
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            auto oldKey = GetKey(oldIndex);
            const Offset newIndex = oldKey.GetHash().mHash & hashmask;
            if (oldIndex != newIndex) {
               // Move key & value to swapper                           
               // No chance of overlap, so do it forwards               
               keyswap.CallUnknownSemanticConstructors<false>(
                  1, Abandon(oldKey));
               valswap.CallUnknownSemanticConstructors<false>(
                  1, Abandon(GetValue(oldIndex)));
               keyswap.mCount = valswap.mCount = 1;

               RemoveIndex(oldIndex);

               InsertInnerUnknown<false>(
                  newIndex, Abandon(keyswap), Abandon(valswap)
               );
               /*if (oldIndex == InsertInnerUnknown<false>(
                  newIndex, Abandon(keyswap), Abandon(valswap))) {
                  // Index might still end up at its old index, make    
                  // sure we don't loop forever in that case            
                  ++oldInfo;
               }*/
            }
         }

         ++oldInfo;
      }

      // Free the allocated swapper memory                              
      keyswap.Free();
      valswap.Free();
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - number of pairs to allocate                           
   inline void BlockMap::AllocateInner(const Count& count) {
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

   /// Get the bucket index, depending on key hash                            
   ///   @param key - the key to hash                                         
   ///   @return the bucket offset                                            
   template<CT::NotSemantic K>
   LANGULUS(ALWAYSINLINE)
   Offset BlockMap::GetBucket(const K& key) const noexcept {
      return HashData(key).mHash & (GetReserved() - 1);
   }

   /// Insert a single pair inside table via copy                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Count BlockMap::Insert(const K& key, const V& value) {
      return Insert(Copy(key), Copy(value));
   }

   /// Insert a single pair inside table via key copy and value move          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Count BlockMap::Insert(const K& key, V&& value) {
      return Insert(Copy(key), Move(value));
   }

   /// Insert a single pair inside table via key move and value copy          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Count BlockMap::Insert(K&& key, const V& value) {
      return Insert(Move(key), Copy(value));
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(ALWAYSINLINE)
   Count BlockMap::Insert(K&& key, V&& value) {
      return Insert(Move(key), Move(value));
   }
   
   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::Semantic SK, CT::Semantic SV>
   Count BlockMap::Insert(SK&& key, SV&& value) {
      using K = TypeOf<SK>;
      using V = TypeOf<SV>;

      Mutate<K, V>();
      Allocate(GetCount() + 1);
      InsertInner<true>(GetBucket(key.mValue), key.Forward(), value.Forward());
      return 1;
   }

   /// Insert a single pair inside table via move (unknown version)           
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::Semantic SK, CT::Semantic SV>
   inline Count BlockMap::InsertUnknown(SK&& key, SV&& val) {
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
   
   /// Inner insertion function                                               
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @tparam SK - key type and semantic (deducible)                       
   ///   @tparam SV - value type and semantic (deducible)                     
   ///   @param start - the starting index                                    
   ///   @param key - key & semantic to insert                                
   ///   @param value - value & semantic to insert                            
   ///   @return the offset at which pair was inserted                        
   template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
   Offset BlockMap::InsertInner(const Offset& start, SK&& key, SV&& value) {
      using K = TypeOf<SK>;
      using V = TypeOf<SV>;
      auto keyswapper = SemanticMakeHandle<K>(key.Forward());
      auto valswapper = SemanticMakeHandle<V>(value.Forward());

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            const auto& candidate = GetRawKey<K>(index);
            if (candidate == keyswapper) {
               // Neat, the key already exists - just set value and go  
               SemanticAssignHandle(GetValueHandle<V>(index), Abandon(valswapper));
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            SwapHandles(GetKeyHandle<K>(index), keyswapper);
            SwapHandles(GetValueHandle<V>(index), valswapper);
            ::std::swap(attempts, *psl);
         }

         ++attempts;

         // Wrap around and start from the beginning if we have to      
         if (psl < pslEnd - 1)
            ++psl;
         else 
            psl = GetInfo();
      }

      // If reached, empty slot reached, so put the pair there          
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      const auto index = psl - GetInfo();
      SemanticNewHandle<K>(GetKeyHandle<K>(index), Abandon(keyswapper));
      SemanticNewHandle<V>(GetValueHandle<V>(index), Abandon(valswapper));

      *psl = attempts;
      ++mValues.mCount;
      return index;
   }
   
   /// Inner insertion function based on reflected move-assignment            
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param key - key to move in                                          
   ///   @param value - value to move in                                      
   template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
   Offset BlockMap::InsertInnerUnknown(const Offset& start, SK&& key, SV&& value) {
      static_assert(CT::Block<TypeOf<SK>>,
         "SK::Type must be a block type");
      static_assert(CT::Block<TypeOf<SV>>,
         "SV::Type must be a block type");

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetKey(index);
            if (candidate == key.mValue) {
               // Neat, the key already exists - just set value and go  
               GetValue(index)
                  .CallUnknownSemanticAssignment(1, value.Forward());

               if constexpr (SV::Move) {
                  value.mValue.CallUnknownDestructors();
                  value.mValue.mCount = 0;
               }

               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetKey(index).SwapUnknown(key.Forward());
            GetValue(index).SwapUnknown(value.Forward());
            ::std::swap(attempts, *psl);
         }

         ++attempts;

         // Wrap around and start from the beginning if needed          
         if (psl < pslEnd - 1)
            ++psl;
         else
            psl = GetInfo();
      }

      // If reached, empty slot reached, so put the pair there	         
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      // We're moving only a single element, so no chance of overlap    
      const auto index = psl - GetInfo();
      GetKey(index)
         .CallUnknownSemanticConstructors(1, key.Forward());
      GetValue(index)
         .CallUnknownSemanticConstructors(1, value.Forward());

      if constexpr (SK::Move) {
         key.mValue.CallUnknownDestructors();
         key.mValue.mCount = 0;
      }

      if constexpr (SV::Move) {
         value.mValue.CallUnknownDestructors();
         value.mValue.mCount = 0;
      }

      *psl = attempts;
      ++mValues.mCount;
      return index;
   }

   /// Copy-insert a templated pair inside the map                            
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   template<CT::Data K, CT::Data V>
   BlockMap& BlockMap::operator << (const TPair<K, V>& item) {
      Insert(item.mKey, item.mValue);
      return *this;
   }

   /// Move-insert a templated pair inside the map                            
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   template<CT::Data K, CT::Data V>
   BlockMap& BlockMap::operator << (TPair<K ,V>&& item) {
      Insert(Move(item.mKey), Move(item.mValue));
      return *this;
   }

   /// Copy-insert a type-erased pair inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   inline BlockMap& BlockMap::operator << (const Pair& item) {
      InsertUnknown(Copy(item.mKey), Copy(item.mValue));
      return *this;
   }

   /// Move-insert a type-erased pair inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   inline BlockMap& BlockMap::operator << (Pair&& item) {
      InsertUnknown(Move(item.mKey), Move(item.mValue));
      return *this;
   }

   /// Destroy everything valid inside the map                                
   ///   @attention assumes there's at least one valid pair                   
   inline void BlockMap::ClearInner() {
      LANGULUS_ASSUME(DevAssumes, !IsEmpty(), "Map is empty");
      auto inf = GetInfo();
      const auto infEnd = GetInfoEnd();
      while (inf != infEnd) {
         if (*inf) {
            const auto offset = inf - GetInfo();
            GetKey(offset).CallUnknownDestructors();
            GetValue(offset).CallUnknownDestructors();
         }

         ++inf;
      }
   }

   /// Clears all data, but doesn't deallocate                                
   inline void BlockMap::Clear() {
      if (IsEmpty())
         return;

      if (mValues.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // Clear all info to zero                                      
         ::std::memset(GetInfo(), 0, GetReserved());
         mValues.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         mInfo = nullptr;
         mValues.mEntry->Free();
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }
   }

   /// Clears all data and deallocates                                        
   inline void BlockMap::Reset() {
      if (mValues.mEntry) {
         if (mValues.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            if (!IsEmpty())
               ClearInner();

            // No point in resetting info, we'll be deallocating it     
            Allocator::Deallocate(mKeys.mEntry);
            Allocator::Deallocate(mValues.mEntry);
         }
         else {
            // Data is used from multiple locations, just deref values  
            mValues.mEntry->Free();
         }

         mInfo = nullptr;
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }

      mKeys.ResetState();
      mValues.ResetState();
   }

   /// Erases element at a specific index                                     
   ///   @attention assumes that offset points to a valid entry               
   ///   @param offset - the index to remove                                  
   inline void BlockMap::RemoveIndex(const Offset& offset) noexcept {
      auto psl = GetInfo() + offset;
      const auto pslEnd = GetInfoEnd();
      auto key = GetKey(offset);
      auto val = GetValue(offset);

      // Destroy the key, info and value at the offset                  
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid pair");
      key.CallUnknownDestructors();
      val.CallUnknownDestructors();

      *(psl++) = 0;
      key.Next();
      val.Next();

      // And shift backwards, until a zero or 1 is reached              
      // That way we move every entry that is far from its start        
      // closer to it. Moving is costly, unless you use pointers        
      try_again:
      while (*psl > 1) {
         psl[-1] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         const_cast<const Block&>(key).Prev()
            .CallUnknownSemanticConstructors(1, Abandon(key));
         const_cast<const Block&>(val).Prev()
            .CallUnknownSemanticConstructors(1, Abandon(val));

         key.CallUnknownDestructors();
         val.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();
         val.Next();
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd && *GetInfo() > 1) UNLIKELY() {
         psl = GetInfo();
         key = GetKey(0);
         val = GetValue(0);

         // Shift first entry to the back                               
         const auto last = mValues.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         GetKey(last)
            .CallUnknownSemanticConstructors(1, Abandon(key));
         GetValue(last)
            .CallUnknownSemanticConstructors(1, Abandon(val));

         key.CallUnknownDestructors();
         val.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();
         val.Next();

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mValues.mCount;
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data K>
   const TAny<K>& BlockMap::GetKeys() const noexcept {
      return reinterpret_cast<const TAny<K>&>(mKeys);
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data K>
   TAny<K>& BlockMap::GetKeys() noexcept {
      return reinterpret_cast<TAny<K>&>(mKeys);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data V>
   const TAny<V>& BlockMap::GetValues() const noexcept {
      return reinterpret_cast<const TAny<V>&>(mValues);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data V>
   TAny<V>& BlockMap::GetValues() noexcept {
      return reinterpret_cast<TAny<V>&>(mValues);
   }

   /// Erase a pair via key                                                   
   ///   @param match - the key to search for                                 
   ///   @return the number of removed pairs                                  
   template<CT::NotSemantic K>
   Count BlockMap::RemoveKey(const K& match) {
      // Get the starting index based on the key hash                   
      const auto start = GetBucket(match);
      auto key = &GetRawKey<K>(start);
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
   ///   @param match - the value to search for                               
   ///   @return the number of removed pairs                                  
   template<CT::NotSemantic V>
   Count BlockMap::RemoveValue(const V& match) {
      Count removed {};
      auto value = &GetRawValue<V>(0);
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
   inline void BlockMap::Compact() {
      //TODO();
   }

   ///                                                                        
   ///   SEARCH                                                               
   ///                                                                        
   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   template<CT::NotSemantic K>
   bool BlockMap::ContainsKey(const K& key) const {
      if (IsEmpty())
         return false;
      return FindIndex(key) != GetReserved();
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   template<CT::NotSemantic K>
   Index BlockMap::FindKeyIndex(const K& key) const {
      const auto offset = FindIndex(key);
      return offset != GetReserved() ? Index {offset} : IndexNone;
   }

   /// Search for a value inside the table                                    
   ///   @param value - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   template<CT::NotSemantic V>
   bool BlockMap::ContainsValue(const V& match) const {
      if (IsEmpty())
         return false;

      auto value = &GetRawValue<V>(0);
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
   template<CT::NotSemantic K, CT::NotSemantic V>
   bool BlockMap::ContainsPair(const TPair<K, V>& pair) const {
      const auto found = FindIndex(pair.mKey);
      return found != GetReserved() && GetValue(found) == pair.mValue;
   }

   /// Get a type-erased key                                                  
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the key, wrapped inside a block                              
   inline Block BlockMap::GetKey(const Offset& i) noexcept {
      Block result {mKeys};
      result.mState += DataState::Static;
      result.mCount = 1;
      result.mReserved = mValues.mReserved;
      result.mRaw += i * mKeys.mType->mSize;
      return result;
   }

   /// Get a type-erased key (const)                                          
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the key, wrapped inside an immutable block                   
   inline Block BlockMap::GetKey(const Offset& i) const noexcept {
      auto result = const_cast<BlockMap*>(this)->GetKey(i);
      result.MakeConst();
      return result;
   }

   /// Get a type-erased value                                                
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the value, wrapped inside a block                            
   inline Block BlockMap::GetValue(const Offset& i) noexcept {
      Block result {mValues};
      result.mState += DataState::Static;
      result.mCount = 1;
      result.mRaw += i * mValues.mType->mSize;
      return result;
   }

   /// Get a type-erased value (const)                                        
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the value, wrapped inside an immutable block                 
   inline Block BlockMap::GetValue(const Offset& i) const noexcept {
      auto result = const_cast<BlockMap*>(this)->GetValue(i);
      result.MakeConst();
      return result;
   }

   /// Get a pair by an unsafe offset (const)                                 
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   inline Pair BlockMap::GetPair(const Offset& i) const noexcept {
      return {GetKey(i), GetValue(i)};
   }

   /// Get a pair by an unsafe offset                                         
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   inline Pair BlockMap::GetPair(const Offset& i) noexcept {
      return {GetKey(i), GetValue(i)};
   }

   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   template<CT::NotSemantic K>
   Block BlockMap::At(const K& key) {
      const auto found = FindIndex(key);
      if (found == GetReserved()) {
         // Key wasn't found, but map is mutable and we can add it      
         LANGULUS_ASSERT(
            mValues.IsSparse() || GetValueType()->mDefaultConstructor != nullptr,
            Construct,
            "Can't implicitly create key - value is not default-constructible"
         );

         auto newk = Block::From(key);
         auto newv = Any::FromMeta(GetValueType(), mValues.GetState());
         newv.template AllocateMore<true>(1);
         InsertUnknown(Copy(newk), Abandon(newv));
         return GetValue(FindIndex(key));
      }

      return GetValue(found);
   }

   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   template<CT::NotSemantic K>
   Block BlockMap::At(const K& key) const {
      const auto found = FindIndex(key);
      LANGULUS_ASSERT(found != GetReserved(), OutOfRange, "Key not found");
      return GetValue(found);
   }

   /// Get a key by a safe index (const)                                      
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   inline Block BlockMap::GetKey(const Index& index) const {
      return const_cast<BlockMap&>(*this).GetKey(index);
   }

   /// Get a key by a safe index                                              
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   inline Block BlockMap::GetKey(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         OutOfRange, "Bad index");
      return GetKey(offset);
   }

   /// Get a value by a safe index (const)                                    
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   inline Block BlockMap::GetValue(const Index& index) const {
      return const_cast<BlockMap&>(*this).GetValue(index);
   }

   /// Get a value by a safe index                                            
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   inline Block BlockMap::GetValue(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         OutOfRange, "Bad index");
      return GetValue(offset);
   }

   /// Get a pair by a safe index (const)                                     
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   inline Pair BlockMap::GetPair(const Index& index) const {
      return const_cast<BlockMap&>(*this).GetPair(index);
   }

   /// Get a pair by a safe index                                             
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   inline Pair BlockMap::GetPair(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         OutOfRange, "Bad index");
      return GetPair(offset);
   }

   /// Find the index of a pair by key                                        
   ///   @param key - the key to search for                                   
   ///   @return the index                                                    
   template<CT::NotSemantic K>
   Offset BlockMap::FindIndex(const K& key) const {
      // Get the starting index based on the key hash                   
      // Since reserved elements are always power-of-two, we use them   
      // as a mask to the hash, to extract the relevant bucket          
      const auto start = GetBucket(key);
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd() - 1;
      auto candidate = &GetRawKey<K>(start);

      Count attempts{};
      while (*psl > attempts) {
         if (*candidate != key) {
            // There might be more keys to the right, check them        
            if (psl == pslEnd) UNLIKELY() {
               // By 'to the right' I also mean looped back to start    
               psl = GetInfo();
               candidate = &GetRawKey<K>(0);
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
   
   /// Find the index of a pair by an unknown type-erased key                 
   ///   @param key - the key to search for                                   
   ///   @return the index                                                    
   inline Offset BlockMap::FindIndexUnknown(const Block& key) const {
      // Get the starting index based on the key hash                   
      // Since reserved elements are always power-of-two, we use them   
      // as a mask to the hash, to extract the relevant bucket          
      const auto start = GetBucket(key);
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd() - 1;
      auto candidate = GetKey(start);
      Count attempts{};
      while (*psl > attempts) {
         if (candidate != key) {
            // There might be more keys to the right, check them        
            if (psl == pslEnd) UNLIKELY() {
               // By 'to the right' I also mean looped back to start    
               psl = GetInfo();
               candidate = GetKey(0);
            }
            else LIKELY() {
               ++psl;
               candidate.Next();
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
   ///   @return a the value wrapped inside an Any                            
   template<CT::NotSemantic K>
   Block BlockMap::operator[] (const K& key) const {
      return At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a the value wrapped inside an Any                            
   template<CT::NotSemantic K>
   Block BlockMap::operator[] (const K& key) {
      return At(key);
   }

   /// Get the number of inserted pairs                                       
   ///   @return the number of inserted pairs                                 
   constexpr Count BlockMap::GetCount() const noexcept {
      return mValues.GetCount();
   }

   /// Get the number of allocated pairs                                      
   ///   @return the number of allocated pairs                                
   constexpr Count BlockMap::GetReserved() const noexcept {
      return mValues.GetReserved();
   }

   /// Check if there are any pairs in this map                               
   ///   @return true if there's at least one pair available                  
   constexpr bool BlockMap::IsEmpty() const noexcept {
      return mValues.IsEmpty();
   }

   /// Check if the map has been allocated                                    
   ///   @return true if the map uses dynamic memory                          
   constexpr bool BlockMap::IsAllocated() const noexcept {
      return mValues.IsAllocated();
   }

   /// Check if the memory for the table is owned by us                       
   /// This is always true, since the map can't be initialized with outside   
   /// memory - the memory layout requirements are too strict to allow for it 
   ///   @return true                                                         
   constexpr bool BlockMap::HasAuthority() const noexcept {
      return IsAllocated();
   }

   /// Get the number of references for the allocated memory                  
   ///   @attention always returns zero if we don't have authority            
   ///   @return the number of references                                     
   constexpr Count BlockMap::GetUses() const noexcept {
      return mValues.GetUses();
   }

   /// Get hash of the map contents                                           
   ///   @attention the hash is not cached, so this is a slow operation       
   ///   @return the hash                                                     
   inline Hash BlockMap::GetHash() const {
      TAny<Hash> hashes;
      for (auto pair : *this)
         hashes << pair.GetHash();
      return hashes.GetHash();
   }



   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   inline typename BlockMap::Iterator BlockMap::begin() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const BlockMap*>(this)->begin();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   inline typename BlockMap::Iterator BlockMap::end() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const BlockMap*>(this)->end();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   inline typename BlockMap::Iterator BlockMap::last() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const BlockMap*>(this)->last();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   inline typename BlockMap::ConstIterator BlockMap::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (!*info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         GetKey(offset),
         GetValue(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   inline typename BlockMap::ConstIterator BlockMap::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), {}, {}};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   inline typename BlockMap::ConstIterator BlockMap::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
         GetKey(offset),
         GetValue(offset)
      };
   }

   /// Execute functions for each element inside container                    
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function types (deducible)                           
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool MUTABLE, bool REVERSE, class F>
   Count BlockMap::ForEachSplitter(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      return ForEachInner<R, A, REVERSE, MUTABLE>(part, Forward<F>(call));
   }

   /// Execute functions for each element inside container, nested for any    
   /// contained deep containers                                              
   ///   @tparam SKIP - set to false, to execute F for containers, too        
   ///                  set to true, to execute only for non-deep elements    
   ///   @tparam MUTABLE - whether or not a change to container is allowed    
   ///                     while iterating                                    
   ///   @tparam F - the function type (deducible                             
   ///   @param call - the instance of the function F to call                 
   ///   @return the number of called functions                               
   template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
   Count BlockMap::ForEachDeepSplitter(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      if constexpr (CT::Deep<A>) {
         // If argument type is deep                                    
         return ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(
            part, Forward<F>(call));
      }
      else if constexpr (CT::Constant<A>) {
         // Any other type is wrapped inside another ForEachDeep call   
         return ForEachDeep<SKIP, MUTABLE>(part, [&call](const Block& block) {
            block.ForEach(Forward<F>(call));
         });
      }
      else {
         // Any other type is wrapped inside another ForEachDeep call   
         return ForEachDeep<SKIP, MUTABLE>(part, [&call](Block& block) {
            block.ForEach(Forward<F>(call));
         });
      }
   }

   /// Iterate and execute call for each element                              
   ///   @param call - the function to execute for each element of type T     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
   Count BlockMap::ForEachInner(Block& part, TFunctor<R(A)>&& call) {
      if (IsEmpty() || !part.mType->CastsTo<A, true>())
         return 0;
       
      constexpr bool HasBreaker = CT::Bool<R>;
      Count done {};
      Count index {};

      while (index < mValues.mReserved) {
         if (!mInfo[index]) {
            ++index;
            continue;
         }

         if constexpr (REVERSE) {
            if constexpr (HasBreaker) {
               if (!call(part.Get<A>(mValues.mReserved - index - 1)))
                  return ++done;
            }
            else call(part.Get<A>(mValues.mReserved - index - 1));
         }
         else {
            if constexpr (HasBreaker) {
               if (!call(part.Get<A>(index)))
                  return ++done;
            }
            else call(part.Get<A>(index));
         }

         ++index;
         ++done;
      }

      return done;
   }
   
   /// Iterate and execute call for each element                              
   ///   @param call - the function to execute for each element of type T     
   ///   @return the number of executions that occured                        
   template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
   Count BlockMap::ForEachDeepInner(Block& part, TFunctor<R(A)>&& call) {
      constexpr bool HasBreaker = CT::Bool<R>;
      auto count {part.GetCountDeep()};
      Count index {};
      while (index < count) {
         auto block = ReinterpretCast<A>(part.GetBlockDeep(index));//TODO custom checked getblockdeep here, write tests and you'll see
         if constexpr (SKIP) {
            // Skip deep/empty sub blocks                               
            if (block->IsDeep() || block->IsEmpty()) {
               ++index;
               continue;
            }
         }

         if constexpr (HasBreaker) {
            if (!call(*block))
               return ++index;
         }
         else call(*block);

         ++index;
      }

      return index;
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool MUTABLE, class F>
   Count BlockMap::ForEachElement(Block& part, F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Block<A>,
         "Function argument must be a CT::Block type");
      static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
         "Non constant iterator for constant memory block");

      Count index {};
      while (index < GetReserved()) {
         if (!mInfo[index]) {
            ++index;
            continue;
         }

         A block = part.GetElement(index);
         if constexpr (CT::Bool<R>) {
            if (!call(block))
               return ++index;
         }
         else call(block);

         ++index;
      }

      return index;
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool MUTABLE, class F>
   Count BlockMap::ForEachKeyElement(F&& f) {
      return ForEachElement<MUTABLE>(mKeys, Forward<F>(f));
   }

   template<class F>
   Count BlockMap::ForEachKeyElement(F&& f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachKeyElement<false>(Forward<F>(f));
   }

   /// Iterate all values inside the map, and perform f() on them             
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   template<bool MUTABLE, class F>
   Count BlockMap::ForEachValueElement(F&& f) {
      return ForEachElement<MUTABLE>(mValues, Forward<F>(f));
   }

   template<class F>
   Count BlockMap::ForEachValueElement(F&& f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachValueElement<false>(Forward<F>(f));
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool MUTABLE, class... F>
   Count BlockMap::ForEachKey(F&&... f) {
      return (... || ForEachSplitter<MUTABLE, false>(mKeys, Forward<F>(f)));
   }

   template<class... F>
   Count BlockMap::ForEachKey(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachKey<false>(Forward<F>(f)...);
   }

   template<bool MUTABLE, class... F>
   Count BlockMap::ForEachValue(F&&... f) {
      return (... || ForEachSplitter<MUTABLE, false>(mValues, Forward<F>(f)));
   }

   template<class... F>
   Count BlockMap::ForEachValue(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachValue<false>(Forward<F>(f)...);
   }

   template<bool MUTABLE, class... F>
   Count BlockMap::ForEachKeyRev(F&&... f) {
      return (... || ForEachSplitter<MUTABLE, true>(mKeys, Forward<F>(f)));
   }

   template<class... F>
   Count BlockMap::ForEachKeyRev(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachKeyRev<false>(Forward<F>(f)...);
   }

   template<bool MUTABLE, class... F>
   Count BlockMap::ForEachValueRev(F&&... f) {
      return (... || ForEachSplitter<MUTABLE, true>(mValues, Forward<F>(f)));
   }

   template<class... F>
   Count BlockMap::ForEachValueRev(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachValueRev<false>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   Count BlockMap::ForEachKeyDeep(F&&... f) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, false>(mKeys, Forward<F>(f)));
   }

   template<bool SKIP, class... F>
   Count BlockMap::ForEachKeyDeep(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachKeyDeep<SKIP, false>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   Count BlockMap::ForEachValueDeep(F&&... f) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, false>(mValues, Forward<F>(f)));
   }

   template<bool SKIP, class... F>
   Count BlockMap::ForEachValueDeep(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachValueDeep<SKIP, false>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   Count BlockMap::ForEachKeyDeepRev(F&&... f) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, true>(mKeys, Forward<F>(f)));
   }

   template<bool SKIP, class... F>
   Count BlockMap::ForEachKeyDeepRev(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachKeyDeepRev<SKIP, false>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   Count BlockMap::ForEachValueDeepRev(F&&... f) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, true>(mValues, Forward<F>(f)));
   }

   template<bool SKIP, class... F>
   Count BlockMap::ForEachValueDeepRev(F&&... f) const {
      return const_cast<BlockMap&>(*this)
         .template ForEachValueDeepRev<SKIP, false>(Forward<F>(f)...);
   }


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
      BlockMap::TIterator<MUTABLE>::TIterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const Block& key, 
      const Block& value
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
      , mKey {key}
      , mValue {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename BlockMap::TIterator<MUTABLE>& BlockMap::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (!*++mInfo);
      const auto offset = mInfo - previous;
      mKey.mRaw += offset * mKey.GetStride();
      mValue.mRaw += offset * mValue.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename BlockMap::TIterator<MUTABLE> BlockMap::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   bool BlockMap::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   Pair BlockMap::TIterator<MUTABLE>::operator * () const noexcept {
      return {Disown(mKey), Disown(mValue)};
   }

} // namespace Langulus::Anyness
