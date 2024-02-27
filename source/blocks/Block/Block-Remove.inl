///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Block.hpp"
#include "../../one/Handle.inl"


namespace Langulus::Anyness
{
   
   /// Remove the first occurence of a given item                             
   ///   @tparam REVERSE - whether to search from the back                    
   ///   @param item - the item type to search for and remove                 
   ///   @return 1 if the element was found and removed, 0 otherwise          
   template<bool REVERSE, CT::Block THIS> LANGULUS(INLINED)
   Count Block::Remove(const CT::NotSemantic auto& item) {
      const auto found = Find<REVERSE, THIS>(item);
      return found ? RemoveIndex<THIS>(found.GetOffsetUnsafe(), 1) : 0;
   }
   
   /// Remove sequential indices                                              
   ///   @param index - index to start removing from                          
   ///   @param count - number of contiguous items to remove                  
   ///   @return the number of removed elements                               
   template<CT::Block THIS>
   Count Block::RemoveIndex(CT::Index auto index, const Count count) {
      using INDEX = Deref<decltype(index)>;

      if constexpr (CT::Similar<INDEX, Index>) {
         // By special indices                                          
         if (index == IndexAll) {
            const auto oldCount = mCount;
            Free<THIS>();
            ResetMemory();
            ResetState();
            return oldCount;
         }

         auto idx = Constrain<THIS>(index);
         if (idx.IsSpecial())
            return 0;

         return RemoveIndex<THIS>(idx.GetOffsetUnsafe(), count);
      }
      else {
         // By simple index                                             
         Offset idx = index;
         const auto ender = idx + count;
         const auto removed = ender - idx;
         LANGULUS_ASSUME(DevAssumes, ender <= mCount, "Out of range");

         if constexpr (CT::Typed<THIS>) {
            using T = TypeOf<THIS>;

            if (IsStatic() and ender == mCount) {
               // If data is static and elements are on the back, we    
               // can get around constantness and staticness, by        
               // truncating the count without any reprecussions        
               // We can't destroy static element anyways               
               mCount = idx;
               return count;
            }

            if (GetUses() > 1) {
               // Block is used from multiple locations, and we mush    
               // branch out before changing it                         
               TODO();
            }

            LANGULUS_ASSERT(IsMutable(), Access,
               "Attempting to remove from constant container");
            LANGULUS_ASSERT(not IsStatic(), Access,
               "Attempting to remove from static container");

            if constexpr (CT::Sparse<T> or CT::POD<T>) {
               MoveMemory(
                  GetRawAs<T>() + idx,
                  GetRawAs<T>() + ender,
                  mCount - ender
               );
            }
            else {
               // Call the destructors on the correct region            
               CropInner(idx, count).Destroy<THIS>();

               if (ender < mCount) {
                  // Fill gap	if any by invoking move constructions     
                  // Moving to the left, so no overlap possible         
                  const auto tail = mCount - ender;
                  CropInner(idx, tail)
                     .CreateSemantic<THIS>(
                        Abandon(CropInner(ender, tail)));
               }
            }
         }
         else {
            if (IsConstant() or IsStatic()) {
               if (mType->mIsPOD and idx + count >= mCount) {
                  // If data is POD and elements are on the back, we can
                  // get around constantness and staticness, by simply  
                  // truncating the count without any reprecussions     
                  const auto removed = mCount - idx;
                  mCount = idx;
                  return removed;
               }
               else {
                  LANGULUS_ASSERT(not IsConstant(), Access,
                     "Removing from constant container");
                  LANGULUS_ASSERT(not IsStatic(), Access,
                     "Removing from static container");
                  return 0;
               }
            }

            if (GetUses() > 1) {
               // Block is used from multiple locations, and we mush    
               // branch out before changing it                         
               TODO();
            }

            // First call the destructors on the correct region         
            CropInner(idx, removed).Destroy<THIS>();

            if (ender < mCount) {
               // Fill gap by invoking abandon-constructors             
               // We're moving to the left, so no reverse is required   
               const auto tail = mCount - ender;
               CropInner(idx, tail)
                  .CreateSemantic<THIS>(
                     Abandon(CropInner(ender, tail)));
            }
         }

         // Change count                                                
         mCount -= removed;
         return removed;
      }
   }

   /// Remove a deep index corresponding to a whole sub-block                 
   ///   @param index - index to remove                                       
   ///   @return 1 if block at that index was removed, 0 otherwise            
   template<CT::Block THIS>
   Count Block::RemoveIndexDeep(CT::Index auto index) {
      if constexpr (not CT::Same<decltype(index), Index>) {
         if (not IsDeep<THIS>())
            return 0;

         --index;

         for (Count i = 0; i != mCount; i += 1) {
            if (index == 0)
               return RemoveIndex<THIS>(i);

            auto ith = As<Block*>(i);
            const auto count = ith->GetCountDeep<Block>();
            if (index <= count and ith->RemoveIndexDeep<Block>(index)) //TODO could be further optimized using TypeOf<THIS> instead of Block when possible
               return 1;

            index -= count;
         }

         return 0;
      }
      else TODO();
   }

