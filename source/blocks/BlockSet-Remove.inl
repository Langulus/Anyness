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

   /// Erase an element                                                       
   ///   @tparam T - the type of the element to remove (deducible)            
   ///   @param match - the element to search for                             
   ///   @return 1 if element was removed, 0 if not                           
   template<CT::NotSemantic T>
   Count BlockSet::Remove(const T& match) {
      // Get the starting index based on the key hash                   
      const auto start = GetBucket(GetReserved() - 1, match);
      auto key = mKeys.GetRawAs<T>() + start;
      auto info = GetInfo() + start;
      const auto infoEnd = GetInfoEnd();

      while (info != infoEnd) {
         if (*info && *key == match) {
            // Found it                                                 
            RemoveIndex(info - GetInfo());
            return 1;
         }

         ++key; ++info;
      }
      
      // No such key was found                                          
      return 0;
   }
   
   /// Erases element at a specific index                                     
   ///   @attention assumes that offset points to a valid entry               
   ///   @param offset - the index to remove                                  
   inline void BlockSet::RemoveIndex(const Offset& offset) SAFETY_NOEXCEPT() {
      auto psl = GetInfo() + offset;
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid key");

      const auto pslEnd = GetInfoEnd();
      auto key = mKeys.GetElement(offset);

      // Destroy the key, info and value at the offset                  
      key.CallUnknownDestructors();

      *(psl++) = 0;
      key.Next();

      // And shift backwards, until a zero or 1 is reached              
      // That way we move every entry that is far from its start        
      // closer to it. Moving is costly, unless you use pointers        
      try_again:
      while (*psl > 1) {
         psl[-1] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         const_cast<const Block&>(key).Prev()
            .CallUnknownSemanticConstructors(1, Abandon(key));

         key.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd && *GetInfo() > 1) UNLIKELY() {
         psl = GetInfo();
         key = mKeys.GetElement();

         // Shift first entry to the back                               
         const auto last = mKeys.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         GetValue(last)
            .CallUnknownSemanticConstructors(1, Abandon(key));

         key.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mKeys.mCount;
   }

   /// Clears all data, but doesn't deallocate                                
   inline void BlockSet::Clear() {
      if (IsEmpty())
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // Clear all info to zero                                      
         ZeroMemory(mInfo, GetReserved());
         mKeys.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         mInfo = nullptr;
         mKeys.mEntry->Free();
         mKeys.ResetMemory();
      }
   }

   /// Clears all data and deallocates                                        
   inline void BlockSet::Reset() {
      if (mKeys.mEntry) {
         if (mKeys.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            ClearInner();

            // No point in resetting info, we'll be deallocating it     
            Allocator::Deallocate(mKeys.mEntry);
         }
         else {
            // Data is used from multiple locations, just deref values  
            mKeys.mEntry->Free();
         }

         mInfo = nullptr;
         mKeys.ResetMemory();
      }

      mKeys.ResetState();
   }

   /// If possible reallocates the map to a smaller one                       
   LANGULUS(INLINED)
   void BlockSet::Compact() {
      //TODO();
   }
   
   /// Destroy everything initialized inside the map                          
   LANGULUS(INLINED)
   void BlockSet::ClearInner() {
      auto inf = GetInfo();
      const auto infEnd = GetInfoEnd();
      while (inf != infEnd) {
         if (*inf) {
            const auto offset = inf - GetInfo();
            GetValue(offset).CallUnknownDestructors();
         }

         ++inf;
      }
   }

} // namespace Langulus::Anyness
