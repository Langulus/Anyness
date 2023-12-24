///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedSet.hpp"
#include "UnorderedSet.inl"

#define TEMPLATE() template<CT::Data T>
#define TABLE() TUnorderedSet<T>
#define ITERATOR() TABLE()::template TIterator<MUTABLE>


namespace Langulus::Anyness
{

   /// Default construction                                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TABLE()::TUnorderedSet()
      : UnorderedSet {} {
      mKeys.mState = DataState::Typed;
      if constexpr (CT::Constant<T>)
         mKeys.MakeConst();
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedSet(const TUnorderedSet& other)
      : TUnorderedSet {Copy(other)} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::TUnorderedSet(TUnorderedSet&& other)
      : TUnorderedSet {Move(other)} {}

   /// Semantic-construction                                                  
   ///   @param other - the set and semantic to construct with                
   TEMPLATE() template<template<class> class S>
   requires (CT::Inner::SemanticMakable<S, T>) LANGULUS(INLINED)
   TABLE()::TUnorderedSet(S<TUnorderedSet>&& other) {
      BlockTransfer<TUnorderedSet>(other.Forward());
   }
   
   /// Create from a list of elements, each of them can be semantic or not,   
   /// an array, as well as any other kinds of sets                           
   ///   @param t1 - first element                                            
   ///   @param tail - tail of elements (optional)                            
   TEMPLATE() template<class T1, class... TAIL>
   requires (CT::Inner::UnfoldMakableFrom<T, T1, TAIL...>) LANGULUS(INLINED)
   TABLE()::TUnorderedSet(T1&& t1, TAIL&&... tail) {
      Insert(Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Destroys the set and all it's contents                                 
   TEMPLATE()
   TABLE()::~TUnorderedSet() {
      if (not mKeys.mEntry)
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys, they're used only here                
         // This is a statically-optimized equivalent                   
         ClearInner();

         // Deallocate stuff                                            
         Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
      }
      else {
         // Data is used from multiple locations, just deref            
         const_cast<Allocation*>(mKeys.mEntry)->Free();
      }

      mKeys.mEntry = nullptr;
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const TUnorderedSet& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (TUnorderedSet&& rhs) {
      return operator = (Move(rhs));
   }
   
   /// Insert a single element into a cleared set                             
   ///   @param rhs - the element to copy                                     
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }
   
   /// Insert a single element into a cleared set                             
   ///   @param rhs - the element to copy                                     
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }

   /// Emplace a single element into a cleared set                            
   ///   @param rhs - the element to emplace                                  
   ///   @return a reference to this table                                    
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::NotSemantic auto&& rhs) {
      return operator = (Move(rhs));
   }
   
   /// General semantic assignment for any set/element                        
   ///   @param rhs - the set/element to use for assignment                   
   TEMPLATE()
   TABLE()& TABLE()::AssignFrom(CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Set<ST>) {
         if (&static_cast<const BlockSet&>(*rhs) == this)
            return *this;

         Reset();
         new (this) Self {rhs.Forward()};
      }
      else if constexpr (CT::Exact<T, ST>) {
         Clear();
         Insert(rhs.Forward());
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");

      return *this;
   }

   /// Shallow semantic assignment for any set/element                        
   ///   @param rhs - the set/element to use for construction                 
   TEMPLATE()
   TABLE()& TABLE()::operator = (CT::ShallowSemantic auto&& rhs) {
      return AssignFrom(rhs.Forward());
   }

   /// Deep semantic assignment for any set/element                           
   ///   @param rhs - the set/element to use for construction                 
   TEMPLATE()
   TABLE()& TABLE()::operator = (CT::DeepSemantic auto&& rhs) requires CT::CloneAssignable<T> {
      return AssignFrom(rhs.Forward());
   }

   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if key type is abstract                                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsAbstract() const noexcept {
      return CT::Abstract<T>;
   }
   
   /// Check if key type is default-constructible                             
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsConstructible() const noexcept {
      return CT::Defaultable<T>;
   }
   
   /// Check if key type is deep                                              
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsDeep() const noexcept {
      return CT::Deep<T>;
   }

