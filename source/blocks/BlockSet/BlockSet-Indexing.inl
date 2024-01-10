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
   template<CT::Set THIS> LANGULUS(INLINED)
   decltype(auto) BlockSet::Get(const CT::Index auto index) {
      if (IsEmpty())
         LANGULUS_OOPS(OutOfRange, "Set is empty");

      const auto idx = SimplifyIndex<THIS>(index);
      if (not mInfo[idx])
         LANGULUS_OOPS(OutOfRange, "No element at given index");
      return GetRaw<THIS>(idx);
   }

   template<CT::Set THIS> LANGULUS(INLINED)
   decltype(auto) BlockSet::Get(const CT::Index auto index) const {
      return const_cast<BlockSet*>(this)->template Get<THIS>(index);
   }

   /// Get a raw key by an unsafe offset                                      
   ///   @attention assumes index is in container's limits                    
   ///   @param i - the offset to use                                         
   ///   @return the element, wrapped in a Block                              
   template<CT::Set THIS> LANGULUS(INLINED)
   decltype(auto) BlockSet::GetRaw(const Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing set",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::TypedSet<THIS>) {
         using K = TypeOf<THIS>;
         LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
            "Wrong type when accessing set",
            ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
         return GetValues<THIS>().GetRaw()[i];
      }
      else return GetValues<THIS>().GetElement(i);
   }

   template<CT::Set THIS> LANGULUS(INLINED)
   decltype(auto) BlockSet::GetRaw(const Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockSet*>(this)->template GetRaw<THIS>(i);
   }

   /// Get the bucket index, based on the provided value's hash               
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash                                     
   ///   @return the bucket index                                             
   LANGULUS(INLINED)
   Offset BlockSet::GetBucket(const Offset mask, const CT::NotSemantic auto& value) noexcept {
      return HashOf(value).mHash & mask;
   }
   
   /// Get the bucket index, based on the wrapped value's hash                
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash, wrapped in a block                 
   ///   @return the bucket index                                             
   LANGULUS(INLINED)
   Offset BlockSet::GetBucketUnknown(const Offset mask, const Block& value) noexcept {
      return value.GetHash().mHash & mask;
   }

   /// Get an element handle                                                  
   ///   @attention assumes index is in container's limits                    
   ///   @param i - the key index                                             
   ///   @return the handle                                                   
   template<CT::Set THIS> LANGULUS(INLINED)
   auto BlockSet::GetHandle(const Offset i) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::TypedSet<THIS>) {
         using K = TypeOf<THIS>;
         LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
            "Wrong type when accessing map key",
            ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
         return GetValues<THIS>().GetHandle(i);
      }
      else return GetValues<THIS>().GetElement(i);
   }

} // namespace Langulus::Anyness
