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

   /// Get a valid key by any index, safely                                   
   ///   @param index - the index to use                                      
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::Get(const CT::Index auto& index) {
      auto offset = mKeys.SimplifyIndex<void, true>(index);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            if (offset == 0)
               return GetInner(info - GetInfo());
            --offset;
         }
         ++info;
      }

      LANGULUS_THROW(Access, "This shouldn't be reached, ever");
   }

   /// Get a valid key by any index, safely (const)                           
   ///   @param index - the index to use                                      
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::Get(const CT::Index auto& index) const {
      return const_cast<BlockSet*>(this)->Get(index);
   }
   
   /// Get a valid key by any index, safely                                   
   ///   @param index - the index to use                                      
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::operator[] (const CT::Index auto& index) {
      return Get(index);
   }

   /// Get a valid key by any index, safely (const)                           
   ///   @param index - the index to use                                      
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::operator[] (const CT::Index auto& index) const {
      return Get(index);
   }

   /// Get a raw key by an unsafe offset                                      
   ///   @attention assumes index is in container's limits                    
   ///   @param i - the offset to use                                         
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::GetInner(const Offset& i) IF_UNSAFE(noexcept) {
      return mKeys.GetElement(i);
   }

   /// Get a raw key by an unsafe offset (const)                              
   ///   @attention assumes index is in container's limits                    
   ///   @param i - the offset to use                                         
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::GetInner(const Offset& i) const IF_UNSAFE(noexcept) {
      return mKeys.GetElement(i);
   }

   /// Get the bucket index, based on the provided value's hash               
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash                                     
   ///   @return the bucket index                                             
   LANGULUS(INLINED)
   Offset BlockSet::GetBucket(Offset mask, const CT::NotSemantic auto& value) noexcept {
      return HashOf(value).mHash & mask;
   }
   
   /// Get the bucket index, based on the wrapped value's hash                
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash, wrapped in a block                 
   ///   @return the bucket index                                             
   LANGULUS(INLINED)
   Offset BlockSet::GetBucketUnknown(Offset mask, const Block& value) noexcept {
      return value.GetHash().mHash & mask;
   }

   /// Get a reference to a contained element                                 
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes T is similar to the contained type                
   ///   @tparam T - the type to reinterpret contained elements as            
   ///   @param i - the key index                                             
   ///   @return a constant reference to the element                          
   template<CT::Data T>
   LANGULUS(INLINED)
   constexpr T& BlockSet::GetRaw(Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing set value",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<T>(),
         "Wrong type when accessing set value",
         ", using type `", NameOf<T>(),
         "` instead of `", mKeys.GetType(), '`');
      return mKeys.template GetRawAs<T>()[i];
   }

   template<CT::Data T>
   LANGULUS(INLINED)
   constexpr const T& BlockSet::GetRaw(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockSet*>(this)->template GetRaw<T>(i);
   }

   /// Get an element handle                                                  
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes T is similar to the contained type                
   ///   @tparam T - the type to reinterpret contained elements as            
   ///   @param i - the key index                                             
   ///   @return the handle                                                   
   template<CT::Data T>
   LANGULUS(INLINED)
   constexpr Handle<T> BlockSet::GetHandle(Offset i) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing set value",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<T>(),
         "Wrong type when accessing set value",
         ", using type `", NameOf<T>(),
         "` instead of `", mKeys.GetType(), '`');
      return mKeys.template GetHandle<T>(i);
   }

} // namespace Langulus::Anyness
