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
#define ITERATOR() TABLE()::template TIterator<MUTABLE>

namespace Langulus::Anyness
{

   /// Default construction                                                   
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   constexpr TABLE()::TUnorderedSet()
      : UnorderedSet {} {
      mKeys.mState = DataState::Typed;
      if constexpr (CT::Constant<T>)
         mKeys.MakeConst();
   }

   /// Create from a list of elements                                         
   ///   @param list - list of elements                                       
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   TABLE()::TUnorderedSet(::std::initializer_list<T> initlist)
      : TUnorderedSet {} {
      mKeys.mType = MetaData::Of<T>();

      AllocateFresh(
         Roof2(
            initlist.size() < MinimalAllocation
               ? MinimalAllocation
               : initlist.size()
         )
      );

      ZeroMemory(mInfo, GetReserved());
      mInfo[GetReserved()] = 1;

      for (auto& it : initlist)
         InsertUnknown(Copy(it));
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   TABLE()::TUnorderedSet(const TUnorderedSet& other)
      : TUnorderedSet {Langulus::Copy(other)} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   TABLE()::TUnorderedSet(TUnorderedSet&& other) noexcept
      : TUnorderedSet {Langulus::Move(other)} {}

   /// Semantic constructor from any set/element                              
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   TABLE_TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   TABLE()::TUnorderedSet(S&& other) noexcept
      : TUnorderedSet {} {
      using ST = TypeOf<S>;

      if constexpr (CT::Set<ST>) {
         // Construct from any kind of set                              
         if constexpr (ST::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
            mKeys.mType = MetaData::Of<T>();

            AllocateFresh(other.mValue.GetReserved());

            ZeroMemory(mInfo, GetReserved());
            mInfo[GetReserved()] = 1;

            other.mValue.ForEach([this](const Block& element) {
               InsertUnknown(S::Nest(element));
               });
         }
         else {
            // We can directly interface set, because it is unordered   
            // and uses the same bucketing approach                     
            BlockTransfer<TUnorderedSet>(other.Forward());
         }
      }
      else if constexpr (CT::Exact<T, ST>) {
         // Construct from any kind of element                          
         mKeys.mType = MetaData::Of<T>();

         AllocateFresh(MinimalAllocation);

         ZeroMemory(mInfo, GetReserved());
         mInfo[GetReserved()] = 1;

         TODO();
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
   }

   /// Destroys the set and all it's contents                                 
   TABLE_TEMPLATE()
   TABLE()::~TUnorderedSet() {
      if (!mKeys.mEntry)
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys, they're used only here                
         // This is a statically-optimized equivalent                   
         ClearInner();

         // Deallocate stuff                                            
         Allocator::Deallocate(mKeys.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref            
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

         const auto rhs = other.FindIndex(GetRaw(lhs));
         if (rhs == other.GetReserved())
            return false;
      }

      return true;
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (TUnorderedSet&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (const TUnorderedSet& rhs) {
      return operator = (Copy(rhs));
   }
   
   /// Insert a single element into a cleared set                             
   ///   @param rhs - the element to copy                                     
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (const CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }

   /// Emplace a single element into a cleared set                            
   ///   @param rhs - the element to emplace                                  
   ///   @return a reference to this table                                    
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator = (CT::NotSemantic auto&& rhs) noexcept {
      return operator = (Move(rhs));
   }
   
   /// Semantic assignment for any set/element                                
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the set/element to use for construction                 
   TABLE_TEMPLATE()
   template<CT::Semantic S>
   TABLE()& TABLE()::operator = (S&& rhs) noexcept {
      using ST = TypeOf<S>;

      if constexpr (CT::Set<ST>) {
         if (&static_cast<const BlockSet&>(rhs.mValue) == this)
            return *this;

         Reset();
         new (this) Self {rhs.Forward()};
      }
      else if constexpr (CT::Exact<T, ST>) {
         Clear();
         Insert(S::Nest(rhs.mValue));
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");

      return *this;
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
      return sizeof(T); 
   }

   /// Get a raw key entry (const)                                            
   ///   @param index - the key index                                         
   ///   @return a constant reference to the element                          
   TABLE_TEMPLATE()
   constexpr const T& TABLE()::GetRaw(Offset index) const noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a raw key entry                                                    
   ///   @param index - the key index                                         
   ///   @return a mutable reference to the element                           
   TABLE_TEMPLATE()
   constexpr T& TABLE()::GetRaw(Offset index) noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a handle to a key                                                  
   ///   @param index - the key index                                         
   ///   @return the handle                                                   
   TABLE_TEMPLATE()
   constexpr Handle<T> TABLE()::GetHandle(Offset index) noexcept {
      return GetValues().GetHandle(index);
   }

   /// Get the size of all pairs, in bytes                                    
   ///   @return the total amount of initialized bytes                        
   TABLE_TEMPLATE()
   constexpr Size TABLE()::GetByteSize() const noexcept {
      return sizeof(Pair) * GetCount(); 
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
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator << (const T& rhs) {
      return operator << (Copy(rhs));
   }

   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   TABLE()& TABLE()::operator << (T&& rhs) {
      return operator << (Move(rhs));
   }
   
   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TABLE_TEMPLATE()
   template<CT::Semantic S>
   TABLE()& TABLE()::operator << (S&& rhs) requires (CT::Exact<TypeOf<S>, T>) {
      Insert(S::Nest(rhs.mValue));
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
   LANGULUS(ALWAYSINLINE)
   Size TABLE()::RequestKeyAndInfoSize(const Count count, Offset& infoStart) noexcept {
      Size keymemory = count * sizeof(T);
      if constexpr (CT::Sparse<T>)
         keymemory *= 2;
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + count + 1;
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   TABLE_TEMPLATE()
   void TABLE()::Allocate(const Count& count) {
      AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
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

      mKeys.mReserved = count;

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
            MoveMemory(mInfo, oldInfo, oldCount);
            ZeroMemory(mInfo + oldCount, count - oldCount);

            // Data was reused, but entries always move if sparse keys  
            if constexpr (CT::Sparse<T>) {
               MoveMemory(
                  mKeys.mRawSparse + count,
                  mKeys.mRawSparse + oldCount,
                  oldCount
               );
            };

            Rehash(count, oldCount);
         }
         else ZeroMemory(mInfo, count);
      }
      else ZeroMemory(mInfo, count);

      if (oldKeys.IsEmpty()) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      auto key = oldKeys.GetHandle<T>(0);
      const auto hashmask = GetReserved() - 1;
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            const auto index = HashData(key.Get()).mHash & hashmask;
            InsertInner<false>(index, Abandon(key));
            key.Destroy();
         }
         
         ++oldInfo;
         ++key;
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
         // (keys are always present, if values are present)            
         if (oldKeys.mEntry->GetUses() > 1)
            oldKeys.mEntry->Free();
         else
            Allocator::Deallocate(oldKeys.mEntry);
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

      auto oldKey = GetHandle(0);
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
               HandleLocal<T> keyswap {Abandon(oldKey)};
               RemoveIndex(oldIndex);

               if (oldIndex == InsertInner<false>(newIndex, Abandon(keyswap))) {
                  // Sometimes insertion can be reinserted at same spot,
                  // make sure we account for that corner case          
                  ++oldKey;
                  ++oldInfo;
               }

               continue;
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
   template<bool CHECK_FOR_MATCH, CT::Semantic S>
   Offset TABLE()::InsertInner(const Offset& start, S&& key) {
      HandleLocal<T> keyswap {key.Forward()};

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto& candidate = GetRaw(index);
            if (keyswap.Compare(candidate)) {
               // Neat, the key already exists - just return            
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetHandle(index).Swap(keyswap);
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
      GetHandle(index).New(Abandon(keyswap));
      *psl = attempts;
      ++mKeys.mCount;
      return index;
   }

   /// Get the bucket index, depending on key hash                            
   ///   @param key - the key to hash                                         
   ///   @return the bucket offset                                            
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Offset TABLE()::GetBucket(const T& key) const noexcept {
      return HashData(key).mHash & (GetReserved() - 1);
   }

   /// Insert a single pair inside table via copy                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Count TABLE()::Insert(const T& key) {
      return Insert(Copy(key));
   }

   /// Insert a single pair inside table via key move and value copy          
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Count TABLE()::Insert(T&& key) {
      return Insert(Move(key));
   }

   /// Insert a single pair inside table via move                             
   ///   @param key - the key to add                                          
   ///   @param value - the value to add                                      
   ///   @return 1 if pair was inserted, zero otherwise                       
   TABLE_TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Count TABLE()::Insert(S&& key) requires (CT::Exact<TypeOf<S>, T>) {
      Allocate(GetCount() + 1);
      InsertInner<true>(GetBucket(key.mValue), key.Forward());
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
            GetHandle(offset).Destroy();
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
         ZeroMemory(mInfo, GetReserved());
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

      return {mInfo + offset, index.mSentinel, &GetRaw(offset)};
   }
   
   /// Erases element at a specific index                                     
   ///   @attention assumes that index points to a valid entry                
   ///   @param index - the index to remove                                   
   TABLE_TEMPLATE()
   void TABLE()::RemoveIndex(const Offset& index) noexcept {
      auto psl = GetInfo() + index;
      const auto pslEnd = GetInfoEnd();
      auto key = GetHandle(index);

      // Destroy the key, info and value at the start                   
      (key++).Destroy();
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

         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic pop
         #endif

         (key++).Destroy();
         *(psl++) = 0;
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd && *GetInfo() > 1) {
         psl = GetInfo();
         key = GetHandle(0);

         // Shift first entry to the back                               
         const auto last = mKeys.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         GetHandle(last).New(Abandon(key));

         (key++).Destroy();
         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mKeys.mCount;
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
      new (&to) T {Forward<T>(from)};
   }

   /// Erase a pair via key                                                   
   ///   @param key - the key to search for                                   
   ///   @return the number of removed pairs                                  
   TABLE_TEMPLATE()
   Count TABLE()::Remove(const T& match) {
      // Get the starting index based on the key hash                   
      const auto start = GetBucket(match);
      auto key = &GetRaw(start);
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

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   const TAny<T>& TABLE()::GetValues() const noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TABLE_TEMPLATE()
   TAny<T>& TABLE()::GetValues() noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get a key at an index                                                  
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the constant key reference                                   
   TABLE_TEMPLATE()
   const T& TABLE()::Get(const CT::Index auto& index) const {
      const auto idx = GetValues().template SimplifyIndex<T, false>(index);
      if (!mInfo[idx])
         LANGULUS_THROW(OutOfRange, "No pair at given index");
      return GetValues().GetRaw()[idx];
   }

   /// Get a key at an index                                                  
   ///   @attention will throw OutOfRange if there's no pair at the index     
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TABLE_TEMPLATE()
   T& TABLE()::Get(const CT::Index auto& index) {
      const auto idx = GetValues().template SimplifyIndex<T, false>(index);
      if (!mInfo[idx])
         LANGULUS_THROW(OutOfRange, "No pair at given index");
      return GetValues().GetRaw()[idx];
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
      auto candidate = &GetRaw(start);

      Count attempts{};
      while (*psl > attempts) {
         if (*candidate != key) {
            // There might be more keys to the right, check them        
            if (psl == pslEnd) UNLIKELY() {
               // By 'to the right' I also mean looped back to start    
               psl = GetInfo();
               candidate = &GetRaw(0);
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
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (!*info) ++info;
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TABLE_TEMPLATE()
   typename TABLE()::Iterator TABLE()::end() noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TABLE_TEMPLATE()
   typename TABLE()::Iterator TABLE()::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
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
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
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
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }
   
   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   T& TABLE()::Last() {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);
      return GetRaw(static_cast<Offset>(info - GetInfo()));
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TABLE_TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   const T& TABLE()::Last() const {
      LANGULUS_ASSERT(!IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);
      return GetRaw(static_cast<Offset>(info - GetInfo()));
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

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param value - pointer to the value element                          
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   TABLE()::TIterator<MUTABLE>::TIterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const T* value
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
   T& TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return {*const_cast<T*>(mValue)};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const T& TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return {*mValue};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   T& TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return **this;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TABLE_TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const T& TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return **this;
   }

} // namespace Langulus::Anyness

#undef ITERATOR
#undef TABLE_TEMPLATE
#undef TABLE
