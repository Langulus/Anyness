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


namespace Langulus::Anyness
{
   
   /// Semantically transfer the members of one map onto another              
   ///   @tparam TO - the type of map we're transferring to                   
   ///   @param other - the map and semantic to transfer from                 
   template<CT::Map TO, template<class> class S, CT::Map FROM>
   requires CT::Semantic<S<FROM>> LANGULUS(INLINED)
   void BlockMap::BlockTransfer(S<FROM>&& other) {
      using SS = S<FROM>;

      if constexpr (not CT::TypedMap<TO>) {
         // TO is not statically typed, so we can safely                
         // overwrite type and state                                    
         mKeys.mType = other->GetKeyType();
         mValues.mType = other->GetValueType();
         mKeys.mState = other->mKeys.mState;
         mValues.mState = other->mValues.mState;
      }
      else {
         // TO is typed, so we never touch mType, and we make sure that 
         // we don't affect Typed state                                 
         mKeys.mType = MetaDataOf<typename TO::Key>();
         mValues.mType = MetaDataOf<typename TO::Value>();
         mKeys.mState = other->mKeys.mState + DataState::Typed;
         mValues.mState = other->mValues.mState + DataState::Typed;
      }

      if constexpr (SS::Shallow) {
         if constexpr (SS::Keep) {
            // Move/Refer/Copy other                                    
            if constexpr (SS::Move) {
               // Move                                                  
               mKeys.mEntry = other->mKeys.mEntry;
               mValues.mEntry = other->mValues.mEntry;
               mKeys.mCount = other->mKeys.mCount;
               mKeys.mRaw = other->mKeys.mRaw;
               mKeys.mReserved = other->mKeys.mReserved;
               mValues.mRaw = other->mValues.mRaw;
               mInfo = other->mInfo;

               if constexpr (not FROM::Ownership) {
                  // Since we are not aware if that block is referenced 
                  // or not we reference it just in case, and we also   
                  // do not reset 'other' to avoid leaks When using raw 
                  // BlockMaps, it's your responsibility to take care   
                  // of ownership.                                      
                  Keep();
               }
               else {
                  other->mKeys.ResetMemory();
                  other->mKeys.ResetState();
                  other->mValues.ResetMemory();
                  other->mValues.ResetState();
               }
            }
            else if constexpr (CT::Referred<SS>) {
               // Refer                                                 
               mKeys.mEntry = other->mKeys.mEntry;
               mValues.mEntry = other->mValues.mEntry;
               mKeys.mCount = other->mKeys.mCount;
               mKeys.mRaw = other->mKeys.mRaw;
               mKeys.mReserved = other->mKeys.mReserved;
               mValues.mRaw = other->mValues.mRaw;
               mInfo = other->mInfo;

               Keep();
            }
            else {
               // Copy                                                  
               // We're shallow-copying, so we're 100% sure, that       
               // each pair will end up in the same place               
               mKeys.mState -= DataState::Static | DataState::Constant;
               mValues.mState -= DataState::Static | DataState::Constant;
               if (other->IsEmpty())
                  return;

               // Always prefer statically typed map interface (if any) 
               using B = Conditional<CT::Typed<FROM>, FROM, TO>;
               using K = Conditional<CT::Typed<B>, typename B::Key,   void>;
               using V = Conditional<CT::Typed<B>, typename B::Value, void>;
               auto asFrom = const_cast<B*>(reinterpret_cast<const B*>(&*other));

               if constexpr (CT::Untyped<B>) {
                  // Runtime checks are required before allocating      
                  LANGULUS_ASSERT(asFrom->mKeys.mType->mReferConstructor, Construct,
                     "Can't refer-construct keys"
                     " - no refer-constructor was reflected for type ", asFrom->mKeys.mType);
                  LANGULUS_ASSERT(asFrom->mValues.mType->mReferConstructor, Construct,
                     "Can't refer-construct values"
                     " - no refer-constructor was reflected for type ", asFrom->mValues.mType);
               }
               else {
                  static_assert(CT::ReferMakable<K>,
                     "Key type is not refer-constructible");
                  static_assert(CT::ReferMakable<V>,
                     "Value type is not refer-constructible");
               }

               AllocateFresh<B>(other->GetReserved());
               CopyMemory(mInfo, other->mInfo, GetReserved() + 1);

               if constexpr (CT::Typed<B>) {
                  // At least one of the maps is typed                  
                  if constexpr (CT::POD<K>) {
                     // Data is POD, we can directly copy all keys      
                     CopyMemory(
                        mKeys.mRaw, asFrom->mKeys.mRaw,
                        GetReserved() * sizeof(K)
                     );
                  }
                  else {
                     // Data isn't pod, refer valid keys one by one     
                     auto info = GetInfo();
                     const auto infoEnd = GetInfoEnd();
                     auto dstKey = GetKeyHandle<B>(0);
                     auto srcKey = asFrom->template GetKeyHandle<B>(0);
                     while (info != infoEnd) {
                        if (*info)
                           dstKey.CreateSemantic(Refer(srcKey));

                        ++info;
                        ++dstKey;
                        ++srcKey;
                     }
                  }
               }
               else {
                  // Both maps are type-erased                          
                  if (asFrom->mKeys.mType->mIsPOD) {
                     // Keys are POD, we can directly copy them all     
                     CopyMemory(
                        mKeys.mRaw, asFrom->mKeys.mRaw,
                        GetReserved() * asFrom->mKeys.mType->mSize
                     );
                  }
                  else {
                     // Keys aren't POD, clone valid keys one by one    
                     auto info = GetInfo();
                     const auto infoEnd = GetInfoEnd();
                     auto dstKey = GetKeyHandle<B>(0);
                     auto srcKey = asFrom->template GetKeyHandle<B>(0);
                     while (info != infoEnd) {
                        if (*info)
                           dstKey.CreateSemantic(Refer(srcKey));

                        ++info;
                        ++dstKey;
                        ++srcKey;
                     }
                  }
               }

               CloneValuesInner(Refer(*asFrom));

               // This validates elements, do it last in case           
               // something throws along the way                        
               mKeys.mCount = other->GetCount();
            }
         }
         else if constexpr (SS::Move) {
            // Abandon                                                  
            mKeys.mEntry = other->mKeys.mEntry;
            mValues.mEntry = other->mValues.mEntry;
            mKeys.mCount = other->mKeys.mCount;
            mKeys.mRaw = other->mKeys.mRaw;
            mKeys.mReserved = other->mKeys.mReserved;
            mValues.mRaw = other->mValues.mRaw;
            mInfo = other->mInfo;

            other->mKeys.mEntry = nullptr;
         }
         else {
            // Disown                                                   
            mKeys.mCount = other->mKeys.mCount;
            mKeys.mRaw = other->mKeys.mRaw;
            mKeys.mReserved = other->mKeys.mReserved;
            mValues.mRaw = other->mValues.mRaw;
            mInfo = other->mInfo;
         }
      }
      else {
         // We're cloning, so we guarantee, that data is no longer      
         // static                                                      
         mKeys.mState -= DataState::Static | DataState::Constant;
         mValues.mState -= DataState::Static | DataState::Constant;
         if (other->IsEmpty())
            return;

         // Always prefer statically typed map interface (if any)       
         using B = Conditional<CT::Typed<FROM>, FROM, TO>;
         using K = Conditional<CT::Typed<B>, typename B::Key, void>;
         using V = Conditional<CT::Typed<B>, typename B::Value, void>;
         auto asFrom = const_cast<B*>(reinterpret_cast<const B*>(&*other));

         if constexpr (CT::Untyped<B>) {
            // Runtime checks are required before allocating      
            LANGULUS_ASSERT(asFrom->mKeys.mType->mCloneConstructor, Construct,
               "Can't clone-construct keys"
               " - no clone-constructor was reflected for type ", asFrom->mKeys.mType);
            LANGULUS_ASSERT(asFrom->mValues.mType->mCloneConstructor, Construct,
               "Can't clone-construct values"
               " - no clone-constructor was reflected for type ", asFrom->mValues.mType);
         }
         else {
            static_assert(CT::CloneMakable<K>,
               "Key type is not clone-constructible");
            static_assert(CT::CloneMakable<V>,
               "Value type is not clone-constructible");
         }

         AllocateFresh<B>(other->GetReserved());

         if constexpr (CT::Typed<B>) {
            // At least one of the maps is typed                        
            if constexpr (CT::Dense<K>) {
               // We're cloning dense keys, so we're 100% sure, that    
               // each pair will end up in the same place               
               CopyMemory(mInfo, other->mInfo, GetReserved() + 1);

               if constexpr (CT::POD<K>) {
                  // Data is POD, we can directly copy all keys         
                  CopyMemory(
                     mKeys.mRaw, asFrom->mKeys.mRaw,
                     GetReserved() * sizeof(K)
                  );
               }
               else {
                  // Data isn't pod, clone valid keys one by one        
                  auto info = GetInfo();
                  const auto infoEnd = GetInfoEnd();
                  auto dstKey = GetKeyHandle<B>(0);
                  auto srcKey = asFrom->template GetKeyHandle<B>(0);
                  while (info != infoEnd) {
                     if (*info)
                        dstKey.CreateSemantic(SS::Nest(srcKey));

                     ++info;
                     ++dstKey;
                     ++srcKey;
                  }
               }

               CloneValuesInner(SS::Nest(*asFrom));

               // This validates elements, do it last in case           
               // something throws along the way                        
               mKeys.mCount = other->GetCount();
            }
            else {
               // We're cloning pointers, which will inevitably end up  
               // pointing elsewhere, which means that all pairs must   
               // be rehashed and reinserted                            
               using CK = TAny<Deptr<K>>;
               CK coalescedKeys;
               coalescedKeys.Reserve(asFrom->GetCount());

               // Coalesce all densified elements, to avoid multiple    
               // allocations                                           
               for (auto& item : *asFrom) {
                  coalescedKeys.template InsertInner<CK, void, false>(
                     IndexBack, SS::Nest(*item.mKey));
               }

               const_cast<Allocation*>(coalescedKeys.mEntry)
                  ->Keep(asFrom->GetCount());

               // Zero info bytes and insert pointers                   
               ZeroMemory(mInfo, mKeys.mReserved);
               mInfo[mKeys.mReserved] = 1;

               CloneValuesReinsertInner(coalescedKeys, SS::Nest(*asFrom));
            }
         }
         else {
            // Both maps are type-erased                                
            if (not asFrom->mKeys.mType->mIsSparse) {
               // We're cloning dense elements, so we're 100% sure, that
               // each element will end up in the same place            
               CopyMemory(mInfo, other->mInfo, GetReserved() + 1);

               if (asFrom->mKeys.mType->mIsPOD) {
                  // Keys are POD, we can directly copy them all        
                  CopyMemory(
                     mKeys.mRaw, asFrom->mKeys.mRaw,
                     GetReserved() * asFrom->mKeys.mType->mSize
                  );
               }
               else {
                  // Keys aren't POD, clone valid keys one by one       
                  auto info = GetInfo();
                  const auto infoEnd = GetInfoEnd();
                  auto dstKey = GetKeyHandle<B>(0);
                  auto srcKey = asFrom->template GetKeyHandle<B>(0);
                  while (info != infoEnd) {
                     if (*info)
                        dstKey.CreateSemantic(SS::Nest(srcKey));

                     ++info;
                     ++dstKey;
                     ++srcKey;
                  }
               }
               
               CloneValuesInner(SS::Nest(*asFrom));

               // This validates elements, do it last in case           
               // something throws along the way                        
               mKeys.mCount = other->GetCount();
            }
            else {
               // We're cloning pointers, which will inevitably end up  
               // pointing elsewhere, which means that all elements must
               // be rehashed and reinserted                            
               auto coalescedKeys = Any::FromMeta(asFrom->mKeys.mType->mDeptr);
               coalescedKeys.Reserve(asFrom->GetCount());

               // Coalesce all densified elements, to avoid multiple    
               // allocations                                           
               for (auto item : *asFrom) {
                  coalescedKeys.template InsertBlockInner<Any, void, false>(
                     IndexBack, SS::Nest(*item.mKey));
               }

               const_cast<Allocation*>(coalescedKeys.mEntry)
                  ->Keep(asFrom->GetCount());

               // Zero info bytes and insert pointers                   
               ZeroMemory(mInfo, mKeys.mReserved);
               mInfo[mKeys.mReserved] = 1;

               CloneValuesReinsertInner(coalescedKeys, SS::Nest(*asFrom));
            }
         }
      }
   }

