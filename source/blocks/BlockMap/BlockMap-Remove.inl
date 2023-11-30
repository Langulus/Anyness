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
   /// The key may not match the contained key type                           
   ///   @tparam MAP - map we're removing from, using to deduce value type,   
   ///                 and as runtime optimization                            
   ///   @param key - the key to search for                                   
   ///   @return the number of removed pairs                                  
   template<class MAP>
   LANGULUS(INLINED)
   Count BlockMap::RemoveKey(const CT::NotSemantic auto& key) {
      static_assert(CT::Map<MAP>, "MAP must be a map type");
      using K = Deref<decltype(key)>;
      if (IsEmpty())
         return 0;

      auto& THIS = reinterpret_cast<MAP&>(*this);
      if constexpr (CT::Array<K> and CT::ExactAsOneOf<Decvq<Deext<K>>, char, wchar_t>) {
         if (THIS.template KeyIsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return RemoveKeyInner<MAP>(Text {Disown(key)});
         }
         else if (THIS.template KeyIsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return RemoveKeyInner<MAP>(static_cast<const Deext<K>*>(key));
         }
         else return 0;
      }
      else if (THIS.template KeyIsSimilar<K>())
         return RemoveKeyInner<MAP>(key);
      
      return 0;
   }
   
   /// Erase a pair via key (inner)                                           
   ///   @tparam MAP - map we're removing from, using to deduce value type,   
   ///                 and as runtime optimization                            
   ///   @param key - the key to search for                                   
   ///   @return 1 if pair was removed                                  
   template<class MAP>
   LANGULUS(INLINED)
   Count BlockMap::RemoveKeyInner(const CT::NotSemantic auto& key) {
      static_assert(CT::Map<MAP>, "MAP must be a map type");
      using K = Deref<decltype(key)>;

      const auto found = FindInner<MAP>(key);
      if (found != InvalidOffset) {
         // Key found, remove the pair                                  
         if constexpr (CT::TypedMap<MAP>)
            RemoveInner<K, typename MAP::Value>(found);
         else
            RemoveInner<K, void>(found);
         return 1;
      }

      // No such key was found                                          
      return 0;
   }

   /// Erase pairs via value                                                  
   /// The value may not match the contained value type                       
   ///   @tparam MAP - map we're removing from, using to deduce value type,   
   ///                 and as runtime optimization                            
   ///   @param value - the values to search for                              
   ///   @return the number of removed pairs                                  
   template<class MAP>
   LANGULUS(INLINED)
   Count BlockMap::RemoveValue(const CT::NotSemantic auto& value) {
      static_assert(CT::Map<MAP>, "MAP must be a map type");
      using V = Deref<decltype(value)>;
      if (IsEmpty())
         return 0;

      auto& THIS = reinterpret_cast<MAP&>(*this);
      if constexpr (CT::Array<V> and CT::ExactAsOneOf<Decvq<Deext<V>>, char, wchar_t>) {
         if (THIS.template ValueIsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return RemoveValueInner<MAP>(Text {Disown(value)});
         }
         else if (THIS.template ValueIsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return RemoveValueInner<MAP>(static_cast<const Deext<V>*>(value));
         }
         else return 0;
      }
      else if (THIS.template ValueIsSimilar<V>())
         return RemoveValueInner<MAP>(value);

      return 0;
   }

   /// Erase all pairs with a given value                                     
   ///   @attention this is significantly slower than removing a key          
   ///   @param match - the value to search for                               
   ///   @return the number of removed pairs                                  
   template<class MAP>
   Count BlockMap::RemoveValueInner(const CT::NotSemantic auto& value) {
      static_assert(CT::Map<MAP>, "MAP must be a map type");
      using V = Deref<decltype(value)>;

      Count removed {};
      auto psl = GetInfo();
      const auto pslEnd = GetInfoEnd();
      auto val = GetValueHandle<V>(0);

      while (psl != pslEnd) {
         if (*psl and val.Get() == value) {
            // Remove every pair with matching value                    
            if constexpr (CT::Typed<MAP>)
               GetKeyHandle<typename MAP::Key>(psl - GetInfo()).Destroy();
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
      if constexpr (CT::Typed<MAP>)
         ShiftPairs<typename MAP::Key, V>();
      else
         ShiftPairs<void, V>();
      return removed;
   }
   
   /// Erases a statically typed pair at a specific index                     
   ///   @attention assumes that index points to a valid entry                
   ///   @attention assumes the map contains types similar to K and V, unless 
   ///              one of those is void (aka type-erased)                    
   ///   @param K - type of key (use void for type-erasure)                   
   ///   @param V - type of value (use void for type-erasure)                 
   ///   @param index - the index to remove                                   
   template<class K, class V>
   void BlockMap::RemoveInner(const Offset& index) IF_UNSAFE(noexcept) {
      auto psl = GetInfo() + index;
      LANGULUS_ASSUME(DevAssumes, *psl, "Removing an invalid pair");

      // Destroy the key, info and value at the start                   
      // Use statically typed optimizations where possible              
      auto key = [this, &index]{
         if constexpr (CT::Void<K>) {
            auto key = GetKeyInner(index);
            key.CallUnknownDestructors();
            key.Next();
            return key;
         }
         else {
            auto key = GetKeyHandle<Decvq<K>>(index);
            (key++).Destroy();
            return key;
         }
      }();

      auto val = [this, &index] {
         if constexpr (CT::Void<V>) {
            auto val = GetValueInner(index);
            val.CallUnknownDestructors();
            val.Next();
            return val;
         }
         else {
            auto val = GetValueHandle<Decvq<V>>(index);
            (val++).Destroy();
            return val;
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

         if constexpr (CT::Void<K>) {
            const_cast<const Block&>(key).Prev()
               .CallUnknownSemanticConstructors(1, Abandon(key));
            key.CallUnknownDestructors();
            key.Next();
         }
         else {
            (key - 1).New(Abandon(key));
            (key++).Destroy();
         }

         if constexpr (CT::Void<V>) {
            const_cast<const Block&>(val).Prev()
               .CallUnknownSemanticConstructors(1, Abandon(val));
            val.CallUnknownDestructors();
            val.Next();
         }
         else {
            (val - 1).New(Abandon(val));
            (val++).Destroy();
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
         if constexpr (CT::Void<K>) {
            key = GetKeyInner(0);
            GetKeyInner(last)
               .CallUnknownSemanticConstructors(1, Abandon(key));
            key.CallUnknownDestructors();
            key.Next();
         }
         else {
            key = GetKeyHandle<Decvq<K>>(0);
            GetKeyHandle<Decvq<K>>(last).New(Abandon(key));
            (key++).Destroy();
         }

         if constexpr (CT::Void<V>) {
            val = GetValueInner(0);
            GetValueInner(last)
               .CallUnknownSemanticConstructors(1, Abandon(val));
            val.CallUnknownDestructors();
            val.Next();
         }
         else {
            val = GetValueHandle<Decvq<V>>(0);
            GetValueHandle<Decvq<V>>(last).New(Abandon(val));
            (val++).Destroy();
         }

         *(psl++) = 0;

         // And continue the vicious cycle                              
         goto try_again;
      }

      // Success                                                        
      --mValues.mCount;
   }

   /// Clears all data, but doesn't deallocate                                
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   template<class MAP>
   LANGULUS(INLINED)
   void BlockMap::Clear() {
      if (IsEmpty())
         return;

      if (mValues.mEntry->GetUses() == 1) {
         // Remove all used keys and values, they're used only here     
         ClearInner<MAP>();

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
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   template<class MAP>
   LANGULUS(INLINED)
   void BlockMap::Reset() {
      static_assert(CT::Map<MAP>, "MAP must be a map type");

      if (mValues.mEntry) {
         if (mValues.mEntry->GetUses() == 1) {
            // Remove all used keys and values, they're used only here  
            if (not IsEmpty())
               ClearInner<MAP>();

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
   LANGULUS(INLINED)
   void BlockMap::Compact() {
      //TODO();
   }
   
   /// Destroy everything valid inside the map                                
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @attention assumes there's at least one valid pair                   
   template<class MAP>
   LANGULUS(INLINED)
   void BlockMap::ClearInner() {
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(), "Map is empty");
      static_assert(CT::Map<MAP>, "MAP must be a map type");

      auto inf = GetInfo();
      const auto infEnd = GetInfoEnd();
      while (inf != infEnd) {
         if (*inf) {
            const auto offset = inf - GetInfo();
            if constexpr (CT::TypedMap<MAP>) {
               GetKeyHandle  <typename MAP::Key>  (offset).Destroy();
               GetValueHandle<typename MAP::Value>(offset).Destroy();
            }
            else {
               GetKeyInner  (offset).CallUnknownDestructors();
               GetValueInner(offset).CallUnknownDestructors();
            }
         }

         ++inf;
      }
   }

} // namespace Langulus::Anyness
