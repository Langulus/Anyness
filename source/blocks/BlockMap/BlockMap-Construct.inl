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
   
   /// Semantic copy (block has no ownership, so always just shallow copy)    
   ///   @param other - the block to shallow-copy                             
   LANGULUS(INLINED)
   constexpr BlockMap::BlockMap(CT::Semantic auto&& other) noexcept
      : BlockMap {*other} {
      using S = Decay<decltype(other)>;
      static_assert(CT::Exact<TypeOf<S>, BlockMap>,
         "S type must be exactly BlockMap (build-time optimization)");
   }

   /// Semantic assignment                                                    
   /// Blocks have no ownership, so this always results in a block transfer   
   ///   @attention will never affect RHS                                     
   ///   @param rhs - the block to shallow copy                               
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   constexpr BlockMap& BlockMap::operator = (CT::Semantic auto&& rhs) noexcept {
      using S = Decay<decltype(rhs)>;
      static_assert(CT::Exact<TypeOf<S>, BlockMap>,
         "S type must be exactly BlockMap (build-time optimization)");
      return operator = (*rhs);
   }
   
   /// Semantically transfer the members of one map onto another              
   ///   @tparam TO - the type of map we're transferring to                   
   ///   @param from - the map and semantic to transfer from                  
   template<class TO>
   LANGULUS(INLINED)
   void BlockMap::BlockTransfer(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using FROM = TypeOf<S>;
      static_assert(CT::Map<TO>, "TO must be a map type");
      static_assert(CT::Map<FROM>, "FROM must be a map type");

      mValues.mCount = other->mValues.mCount;

      if constexpr (!CT::TypedMap<TO>) {
         // TO is not statically typed                                  
         mKeys.mType = other->GetKeyType();
         mKeys.mState = other->mKeys.mState;
         mValues.mType = other->GetValueType();
         mValues.mState = other->mValues.mState;
      }
      else {
         // TO is statically typed                                      
         mKeys.mType = MetaData::Of<typename TO::Key>();
         mKeys.mState = other->mKeys.mState + DataState::Typed;
         mValues.mType = MetaData::Of<typename TO::Value>();
         mValues.mState = other->mValues.mState + DataState::Typed;
      }

      if constexpr (S::Shallow) {
         mKeys.mRaw = other->mKeys.mRaw;
         mKeys.mReserved = other->mKeys.mReserved;
         mValues.mRaw = other->mValues.mRaw;
         mValues.mReserved = other->mValues.mReserved;
         mInfo = other->mInfo;

         if constexpr (S::Keep) {
            // Move/Copy other                                          
            mKeys.mEntry = other->mKeys.mEntry;
            mValues.mEntry = other->mValues.mEntry;

            if constexpr (S::Move) {
               if constexpr (!FROM::Ownership) {
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
         else if constexpr (S::Move) {
            // Abandon other                                            
            mKeys.mEntry = other->mKeys.mEntry;
            mValues.mEntry = other->mValues.mEntry;
            other->mValues.mEntry = nullptr;
         }
      }
      else {
         // We're cloning, so we guarantee, that data is no longer      
         // static                                                      
         mKeys.mState -= DataState::Static;
         mValues.mState -= DataState::Static;

         if constexpr (CT::TypedMap<TO>)
            BlockClone<TO>(*other);
         else if constexpr (CT::TypedMap<FROM>)
            BlockClone<FROM>(*other);
         else {
            // Use type-erased cloning                                  
            auto asTo = reinterpret_cast<TO*>(this);
            asTo->AllocateFresh(other->GetReserved());

            // Clone info array                                         
            CopyMemory(asTo->mInfo, other->mInfo, GetReserved() + 1);

            auto info = asTo->GetInfo();
            const auto infoEnd = asTo->GetInfoEnd();
            auto dstKey = asTo->GetKeyInner(0);
            auto dstVal = asTo->GetValueInner(0);
            auto srcKey = other->GetKeyInner(0);
            auto srcVal = other->GetValueInner(0);
            while (info != infoEnd) {
               if (*info) {
                  dstKey.CallUnknownSemanticConstructors(
                     1, Clone(srcKey));
                  dstVal.CallUnknownSemanticConstructors(
                     1, Clone(srcVal));
               }

               ++info;
               dstKey.Next(); dstVal.Next();
               srcKey.Next(); srcVal.Next();
            }
         }
      }
   }
   
   /// Clone info, keys and values from a statically typed map                
   ///   @attention assumes this is not allocated                             
   ///   @tparam T - the statically optimized type of map we're using         
   ///   @param other - the map we'll be cloning                              
   template<class T>
   void BlockMap::BlockClone(const BlockMap& other) {
      static_assert(CT::TypedMap<T>, "T must be statically typed map");
      LANGULUS_ASSUME(DevAssumes, !mValues.mRaw, "Map is already allocated");

      // Use statically optimized cloning                               
      auto asFrom = reinterpret_cast<T*>(&const_cast<BlockMap&>(other));
      auto asTo = reinterpret_cast<T*>(this);
      asTo->AllocateFresh(other.GetReserved());

      // Clone info array                                               
      CopyMemory(asTo->mInfo, asFrom->mInfo, GetReserved() + 1);

      // Clone keys and values                                          
      auto info = asTo->GetInfo();
      const auto infoEnd = asTo->GetInfoEnd();
      auto dstKey = asTo->GetKeyHandle(0);
      auto dstVal = asTo->GetValueHandle(0);
      auto srcKey = asFrom->GetKeyHandle(0);
      auto srcVal = asFrom->GetValueHandle(0);
      while (info != infoEnd) {
         if (*info) {
            dstKey.New(Clone(srcKey));
            dstVal.New(Clone(srcVal));
         }

         ++info;
         ++dstKey; ++dstVal;
         ++srcKey; ++srcVal;
      }
   }

} // namespace Langulus::Anyness
