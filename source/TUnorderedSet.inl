///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedSet.hpp"

#define TABLE_TEMPLATE() template<CT::Data T>
#define TABLE() TUnorderedSet<T>

namespace Langulus::Anyness
{

   /// Default construction                                                   
   TABLE_TEMPLATE()
   constexpr TABLE()::TUnorderedSet()
      : UnorderedSet {} {
      mKeys.mState = DataState::Typed;
      if constexpr (CT::Constant<T>)
         mKeys.MakeConst();
   }

   /// Manual construction via an initializer list                            
   ///   @param initlist - the initializer list to forward                    
   TABLE_TEMPLATE()
   TABLE()::TUnorderedSet(::std::initializer_list<T> initlist)
      : TUnorderedSet {} {
      Allocate(initlist.size());
      for (auto& it : initlist)
         Insert(*it);
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TABLE_TEMPLATE()
   TABLE()::TUnorderedSet(const TUnorderedSet& other)
      : UnorderedSet {other} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TABLE_TEMPLATE()
   TABLE()::TUnorderedSet(TUnorderedSet&& other) noexcept
      : UnorderedSet {Forward<UnorderedSet>(other)} {}

   /// Destroys the map and all it's contents                                 
   TABLE_TEMPLATE()
   TABLE()::~TUnorderedSet() {
      if (!mKeys.mEntry)
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         // This is a statically-optimized equivalent                   
         ClearInner();

         // Deallocate stuff                                            
         Allocator::Deallocate(mKeys.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref values     
         mKeys.mEntry->Free();
      }

      mKeys.mEntry = nullptr;
   }

   /// Checks if both tables contain the same entries                         
   /// Order is irrelevant                                                    
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   TABLE_TEMPLATE()
   bool TABLE()::operator == (const TUnorderedSet& other) const {
      if (other.GetCount() != GetCount())
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         const auto lhs = info - GetInfo();
         if (!*(info++))
            continue;

         const auto& key = Get(lhs);
         const auto rhs = other.FindIndex(key);
         if (rhs == other.GetReserved() || key != other.Get(rhs))
            return false;
      }

      return true;
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (TUnorderedSet&& rhs) noexcept {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) Self {Forward<TUnorderedSet>(rhs)};
      return *this;
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (const TUnorderedSet& rhs) {
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
      TABLE()& TABLE()::operator = (const T& element) {
      Clear();
      Insert(element);
      return *this;
   }

   /// Emplace a single pair into a cleared map                               
   ///   @param pair - the pair to emplace                                    
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (T&& element) noexcept {
      Clear();
      Insert(Forward<T>(element));
      return *this;
   }

   /// Clone all elements in a range                                          
   ///   @param from - source container                                       
   ///   @param to - destination container                                    
   TABLE_TEMPLATE()
   template<class ALT_T>
   void TABLE()::CloneInner(const ALT_T& from, ALT_T& to) const {
      for (Offset i = 0; i < GetReserved(); ++i) {
         if (!mInfo[i])
            continue;

         auto destination = to.CropInner(i, 1);
         from.CropInner(i, 1).Clone(destination);
      }
   }

   /// Clone the table                                                        
   ///   @return the new table                                                
   TABLE_TEMPLATE()
   TABLE() TABLE()::Clone() const {
      if (IsEmpty())
         return {};

      TUnorderedSet result {Disown(*this)};

      // Allocate keys and info                                         
      result.mKeys.mEntry = Allocator::Allocate(mKeys.mEntry->GetAllocatedSize());
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      // Clone the info bytes                                           
      result.mKeys.mRaw = result.mKeys.mEntry->GetBlockStart();
      result.mInfo = reinterpret_cast<InfoType*>(result.mKeys.mRaw)
          + (mInfo - reinterpret_cast<const InfoType*>(mKeys.mRaw));
      ::std::memcpy(result.mInfo, mInfo, GetReserved() + 1);

      // Clone the keys & values                                        
      CloneInner(GetValues(), result.GetValues());
      return Abandon(result);
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsUntyped() const noexcept {
      return false;
   }

   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if key type is abstract                                          
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsAbstract() const noexcept {
      return CT::Abstract<T>;
   }
   
   /// Check if key type is default-constructible                             
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsConstructible() const noexcept {
      return CT::Defaultable<T>;
   }
   
   /// Check if key type is deep                                              
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsDeep() const noexcept {
      return CT::Deep<T>;
   }

   /// Check if the key type is a pointer                                     
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsSparse() const noexcept {
      return CT::Sparse<T>;
   }

   /// Check if the key type is not a pointer                                 
   TABLE_TEMPLATE()
   constexpr bool TABLE()::IsDense() const noexcept {
      return CT::Dense<T>;
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   TABLE_TEMPLATE()
   constexpr Size TABLE()::GetStride() const noexcept {
      return sizeof(ValueInner); 
   }

   /// Get the raw key array (const)                                          
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRaw() const noexcept {
      return GetValues().GetRaw();
   }

   /// Get the raw key array                                                  
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRaw() noexcept {
      return GetValues().GetRaw();
   }

   /// Get the end of the raw key array                                       
   TABLE_TEMPLATE()
   constexpr auto TABLE()::GetRawEnd() const noexcept {
      return GetRaw() + GetReserved();
   }

   /// Get the size of all pairs, in bytes                                    
   ///   @return the total amount of initialized bytes                        
   TABLE_TEMPLATE()
   constexpr Size TABLE()::GetByteSize() const noexcept {
      return sizeof(ValueInner) * GetCount();
   }

   /// Get the key meta data                                                  
   /// Also implicitly initializes the internal key type                      
   ///   @attention this shouldn't be called on static initialization time    
   ///   @return the meta definition of the key type                          
   TABLE_TEMPLATE()
   DMeta TABLE()::GetType() const {
      mKeys.mType = MetaData::Of<T>();
      return mKeys.mType;
   }

   /// Check if key type exactly matches another                              
   TABLE_TEMPLATE()
   template<class ALT_T>
   constexpr bool TABLE()::Is() const noexcept {
      return CT::Same<T, ALT_T>;
   }

   /// Copy-insert a pair inside the map                                      
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator << (const T& item) {
      Insert(item);
      return *this;
   }

   /// Move-insert a pair inside the map                                      
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator << (T&& item) {
      Insert(Forward<T>(item));
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
      const Size keymemory = request * sizeof(ValueInner);
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
      else {
         mKeys.mType = MetaData::Of<T>();
         mKeys.mEntry = Allocator::Allocate(keyAndInfoSize);
      }
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      mKeys.mReserved = count;

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

            // Both keys and values remain in the same place            
            Rehash(count, oldCount);
            return;
         }
         else ::std::memset(mInfo, 0, count);
      }
      else ::std::memset(mInfo, 0, count);

      if (oldKeys.IsEmpty()) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      mKeys.mCount = 0;
      auto key = oldKeys.mEntry->As<ValueInner>();
      const auto hashmask = GetReserved() - 1;
      while (oldInfo != oldInfoEnd) {
         if (!*(oldInfo++)) {
            ++key;
            continue;
         }
         
         const auto index = HashData(*key).mHash & hashmask;
         InsertInner<false>(index, Abandon(*key));
         RemoveInner(key++);
      }

      // Free the old allocations                                       
      if constexpr (REUSE) {
         // When reusing, keys and values can potentially remain same   
         // Avoid deallocating them if that's the case                  
         if (oldKeys.mEntry != mKeys.mEntry)
            Allocator::Deallocate(oldKeys.mEntry);
      }
      else if (oldKeys.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         if (oldKeys.mEntry->GetUses() > 1)
            oldKeys.mEntry->Free();
         else
            Allocator::Deallocate(oldKeys.mEntry);
      }
   }

   /// Similar to insertion, but rehashes each key                            
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - the new number of pairs                               
   TABLE_TEMPLATE()
   void TABLE()::Rehash(const Count& count, const Count& oldCount) {
      auto oldKey = GetRaw();
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
            auto swapper = SemanticMake<ValueInner>(Abandon(*oldKey));
            RemoveIndex(oldIndex);

            if (oldIndex == InsertInner<false>(newIndex, Abandon(swapper))) {
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
   ///   @param start - the starting index                                    
   ///   @param key - key to move in                                          
   ///   @param value - value to move in                                      
   ///   @return the index at which item was inserted                         
   TABLE_TEMPLATE()
   template<bool CHECK_FOR_MATCH, CT::Semantic S>
   Offset TABLE()::InsertInner(const Offset& start, S&& value) {
      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      auto swapper = SemanticMake<ValueInner>(value.Forward());

      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();
         auto& candidate = Get(index);
         if constexpr (CHECK_FOR_MATCH) {
            if (candidate == swapper) {
               // Neat, the key already exists - just return            
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            ::std::swap(candidate, swapper);
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
      auto& candidate = Get(index);
      SemanticNew<ValueInner>(&candidate, Abandon(swapper));
      *psl = attempts;
      ++mKeys.mCount;
      return index;
   }

   /// Get the bucket index, depending on key hash                            
   ///   @param key - the key to hash                                         
   ///   @return the bucket offset                                            
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE) Offset TABLE()::GetBucket(const T& key) const noexcept {
      return HashData(key).mHash & (GetReserved() - 1);
   }

   /// Insert a single pair inside table via copy                             
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   Count TABLE()::Insert(const T& value) {
      return Insert(Langulus::Copy(value));
   }

   /// Insert a single pair inside table via key copy and value move          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   Count TABLE()::Insert(T&& value) {
      return Insert(Langulus::Move(value));
   }
   
   /// Insert a single pair inside table via key copy and value move          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   template<CT::Semantic S>
   Count TABLE()::Insert(S&& value) requires (CT::Exact<TypeOf<S>, T>) {
      Allocate(GetCount() + 1);
      InsertInner<true>(GetBucket(value.mValue), value.Forward());
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
            RemoveInner(GetRaw() + offset);
         }

         ++inf;
      }
   }

   /// Clears all data, but doesn't deallocate, and retains state             
   TABLE_TEMPLATE()
   void TABLE()::Clear() {
      if (IsEmpty())
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // Clear all info to zero                                      
         ::std::memset(mInfo, 0, GetReserved());
         mKeys.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         // Notice keys are not dereferenced, we use only value refs    
         // to save on some redundancy                                  
         mInfo = nullptr;
         mKeys.mEntry->Free();
         mKeys.ResetMemory();
      }
   }

   /// Clears all data, state, and deallocates                                
   TABLE_TEMPLATE()
   void TABLE()::Reset() {
      if (!mKeys.mEntry)
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // No point in resetting info, we'll be deallocating it        
         Allocator::Deallocate(mKeys.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref values     
         // Notice keys are not dereferenced, we use only value refs    
         // to save on some redundancy                                  
         mKeys.mEntry->Free();
      }

      mInfo = nullptr;
      mKeys.ResetState();
      mKeys.ResetMemory();
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
         GetRaw() + offset,
      };
   }
   
