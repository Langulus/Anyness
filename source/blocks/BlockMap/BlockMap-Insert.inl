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
      
   ///                                                                        
   /// All possible ways a key and value could be inserted to the map         
   ///                                                                        

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(const CT::NotSemantic auto& k, const CT::NotSemantic auto& v) {
      return Insert<ORDERED>(Copy(k), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(const CT::NotSemantic auto& k, CT::NotSemantic auto& v) {
      return Insert<ORDERED>(Copy(k), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(const CT::NotSemantic auto& k, CT::NotSemantic auto&& v) {
      return Insert<ORDERED>(Copy(k), Move(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(const CT::NotSemantic auto& k, CT::Semantic auto&& v) {
      return Insert<ORDERED>(Copy(k), v.Forward());
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto& k, const CT::NotSemantic auto& v) {
      return Insert<ORDERED>(Copy(k), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto& k, CT::NotSemantic auto& v) {
      return Insert<ORDERED>(Copy(k), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto& k, CT::NotSemantic auto&& v) {
      return Insert<ORDERED>(Copy(k), Move(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto& k, CT::Semantic auto&& v) {
      return Insert<ORDERED>(Copy(k), v.Forward());
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto&& k, const CT::NotSemantic auto& v) {
      return Insert<ORDERED>(Move(k), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto&& k, CT::NotSemantic auto& v) {
      return Insert<ORDERED>(Move(k), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto&& k, CT::NotSemantic auto&& v) {
      return Insert<ORDERED>(Move(k), Move(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::NotSemantic auto&& k, CT::Semantic auto&& v) {
      return Insert<ORDERED>(Move(k), v.Forward());
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::Semantic auto&& k, const CT::NotSemantic auto& v) {
      return Insert<ORDERED>(k.Forward(), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::Semantic auto&& k, CT::NotSemantic auto& v) {
      return Insert<ORDERED>(k.Forward(), Copy(v));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::Semantic auto&& k, CT::NotSemantic auto&& v) {
      return Insert<ORDERED>(k.Forward(), Move(v));
   }
   
   /// Semantically insert key and value                                      
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::Insert(CT::Semantic auto&& key, CT::Semantic auto&& value) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(value)>;
      using K = TypeOf<SK>;
      using V = TypeOf<SV>;

      Mutate<K, V>();
      Reserve(GetCount() + 1);
      InsertInner<true, ORDERED>(
         GetBucket(GetReserved() - 1, *key), 
         key.Forward(), value.Forward()
      );
      return 1;
   }
   
   /// Semantically insert a type-erased pair                                 
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param key - the key to insert                                       
   ///   @param value - the value to insert                                   
   ///   @return 1 if pair was inserted or value was overwritten              
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::InsertBlock(CT::Semantic auto&& key, CT::Semantic auto&& val) {
      using SK = Decay<decltype(key)>;
      using SV = Decay<decltype(val)>;

      static_assert(CT::Exact<TypeOf<SK>, Block>,
         "SK type must be exactly Block (build-time optimization)");
      static_assert(CT::Exact<TypeOf<SV>, Block>,
         "SV type must be exactly Block (build-time optimization)");

      Mutate(key->mType, val->mType);
      Reserve(GetCount() + 1);
      InsertInnerUnknown<true, ORDERED>(
         GetBucketUnknown(GetReserved() - 1, *key),
         key.Forward(), val.Forward()
      );
      return 1;
   }
   
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::InsertPair(const CT::Pair auto& pair) {
      return InsertPair<ORDERED>(Copy(pair));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::InsertPair(CT::Pair auto& pair) {
      return InsertPair<ORDERED>(Copy(pair));
   }

   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::InsertPair(CT::Pair auto&& pair) {
      return InsertPair<ORDERED>(Move(pair));
   }

   /// Semantically insert any pair                                           
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::InsertPair(CT::Semantic auto&& pair) {
      using S = Decay<decltype(pair)>;
      using T = TypeOf<S>;

      if constexpr (CT::TypedPair<T>)
         return Insert<ORDERED>(S::Nest(pair->mKey), S::Nest(pair->mValue));
      else
         return InsertUnknown<ORDERED>(S::Nest(pair->mKey), S::Nest(pair->mValue));
   }

   /// Semantically insert a type-erased pair                                 
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param pair - the pair to insert                                     
   ///   @return 1 if pair was inserted or value was overwritten              
   template<bool ORDERED>
   LANGULUS(INLINED)
   Count BlockMap::InsertPairBlock(CT::Semantic auto&& pair) {
      using S = Decay<decltype(pair)>;
      using T = TypeOf<S>;
      static_assert(CT::Pair<T> and not CT::TypedPair<T>,
         "SP's type must be type-erased pair type");

      return InsertUnknown<ORDERED>(S::Nest(pair->mKey), S::Nest(pair->mValue));
   }

   /// Copy-insert any pair inside the map                                    
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockMap& BlockMap::operator << (const CT::Pair auto& pair) {
      return operator << (Copy(pair));
   }

   /// Copy-insert any pair inside the map                                    
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockMap& BlockMap::operator << (CT::Pair auto& pair) {
      return operator << (Copy(pair));
   }

   /// Move-insert any pair inside the map                                    
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockMap& BlockMap::operator << (CT::Pair auto&& pair) {
      return operator << (Move(pair));
   }

   /// Semantic insertion of any pair inside the map                          
   ///   @param pair - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockMap& BlockMap::operator << (CT::Semantic auto&& pair) {
      InsertPair(pair.Forward());
      return *this;
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
   LANGULUS(INLINED)
   Size BlockMap::RequestKeyAndInfoSize(const Count request, Offset& infoStart) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mKeys.mType, "Key type was not set");
      auto keymemory = request * mKeys.mType->mSize;
      if (mKeys.mType->mIsSparse)
         keymemory *= 2;
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
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param oldCount - the old number of pairs                            
   template<class MAP>
   void BlockMap::Rehash(const Count& oldCount) {
      LANGULUS_ASSUME(DevAssumes, mValues.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mValues.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      static_assert(CT::Map<MAP>, "MAP must be a map type");
      UNUSED() auto& THIS = reinterpret_cast<const MAP&>(*this); //TODO
      auto oldKey = [this] {
         if constexpr (CT::TypedMap<MAP>)
            return GetKeyHandle<typename MAP::Key>(0);
         else
            return GetKeyInner(0);
      }();

      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mValues.mReserved - 1;

      // First run: move elements closer to their new buckets           
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            Offset oldBucket = (oldCount + oldIndex) - *oldInfo + 1;
            auto newBucket = mValues.mReserved;
            if constexpr (CT::TypedMap<MAP>)
               newBucket += GetBucket(hashmask, oldKey.Get());
            else
               newBucket += GetBucketUnknown(hashmask, oldKey);

            if (oldBucket != newBucket) {
               // Move pair only if it won't end up in same bucket      
               if constexpr (CT::TypedMap<MAP>) {
                  using K = typename MAP::Key;
                  using V = typename MAP::Value;

                  auto oldValue = GetValueHandle<V>(oldIndex);
                  HandleLocal<K> keyswap {Abandon(oldKey)};
                  HandleLocal<V> valswap {Abandon(oldValue)};

                  // Destroy the key, info and value                    
                  oldKey.Destroy();
                  oldValue.Destroy();
                  *oldInfo = 0;
                  --mValues.mCount;

                  // Reinsert at the new bucket                         
                  LANGULUS_ASSUME(DevAssumes, newBucket > mValues.mReserved, "Oops");
                  InsertInner<false, MAP::Ordered>(
                     newBucket - mValues.mReserved,
                     Abandon(keyswap), Abandon(valswap)
                  );
               }
               else {
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

                  LANGULUS_ASSUME(DevAssumes, newBucket > mValues.mReserved, "Oops");
                  InsertInnerUnknown<false, MAP::Ordered>(
                     newBucket - mValues.mReserved,
                     Abandon(keyswap), Abandon(valswap)
                  );

                  keyswap.Free();
                  valswap.Free();
               }
            }
         }

         if constexpr (CT::TypedMap<MAP>)
            ++oldKey;
         else
            oldKey.Next();

         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      if constexpr (CT::TypedMap<MAP>)
         ShiftPairs<typename MAP::Key, typename MAP::Value>();
      else
         ShiftPairs<void, void>();
   }
   
   /// Rehashes and reinserts each key in the same block, and moves all       
   /// values in from the provided block                                      
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param oldCount - the old number of pairs                            
   ///   @param values - the source of values                                 
   template<class MAP>
   void BlockMap::RehashKeys(const Count& oldCount, Block& values) {
      LANGULUS_ASSUME(DevAssumes, mValues.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mValues.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      static_assert(CT::Map<MAP>, "MAP must be a map type");
      UNUSED() auto& THIS = reinterpret_cast<const MAP&>(*this); //TODO
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
               
               LANGULUS_ASSUME(DevAssumes, newBucket > mValues.mReserved, "Oops");
               InsertInnerUnknown<false, MAP::Ordered>(
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
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param oldCount - the old number of pairs                            
   ///   @param keys - the source of keys                                     
   template<class MAP>
   void BlockMap::RehashValues(const Count& oldCount, Block& keys) {
      LANGULUS_ASSUME(DevAssumes, mValues.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mValues.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      static_assert(CT::Map<MAP>, "MAP must be a map type");
      UNUSED() auto& THIS = reinterpret_cast<const MAP&>(*this); //TODO
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
               
               LANGULUS_ASSUME(DevAssumes, newBucket > mValues.mReserved, "Oops");
               InsertInnerUnknown<false, MAP::Ordered>(
                  newBucket - mValues.mReserved,
                  Copy(oldKey), Abandon(valswap)
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
            while (mInfo[to] and attempt < *oldInfo) {
               // Might loop around                                     
               ++to;
               if (to >= mValues.mReserved)
                  to -= mValues.mReserved;

               ++attempt;
            }

            if (not mInfo[to] and attempt < *oldInfo) {
               // Empty spot found, so move pair there                  
               if constexpr (CT::Void<K>) {
                  auto key = GetKeyInner(oldIndex);
                  GetKeyInner(to)
                     .CallUnknownSemanticConstructors(1, Abandon(key));
                  key.CallUnknownDestructors();
               }
               else {
                  auto key = GetKeyHandle<Decvq<K>>(oldIndex);
                  GetKeyHandle<Decvq<K>>(to).New(Abandon(key));
                  key.Destroy();
               }

               if constexpr (CT::Void<V>) {
                  auto val = GetValueInner(oldIndex);
                  GetValueInner(to)
                     .CallUnknownSemanticConstructors(1, Abandon(val));
                  val.CallUnknownDestructors();
               }
               else {
                  auto val = GetValueHandle<Decvq<V>>(oldIndex);
                  GetValueHandle<Decvq<V>>(to).New(Abandon(val));
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
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param start - the starting index                                    
   ///   @param key - key & semantic to insert                                
   ///   @param val - value & semantic to insert                              
   ///   @return the offset at which pair was inserted                        
   template<bool CHECK_FOR_MATCH, bool ORDERED>
   Offset BlockMap::InsertInner(
      const Offset& start, CT::Semantic auto&& key, CT::Semantic auto&& val
   ) {
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
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param start - the starting index                                    
   ///   @param key - key to move in                                          
   ///   @param val - value to move in                                        
   ///   @return the offset at which pair was inserted                        
   template<bool CHECK_FOR_MATCH, bool ORDERED>
   Offset BlockMap::InsertInnerUnknown(
      const Offset& start, CT::Semantic auto&& key, CT::Semantic auto&& val
   ) {
      using SK = Deref<decltype(key)>;
      using SV = Deref<decltype(val)>;
      static_assert(CT::Exact<TypeOf<SK>, Block>,
         "SK type must be exactly Block (build-time optimization)");
      static_assert(CT::Exact<TypeOf<SV>, Block>,
         "SV type must be exactly Block (build-time optimization)");

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mValues.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetKeyInner(index);
            if (candidate == *key) {
               // Neat, the key already exists - just set value and go  
               GetValueInner(index)
                  .CallUnknownSemanticAssignment(1, val.Forward());

               if constexpr (SV::Move) {
                  val->CallUnknownDestructors();
                  val->mCount = 0;
               }

               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetKeyInner(index)
               .SwapUnknown(key.Forward());
            GetValueInner(index)
               .SwapUnknown(val.Forward());

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
         .CallUnknownSemanticConstructors(1, val.Forward());

      if (insertedAt == mValues.mReserved)
         insertedAt = index;

      if constexpr (SK::Move) {
         key->CallUnknownDestructors();
         key->mCount = 0;
      }

      if constexpr (SV::Move) {
         val->CallUnknownDestructors();
         val->mCount = 0;
      }

      *psl = attempts;
      ++mValues.mCount;
      return insertedAt;
   }
   
   /// Insert any pair into a preinitialized map                              
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @tparam ORDERED - the bucketing approach to use                      
   ///   @param hashmask - precalculated hashmask                             
   ///   @param pair - the semantic and pair type to insert                   
   template<bool CHECK_FOR_MATCH, bool ORDERED>
   void BlockMap::InsertPairInner(const Count& hashmask, CT::Semantic auto&& pair) {
      using S = Deref<decltype(pair)>;
      using ST = TypeOf<S>;
      static_assert(CT::Pair<ST>, "ST must be a pair type");

      if constexpr (CT::TypedPair<ST>) {
         // Insert a statically typed pair                              
         InsertInner<CHECK_FOR_MATCH, ORDERED>(
            GetBucket(hashmask, pair->mKey),
            S::Nest(pair->mKey),
            S::Nest(pair->mValue)
         );
      }
      else {
         // Insert a type-erased pair                                   
         InsertInnerUnknown<CHECK_FOR_MATCH, ORDERED>(
            GetBucketUnknown(hashmask, pair->mKey),
            S::Nest(pair->mKey),
            S::Nest(pair->mValue)
         );
      }
   }

} // namespace Langulus::Anyness
