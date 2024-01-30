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


namespace Langulus::Anyness
{
   
   /// Semantically transfer the members of one set onto another              
   ///   @tparam TO - the type of set we're transferring to                   
   ///   @param other - the set and semantic to transfer from                 
   template<CT::Set TO, template<class> class S, CT::Set FROM>
   requires CT::Semantic<S<FROM>> LANGULUS(INLINED)
   void BlockSet::BlockTransfer(S<FROM>&& other) {
      using SS = S<FROM>;

      if constexpr (not CT::TypedSet<TO>) {
         // TO is not statically typed, so we can safely                
         // overwrite type and state                                    
         mKeys.mType = other->GetType();
         mKeys.mState = other->mKeys.mState;
      }
      else {
         // TO is typed, so we never touch mType, and we make sure that 
         // we don't affect Typed state                                 
         mKeys.mType = MetaDataOf<TypeOf<TO>>();
         mKeys.mState = other->mKeys.mState + DataState::Typed;
      }

      if constexpr (SS::Shallow) {
         // We're transferring via a shallow semantic                   
         mKeys.mCount = other->mKeys.mCount;
         mKeys.mRaw = other->mKeys.mRaw;
         mKeys.mReserved = other->mKeys.mReserved;
         mInfo = other->mInfo;

         if constexpr (SS::Keep) {
            // Move/Copy other                                          
            mKeys.mEntry = other->mKeys.mEntry;

            if constexpr (SS::Move) {
               if constexpr (not FROM::Ownership) {
                  // Since we are not aware if that block is referenced 
                  // or not we reference it just in case, and we also   
                  // do not reset 'other' to avoid leaks. When using    
                  // raw Blocks, it's your responsibility to take care  
                  // of ownership.                                      
                  Keep();
               }
               else {
                  other->mKeys.ResetMemory();
                  other->mKeys.ResetState();
               }
            }
            else Keep();
         }
         else if constexpr (SS::Move) {
            // Abandon other                                            
            mKeys.mEntry = other->mKeys.mEntry;
            other->mKeys.mEntry = nullptr;
         }
      }
      else {
         // We're cloning, so we guarantee, that data is no longer      
         // static and constant via state                               
         mKeys.mState -= DataState::Static | DataState::Constant;
         if (other->IsEmpty())
            return;

         // Always prefer statically typed set interface (if any)       
         using B = Conditional<CT::Typed<FROM>, FROM, TO>;
         AllocateFresh<B>(other->GetReserved());
         auto asFrom = const_cast<B*>(reinterpret_cast<const B*>(&*other));

         if (not asFrom->mKeys.mType->mIsSparse) {
            // We're cloning dense elements, so we're 100% sure, that   
            // each element will end up in the same place               
            CopyMemory(mInfo, other->mInfo, GetReserved() + 1);

            if constexpr (CT::Typed<B> and CT::Inner::POD<TypeOf<B>>) {
               // Data is POD, we can directly copy the entire table    
               CopyMemory(
                  mKeys.mRaw, asFrom->mKeys.mRaw,
                  GetReserved() * sizeof(TypeOf<B>)
               );
            }
            else if (asFrom->mKeys.mType->mIsPOD) {
               // Data is POD, we can directly copy the entire table    
               CopyMemory(
                  mKeys.mRaw, asFrom->mKeys.mRaw,
                  GetReserved() * asFrom->mKeys.mType->mSize
               );
            }
            else {
               // Data isn't pod, clone valid elements one by one       
               auto info = GetInfo();
               const auto infoEnd = GetInfoEnd();
               auto dstKey = GetHandle<B>(0);
               auto srcKey = asFrom->template GetHandle<B>(0);
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
            // We're cloning pointers, which will inevitably end up     
            // pointing elsewhere, which means that all elements must   
            // be rehashed and reinserted                               
            Conditional<CT::Typed<B>, Deptr<TypeOf<B>>, Any> coalesced;
            if constexpr (not CT::Typed<B>)
               coalesced.SetType(asFrom->mKeys.mType->mDeptr);
            coalesced.Reserve(GetCount());

            // Coalesce all densified elements, to avoid multiple       
            // allocations                                              
            for (auto& item : *asFrom) {
               if constexpr (CT::Typed<B>)
                  coalesced.Insert(IndexBack, SS::Nest(*item));
               else
                  coalesced.InsertBlock(IndexBack, SS::Nest(*item));
            }
            const_cast<Allocation*>(coalesced.mEntry)->Keep(GetCount());

            // Zero info bytes and insert pointers                      
            ZeroMemory(mInfo, mKeys.mReserved);
            mInfo[mKeys.mReserved] = 1;

            auto ptr = coalesced.GetRaw();
            const auto ptrEnd = coalesced.GetRawEnd();
            const Size stride = coalesced.GetStride();
            while (ptr != ptrEnd) {
               InsertInner<B, false>(
                  GetBucket(GetReserved() - 1, ptr),
                  Abandon(HandleLocal<void*> {Copy(ptr), coalesced.mEntry})
               );
               ptr += stride.mSize;
            }
         }
      }
   }

} // namespace Langulus::Anyness