   /// Erases element at a specific index                                     
   ///   @attention assumes that index points to a valid entry                
   ///   @param index - the index to remove                                   
   TABLE_TEMPLATE()
   void TABLE()::RemoveIndex(const Offset& index) noexcept {
      auto psl = GetInfo() + index;
      const auto pslEnd = GetInfoEnd();
      auto key = GetRaw() + index;

      // Destroy the key, info and value at the start                   
      RemoveInner(key++);
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

         SemanticNew<ValueInner>(key - 1, Abandon(*key));

         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic pop
         #endif

         RemoveInner(key++);
         *(psl++) = 0;
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd && *GetInfo() > 1) UNLIKELY() {
         psl = GetInfo();
         key = GetRaw();

         // Shift first entry to the back                               
         const auto last = mKeys.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         SemanticNew<ValueInner>(GetRaw() + last, Abandon(*key));

         RemoveInner(key++);
         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mKeys.mCount;
   }

   /// Destroy a single value or key, either sparse or dense                  
   ///   @tparam T - the type to remove, either key or value (deducible)      
   ///   @param element - the address of the element to remove                
   TABLE_TEMPLATE()
   template<class ALT_T>
   void TABLE()::RemoveInner(ALT_T* element) noexcept {
      if constexpr (CT::Destroyable<ALT_T>)
         element->~ALT_T();
   }

