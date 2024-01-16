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

   /// Insert an element, or an array of elements                             
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   template<CT::Set THIS>
   Count BlockSet::UnfoldInsert(auto&& item) {
      using E = Conditional<CT::Typed<THIS>, TypeOf<THIS>, void>;
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;

      if constexpr (CT::Array<T>) {
         if constexpr (not CT::TypeErased<E>) {
            if constexpr (CT::Inner::MakableFrom<E, Deext<T>>) {
               // Construct from an array of elements, each of which    
               // can be used to initialize an element, nesting any     
               // semantic while doing it                               
               Reserve(GetCount() + ExtentOf<T>);
               for (auto& key : DesemCast(item)) {
                  InsertInner<THIS, true>(
                     GetBucket(GetReserved() - 1, DesemCast(key)),
                     S::Nest(key)
                  );
               }
               return ExtentOf<T>;
            }
            else if constexpr (CT::Inner::MakableFrom<E, Unfold<Deext<T>>>) {
               // Construct from an array of things, which can't be used
               // to directly construct elements, so nest this insert   
               Count inserted = 0;
               for (auto& key : DesemCast(item))
                  inserted += UnfoldInsert<THIS>(S::Nest(key));
               return inserted;
            }
            else LANGULUS_ERROR("Array elements aren't insertable");
         }
         else if constexpr (CT::StringLiteral<T>) {
            // Implicitly convert string literals to Text containers    
            Reserve(GetCount() + 1);
            Text text {S::Nest(item)};
            return InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, text),
               Abandon(text)
            );
         }
         else {
            // Insert the array                                         
            Reserve(GetCount() + ExtentOf<T>);
            Count inserted = 0;
            for (auto& e : DesemCast(item)) {
               inserted += InsertInner<THIS, true>(
                  GetBucket(GetReserved() - 1, e),
                  S::Nest(e)
               );
            }
            return inserted;
         }
      }
      else if constexpr (not CT::TypeErased<E>) {
         if constexpr (CT::Inner::MakableFrom<E, T>) {
            // Some of the arguments might still be used directly to    
            // make an element, forward these to standard insertion here
            Reserve(GetCount() + 1);
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, DesemCast(item)),
               S::Nest(item)
            );
            return 1;
         }
         else if constexpr (CT::Set<T>) {
            // Construct from any kind of set                           
            if constexpr (CT::Typed<T>) {
               // The contained type is known at compile-time           
               using T2 = TypeOf<T>;

               if constexpr (CT::Inner::MakableFrom<E, T2>) {
                  // Elements are mappable                              
                  Reserve(GetCount() + DesemCast(item).GetCount());
                  for (auto& key : DesemCast(item)) {
                     InsertInner<THIS, true>(
                        GetBucket(GetReserved() - 1, key),
                        S::Nest(key)
                     );
                  }
                  return DesemCast(item).GetCount();
               }
               else if constexpr (CT::Inner::MakableFrom<E, Unfold<T2>>) {
                  // Set elements need to be unfolded one by one        
                  Count inserted = 0;
                  for (auto& key : DesemCast(item))
                     inserted += UnfoldInsert<THIS>(S::Nest(key));
                  return inserted;
               }
               else LANGULUS_ERROR("Sets aren't mappable to each other");
            }
            else {
               // The rhs set is type-erased                            
               LANGULUS_ASSERT(DesemCast(item).template IsSimilar<E>(), Meta,
                  "Type mismatch");

               Reserve(GetCount() + DesemCast(item).GetCount());
               for (auto& key : DesemCast(item)) {
                  InsertInnerUnknown<THIS, true>(
                     GetBucketUnknown(GetReserved() - 1, key),
                     S::Nest(key)
                  );
               }
               return DesemCast(item).GetCount();
            }
         }
         else LANGULUS_ERROR("Can't insert argument");
      }
      else {
         // This set is type-erased                                     
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         Reserve(GetCount() + 1);
         return InsertInner<THIS, true>(
            GetBucket(GetReserved() - 1, DesemCast(item)),
            S::Nest(item)
         );
      }
   }

   /// Insert elements, semantically or not                                   
   ///   @param t1 - the first item to insert                                 
   ///   @param tail... - the rest of items to insert (optional)              
   ///   @return number of inserted elements                                  
   template<CT::Set THIS, class T1, class...TAIL> LANGULUS(INLINED)
   Count BlockSet::Insert(T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
        inserted += UnfoldInsert<THIS>(Forward<T1>(t1));
      ((inserted += UnfoldInsert<THIS>(Forward<TAIL>(tail))), ...);
      return inserted;
   }
   
   /// Insert all elements of a set, semantically or not                      
   ///   @param item - the set to insert                                      
   ///   @return number of inserted elements                                  
   template<CT::Set THIS, class T>
   requires CT::Set<Desem<T>> LANGULUS(INLINED)
   Count BlockSet::InsertBlock(T&& item) {
      using S = SemanticOf<decltype(item)>;
      using ST = TypeOf<S>;

      const auto count = DesemCast(item).GetCount();
      Reserve(GetCount() + count);

      if constexpr (CT::Typed<ST> or CT::Typed<THIS>) {
         // Merging with a statically typed set                         
         using SET = Conditional<CT::Typed<ST>, ST, THIS>;
         for (auto& it : reinterpret_cast<const SET&>(DesemCast(item))) {
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, it),
               S::Nest(it)
            );
         }
      }
      else {
         // Merging with a type-erased set                              
         for (Block it : static_cast<const BlockSet&>(DesemCast(item))) {
            InsertInnerUnknown<THIS, true>(
               GetBucketUnknown(GetReserved() - 1, it),
               S::Nest(it)
            );
         }
      }

      return count;
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
   template<CT::Set THIS> LANGULUS(INLINED)
   Size BlockSet::RequestKeyAndInfoSize(
      const Count request, Offset& infoStart
   ) const IF_UNSAFE(noexcept) {
      Size keymemory;
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         keymemory = request * sizeof(T);
         if constexpr (CT::Sparse<T>)
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

   /// Rehashes and reinserts each pair in the same block                     
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param oldCount - the old number of pairs                            
   template<CT::Set THIS>
   void BlockSet::Rehash(const Count oldCount) {
      LANGULUS_ASSUME(DevAssumes, mKeys.mReserved > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(mKeys.mReserved),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      UNUSED() auto& me = reinterpret_cast<const THIS&>(*this);
      auto oldKey = GetHandle<THIS>(0);
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
            if constexpr (CT::TypedSet<THIS>)
               newBucket += GetBucket(hashmask, oldKey.Get());
            else
               newBucket += GetBucketUnknown(hashmask, oldKey);

            if (oldBucket < oldCount or oldBucket - oldCount != newBucket) {
               // Move it only if it won't end up in same bucket        
               if constexpr (CT::Typed<THIS>) {
                  using K = TypeOf<THIS>;
                  HandleLocal<K> keyswap {Abandon(oldKey)};

                  // Destroy the key, info and value                    
                  oldKey.Destroy();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  // Reinsert at the new bucket                         
                  InsertInner<THIS, false>(newBucket, Abandon(keyswap));
               }
               else {
                  Block keyswap {GetState(), GetType(), 1, nullptr, nullptr};
                  keyswap.AllocateFresh<Any>(keyswap.RequestSize<Any>(1));
                  keyswap.CreateSemantic(Abandon(oldKey));

                  // Destroy the pair and info at old index             
                  oldKey.Destroy();
                  *oldInfo = 0;
                  --mKeys.mCount;

                  InsertBlockInner<THIS, false>(newBucket, Abandon(keyswap));
                  keyswap.Free();
               }
            }
         }

         if constexpr (CT::TypedSet<THIS>)
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
   template<CT::Set THIS>
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
               auto key   = GetHandle<THIS>(oldIndex);
               auto tokey = GetHandle<THIS>(to);

               if constexpr (CT::Typed<THIS>) {
                  tokey.New(Abandon(key));
                  key.Destroy();
               }
               else {
                  tokey.CallSemanticConstructors(1, Abandon(key));
                  key.CallDestructors();
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
   ///   @return the offset at which pair was inserted                        
   template<CT::Set THIS, bool CHECK_FOR_MATCH, template<class> class S, CT::Data T>
   requires CT::Semantic<S<T>>
   Offset BlockSet::InsertInner(const Offset start, S<T>&& key) {
      using K = Conditional<CT::Handle<T>, TypeOf<T>, T>;
      HandleLocal<K> keyswapper {key.Forward()};

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            const auto& candidate = GetRaw<THIS>(index);
            if (keyswapper.Compare(candidate)) {
               // Neat, the value already exists - just return          
               return index;
            }
         }

         if (attempts > *psl) {
            // The value we're inserting is closer to bucket, so swap   
            GetHandle<THIS>(index).Swap(keyswapper);
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
      GetHandle<THIS>(index).New(Abandon(keyswapper));
      if (insertedAt == mKeys.mReserved)
         insertedAt = index;

      *psl = attempts;
      ++mKeys.mCount;
      return insertedAt;
   }
   
   /// Inner insertion function from type-erased block                        
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param key - key & semantic to insert                                
   ///   @return the offset at which pair was inserted                        
   template<CT::Set THIS, bool CHECK_FOR_MATCH, template<class> class S>
   requires CT::Semantic<S<Block>>
   Offset BlockSet::InsertBlockInner(const Offset start, S<Block>&& key) {
      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetHandle<THIS>(index);
            if (candidate == *key) {
               // Neat, the key already exists - just return            
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetHandle<THIS>(index).Swap(key.Forward());
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
      GetHandle<THIS>(index).CreateSemantic(key.Forward());
      if (insertedAt == mKeys.mReserved)
         insertedAt = index;

      if constexpr (S<Block>::Move) {
         key->CallDestructors();
         key->mCount = 0;
      }

      *psl = attempts;
      ++mKeys.mCount;
      return insertedAt;
   }

} // namespace Langulus::Anyness
