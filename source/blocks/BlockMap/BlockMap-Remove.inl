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
#include "../../text/Text.hpp"


namespace Langulus::Anyness
{
      
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this map instance         
   ///   @attention assumes that iterator points to a valid entry             
   ///   @param index - the index to remove                                   
   ///   @return the iterator of the previous element, unless index is the    
   ///           first, or already at the end                                 
   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::Iterator<THIS> BlockMap::RemoveIt(const Iterator<THIS>& index) {
      const auto sentinel = GetReserved();
      auto offset = static_cast<Offset>(index.mInfo - mInfo);
      if (offset >= sentinel)
         return end();

      RemoveInner<THIS>(offset--);

      if (IsEmpty())
         return end();
      
      while (offset < sentinel and not mInfo[offset])
         --offset;

      if (offset >= sentinel)
         offset = 0;

      return {
         mInfo + offset, 
         index.mSentinel,
         GetRawKey<THIS>(offset),
         GetRawVal<THIS>(offset)
      };
   }

   /// Unfold-erase pairs by key                                              
   ///   @param key - the key to search for                                   
   ///   @return the number of removed pairs                                  
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::RemoveKey(const CT::NotSemantic auto& key) {
      using K = Deref<decltype(key)>;
      if (IsEmpty())
         return 0;

      if constexpr (CT::Array<K>) {
         if constexpr (CT::StringLiteral<K>) {
            if (IsKeySimilar<THIS, Text>()) {
               // Implicitly make a text container on string literal    
               return RemoveKeyInner<THIS>(Text {Disown(key)});
            }
            else if (IsKeySimilar<THIS, char*, wchar_t*>()) {
               // Cast away the extent, search for pointer              
               return RemoveKeyInner<THIS>(static_cast<const Deext<K>*>(key));
            }
            else return 0;
         }
         else if (IsKeySimilar<THIS, Deext<K>>()
         or (CT::Typed<THIS> and CT::Comparable<typename THIS::Key, Deext<K>>)) {
            // Remove all matching keys in the array                    
            Count removed = 0;
            for (auto& element : key)
               removed += RemoveKeyInner<THIS>(key);
            return removed;
         }
      }
      else {
         if (IsKeySimilar<THIS, K>()
         or (CT::Typed<THIS> and CT::Comparable<typename THIS::Key, K>)) {
            // Remove a single key                                      
            return RemoveKeyInner<THIS>(key);
         }
         else if constexpr (CT::Owned<K>) {
            if (IsKeySimilar<THIS, TypeOf<K>>()
            or (CT::Typed<THIS> and CT::Comparable<typename THIS::Key, TypeOf<K>>)) {
               // Remove a single key                                      
               return RemoveKeyInner<THIS>(key.Get());
            }
         }
      }
      return 0;
   }
   
   /// Erase a pair via key (inner)                                           
   ///   @param key - the key to search for                                   
   ///   @return 1 if pair was removed                                        
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::RemoveKeyInner(const CT::NotSemantic auto& key) {
      const auto found = FindInner<THIS>(key);
      if (found != InvalidOffset) {
         // Key found, remove the pair                                  
         RemoveInner<THIS>(found);
         return 1;
      }

      // No such key was found                                          
      return 0;
   }

   /// Unfold-erase pairs by value                                            
   ///   @param value - the values to search for                              
   ///   @return the number of removed pairs                                  
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::RemoveValue(const CT::NotSemantic auto& value) {
      using V = Deref<decltype(value)>;
      if (IsEmpty())
         return 0;

      if constexpr (CT::Array<V>) {
         if constexpr (CT::StringLiteral<V>) {
            if (IsValueSimilar<THIS, Text>()) {
               // Implicitly make a text container on string literal    
               return RemoveValInner<THIS>(Text {Disown(value)});
            }
            else if (IsValueSimilar<THIS, char*, wchar_t*>()) {
               // Cast away the extent, search for pointer instead      
               return RemoveValInner<THIS>(static_cast<const Deext<V>*>(value));
            }
            else return 0;
         }
         else if (IsValueSimilar<THIS, Deext<V>>()
         or (CT::Typed<THIS> and CT::Comparable<typename THIS::Value, Deext<V>>)) {
            // Remove all matching values in the array                  
            Count removed = 0;
            for (auto& element : value)
               removed += RemoveValInner<THIS>(value);
            return removed;
         }
      }
      else if constexpr (CT::Owned<V> or CT::Handle<V>) {
         // Remove a single value                                       
         return RemoveValInner<THIS>(value.Get());
      }
      else if (IsValueSimilar<THIS, V>()
      or (CT::Typed<THIS> and CT::Comparable<typename THIS::Value, V>)) {
         // Remove a single value                                       
         return RemoveValInner<THIS>(value);
      }
      return 0;
   }