   /// Insert a single value or key, either sparse or dense                   
   ///   @tparam T - the type to add, either key or value (deducible)         
   ///   @param element - the address of the element to remove                
   TABLE_TEMPLATE()
   template<class ALT_T>
   void TABLE()::Overwrite(ALT_T&& from, ALT_T& to) noexcept {
      // Remove the old entry                                           
      RemoveInner(&to);

      // Reconstruct the new one in place                               
      new (&to) ALT_T {Forward<ALT_T>(from)};
   }

   /// Erase a pair via key                                                   
   ///   @param key - the key to search for                                   
   ///   @return the number of removed pairs                                  
   TABLE_TEMPLATE()
   Count TABLE()::Remove(const T& match) {
      // Get the starting index based on the key hash                   
      const auto start = GetBucket(match);
      auto key = GetRaw() + start;
      auto info = GetInfo() + start;
      const auto keyEnd = GetRawEnd();

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
   bool TABLE()::Contains(const T& key) const {
      if (IsEmpty())
         return false;
      return FindIndex(key) != GetReserved();
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TABLE_TEMPLATE()
   Index TABLE()::Find(const T& key) const {
      const auto offset = FindIndex(key);
      return offset != GetReserved() ? Index {offset} : IndexNone;
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   const TAny<T>& TABLE()::GetValues() const noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   TAny<T>& TABLE()::GetValues() noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get a value by an unsafe offset (const)                                
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::Get(const Offset& i) const noexcept {
      return GetRaw()[i];
   }

   /// Get a value by an unsafe offset                                        
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::Get(const Offset& i) noexcept {
      return GetRaw()[i];
   }

   /// Get a value by a safe index (const)                                    
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::Get(const Index& index) const {
      return const_cast<TABLE()&>(*this).Get(index);
   }

   /// Get a value by a safe index                                            
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   TABLE_TEMPLATE()
   decltype(auto) TABLE()::Get(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         OutOfRange, "Bad index");
      return GetValue(offset);
   }

   /// Find the index of a pair by key                                        
   ///   @param key - the key to search for                                   
   ///   @return the index                                                    
   TABLE_TEMPLATE()
   Offset TABLE()::FindIndex(const T& key) const {
      if (IsEmpty())
         return GetReserved();

      // Get the starting index based on the key hash                   
      // Since reserved elements are always power-of-two, we use them   
      // as a mask to the hash, to extract the relevant bucket          
      const auto start = GetBucket(key);
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd() - 1;
      auto candidate = GetRaw() + start;

      const auto keysAreEqual = [](const ValueInner* lhs, const T& rhs) {
         if constexpr (CT::Sparse<T>)
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
               candidate = GetRaw();
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
         GetRaw() + offset
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TABLE_TEMPLATE()
   typename TABLE()::ConstIterator TABLE()::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr};
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
         GetRaw() + offset
      };
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   decltype(auto) TABLE()::Last() {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);
      return Get(static_cast<Offset>(info - GetInfo()));
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   decltype(auto) TABLE()::Last() const {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);
      return Get(static_cast<Offset>(info - GetInfo()));
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
   Count TABLE()::ForEachElement(TFunctor<bool(const Block&)>&& f) const {
      Offset i {};
      return GetValues().ForEachElement([&](const Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all keys inside the map, and perform f() on them (mutable)     
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
   Count TABLE()::ForEachElement(TFunctor<bool(Block&)>&& f) {
      Offset i {};
      return GetValues().ForEachElement([&](Block& element) {
         return mInfo[i++] ? f(element) : true;
      });
   }

   /// Iterate all keys inside the map, and perform f() on them               
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
   Count TABLE()::ForEachElement(TFunctor<void(const Block&)>&& f) const {
      Offset i {};
      return GetValues().ForEachElement([&](const Block& element) {
         if (mInfo[i++])
            f(element);
      });
   }

   /// Iterate all keys inside the map, and perform f() on them (mutable)     
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TABLE_TEMPLATE()
   Count TABLE()::ForEachElement(TFunctor<void(Block&)>&& f) {
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
      const ValueInner* value
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
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
   typename TABLE()::ValueInner& TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return {*const_cast<ValueInner*>(mValue)};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const typename TABLE()::ValueInner& TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return {*mValue};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename TABLE()::ValueInner& TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return **this;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const typename TABLE()::ValueInner& TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return **this;
   }

} // namespace Langulus::Anyness

#undef ITERATOR
#undef TABLE_TEMPLATE
#undef TABLE
