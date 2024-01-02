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
   Block BlockMap::GetKey(CT::Index auto index) {
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
   Block BlockMap::GetKey(CT::Index auto index) const {
      return const_cast<BlockMap*>(this)->GetKey(index);
   }

   /// Get a value by a safe index                                            
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   LANGULUS(INLINED)
   Block BlockMap::GetValue(CT::Index auto index) {
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
   Block BlockMap::GetValue(CT::Index auto index) const {
      return const_cast<BlockMap&>(*this).GetValue(index);
   }

   /// Get a pair by a safe index                                             
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   LANGULUS(INLINED)
   Pair BlockMap::GetPair(CT::Index auto index) {
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
   Pair BlockMap::GetPair(CT::Index auto index) const {
      return const_cast<BlockMap&>(*this).GetPair(index);
   }

   /// Get a type-erased key                                                  
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the key, wrapped inside a block                              
   LANGULUS(INLINED)
   Block BlockMap::GetKeyInner(Offset i) IF_UNSAFE(noexcept) {
      return mKeys.GetElement(i);
   }

   /// Get a type-erased key (const)                                          
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the key, wrapped inside an immutable block                   
   LANGULUS(INLINED)
   Block BlockMap::GetKeyInner(Offset i) const IF_UNSAFE(noexcept) {
      return mKeys.GetElement(i);
   }

   /// Get a type-erased value                                                
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the value, wrapped inside a block                            
   LANGULUS(INLINED)
   Block BlockMap::GetValueInner(Offset i) IF_UNSAFE(noexcept) {
      return mValues.GetElement(i);
   }

   /// Get a type-erased value (const)                                        
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the value, wrapped inside an immutable block                 
   LANGULUS(INLINED)
   Block BlockMap::GetValueInner(Offset i) const IF_UNSAFE(noexcept) {
      return mValues.GetElement(i);
   }

   /// Get a pair by an unsafe offset (const)                                 
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   LANGULUS(INLINED)
   Pair BlockMap::GetPairInner(Offset i) const IF_UNSAFE(noexcept) {
      return {GetKeyInner(i), GetValueInner(i)};
   }

   /// Get a pair by an unsafe offset                                         
   ///   @attention as unsafe as it gets, for internal use only               
   ///   @param i - the offset to use                                         
   ///   @return the pair                                                     
   LANGULUS(INLINED)
   Pair BlockMap::GetPairInner(Offset i) IF_UNSAFE(noexcept) {
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
   ///   @param i - the key index                                             
   ///   @return a reference to the key                                       
   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetRawKey(Offset i) IF_UNSAFE(noexcept) {
      using K = typename THIS::Key;

      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
         "Wrong type when accessing map key",
         ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
      return GetKeys<THIS>().GetRaw()[i];
   }
   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetRawKey(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawKey<THIS>(i);
   }

   /// Get a key handle                                                       
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes K is similar to the contained key type            
   ///   @param i - the key index                                             
   ///   @return the handle                                                   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetKeyHandle(Offset i) const IF_UNSAFE(noexcept) {
      using K = typename THIS::Key;

      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
         "Wrong type when accessing map key",
         ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
      return GetKeys<THIS>().GetHandle(i);
   }

   /// Get a value reference                                                  
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @param i - the value index                                           
   ///   @return a reference to the value                                     
   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetRawValue(Offset i) IF_UNSAFE(noexcept) {
      using V = typename THIS::Value;

      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map value",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mValues.template IsSimilar<V>(),
         "Wrong type when accessing map value",
         ", using type `", NameOf<V>(), "` instead of `", mValues.GetType(), '`');
      return GetValues<THIS>().GetRaw()[i];
   }

   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetRawValue(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawValue<THIS>(i);
   }
   
   /// Get a value handle                                                     
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @param i - the value index                                           
   ///   @return the handle                                                   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetValueHandle(Offset i) const IF_UNSAFE(noexcept) {
      using V = typename THIS::Value;

      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map value", 
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mValues.template IsSimilar<V>(), 
         "Wrong type when accessing map value", 
         ", using type `", NameOf<V>(), "` instead of `", mValues.GetType(), '`');
      return GetValues<THIS>().GetHandle(i);
   }

} // namespace Langulus::Anyness
