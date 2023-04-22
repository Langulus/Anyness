///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet.hpp"

namespace Langulus::Anyness
{
   
   /// Semantic copy (block has no ownership, so always just shallow copy)    
   ///   @param other - the block to shallow-copy                             
   LANGULUS(INLINED)
   constexpr BlockSet::BlockSet(CT::Semantic auto&& other) noexcept
      : BlockSet {static_cast<const BlockSet&>(other.mValue)} {}

   /// Semantic assignment                                                    
   /// Blocks have no ownership, so this always results in a block transfer   
   ///   @attention will never affect RHS                                     
   ///   @param rhs - the block to shallow copy                               
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   constexpr BlockSet& BlockSet::operator = (CT::Semantic auto&& rhs) noexcept {
      return operator = (static_cast<const BlockSet&>(rhs.mValue));
   }
   
   /// Semantically transfer the members of one set onto another              
   ///   @tparam TO - the type of set we're transferring to                   
   ///   @param from - the set and semantic to transfer from                  
   template<class TO>
   LANGULUS(INLINED)
   void BlockSet::BlockTransfer(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using FROM = TypeOf<S>;
      static_assert(CT::Set<TO>, "TO must be a set type");
      static_assert(CT::Set<FROM>, "FROM must be a set type");

      mKeys.mCount = other.mValue.mKeys.mCount;

      if constexpr (!CT::TypedSet<TO>) {
         // TO is not statically typed                                  
         mKeys.mType = other.mValue.GetType();
         mKeys.mState = other.mValue.mKeys.mState;
      }
      else {
         // TO is statically typed                                      
         mKeys.mType = MetaData::Of<TypeOf<TO>>();
         mKeys.mState = other.mValue.mKeys.mState + DataState::Typed;
      }

      if constexpr (S::Shallow) {
         mKeys.mRaw = other.mValue.mKeys.mRaw;
         mKeys.mReserved = other.mValue.mKeys.mReserved;
         mInfo = other.mValue.mInfo;

         if constexpr (S::Keep) {
            // Move/Copy other                                          
            mKeys.mEntry = other.mValue.mKeys.mEntry;

            if constexpr (S::Move) {
               if constexpr (!FROM::Ownership) {
                  // Since we are not aware if that block is referenced 
                  // or not we reference it just in case, and we also   
                  // do not reset 'other' to avoid leaks When using raw 
                  // BlockSets, it's your responsibility to take care   
                  // of ownership.                                      
                  Keep();
               }
               else {
                  other.mValue.mKeys.ResetMemory();
                  other.mValue.mKeys.ResetState();
               }
            }
            else Keep();
         }
         else if constexpr (S::Move) {
            // Abandon other                                            
            mKeys.mEntry = other.mValue.mKeys.mEntry;
            other.mValue.mKeys.mEntry = nullptr;
         }
      }
      else {
         // We're cloning, so we guarantee, that data is no longer      
         // static                                                      
         mKeys.mState -= DataState::Static;

         if constexpr (CT::TypedSet<TO>)
            BlockClone<TO>(other.mValue);
         else if constexpr (CT::TypedSet<FROM>)
            BlockClone<FROM>(other.mValue);
         else {
            // Use type-erased cloning                                  
            auto asTo = reinterpret_cast<TO*>(this);
            asTo->AllocateFresh(other.mValue.GetReserved());

            // Clone info array                                         
            CopyMemory(asTo->mInfo, other.mValue.mInfo, GetReserved() + 1);

            auto info = asTo->GetInfo();
            const auto infoEnd = asTo->GetInfoEnd();
            auto dstKey = asTo->GetValue(0);
            auto srcKey = other.mValue.GetValue(0);
            while (info != infoEnd) {
               if (*info) {
                  dstKey.CallUnknownSemanticConstructors(
                     1, Langulus::Clone(srcKey));
               }

               ++info;
               dstKey.Next();
               srcKey.Next();
            }
         }
      }
   }
   
   /// Clone info and keys from a statically typed set                        
   ///   @attention assumes this is not allocated                             
   ///   @tparam T - the statically optimized type of set we're using         
   ///   @param other - the set we'll be cloning                              
   template<class T>
   void BlockSet::BlockClone(const BlockSet& other) {
      static_assert(CT::TypedSet<T>, "T must be statically typed set");
      LANGULUS_ASSUME(DevAssumes, !mKeys.mRaw, "Set is already allocated");

      // Use statically optimized cloning                               
      auto asFrom = reinterpret_cast<T*>(&const_cast<BlockSet&>(other));
      auto asTo = reinterpret_cast<T*>(this);
      asTo->AllocateFresh(other.GetReserved());

      // Clone info array                                               
      CopyMemory(asTo->mInfo, asFrom->mInfo, GetReserved() + 1);

      // Clone keys and values                                          
      auto info = asTo->GetInfo();
      const auto infoEnd = asTo->GetInfoEnd();
      auto dstKey = asTo->GetHandle(0);
      auto srcKey = asFrom->GetHandle(0);
      while (info != infoEnd) {
         if (*info)
            dstKey.New(Langulus::Clone(srcKey));

         ++info;
         ++dstKey;
         ++srcKey;
      }
   }

} // namespace Langulus::Anyness