   /// Clone a key/value block                                                
   ///   @attention assumes key type is dense, and values go into the same    
   ///      places                                                            
   ///   @attention assumes key and value types are clone-constructible       
   template<template<class> class S, CT::Map B> requires CT::Semantic<S<B>>
   void BlockMap::CloneValuesInner(S<B>&& asFrom) {
      using SS = S<B>;

      if constexpr (CT::Typed<B>) {
         // At least one of the maps is dense                           
         using V = typename B::Value;

         // We're cloning dense keys or shallow-copying any keys, so    
         // we're 100% sure, that each pair will end up in the same spot
         if constexpr (CT::Dense<V> or CT::ShallowSemantic<SS>) {
            if constexpr (CT::POD<V>) {
               // Data is POD, we can directly copy all values          
               CopyMemory(
                  mValues.mRaw, asFrom->mValues.mRaw,
                  GetReserved() * sizeof(V)
               );
            }
            else {
               // Data isn't pod, clone valid values one by one         
               auto info = GetInfo();
               const auto infoEnd = GetInfoEnd();
               auto dstKey = GetValHandle<B>(0);
               auto srcKey = asFrom->template GetValHandle<B>(0);
               while (info != infoEnd) {
                  if (*info)
                     dstKey.CreateSemantic(SS::Nest(srcKey));

                  ++info;
                  ++dstKey;
                  ++srcKey;
               }
            }
         }
         else {
            // Values are sparse, too - treat them the same             
            using CV = TAny<Deptr<V>>;
            CV coalescedValues;
            coalescedValues.Reserve(asFrom->GetCount());
            for (auto item : *asFrom) {
               coalescedValues.template InsertInner<CV, void, false>(
                  IndexBack, SS::Nest(*item.mValue));
            }

            // We're using Handle::Create, instead of CreateSemantic    
            // so we have to reference here                             
            const_cast<Allocation*>(coalescedValues.mEntry)
               ->Keep(asFrom->GetCount());

            auto ptrVal = coalescedValues.GetRaw();
            auto info = GetInfo();
            const auto infoEnd = GetInfoEnd();
            auto dstKey = GetValHandle<B>(0);
            while (info != infoEnd) {
               if (*info)
                  dstKey.Create(ptrVal++, coalescedValues.mEntry);

               ++info;
               ++dstKey;
            }
         }
      }
      else {
         // Both maps are type-erased                                   
         // We're cloning dense elements or shallow-copying any, so     
         // we're 100% sure, that each element will end up in the same  
         // spot                                                        
         if (not asFrom->mValues.mType->mIsSparse or CT::ShallowSemantic<SS>) {
            if (asFrom->mValues.mType->mIsPOD) {
               // Values are POD, we can directly copy them all         
               CopyMemory(
                  mValues.mRaw, asFrom->mValues.mRaw,
                  GetReserved() * asFrom->mValues.mType->mSize
               );
            }
            else {
               // Values aren't POD, clone valid keys one by one        
               auto info = GetInfo();
               const auto infoEnd = GetInfoEnd();
               auto dstKey = GetValHandle<B>(0);
               auto srcKey = asFrom->template GetValHandle<B>(0);
               while (info != infoEnd) {
                  if (*info)
                     dstKey.CreateSemantic(SS::Nest(srcKey));

                  ++info;
                  ++dstKey;
                  ++srcKey;
               }
            }
         }
         else {
            // Values are sparse, too - treat them the same             
            auto coalescedValues = Any::FromMeta(asFrom->mValues.mType->mDeptr);
            coalescedValues.Reserve(asFrom->GetCount());
            for (auto item : *asFrom) {
               coalescedValues.template InsertBlockInner<Any, void, false>(
                  IndexBack, SS::Nest(*item.mValue));
            }

            const_cast<Allocation*>(coalescedValues.mEntry)
               ->Keep(asFrom->GetCount());

            auto ptrVal = coalescedValues.mRaw;
            const Size valstride = coalescedValues.GetStride();
            auto info = GetInfo();
            const auto infoEnd = GetInfoEnd();
            while (info != infoEnd) {
               if (*info) {
                  GetValHandle<B>(info - GetInfo()).CreateSemantic(
                     Abandon(HandleLocal<void*> {ptrVal, coalescedValues.mEntry})
                  );
                  ptrVal += valstride.mSize;
               }

               ++info;
            }
         }
      }
   }
   
