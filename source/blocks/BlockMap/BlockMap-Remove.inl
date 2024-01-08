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
         return end<THIS>();

      RemoveInner<THIS>(offset--); //TODO what if map shrinks, offset might become invalid? Doesn't shrink for now
      
      while (offset < sentinel and not mInfo[offset])
         --offset;

      if (offset >= sentinel)
         offset = 0;

      return {
         mInfo + offset, 
         index.mSentinel,
         GetRawKey<THIS>(offset),
         GetRawValue<THIS>(offset)
      };
   }

   /// Erase a pair via a key match                                           
   ///   @param key - the key to search for                                   
   ///   @return the number of removed pairs                                  
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::RemoveKey(const CT::NotSemantic auto& key) {
      if (IsEmpty())
         return 0;

      using K = Deref<decltype(key)>;
      auto& me = reinterpret_cast<THIS&>(*this);

      if (me.template KeyIsSimilar<K>()
      or (CT::Typed<THIS> and ::std::equality_comparable_with<typename THIS::Key, K>)) {
         return RemoveKeyInner<THIS>(key);
      }
      else if constexpr (CT::StringLiteral<K>) {
         if (me.template KeyIsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return RemoveKeyInner<THIS>(Text {Disown(key)});
         }
         else if (me.template KeyIsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return RemoveKeyInner<THIS>(static_cast<const Deext<K>*>(key));
         }
         else return 0;
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

   /// Erase pairs via value                                                  
   ///   @param value - the values to search for                              
   ///   @return the number of removed pairs                                  
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::RemoveValue(const CT::NotSemantic auto& value) {
      if (IsEmpty())
         return 0;

      using V = Deref<decltype(value)>;
      auto& me = reinterpret_cast<THIS&>(*this);

      if (me.template ValueIsSimilar<V>()
      or (CT::Typed<THIS> and ::std::equality_comparable_with<typename THIS::Value, V>)) {
         return RemoveValueInner<THIS>(value);
      }
      else if constexpr (CT::StringLiteral<V>) {
         if (me.template ValueIsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return RemoveValueInner<THIS>(Text {Disown(value)});
         }
         else if (me.template ValueIsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return RemoveValueInner<THIS>(static_cast<const Deext<V>*>(value));
         }
         else return 0;
      }

      return 0;
   }

   /// Erase all pairs with a given value                                     
   ///   @attention this is significantly slower than removing a key          
   ///   @param match - the value to search for                               
   ///   @return the number of removed pairs                                  
   template<CT::Map THIS>
   Count BlockMap::RemoveValueInner(const CT::NotSemantic auto& value) {
      Count removed = 0;
      auto psl = GetInfo();
      const auto pslEnd = GetInfoEnd();
      auto val = GetValueHandle<THIS>(0);

      while (psl != pslEnd) {
         if (*psl and val.Get() == value) {
            // Remove every pair with matching value                    
            if constexpr (CT::Typed<THIS>)
               GetKeyHandle<THIS>(psl - GetInfo()).Destroy();
            else
               GetKeyHandle<THIS>(psl - GetInfo()).CallUnknownDestructors();

            val.Destroy();
            *psl = 0;
            ++removed;
            --mValues.mCount;
         }

         ++psl; ++val;
      }

      // Fill gaps if any                                               
      ShiftPairs<THIS>();
      return removed;
   }
   
   /// Erases a statically typed pair at a specific index                     
   ///   @attention assumes that index points to a valid entry                
   ///   @param index - the index to remove                                   
   template<CT::Map THIS>
   void BlockMap::RemoveInner(Offset index) IF_UNSAFE(noexcept) {
      auto psl = GetInfo() + index;
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid pair");

      // Destroy the key, info and value at the start                   
      // Use statically typed optimizations where possible              
      auto key = GetKeyHandle<THIS>(index);
      auto val = GetValueHandle<THIS>(index);

      if constexpr (CT::Typed<THIS>) {
         (key++).Destroy();
         (val++).Destroy();
      }
      else {
         key.template CallDestructors<Any>();
         key.Next();

         val.template CallDestructors<Any>();
         val.Next();
      }

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

         if constexpr (CT::Typed<THIS>) {
            (key - 1).New(Abandon(key));
            (key++).Destroy();

            (val - 1).New(Abandon(val));
            (val++).Destroy();
         }
         else {
            const_cast<const Block&>(key).Prev()
               .CallSemanticConstructors<Any>(1, Abandon(key));
            key.template CallDestructors<Any>();
            key.Next();

            const_cast<const Block&>(val).Prev()
               .CallSemanticConstructors<Any>(1, Abandon(val));
            val.template CallDestructors<Any>();
            val.Next();
         }

         #if LANGULUS_COMPILER_GCC()
            #pragma GCC diagnostic pop
         #endif

         *(psl++) = 0;
      }

      // Be aware, that iterator might loop around                      
      const auto pslEnd = GetInfoEnd();
      if (psl == pslEnd and *GetInfo() > 1) {
         const auto last = mValues.mReserved - 1;
         psl = GetInfo();
         GetInfo()[last] = (*psl) - 1;

         // Shift first pair to the back                                
         if constexpr (CT::Typed<THIS>) {
            key = GetKeyHandle<THIS>(0);
            GetKeyHandle<THIS>(last).New(Abandon(key));
            (key++).Destroy();

            val = GetValueHandle<THIS>(0);
            GetValueHandle<THIS>(last).New(Abandon(val));
            (val++).Destroy();
         }
         else {
            key = GetKeyHandle<THIS>(0);
            GetKeyHandle<THIS>(last)
               .template CallSemanticConstructors<Any>(1, Abandon(key));
            key.template CallDestructors<Any>();
            key.Next();

            val = GetValueHandle<THIS>(0);
            GetValueHandle<THIS>(last)
               .template CallSemanticConstructors<Any>(1, Abandon(val));
            val.template CallDestructors<Any>();
            val.Next();
         }

         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mValues.mCount;
   }

   /// Clears all data, but doesn't deallocate                                
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Clear() {
      if (IsEmpty())
         return;

      if (mValues.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner<THIS>();

         // Clear all info to zero                                      
         ZeroMemory(mInfo, GetReserved());
         mValues.mCount = 0;
      }
      else {
         // Data is used from multiple locations, don't change data     
         // We're forced to dereference and reset memory pointers       
         mInfo = nullptr;
         const_cast<Allocation*>(mValues.mEntry)->Free();
         mKeys.ResetMemory();
         mValues.ResetMemory();
      }
   }

   /// Clears all data and deallocates                                        
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::Reset() {
      if (mValues.mEntry) {
         if (mValues.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            if (not IsEmpty())
               ClearInner<THIS>();

            // No point in resetting info, we'll be deallocating it     
            LANGULUS_ASSUME(DevAssumes, mKeys.mEntry->GetUses() == 1,
               "Bad assumption");
            Allocator::Deallocate(const_cast<Allocation*>(mKeys.mEntry));
            Allocator::Deallocate(const_cast<Allocation*>(mValues.mEntry));
         }
         else {
            // Data is used from multiple locations, just deref values  
            const_cast<Allocation*>(mValues.mEntry)->Free();
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
   
   /// Destroy everything valid inside the map, but don't deallocate          
   ///   @attention doesn't affect count, or any container state              
   template<CT::Map THIS> LANGULUS(INLINED)
   void BlockMap::ClearInner() {
      auto remaining = GetCount();
      auto inf = GetInfo();
      const auto infEnd = GetInfoEnd();

      while (inf != infEnd and remaining) {
         if (*inf) {
            const auto offset = inf - GetInfo();
            auto key = GetKeyHandle  <THIS>(offset);
            auto val = GetValueHandle<THIS>(offset);

            if constexpr (CT::Typed<THIS>) {
               key.Destroy();
               val.Destroy();
            }
            else {
               key.template CallDestructors<Any>();
               val.template CallDestructors<Any>();
            }

            --remaining;
         }

         ++inf;
      }

      LANGULUS_ASSUME(DevAssumes, not remaining, "Leftover");
   }

} // namespace Langulus::Anyness
