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
#include "BlockMap-Indexing.inl"


namespace Langulus::Anyness
{
   
   /// Check if map has its key type set                                      
   ///   @return true if type is available                                    
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyUntyped() const noexcept {
      return GetKeys<THIS>().IsUntyped();
   }
   
   /// Check if map has its value type set                                    
   ///   @return true if type is available                                    
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueUntyped() const noexcept {
      return GetVals<THIS>().IsUntyped();
   }
   
   /// Check if map has its key type-constrained                              
   ///   @return true if type is constrained                                  
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyTypeConstrained() const noexcept {
      return GetKeys<THIS>().IsTypeConstrained();
   }
   
   /// Check if map has its value type-constrained                            
   ///   @return true if type is constrained                                  
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueTypeConstrained() const noexcept {
      return GetVals<THIS>().IsTypeConstrained();
   }
   
   /// Check if key type is deep                                              
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyDeep() const noexcept {
      return GetKeys<THIS>().IsDeep();
   }
   
   /// Check if value type is deep                                            
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueDeep() const noexcept {
      return GetVals<THIS>().IsDeep();
   }

   /// Check if the key type is a pointer                                     
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeySparse() const noexcept {
      return GetKeys<THIS>().IsSparse();
   }
   
   /// Check if the value type is a pointer                                   
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueSparse() const noexcept {
      return GetVals<THIS>().IsSparse();
   }

   /// Check if the key type is not a pointer                                 
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsKeyDense() const noexcept {
      return GetKeys<THIS>().IsDense();
   }

   /// Check if the value type is not a pointer                               
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr bool BlockMap::IsValueDense() const noexcept {
      return GetVals<THIS>().IsDense();
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr Size BlockMap::GetKeyStride() const noexcept {
      return GetKeys<THIS>().GetStride();
   }
   
   /// Get the size of a single value, in bytes                               
   ///   @return the number of bytes a single value contains                  
   template<CT::Map THIS> LANGULUS(INLINED)
   constexpr Size BlockMap::GetValueStride() const noexcept {
      return GetVals<THIS>().GetStride();
   }

   #if LANGULUS(TESTING)
      /// Get raw key memory pointer, used only in testing                    
      ///   @return the pointer                                               
      LANGULUS(INLINED)
      constexpr const void* BlockMap::GetRawKeysMemory() const noexcept {
         return mKeys.mRaw;
      }

      /// Get raw value memory pointer, used only in testing                  
      ///   @return the pointer                                               
      LANGULUS(INLINED)
      constexpr const void* BlockMap::GetRawValsMemory() const noexcept {
         return mValues.mRaw;
      }
   #endif

   /// Get the key type                                                       
   ///   @return the key type definition                                      
   template<CT::Map THIS> LANGULUS(INLINED)
   DMeta BlockMap::GetKeyType() const noexcept {
      return GetKeys<THIS>().GetType();
   }

   /// Get the value type                                                     
   ///   @return the value type definition                                    
   template<CT::Map THIS> LANGULUS(INLINED)
   DMeta BlockMap::GetValueType() const noexcept {
      return GetVals<THIS>().GetType();
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

   /// Get the key container                                                  
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetKeys() const noexcept {
      if constexpr (CT::Typed<THIS>)
         return reinterpret_cast<const TAny<typename THIS::Key>&>(mKeys);
      else
         return reinterpret_cast<const Any&>(mKeys);
   }

   /// Get the templated key container                                        
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetKeys() noexcept {
      if constexpr (CT::Typed<THIS>)
         return reinterpret_cast<TAny<typename THIS::Key>&>(mKeys);
      else
         return reinterpret_cast<Any&>(mKeys);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetVals() const noexcept {
      if constexpr (CT::Typed<THIS>)
         return reinterpret_cast<const TAny<typename THIS::Value>&>(mValues);
      else
         return reinterpret_cast<const Any&>(mValues);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Map THIS> LANGULUS(INLINED)
   auto& BlockMap::GetVals() noexcept {
      if constexpr (CT::Typed<THIS>)
         return reinterpret_cast<TAny<typename THIS::Value>&>(mValues);
      else
         return reinterpret_cast<Any&>(mValues);
   }

   /// Get the number of inserted pairs                                       
   ///   @return the number of inserted pairs                                 
   LANGULUS(INLINED)
   constexpr Count BlockMap::GetCount() const noexcept {
      return mKeys.mCount;
   }

   /// Get the number of deep key containers                                  
   ///   @return the number of deep key containers                            
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::GetKeyCountDeep() const noexcept {
      return GetCountDeep(GetKeys<THIS>());
   }

   /// Get the number of deep key containers                                  
   ///   @return the number of deep key containers                            
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::GetKeyCountElementsDeep() const noexcept {
      return GetCountElementsDeep(GetKeys<THIS>());
   }

   /// Get the number of deep key containers                                  
   ///   @return the number of deep key containers                            
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::GetValueCountDeep() const noexcept {
      return GetCountDeep(GetVals<THIS>());
   }

   /// Get the number of deep key containers                                  
   ///   @return the number of deep key containers                            
   template<CT::Map THIS> LANGULUS(INLINED)
   Count BlockMap::GetValueCountElementsDeep() const noexcept {
      return GetCountElementsDeep(GetVals<THIS>());
   }

   /// Inner function, for counting nested containers in key or value blocks  
   ///   @param what - the block to scan                                      
   ///   @return the number of found blocks                                   
   Count BlockMap::GetCountDeep(const CT::Block auto& what) const noexcept {
      if (IsEmpty() or not what.IsDeep())
         return 1;

      Count counter = 1;
      auto data = what.template GetRawAs<Block>();
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info)
            counter += (data + (info - GetInfo()))->GetCountDeep();
         ++info;
      }
      return counter;
   }

   /// Inner function, for counting nested elements in key or value blocks    
   ///   @param what - the block to scan                                      
   ///   @return the number of found blocks                                   
   Count BlockMap::GetCountElementsDeep(const CT::Block auto& what) const noexcept {
      if (IsEmpty() or what.IsUntyped())
         return 0;

      if (not what.IsDeep())
         return GetCount();

      Count counter = 0;
      auto data = what.template GetRawAs<Block>();
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info)
            counter += (data + (info - GetInfo()))->GetCountElementsDeep();
         ++info;
      }

      return counter;
   }

