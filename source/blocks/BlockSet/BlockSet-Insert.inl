///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockSet.hpp"
#include "../../blocks/Block/Block-Insert.inl"


namespace Langulus::Anyness
{
      
   ///                                                                        
   /// All possible ways a key could be inserted to the set                   
   ///                                                                        
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::Insert(const CT::NotSemantic auto& k) {
      return Insert<ORDERED>(Copy(k));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::Insert(CT::NotSemantic auto& k) {
      return Insert<ORDERED>(Copy(k));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::Insert(CT::NotSemantic auto&& k) {
      return Insert<ORDERED>(Move(k));
   }
   
   /// Semantically insert key                                                
   ///   @param key - the key to insert                                       
   ///   @return 1 if key was inserted, zero otherwise                        
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::Insert(CT::Semantic auto&& key) {
      using S = Decay<decltype(key)>;
      using K = TypeOf<S>;

      Mutate<K>();
      Reserve(GetCount() + 1);
      InsertInner<true, ORDERED>(
         GetBucket(GetReserved() - 1, *key), 
         key.Forward()
      );
      return 1;
   }
   
   /// Semantically insert a type-erased key                                  
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param key - the key to insert                                       
   ///   @return 1 if key was inserted                                        
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::InsertBlock(CT::Semantic auto&& key) {
      using S = Decay<decltype(key)>;

      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      Mutate(key->mType);
      Reserve(GetCount() + 1);
      InsertInnerUnknown<true, ORDERED>(
         GetBucketUnknown(GetReserved() - 1, *key),
         key.Forward()
      );
      return 1;
   }

   /// Merge the contents of two sets by shallow copy                         
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::Merge(const BlockSet& set) {
      return Merge<ORDERED>(Copy(set));
   }
 
   /// Merge the contents of two sets by move                                 
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::Merge(BlockSet&& set) {
      return Merge<ORDERED>(Move(set));
   }
 
   /// Merge the contents of two sets by using a semantic                     
   ///   @param set - the set to merge with this one                          
   ///   @return the size of provided set                                     
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockSet::Merge(CT::Semantic auto&& set) {
      using S = Decay<decltype(set)>;
      using T = TypeOf<S>;
      static_assert(CT::Set<T>, "You can only merge other sets");
 
      if constexpr (CT::Typed<T>) {
         // Merging with a statically typed set                         
         Mutate<TypeOf<T>>(set->GetType());
         Reserve(GetCount() + set->GetCount());
 
         for (auto& it : *set) {
            InsertInner<true, ORDERED>(
               GetBucket(GetReserved() - 1, it),
               S::Nest(it)
            );
         }
      }
      else {
         // Merging with a type-erased set                              
         Mutate(set->GetType());
         Reserve(GetCount() + set->GetCount());
 
         for (Block it : static_cast<const BlockSet&>(*set)) {
            InsertInnerUnknown<true, ORDERED>(
               GetBucketUnknown(GetReserved() - 1, it),
               S::Nest(it)
            );
         }
      }

      return set->GetCount();
   }

   /// Merge an element via copy                                              
   ///   @param item - the value to merge                                     
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (const CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }

   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }

   /// Merge an element via move                                              
   ///   @param item - the value to merge                                     
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (CT::NotSemantic auto&& item) {
      return operator << (Move(item));
   }

   /// Merge an element via semantic                                          
   ///   @param item - the value to merge                                     
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (CT::Semantic auto&& item) {
      Insert(item.Forward());
      return *this;
   }
   
   /// Request a new size of keys and info                                    
   /// The memory layout is:                                                  
   ///   [keys for each bucket, including entries, if sparse]                 
   ///         [padding for alignment]                                        
   ///               [info for each bucket]                                   
   ///                     [one sentinel byte for terminating loops]          
   ///   @attention assumes key type has been set                             
   ///   @param request - number of keys to allocate                          
   ///   @param infoStart - [out] the offset at which info bytes start        
   ///   @return the requested byte size                                      
   LANGULUS(INLINED)
   Size BlockSet::RequestKeyAndInfoSize(const Count request, Offset& infoStart) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mKeys.mType, "Key type was not set");
      auto keymemory = request * mKeys.mType->mSize;
      if (mKeys.mType->mIsSparse)
         keymemory *= 2;
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + request + 1;
   }

   /// Rehashes and reinserts each pair in the same block                     
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param oldCount - the old number of pairs                            
   template<class SET>
   void BlockSet::Rehash(const Count& oldCount) {
      LANGULUS_ASSUME(DevAssumes, mKeys.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mKeys.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      static_assert(CT::Set<SET>, "SET must be a set type");
      UNUSED() auto& THIS = reinterpret_cast<const SET&>(*this); //TODO
      auto oldKey = [this] {
         if constexpr (CT::TypedSet<SET>)
            return GetHandle<TypeOf<SET>>(0);
         else
            return GetInner(0);
      }();

      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mKeys.mReserved - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            Offset oldBucket = (oldCount + oldIndex) - *oldInfo + 1;
            Offset newBucket = 0;
            if constexpr (CT::TypedSet<SET>)
               newBucket += GetBucket(hashmask, oldKey.Get());
            else
               newBucket += GetBucketUnknown(hashmask, oldKey);

            if (oldBucket < oldCount or oldBucket - oldCount != newBucket) {
               // Move it only if it won't end up in same bucket        
               if constexpr (CT::TypedSet<SET>) {
                  using K = TypeOf<SET>;

                  HandleLocal<K> keyswap {Abandon(oldKey)};

                  // Destroy the key, info and value                    
                  oldKey.Destroy();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<false, SET::Ordered>(
                     newBucket, Abandon(keyswap)
                  );
               }
               else {
                  Block keyswap {mKeys.GetState(), GetType()};
                  keyswap.AllocateFresh(keyswap.RequestSize(1));
                  keyswap.CallUnknownSemanticConstructors(1, Abandon(oldKey));
                  keyswap.mCount = 1;

                  // Destroy the pair and info at old index             
                  oldKey.CallUnknownDestructors();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  InsertInnerUnknown<false, SET::Ordered>(
                     newBucket, Abandon(keyswap)
                  );

                  keyswap.Free();
               }
            }
         }

         if constexpr (CT::TypedSet<SET>)
            ++oldKey;
         else
            oldKey.Next();

         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      if constexpr (CT::TypedSet<SET>)
         ShiftPairs<TypeOf<SET>>();
      else
         ShiftPairs<void>();
   }
   
   /// Shift elements left, where possible                                    
   ///   @param K - type of key (use void for type-erasure)                   
   template<class K>
   void BlockSet::ShiftPairs() {
      auto oldInfo = mInfo;
      const auto newInfoEnd = GetInfoEnd();
      while (oldInfo != newInfoEnd) {
         if (*oldInfo > 1) {
            const Offset oldIndex = oldInfo - GetInfo();

            // Might loop around                                        
            Offset to = (mKeys.mReserved + oldIndex) - *oldInfo + 1;
            if (to >= mKeys.mReserved)
               to -= mKeys.mReserved;

            InfoType attempt = 1;
            while (mInfo[to] and attempt < *oldInfo) {
               // Might loop around                                     
               ++to;
               if (to >= mKeys.mReserved)
                  to -= mKeys.mReserved;
               ++attempt;
            }

            if (not mInfo[to] and attempt < *oldInfo) {
               // Empty spot found, so move element there               
               if constexpr (CT::Void<K>) {
                  auto key = GetInner(oldIndex);
                  GetInner(to)
                     .CallUnknownSemanticConstructors(1, Abandon(key));
                  key.CallUnknownDestructors();
               }
               else {
                  auto key = GetHandle<Decvq<K>>(oldIndex);
                  GetHandle<Decvq<K>>(to).New(Abandon(key));
                  key.Destroy();
               }

               mInfo[to] = attempt;
               *oldInfo = 0;
            }
         }

         ++oldInfo;
      }
   }

   /// Inner insertion function                                               
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param start - the starting index                                    
   ///   @param key - key & semantic to insert                                
   ///   @return the offset at which pair was inserted                        
   template<bool CHECK_FOR_MATCH, bool ORDERED>
   Offset BlockSet::InsertInner(const Offset& start, CT::Semantic auto&& key) {
      using S = Deref<decltype(key)>;
      using K = Conditional<CT::Handle<TypeOf<S>>, TypeOf<TypeOf<S>>, TypeOf<S>>;
      HandleLocal<K> keyswapper {key.Forward()};

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            const auto& candidate = GetRaw<K>(index);
            if (keyswapper.Compare(candidate)) {
               // Neat, the value already exists - just return          
               return index;
            }
         }

         if (attempts > *psl) {
            // The value we're inserting is closer to bucket, so swap   
            GetHandle<K>(index).Swap(keyswapper);
            ::std::swap(attempts, *psl);
            if (insertedAt == mKeys.mReserved)
               insertedAt = index;
         }

         ++attempts;

         // Wrap around and start from the beginning if we have to      
         if (psl < pslEnd - 1)
            ++psl;
         else 
            psl = GetInfo();
      }

      // If reached, empty slot reached, so put the value there         
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      const auto index = psl - GetInfo();
      GetHandle<K>(index).New(Abandon(keyswapper));
      if (insertedAt == mKeys.mReserved)
         insertedAt = index;

      *psl = attempts;
      ++mKeys.mCount;
      return insertedAt;
   }
   
   /// Inner insertion function based on reflected move-assignment            
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param start - the starting index                                    
   ///   @param key - value & semantic to insert                              
   ///   @return the offset at which pair was inserted                        
   template<bool CHECK_FOR_MATCH, bool ORDERED>
   Offset BlockSet::InsertInnerUnknown(const Offset& start, CT::Semantic auto&& key) {
      using S = Deref<decltype(key)>;
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetInner(index);
            if (candidate == *key) {
               // Neat, the key already exists - just return            
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetInner(index).SwapUnknown(key.Forward());
            ::std::swap(attempts, *psl);
            if (insertedAt == mKeys.mReserved)
               insertedAt = index;
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
      // eventually reached, unless element exists and returns early    
      // We're moving only a single element, so no chance of overlap    
      const auto index = psl - GetInfo();
      GetInner(index)
         .CallUnknownSemanticConstructors(1, key.Forward());

      if (insertedAt == mKeys.mReserved)
         insertedAt = index;

      if constexpr (S::Move) {
         key->CallUnknownDestructors();
         key->mCount = 0;
      }

      *psl = attempts;
      ++mKeys.mCount;
      return insertedAt;
   }

} // namespace Langulus::Anyness
