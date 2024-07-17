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


namespace Langulus::Anyness
{
   
   /// Remove the first occurence of a given item                             
   ///   @tparam REVERSE - whether to search from the back                    
   ///   @param item - the item type to search for and remove                 
   ///   @return 1 if the element was found and removed, 0 otherwise          
   template<class TYPE> template<bool REVERSE> LANGULUS(INLINED)
   Count Block<TYPE>::Remove(const CT::NoIntent auto& item) {
      const auto found = Find<REVERSE>(item);
      return found ? RemoveIndex(found.GetOffsetUnsafe(), 1) : 0;
   }
   
   /// Remove sequential indices                                              
   ///   @param index - index to start removing from                          
   ///   @param count - number of contiguous items to remove                  
   ///   @return the number of removed elements                               
   template<class TYPE>
   Count Block<TYPE>::RemoveIndex(CT::Index auto index, const Count count) {
      using INDEX = Deref<decltype(index)>;

      if constexpr (CT::Similar<INDEX, Index>) {
         // By special indices                                          
         if (index == IndexAll) {
            const auto oldCount = mCount;
            Free();
            ResetMemory();
            ResetState();
            return oldCount;
         }

         auto idx = Constrain(index);
         if (idx.IsSpecial())
            return 0;

         return RemoveIndex(idx.GetOffsetUnsafe(), count);
      }
      else {
         // By simple index                                             
         Offset idx = index;
         const auto ender = idx + count;
         const auto removed = ender - idx;
         LANGULUS_ASSUME(DevAssumes, ender <= mCount, "Out of range");

         if constexpr (TypeErased) {
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

            // First call the destructors on the correct region         
            BranchOut();
            CropInner(idx, removed).Destroy();

            if (ender < mCount) {
               // Fill gap by invoking abandon-constructors             
               // We're moving to the left, so no reverse is required   
               const auto tail = mCount - ender;
               CropInner(idx, tail).CreateWithIntent(
                  Abandon(CropInner(ender, tail)));
            }
         }
         else {
            if (IsStatic() and ender == mCount) {
               // If data is static and elements are on the back, we    
               // can get around constantness and staticness, by        
               // truncating the count without any reprecussions        
               // We can't destroy static element anyways               
               mCount = idx;
               return count;
            }

            LANGULUS_ASSERT(IsMutable(), Access,
               "Attempting to remove from constant container");
            LANGULUS_ASSERT(not IsStatic(), Access,
               "Attempting to remove from static container");
            BranchOut();

            if constexpr (CT::Sparse<TYPE> or CT::POD<TYPE>) {
               // Batch move                                            
               MoveMemory(GetRaw() + idx, GetRaw() + ender, mCount - ender);
            }
            else {
               // Call the destructors on the correct region            
               CropInner(idx, count).Destroy();

               if (ender < mCount) {
                  // Fill gap	if any by invoking move constructions     
                  // Moving to the left, so no overlap possible         
                  const auto tail = mCount - ender;
                  CropInner(idx, tail).CreateWithIntent(
                     Abandon(CropInner(ender, tail)));
               }
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
   template<class TYPE>
   Count Block<TYPE>::RemoveIndexDeep(CT::Index auto index) {
      if constexpr (not CT::Same<decltype(index), Index>) {
         if (not IsDeep())
            return 0;

         --index;

         for (Count i = 0; i != mCount; i += 1) {
            if (index == 0)
               return RemoveIndex(i);

            auto& ith = GetDeep(i);
            const auto count = ith.GetCountDeep();
            if (index <= count and ith.RemoveIndexDeep(index))
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
   template<class TYPE>
   typename Block<TYPE>::Iterator Block<TYPE>::RemoveIt(
      const Iterator& index, const Count count
   ) {
      if (index.mValue >= GetRawEnd())
         return end();

      const auto rawstart = GetRaw();
      RemoveIndex(index.mValue - rawstart, count);

      if (IsEmpty())
         return end();
      else if (index.mValue == rawstart)
         return {rawstart, GetRawEnd()};
      else
         return {index.mValue - 1, GetRawEnd()};
   }

   /// Remove elements at the back                                            
   ///   @param count - the new count                                         
   template<class TYPE>
   void Block<TYPE>::Trim(const Count count) {
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

      // Call destructors and change count                              
      BranchOut();
      CropInner(count, mCount - count).Destroy();
      mCount = count;
   }

   /// Flattens unnecessarily deep containers and combines their states       
   /// when possible                                                          
   /// Discards ORness if container has only one element                      
   template<class TYPE>
   void Block<TYPE>::Optimize() {
      while (GetCount() == 1 and IsDeep()) {
         auto& subPack = GetDeep();
         if (not CanFitState(subPack)) {
            subPack.Optimize();
            if (subPack.IsEmpty())
               Reset();
            return;
         }

         auto temporary = subPack;
         subPack.ResetMemory();
         Free();
         *this = temporary;
      }

      if (GetCount() > 1 and IsDeep()) {
         for (Count i = 0; i < mCount; ++i) {
            auto& subBlock = GetDeep(i);
            subBlock.Optimize();
            if (subBlock.IsEmpty()) {
               RemoveIndex(i);
               --i;
            }
         }
      }
   }

   /// Destroy all elements, but don't deallocate memory if possible          
   template<class TYPE> LANGULUS(INLINED)
   void Block<TYPE>::Clear() {
      if (not mEntry) {
         // Data is either static or unallocated                        
         // Don't call destructors, just clear it up                    
         mRaw   = nullptr;
         mCount = mReserved = 0;
         ResetType();
         return;
      }

      if (mEntry->GetUses() == 1) {
         // Entry is used only in this block, so it's safe to           
         // destroy all elements. We will reuse the entry and type      
         Destroy();
         mCount = 0;
      }
      else {
         // If reached, then data is referenced from multiple places    
         // Don't call destructors, just clear it up and dereference    
         const_cast<Allocation*>(mEntry)->Free();
         mRaw   = nullptr;
         mEntry = nullptr;
         mCount = mReserved = 0;
         ResetType();
      }
   }

   /// Destroy all elements, deallocate block and reset state                 
   template<class TYPE> LANGULUS(INLINED)
   void Block<TYPE>::Reset() {
      Free();
      ResetMemory();
      ResetState();
   }
   
   /// Reset the block's state                                                
   /// Type constraints shall remain, if any                                  
   template<class TYPE> LANGULUS(INLINED)
   constexpr void Block<TYPE>::ResetState() noexcept {
      mState &= DataState::Typed;
      ResetType();
   }

   /// Call destructors of all initialized items                              
   ///   @attention never modifies any block state                            
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is not static                               
   ///   @tparam FORCE - used only when GetUses() == 1                        
   ///   @param mask - internally used for destroying tables (tag dispatch)   
   template<class TYPE> template<bool FORCE, class MASK>
   void Block<TYPE>::Destroy(MASK mask) const {
      LANGULUS_ASSUME(DevAssumes, not FORCE or GetUses() == 1,
         "Attempting to destroy elements used from multiple locations");
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Attempting to destroy elements in an empty container");
      LANGULUS_ASSUME(DevAssumes, not IsStatic(),
         "Destroying elements in a static container is not allowed");

      constexpr bool MASKED = not CT::Nullptr<MASK>;
      const auto mthis = const_cast<Block*>(this);

      if constexpr (not TypeErased) {
         using DT = Decay<TYPE>;

         if constexpr (CT::Sparse<TYPE> and FORCE) {
            // Destroy every sparse element                             
            DestroySparse(mask);
         }
         else if constexpr (CT::Dense<TYPE> and CT::Destroyable<DT>
         and (FORCE or CT::Referencable<DT>)) {
            // Destroy every dense element                              
            const auto count = not MASKED ? mCount : mReserved;
            auto data = mthis->GetRaw();
            const auto begMarker = data;
            const auto endMarker = data + count;
            UNUSED() Count remaining;
            if constexpr (MASKED)
               remaining = GetCount();

            while (data != endMarker) {
               if constexpr (MASKED) {
                  if (not remaining)
                     break;

                  if (not mask[data - begMarker]) {
                     ++data;
                     continue;
                  }

                  --remaining;
               }

               if constexpr (FORCE) {
                  if constexpr (CT::Referencable<DT>)
                     data->Reference(-1);
                  data->~DT();
               }
               else {
                  if constexpr (CT::Referencable<DT>) {
                     if (not data->Reference(-1))
                        data->~DT();
                  }
               }

               ++data;
            }
         }
      }
      else {
         if (FORCE and mType->mIsSparse) {
            // Destroy every sparse element                             
            DestroySparse(mask);
         }
         else if (not mType->mIsSparse and mType->mDestructor
         and (FORCE or mType->mReference)) {
            // Destroy every dense element                              
            const auto count = not MASKED ? mCount : mReserved;
            auto data = mthis->mRaw;
            UNUSED() int index;
            UNUSED() Count remaining;
            if constexpr (MASKED) {
               index = 0;
               remaining = GetCount();
            }
            const auto endMarker = data + mType->mSize * count;

            if (mType->mReference) {
               while (data != endMarker) {
                  if constexpr (MASKED) {
                     if (not remaining)
                        break;

                     if (not mask[index]) {
                        data += mType->mSize;
                        ++index;
                        continue;
                     }

                     --remaining;
                  }

                  if constexpr (FORCE) {
                     mType->mReference(data, -1);
                     mType->mDestructor(data);
                  }
                  else {
                     if (not mType->mReference(data, -1))
                        mType->mDestructor(data);
                  }

                  data += mType->mSize;

                  if constexpr (MASKED)
                     ++index;
               }
            }
            else if constexpr (FORCE) {
               while (data != endMarker) {
                  if constexpr (MASKED) {
                     if (not remaining)
                        break;

                     if (not mask[index]) {
                        data += mType->mSize;
                        ++index;
                        continue;
                     }

                     --remaining;
                  }

                  mType->mDestructor(data);
                  data += mType->mSize;

                  if constexpr (MASKED)
                     ++index;
               }
            }
         }
      }

      // Always nullify upon destruction only if we're paranoid         
      //TODO IF_LANGULUS_PARANOID(ZeroMemory(mRaw, GetBytesize<THIS>()));
   }

   /// Call destructors of all initialized sparse items                       
   ///   @attention never modifies any block state                            
   ///   @attention assumes block is not empty                                
   ///   @attention assumes block is not static                               
   ///   @param mask - internally used for destroying tables (tag dispatch)   
   template<class TYPE> template<class MASK>
   void Block<TYPE>::DestroySparse(MASK mask) const {
      LANGULUS_ASSUME(DevAssumes, IsSparse(), "Container must be sparse");

      // Destroy all indirection layers, if their references reach      
      // 1, and destroy the dense element, if it has destructor         
      // This is done in the following way:                             
      //    1. First dereference all handles that point to the          
      //       same memory together as one                              
      //    2. Destroy those groups, that are fully dereferenced        
      constexpr bool MASKED = not CT::Nullptr<MASK>;
      const auto count = not MASKED ? mCount : mReserved;
      const auto mthis = const_cast<Block*>(this);

      auto handle = mthis->template GetHandle<void*>(0);
      const auto begMarker = handle.mValue;
      const auto endMarker = handle.mValue + count;
      UNUSED() Count remaining;
      if constexpr (MASKED)
         remaining = GetCount();

      // Execute a call for each handle that matches current entry      
      const auto for_each_match = [&](auto&& call) {
         auto handle2 = handle + 1;
         UNUSED() Count remaining2;
         if constexpr (MASKED)
            remaining2 = remaining;
         while (handle2.mValue != endMarker) {
            if constexpr (MASKED) {
               if (not remaining2)
                  break;
               if (not mask[handle2.mValue - begMarker]) {
                  ++handle2;
                  continue;
               }
               --remaining2;
            }
            if (handle.GetEntry() == handle2.GetEntry())
               call(handle2);
            ++handle2;
         }
      };

      //                                                                
      while (handle.mValue != endMarker) {
         if constexpr (MASKED) {
            if (not remaining)
               break;

            if (not mask[handle.mValue - begMarker]) {
               ++handle;
               continue;
            }

            --remaining;
         }

         if (not handle.GetEntry()) {
            ++handle;
            continue;
         }

         // Count all handles that match the current entry              
         auto matches = 0;
         for_each_match([&matches](const Handle<void*>&) {
            ++matches;
         });

         if (matches) {
            const_cast<Allocation*>(handle.GetEntry())->Free(matches);

            if (1 == handle.GetEntry()->GetUses()) {
               // Destroy all matching handles, but deallocate only     
               // once after that                                       
               for_each_match([&](Handle<void*>& h) {
                  h.template Destroy<true, false>(mType);
               });
            }
            else {
               // Just dereference once more, but also reset            
               // the matching handle entries                           
               for_each_match([](Handle<void*>& h) {
                  h.GetEntry() = nullptr;
               });
            }
         }

         handle.Destroy(mType);
         ++handle;
      }
   }

   /// Reset the memory inside the block                                      
   template<class TYPE> LANGULUS(INLINED)
   constexpr void Block<TYPE>::ResetMemory() noexcept {
      mRaw   = nullptr;
      mEntry = nullptr;
      mCount = mReserved = 0;
   }

} // namespace Langulus::Anyness