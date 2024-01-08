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

   /// Convert an index to an offset                                          
   /// Complex indices will be fully constrained                              
   /// Unsigned/signed integers are directly forwarded without any overhead   
   ///   @attention assumes index is in container reserve limit, if integer   
   ///   @param index - the index to simplify                                 
   ///   @return the simplified index, as a simple offset                     
   template<CT::Map THIS, CT::Index INDEX> LANGULUS(INLINED)
   Offset BlockMap::SimplifyIndex(const INDEX index) const
   noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>) {
      if constexpr (CT::Same<INDEX, Index>) {
         // This is the most safe path, throws on errors, but slow      
         if constexpr (CT::Typed<THIS>) {
            const auto result = index.Constrained(GetReserved());

            if (result == IndexBiggest) {
               TODO();
            }
            else if (result == IndexSmallest) {
               TODO();
            }

            return result.GetOffset();
         }
         else return index.Constrained(GetReserved()).GetOffset();
      }
      else {
         // Unsafe, works only on assumptions                           
         // Using an integer index explicitly makes a statement, that   
         // you know what you're doing                                  
         LANGULUS_ASSUME(UserAssumes,
            static_cast<Count>(index) < GetReserved(),
            "Integer index out of range");

         if constexpr (CT::Signed<INDEX>) {
            LANGULUS_ASSUME(UserAssumes, index >= 0, 
               "Integer index is below zero, "
               "use Index for reverse indices instead"
            );
         }

         return index;
      }
   }

   /// Get a key by a safe index                                              
   ///   @param index - the index to use                                      
   ///   @return a reference to the key                                       
   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetKey(CT::Index auto index) {
      if (IsEmpty())
         LANGULUS_OOPS(OutOfRange, "Map is empty");

      const auto idx = SimplifyIndex<THIS>(index);
      if (not mInfo[idx])
         LANGULUS_OOPS(OutOfRange, "No pair at given index");
      return GetRawKey<THIS>(idx);
   }

   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetKey(CT::Index auto index) const {
      return const_cast<BlockMap*>(this)->template GetKey<THIS>(index);
   }

   /// Get a value by a safe index                                            
   ///   @param index - the index to use                                      
   ///   @return a reference to the value                                     
   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetValue(CT::Index auto index) {
      if (IsEmpty())
         LANGULUS_OOPS(OutOfRange, "Map is empty");

      const auto idx = SimplifyIndex<THIS>(index);
      if (not mInfo[idx])
         LANGULUS_OOPS(OutOfRange, "No pair at given index");
      return GetRawValue<THIS>(idx);
   }

   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetValue(CT::Index auto index) const {
      return const_cast<BlockMap&>(*this).template GetValue<THIS>(index);
   }

   /// Get a pair by a safe index                                             
   ///   @param index - the index to use                                      
   ///   @return the pair                                                     
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetPair(CT::Index auto index) {
      if (IsEmpty())
         LANGULUS_OOPS(OutOfRange, "Map is empty");

      auto idx = SimplifyIndex<THIS>(index);
      if (not mInfo[idx])
         LANGULUS_OOPS(OutOfRange, "No pair at given index");

      return typename THIS::PairRef {
         GetRawKey  <THIS>(idx),
         GetRawValue<THIS>(idx)
      };
   }
   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetPair(CT::Index auto index) const {
      return typename THIS::PairConstRef {
         const_cast<BlockMap*>(this)->template GetPair<THIS>(index)
      };
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

   /// Get a key reference if THIS is typed, otherwise get a block            
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes K is similar to the contained key type            
   ///   @param i - the key index                                             
   ///   @return a reference to the key                                       
   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetRawKey(Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::TypedMap<THIS>) {
         using K = typename THIS::Key;
         LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
            "Wrong type when accessing map key",
            ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
         return GetKeys<THIS>().GetRaw()[i];
      }
      else return GetKeys<THIS>().GetElement(i);
   }
   
   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetRawKey(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawKey<THIS>(i);
   }

   /// Get a value reference                                                  
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @param i - the value index                                           
   ///   @return a reference to the value                                     
   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetRawValue(Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map value",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::TypedMap<THIS>) {
         using V = typename THIS::Value;
         LANGULUS_ASSUME(DevAssumes, mValues.template IsSimilar<V>(),
            "Wrong type when accessing map value",
            ", using type `", NameOf<V>(), "` instead of `", mValues.GetType(), '`');
         return GetValues<THIS>().GetRaw()[i];
      }
      else return GetValues<THIS>().GetElement(i);
   }

   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetRawValue(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawValue<THIS>(i);
   }

   /// Get a key handle if THIS is typed, otherwise get a block               
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes K is similar to the contained key type            
   ///   @param i - the key index                                             
   ///   @return the handle                                                   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetKeyHandle(Offset i) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::TypedMap<THIS>) {
         using K = typename THIS::Key;
         LANGULUS_ASSUME(DevAssumes, mKeys.template IsSimilar<K>(),
            "Wrong type when accessing map key",
            ", using type `", NameOf<K>(), "` instead of `", mKeys.GetType(), '`');
         return GetKeys<THIS>().GetHandle(i);
      }
      else return GetKeys<THIS>().GetElement(i);
   }
   
   /// Get a value handle if THIS is typed, otherwise get a block             
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @param i - the value index                                           
   ///   @return the handle                                                   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetValueHandle(Offset i) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing map value",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::TypedMap<THIS>) {
         using V = typename THIS::Value;
         LANGULUS_ASSUME(DevAssumes, mValues.template IsSimilar<V>(),
            "Wrong type when accessing map value",
            ", using type `", NameOf<V>(), "` instead of `", mValues.GetType(), '`');
         return GetValues<THIS>().GetHandle(i);
      }
      else return GetValues<THIS>().GetElement(i);
   }

} // namespace Langulus::Anyness
