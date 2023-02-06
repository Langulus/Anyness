///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet.hpp"

namespace Langulus::Anyness
{

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   LANGULUS(ALWAYSINLINE)
   BlockSet::BlockSet(const BlockSet& other)
      : mInfo {other.mInfo}
      , mKeys {other.mKeys} {
      mKeys.Keep();
   }

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   LANGULUS(ALWAYSINLINE)
   BlockSet::BlockSet(BlockSet&& other) noexcept
      : mInfo {other.mInfo}
      , mKeys {other.mKeys} {
      other.mKeys.ResetMemory();
      other.mKeys.ResetState();
   }

   /// Shallow-copy construction without referencing                          
   ///   @param other - the disowned table to copy                            
   LANGULUS(ALWAYSINLINE)
   constexpr BlockSet::BlockSet(Disowned<BlockSet>&& other) noexcept
      : mInfo {other.mValue.mInfo}
      , mKeys {other.mValue.mKeys} {
      mKeys.mEntry = nullptr;
   }

   /// Minimal move construction from abandoned table                         
   ///   @param other - the abandoned table to move                           
   LANGULUS(ALWAYSINLINE)
   constexpr BlockSet::BlockSet(Abandoned<BlockSet>&& other) noexcept
      : mInfo {other.mValue.mInfo}
      , mKeys {other.mValue.mKeys} {
      other.mValue.mKeys.mEntry = nullptr;
   }