   /// Remove an element by using an iterator                                 
   ///   @param index - the iterator of the starting element                  
   ///   @param count - the number of elements to remove                      
   ///   @return an iterator pointing to the element at index - 1, or at end  
   ///      if block became empty                                             
   template<CT::Block THIS>
   Block::Iterator<THIS> Block::RemoveIt(const Iterator<THIS>& index, const Count count) {
      if (index.mValue >= GetRawEnd<THIS>())
         return end();

      const auto rawstart = GetRaw<THIS>();
      RemoveIndex<THIS>(index.mValue - rawstart, count);

      if (IsEmpty())
         return end();
      else if (index.mValue == rawstart)
         return {rawstart, GetRawEndAs<Byte, THIS>()};
      else
         return {index.mValue - 1, GetRawEndAs<Byte, THIS>()};
   }

   /// Remove elements at the back                                            
   ///   @param count - the new count                                         
   template<CT::Block THIS>
   void Block::Trim(const Count count) {
      if (count >= mCount)
         return;

      if (IsConstant() or IsStatic()) {
         if (mType->mIsPOD) {
            // If data is POD and elements are on the back, we can      
            // get around constantness and staticness, by simply        
            // truncating the count without any reprecussions           
            mCount = count;
         }
         else {
            LANGULUS_ASSERT(not IsConstant(), Access,
               "Removing from constant container");
            LANGULUS_ASSERT(not IsStatic(), Access,
               "Removing from static container");
         }

         return;
      }

      if (GetUses() > 1) {
         // Block is used from multiple locations, and we mush          
         // branch out before changing it                               
         TODO();
      }

      // Call destructors and change count                              
      CropInner(count, mCount - count).Destroy<THIS>();
      mCount = count;
   }

   /// Flattens unnecessarily deep containers and combines their states       
   /// when possible                                                          
   /// Discards ORness if container has only one element                      
   template<CT::Block THIS>
   void Block::Optimize() {
      if (IsOr() and GetCount() == 1)
         MakeAnd();

      while (GetCount() == 1 and IsDeep<THIS>()) {
         auto& subPack = As<Block>();
         if (not CanFitState<THIS>(subPack)) {
            subPack.Optimize<Block>();
            if (subPack.IsEmpty())
               Reset<THIS>();
            return;
         }

         Block temporary {subPack};
         subPack.ResetMemory();
         Free<THIS>();
         *this = temporary;
      }

      if (GetCount() > 1 and IsDeep<THIS>()) {
         for (Count i = 0; i < mCount; ++i) {
            auto& subBlock = As<Block>(i);
            subBlock.Optimize<Block>();
            if (subBlock.IsEmpty()) {
               RemoveIndex<THIS>(i);
               --i;
            }
         }
      }
   }

   /// Destroy all elements, but don't deallocate memory if possible          
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::Clear() {
      if (not mEntry) {
         // Data is either static or unallocated                        
         // Don't call destructors, just clear it up                    
         mRaw = nullptr;
         mCount = mReserved = 0;
         return;
      }

      if (mEntry->GetUses() == 1) {
         // Entry is used only in this block, so it's safe to           
         // destroy all elements. We can reuse the entry                
         Destroy<THIS>();
         mCount = 0;
      }
      else {
         // If reached, then data is referenced from multiple places    
         // Don't call destructors, just clear it up and dereference    
         const_cast<Allocation*>(mEntry)->Free();
         mRaw = nullptr;
         mEntry = nullptr;
         mCount = mReserved = 0;
      }
   }

   /// Destroy all elements, deallocate block and reset state                 
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::Reset() {
      Free<THIS>();
      ResetMemory();
      ResetState();
   }
   
   /// Reset the block's state                                                
   /// Type constraints shall remain, if any                                  
   template<CT::Block THIS> LANGULUS(INLINED)
   constexpr void Block::ResetState() noexcept {
      mState &= DataState::Typed;
      if constexpr (not CT::Typed<THIS>)
         ResetType<THIS>();
   }

