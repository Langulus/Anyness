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

   /// Wrap the argument into a handle with key's type                        
   ///   @attention if key is a type-erased handle or void*, we assume that   
   ///      the pointer always points to a valid instance of the current      
   ///      key type                                                          
   ///   @param key - the key to wrap, with or without intent                 
   ///   @return the handle object                                            
   template<CT::Map THIS>
   auto BlockMap::CreateKeyHandle(auto&& key) {
      using S = IntentOf<decltype(key)>;
      using T = TypeOf<S>;

      if constexpr (CT::Typed<THIS>) {
         using K = Conditional<CT::Typed<THIS>, typename THIS::Key, TypeOf<T>>;
         return HandleLocal<K> {S::Nest(key)};
      }
      else {
         // Make sure that value is always inserted, and never absorbed 
         auto result = Many::Wrap(S::Nest(key));
         // And make sure that type is set to the contained value type  
         result.mType = mKeys.mType;
         return result;
      }
   }

   /// Wrap the argument into a handle with value's type                      
   ///   @attention if value is a type-erased handle or void*, we assume that 
   ///      the pointer always points to a valid instance of the current      
   ///      value type                                                        
   ///   @param val - the value to wrap, with or without intent               
   ///   @return the handle object                                            
   template<CT::Map THIS>
   auto BlockMap::CreateValHandle(auto&& val) {
      using S = IntentOf<decltype(val)>;
      using T = TypeOf<S>;

      if constexpr (CT::Typed<THIS>) {
         using V = Conditional<CT::Typed<THIS>, typename THIS::Value, TypeOf<T>>;
         return HandleLocal<V> {S::Nest(val)};
      }
      else {
         // Make sure that value is always inserted, and never absorbed 
         auto result = Many::Wrap(S::Nest(val));
         // And make sure that type is set to the contained value type  
         result.mType = mValues.mType;
         return result;
      }
   }

   /// Insert a pair, or an array of pairs                                    
   ///   @param item - the argument to unfold and insert, can have intent     
   ///   @return the number of inserted elements after unfolding              
   template<CT::Map THIS>
   Count BlockMap::UnfoldInsert(auto&& item) {
      using E = Conditional<CT::Typed<THIS>, typename THIS::Pair, Anyness::Pair>;
      using S = IntentOf<decltype(item)>;
      using T = TypeOf<S>;

      if constexpr (CT::Typed<THIS>) {
         mKeys.mType = MetaDataOf<typename THIS::Key>();
         mValues.mType = MetaDataOf<typename THIS::Value>();
      }

      Count inserted = 0;
      if constexpr (CT::Array<T>) {
         if constexpr (CT::Typed<THIS>) {
            if constexpr (CT::MakableFrom<E, Deext<T>>) {
               // Construct from an array of elements, each of which    
               // can be used to initialize a pair, nesting any intent  
               // while at it                                           
               Reserve<THIS>(GetCount() + ExtentOf<T>);
               const auto mask = GetReserved() - 1;
               for (auto& pair : DeintCast(item))
                  inserted += InsertPairInner<THIS, true>(mask, S::Nest(pair));
            }
            else if constexpr (CT::MakableFrom<E, CT::Unfold<Deext<T>>>) {
               // Construct from an array of things, which can't be used
               // to directly construct elements, so nest this insert   
               for (auto& pair : DeintCast(item))
                  inserted += UnfoldInsert<THIS>(S::Nest(pair));
            }
            else LANGULUS_ERROR("Array elements aren't insertable as pairs");
         }
         else {
            // Insert the array                                         
            const auto& firstPair = DeintCast(item)[0];
            Mutate<THIS>(
               firstPair.GetKeyBlock().GetType(),
               firstPair.GetValueBlock().GetType()
            );
            Reserve<THIS>(GetCount() + ExtentOf<T>);
            const auto mask = GetReserved() - 1;
            for (auto& pair : DeintCast(item)) {
               Mutate<THIS>(
                  pair.GetKeyBlock().GetType(),
                  pair.GetValueBlock().GetType()
               );
               inserted += InsertPairInner<THIS, true>(mask, S::Nest(pair));
            }
         }
      }
      else if constexpr (CT::Typed<THIS>) {
         if constexpr (CT::MakableFrom<E, T>) {
            // Some of the arguments might still be used directly to    
            // make a pair, forward these to standard insertion here    
            Reserve<THIS>(GetCount() + 1);
            const auto mask = GetReserved() - 1;
            inserted += InsertPairInner<THIS, true>(mask, S::Nest(item));
         }
         else if constexpr (CT::Map<T>) {
            // Construct from any kind of map                           
            if constexpr (CT::Typed<T>) {
               // The contained type is known at compile-time           
               using T2 = TypeOf<T>;

               if constexpr (CT::MakableFrom<E, T2>) {
                  // Elements are mappable                              
                  Reserve<THIS>(GetCount() + DeintCast(item).GetCount());
                  const auto mask = GetReserved() - 1;
                  for (auto& pair : DeintCast(item))
                     inserted += InsertPairInner<THIS, true>(mask, S::Nest(pair));
               }
               else if constexpr (CT::MakableFrom<E, CT::Unfold<T2>>) {
                  // Map pairs need to be unfolded one by one           
                  for (auto& pair : DeintCast(item))
                     inserted += UnfoldInsert<THIS>(S::Nest(pair));
               }
               else LANGULUS_ERROR("Maps aren't mappable to each other");
            }
            else {
               // The rhs map is type-erased                            
               Mutate<THIS>(
                  DeintCast(item).GetKeyType(),
                  DeintCast(item).GetValueType()
               );
               Reserve<THIS>(GetCount() + DeintCast(item).GetCount());
               const auto mask = GetReserved() - 1;
               for (auto& pair : DeintCast(item))
                  inserted += InsertPairInner<THIS, true>(mask, S::Nest(pair));
            }
         }
         else LANGULUS_ERROR("Can't insert argument");
      }
      else if constexpr (CT::Pair<T>) {
         // This map is type-erased                                     
         // Some of the arguments might still be used directly to       
         // make pairs, forward these to standard insertion here        
         Mutate<THIS>(
            DeintCast(item).GetKeyBlock().GetType(),
            DeintCast(item).GetValueBlock().GetType()
         );
         Reserve<THIS>(GetCount() + 1);
         const auto mask = GetReserved() - 1;
         inserted += InsertPairInner<THIS, true>(mask, S::Nest(item));
      }
      else LANGULUS_ERROR("T isn't a pair/map, or an array of pairs/maps");

      return inserted;
   }

   /// Manually insert pair, with or without intent                           
   ///   @param key - the key to insert                                       
   ///   @param val - the value to insert                                     
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::Insert(auto&& key, auto&& val) {
      using SK = IntentOf<decltype(key)>;
      using SV = IntentOf<decltype(val)>;

      Mutate<THIS, TypeOf<SK>, TypeOf<SV>>();
      Reserve<THIS>(GetCount() + 1);
      InsertInner<THIS, true>(
         GetBucket(GetReserved() - 1, DeintCast(key)), 
         SK::Nest(key), SV::Nest(val)
      );
      return 1;
   }
   
   /// Manually insert type-rased pair, with or without intent                
   ///   @param key - the key to insert                                       
   ///   @param val - the value to insert                                     
   ///   @return 1 if pair was inserted or value was overwritten              
   template<CT::Map THIS, class T1, class T2>
   requires CT::Block<Deint<T1>, Deint<T2>> LANGULUS(INLINED)
   Count BlockMap::InsertBlock(T1&& key, T2&& val) {
      using SK = IntentOf<decltype(key)>;
      using SV = IntentOf<decltype(val)>;
      using KB = TypeOf<SK>;
      using VB = TypeOf<SV>;

      // Type checks and mutations                                      
      if constexpr (CT::Typed<KB, VB>)
         Mutate<THIS, TypeOf<KB>, TypeOf<VB>>();
      else {
         Mutate<THIS>(
            DeintCast(key).GetType(),
            DeintCast(val).GetType()
         );
      }

      const auto count = ::std::min(
         DeintCast(key).GetCount(),
         DeintCast(val).GetCount()
      );

      Reserve<THIS>(GetCount() + count);

      for (Offset i = 0; i < count; ++i) {
         if constexpr (not CT::Typed<KB> or not CT::Typed<VB>) {
            // Type-erased insertion                                    
            auto keyBlock = DeintCast(key).GetElement(i);
            InsertBlockInner<THIS, true>(
               GetBucketUnknown(GetReserved() - 1, keyBlock),
               SK::Nest(keyBlock),
               SV::Nest(DeintCast(val).GetElement(i))
            );
         }
         else {
            // Static type insertion                                    
            auto& keyRef = DeintCast(key)[i];
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, keyRef),
               SK::Nest(keyRef),
               SV::Nest(DeintCast(val)[i])
            );
         }
      }

      return count;
   }

   /// Unfold-insert pairs, with or without intent                            
   ///   @param t1 - the first pair to insert                                 
   ///   @param tn... - the rest of the pairs to insert (optional)            
   ///   @return the number of inserted pairs                                 
   template<CT::Map THIS, class T1, class...TN>
   Count BlockMap::InsertPair(T1&& t1, TN&&...tn) {
      Count inserted = 0;
        inserted += UnfoldInsert<THIS>(Forward<T1>(t1));
      ((inserted += UnfoldInsert<THIS>(Forward<TN>(tn))), ...);
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
      Offset keymemory;
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
      LANGULUS_ASSUME(DevAssumes, mKeys.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mKeys.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldKey = GetKeyHandle<THIS>(0);
      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = mKeys.mReserved - 1;

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

                  auto oldValue = GetValHandle<THIS>(oldIndex);
                  HandleLocal<K> keyswap {Abandon(oldKey)};
                  HandleLocal<V> valswap {Abandon(oldValue)};

                  // Destroy the key, info and value                    
                  oldKey.FreeInner();
                  oldValue.FreeInner();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<THIS, false>(
                     newBucket, Abandon(keyswap), Abandon(valswap)
                  );
               }
               else {
                  Block<> keyswap {mKeys.GetState(), GetKeyType<THIS>(), 1};
                  keyswap.AllocateFresh(keyswap.RequestSize(1));
                  keyswap.CreateWithIntent(Abandon(oldKey));

                  auto oldValue = GetValHandle<THIS>(oldIndex);
                  Block<> valswap {mValues.GetState(), GetValueType<THIS>(), 1};
                  valswap.AllocateFresh(valswap.RequestSize(1));
                  valswap.CreateWithIntent(Abandon(oldValue));

                  // Destroy the pair and info at old index             
                  oldKey.FreeInner();
                  oldValue.FreeInner();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  InsertBlockInner<THIS, false>(
                     newBucket, Abandon(keyswap), Abandon(valswap)
                  );

                  keyswap.Free();
                  valswap.Free();
               }
            }
         }

         ++oldKey;
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
                  oldKey.FreeInner();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<THIS, false>(newBucket,
                     Abandon(keyswap),
                     Abandon(old.GetValHandle<THIS>(oldIndex))
                  );
               }
               else {
                  Block<> keyswap {mKeys.GetState(), GetKeyType<THIS>(), 1};
                  keyswap.AllocateFresh(keyswap.RequestSize(1));
                  keyswap.CreateWithIntent(Abandon(oldKey));

                  // Destroy the pair and info at old index             
                  oldKey.FreeInner();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  InsertBlockInner<THIS, false>(newBucket,
                     Abandon(keyswap),
                     Abandon(old.GetValHandle<THIS>(oldIndex))
                  );

                  keyswap.Free();
               }
            }
         }

         ++oldKey;
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
   void BlockMap::RehashVals(BlockMap& old) {
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
            const Offset oldIndex  = oldInfo - GetInfo();
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

                  auto oldValue = old.GetValHandle<THIS>(oldIndex);
                  HandleLocal<V> valswap {Abandon(oldValue)};

                  // Destroy the key, info and value                    
                  oldValue.FreeInner();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<THIS, false>(
                     newBucket, Abandon(oldKey), Abandon(valswap));
               }
               else {
                  auto oldValue = old.GetValHandle<THIS>(oldIndex);
                  Block<> valswap {mValues.GetState(), GetValueType<THIS>(), 1};
                  valswap.AllocateFresh(valswap.RequestSize(1));
                  valswap.CreateWithIntent(Abandon(oldValue));

                  // Destroy the pair and info at old index             
                  oldValue.FreeInner();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  InsertBlockInner<THIS, false>(
                     newBucket, Abandon(oldKey), Abandon(valswap));

                  valswap.Free();
               }
            }
         }

         ++oldKey;
         ++oldInfo;
      }

      // First run might cause gaps                                     
      // Second run: shift elements left, where possible                
      ShiftPairs<THIS>();
   }
   
   /// Shift elements left where possible                                     
   /// Repeat this couple of times until no more moves are possible           
   template<CT::Map THIS>
   void BlockMap::ShiftPairs() {
      int moves_performed;
      do {
         moves_performed = 0;
         auto oldInfo = mInfo;
         const auto newInfoEnd = GetInfoEnd();
         while (oldInfo != newInfoEnd) {
            if (*oldInfo > 1) {
               // Entry can be moved by *oldInfo - 1 cells to the left  
               const Offset oldIndex = oldInfo - GetInfo();

               // Will loop around if it goes beyond mKeys.mReserved    
               Offset to = mKeys.mReserved + oldIndex - *oldInfo + 1;
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
                  // Empty spot found, so move pair there               
                  auto key = GetKeyHandle<THIS>(oldIndex);
                  GetKeyHandle<THIS>(to).CreateWithIntent(Abandon(key));
                  key.FreeInner();

                  auto val = GetValHandle<THIS>(oldIndex);
                  GetValHandle<THIS>(to).CreateWithIntent(Abandon(val));
                  val.FreeInner();

                  mInfo[to] = attempt;
                  *oldInfo = 0;
                  ++moves_performed;
               }
            }

            ++oldInfo;
         }
      } while (moves_performed);
   }
   
   /// Inner insertion function                                               
   ///   @attention assumes that keys and values are constructible with the   
   ///      provided arguments                                                
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param key - key to insert, with or without intent                   
   ///   @param val - value to insert, with or without intent                 
   ///   @return the offset at which pair was inserted                        
   template<CT::Map THIS, bool CHECK_FOR_MATCH>
   Offset BlockMap::InsertInner(const Offset start, auto&& key, auto&& val) {
      BranchOut<THIS>();
      using SK = IntentOf<decltype(key)>;
      using SV = IntentOf<decltype(val)>;
      auto keyswapper = CreateKeyHandle<THIS>(SK::Nest(key));
      auto valswapper = CreateValHandle<THIS>(SV::Nest(val));

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts = 1;
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            if (keyswapper.Compare(GetKeyRef<THIS>(index))) {
               // Neat, the key already exists - just set value and go  
               if constexpr (CT::Sparse<TypeOf<decltype(valswapper)>>)
                  GetValHandle<THIS>(index).AssignWithIntent(Refer(valswapper));
               else
                  GetValHandle<THIS>(index).AssignWithIntent(Abandon(valswapper));
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetKeyHandle<THIS>(index).Swap(keyswapper);
            GetValHandle<THIS>(index).Swap(valswapper);

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

      // If reached, empty slot reached, so put the pair there          
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      const auto index = psl - GetInfo();
      if constexpr (CT::Sparse<TypeOf<decltype(keyswapper)>>)
         GetKeyHandle<THIS>(index).CreateWithIntent(Refer(keyswapper));
      else
         GetKeyHandle<THIS>(index).CreateWithIntent(Abandon(keyswapper));

      if constexpr (CT::Sparse<TypeOf<decltype(valswapper)>>)
         GetValHandle<THIS>(index).CreateWithIntent(Refer(valswapper));
      else
         GetValHandle<THIS>(index).CreateWithIntent(Abandon(valswapper));

      //GetKeyHandle<THIS>(index).CreateWithIntent(Abandon(keyswapper));
      //GetValHandle<THIS>(index).CreateWithIntent(Abandon(valswapper));
      if (insertedAt == mKeys.mReserved)
         insertedAt = index;

      *psl = attempts;
      ++mKeys.mCount;
      return insertedAt;
   }
   
   /// Inner insertion function based on reflected move-assignment            
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param key - key to move in                                          
   ///   @param val - value to move in                                        
   ///   @return the offset at which pair was inserted                        
   template<CT::Map THIS, bool CHECK_FOR_MATCH, template<class> class S1, template<class> class S2, CT::Block T>
   requires CT::Intent<S1<T>, S2<T>>
   Offset BlockMap::InsertBlockInner(const Offset start, S1<T>&& key, S2<T>&& val) {
      BranchOut<THIS>();

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts = 1;
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetKeyHandle<THIS>(index);
            if (candidate == *key) {
               // Neat, the key already exists - just set value and go  
               GetValHandle<THIS>(index).AssignWithIntent(val.Forward());

               if constexpr (S2<T>::Move) {
                  val->FreeInner();
                  val->mCount = 0;
               }

               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetKeyHandle<THIS>(index).Swap(key.Forward());
            GetValHandle<THIS>(index).Swap(val.Forward());

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
      // eventually reached, unless key exists and returns early        
      // We're moving only a single element, so no chance of overlap    
      const auto index = psl - GetInfo();
      GetKeyHandle<THIS>(index).CreateWithIntent(key.Forward());
      GetValHandle<THIS>(index).CreateWithIntent(val.Forward());

      if (insertedAt == mKeys.mReserved)
         insertedAt = index;

      if constexpr (S1<T>::Move) {
         key->FreeInner();
         key->mCount = 0;
      }

      if constexpr (S2<T>::Move) {
         val->FreeInner();
         val->mCount = 0;
      }

      *psl = attempts;
      ++mKeys.mCount;
      return insertedAt;
   }
   
   /// Insert any pair into a preinitialized map                              
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param hashmask - precalculated hashmask                             
   ///   @param pair - the pair to insert, with or without intent             
   ///   @return the number of newly inserted pairs                           
   template<CT::Map THIS, bool CHECK_FOR_MATCH, template<class> class S, CT::Pair T>
   requires CT::Intent<S<T>>
   Count BlockMap::InsertPairInner(const Count hashmask, S<T>&& pair) {
      const auto initialCount = GetCount();
      if constexpr (CT::Typed<T>) {
         // Insert a statically typed pair                              
         InsertInner<THIS, CHECK_FOR_MATCH>(
            GetBucket(hashmask, pair->mKey),
            S<T>::Nest(pair->GetKeyHandle()),
            S<T>::Nest(pair->GetValueHandle())
         );
      }
      else {
         // Insert a type-erased pair                                   
         InsertBlockInner<THIS, CHECK_FOR_MATCH>(
            GetBucketUnknown(hashmask, pair->mKey),
            S<T>::Nest(pair->mKey),
            S<T>::Nest(pair->mValue)
         );
      }
      return GetCount() - initialCount;
   }

} // namespace Langulus::Anyness
