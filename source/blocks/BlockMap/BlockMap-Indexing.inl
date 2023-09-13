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

   /// Get a key by a safe index                                              
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   LANGULUS(INLINED)
   Block BlockMap::GetKey(const CT::Index auto& index) {
      auto offset = mValues.SimplifyIndex<void, true>(index);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            if (offset == 0)
               return GetKeyInner(info - GetInfo());
            --offset;
         }
         ++info;
      }

      LANGULUS_THROW(Access, "This shouldn't be reached, ever");
   }

   /// Get a key by a safe index (const)                                      
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   LANGULUS(INLINED)
   Block BlockMap::GetKey(const CT::Index auto& index) const {
      return const_cast<BlockMap*>(this)->GetKey(index);
   }

   /// Get a value by a safe index                                            
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   LANGULUS(INLINED)
   Block BlockMap::GetValue(const CT::Index auto& index) {
      auto offset = mValues.SimplifyIndex<void, true>(index);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            if (offset == 0)
               return GetValueInner(info - GetInfo());
            --offset;
         }
         ++info;
      }

      LANGULUS_THROW(Access, "This shouldn't be reached, ever");
   }

   /// Get a value by a safe index (const)                                    
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   LANGULUS(INLINED)
   Block BlockMap::GetValue(const CT::Index auto& index) const {
      return const_cast<BlockMap&>(*this).GetValue(index);
   }

   /// Get a pair by a safe index                                             
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   LANGULUS(INLINED)
   Pair BlockMap::GetPair(const CT::Index auto& index) {
      auto offset = mValues.SimplifyIndex<void, true>(index);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            if (offset == 0)
               return GetPairInner(info - GetInfo());
            --offset;
         }
         ++info;
      }

      LANGULUS_THROW(Access, "This shouldn't be reached, ever");
   }
   
   /// Get a pair by a safe index (const)                                     
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   LANGULUS(INLINED)
   Pair BlockMap::GetPair(const CT::Index auto& index) const {
      return const_cast<BlockMap&>(*this).GetPair(index);
   }

   /// Get a type-erased key                                                  
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the key, wrapped inside a block                              
   LANGULUS(INLINED)
   Block BlockMap::GetKeyInner(const Offset& i) IF_UNSAFE(noexcept) {
      return mKeys.GetElement(i);
   }

   /// Get a type-erased key (const)                                          
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the key, wrapped inside an immutable block                   
   LANGULUS(INLINED)
   Block BlockMap::GetKeyInner(const Offset& i) const IF_UNSAFE(noexcept) {
      return mKeys.GetElement(i);
   }

   /// Get a type-erased value                                                
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the value, wrapped inside a block                            
   LANGULUS(INLINED)
   Block BlockMap::GetValueInner(const Offset& i) IF_UNSAFE(noexcept) {
      return mValues.GetElement(i);
   }

   /// Get a type-erased value (const)                                        
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the value, wrapped inside an immutable block                 
   LANGULUS(INLINED)
   Block BlockMap::GetValueInner(const Offset& i) const IF_UNSAFE(noexcept) {
      return mValues.GetElement(i);
   }

   /// Get a pair by an unsafe offset (const)                                 
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   LANGULUS(INLINED)
   Pair BlockMap::GetPairInner(const Offset& i) const IF_UNSAFE(noexcept) {
      return {GetKeyInner(i), GetValueInner(i)};
   }

   /// Get a pair by an unsafe offset                                         
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   LANGULUS(INLINED)
   Pair BlockMap::GetPairInner(const Offset& i) IF_UNSAFE(noexcept) {
      return {GetKeyInner(i), GetValueInner(i)};
   }

   /// Get the bucket index, based on the provided value's hash               
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash                                     
   ///   @return the bucket index                                             
   LANGULUS(INLINED)
   Offset BlockMap::GetBucket(Offset mask, const CT::NotSemantic auto& value) noexcept {
      return HashOf(value).mHash & mask;
   }
   
   /// Get the bucket index, based on the wrapped value's hash                
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash, wrapped in a block                 
   ///   @return the bucket index                                             
   LANGULUS(INLINED)
   Offset BlockMap::GetBucketUnknown(Offset mask, const Block& value) noexcept {
      return value.GetHash().mHash & mask;
   }
   
   /// Get a raw key entry (const)                                            
   ///   @param index - the key index                                         
   ///   @return a constant reference to the element                          
   template<CT::Data K>
   LANGULUS(INLINED)
   constexpr const K& BlockMap::GetRawKey(Offset index) const noexcept {
      return GetKeys<K>().GetRaw()[index];
   }

   /// Get a raw key entry                                                    
   ///   @param index - the key index                                         
   ///   @return a mutable reference to the element                           
   template<CT::Data K>
   LANGULUS(INLINED)
   constexpr K& BlockMap::GetRawKey(Offset index) noexcept {
      return GetKeys<K>().GetRaw()[index];
   }

   /// Get a key handle if sparse, or a key pointer                           
   ///   @param index - the key index                                         
   ///   @return the handle                                                   
   template<CT::Data K>
   LANGULUS(INLINED)
   constexpr Handle<K> BlockMap::GetKeyHandle(Offset index) const noexcept {
      return GetKeys<K>().GetHandle(index);
   }

   /// Get a raw value entry (const)                                          
   ///   @param index - the value index                                       
   ///   @return a constant reference to the element                          
   template<CT::Data V>
   LANGULUS(INLINED)
   constexpr const V& BlockMap::GetRawValue(Offset index) const noexcept {
      return GetValues<V>().GetRaw()[index];
   }

   /// Get a raw value entry                                                  
   ///   @param index - the value index                                       
   ///   @return a mutable reference to the element                           
   template<CT::Data V>
   LANGULUS(INLINED)
   constexpr V& BlockMap::GetRawValue(Offset index) noexcept {
      return GetValues<V>().GetRaw()[index];
   }
   
   /// Get a value handle if sparse, or a key pointer                         
   ///   @param index - the value index                                       
   ///   @return the handle                                                   
   template<CT::Data V>
   LANGULUS(INLINED)
   constexpr Handle<V> BlockMap::GetValueHandle(Offset index) const noexcept {
      return GetValues<V>().GetHandle(index);
   }

} // namespace Langulus::Anyness