   /// Clone a key/value block                                                
   ///   @attention assumes keys are sparse and all pairs will end up in      
   ///      different places                                                  
   ///   @attention assumes key and value types are clone-constructible       
   template<template<class> class S, CT::Map B> requires CT::Semantic<S<B>>
   void BlockMap::CloneValuesReinsertInner(CT::Block auto& coalescedKeys, S<B>&& asFrom) {
      using SS = S<B>;

      if constexpr (CT::Typed<B>) {
         // At least one of the maps is dense                           
         // We're cloning pointers, which will inevitably end up        
         // pointing elsewhere, which means that all pairs must         
         // be rehashed and reinserted                                  
         using K = typename B::Key;
         using V = typename B::Value;

         auto ptr = coalescedKeys.GetRaw();
         const auto ptrEnd = coalescedKeys.GetRawEnd();

         if constexpr (CT::Dense<V>) {
            // Values are dense, however                                
            int valIdx = 0;
            while (not asFrom->mInfo[valIdx])
               ++valIdx;

            while (ptr != ptrEnd) {
               InsertInner<B, false>(
                  GetBucket(GetReserved() - 1, ptr),
                  Abandon(HandleLocal<K> {ptr, coalescedKeys.mEntry}),
                  SS::Nest(asFrom->template GetValHandle<B>(valIdx))
               );

               ++ptr;
               ++valIdx;
               while (not asFrom->mInfo[valIdx])
                  ++valIdx;
            }
         }
         else {
            // Values are sparse, too - treat them the same             
            using CV = TAny<Deptr<V>>;
            CV coalescedValues;
            coalescedValues.Reserve(asFrom->GetCount());
            for (auto& item : *asFrom) {
               coalescedValues.template InsertInner<CV, void, false>(
                  IndexBack, SS::Nest(*item.mValue));
            }

            const_cast<Allocation*>(coalescedValues.mEntry)
               ->Keep(asFrom->GetCount());

            auto ptrVal = coalescedValues.GetRaw();
            while (ptr != ptrEnd) {
               InsertInner<B, false>(
                  GetBucket(GetReserved() - 1, ptr),
                  Abandon(HandleLocal<K> {ptr,    coalescedKeys.mEntry}),
                  Abandon(HandleLocal<V> {ptrVal, coalescedValues.mEntry})
               );

               ++ptr;
               ++ptrVal;
            }
         }
      }
      else {
         // Both maps are type-erased                                   
         // We're cloning pointers, which will inevitably end up        
         // pointing elsewhere, which means that all elements must      
         // be rehashed and reinserted                                  
         auto ptr = coalescedKeys.mRaw;
         const auto ptrEnd = coalescedKeys.mRaw + coalescedKeys.GetBytesize();
         const Size stride = coalescedKeys.GetStride();

         if (not asFrom->mValues.mType->mIsSparse) {
            // Values are dense, however                                
            int valIdx = 0;
            while (not asFrom->mInfo[valIdx])
               ++valIdx;

            while (ptr != ptrEnd) {
               InsertInner<B, false>(
                  GetBucket(GetReserved() - 1, ptr),
                  Abandon(HandleLocal<void*> {ptr, coalescedKeys.mEntry}),
                  SS::Nest(asFrom->template GetValHandle<B>(valIdx))
               );

               ++ptr;
               ++valIdx;
               while (not asFrom->mInfo[valIdx])
                  ++valIdx;
            }
         }
         else {
            // Values are sparse, too - treat them the same             
            auto coalescedValues = Any::FromMeta(asFrom->mValues.mType->mDeptr);
            coalescedValues.Reserve(asFrom->GetCount());
            for (auto item : *asFrom) {
               coalescedValues.template InsertBlockInner<Any, void, false>(
                  IndexBack, SS::Nest(*item.mValue));
            }

            const_cast<Allocation*>(coalescedValues.mEntry)
               ->Keep(asFrom->GetCount());

            auto ptrVal = coalescedValues.mRaw;
            const Size valstride = coalescedValues.GetStride();
            while (ptr != ptrEnd) {
               InsertInner<B, false>(
                  GetBucket(GetReserved() - 1, ptr),
                  Abandon(HandleLocal<void*> {ptr, coalescedKeys.mEntry}),
                  Abandon(HandleLocal<void*> {ptrVal, coalescedValues.mEntry})
               );

               ptr += stride.mSize;
               ptrVal += valstride.mSize;
            }
         }
      }
   }

} // namespace Langulus::Anyness
