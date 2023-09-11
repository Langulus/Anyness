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
   
   /// Erase a pair via key                                                   
   ///   @tparam THIS - type of map to use for FindIndex and RemoveIndex      
   ///   @tparam K - type of key (deducible)                                  
   ///   @param match - the key to search for                                 
   ///   @return the number of removed pairs                                  
   template<class THIS, CT::NotSemantic K>
   Count BlockMap::RemoveKey(const K& match) {
      static_assert(CT::Map<THIS>, "THIS must be a map type");
      auto& This = reinterpret_cast<THIS&>(*this);
      const auto found = FindIndex<THIS>(match);
      if (found != GetReserved()) {
         // Key found, remove the pair                                  
         This.RemoveIndex(found);
         return 1;
      }

      // No such key was found                                          
      return 0;
   }

   /// Erase all pairs with a given value                                     
   ///   @tparam THIS - type of map to use for FindIndex and RemoveIndex      
   ///   @tparam V - type of value to seek (deducible)                        
   ///   @attention this is very significantly slower than removing a key     
   ///   @param match - the value to search for                               
   ///   @return the number of removed pairs                                  
   template<class THIS, CT::NotSemantic V>
   Count BlockMap::RemoveValue(const V& match) {
      static_assert(CT::Map<THIS>, "THIS must be a map type");
      Count removed {};
      auto psl = GetInfo();
      const auto pslEnd = GetInfoEnd();
      auto val = GetValueHandle<V>(0);

      while (psl != pslEnd) {
         if (*psl and val.Get() == match) {
            // Remove every pair with matching value                    
            if constexpr (CT::Typed<THIS>)
               GetKeyHandle<typename THIS::Key>(psl - GetInfo()).Destroy();
            else
               GetKeyInner(psl - GetInfo()).CallUnknownDestructors();
            val.Destroy();
            *psl = 0;
            ++removed;
            --mValues.mCount;
         }

         ++psl; ++val;
      }

      // Fill gaps if any                                               
      if constexpr (CT::Typed<THIS>)
         ShiftPairs<typename THIS::Key, V>();
      else
         ShiftPairs<void, V>();
      return removed;
   }

   /// Erases element at a specific index                                     
   ///   @attention assumes that offset points to a valid entry               
   ///   @param offset - the index to remove                                  
   inline void BlockMap::RemoveIndex(const Offset& offset) SAFETY_NOEXCEPT() {
      auto psl = GetInfo() + offset;
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid pair");

      const auto pslEnd = GetInfoEnd();
      auto key = GetKeyInner(offset);
      auto val = GetValueInner(offset);

      // Destroy the key, info and value at the offset                  
      key.CallUnknownDestructors();
      val.CallUnknownDestructors();

      *(psl++) = 0;
      key.Next();
      val.Next();

      // And shift backwards, until a zero or 1 is reached              
      // That way we move every entry that is far from its start        
      // closer to it. Moving is costly, unless you use pointers        
      try_again:
      while (*psl > 1) {
         psl[-1] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         const_cast<const Block&>(key).Prev()
            .CallUnknownSemanticConstructors(1, Abandon(key));
         const_cast<const Block&>(val).Prev()
            .CallUnknownSemanticConstructors(1, Abandon(val));

         key.CallUnknownDestructors();
         val.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();
         val.Next();
      }

      // Be aware, that psl might loop around                           
      if (psl == pslEnd and *GetInfo() > 1) UNLIKELY() {
         psl = GetInfo();
         key = GetKeyInner(0);
         val = GetValueInner(0);

         // Shift first entry to the back                               
         const auto last = mValues.mReserved - 1;
         GetInfo()[last] = (*psl) - 1;

         // We're moving only a single element, so no chance of overlap 
         GetKeyInner(last)
            .CallUnknownSemanticConstructors(1, Abandon(key));
         GetValueInner(last)
            .CallUnknownSemanticConstructors(1, Abandon(val));

         key.CallUnknownDestructors();
         val.CallUnknownDestructors();

         *(psl++) = 0;
         key.Next();
         val.Next();

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mValues.mCount;
   }

   /// Clears all data, but doesn't deallocate                                
   LANGULUS(INLINED)
   void BlockMap::Clear() {
      if (IsEmpty())
         return;

      if (mValues.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner();

         // Clear all info to zero                                      
         ZeroMemory(mInfo, GetReserved());
         mValues.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         mInfo = nullptr;
         mValues.mEntry->Free();
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }
   }

   /// Clears all data and deallocates                                        
   LANGULUS(INLINED)
   void BlockMap::Reset() {
      if (mValues.mEntry) {
         if (mValues.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            if (not IsEmpty())
               ClearInner();

            // No point in resetting info, we'll be deallocating it     
            Allocator::Deallocate(mKeys.mEntry);
            Allocator::Deallocate(mValues.mEntry);
         }
         else {
            // Data is used from multiple locations, just deref values  
            mValues.mEntry->Free();
         }

         mInfo = nullptr;
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }

      mKeys.ResetState();
      mValues.ResetState();
   }
   
   /// If possible reallocates the map to a smaller one                       
   inline void BlockMap::Compact() {
      //TODO();
   }
   
   /// Destroy everything valid inside the map                                
   ///   @attention assumes there's at least one valid pair                   
   LANGULUS(INLINED)
   void BlockMap::ClearInner() {
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(), "Map is empty");
      auto inf = GetInfo();
      const auto infEnd = GetInfoEnd();
      while (inf != infEnd) {
         if (*inf) {
            const auto offset = inf - GetInfo();
            GetKeyInner(offset).CallUnknownDestructors();
            GetValueInner(offset).CallUnknownDestructors();
         }

         ++inf;
      }
   }

} // namespace Langulus::Anyness
