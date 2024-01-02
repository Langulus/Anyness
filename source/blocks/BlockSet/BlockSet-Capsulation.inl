///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet-Iteration.inl"


namespace Langulus::Anyness
{
   
   /// Templated tables are always typed                                      
   ///   @return false                                                        
   LANGULUS(INLINED)
   constexpr bool BlockSet::IsUntyped() const noexcept {
      return mKeys.IsUntyped();
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   LANGULUS(INLINED)
   constexpr bool BlockSet::IsTypeConstrained() const noexcept {
      return mKeys.IsTypeConstrained();
   }

   /// Check if key type is deep                                              
   LANGULUS(INLINED)
   constexpr bool BlockSet::IsDeep() const noexcept {
      return mKeys.IsDeep();
   }

   /// Check if the key type is a pointer                                     
   LANGULUS(INLINED)
   constexpr bool BlockSet::IsSparse() const noexcept {
      return mKeys.IsSparse();
   }

   /// Check if the key type is not a pointer                                 
   LANGULUS(INLINED)
   constexpr bool BlockSet::IsDense() const noexcept {
      return mKeys.IsDense();
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   LANGULUS(INLINED)
   constexpr Size BlockSet::GetStride() const noexcept {
      return mKeys.GetStride();
   }

   /// Get the size of all elements, in bytes                                 
   ///   @return the total amount of initialized bytes                        
   LANGULUS(INLINED)
   constexpr Size BlockSet::GetBytesize() const noexcept {
      return GetStride() * GetCount(); 
   }

   /// Get the type of the set                                                
   LANGULUS(INLINED)
   DMeta BlockSet::GetType() const noexcept {
      return mKeys.GetType();
   }

   /// Get the info array (const)                                             
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(INLINED)
   const BlockSet::InfoType* BlockSet::GetInfo() const noexcept {
      return mInfo;
   }

   /// Get the info array                                                     
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(INLINED)
   BlockSet::InfoType* BlockSet::GetInfo() noexcept {
      return mInfo;
   }

   /// Get the end of the info array                                          
   ///   @return a pointer to the first element inside the info array         
   LANGULUS(INLINED)
   const BlockSet::InfoType* BlockSet::GetInfoEnd() const noexcept {
      return mInfo + GetReserved();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data T> LANGULUS(INLINED)
   const TAny<T>& BlockSet::GetValues() const noexcept {
      return reinterpret_cast<const TAny<T>&>(mKeys);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   template<CT::Data T> LANGULUS(INLINED)
   TAny<T>& BlockSet::GetValues() noexcept {
      return reinterpret_cast<TAny<T>&>(mKeys);
   }

   /// Get the number of inserted pairs                                       
   ///   @return the number of inserted pairs                                 
   LANGULUS(INLINED)
   constexpr Count BlockSet::GetCount() const noexcept {
      return mKeys.GetCount();
   }

   /// Get the number of allocated pairs                                      
   ///   @return the number of allocated pairs                                
   LANGULUS(INLINED)
   constexpr Count BlockSet::GetReserved() const noexcept {
      return mKeys.GetReserved();
   }

   /// Check if there are any pairs in this map                               
   ///   @return true if there's at least one pair available                  
   LANGULUS(INLINED)
   constexpr bool BlockSet::IsEmpty() const noexcept {
      return mKeys.IsEmpty();
   }

   /// Check if the map has been allocated                                    
   ///   @return true if the map uses dynamic memory                          
   LANGULUS(INLINED)
   constexpr bool BlockSet::IsAllocated() const noexcept {
      return mKeys.IsAllocated();
   }

   /// Check if the set is marked missing                                     
   ///   @return true if the set is marked as missing                         
   LANGULUS(INLINED)
   bool BlockSet::IsMissing() const noexcept {
      return mKeys.IsMissing();
   }
   
   /// Check if the set contains at least one missing entry (nested)          
   ///   @return true if the set has missing entries                          
   LANGULUS(INLINED)
   bool BlockSet::IsMissingDeep() const {
      bool missing = false;
      ForEachDeep([&](const Block& value) {
         missing = value.IsMissing();
         return not missing;
      });

      return missing;
   }

   /// Check if the memory for the table is owned by us                       
   /// This is always true, since the map can't be initialized with outside   
   /// memory - the memory layout requirements are too strict to allow for it 
   ///   @return true                                                         
   LANGULUS(INLINED)
   constexpr bool BlockSet::HasAuthority() const noexcept {
      return IsAllocated();
   }

   /// Get the number of references for the allocated memory                  
   ///   @attention always returns zero if we don't have authority            
   ///   @return the number of references                                     
   LANGULUS(INLINED)
   constexpr Count BlockSet::GetUses() const noexcept {
      return mKeys.GetUses();
   }

#if LANGULUS(TESTING)
   /// Get raw key memory pointer, used only in testing                       
   ///   @return the pointer                                                  
   LANGULUS(INLINED)
   constexpr const void* BlockSet::GetRawMemory() const noexcept {
      return mKeys.mRaw;
   }

   /// Get allocation, used only in testing                                   
   ///   @return the allocation entry                                         
   LANGULUS(INLINED)
   const Allocation* BlockSet::GetEntry() const noexcept {
      return mKeys.mEntry;
   }
#endif
      
   /// Explicit bool cast operator, for use in if statements                  
   ///   @return true if block contains at least one valid element            
   LANGULUS(INLINED)
   constexpr BlockSet::operator bool() const noexcept {
      return not IsEmpty();
   }

#if LANGULUS(DEBUG)
   inline void BlockSet::Dump() const {
      Logger::Info("---------------- BlockSet::Dump start ----------------");
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         const auto index = info - GetInfo();
         if (*info)
            Logger::Info('[', index, "] -", (*info-1), " -> ", GetInner(index).GetHash().mHash);
         else
            Logger::Info('[', index, "] empty");

         ++info;
      }
      Logger::Info("----------------  BlockSet::Dump end  ----------------");
   }
#endif

} // namespace Langulus::Anyness