   /// Get the number of allocated pairs                                      
   ///   @return the number of allocated pairs                                
   LANGULUS(INLINED)
   constexpr Count BlockMap::GetReserved() const noexcept {
      return mKeys.mReserved;
   }

   /// Check if there are any pairs in this map                               
   ///   @return true if there's at least one pair available                  
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsEmpty() const noexcept {
      return mKeys.IsEmpty();
   }

   /// Check if the map has been allocated                                    
   ///   @return true if the map uses dynamic memory                          
   LANGULUS(INLINED)
   constexpr bool BlockMap::IsAllocated() const noexcept {
      return mKeys.IsAllocated();
   }
   
   /// Check if keys or values are marked missing                             
   ///   @return true if the map is marked missing                            
   LANGULUS(INLINED)
   bool BlockMap::IsMissing() const noexcept {
      return mKeys.IsMissing() or mValues.IsMissing();
   }
   
   /// Check if the map contains at least one missing entry (nested)          
   ///   @return true if the map has missing entries                          
   template<CT::Map THIS> LANGULUS(INLINED)
   bool BlockMap::IsMissingDeep() const {
      if (IsMissing())
         return true;

      bool missing = false;
      ForEachKeyDeep<THIS>([&](const Block& key) {
         return not (missing = key.IsMissing());
      });
      ForEachValueDeep<THIS>([&](const Block& val) {
         return not (missing = val.IsMissing());
      });
      return missing;
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
      return mKeys.GetUses();
   }
   
   /// Explicit bool cast operator, for use in if statements                  
   ///   @return true if block contains at least one valid element            
   LANGULUS(INLINED)
   constexpr BlockMap::operator bool() const noexcept {
      return not IsEmpty();
   }

#if LANGULUS(DEBUG)
   template<CT::Map THIS>
   void BlockMap::Dump() const {
      Logger::Info("---------------- BlockMap::Dump start ----------------");
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         const auto index = info - GetInfo();
         if (*info) {
            Logger::Info('[', index, "] -", (*info - 1), " -> ",
               HashOf(GetKeyRef<THIS>(index)), " | ",
               HashOf(GetValRef<THIS>(index))
            );
         }
         else Logger::Info('[', index, "] empty");

         ++info;
      }
      Logger::Info("----------------  BlockMap::Dump end  ----------------");
   }
#endif

} // namespace Langulus::Anyness
