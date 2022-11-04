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

namespace Langulus::Anyness
{

   /// Default construction                                                   
   TABLE_TEMPLATE()
   TABLE()::TUnorderedMap()
      : UnorderedMap {} {
      mKeys.mState = DataState::Typed;
      mKeys.mType = MetaData::Of<Decay<K>>();
      mValues.mState = DataState::Typed;
      mValues.mType = MetaData::Of<Decay<V>>();
      if constexpr (CT::Sparse<K>)
         mKeys.MakeSparse();
      if constexpr (CT::Sparse<V>)
         mValues.MakeSparse();
      if constexpr (CT::Constant<K>)
         mKeys.MakeConst();
      if constexpr (CT::Constant<V>)
         mValues.MakeConst();
   }

   /// Manual construction via an initializer list                            
   ///   @param initlist - the initializer list to forward                    
   TABLE_TEMPLATE()
   TABLE()::TUnorderedMap(::std::initializer_list<Pair> initlist)
      : TUnorderedMap {} {
      Allocate(initlist.size());
      for (auto& it : initlist)
         Insert(*it);
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TABLE_TEMPLATE()
   TABLE()::TUnorderedMap(const TUnorderedMap& other)
      : UnorderedMap {other} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TABLE_TEMPLATE()
   TABLE()::TUnorderedMap(TUnorderedMap&& other) noexcept
      : UnorderedMap {Forward<UnorderedMap>(other)} {}

   /// Shallow-copy construction without referencing                          
   ///   @param other - the disowned table to copy                            
   TABLE_TEMPLATE()
   TABLE()::TUnorderedMap(Disowned<TUnorderedMap>&& other) noexcept
      : UnorderedMap {other.template Forward<UnorderedMap>()} {}

   /// Minimal move construction from abandoned table                         
   ///   @param other - the abandoned table to move                           
   TABLE_TEMPLATE()
   TABLE()::TUnorderedMap(Abandoned<TUnorderedMap>&& other) noexcept
      : UnorderedMap {other.template Forward<UnorderedMap>()} {}

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

         const auto key = GetRawKeys() + lhs;
         const auto rhs = other.FindIndex(*key);
         if constexpr (CT::Sparse<V>) {
            if (rhs == other.GetReserved() || *GetValue(lhs) != *other.GetValue(rhs))
               return false;
         }
         else {
            if (rhs == other.GetReserved() || GetValue(lhs) != other.GetValue(rhs))
               return false;
         }
      }

