///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Block.hpp"
#include "../TAny.hpp"

namespace Langulus::Anyness
{

   /// Remove elements on the back                                            
   ///   @param count - the new count                                         
   void Block::Trim(const Count count) {
      if (count >= mCount)
         return;

      if (IsConstant() || IsStatic()) {
         if (mType->mIsPOD) {
            // If data is POD and elements are on the back, we can      
            // get around constantness and staticness, by simply        
            // truncating the count without any reprecussions           
            mCount = count;
         }
         else {
            LANGULUS_ASSERT(!IsConstant(), Access,
               "Removing from constant container");
            LANGULUS_ASSERT(!IsStatic(), Access,
               "Removing from static container");
         }

         return;
      }

      // Call destructors and change count                              
      CropInner(count, mCount - count).CallUnknownDestructors();
      mCount = count;
   }

   /// Destroy all elements, but don't deallocate memory                      
   void Block::Clear() {
      if (!mEntry) {
         // Data is either static or unallocated                        
         // Don't call destructors, just clear it up                    
         mRaw = nullptr;
         mCount = mReserved = 0;
         return;
      }

      if (mEntry->GetUses() == 1) {
         // Destroy all elements but don't deallocate the entry         
         CallUnknownDestructors();
         mCount = 0;
         return;
      }
      
      // If reached, then data is referenced from multiple places       
      // Don't call destructors, just clear it up and dereference       
      mEntry->Free();
      mRaw = nullptr;
      mEntry = nullptr;
      mCount = mReserved = 0;
   }

   /// Destroy all elements, deallocate block and reset state                 
   void Block::Reset() {
      Free();
      ResetMemory();
      ResetState();
   }

   /// Flattens unnecessarily deep containers and combines their states       
   /// when possible. Discards ORness if container has only one element       
   void Block::Optimize() {
      if (IsOr() && GetCount() == 1)
         MakeAnd();

      while (GetCount() == 1 && IsDeep()) {
         auto& subPack = As<Block>();
         if (!CanFitState(subPack)) {
            subPack.Optimize();
            if (subPack.IsEmpty())
               Reset();
            return;
         }

         Block temporary {subPack};
         subPack.ResetMemory();
         Free();
         *this = temporary;
      }

      if (GetCount() > 1 && IsDeep()) {
         for (Count i = 0; i < mCount; ++i) {
            auto& subBlock = As<Block>(i);
            subBlock.Optimize();
            if (subBlock.IsEmpty()) {
               RemoveIndex(i);
               --i;
            }
         }
      }
   }

} // namespace Langulus::Anyness
