///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"

namespace Langulus::Anyness
{
   
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
   LANGULUS(INLINED)
   Size BlockMap::RequestKeyAndInfoSize(const Count count, Offset& infoStart) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mKeys.mType, "Key type was not set");
      auto keymemory = count * mKeys.mType->mSize;
      if (mKeys.mType->mIsSparse)
         keymemory *= 2;
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + count + 1;
   }

   /// Request a new size of value container                                  
   ///   @param count - number of values to allocate                          
   ///   @return the requested byte size                                      
   LANGULUS(INLINED)
   Size BlockMap::RequestValuesSize(const Count count) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mValues.mType, "Value type was not set");
      auto valueByteSize = count * mValues.mType->mSize;
      if (mValues.mType->mIsSparse)
         valueByteSize *= 2;
      return valueByteSize;
   }

   /// Rehashes and reinserts each pair in the same block                     
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param oldCount - the old number of pairs                            
   inline void BlockMap::Rehash(const Count& oldCount) {
      LANGULUS_ASSUME(DevAssumes, mValues.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mValues.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldKey = GetKeyInner(0);
      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mValues.mReserved - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            Offset oldBucket = (oldCount + oldIndex) - *oldInfo + 1;
            const auto newBucket = mValues.mReserved + GetBucketUnknown(hashmask, oldKey);
            if (oldBucket != newBucket) {
               // Move pair only if it won't end up in same bucket      
               Block keyswap {mKeys.GetState(), GetKeyType()};
               keyswap.AllocateFresh(keyswap.RequestSize(1));
               keyswap.CallUnknownSemanticConstructors(1, Abandon(oldKey));
               keyswap.mCount = 1;

               auto oldValue = GetValueInner(oldIndex);
               Block valswap {mValues.GetState(), GetValueType()};
               valswap.AllocateFresh(valswap.RequestSize(1));
               valswap.CallUnknownSemanticConstructors(1, Abandon(oldValue));
               valswap.mCount = 1;
               
               // Destroy the pair and info at old index                
               oldKey.CallUnknownDestructors();
               oldValue.CallUnknownDestructors();
               *oldInfo = 0;
               --mValues.mCount;
               
               InsertInnerUnknown<false>(
                  newBucket - mValues.mReserved, Abandon(keyswap), Abandon(valswap)
               );

               keyswap.Free();
               valswap.Free();
            }
         }

         oldKey.Next();
         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      ShiftPairs<void, void>();
   }
   
   /// Rehashes and reinserts each key in the same block, and moves all       
   /// values in from the provided block                                      
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param oldCount - the old number of pairs                            
   ///   @param values - the source of values                                 
   inline void BlockMap::RehashKeys(const Count& oldCount, Block& values) {
      LANGULUS_ASSUME(DevAssumes, mValues.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mValues.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldKey = GetKeyInner(0);
      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mValues.mReserved - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            Offset oldBucket = (oldCount + oldIndex) - *oldInfo + 1;
            const auto newBucket = mValues.mReserved + GetBucketUnknown(hashmask, oldKey);
            if (oldBucket != newBucket) {
               // Move pair only if it won't end up in same bucket      
               Block keyswap {mKeys.GetState(), GetKeyType()};
               keyswap.AllocateFresh(keyswap.RequestSize(1));
               keyswap.CallUnknownSemanticConstructors(1, Abandon(oldKey));
               keyswap.mCount = 1;
               
               // Destroy the pair and info at old index                
               oldKey.CallUnknownDestructors();
               *oldInfo = 0;
               --mValues.mCount;
               
               InsertInnerUnknown<false>(
                  newBucket - mValues.mReserved, 
                  Abandon(keyswap), 
                  Copy(values.GetElement(oldIndex))
               );

               keyswap.Free();
            }
         }

         oldKey.Next();
         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      ShiftPairs<void, void>();
   }
   
   /// Rehashes and reinserts each value in the same block, and moves all     
   /// keys in from the provided block                                        
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param oldCount - the old number of pairs                            
   ///   @param keys - the source of keys                                     
   inline void BlockMap::RehashValues(const Count& oldCount, Block& keys) {
      LANGULUS_ASSUME(DevAssumes, mValues.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mValues.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldKey = keys.GetElement(0);
      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mValues.mReserved - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            Offset oldBucket = (oldCount + oldIndex) - *oldInfo + 1;
            const auto newBucket = mValues.mReserved + GetBucketUnknown(hashmask, oldKey);
            if (oldBucket != newBucket) {
               // Move pair only if it won't end up in same bucket      
               auto oldValue = GetValueInner(oldIndex);
               Block valswap {mValues.GetState(), GetValueType()};
               valswap.AllocateFresh(valswap.RequestSize(1));
               valswap.CallUnknownSemanticConstructors(1, Abandon(oldValue));
               valswap.mCount = 1;
               
               // Destroy the pair and info at old index                
               oldValue.CallUnknownDestructors();
               *oldInfo = 0;
               --mValues.mCount;
               
               InsertInnerUnknown<false>(
                  newBucket - mValues.mReserved,
                  Copy(oldKey),
                  Abandon(valswap)
               );

               valswap.Free();
            }
         }

         oldKey.Next();
         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      ShiftPairs<void, void>();
   }
   
   /// Shift elements left, where possible                                    
   ///   @param K - type of key (use void for type-erasure)                   
   ///   @param V - type of value (use void for type-erasure)                 
   template<class K, class V>
   void BlockMap::ShiftPairs() {
      auto oldInfo = mInfo;
      const auto newInfoEnd = GetInfoEnd();
      while (oldInfo != newInfoEnd) {
         if (*oldInfo > 1) {
            const Offset oldIndex = oldInfo - GetInfo();

            // Might loop around                                        
            Offset to = oldIndex - (*oldInfo - 1);
            if (to >= mValues.mReserved)
               to += mValues.mReserved;

            InfoType attempt = 1;
            while (mInfo[to] && attempt < *oldInfo) {
               // Might loop around                                     
               ++to;
               if (to >= mValues.mReserved)
                  to -= mValues.mReserved;

               ++attempt;
            }

            if (!mInfo[to] && attempt < *oldInfo) {
               // Empty spot found, so move pair there                  
               if constexpr (CT::Void<K>) {
                  auto key = GetKeyInner(oldIndex);
                  GetKeyInner(to).CallUnknownSemanticConstructors(
                     1, Abandon(key));
                  key.CallUnknownDestructors();
               }
               else {
                  auto key = GetKeyHandle<K>(oldIndex);
                  GetKeyHandle<K>(to).New(Abandon(key));
                  key.Destroy();
               }

               if constexpr (CT::Void<V>) {
                  auto val = GetValueInner(oldIndex);
                  GetValueInner(to).CallUnknownSemanticConstructors(
                     1, Abandon(val));
                  val.CallUnknownDestructors();
               }
               else {
                  auto val = GetValueHandle<V>(oldIndex);
                  GetValueHandle<V>(to).New(Abandon(val));
                  val.Destroy();
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
   ///   @tparam SK - key type and semantic (deducible)                       
   ///   @tparam SV - value type and semantic (deducible)                     
   ///   @param start - the starting index                                    
   ///   @param key - key & semantic to insert                                
   ///   @param value - value & semantic to insert                            
   ///   @return the offset at which pair was inserted                        
   template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
   Offset BlockMap::InsertInner(const Offset& start, SK&& key, SV&& val) {
      using K = TypeOf<SK>;
      using V = TypeOf<SV>;
      HandleLocal<K> keyswapper {key.Forward()};
      HandleLocal<V> valswapper {val.Forward()};

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mValues.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            const auto& candidate = GetRawKey<K>(index);
            if (keyswapper.Compare(candidate)) {
               // Neat, the key already exists - just set value and go  
               GetValueHandle<V>(index).Assign(Abandon(valswapper));
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetKeyHandle<K>(index).Swap(keyswapper);
            GetValueHandle<V>(index).Swap(valswapper);
            ::std::swap(attempts, *psl);
            if (insertedAt == mValues.mReserved)
               insertedAt = index;
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
      GetKeyHandle<K>(index).New(Abandon(keyswapper));
      GetValueHandle<V>(index).New(Abandon(valswapper));
      if (insertedAt == mValues.mReserved)
         insertedAt = index;

      *psl = attempts;
      ++mValues.mCount;
      return insertedAt;
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
      Offset insertedAt = mValues.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetKeyInner(index);
            if (candidate == key.mValue) {
               // Neat, the key already exists - just set value and go  
               GetValueInner(index)
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
            GetKeyInner(index).SwapUnknown(key.Forward());
            GetValueInner(index).SwapUnknown(value.Forward());
            ::std::swap(attempts, *psl);
            if (insertedAt == mValues.mReserved)
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
      // eventually reached, unless key exists and returns early        
      // We're moving only a single element, so no chance of overlap    
      const auto index = psl - GetInfo();
      GetKeyInner(index)
         .CallUnknownSemanticConstructors(1, key.Forward());
      GetValueInner(index)
         .CallUnknownSemanticConstructors(1, value.Forward());

      if (insertedAt == mValues.mReserved)
         insertedAt = index;

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
      return insertedAt;
   }
   
   /// Insert any pair into a preinitialized map                              
   ///   @tparam THIS - used to reinterpret this container to call optimized  
   ///                  functions                                             
   ///   @tparam S - the semantic to use for the insertion (deducible)        
   ///   @param hashmask - precalculated hashmask                             
   ///   @param other - the semantic and pair type to insert                  
   template<class THIS, CT::Semantic S>
   void BlockMap::InsertPairInner(const Count& hashmask, S&& other) {
      static_assert(CT::Map<THIS>, "THIS must be a map type");
      auto& This = reinterpret_cast<THIS&>(*this);

      if constexpr (CT::TypedPair<TypeOf<S>>) {
         // Insert a statically typed pair                              
         This.template InsertInner<false>(
            This.GetBucket(hashmask, other.mValue.mKey),
            S::Nest(other.mValue.mKey),
            S::Nest(other.mValue.mValue)
         );
      }
      else {
         // Insert a type-erased pair                                   
         This.template InsertInnerUnknown<false>(
            This.GetBucketUnknown(hashmask, other.mValue.mKey),
            S::Nest(other.mValue.mKey),
            S::Nest(other.mValue.mValue)
         );
      }
   }

} // namespace Langulus::Anyness
