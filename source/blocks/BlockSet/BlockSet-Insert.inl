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
#include "../../text/Text.hpp"


namespace Langulus::Anyness
{

   /// Wrap the argument semantically into a handle with value's type         
   ///   @param val - the val to wrap                                         
   ///   @return the handle object                                            
   template<CT::Set THIS>
   auto BlockSet::CreateValHandle(auto&& val) {
      using S = SemanticOf<decltype(val)>;
      using T = TypeOf<S>;

      if constexpr (CT::Typed<THIS>) {
         using V = Conditional<CT::Typed<THIS>, TypeOf<THIS>, TypeOf<T>>;
         return HandleLocal<V> {S::Nest(val)};
      }
      else return Any::Wrap(S::Nest(val));
   }

   /// Insert an element, or an array of elements                             
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   template<CT::Set THIS>
   Count BlockSet::UnfoldInsert(auto&& item) {
      using E = Conditional<CT::Typed<THIS>, TypeOf<THIS>, void>;
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;

      if constexpr (not CT::TypeErased<E>)
         mKeys.mType = MetaDataOf<E>();

      if constexpr (CT::Array<T>) {
         if constexpr (not CT::TypeErased<E>) {
            if constexpr (CT::MakableFrom<E, Deext<T>>) {
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
            else if constexpr (CT::MakableFrom<E, Unfold<Deext<T>>>) {
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
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, text),
               Abandon(text)
            );
            return 1;
         }
         else {
            // Insert the array                                         
            Mutate<THIS, Deext<T>>();
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
         if constexpr (CT::Handle<T> and CT::Similar<E, TypeOf<T>>) {
            // Insert a handle                                          
            Reserve(GetCount() + 1);
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, DesemCast(item).Get()),
               S::Nest(item)
            );
            return 1;
         }
         else if constexpr (CT::MakableFrom<E, T>) {
            // Some of the arguments might still be used directly to    
            // make an element, forward these to standard insertion here
            mKeys.mType = MetaDataOf<E>();
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

               if constexpr (CT::MakableFrom<E, T2>) {
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
               else if constexpr (CT::MakableFrom<E, Unfold<T2>>) {
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
                  InsertBlockInner<THIS, true>(
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
         if constexpr (CT::Handle<T>) {
            // Insert a handle                                          
            Mutate<THIS, Decvq<TypeOf<T>>>();
            Reserve(GetCount() + 1);
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, DesemCast(item).Get()),
               S::Nest(item)
            );
         }
         else {
            // Some of the arguments might still be used directly to    
            // make an element, forward these to standard insertion here
            Mutate<THIS, Decvq<T>>();
            Reserve(GetCount() + 1);
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, DesemCast(item)),
               S::Nest(item)
            );
         }

         return 1;
      }
   }

   /// Insert elements, semantically or not                                   
   ///   @param t1 - the first item to insert                                 
   ///   @param tn... - the rest of items to insert (optional)                
   ///   @return number of inserted elements                                  
   template<CT::Set THIS, class T1, class...TN> LANGULUS(INLINED)
   Count BlockSet::Insert(T1&& t1, TN&&...tn) {
      Count inserted = 0;
        inserted += UnfoldInsert<THIS>(Forward<T1>(t1));
      ((inserted += UnfoldInsert<THIS>(Forward<TN>(tn))), ...);
      return inserted;
   }
   
   /// Insert all elements of a set, semantically or not                      
   ///   @param item - the set to insert                                      
   ///   @return number of inserted elements                                  
   template<CT::Set THIS, class T> requires CT::Set<Desem<T>> LANGULUS(INLINED)
   Count BlockSet::InsertBlock(T&& item) {
      using S = SemanticOf<decltype(item)>;
      using ST = TypeOf<S>;
      const auto count = DesemCast(item).GetCount();
      if (not count)
         return 0;

      Reserve(GetCount() + count);

      if (IsEmpty()) {
         // This set was empty, so no chance of collision, and since    
         // we're inserting a set, we can save a lot of CPU by not      
         // checking for matches                                        
         if constexpr (CT::Typed<ST> or CT::Typed<THIS>) {
            // Merging with a statically typed sets                     
            using B = Conditional<CT::Typed<ST>, ST, THIS>;
            for (auto& it : reinterpret_cast<const B&>(DesemCast(item))) {
               InsertInner<THIS, false>(
                  GetBucket(GetReserved() - 1, it),
                  S::Nest(it)
               );
            }
         }
         else {
            // Merging type-erased sets                                 
            for (auto& it : DesemCast(item)) {
               InsertBlockInner<THIS, false>(
                  GetBucketUnknown(GetReserved() - 1, it),
                  S::Nest(it)
               );
            }
         }
      }
      else {
         // This set already contains some stuff, so be careful when    
         // inserting elements from the other set - they might repeat   
         if constexpr (CT::Typed<ST> or CT::Typed<THIS>) {
            // Merging with a statically typed sets                     
            using B = Conditional<CT::Typed<ST>, ST, THIS>;
            for (auto& it : reinterpret_cast<const B&>(DesemCast(item))) {
               InsertInner<THIS, true>(
                  GetBucket(GetReserved() - 1, it),
                  S::Nest(it)
               );
            }
         }
         else {
            // Merging type-erased sets                                 
            for (auto& it : DesemCast(item)) {
               InsertBlockInner<THIS, true>(
                  GetBucketUnknown(GetReserved() - 1, it),
                  S::Nest(it)
               );
            }
         }
      }

      return count;
   }
   
   /// Insert all elements of a block, semantically or not                    
   ///   @param item - the set to insert                                      
   ///   @return number of inserted elements                                  
   template<CT::Set THIS, class T> requires CT::Block<Desem<T>> LANGULUS(INLINED)
   Count BlockSet::InsertBlock(T&& item) {
      using S = SemanticOf<decltype(item)>;
      using ST = TypeOf<S>;
      const auto count = DesemCast(item).GetCount();
      if (not count)
         return 0;

      Reserve(GetCount() + count);

      if constexpr (CT::Typed<ST> or CT::Typed<THIS>) {
         // Merging with a statically typed set and/or block            
         using B = Conditional<CT::Typed<ST>, ST, typename THIS::BlockType>;
         for (auto& it : reinterpret_cast<const B&>(DesemCast(item))) {
            InsertInner<THIS, true>(
               GetBucket(GetReserved() - 1, it),
               S::Nest(it)
            );
         }
      }
      else {
         // Merging type-erased block with a type-erased set            
         for (auto& it : DesemCast(item)) {
            InsertBlockInner<THIS, true>(
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
      Offset keymemory;
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
                  Block keyswap {DataState {}, GetType(), 1};
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

         ++oldKey;
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
               tokey.CreateSemantic(Abandon(key));
               key.Destroy();
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
   template<CT::Set THIS, bool CHECK_FOR_MATCH>
   Offset BlockSet::InsertInner(const Offset start, auto&& key) {
      using S = SemanticOf<decltype(key)>;

      if (GetUses() > 1) {
         // Set is used from multiple locations, and we must branch out 
         // before changing it - only this copy will be affected        
         const BlockSet backup = *this;
         const_cast<Allocation*>(mKeys.mEntry)->Free();
         new (this) THIS {Copy(reinterpret_cast<const THIS&>(backup))};
      }

      auto keyswapper = CreateValHandle<THIS>(S::Nest(key));

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            if (keyswapper == GetRef<THIS>(index)) {
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
      GetHandle<THIS>(index).CreateSemantic(Abandon(keyswapper));
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
      if (GetUses() > 1) {
         // Set is used from multiple locations, and we mush branch out 
         // before changing it                                          
         TODO();
      }

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      Offset insertedAt = mKeys.mReserved;
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            if (GetHandle<THIS>(index) == *key) {
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
         key->Destroy();
         key->mCount = 0;
      }

      *psl = attempts;
      ++mKeys.mCount;
      return insertedAt;
   }

} // namespace Langulus::Anyness