   /// Destroys the map and all it's contents                                 
   inline BlockSet::~BlockSet() {
      if (!mKeys.mEntry)
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used values, they're used only here              
         ClearInner();

         // Deallocate stuff                                            
         Allocator::Deallocate(mKeys.mEntry);
      }
      else {
         // Data is used from multiple locations, just deref            
         mKeys.mEntry->Free();
      }
   }

   /// Checks if both tables contain the same entries                         
   /// Order is irrelevant                                                    
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   inline bool BlockSet::operator == (const BlockSet& other) const {
      if (other.GetCount() != GetCount())
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         const auto lhs = info - GetInfo();
         if (!*(info++))
            continue;

         const auto rhs = other.FindIndexUnknown(GetValue(lhs));
         if (rhs == other.GetReserved() || GetValue(lhs) != other.GetValue(rhs))
            return false;
      }

      return true;
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator = (BlockSet&& rhs) noexcept {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) BlockSet {Forward<BlockSet>(rhs)};
      return *this;
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator = (const BlockSet& rhs) {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) BlockSet {rhs};
      return *this;
   }

   /// Emplace a single pair into a cleared map                               
   ///   @param pair - the pair to emplace                                    
   ///   @return a reference to this table                                    
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator = (T&& element) noexcept {
      Clear();
      Insert(Forward<T>(element));
      return *this;
   }

   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator = (const T& element) {
      Clear();
      Insert(element);
      return *this;
   }

   /// Clone all elements in a range                                          
   ///   @attention assumes from & to have been preallocated                  
   ///   @param from - source                                                 
   ///   @param to - destrination                                             
   LANGULUS(ALWAYSINLINE)
   void BlockSet::CloneInner(const Block& from, Block& to) const {
      for (Offset i = 0; i < GetReserved(); ++i) {
         if (!mInfo[i])
            continue;

         to.GetElement(i).CallUnknownSemanticConstructors(
            1, Langulus::Clone(from.GetElement(i)));
      }
   }

   /// Clone the table                                                        
   ///   @return the new table                                                
   inline BlockSet BlockSet::Clone() const {
      if (IsEmpty())
         return {};

      BlockSet result {Disown(*this)};

      // Allocate keys and info                                         
      result.mKeys.mEntry = Allocator::Allocate(mKeys.mEntry->GetAllocatedSize());
      LANGULUS_ASSERT(result.mKeys.mEntry, Allocate, "Out of memory");

      // Clone the info bytes (including sentinel)                      
      result.mKeys.mRaw = result.mKeys.mEntry->GetBlockStart();
      result.mInfo = reinterpret_cast<InfoType*>(result.mKeys.mRaw)
         + (mInfo - reinterpret_cast<const InfoType*>(mKeys.mRaw));
      ::std::memcpy(result.mInfo, mInfo, GetReserved() + 1);

      // Clone the rest                                                 
      CloneInner(mKeys, result.mKeys);
      return Abandon(result);
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsUntyped() const noexcept {
      return mKeys.IsUntyped();
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsTypeConstrained() const noexcept {
      return mKeys.IsTypeConstrained();;
   }
   
   /// Check if key type is abstract                                          
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsAbstract() const noexcept {
      return mKeys.IsAbstract() && mKeys.IsDense();
   }
   
   /// Check if key type is default-constructible                             
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsConstructible() const noexcept {
      return mKeys.IsDefaultable();
   }
   
   /// Check if key type is deep                                              
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsDeep() const noexcept {
      return mKeys.IsDeep();
   }

   /// Check if the key type is a pointer                                     
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsSparse() const noexcept {
      return mKeys.IsSparse();
   }

   /// Check if the key type is not a pointer                                 
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsDense() const noexcept {
      return mKeys.IsDense();
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   LANGULUS(ALWAYSINLINE)
   constexpr Size BlockSet::GetStride() const noexcept {
      return mKeys.GetStride();
   }

   /// Get the raw array (const)                                              
   ///   @attention for internal use only                                     
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   constexpr decltype(auto) BlockSet::GetRaw() const noexcept {
      return GetValues<T>().GetRaw();
   }

   /// Get the raw array                                                      
   ///   @attention for internal use only                                     
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   constexpr decltype(auto) BlockSet::GetRaw() noexcept {
      return GetValues<T>().GetRaw();
   }

   /// Get the end of the array                                               
   ///   @attention for internal use only                                     
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   constexpr decltype(auto) BlockSet::GetRawEnd() const noexcept {
      return GetRaw<T>() + GetReserved();
   }

   /// Get the size of all elements, in bytes                                 
   ///   @return the total amount of initialized bytes                        
   LANGULUS(ALWAYSINLINE)
   constexpr Size BlockSet::GetByteSize() const noexcept {
      return GetStride() * GetCount(); 
   }

   /// Get the type of the set                                                
   LANGULUS(ALWAYSINLINE)
   DMeta BlockSet::GetType() const noexcept {
      return mKeys.GetType();
   }

   /// Check if key type matches another, ignoring qualifiers                 
   ///   @tparam ALT_T the type to compare against                            
   ///   @return true if types loosely match                                  
   template<class ALT_T>
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::Is() const noexcept {
      return mKeys.template Is<ALT_T>();
   }

   /// Request a new size of keys and info via the value container            
   /// The memory layout is:                                                  
   ///   [keys for each bucket]                                               
   ///         [padding for alignment]                                        
   ///               [info for each bucket]                                   
   ///                     [one sentinel byte for terminating loops]          
   ///   @attention assumes key type has been set                             
   ///   @param infoStart - [out] the offset at which info bytes start        
   ///   @return the requested byte size                                      
   LANGULUS(ALWAYSINLINE)
   Size BlockSet::RequestKeyAndInfoSize(const Count request, Offset& infoStart) noexcept {
      const Size keymemory = request * mKeys.GetStride();
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + request + 1;
   }

   /// Get the info array (const)                                             
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(ALWAYSINLINE)
   const BlockSet::InfoType* BlockSet::GetInfo() const noexcept {
      return mInfo;
   }

   /// Get the info array                                                     
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(ALWAYSINLINE)
   BlockSet::InfoType* BlockSet::GetInfo() noexcept {
      return mInfo;
   }

   /// Get the end of the info array                                          
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(ALWAYSINLINE)
   const BlockSet::InfoType* BlockSet::GetInfoEnd() const noexcept {
      return mInfo + GetReserved();
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @tparam T - the type                                                 
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   void BlockSet::Mutate() {
      Mutate(MetaData::Of<T>());
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @param key - the key type                                            
   LANGULUS(ALWAYSINLINE)
   void BlockSet::Mutate(DMeta key) {
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
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   LANGULUS(ALWAYSINLINE)
   void BlockSet::Allocate(const Count& count) {
      AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
   }

   /// Allocate or reallocate key and info array                              
   ///   @attention assumes count is a power-of-two                           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   template<bool REUSE>
   void BlockSet::Allocate(const Count& count) {
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
            Rehash(count, oldCount);
         }
         else ::std::memset(mInfo, 0, count);
      }
      else ::std::memset(mInfo, 0, count);

      if (oldKeys.IsEmpty()) {
         // There are no old values, the previous set was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys or values (or both) moved                
      // Reinsert all pairs to rehash                                   
      mKeys.mCount = 0;
      auto key = oldKeys.GetElement();
      const auto hashmask = count - 1;
      while (oldInfo != oldInfoEnd) {
         if (!*(oldInfo++)) {
            key.Next();
            continue;
         }

         InsertInnerUnknown<false>(
            key.GetHash().mHash & hashmask, 
            Abandon(key)
         );

         if (!key.IsEmpty())
            key.CallUnknownDestructors();
         else
            key.mCount = 1;

         key.Next();
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

   /// Rehashes and reinserts each pair in the same block                     
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param count - the new number of pairs                               
   ///   @param oldCount - the old number of pairs                            
   inline void BlockSet::Rehash(const Count& count, const Count& oldCount) {
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
      Block keyswap {mKeys.GetState(), GetType()};
      keyswap.AllocateFresh(keyswap.RequestSize(1));

      // For each old existing key...                                   
      while (oldInfo != oldInfoEnd) {
         if (!*oldInfo) {
            ++oldInfo;
            continue;
         }

         // Rehash and check if hashes match                            
         const Offset oldIndex = oldInfo - GetInfo();
         auto oldKey = GetValue(oldIndex);
         const Offset newIndex = oldKey.GetHash().mHash & hashmask;
         if (oldIndex != newIndex) {
            // Move key & value to swapper                              
            // No chance of overlap, so do it forwards                  
            keyswap.CallUnknownSemanticConstructors<false>(1, Abandon(oldKey));
            keyswap.mCount = 1;
            RemoveIndex(oldIndex);
            if (oldIndex == InsertInnerUnknown<false>(newIndex, Abandon(keyswap))) {
               // Index might still end up at its old index, make sure  
               // we don't loop forever in that case                    
               ++oldInfo;
            }

            // Notice iterators are not incremented                     
            continue;
         }

         ++oldInfo;
      }

      // Free the allocated swapper memory                              
      keyswap.Free();
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - number of pairs to allocate                           
   LANGULUS(ALWAYSINLINE)
   void BlockSet::AllocateInner(const Count& count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() && GetUses() == 1)
         Allocate<true>(count);
      else
         Allocate<false>(count);
   }

   /// Inner insertion function                                               
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param value - value to move in                                      
   template<bool CHECK_FOR_MATCH, CT::Semantic S>
   Offset BlockSet::InsertInner(const Offset& start, S&& value) {
      using T = TypeOf<S>;

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetRaw<T>() + index;
            if (*candidate == value.mValue) {
               // Neat, the key already exists - just return            
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            ::std::swap(GetRaw<T>()[index], value.mValue);
            ::std::swap(attempts, *psl);
         }

         ++attempts;

         if (psl < pslEnd - 1)
            ++psl;
         else
            psl = GetInfo();
      }

      // If reached, empty slot reached                                 
      // Might not seem like it, but we have a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      const auto index = psl - GetInfo();
      if constexpr (S::Move) {
         if constexpr (!S::Keep && CT::AbandonMakable<T>)
            new (&GetRaw<T>()[index]) T {value.Forward()};
         else if constexpr (CT::MoveMakable<T>)
            new (&GetRaw<T>()[index]) T {std::move(value.mValue)};
         else if constexpr (CT::CopyMakable<T>)
            new (&GetRaw<T>()[index]) T {value.mValue};
         else
            LANGULUS_ERROR("Can't abandon/move/copy value");
      }
      else {
         if constexpr (!S::Keep && CT::DisownMakable<T>)
            new (&GetRaw<T>()[index]) T {value.Forward()};
         else if constexpr (CT::CopyMakable<T>)
            new (&GetRaw<T>()[index]) T {value.mValue};
         else
            LANGULUS_ERROR("Can't disown/copy value");
      }

      *psl = attempts;
      ++mKeys.mCount;
      return index;
   }
   
   /// Inner insertion function based on reflected move-assignment            
   ///   @attention after this call, key and/or value might be empty          
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param value - value to move in                                      
   template<bool CHECK_FOR_MATCH, CT::Semantic S>
   Offset BlockSet::InsertInnerUnknown(const Offset& start, S&& value) {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = Get(index);
            if (candidate == value.mValue) {
               // Neat, the key already exists - just set value and go  
               GetValue(index)
                  .CallUnknownSemanticAssignment(1, value.Forward());

               if constexpr (S::Move) {
                  value.mValue.CallUnknownDestructors();
                  value.mValue.mCount = 0;
               }

               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
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
      GetValue(index)
         .CallUnknownSemanticConstructors(1, value.Forward());
      
      if constexpr (S::Move) {
         value.mValue.CallUnknownDestructors();
         value.mValue.mCount = 0;
      }

      *psl = attempts;
      ++mKeys.mCount;
      return index;
   }

   /// Get the bucket index, depending on key hash                            
   ///   @param value - the value to hash                                     
   ///   @return the bucket offset                                            
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   Offset BlockSet::GetBucket(const T& value) const noexcept {
      return HashData(value).mHash & (GetReserved() - 1);
   }

   /// Insert a single pair inside table via copy                             
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::Insert(const T& value) {
      return Insert(Copy(value));
   }

   /// Insert a single pair inside table via key copy and value move          
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::Insert(S&& value) {
      using T = TypeOf<S>;
      Mutate<T>();
      Allocate(GetCount() + 1);
      InsertInner<true>(GetBucket(value.mValue), value.Forward());
      return 1;
   }

   /// Insert a single value inside table via copy (unknown version)          
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::InsertUnknown(const Block& value) {
      return InsertUnknown(Langulus::Copy(value));
   }

   /// Insert a single pair inside table via move (unknown version)           
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::InsertUnknown(Block&& value) {
      return InsertUnknown(Langulus::Move(value));
   }
   
   /// Insert a single pair inside table via move (unknown version)           
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::InsertUnknown(S&& value) requires (CT::Block<TypeOf<S>>) {
      Mutate(value.mValue.mType);
      Allocate(GetCount() + 1);
      const auto index = value.mValue.GetHash().mHash & (GetReserved() - 1);
      InsertInnerUnknown<true>(index, value.Forward());
      return 1;
   }

   /// Copy-insert a templated pair inside the map                            
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator << (const T& item) {
      Insert(Copy(item));
      return *this;
   }

   /// Move-insert a templated pair inside the map                            
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator << (T&& item) {
      Insert(Move(item));
      return *this;
   }

   /// Copy-insert a type-erased item inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator << (const Block& item) {
      InsertUnknown(item);
      return *this;
   }

   /// Move-insert a type-erased item inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(ALWAYSINLINE)
   BlockSet& BlockSet::operator << (Block&& item) {
      InsertUnknown(Forward<Block>(item));
      return *this;
   }

   /// Merge the contents of both sets in this set by doing shallow copies    
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::Merge(const BlockSet& set) {
      return Merge(Copy(set));
   }

   /// Merge the contents of both sets in this set by moving elements         
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::Merge(BlockSet&& set) {
      return Merge(Move(set));
   }

   /// Merge the contents of both sets in this set by using a semantic        
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::Merge(S&& set) {
      static_assert(CT::Set<TypeOf<S>>, "You can only merge other sets");
      Count inserted {};
      for (auto it : set.mValue)
         inserted += InsertUnknown(S::Nest(it));
      return inserted;
   }

   /// Destroy everything initialized inside the map                          
   LANGULUS(ALWAYSINLINE)
   void BlockSet::ClearInner() {
      auto inf = GetInfo();
      const auto infEnd = GetInfoEnd();
      while (inf != infEnd) {
         if (*inf) {
            const auto offset = inf - GetInfo();
            GetValue(offset).CallUnknownDestructors();
         }

         ++inf;
      }
   }

   /// Clears all data, but doesn't deallocate                                
   inline void BlockSet::Clear() {
      if (IsEmpty())
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // Clear all info to zero                                      
         ::std::memset(GetInfo(), 0, GetReserved());
         mKeys.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         mInfo = nullptr;
         mKeys.mEntry->Free();
         mKeys.ResetMemory();
      }
   }

   /// Clears all data and deallocates                                        
   inline void BlockSet::Reset() {
      if (mKeys.mEntry) {
         if (mKeys.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            ClearInner();

            // No point in resetting info, we'll be deallocating it     
            Allocator::Deallocate(mKeys.mEntry);
         }
         else {
            // Data is used from multiple locations, just deref values  
            mKeys.mEntry->Free();
         }

         mInfo = nullptr;
         mKeys.ResetMemory();
      }

      mKeys.ResetState();
   }

   /// Erases element at a specific index                                     
   ///   @attention assumes that offset points to a valid entry               
   ///   @param offset - the index to remove                                  
   inline void BlockSet::RemoveIndex(const Offset& offset) noexcept {
      auto psl = GetInfo() + offset;
      const auto pslEnd = GetInfoEnd();
      auto key = mKeys.GetElement(offset);

      // Destroy the key, info and value at the offset                  
      key.CallUnknownDestructors();

      *(psl++) = 0;
      key.Next();

      // And shift backwards, until a zero or 1 is reached              
      // That way we move every entry that is far from its start        
      // closer to it. Moving is costly, unless you use pointers        
      try_again:
      while (*psl > 1) {
         psl[-1] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         const_cast<const Block&>(key).Prev()
            .CallUnknownSemanticConstructors(1, Abandon(key));

         key.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd && *GetInfo() > 1) UNLIKELY() {
         psl = GetInfo();
         key = mKeys.GetElement();

         // Shift first entry to the back                               
         const auto last = mKeys.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         GetValue(last)
            .CallUnknownSemanticConstructors(1, Abandon(key));

         key.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mKeys.mCount;
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   const TAny<T>& BlockSet::GetValues() const noexcept {
      return reinterpret_cast<const TAny<T>&>(mKeys);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   TAny<T>& BlockSet::GetValues() noexcept {
      return reinterpret_cast<TAny<T>&>(mKeys);
   }

   /// Erase a pair via key                                                   
   ///   @param match - the key to search for                                 
   ///   @return the number of removed pairs                                  
   template<CT::NotSemantic T>
   Count BlockSet::Remove(const T& match) {
      // Get the starting index based on the key hash                   
      const auto start = GetBucket(match);
      auto key = GetRaw<T>() + start;
      const auto keyEnd = GetRawEnd<T>();
      auto info = GetInfo() + start;

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
   LANGULUS(ALWAYSINLINE)
   void BlockSet::Compact() {
      //TODO();
   }

   ///                                                                        
   ///   SEARCH                                                               
   ///                                                                        
   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   bool BlockSet::Contains(const T& key) const {
      if (IsEmpty())
         return false;
      return FindIndex(key) != GetReserved();
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Index BlockSet::Find(const T& key) const {
      const auto offset = FindIndex(key);
      return offset != GetReserved() ? Index {offset} : IndexNone;
   }

   /// Get a key by an unsafe offset (const)                                  
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the key                                       
   LANGULUS(ALWAYSINLINE)
   Block BlockSet::GetValue(const Offset& i) const noexcept {
      return mKeys.GetElement(i);
   }

   /// Get a key by an unsafe offset                                          
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return a reference to the key                                       
   LANGULUS(ALWAYSINLINE)
   Block BlockSet::GetValue(const Offset& i) noexcept {
      return mKeys.GetElement(i);
   }

   /// Get a key by a safe index (const)                                      
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   LANGULUS(ALWAYSINLINE)
   Block BlockSet::Get(const Index& index) const {
      return const_cast<BlockSet&>(*this).Get(index);
   }

   /// Get a key by a safe index                                              
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   LANGULUS(ALWAYSINLINE)
   Block BlockSet::Get(const Index& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         OutOfRange, "Bad index");
      return GetValue(offset);
   }

   /// Find the index of a pair by key                                        
   ///   @param key - the key to search for                                   
   ///   @return the index                                                    
   template<CT::Data T>
   Offset BlockSet::FindIndex(const T& key) const {
      // Get the starting index based on the key hash                   
      // Since reserved elements are always power-of-two, we use them   
      // as a mask to the hash, to extract the relevant bucket          
      const auto start = GetBucket(key);
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd() - 1;
      auto candidate = GetRaw<T>() + start;
      Count attempts{};
      while (*psl > attempts) {
         if (*candidate != key) {
            // There might be more keys to the right, check them        
            if (psl == pslEnd) UNLIKELY() {
               // By 'to the right' I also mean looped back to start    
               psl = GetInfo();
               candidate = GetRaw<T>();
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
   inline Offset BlockSet::FindIndexUnknown(const Block& key) const {
      // Get the starting index based on the key hash                   
      // Since reserved elements are always power-of-two, we use them   
      // as a mask to the hash, to extract the relevant bucket          
      const auto start = GetBucket(key);
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd() - 1;
      auto candidate = GetValue(start);
      Count attempts{};
      while (*psl > attempts) {
         if (candidate != key) {
            // There might be more keys to the right, check them        
            if (psl == pslEnd) UNLIKELY() {
               // By 'to the right' I also mean looped back to start    
               psl = GetInfo();
               candidate = GetValue(0);
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

   /// Get the number of inserted pairs                                       
   ///   @return the number of inserted pairs                                 
   LANGULUS(ALWAYSINLINE)
   constexpr Count BlockSet::GetCount() const noexcept {
      return mKeys.GetCount();
   }

   /// Get the number of allocated pairs                                      
   ///   @return the number of allocated pairs                                
   LANGULUS(ALWAYSINLINE)
   constexpr Count BlockSet::GetReserved() const noexcept {
      return mKeys.GetReserved();
   }

   /// Check if there are any pairs in this map                               
   ///   @return true if there's at least one pair available                  
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsEmpty() const noexcept {
      return mKeys.IsEmpty();
   }

   /// Check if the map has been allocated                                    
   ///   @return true if the map uses dynamic memory                          
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::IsAllocated() const noexcept {
      return mKeys.IsAllocated();
   }

   /// Check if the memory for the table is owned by us                       
   /// This is always true, since the map can't be initialized with outside   
   /// memory - the memory layout requirements are too strict to allow for it 
   ///   @return true                                                         
   LANGULUS(ALWAYSINLINE)
   constexpr bool BlockSet::HasAuthority() const noexcept {
      return IsAllocated();
   }

   /// Get the number of references for the allocated memory                  
   ///   @attention always returns zero if we don't have authority            
   ///   @return the number of references                                     
   LANGULUS(ALWAYSINLINE)
   constexpr Count BlockSet::GetUses() const noexcept {
      return mKeys.GetUses();
   }

   /// Get hash of the set contents                                           
   ///   @attention the hash is not cached, so this is a slow operation       
   ///   @return the hash                                                     
   LANGULUS(ALWAYSINLINE)
   Hash BlockSet::GetHash() const {
      TAny<Hash> hashes;
      for (auto element : *this)
         hashes << element.GetHash();
      return hashes.GetHash();
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::Iterator BlockSet::begin() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const BlockSet*>(this)->begin();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::Iterator BlockSet::end() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const BlockSet*>(this)->end();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::Iterator BlockSet::last() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const BlockSet*>(this)->last();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::ConstIterator BlockSet::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (!*info) ++info;

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(), 
         GetValue(offset)
      };
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::ConstIterator BlockSet::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), {}};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::ConstIterator BlockSet::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() && !*--info);

      const auto offset = info - GetInfo();
      return {
         info, GetInfoEnd(),
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
   Count BlockSet::ForEachSplitter(Block& part, F&& call) {
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
   Count BlockSet::ForEachDeepSplitter(Block& part, F&& call) {
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
   Count BlockSet::ForEachInner(Block& part, TFunctor<R(A)>&& call) {
      if (IsEmpty() || !part.mType->template CastsTo<A, true>())
         return 0;
       
      constexpr bool HasBreaker = CT::Bool<R>;
      Count done {};
      Count index {};

      while (index < mKeys.mReserved) {
         if (!mInfo[index]) {
            ++index;
            continue;
         }

         if constexpr (REVERSE) {
            if constexpr (HasBreaker) {
               if (!call(part.template Get<A>(mKeys.mReserved - index - 1)))
                  return ++done;
            }
            else call(part.template Get<A>(mKeys.mReserved - index - 1));
         }
         else {
            if constexpr (HasBreaker) {
               if (!call(part.template Get<A>(index)))
                  return ++done;
            }
            else call(part.template Get<A>(index));
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
   Count BlockSet::ForEachDeepInner(Block& part, TFunctor<R(A)>&& call) {
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
   Count BlockSet::ForEachElement(Block& part, F&& call) {
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

   /// Iterate all elements inside the map, and perform f() on them           
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each element block               
   ///   @return the number of successful f() executions                      
   template<bool MUTABLE, class F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachElement(F&& f) {
      return ForEachElement<MUTABLE>(mKeys, Forward<F>(f));
   }

   template<class F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachElement(F&& f) const {
      return const_cast<BlockSet&>(*this)
         .template ForEachElement<false>(Forward<F>(f));
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   template<bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEach(F&&... f) {
      return (... || ForEachSplitter<MUTABLE, false>(mKeys, Forward<F>(f)));
   }

   template<class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEach(F&&... f) const {
      return const_cast<BlockSet&>(*this)
         .template ForEach<false>(Forward<F>(f)...);
   }

   template<bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachRev(F&&... f) {
      return (... || ForEachSplitter<MUTABLE, true>(mKeys, Forward<F>(f)));
   }

   template<class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachRev(F&&... f) const {
      return const_cast<BlockSet&>(*this)
         .template ForEachRev<false>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachDeep(F&&... f) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, false>(mKeys, Forward<F>(f)));
   }

   template<bool SKIP, class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachDeep(F&&... f) const {
      return const_cast<BlockSet&>(*this)
         .template ForEachDeep<SKIP, false>(Forward<F>(f)...);
   }

   template<bool SKIP, bool MUTABLE, class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachDeepRev(F&&... f) {
      return (... || ForEachDeepSplitter<SKIP, MUTABLE, true>(mKeys, Forward<F>(f)));
   }

   template<bool SKIP, class... F>
   LANGULUS(ALWAYSINLINE)
   Count BlockSet::ForEachDeepRev(F&&... f) const {
      return const_cast<BlockSet&>(*this)
         .template ForEachDeepRev<SKIP, false>(Forward<F>(f)...);
   }


   ///                                                                        
   ///   Set iterator                                                         
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param key - pointer to the key element                              
   ///   @param value - pointer to the value element                          
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   BlockSet::TIterator<MUTABLE>::TIterator(
      const InfoType* info, 
      const InfoType* sentinel, 
      const Block& value
   ) noexcept
      : mInfo {info}
      , mSentinel {sentinel}
      , mKey {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::TIterator<MUTABLE>& BlockSet::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (!*++mInfo);
      const auto offset = mInfo - previous;
      mKey.mRaw += offset * mKey.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename BlockSet::TIterator<MUTABLE> BlockSet::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   bool BlockSet::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   Any BlockSet::TIterator<MUTABLE>::operator * () const noexcept {
      return {Disown(mKey)};
   }

} // namespace Langulus::Anyness
