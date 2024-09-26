///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
      return GetKeyRef<THIS>(idx);
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
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
      return GetValRef<THIS>(idx);
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
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

      const auto idx = SimplifyIndex<THIS>(index);
      if (not mInfo[idx])
         LANGULUS_OOPS(OutOfRange, "No pair at given index");

      return typename THIS::PairRef {
         GetKeyRef<THIS>(idx),
         GetValRef<THIS>(idx)
      };
   }
   
   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   auto BlockMap::GetPair(CT::Index auto index) const {
      return typename THIS::PairConstRef {
         const_cast<BlockMap*>(this)->template GetPair<THIS>(index)
      };
   }

   /// Get the bucket index, based on the provided value's hash               
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash                                     
   ///   @return the bucket index                                             
   LANGULUS(ALWAYS_INLINED)
   Offset BlockMap::GetBucket(Offset mask, const CT::NoIntent auto& value) noexcept {
      return HashOf(value).mHash & mask;
   }
   
   /// Get the bucket index, based on the wrapped value's hash                
   ///   @param mask - a mask for ANDing the relevant part of the hash        
   ///   @param value - the value to hash, wrapped in a block                 
   ///   @return the bucket index                                             
   LANGULUS(ALWAYS_INLINED)
   Offset BlockMap::GetBucketUnknown(Offset mask, const Block<>& value) noexcept {
      return value.GetHash().mHash & mask;
   }

   /// Get a key reference if THIS is typed, otherwise get a block            
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes K is similar to the contained key type            
   ///   @param i - the key index                                             
   ///   @return a reference to the key                                       
   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetRawKey(const Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing ", NameOf<THIS>(), " key",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::Typed<THIS>) {
         IF_SAFE(using K = typename THIS::Key);
         LANGULUS_ASSUME(DevAssumes, (IsKeySimilar<THIS, K>()),
            "Wrong type when accessing ", NameOf<THIS>(), " key",
            ", using type `", NameOf<K>(), "` instead of `", GetKeyType(), '`');
         return GetKeys<THIS>().GetRaw() + i;
      }
      else return GetKeys<THIS>().GetElementInner(i);
   }
   
   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   decltype(auto) BlockMap::GetRawKey(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawKey<THIS>(i);
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   decltype(auto) BlockMap::GetKeyRef(Offset i) IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>)
         return *GetRawKey<THIS>(i);
      else
         return GetRawKey<THIS>(i);
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   decltype(auto) BlockMap::GetKeyRef(Offset i) const IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>)
         return *GetRawKey<THIS>(i);
      else
         return GetRawKey<THIS>(i);
   }

   /// Get a value reference                                                  
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @param i - the value index                                           
   ///   @return a reference to the value                                     
   template<CT::Map THIS> LANGULUS(INLINED)
   decltype(auto) BlockMap::GetRawVal(const Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing ", NameOf<THIS>(), " value",
         ", index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::Typed<THIS>) {
         IF_SAFE(using V = typename THIS::Value);
         LANGULUS_ASSUME(DevAssumes, (IsValueSimilar<THIS, V>()),
            "Wrong type when accessing ", NameOf<THIS>(), " value",
            ", using type `", NameOf<V>(), "` instead of `", GetValueType(), '`');
         return GetVals<THIS>().GetRaw() + i;
      }
      else {
         // We use mReserved for different purpose in this map, so we   
         // have to compensate for that here                            
         auto e = GetVals<THIS>().GetElementInner(i);
         e.mReserved = mKeys.mReserved;
         return e;
      }
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   decltype(auto) BlockMap::GetRawVal(Offset i) const IF_UNSAFE(noexcept) {
      return const_cast<BlockMap*>(this)->template GetRawVal<THIS>(i);
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   decltype(auto) BlockMap::GetValRef(Offset i) IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>)
         return *GetRawVal<THIS>(i);
      else
         return  GetRawVal<THIS>(i);
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   decltype(auto) BlockMap::GetValRef(Offset i) const IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>)
         return *GetRawVal<THIS>(i);
      else
         return  GetRawVal<THIS>(i);
   }


   /// Get a key handle if THIS is typed, otherwise get a block               
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes K is similar to the contained key type            
   ///   @param i - the key index                                             
   ///   @return the handle                                                   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetKeyHandle(const Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing ", NameOf<THIS>(), " key, ",
         "index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::Typed<THIS>) {
         IF_SAFE(using K = typename THIS::Key);
         LANGULUS_ASSUME(DevAssumes, (IsKeySimilar<THIS, K>()),
            "Wrong type when accessing ", NameOf<THIS>(), " key, ",
            "using type `", NameOf<K>(), "` instead of `", GetKeyType(), '`');
         return GetKeys<THIS>().GetHandle(i);
      }
      else return GetKeys<THIS>().GetElementInner(i);
   }
   
   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   auto BlockMap::GetKeyHandle(const Offset i) const IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>) {
         return const_cast<BlockMap*>(this)->template
            GetKeyHandle<THIS>(i).MakeConst();
      }
      else {
         auto block = const_cast<BlockMap*>(this)->template
            GetKeyHandle<THIS>(i);
         block.MakeConst();
         return block;
      }
   }
   
   /// Get a value handle if THIS is typed, otherwise get a block             
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes V is similar to the contained value type          
   ///   @param i - the value index                                           
   ///   @return the handle                                                   
   template<CT::Map THIS> LANGULUS(INLINED)
   auto BlockMap::GetValHandle(const Offset i) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(),
         "Index out of limits when accessing ", NameOf<THIS>(), " value, ",
         "index ", i, " is beyond the reserved ", GetReserved(), " elements");

      if constexpr (CT::Typed<THIS>) {
         using V = typename THIS::Value;
         LANGULUS_ASSUME(DevAssumes, (IsValueSimilar<THIS, V>()),
            "Wrong type when accessing ", NameOf<THIS>(), " value, ",
            "using type `", NameOf<V>(), "` instead of `", GetValueType(), '`');

         // We can't rely on Block::GetHandle, because it uses mReserved
         if constexpr (CT::Sparse<V>) {
            return Handle<V> {
               mValues.template GetRaw<V>() + i,
               mValues.template GetRaw<const Allocation*>() + mKeys.mReserved + i,
               //reinterpret_cast<const Allocation**>(mValues.mRawSparse) + mKeys.mReserved + i
            };
         }
         else return Handle<V> {
            mValues.template GetRaw<V>() + i,
            mValues.mEntry
         };
      }
      else {
         // We use mReserved for different purpose in this map, so we   
         // have to compensate for that here                            
         auto e = GetVals<THIS>().GetElementInner(i);
         e.mReserved = mKeys.mReserved;
         return e;
      }
   }

   template<CT::Map THIS> LANGULUS(ALWAYS_INLINED)
   auto BlockMap::GetValHandle(const Offset i) const IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>) {
         return const_cast<BlockMap*>(this)->template
            GetValHandle<THIS>(i).MakeConst();
      }
      else {
         auto block = const_cast<BlockMap*>(this)->template
            GetValHandle<THIS>(i);
         block.MakeConst();
         return block;
      }
   }

} // namespace Langulus::Anyness
