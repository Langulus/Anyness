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
         mKeys.mCount = other->mKeys.mCount;
         mKeys.mRaw = other->mKeys.mRaw;
         mKeys.mReserved = other->mKeys.mReserved;
         mValues.mRaw = other->mValues.mRaw;
         mInfo = other->mInfo;

         if constexpr (SS::Keep) {
            // Move/Copy other                                          
            mKeys.mEntry = other->mKeys.mEntry;
            mValues.mEntry = other->mValues.mEntry;

            if constexpr (SS::Move) {
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
            else Keep();
         }
         else if constexpr (SS::Move) {
            // Abandon other                                            
            mKeys.mEntry = other->mKeys.mEntry;
            mValues.mEntry = other->mValues.mEntry;
            other->mKeys.mEntry = nullptr;
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
         AllocateFresh<B>(other->GetReserved());
         auto asFrom = const_cast<B*>(reinterpret_cast<const B*>(&*other));

         if constexpr (CT::Typed<B>) {
            // At least one of the maps is dense                        
            using K = typename B::Key;
            using V = typename B::Value;

            if constexpr (CT::Dense<K>) {
               // We're cloning dense keys, so we're 100% sure, that    
               // each pair will end up in the same place               
               CopyMemory(mInfo, other->mInfo, GetReserved() + 1);

               if constexpr (CT::Inner::POD<K>) {
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

               if constexpr (CT::Dense<V>) {
                  if constexpr (CT::Inner::POD<V>) {
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
                  TAny<Deptr<V>> coalescedValues;
                  coalescedValues.Reserve(asFrom->GetCount());
                  for (auto item : *asFrom)
                     coalescedValues.Insert(IndexBack, SS::Nest(*item.mValue));
                  const_cast<Allocation*>(coalescedValues.mEntry)->Keep(asFrom->GetCount());
                  auto ptrVal = coalescedValues.GetRaw();

                  auto info = GetInfo();
                  const auto infoEnd = GetInfoEnd();
                  auto dstKey = GetValHandle<B>(0);
                  while (info != infoEnd) {
                     if (*info)
                        dstKey.Create(ptrVal, coalescedValues.mEntry);

                     ++info;
                     ++dstKey;
                     ++ptrVal;
                  }
               }

               mKeys.mCount = other->GetCount();
            }
            else {
               // We're cloning pointers, which will inevitably end up  
               // pointing elsewhere, which means that all pairs must   
               // be rehashed and reinserted                            
               TAny<Deptr<K>> coalescedKeys;
               coalescedKeys.Reserve(asFrom->GetCount());

               // Coalesce all densified elements, to avoid multiple    
               // allocations                                           
               for (auto& item : *asFrom)
                  coalescedKeys.Insert(IndexBack, SS::Nest(*item.mKey));
               const_cast<Allocation*>(coalescedKeys.mEntry)->Keep(asFrom->GetCount());

               // Zero info bytes and insert pointers                   
               ZeroMemory(mInfo, mKeys.mReserved);
               mInfo[mKeys.mReserved] = 1;

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
                        Abandon(HandleLocal<K> {Copy(ptr), coalescedKeys.mEntry}),
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
                  TAny<Deptr<V>> coalescedValues;
                  coalescedValues.Reserve(asFrom->GetCount());
                  for (auto& item : *asFrom)
                     coalescedValues.Insert(IndexBack, SS::Nest(*item.mValue));
                  const_cast<Allocation*>(coalescedValues.mEntry)->Keep(asFrom->GetCount());
                  auto ptrVal = coalescedValues.GetRaw();

                  while (ptr != ptrEnd) {
                     InsertInner<B, false>(
                        GetBucket(GetReserved() - 1, ptr),
                        Abandon(HandleLocal<K> {Copy(ptr),    coalescedKeys.mEntry}),
                        Abandon(HandleLocal<V> {Copy(ptrVal), coalescedValues.mEntry})
                     );

                     ++ptr;
                     ++ptrVal;
                  }
               }
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
               
               if (not asFrom->mValues.mType->mIsSparse) {
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
                  for (auto item : *asFrom)
                     coalescedValues.Insert(IndexBack, SS::Nest(*item.mValue));
                  const_cast<Allocation*>(coalescedValues.mEntry)->Keep(asFrom->GetCount());
                  auto ptrVal = coalescedValues.mRaw;
                  const Size valstride = coalescedValues.GetStride();

                  auto info = GetInfo();
                  const auto infoEnd = GetInfoEnd();
                  while (info != infoEnd) {
                     if (*info) {
                        GetValHandle<B>(info - GetInfo()).CreateSemantic(
                           Abandon(HandleLocal<void*> {Copy(ptrVal), coalescedValues.mEntry})
                        );
                     }

                     ++info;
                     ptrVal += valstride.mSize;
                  }
               }

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
               for (auto item : *asFrom)
                  coalescedKeys.InsertBlock(IndexBack, SS::Nest(*item.mKey));
               const_cast<Allocation*>(coalescedKeys.mEntry)->Keep(asFrom->GetCount());

               // Zero info bytes and insert pointers                   
               ZeroMemory(mInfo, mKeys.mReserved);
               mInfo[mKeys.mReserved] = 1;

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
                        Abandon(HandleLocal<void*> {Copy(ptr), coalescedKeys.mEntry}),
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
                  for (auto item : *asFrom)
                     coalescedValues.Insert(IndexBack, SS::Nest(*item.mValue));
                  const_cast<Allocation*>(coalescedValues.mEntry)->Keep(asFrom->GetCount());
                  auto ptrVal = coalescedValues.mRaw;
                  const Size valstride = coalescedValues.GetStride();

                  while (ptr != ptrEnd) {
                     InsertInner<B, false>(
                        GetBucket(GetReserved() - 1, ptr),
                        Abandon(HandleLocal<void*> {Copy(ptr), coalescedKeys.mEntry}),
                        Abandon(HandleLocal<void*> {Copy(ptrVal), coalescedValues.mEntry})
                     );

                     ptr += stride.mSize;
                     ptrVal += valstride.mSize;
                  }
               }
            }
         }
      }
   }

} // namespace Langulus::Anyness
