///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"

namespace Langulus::Anyness
{
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyUntyped() const noexcept {
      return mKeys.IsUntyped();
   }
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueUntyped() const noexcept {
      return mValues.IsUntyped();
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyTypeConstrained() const noexcept {
      return mKeys.IsTypeConstrained();;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueTypeConstrained() const noexcept {
      return mValues.IsTypeConstrained();;
   }
   
   /// Check if key type is abstract                                          
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyAbstract() const noexcept {
      return mKeys.IsAbstract() && mKeys.IsDense();
   }
   
   /// Check if value type is abstract                                        
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueAbstract() const noexcept {
      return mValues.IsAbstract() && mKeys.IsDense();
   }
   
   /// Check if key type is default-constructible                             
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyConstructible() const noexcept {
      return mKeys.IsDefaultable();
   }
   
   /// Check if value type is default-constructible                           
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueConstructible() const noexcept {
      return mValues.IsDefaultable();
   }
   
   /// Check if key type is deep                                              
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyDeep() const noexcept {
      return mKeys.IsDeep();
   }
   
   /// Check if value type is deep                                            
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueDeep() const noexcept {
      return mValues.IsDeep();
   }

   /// Check if the key type is a pointer                                     
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeySparse() const noexcept {
      return mKeys.IsSparse();
   }
   
   /// Check if the value type is a pointer                                   
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueSparse() const noexcept {
      return mValues.IsSparse();
   }

   /// Check if the key type is not a pointer                                 
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyDense() const noexcept {
      return mKeys.IsDense();
   }

   /// Check if the value type is not a pointer                               
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueDense() const noexcept {
      return mValues.IsDense();
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   LANGULUS(INLINED)
   constexpr Size BlockMap::GetKeyStride() const noexcept {
      return mKeys.GetStride();
   }
   
   /// Get the size of a single value, in bytes                               
   ///   @return the number of bytes a single value contains                  
   LANGULUS(INLINED)
   constexpr Size BlockMap::GetValueStride() const noexcept {
      return mValues.GetStride();
   }

   #ifdef LANGULUS_ENABLE_TESTING
      /// Get raw key memory pointer, used only in testing                    
      ///   @return the pointer                                               
      LANGULUS(INLINED)
      constexpr const void* BlockMap::GetRawKeysMemory() const noexcept {
         return mKeys.mRaw;
      }

      /// Get raw value memory pointer, used only in testing                  
      ///   @return the pointer                                               
      LANGULUS(INLINED)
      constexpr const void* BlockMap::GetRawValuesMemory() const noexcept {
         return mValues.mRaw;
      }
   #endif

   /// Get the size of all pairs, in bytes                                    
   ///   @return the total amount of initialized bytes                        
   LANGULUS(INLINED)
   constexpr Size BlockMap::GetByteSize() const noexcept {
      return sizeof(Pair) * GetCount(); 
   }

   /// Get the key meta data                                                  
   LANGULUS(INLINED)
   DMeta BlockMap::GetKeyType() const noexcept {
      return mKeys.GetType();
   }

   /// Get the value meta data                                                
   LANGULUS(INLINED)
   DMeta BlockMap::GetValueType() const noexcept {
      return mValues.GetType();
   }

   /// Get the info array (const)                                             
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(INLINED)
   const BlockMap::InfoType* BlockMap::GetInfo() const noexcept {
      return mInfo;
   }

   /// Get the info array                                                     
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(INLINED)
   BlockMap::InfoType* BlockMap::GetInfo() noexcept {
      return mInfo;
   }

   /// Get the end of the info array                                          
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(INLINED)
   const BlockMap::InfoType* BlockMap::GetInfoEnd() const noexcept {
      return mInfo + GetReserved();
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data K>
   LANGULUS(INLINED)
   const TAny<K>& BlockMap::GetKeys() const noexcept {
      return reinterpret_cast<const TAny<K>&>(mKeys);
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data K>
   LANGULUS(INLINED)
   TAny<K>& BlockMap::GetKeys() noexcept {
      return reinterpret_cast<TAny<K>&>(mKeys);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data V>
   LANGULUS(INLINED)
   const TAny<V>& BlockMap::GetValues() const noexcept {
      return reinterpret_cast<const TAny<V>&>(mValues);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data V>
   LANGULUS(INLINED)
   TAny<V>& BlockMap::GetValues() noexcept {
      return reinterpret_cast<TAny<V>&>(mValues);
   }

   /// Get the number of inserted pairs                                       
   ///   @return the number of inserted pairs                                 
   LANGULUS(INLINED)
   constexpr Count BlockMap::GetCount() const noexcept {
      return mValues.GetCount();
   }

   /// Get the number of allocated pairs                                      
   ///   @return the number of allocated pairs                                
   LANGULUS(INLINED)
   constexpr Count BlockMap::GetReserved() const noexcept {
      return mValues.GetReserved();
   }

   /// Check if there are any pairs in this map                               
   ///   @return true if there's at least one pair available                  
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsEmpty() const noexcept {
      return mValues.IsEmpty();
   }

   /// Check if the map has been allocated                                    
   ///   @return true if the map uses dynamic memory                          
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsAllocated() const noexcept {
      return mValues.IsAllocated();
   }

   /// Check if the memory for the table is owned by us                       
   /// This is always true, since the map can't be initialized with outside   
   /// memory - the memory layout requirements are too strict to allow for it 
   ///   @return true                                                         
   LANGULUS(INLINED)
   constexpr bool BlockMap::HasAuthority() const noexcept {
      return IsAllocated();
   }

   /// Get the number of references for the allocated memory                  
   ///   @attention always returns zero if we don't have authority            
   ///   @return the number of references                                     
   LANGULUS(INLINED)
   constexpr Count BlockMap::GetUses() const noexcept {
      return mValues.GetUses();
   }
   
#ifdef LANGULUS_ENABLE_DEBUGGING
   inline void BlockMap::Dump() const {
      Logger::Info("---------------- BlockMap::Dump start ----------------");
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         const auto index = info - GetInfo();
         if (*info) {
            Logger::Info('[', index, "] -", (*info - 1), " -> ",
               GetKeyInner(index).GetHash().mHash, " | ",
               GetValueInner(index).GetHash().mHash
            );
         }
         else Logger::Info('[', index, "] empty");

         ++info;
      }
      Logger::Info("----------------  BlockMap::Dump end  ----------------");
   }
#endif

} // namespace Langulus::Anyness
