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

   /// Erase a key                                                            
   /// The key may not match the contained key type                           
   ///   @tparam SET - set we're removing from, using to deduce type,         
   ///                 and as runtime optimization                            
   ///   @param key - the key to search for                                   
   ///   @return the number of removed pairs                                  
   template<class SET>
   LANGULUS(INLINED)
   Count BlockSet::Remove(const CT::NotSemantic auto& key) {
      static_assert(CT::Set<SET>, "SET must be a set type");
      using K = Deref<decltype(key)>;
      if (IsEmpty())
         return 0;

      auto& THIS = reinterpret_cast<SET&>(*this);
      Offset found = InvalidOffset;
      if constexpr (CT::Array<K> and CT::ExactAsOneOf<Decvq<Deext<K>>, char, wchar_t>) {
         if (THIS.template IsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            found = FindInner<SET>(Text {Disown(key)});
         }
         else if (THIS.template IsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            found = FindInner<SET>(static_cast<const Deext<K>*>(key));
         }
      }
      else if (THIS.template IsSimilar<K>())
         found = FindInner<SET>(key);

      if (found != InvalidOffset) {
         // Key found, remove it                                        
         RemoveInner<K>(found);
         return 1;
      }

      // No such key was found                                          
      return 0;
   }
   
   /// Erases element at a specific index                                     
   ///   @attention assumes that index points to a valid entry                
   ///   @attention assumes the set contains type similar to T, unless void   
   ///              (aka type-erased)                                         
   ///   @param T - type of key (use void for type-erasure)                   
   ///   @param index - the index to remove                                   
   template<class T>
   void BlockSet::RemoveInner(const Offset& index) IF_UNSAFE(noexcept) {
      auto psl = GetInfo() + index;
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid key");

      // Destroy the keyand info at the start                           
      // Use statically typed optimizations where possible              
      auto key = [this, &index]{
         if constexpr (CT::Void<T>) {
            auto key = GetInner(index);
            key.CallUnknownDestructors();
            key.Next();
            return key;
         }
         else {
            auto key = GetHandle<Decvq<T>>(index);
            (key++).Destroy();
            return key;
         }
      }();

      *(psl++) = 0;

      // And shift backwards, until a zero or 1 is reached              
      // That way we move every entry that is far from its start        
      // closer to it. Moving is costly, unless you use pointers        
      try_again:
      while (*psl > 1) {
         psl[-1] = (*psl) - 1;

         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wplacement-new"
         #endif

         if constexpr (CT::Void<T>) {
            const_cast<const Block&>(key).Prev()
               .CallUnknownSemanticConstructors(1, Abandon(key));
            key.CallUnknownDestructors();
            key.Next();
         }
         else {
            (key - 1).New(Abandon(key));
            (key++).Destroy();
         }

         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic pop
         #endif

         *(psl++) = 0;
      }

      // Be aware, that psl might loop around                           
      const auto pslEnd = GetInfoEnd();
      if (psl == pslEnd and *GetInfo() > 1) {
         const auto last = mKeys.mReserved - 1;
         psl = GetInfo();
         GetInfo()[last] = (*psl) - 1;

         // Shift first entry to the back                               
         if constexpr (CT::Void<T>) {
            key = GetInner(0);
            GetInner(last)
               .CallUnknownSemanticConstructors(1, Abandon(key));
            key.CallUnknownDestructors();
            key.Next();
         }
         else {
            key = GetHandle<Decvq<T>>(0);
            GetHandle<Decvq<T>>(last).New(Abandon(key));
            (key++).Destroy();
         }

         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mKeys.mCount;
   }

   /// Clears all data, but doesn't deallocate                                
   LANGULUS(INLINED)
   void BlockSet::Clear() {
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
         const_cast<Allocation*>(mKeys.mEntry)->Free();
         mKeys.ResetMemory();
      }
   }

   /// Clears all data and deallocates                                        
   LANGULUS(INLINED)
   void BlockSet::Reset() {
      if (mKeys.mEntry) {
         if (mKeys.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            ClearInner();

            // No point in resetting info, we'll be deallocating it     
            Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
         }
         else {
            // Data is used from multiple locations, just deref values  
            const_cast<Allocation*>(mKeys.mEntry)->Free();
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
         if (*inf)
            GetInner(inf - GetInfo()).CallUnknownDestructors();
         ++inf;
      }
   }

} // namespace Langulus::Anyness
