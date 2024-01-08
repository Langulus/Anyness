///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"
#include "../../blocks/Block/Block-Insert.inl"


namespace Langulus::Anyness
{
   
   /// Manually insert pair, semantically or not                              
   ///   @param key - the key to insert                                       
   ///   @param val - the value to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::Insert(auto&& key, auto&& val) {
      using SK = SemanticOf<decltype(key)>;
      using SV = SemanticOf<decltype(val)>;

      Mutate<TypeOf<SK>, TypeOf<SV>>();
      Reserve<THIS>(GetCount() + 1);
      InsertInner<THIS, true>(
         GetBucket(GetReserved() - 1, DesemCast(key)), 
         SK::Nest(key), SV::Nest(val)
      );
      return 1;
   }
   
   /// Manually insert type-rased pair, semantically or not                   
   ///   @param key - the key to insert                                       
   ///   @param val - the value to insert                                     
   ///   @return 1 if pair was inserted or value was overwritten              
   template<CT::Map THIS, class T1, class T2>
   requires CT::Block<Desem<T1>, Desem<T2>> LANGULUS(INLINED)
   Count BlockMap::InsertBlock(T1&& key, T2&& val) {
      using SK = SemanticOf<decltype(key)>;
      using SV = SemanticOf<decltype(val)>;

      Mutate(DesemCast(key).mType, DesemCast(val).mType);
      Reserve<THIS>(GetCount() + 1);
      InsertInnerUnknown<THIS, true>(
         GetBucketUnknown(GetReserved() - 1, DesemCast(key)),
         SK::Nest(key), SV::Nest(val)
      );
      return 1;
   }

   /// Unfold-insert pairs, semantically or not                               
   ///   @param t1 - the first pair to insert                                 
   ///   @param tail... - the rest of the pairs to insert (optional)          
   ///   @return the number of inserted pairs                                 
   template<CT::Map THIS, class T1, class...TAIL>
   Count BlockMap::InsertPair(T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
        inserted += UnfoldInsert<THIS>(Forward<T1>(t1));
      ((inserted += UnfoldInsert<THIS>(Forward<TAIL>(tail))), ...);
      return inserted;
   }

   /// Request a new size of keys and info via the value container            
   /// The memory layout is:                                                  
   ///   [keys for each bucket, including entries, if sparse]                 
   ///         [padding for alignment]                                        
   ///               [info for each bucket]                                   
   ///                     [one sentinel byte for terminating loops]          
   ///   @attention assumes key type has been set                             
   ///   @param request - number of keys to allocate                          
   ///   @param infoStart - [out] the offset at which info bytes start        
   ///   @return the requested byte size                                      
   template<CT::Map THIS> LANGULUS(INLINED)
   Size BlockMap::RequestKeyAndInfoSize(
      const Count request, Offset& infoStart
   ) const IF_UNSAFE(noexcept) {
      Size keymemory;
      if constexpr (CT::Typed<THIS>) {
         using K = typename THIS::Key;
         keymemory = request * sizeof(K);
         if constexpr (CT::Sparse<K>)
            keymemory *= 2;
      }
      else {
         LANGULUS_ASSUME(DevAssumes, mKeys.mType, "Key type was not set");
         keymemory = request * mKeys.mType->mSize;
         if (mKeys.mType->mIsSparse)
            keymemory *= 2;
      }

      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + request + 1;
   }

   /// Request a new size of value container                                  
   ///   @param count - number of values to allocate                          
   ///   @return the requested byte size                                      
   LANGULUS(INLINED)
   Size BlockMap::RequestValuesSize(const Count count) const IF_UNSAFE(noexcept) {
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
   template<CT::Map THIS>
   void BlockMap::Rehash(const Count oldCount) {
      LANGULUS_ASSUME(DevAssumes, mValues.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mValues.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldKey = GetKeyHandle<THIS>(0);
      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mValues.mReserved - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            const Offset oldBucket = (oldCount + oldIndex) - *oldInfo + 1;
            Offset newBucket = 0;
            if constexpr (CT::Typed<THIS>)
               newBucket += GetBucket(hashmask, oldKey.Get());
            else
               newBucket += GetBucketUnknown(hashmask, oldKey);

            if (oldBucket < oldCount or oldBucket - oldCount != newBucket) {
               // Move pair only if it won't end up in same bucket      
               if constexpr (CT::Typed<THIS>) {
                  using K = typename THIS::Key;
                  using V = typename THIS::Value;

                  auto oldValue = GetValueHandle<THIS>(oldIndex);
                  HandleLocal<K> keyswap {Abandon(oldKey)};
                  HandleLocal<V> valswap {Abandon(oldValue)};

                  // Destroy the key, info and value                    
                  oldKey.Destroy();
                  oldValue.Destroy();
                  *oldInfo = 0;
                  --mValues.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<THIS, false>(
                     newBucket, Abandon(keyswap), Abandon(valswap)
                  );
               }
               else {
                  Block keyswap {mKeys.GetState(), GetKeyType<THIS>()};
                  keyswap.AllocateFresh<Any>(keyswap.RequestSize<Any>(1));
                  keyswap.CallSemanticConstructors<Any>(1, Abandon(oldKey));
                  keyswap.mCount = 1;

                  auto oldValue = GetValueHandle<THIS>(oldIndex);
                  Block valswap {mValues.GetState(), GetValueType<THIS>()};
                  valswap.AllocateFresh<Any>(valswap.RequestSize<Any>(1));
                  valswap.CallSemanticConstructors<Any>(1, Abandon(oldValue));
                  valswap.mCount = 1;

                  // Destroy the pair and info at old index             
                  oldKey.template CallDestructors<Any>();
                  oldValue.template CallDestructors<Any>();
                  *oldInfo = 0;
                  --mValues.mCount;

                  InsertInnerUnknown<THIS, false>(
                     newBucket, Abandon(keyswap), Abandon(valswap)
                  );

                  keyswap.Free();
                  valswap.Free();
               }
            }
         }

         if constexpr (CT::Typed<THIS>)
            ++oldKey;
         else
            oldKey.Next();

         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      ShiftPairs<THIS>();
   }
   
   /// Rehashes and reinserts each key in the same block, and moves all       
   /// values in from the provided block                                      
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param old - the old block, where keys and values come from          
   template<CT::Map THIS>
   void BlockMap::RehashKeys(BlockMap& old) {
      LANGULUS_ASSUME(DevAssumes, GetReserved() > old.GetReserved(),
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(GetReserved()),
         "New count is not a power-of-two");

      auto oldKey = old.GetKeyHandle<THIS>(0);
      auto oldInfo = old.GetInfo();
      const auto oldInfoEnd = old.GetInfoEnd();
      const auto hashmask = GetReserved() - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            const Offset oldBucket = (old.GetReserved() + oldIndex) - *oldInfo + 1;
            Offset newBucket = 0;
            if constexpr (CT::Typed<THIS>)
               newBucket += GetBucket(hashmask, oldKey.Get());
            else
               newBucket += GetBucketUnknown(hashmask, oldKey);

            if (oldBucket < old.GetReserved() or oldBucket - old.GetReserved() != newBucket) {
               // Move pair only if it won't end up in same bucket      
               if constexpr (CT::Typed<THIS>) {
                  using K = typename THIS::Key;

                  HandleLocal<K> keyswap {Abandon(oldKey)};

                  // Destroy the key, info and value                    
                  oldKey.Destroy();
                  *oldInfo = 0;
                  --mValues.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<THIS, false>(
                     newBucket,
                     Abandon(keyswap),
                     Copy(old.GetValueHandle<THIS>(oldIndex))
                  );
               }
               else {
                  Block keyswap {mKeys.GetState(), GetKeyType<THIS>()};
                  keyswap.AllocateFresh<Any>(keyswap.RequestSize<Any>(1));
                  keyswap.CallSemanticConstructors<Any>(1, Abandon(oldKey));
                  keyswap.mCount = 1;

                  // Destroy the pair and info at old index             
                  oldKey.template CallDestructors<Any>();
                  *oldInfo = 0;
                  --mValues.mCount;

                  InsertInnerUnknown<THIS, false>(
                     newBucket,
                     Abandon(keyswap),
                     Copy(old.GetValueHandle<THIS>(oldIndex))
                  );

                  keyswap.Free();
               }
            }
         }

         if constexpr (CT::Typed<THIS>)
            ++oldKey;
         else
            oldKey.Next();

         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      ShiftPairs<THIS>();
   }
   
   /// Rehashes and reinserts each value in the same block, and moves all     
   /// keys in from the provided block                                        
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param old - the old block, where keys and values come from          
   template<CT::Map THIS>
   void BlockMap::RehashValues(BlockMap& old) {
      LANGULUS_ASSUME(DevAssumes, GetReserved() > old.GetReserved(),
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(GetReserved()),
         "New count is not a power-of-two");

      auto oldKey = old.GetKeyHandle<THIS>(0);
      auto oldInfo = old.GetInfo();
      const auto oldInfoEnd = old.GetInfoEnd();
      const auto hashmask = GetReserved() - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            const Offset oldBucket = (old.GetReserved() + oldIndex) - *oldInfo + 1;
            Offset newBucket = 0;
            if constexpr (CT::Typed<THIS>)
               newBucket += GetBucket(hashmask, oldKey.Get());
            else
               newBucket += GetBucketUnknown(hashmask, oldKey);

            if (oldBucket < old.GetReserved() or oldBucket - old.GetReserved() != newBucket) {
               // Move pair only if it won't end up in same bucket      
               if constexpr (CT::Typed<THIS>) {
                  using V = typename THIS::Value;

                  auto oldValue = old.GetValueHandle<THIS>(oldIndex);
                  HandleLocal<V> valswap {Abandon(oldValue)};

                  // Destroy the key, info and value                    
                  oldValue.Destroy();
                  *oldInfo = 0;
                  --mValues.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<THIS, false>(
                     newBucket, Copy(oldKey), Abandon(valswap));
               }
               else {
                  auto oldValue = old.GetValueHandle<THIS>(oldIndex);
                  Block valswap {mValues.GetState(), GetValueType<THIS>()};
                  valswap.AllocateFresh<Any>(valswap.RequestSize<Any>(1));
                  valswap.CallSemanticConstructors<Any>(1, Abandon(oldValue));
                  valswap.mCount = 1;

                  // Destroy the pair and info at old index             
                  oldValue.template CallDestructors<Any>();
                  *oldInfo = 0;
                  --mValues.mCount;

                  InsertInnerUnknown<THIS, false>(
                     newBucket, Copy(oldKey), Abandon(valswap));

                  valswap.Free();
               }
            }
         }

         if constexpr (CT::Typed<THIS>)
            ++oldKey;
         else
            oldKey.Next();

         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      ShiftPairs<THIS>();
   }
   
   /// Shift elements left, where possible                                    
   template<CT::Map THIS>
   void BlockMap::ShiftPairs() {
      auto oldInfo = mInfo;
      const auto newInfoEnd = GetInfoEnd();
      while (oldInfo != newInfoEnd) {
         if (*oldInfo > 1) {
            const Offset oldIndex = oldInfo - GetInfo();

            // Might loop around                                        
            Offset to = (mKeys.mReserved + oldIndex) - *oldInfo + 1;
            if (to >= mValues.mReserved)
               to -= mValues.mReserved;

            InfoType attempt = 1;
            while (mInfo[to] and attempt < *oldInfo) {
               // Might loop around                                     
               ++to;
               if (to >= mValues.mReserved)
                  to -= mValues.mReserved;
               ++attempt;
            }

            if (not mInfo[to] and attempt < *oldInfo) {
               // Empty spot found, so move pair there                  
               if constexpr (CT::Typed<THIS>) {
                  auto key = GetKeyHandle<THIS>(oldIndex);
                  GetKeyHandle<THIS>(to).New(Abandon(key));
                  key.Destroy();

                  auto val = GetValueHandle<THIS>(oldIndex);
                  GetValueHandle<THIS>(to).New(Abandon(val));
                  val.Destroy();
               }
               else {
                  auto key = GetKeyHandle<THIS>(oldIndex);
                  GetKeyHandle<THIS>(to)
                     .CallUnknownSemanticConstructors(1, Abandon(key));
                  key.CallUnknownDestructors();

                  auto val = GetValueHandle<THIS>(oldIndex);
                  GetValueHandle<THIS>(to)
                     .CallUnknownSemanticConstructors(1, Abandon(val));
                  val.CallUnknownDestructors();
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
   ///   @param start - the starting index                                    
   ///   @param key - key & semantic to insert                                
   ///   @param val - value & semantic to insert                              
   ///   @return the offset at which pair was inserted                        
   template<CT::Map THIS, bool CHECK_FOR_MATCH>
   Offset BlockMap::InsertInner(
      const Offset start, CT::Semantic auto&& key, CT::Semantic auto&& val
   ) {
      static_assert(CT::Typed<THIS>, "THIS must be typed");
      using SK = Deref<decltype(key)>;
      using SV = Deref<decltype(val)>;
      using K = Conditional<CT::Handle<TypeOf<SK>>, TypeOf<TypeOf<SK>>, TypeOf<SK>>;
      using V = Conditional<CT::Handle<TypeOf<SV>>, TypeOf<TypeOf<SV>>, TypeOf<SV>>;
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
            const auto& candidate = GetRawKey<THIS>(index);
            if (keyswapper.Compare(candidate)) {
               // Neat, the key already exists - just set value and go  
               GetValueHandle<THIS>(index).Assign(Abandon(valswapper));
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetKeyHandle<THIS>(index).Swap(keyswapper);
            GetValueHandle<THIS>(index).Swap(valswapper);
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
      GetKeyHandle<THIS>(index).New(Abandon(keyswapper));
      GetValueHandle<THIS>(index).New(Abandon(valswapper));
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
   ///   @param val - value to move in                                        
   ///   @return the offset at which pair was inserted                        
   template<CT::Map THIS, bool CHECK_FOR_MATCH, template<class> class S1, template<class> class S2, CT::Block T>
   requires CT::Semantic<S1<T>, S2<T>>
   Offset BlockMap::InsertInnerUnknown(const Offset start, S1<T>&& key, S2<T>&& val) {
      static_assert(not CT::Typed<THIS>, "THIS must not be typed");

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mValues.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetRawKey<THIS>(index);
            if (candidate == *key) {
               // Neat, the key already exists - just set value and go  
               GetRawValue<THIS>(index)
                  .CallUnknownSemanticAssignment(1, val.Forward());

               if constexpr (S2<T>::Move) {
                  val->CallUnknownDestructors();
                  val->mCount = 0;
               }

               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetRawKey  <THIS>(index).template SwapInner<Any>(key.Forward());
            GetRawValue<THIS>(index).template SwapInner<Any>(val.Forward());

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
      GetRawKey<THIS>(index)
         .CallUnknownSemanticConstructors(1, key.Forward());
      GetRawValue<THIS>(index)
         .CallUnknownSemanticConstructors(1, val.Forward());

      if (insertedAt == mValues.mReserved)
         insertedAt = index;

      if constexpr (S1<T>::Move) {
         key->CallUnknownDestructors();
         key->mCount = 0;
      }

      if constexpr (S2<T>::Move) {
         val->CallUnknownDestructors();
         val->mCount = 0;
      }

      *psl = attempts;
      ++mValues.mCount;
      return insertedAt;
   }
   
   /// Insert any pair into a preinitialized map                              
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param hashmask - precalculated hashmask                             
   ///   @param pair - the semantic and pair type to insert                   
   template<CT::Map THIS, bool CHECK_FOR_MATCH, template<class> class S, CT::Pair T>
   requires CT::Semantic<S<T>>
   void BlockMap::InsertPairInner(const Count hashmask, S<T>&& pair) {
      if constexpr (CT::TypedPair<T>) {
         // Insert a statically typed pair                              
         InsertInner<THIS, CHECK_FOR_MATCH>(
            GetBucket(hashmask, pair->mKey),
            S<T>::Nest(pair->mKey),
            S<T>::Nest(pair->mValue)
         );
      }
      else {
         // Insert a type-erased pair                                   
         InsertInnerUnknown<THIS, CHECK_FOR_MATCH>(
            GetBucketUnknown(hashmask, pair->mKey),
            S<T>::Nest(pair->mKey),
            S<T>::Nest(pair->mValue)
         );
      }
   }

} // namespace Langulus::Anyness