      return true;
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (TUnorderedMap&& rhs) noexcept {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) Self {Forward<TUnorderedMap>(rhs)};
      return *this;
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (const TUnorderedMap& rhs) {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) Self {rhs};
      return *this;
   }

   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
      TABLE()& TABLE()::operator = (const TPair<K, V>& pair) {
      Clear();
      Insert(pair.mKey, pair.mValue);
      return *this;
   }

   /// Emplace a single pair into a cleared map                               
   ///   @param pair - the pair to emplace                                    
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (TPair<K, V>&& pair) noexcept {
      Clear();
      Insert(Move(pair.mKey), Move(pair.mValue));
      return *this;
   }

   /// Clone all elements in a range                                          
   ///   @param from - source container                                       
   ///   @param to - destination container                                    
   TABLE_TEMPLATE()
   template<class T>
   void TABLE()::CloneInner(const T& from, T& to) const {
      for (Offset i = 0; i < GetReserved(); ++i) {
         if (!mInfo[i])
            continue;

         auto destination = to.CropInner(i, 1, 1);
         from.CropInner(i, 1, 1).Clone(destination);
      }
   }

   /// Clone the table                                                        
   ///   @return the new table                                                
   TABLE_TEMPLATE()
   TABLE() TABLE()::Clone() const {
      if (IsEmpty())
         return {};

      TUnorderedMap result {Disown(*this)};

      // Allocate keys and info                                         
      result.mKeys.mEntry = Allocator::Allocate(mKeys.mEntry->GetAllocatedSize());
      LANGULUS_ASSERT(mKeys.mEntry, Except::Allocate, "Out of memory");

      // Allocate values                                                
      result.mValues.mEntry = Allocator::Allocate(mValues.mEntry->GetAllocatedSize());
      if (!result.mValues.mEntry) {
         Allocator::Deallocate(result.mKeys.mEntry);
         result.mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate, "Out of memory");
      }

      // Clone the info bytes                                           
      result.mKeys.mRaw = result.mKeys.mEntry->GetBlockStart();
      result.mValues.mRaw = result.mValues.mEntry->GetBlockStart();
      result.mInfo = reinterpret_cast<InfoType*>(result.mKeys.mRaw)
         + (mInfo - reinterpret_cast<const InfoType*>(mKeys.mRaw));
      ::std::memcpy(result.mInfo, mInfo, GetReserved() + 1);

      // Clone the keys & values                                        
      CloneInner(GetKeys(), result.GetKeys());
      CloneInner(GetValues(), result.GetValues());
      return Abandon(result);
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsKeyUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsValueUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsKeyTypeConstrained() const noexcept {
      return true;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsValueTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if key type is abstract                                          
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsKeyAbstract() const noexcept {
      return CT::Abstract<K> && !IsKeySparse();
   }
   
   /// Check if value type is abstract                                        
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsValueAbstract() const noexcept {
      return CT::Abstract<V> && !IsValueSparse();
   }
   
   /// Check if key type is default-constructible                             
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsKeyConstructible() const noexcept {
      return CT::Defaultable<K>;
   }
   
   /// Check if value type is default-constructible                           
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsValueConstructible() const noexcept {
      return CT::Defaultable<V>;
   }
   
   /// Check if key type is deep                                              
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsKeyDeep() const noexcept {
      return CT::Deep<K>;
   }
   
   /// Check if value type is deep                                            
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsValueDeep() const noexcept {
      return CT::Deep<V>;
   }

   /// Check if the key type is a pointer                                     
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsKeySparse() const noexcept {
      return CT::Sparse<K>;
   }
   
   /// Check if the value type is a pointer                                   
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsValueSparse() const noexcept {
      return CT::Sparse<V>;
   }

   /// Check if the key type is not a pointer                                 
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsKeyDense() const noexcept {
      return CT::Dense<K>;
   }

   /// Check if the value type is not a pointer                               
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsValueDense() const noexcept {
      return CT::Dense<V>;
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   TABLE_TEMPLATE()
   constexpr Size TABLE()::GetKeyStride() const noexcept {
      return sizeof(KeyInner); 
   }
   
   /// Get the size of a single value, in bytes                               
   ///   @return the number of bytes a single value contains                  
   TABLE_TEMPLATE()
   constexpr Size TABLE()::GetValueStride() const noexcept {
      return sizeof(ValueInner); 
   }

   /// Get the raw key array (const)                                          
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRawKeys() const noexcept {
      return GetKeys().GetRaw();
   }

   /// Get the raw key array                                                  
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRawKeys() noexcept {
      return GetKeys().GetRaw();
   }

   /// Get the end of the raw key array                                       
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRawKeysEnd() const noexcept {
      return GetRawKeys() + GetReserved();
   }

   /// Get the raw value array (const)                                        
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRawValues() const noexcept {
      return GetValues().GetRaw();
   }

   /// Get the raw value array                                                
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRawValues() noexcept {
      return GetValues().GetRaw();
   }

   /// Get end of the raw value array                                         
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRawValuesEnd() const noexcept {
      return GetRawValues() + GetReserved();
   }

   /// Get the size of all pairs, in bytes                                    
   ///   @return the total amount of initialized bytes                        
   TABLE_TEMPLATE()
   constexpr Size TABLE()::GetByteSize() const noexcept {
      return sizeof(Pair) * GetCount(); 
   }

   /// Get the key meta data                                                  
   TABLE_TEMPLATE()
   DMeta TABLE()::GetKeyType() const {
      return MetaData::Of<Decay<K>>();
   }

   /// Get the value meta data                                                
   TABLE_TEMPLATE()
   DMeta TABLE()::GetValueType() const {
      return MetaData::Of<Decay<V>>();
   }

   /// Check if key type exactly matches another                              
   TABLE_TEMPLATE()
   template<class ALT_K>
   constexpr bool TABLE()::KeyIs() const noexcept {
      return CT::Same<K, ALT_K>;
   }

   /// Check if value type exactly matches another                            
   TABLE_TEMPLATE()
   template<class ALT_V>
   constexpr bool TABLE()::ValueIs() const noexcept {
      return CT::Same<V, ALT_V>;
   }

   /// Copy-insert a pair inside the map                                      
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator << (const TPair<K, V>& item) {
      Insert(item.mKey, item.mValue);
      return *this;
   }

   /// Move-insert a pair inside the map                                      
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator << (TPair<K, V>&& item) {
      Insert(Move(item.mKey), Move(item.mValue));
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
   TABLE_TEMPLATE()
   Size TABLE()::RequestKeyAndInfoSize(const Count request, Offset& infoStart) noexcept {
      const Size keymemory = request * sizeof(KeyInner);
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + request + 1;
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   TABLE_TEMPLATE()
   void TABLE()::Allocate(const Count& count) {
      AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
   }

   /// Allocate or reallocate key and info array                              
   ///   @attention assumes count is a power-of-two                           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   TABLE_TEMPLATE()
   template<bool REUSE>
   void TABLE()::AllocateKeys(const Count& count) {
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

      LANGULUS_ASSERT(mKeys.mEntry, Except::Allocate, "Out of memory");

      // Allocate new values                                            
      const Block oldValues {mValues};
      if constexpr (REUSE)
         mValues.mEntry = Allocator::Reallocate(count * sizeof(ValueInner), mValues.mEntry);
      else
         mValues.mEntry = Allocator::Allocate(count * sizeof(ValueInner));

      if (!mValues.mEntry) {
         Allocator::Deallocate(mKeys.mEntry);
         mKeys.mEntry = nullptr;
         LANGULUS_THROW(Allocate, "Out of memory");
      }

      mValues.mRaw = mValues.mEntry->GetBlockStart();
      mValues.mReserved = count;

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
            ::std::memmove(mInfo, oldInfo, oldCount);
            ::std::memset(mInfo + oldCount, 0, count - oldCount);

            if (mValues.mEntry == oldValues.mEntry) {
               // Both keys and values remain in the same place         
               Rehash(count, oldCount);
               return;
            }
         }
         else ::std::memset(mInfo, 0, count);
      }
      else ::std::memset(mInfo, 0, count);

      if (oldValues.IsEmpty()) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      mValues.mCount = 0;
      auto key = oldKeys.mEntry->As<KeyInner>();
      auto value = oldValues.mEntry->As<ValueInner>();
      const auto hashmask = GetReserved() - 1;
      while (oldInfo != oldInfoEnd) {
         if (!*(oldInfo++)) {
            ++key; ++value;
            continue;
         }
         
         const auto index = HashData(*key).mHash & hashmask;
         InsertInner<false, false>(index, Move(*key), Move(*value));
         RemoveInner(key++);
         RemoveInner(value++);
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

   /// Similar to insertion, but rehashes each key                            
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - the new number of pairs                               
   TABLE_TEMPLATE()
   void TABLE()::Rehash(const Count& count, const Count& oldCount) {
      auto oldKey = GetRawKeys();
      auto oldInfo = GetInfo();
      const auto oldKeyEnd = oldKey + oldCount;
      const auto hashmask = count - 1;

      // For each old existing key...                                   
      while (oldKey != oldKeyEnd) {
         if (!*oldInfo) {
            ++oldKey; ++oldInfo;
            continue;
         }

         // Rehash and check if hashes match                            
         const Offset oldIndex = oldInfo - GetInfo();
         const Offset newIndex = HashData(*oldKey).mHash & hashmask;
         if (oldIndex != newIndex) {
            // Immediately move the old pair to the swapper             
            auto oldValue = &GetValue(oldIndex);
            KeyInner keyswap {Move(*oldKey)};
            ValueInner valswap {Move(*oldValue)};
            RemoveIndex(oldIndex);
            if (oldIndex == InsertInner<false, false>(newIndex, Move(keyswap), Move(valswap))) {
               // Index might still end up at its old index, make sure  
               // we don't loop forever in that case                    
               ++oldKey;
               ++oldInfo;
            }

            // Notice iterators are not incremented                     
            continue;
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
         AllocateKeys<true>(count);
      else
         AllocateKeys<false>(count);
   }

   /// Inner insertion function                                               
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @tparam KEEP - false if you want to abandon-construct, true to move  
   ///   @param start - the starting index                                    
   ///   @param key - key to move in                                          
   ///   @param value - value to move in                                      
   ///   @return the index at which item was inserted                         
   TABLE_TEMPLATE()
   template<bool CHECK_FOR_MATCH, bool KEEP>
   Offset TABLE()::InsertInner(const Offset& start, KeyInner&& key, ValueInner&& value) {
      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetRawKeys() + index;
            if (*candidate == key) {
               // Neat, the key already exists - just set value and go  
               if constexpr (!KEEP && CT::AbandonAssignable<ValueInner>)
                  GetValue(index) = Abandon(value);
               else if constexpr (CT::Movable<ValueInner>)
                  GetValue(index) = Move(value);
               else if constexpr (CT::Copyable<ValueInner>)
                  GetValue(index) = value;
               else LANGULUS_ERROR("Can't assign value");
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            ::std::swap(GetKey(index), key);
            ::std::swap(GetValue(index), value);
            ::std::swap(attempts, *psl);
         }

         ++attempts;

         if (psl < pslEnd - 1) LIKELY() {
            ++psl;
         }
         else UNLIKELY() {
            // Wrap around and start from the beginning                 
            psl = GetInfo();
         }
      }

      // If reached, empty slot reached, so put the pair there          
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      const auto index = psl - GetInfo();
      if constexpr (!KEEP && CT::AbandonMakable<KeyInner>)
         new (&GetKey(index)) KeyInner {Abandon(key)};
      else if constexpr (CT::MoveMakable<KeyInner>)
         new (&GetKey(index)) KeyInner {Move(key)};
      else if constexpr (CT::CopyMakable<KeyInner>)
         new (&GetKey(index)) KeyInner {key};
      else LANGULUS_ERROR("Can't instantiate key");

      if constexpr (!KEEP && CT::AbandonMakable<ValueInner>)
         new (&GetValue(index)) ValueInner {Abandon(value)};
      else if constexpr (CT::MoveMakable<ValueInner>)
         new (&GetValue(index)) ValueInner {Move(value)};
      else if constexpr (CT::CopyMakable<ValueInner>)
         new (&GetValue(index)) ValueInner {value};
      else LANGULUS_ERROR("Can't instantiate value");

      *psl = attempts;
      ++mValues.mCount;
      return index;
   }

   /// Get the bucket index, depending on key hash                            
   ///   @param key - the key to hash                                         
   ///   @return the bucket offset                                            
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE) Offset TABLE()::GetBucket(const K& key) const noexcept {
      return HashData(key).mHash & (GetReserved() - 1);
   }

   /// Insert a single pair inside table via copy                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   Count TABLE()::Insert(const K& key, const V& value) {
      static_assert(CT::Sparse<K> || CT::CopyMakable<K>,
         "Key needs to be copy-constructible, but isn't");
      static_assert(CT::Sparse<V> || CT::CopyMakable<V>,
         "Value needs to be copy-constructible, but isn't");

      Allocate(GetCount() + 1);
      InsertInner<true, false>(GetBucket(key), KeyInner {key}, ValueInner {value});
      return 1;
   }

   /// Insert a single pair inside table via key copy and value move          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   Count TABLE()::Insert(const K& key, V&& value) {
      static_assert(CT::Sparse<K> || CT::CopyMakable<K>,
         "Key needs to be copy-constructible, but isn't");
      static_assert(CT::Sparse<V> || CT::MoveMakable<V>,
         "Value needs to be move-constructible, but isn't");

      Allocate(GetCount() + 1);
      if constexpr (CT::Same<V, ValueInner>)
         InsertInner<true, true>(GetBucket(key), KeyInner {key}, Forward<V>(value));
      else
         InsertInner<true, true>(GetBucket(key), KeyInner {key}, ValueInner {Forward<V>(value)});
      return 1;
   }

   /// Insert a single pair inside table via key move and value copy          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   Count TABLE()::Insert(K&& key, const V& value) {
      static_assert(CT::Sparse<K> || CT::MoveMakable<K>,
         "Key needs to be move-constructible, but isn't");
      static_assert(CT::Sparse<V> || CT::CopyMakable<V>,
         "Value needs to be copy-constructible, but isn't");

      Allocate(GetCount() + 1);
      if constexpr (CT::Same<K, KeyInner>)
         InsertInner<true, true>(GetBucket(key), Forward<K>(key), ValueInner {value});
      else
         InsertInner<true, true>(GetBucket(key), KeyInner {Forward<K>(key)}, ValueInner {value});
      return 1;
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   Count TABLE()::Insert(K&& key, V&& value) {
      static_assert(CT::Sparse<K> || CT::MoveMakable<K>,
         "Key needs to be move-constructible, but isn't");
      static_assert(CT::Sparse<V> || CT::MoveMakable<V>,
         "Value needs to be move-constructible, but isn't");

      Allocate(GetCount() + 1);
      if constexpr (CT::Same<K, KeyInner>) {
         if constexpr (CT::Same<V, ValueInner>)
            InsertInner<true, true>(GetBucket(key), Forward<K>(key), Forward<V>(value));
         else
            InsertInner<true, true>(GetBucket(key), Forward<K>(key), ValueInner {Forward<V>(value)});
      }
      else {
         if constexpr (CT::Same<V, ValueInner>)
            InsertInner<true, true>(GetBucket(key), KeyInner {Forward<K>(key)}, Forward<V>(value));
         else
            InsertInner<true, true>(GetBucket(key), KeyInner {Forward<K>(key)}, ValueInner {Forward<V>(value)});
      }
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
            RemoveInner(GetRawKeys() + offset);
            RemoveInner(GetRawValues() + offset);
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
         ::std::memset(mInfo, 0, GetReserved());
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
         GetRawKeys() + offset,
         GetRawValues() + offset
      };
   }
   
   /// Erases element at a specific index                                     
   ///   @attention assumes that index points to a valid entry                
   ///   @param index - the index to remove                                   
   TABLE_TEMPLATE()
   void TABLE()::RemoveIndex(const Offset& index) noexcept {
      auto psl = GetInfo() + index;
      const auto pslEnd = GetInfoEnd();
      auto key = GetRawKeys() + index;
      auto value = GetRawValues() + index;

      // Destroy the key, info and value at the start                   
      RemoveInner(key++);
      RemoveInner(value++);
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
            new (key - 1)   KeyInner {Move(*key)};
            new (value - 1) ValueInner {Move(*value)};
         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic pop
         #endif

         RemoveInner(key++);
         RemoveInner(value++);
         *(psl++) = 0;
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd && *GetInfo() > 1) UNLIKELY() {
         psl = GetInfo();
         key = GetRawKeys();
         value = GetRawValues();

         // Shift first entry to the back                               
         const auto last = mValues.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         new (GetRawKeys() + last)		KeyInner {Move(*key)};
         new (GetRawValues() + last)	ValueInner {Move(*value)};

         RemoveInner(key++);
         RemoveInner(value++);
         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mValues.mCount;
   }

   /// Destroy a single value or key, either sparse or dense                  
   ///   @tparam T - the type to remove, either key or value (deducible)      
   ///   @param element - the address of the element to remove                
   TABLE_TEMPLATE()
   template<class T>
   void TABLE()::RemoveInner(T* element) noexcept {
      if constexpr (CT::Destroyable<T>)
         element->~T();
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
      const auto start = GetBucket(match);
      auto key = GetRawKeys() + start;
      auto info = GetInfo() + start;
      const auto keyEnd = GetRawKeysEnd();

      while (key != keyEnd) {
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
      auto value = GetRawValues();
      auto info = GetInfo();
      const auto valueEnd = GetRawValuesEnd();

      while (value != valueEnd) {
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
   TABLE_TEMPLATE()
   bool TABLE()::ContainsKey(const K& key) const {
      if (IsEmpty())
         return false;
      return FindIndex(key) != GetReserved();
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TABLE_TEMPLATE()
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

      auto value = GetRawValues();
      auto info = GetInfo();
      const auto valueEnd = GetRawValuesEnd();

      while (value != valueEnd) {
         if (*info && *value == match)
            return true;

         ++value; ++info;
      }

      return false;
   }

   /// Search for a pair inside the table                                     
   ///   @param pair - the pair to search for                                 
   ///   @return true if pair is found, false otherwise                       
   TABLE_TEMPLATE()
   bool TABLE()::ContainsPair(const Pair& pair) const {
      const auto found = FindIndex(pair.mKey);
      return found != GetReserved() && GetValue(found) == pair.mValue;
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   const TAny<K>& TABLE()::GetKeys() const noexcept {
      return BlockMap::GetKeys<K>();
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   TAny<K>& TABLE()::GetKeys() noexcept {
      return BlockMap::GetKeys<K>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   const TAny<V>& TABLE()::GetValues() const noexcept {
      return BlockMap::GetValues<V>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   TAny<V>& TABLE()::GetValues() noexcept {
      return BlockMap::GetValues<V>();
   }


   /// Get a key by an unsafe offset (const)                                  
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the key                                       
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetKey(const Offset& i) const noexcept {
      return GetRawKeys()[i];
   }

   /// Get a key by an unsafe offset                                          
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the key                                       
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetKey(const Offset& i) noexcept {
      return GetRawKeys()[i];
   }

   /// Get a value by an unsafe offset (const)                                
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetValue(const Offset& i) const noexcept {
      return GetRawValues()[i];
   }

   /// Get a value by an unsafe offset                                        
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetValue(const Offset& i) noexcept {
      return GetRawValues()[i];
   }

   /// Get a pair by an unsafe offset (const)                                 
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetPair(const Offset& i) const noexcept {
      return TPair<const K&, const V&> {GetKey(i), GetValue(i)};
   }

   /// Get a pair by an unsafe offset                                         
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetPair(const Offset& i) noexcept {
      return TPair<K&, V&> {GetKey(i), GetValue(i)};
   }

   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::At(const K& key) {
      auto found = GetRawValues() + FindIndex(key);
      if (found == GetRawValuesEnd()) {
         // Key wasn't found, but map is mutable and we can add it      
         if constexpr (CT::Defaultable<V>) {
            // Defaultable value, so adding the key is acceptable       
            Insert(key, V {});
            return *(GetRawValues() + FindIndex(key));
         }
         else LANGULUS_THROW(Construct, 
            "Can't implicitly create key - value is not default-constructible");
      }

      return *found;
   }

   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::At(const K& key) const {
      auto found = GetRawValues() + FindIndex(key);
      LANGULUS_ASSERT(found != GetRawValuesEnd(),
         Except::OutOfRange, "Key not found");
      return *found;
   }

   /// Get a key by a safe index (const)                                      
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetKey(const Index& index) const {
      return const_cast<TABLE()&>(*this).GetKey(index);
   }

   /// Get a key by a safe index                                              
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetKey(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         Except::OutOfRange, "Bad index");
      return GetKey(offset);
   }

   /// Get a value by a safe index (const)                                    
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetValue(const Index& index) const {
      return const_cast<TABLE()&>(*this).GetValue(index);
   }

   /// Get a value by a safe index                                            
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetValue(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         Except::OutOfRange, "Bad index");
      return GetValue(offset);
   }

   /// Get a pair by a safe index (const)                                     
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetPair(const Index& index) const {
      return const_cast<TABLE()&>(*this).GetPair(index);
   }

   /// Get a pair by a safe index                                             
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::GetPair(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         Except::OutOfRange, "Bad index");
      return GetPair(offset);
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
      const auto start = GetBucket(key);
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd() - 1;
      auto candidate = GetRawKeys() + start;

      const auto keysAreEqual = [](const KeyInner* lhs, const K& rhs) {
         if constexpr (CT::Sparse<K>)
            return *lhs == rhs/* || **lhs == *rhs*/;
         else
            return lhs == &rhs || *lhs == rhs;
      };

      Count attempts{};
      while (*psl > attempts) {
         if (!keysAreEqual(candidate, key)) {
            // There might be more keys to the right, check them        
            if (psl == pslEnd) UNLIKELY() {
               // By 'to the right' I also mean looped back to start    
               psl = GetInfo();
               candidate = GetRawKeys();
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
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::operator[] (const K& key) const {
      return At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::operator[] (const K& key) {
      return At(key);
   }



   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TABLE_TEMPLATE()
   typename TABLE()::Iterator TABLE()::begin() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const TABLE()*>(this)->begin();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TABLE_TEMPLATE()
   typename TABLE()::Iterator TABLE()::end() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const TABLE()*>(this)->end();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TABLE_TEMPLATE()
   typename TABLE()::Iterator TABLE()::last() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const TABLE()*>(this)->last();
      return reinterpret_cast<const Iterator&>(constant);
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
         GetRawKeys() + offset,
         GetRawValues() + offset
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TABLE_TEMPLATE()
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
         GetRawKeys() + offset,
         GetRawValues() + offset
      };
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
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
   TABLE_TEMPLATE()
   Count TABLE()::ForEachKeyElement(TFunctor<bool(Block&)>&& f) {
      Offset i {};
      return GetKeys().ForEachElement([&](Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all keys inside the map, and perform f() on them               
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
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
   TABLE_TEMPLATE()
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
   TABLE_TEMPLATE()
   Count TABLE()::ForEachValueElement(TFunctor<bool(const Block&)>&& f) const {
      Offset i {};
      return GetValues().ForEachElement([&](const Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all values inside the map, and perform f() on them             
   ///   @param f - the function to call for each values block                
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
   Count TABLE()::ForEachValueElement(TFunctor<bool(Block&)>&& f) {
      Offset i {};
      return GetValues().ForEachElement([&](Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all values inside the map, and perform f() on them             
   ///   @param f - the function to call for each values block                
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
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
   TABLE_TEMPLATE()
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
   #define ITERATOR() TABLE()::template TIterator<MUTABLE>

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   TABLE()::TIterator<MUTABLE>::TIterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const KeyInner* key, 
      const ValueInner* value
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
   LANGULUS(ALWAYSINLINE)
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
   LANGULUS(ALWAYSINLINE)
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
   LANGULUS(ALWAYSINLINE)
   bool TABLE()::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename TABLE()::PairRef TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return {*const_cast<KeyInner*>(mKey), *const_cast<ValueInner*>(mValue)};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename TABLE()::PairConstRef TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return {*mKey, *mValue};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename TABLE()::PairRef TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return **this;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename TABLE()::PairConstRef TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return **this;
   }

} // namespace Langulus::Anyness

#undef ITERATOR
#undef TABLE_TEMPLATE
#undef TABLE