   /// Check if the key type is a pointer                                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsSparse() const noexcept {
      return CT::Sparse<T>;
   }

   /// Check if the key type is not a pointer                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsDense() const noexcept {
      return CT::Dense<T>;
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   TEMPLATE()
   constexpr Size TABLE()::GetStride() const noexcept {
      return sizeof(T); 
   }

   /// Get a raw key entry (const)                                            
   ///   @param index - the key index                                         
   ///   @return a constant reference to the element                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr const T& TABLE()::GetRaw(Offset index) const noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a raw key entry                                                    
   ///   @param index - the key index                                         
   ///   @return a mutable reference to the element                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr T& TABLE()::GetRaw(Offset index) noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a handle to a key                                                  
   ///   @param index - the key index                                         
   ///   @return the handle                                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr Handle<T> TABLE()::GetHandle(Offset index) noexcept {
      return GetValues().GetHandle(index);
   }

   /// Get the size of all elements, in bytes                                 
   ///   @return the total amount of initialized bytes                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TABLE()::GetBytesize() const noexcept {
      return sizeof(T) * GetCount(); 
   }

   /// Get the key meta data                                                  
   /// Also implicitly initializes the internal key type                      
   ///   @attention this shouldn't be called on static initialization time    
   ///   @return the meta definition of the key type                          
   TEMPLATE() LANGULUS(INLINED)
   DMeta TABLE()::GetType() const {
      mKeys.mType = MetaDataOf<T>();
      return mKeys.mType;
   }

   /// Check if value origin type matches any of the list                     
   ///   @tparam T1, TN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TABLE()::Is() const noexcept {
      return CT::SameAsOneOf<T, T1, TN...>;
   }

   /// Check if value origin type matches another                             
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::Is(DMeta value) const noexcept {
      return GetType()->Is(value);
   }

   /// Check if cv-unqualified value type matches any of the list             
   ///   @tparam T1, TN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsSimilar() const noexcept {
      return CT::SimilarAsOneOf<T, T1, TN...>;
   }

   /// Check if cv-unqualified value type matches another                     
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsSimilar(DMeta value) const noexcept {
      return GetType()->IsSimilar(value);
   }

   /// Check if value type exactly matches any of the list                    
   ///   @tparam T1, TN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsExact() const noexcept {
      return CT::ExactAsOneOf<T, T1, TN...>;
   }

   /// Check if value type exactly matches another                            
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsExact(DMeta value) const noexcept {
      return GetType()->IsExact(value);
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
      Size keymemory = count * sizeof(T);
      if constexpr (CT::Sparse<T>)
         keymemory *= 2;
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + count + 1;
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Reserve(const Count& count) {
      AllocateInner(Roof2(count < MinimalAllocation ? MinimalAllocation : count));
   }

   /// Allocate a fresh set keys and values (for internal use only)           
   ///   @attention doesn't initialize anything, but the memory state         
   ///   @attention doesn't modify count, doesn't set info sentinel           
   ///   @attention assumes count is a power-of-two                           
   ///   @param count - the new number of pairs                               
   TEMPLATE()
   void TABLE()::AllocateFresh(const Count& count) {
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "Table reallocation count is not a power-of-two");

      Offset infoOffset;
      const auto keyAndInfoSize = RequestKeyAndInfoSize(count, infoOffset);
      mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);
      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      mKeys.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = const_cast<Byte*>(mKeys.mEntry->GetBlockStart());
      mInfo = reinterpret_cast<InfoType*>(mKeys.mRaw + infoOffset);
   }

   /// Allocate or reallocate key and info array                              
   ///   @attention assumes count is a power-of-two                           
   ///   @tparam REUSE - true to reallocate, false to allocate fresh          
   ///   @param count - the new number of pairs                               
   TEMPLATE() template<bool REUSE>
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
         mKeys.mEntry = Allocator::Reallocate(
            keyAndInfoSize, const_cast<Allocation*>(mKeys.mEntry));
      else {
         mKeys.mType = MetaDataOf<T>();
         mKeys.mEntry = Allocator::Allocate(mKeys.mType, keyAndInfoSize);
      }

      LANGULUS_ASSERT(mKeys.mEntry, Allocate, "Out of memory");

      mKeys.mReserved = count;

      // Precalculate the info pointer, it's costly                     
      mKeys.mRaw = const_cast<Byte*>(mKeys.mEntry->GetBlockStart());
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

            Rehash(oldCount);
            return;
         }
         else ZeroMemory(mInfo, count);
      }
      else ZeroMemory(mInfo, count);

      if (oldKeys.IsEmpty()) {
         // There are no old values, the previous map was empty         
         // Just do an early return right here                          
         return;
      }

      // If reached, then keys moved - reinsert all pairs to rehash     
      mKeys.mCount = 0;
      auto key = oldKeys.GetHandle<T>(0);
      const auto hashmask = GetReserved() - 1;
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            const auto index = GetBucket(hashmask, key.Get());
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
            Allocator::Deallocate(const_cast<Allocation*>(oldKeys.mEntry));
      }
      else if (oldKeys.mEntry) {
         // Not reusing, so either deallocate, or dereference           
         if (oldKeys.mEntry->GetUses() > 1)
            const_cast<Allocation*>(oldKeys.mEntry)->Free();
         else
            Allocator::Deallocate(const_cast<Allocation*>(oldKeys.mEntry));
      }
   }

   /// Rehashes each element and reinserts it                                 
   ///   @attention assumes counts are a power-of-two number                  
   ///   @param oldCount - the old number of pairs                            
   TEMPLATE()
   void TABLE()::Rehash(const Count& oldCount) {
      LANGULUS_ASSUME(DevAssumes, mKeys.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mKeys.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldKey = GetHandle(0);
      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mKeys.mReserved - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            Offset oldBucket = (oldCount + oldIndex) - *oldInfo + 1;
            const auto newBucket = GetBucket(hashmask, oldKey.Get());
            if (oldBucket < oldCount or oldBucket - oldCount != newBucket) {
               // Move element only if it won't end up in same bucket   
               HandleLocal<T> keyswap {Abandon(oldKey)};

               // Destroy the key and info                              
               oldKey.Destroy();
               *oldInfo = 0;
               --mKeys.mCount;

               // Reinsert at the new bucket                            
               InsertInner<false>(newBucket, Abandon(keyswap));
            }
         }

         ++oldKey;
         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      BlockSet::template ShiftPairs<T>();
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @attention assumes count is a power-of-two number                    
   ///   @param count - number of pairs to allocate                           
   TEMPLATE()
   void TABLE()::AllocateInner(const Count& count) {
      // Shrinking is never allowed, you'll have to do it explicitly    
      // via Compact()                                                  
      if (count <= GetReserved())
         return;

      // Allocate/Reallocate the keys and info                          
      if (IsAllocated() and GetUses() == 1)
         AllocateData<true>(count);
      else
         AllocateData<false>(count);
   }

   /// Inner insertion function                                               
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param key - key to move in                                          
   ///   @return the index at which item was inserted                         
   TEMPLATE() template<bool CHECK_FOR_MATCH, class T1>
   Offset TABLE()::InsertInner(const Offset& start, T1&& key) {
      HandleLocal<T> keyswap {Forward<T1>(key)};

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
   
   /// Insert an element, array of elements, or another set                   
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::UnfoldInsert(auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T1 = TypeOf<S>;

      if constexpr (CT::Array<T1>) {
         if constexpr (CT::Inner::MakableFrom<T, Deext<T1>>) {
            // Construct from an array of elements, each of which can   
            // be used to initialize a T, nesting any semantic while    
            // doing it                                                 
            Reserve(GetCount() + ExtentOf<T1>);
            for (auto& key : item) {
               InsertInner<true>(
                  GetBucket(GetReserved() - 1, DesemCast(key)),
                  S::Nest(key)
               );
            }
            return ExtentOf<T1>;
         }
         else if constexpr (CT::Inner::MakableFrom<T, Unfold<Deext<T1>>>) {
            // Construct from an array of elements, which can't be used 
            // to directly construct Ts, so nest the unfolding insert   
            Count inserted = 0;
            for (auto& key : item)
               inserted += UnfoldInsert(S::Nest(key));
            return inserted;
         }
         else LANGULUS_ERROR("Array elements aren't insertable");
      }
      else if constexpr (CT::Inner::MakableFrom<T, T1>) {
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         Reserve(GetCount() + 1);
         InsertInner<true>(
            GetBucket(GetReserved() - 1, DesemCast(item)),
            Forward<T1>(item)
         );
         return 1;
      }
      else if constexpr (CT::Set<T1>) {
         // Construct from any kind of set (ordered or not, type-erased 
         // or not), even sets of a different, but (fold)compatible type
         if constexpr (CT::TypedSet<T1>) {
            // The contained type is known at compile-time              
            using T2 = TypeOf<T1>;
            if constexpr (CT::Inner::MakableFrom<T, T2>) {
               // Each set element can be mapped to one T element       
               Reserve(GetCount() + item.GetCount());
               for (auto& key : item) {
                  InsertInner<true>(
                     GetBucket(GetReserved() - 1, key),
                     S::Nest(key)
                  );
               }
               return item.GetCount();
            }
            else if constexpr (CT::Inner::MakableFrom<T, Unfold<T2>>) {
               // Set elements need to be unfolded, in order to be      
               // inserted as Ts                                        
               Count inserted = 0;
               for (auto& key : item)
                  inserted += UnfoldInsert(S::Nest(key));
               return inserted;
            }
            else LANGULUS_ERROR("Set elements aren't insertable");
         }
         else {
            // The set is type-erased                                   
            LANGULUS_ASSERT(IsSimilar(item.GetType()), Meta, "Type mismatch");
            Reserve(GetCount() + item.GetCount());
            for (auto& key : item) {
               InsertInnerUnknown<true, Ordered>(
                  GetBucketUnknown(GetReserved() - 1, key),
                  S::Nest(key)
               );
            }
            return item.GetCount();
         }
      }
      else LANGULUS_ERROR("Can't insert argument");
   }

   /// Insert elements, semantically or not                                   
   ///   @param t1, tail... - elements to insert                              
   ///   @return the number of inserted elements                              
   TEMPLATE() template<class T1, class... TAIL>
   requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...> LANGULUS(INLINED)
   Count TABLE()::Insert(T1&& t1, TAIL&&...tail){
      Count inserted = 0;
      inserted   += UnfoldInsert(Forward<T1>(t1));
      ((inserted += UnfoldInsert(Forward<TAIL>(tail))), ...);
      return inserted;
   }
   
   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (T1&& rhs){
      Insert(Forward<T1>(rhs));
      return *this;
   }

   /// Destroy everything valid inside the map                                
   TEMPLATE()
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
   TEMPLATE()
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
         const_cast<Allocation*>(mKeys.mEntry)->Free();
         mKeys.ResetMemory();
      }
   }

   /// Clears all data, state, and deallocates                                
   TEMPLATE()
   void TABLE()::Reset() {
      if (not mKeys.mEntry)
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // No point in resetting info, we'll be deallocating it        
         Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
      }
      else {
         // Data is used from multiple locations, just deref values     
         // Notice keys are not dereferenced, we use only value refs    
         // to save on some redundancy                                  
         const_cast<Allocation*>(mKeys.mEntry)->Free();
      }

      mInfo = nullptr;
      mKeys.ResetState();
      mKeys.ResetMemory();
   }
   
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this map instance         
   ///   @param index - the index to remove                                   
   ///   @return the iterator of the previous element, unless index is the    
   ///           first, or at the end already                                 
   TEMPLATE()
   typename TABLE()::Iterator TABLE()::RemoveIt(const Iterator& index) {
      const auto sentinel = GetReserved();
      auto offset = static_cast<Offset>(index.mInfo - mInfo);
      if (offset >= sentinel)
         return end();

      RemoveInner<T>(offset--); //TODO what if map shrinks, offset might become invalid? Doesn't shrink for now
      
      while (offset < sentinel and 0 == mInfo[offset])
         --offset;

      if (offset >= sentinel)
         offset = 0;

      return {mInfo + offset, index.mSentinel, &GetRaw(offset)};
   }

   /// Erase a pair via key                                                   
   ///   @param match - the key to search for                                 
   ///   @return the number of removed pairs                                  
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Remove(const T& match) {
      return BlockSet::template Remove<TABLE(), T>(match);
   }

   /// If possible reallocates the map to a smaller one                       
   TEMPLATE()
   void TABLE()::Compact() {
      TODO();
   }


   ///                                                                        
   ///   SEARCH                                                               
   ///                                                                        
   
   /// Checks if both tables contain the same entries                         
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (const TUnorderedSet& other) const {
      if (other.GetCount() != GetCount())
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            const auto lhs = info - GetInfo();
            const auto rhs = other.template FindInner<TABLE()>(GetRaw(lhs));
            if (rhs == InvalidOffset)
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
   bool TABLE()::Contains(const T& key) const {
      return FindInner<TABLE()>(key) != InvalidOffset;
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TEMPLATE() LANGULUS(INLINED)
   Index TABLE()::Find(const T& key) const {
      const auto offset = FindInner<TABLE()>(key);
      return offset != InvalidOffset ? Index {offset} : IndexNone;
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::FindIt(const T& key) {
      const auto found = FindInner<TABLE()>(key);
      if (found == InvalidOffset)
         return end();

      return {
         GetInfo() + found, GetInfoEnd(),
         &GetRaw(found)
      };
   }
      
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::FindIt(const T& key) const {
      return const_cast<TABLE()*>(this)->FindIt(key);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   const TAny<T>& TABLE()::GetValues() const noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>& TABLE()::GetValues() noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get element at any index, safely                                       
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TEMPLATE()
   T& TABLE()::Get(const CT::Index auto& i) {
      auto offset = mKeys.SimplifyIndex<T, true>(i);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            if (offset == 0)
               return GetRaw(info - GetInfo());
            --offset;
         }
         ++info;
      }

      LANGULUS_THROW(Access, "This shouldn't be reached, ever");
   }
   
   /// Get element at any index, safely (const)                               
   ///   @param i - the index                                                 
   ///   @return the immutable key reference                                  
   TEMPLATE() LANGULUS(INLINED)
   const T& TABLE()::Get(const CT::Index auto& i) const {
      return const_cast<TABLE()*>(this)->Get(i);
   }

   /// Get element at an index, safely                                        
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TEMPLATE() LANGULUS(INLINED)
   T& TABLE()::operator[] (const CT::Index auto& i) {
      return Get(i);
   }

   /// Get element at an index, safely (const)                                
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TEMPLATE() LANGULUS(INLINED)
   const T& TABLE()::operator[] (const CT::Index auto& i) const {
      return Get(i);
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
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::end() noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr};
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
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::end() const noexcept {
      return {GetInfoEnd(), GetInfoEnd(), nullptr};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }
   
   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TEMPLATE() LANGULUS(INLINED)
   T& TABLE()::Last() {
      LANGULUS_ASSERT(not IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);
      return GetRaw(static_cast<Offset>(info - GetInfo()));
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TEMPLATE() LANGULUS(INLINED)
   const T& TABLE()::Last() const {
      LANGULUS_ASSERT(not IsEmpty(), Access, "Can't get last index");
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);
      return GetRaw(static_cast<Offset>(info - GetInfo()));
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TEMPLATE() template<class F>
   Count TABLE()::ForEachElement(F&& f) const {
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

   /// Iterate all keys inside the map, and perform f() on them (mutable)     
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TEMPLATE() template<class F>
   Count TABLE()::ForEachElement(F&& f) {
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

   /// Construct an iterator                                                  
   ///   @param info - the info pointer                                       
   ///   @param sentinel - the end of info pointers                           
   ///   @param value - pointer to the value element                          
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
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
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   typename ITERATOR()& TABLE()::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mInfo == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      const auto previous = mInfo;
      while (not *++mInfo);
      const auto offset = mInfo - previous;
      mValue += offset;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   typename ITERATOR() TABLE()::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   bool TABLE()::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mInfo == rhs.mInfo;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   T& TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return {*const_cast<T*>(mValue)};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   const T& TABLE()::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return {*mValue};
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   T& TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return **this;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE() template<bool MUTABLE> LANGULUS(INLINED)
   const T& TABLE()::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return **this;
   }

} // namespace Langulus::Anyness

#undef ITERATOR
#undef TEMPLATE
#undef TABLE
