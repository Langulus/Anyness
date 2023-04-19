///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet.hpp"

namespace Langulus::Anyness
{

   /// Get a valid key by any index, safely                                   
   ///   @param index - the index to use                                      
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::Get(const CT::Index auto& index) {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         OutOfRange, "Bad index");
      return GetValue(offset);
   }

   /// Get a valid key by any index, safely (const)                           
   ///   @param index - the index to use                                      
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::Get(const CT::Index auto& index) const {
      const auto offset = index.GetOffset();
      LANGULUS_ASSERT(offset < GetReserved() && GetInfo()[offset],
         OutOfRange, "Bad index");
      return GetValue(offset);
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
   Block BlockSet::GetValue(const Offset& i) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(), "Bad index");
      return mKeys.GetElement(i);
   }

   /// Get a raw key by an unsafe offset (const)                              
   ///   @attention assumes index is in container's limits                    
   ///   @param i - the offset to use                                         
   ///   @return the element, wrapped in a Block                              
   LANGULUS(INLINED)
   Block BlockSet::GetValue(const Offset& i) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(), "Bad index");
      return mKeys.GetElement(i);
   }

   /// Get the bucket index, based on the provided value's hash               
   ///   @param value - the value to hash                                     
   ///   @return the bucket index                                             
   LANGULUS(INLINED)
   Offset BlockSet::GetBucket(const CT::Data auto& value) const noexcept {
      return HashData(value).mHash & (GetReserved() - 1);
   }

   /// Get a mutable element reference                                        
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes T is exactly the contained type                   
   ///   @tparam T - the type to reinterpret contained elements as            
   ///   @param i - the key index                                             
   ///   @return a constant reference to the element                          
   template<CT::Data T>
   LANGULUS(INLINED)
   constexpr T& BlockSet::GetRaw(Offset i) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(), "Bad index");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsExact<T>(), "Bad type");
      return GetValues<T>().GetRaw()[i];
   }

   /// Get a constant element reference                                       
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes T is exactly the contained type                   
   ///   @tparam T - the type to reinterpret contained elements as            
   ///   @param i - the key index                                             
   ///   @return a constant reference to the element                          
   template<CT::Data T>
   LANGULUS(INLINED)
   constexpr const T& BlockSet::GetRaw(Offset i) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(), "Bad index");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsExact<T>(), "Bad type");
      return GetValues<T>().GetRaw()[i];
   }

   /// Get an element handle                                                  
   ///   @attention assumes index is in container's limits                    
   ///   @attention assumes T is exactly the contained type                   
   ///   @tparam T - the type to reinterpret contained elements as            
   ///   @param i - the key index                                             
   ///   @return the handle                                                   
   template<CT::Data T>
   LANGULUS(INLINED)
   constexpr Handle<T> BlockSet::GetHandle(Offset i) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, i < GetReserved(), "Bad index");
      LANGULUS_ASSUME(DevAssumes, mKeys.template IsExact<T>(), "Bad type");
      return GetValues<T>().GetHandle(i);
   }

} // namespace Langulus::Anyness