   /// Erase all pairs with a given value                                     
   ///   @attention this is significantly slower than removing a key          
   ///   @param value - the value to search for                               
   ///   @return the number of removed pairs                                  
   template<CT::Map THIS>
   Count BlockMap::RemoveValInner(const CT::NotSemantic auto& value) {
      Count removed = 0;
      auto psl = GetInfo();
      const auto pslEnd = GetInfoEnd();
      auto val = GetValHandle<THIS>(0);

      while (psl != pslEnd) {
         if (*psl and val == value) {
            if (GetUses() > 1) {
               // Map is used from multiple locations, and we must      
               // branch out before changing it                         
               TODO();
            }

            // Remove every pair with matching value                    
            auto key = GetKeyHandle<THIS>(psl - GetInfo());
            key.Destroy();
            val.Destroy();
            *psl = 0;
            ++removed;
            --mKeys.mCount;
         }

         ++val;
         ++psl;
      }

      // Fill gaps if any                                               
      ShiftPairs<THIS>();
      return removed;
   }
   
   /// Erases a statically typed pair at a specific index                     
   ///   @attention if this map has more than one use, it will be copied to   
   ///      a new place, before any removals are done                         
   ///   @attention assumes that index points to a valid entry                
   ///   @param index - the index to remove                                   
   template<CT::Map THIS>
   void BlockMap::RemoveInner(const Offset index) {
      if (GetUses() > 1) {
         // Map is used from multiple locations, and we must branch out 
         // before changing it - only this copy will be affected        
         const BlockMap backup = *this;
         const_cast<Allocation*>(mKeys.mEntry)->Free();
         new (this) THIS {Copy(reinterpret_cast<const THIS&>(backup))};
      }

      auto psl = GetInfo() + index;
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid pair");

      // Destroy the key, info and value at the start                   
      // Use statically typed optimizations where possible              
      auto key = GetKeyHandle<THIS>(index);
      key.Destroy();
      ++key;

      auto val = GetValHandle<THIS>(index);
      val.Destroy();
      ++val;

      *(psl++) = 0;

      // And shift backwards, until a zero or 1 is reached              
      // That way we move every entry that is far from its start        
      // closer to it. Moving is costly, unless you use pointers        
      try_again:
      while (*psl > 1) {
         psl[-1] = (*psl) - 1;

         (key--).CreateSemantic(Abandon(key));
         key.Destroy();
         ++key;

         (val--).CreateSemantic(Abandon(val));
         val.Destroy();
         ++val;

         *(psl++) = 0;
      }

      // Be aware, that iterator might loop around                      
      const auto pslEnd = GetInfoEnd();
      if (psl == pslEnd and *GetInfo() > 1) {
         const auto last = mKeys.mReserved - 1;
         psl = GetInfo();
         GetInfo()[last] = (*psl) - 1;

         // Shift first pair to the back                                
         key = GetKeyHandle<THIS>(0);
         GetKeyHandle<THIS>(last).CreateSemantic(Abandon(key));
         key.Destroy();
         ++key;

         val = GetValHandle<THIS>(0);
         GetValHandle<THIS>(last).CreateSemantic(Abandon(val));
         val.Destroy();
         ++val;

         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mKeys.mCount;
   }

   /// Clears all data, but doesn't deallocate                                
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Clear() {
      if (IsEmpty())
         return;

      if (mKeys.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearPartInner<THIS>(GetKeys<THIS>());
         ClearPartInner<THIS>(GetVals<THIS>());

         // Clear all info to zero                                      
         ZeroMemory(mInfo, GetReserved());
         mKeys.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         ClearPartInner<THIS, false>(GetKeys<THIS>());
         ClearPartInner<THIS, false>(GetVals<THIS>());

         mInfo = nullptr;
         const_cast<Allocation*>(mKeys.mEntry)->Free();
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }
   }

   /// Clears all data and deallocates                                        
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Reset() {
      if (mKeys.mEntry) {
         if (mKeys.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            if (not IsEmpty()) {
               ClearPartInner<THIS>(GetKeys<THIS>());
               ClearPartInner<THIS>(GetVals<THIS>());
            }

            // No point in resetting info, we'll be deallocating it     
            LANGULUS_ASSUME(DevAssumes, mValues.mEntry->GetUses() == 1,
               "Only mKeys.mEntry should carry the reference count");
            Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
            Allocator::Deallocate(const_cast<Allocation*>(mValues.mEntry));
         }
         else {
            // Data is used from multiple locations, just deref values  
            if (not IsEmpty()) {
               ClearPartInner<THIS, false>(GetKeys<THIS>());
               ClearPartInner<THIS, false>(GetVals<THIS>());
            }

            const_cast<Allocation*>(mKeys.mEntry)->Free();
         }

         mInfo = nullptr;
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }

      mKeys.ResetState();
      mValues.ResetState();
   }
   
   /// If possible reallocates the map to a smaller one                       
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Compact() {
      TODO();
   }
   
   /// Destroy each element in 'mthis' that corresponds to a valid map entry  
   ///   @attention doesn't affect count, or any container state              
   ///   @tparam FORCE - used only when GetUses() == 1                        
   template<CT::Map THIS, bool FORCE>
   void BlockMap::ClearPartInner(CT::Block auto& part) {
      // Intentional slice                                              
      Block mthis = part;
      mthis.mCount = mKeys.mCount;
      mthis.mReserved = mKeys.mReserved;
      mthis.template Destroy<FORCE>(mInfo);
   }

} // namespace Langulus::Anyness
