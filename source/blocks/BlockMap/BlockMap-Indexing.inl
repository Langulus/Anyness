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

   /// Get a key reference                                                    
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes K is similar to the contained key type            
   ///   @tparam K - the type to reinterpret key as                           
   ///   @param i - the key index                                             
   ///   @return a reference to the key                                       
   template<CT::Data K>
   LANGULUS(INLINED)
   constexpr K& BlockMap::GetRawKey(Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
         "Wrong type when accessing map key",
         ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
      return GetKeys<K>().GetRaw()[i];
   }
   
   template<CT::Data K>
   LANGULUS(INLINED)
   constexpr const K& BlockMap::GetRawKey(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawKey<K>(i);
   }

   /// Get a key handle                                                       
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes K is similar to the contained key type            
   ///   @tparam K - the type to reinterpret key as                           
   ///   @param i - the key index                                             
   ///   @return the handle                                                   
   template<CT::Data K>
   LANGULUS(INLINED)
   constexpr Handle<K> BlockMap::GetKeyHandle(Offset i) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
         "Wrong type when accessing map key",
         ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
      return GetKeys<K>().GetHandle(i);
   }

   /// Get a value reference                                                  
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @tparam V - the type to reinterpret value as                         
   ///   @param i - the value index                                           
   ///   @return a reference to the value                                     
   template<CT::Data V>
   LANGULUS(INLINED)
   constexpr V& BlockMap::GetRawValue(Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map value",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mValues.template IsSimilar<V>(),
         "Wrong type when accessing map value",
         ", using type `", NameOf<V>(), "` instead of `", mValues.GetType(), '`');
      return GetValues<V>().GetRaw()[i];
   }

   template<CT::Data V>
   LANGULUS(INLINED)
   constexpr const V& BlockMap::GetRawValue(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawValue<V>(i);
   }
   
   /// Get a value handle                                                     
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @tparam V - the type to reinterpret value as                         
   ///   @param i - the value index                                           
   ///   @return the handle                                                   
   template<CT::Data V>
   LANGULUS(INLINED)
   constexpr Handle<V> BlockMap::GetValueHandle(Offset i) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(), 
         "Index out of limits when accessing map value", 
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mValues.template IsSimilar<V>(), 
         "Wrong type when accessing map value", 
         ", using type `", NameOf<V>(), "` instead of `", mValues.GetType(), '`');
      return GetValues<V>().GetHandle(i);
   }

} // namespace Langulus::Anyness