   /// Call destructors of all initialized items                              
   ///   @attention never modifies any block state                            
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is not static                               
   template<CT::Block THIS>
   void Block::Destroy() const {
      LANGULUS_ASSUME(DevAssumes, GetUses() == 1,
         "Attempting to destroy elements used from multiple locations");
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Attempting to destroy elements in an empty container");
      LANGULUS_ASSUME(DevAssumes, not IsStatic(),
         "Destroying elements in a static container is not allowed");

      const auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;

         if constexpr (CT::Sparse<T>) {
            // Destroy all indirection layers, if their references reach
            // 1, and destroy the dense element, if it has destructor   
            // This is done in the following way:                       
            //    1. First dereference all handles that point to the    
            //       same memory together as one                        
            //    2. Destroy those groups, that are fully dereferenced  
            auto handle = GetHandle<T, THIS>(0);
            const auto handleEnd = handle + mCount;

            //                                                          
            while (handle.mValue != handleEnd.mValue) {
               if (not *handle.mEntry) {
                  ++handle;
                  continue;
               }

               // Count all handles that match the current entry        
               auto matches = 0;
               auto handle2 = handle + 1;
               while (handle2.mValue != handleEnd.mValue) {
                  if (*handle.mEntry == *handle2.mEntry)
                     ++matches;
                  ++handle2;
               }

               const_cast<Allocation*>(*handle.mEntry)->Free(matches);
               if (1 == (*handle.mEntry)->GetUses()) {
                  // Destroy all matching handles, but deallocate only  
                  // once after that                                    
                  if (matches) {
                     auto handle3 = handle + 1;
                     while (handle3.mValue != handleEnd.mValue) {
                        if (*handle.mEntry == *handle3.mEntry)
                           handle3.template Destroy<true, false>();
                        ++handle3;
                     }
                  }

                  handle.Destroy();
               }
               else {
                  // Just dereference once more, but also reset         
                  // the matching handle entries                        
                  if (matches) {
                     auto handle3 = handle + 1;
                     while (handle3.mValue != handleEnd.mValue) {
                        if (*handle.mEntry == *handle3.mEntry)
                           *handle3.mEntry = nullptr;
                        ++handle3;
                     }
                  }

                  const_cast<Allocation*>(*handle.mEntry)->Free(1);
               }

               ++handle;
            }
         }
         else if constexpr (CT::Destroyable<T>) {
            // Destroy every dense element                              
            using DT = Decay<T>;
            auto data = mthis->GetRaw<THIS>();
            const auto dataEnd = data + mCount;
            while (data != dataEnd) {
               data->~DT();
               ++data;
            }
         }
      }
      else {
         if (mType->mIsSparse) {
            // Destroy all indirection layers, if their references reach
            // 1, and destroy the dense element, if it has destructor   
            auto handle = mthis->template GetHandle<Byte*, THIS>(0);
            const auto handleEnd = handle + mCount;
            
            //                                                          
            while (handle.mValue != handleEnd.mValue) {
               if (not *handle.mEntry) {
                  ++handle;
                  continue;
               }

               // Count all handles that match the current entry        
               auto matches = 0;
               auto handle2 = handle + 1;
               while (handle2.mValue != handleEnd.mValue) {
                  if (*handle.mEntry == *handle2.mEntry)
                     ++matches;
                  ++handle2;
               }

               const_cast<Allocation*>(*handle.mEntry)->Free(matches);
               if (1 == (*handle.mEntry)->GetUses()) {
                  // Destroy all matching handles, but deallocate only  
                  // once after that                                    
                  if (matches) {
                     auto handle3 = handle + 1;
                     while (handle3.mValue != handleEnd.mValue) {
                        if (*handle.mEntry == *handle3.mEntry)
                           handle3.template DestroyUnknown<true, false>(mType);
                        ++handle3;
                     }
                  }

                  handle.DestroyUnknown(mType);
               }
               else {
                  // Just dereference once more, but also reset         
                  // the matching handle entries                        
                  if (matches) {
                     auto handle3 = handle + 1;
                     while (handle3.mValue != handleEnd.mValue) {
                        if (*handle.mEntry == *handle3.mEntry)
                           *handle3.mEntry = nullptr;
                        ++handle3;
                     }
                  }

                  const_cast<Allocation*>(*handle.mEntry)->Free(1);
               }

               ++handle;
            }
         }
         else if (mType->mDestructor) {
            // Destroy every dense element                              
            auto data = mthis->mRaw;
            const auto dataEnd = data + mType->mSize * mCount;
            while (data != dataEnd) {
               mType->mDestructor(data);
               data += mType->mSize;
            }
         }
      }

      // Always nullify upon destruction only if we're paranoid         
      IF_LANGULUS_PARANOID(ZeroMemory(mRaw, GetBytesize<THIS>()));
   }

   /// Reset the memory inside the block                                      
   LANGULUS(INLINED)
   constexpr void Block::ResetMemory() noexcept {
      mRaw = nullptr;
      mEntry = nullptr;
      mCount = mReserved = 0;
   }

} // namespace Langulus::Anyness