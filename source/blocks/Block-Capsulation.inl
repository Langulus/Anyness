///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block.hpp"

namespace Langulus::Anyness
{
   
   /// Overwrite the current data state                                       
   ///   @attention you can not add/remove constraints like that              
   ///   @param state - the state to overwrite with                           
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::SetState(DataState state) noexcept {
      mState = state - DataState::Constrained;
   }

   /// Add a state                                                            
   ///   @attention you can not add constraints like that                     
   ///   @param state - the state to add to the current                       
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::AddState(DataState state) noexcept {
      mState += state - DataState::Constrained;
   }

   /// Remove a state                                                         
   ///   @attention you can not remove constraints like that                  
   ///   @param state - the state to remove from the current                  
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::RemoveState(DataState state) noexcept {
      mState -= state - DataState::Constrained;
   }

   /// Check if a pointer is anywhere inside the block's reserved memory      
   ///   @attention doesn't check deep or sparse data regions                 
   ///   @param ptr - the pointer to check                                    
   ///   @return true if inside the immediate reserved memory block range     
   LANGULUS(ALWAYSINLINE)
   bool Block::Owns(const void* ptr) const noexcept {
      return ptr >= GetRaw() && ptr < GetRaw() + GetReservedSize();
   }

   /// Check if we have jurisdiction over the contained memory                
   ///   @return true if memory is under our authority                        
   constexpr bool Block::HasAuthority() const noexcept {
      return mEntry != nullptr;
   }

   /// Get the number of references for the allocated memory block            
   ///   @return the references for the memory block, or 0 if memory is       
   ///           outside authority (or unallocated)                           
   LANGULUS(ALWAYSINLINE)
   constexpr Count Block::GetUses() const noexcept {
      return mEntry ? mEntry->GetUses() : 0;
   }
   
   /// Get the contained type                                                 
   ///   @return the meta data                                                
   LANGULUS(ALWAYSINLINE)
   constexpr DMeta Block::GetType() const noexcept {
      return mType;
   }

   /// Get the number of initialized elements                                 
   ///   @return the number of initialized elements                           
   LANGULUS(ALWAYSINLINE)
   constexpr Count Block::GetCount() const noexcept {
      return mCount;
   }

   /// Get the number of reserved (maybe uninitialized) elements              
   ///   @return the number of reserved (maybe uninitialized) elements        
   LANGULUS(ALWAYSINLINE)
   constexpr Count Block::GetReserved() const noexcept {
      return mReserved;
   }
   
   /// Get the number of reserved bytes                                       
   ///   @attention this doesn't include bytes reserved for entries in sparse 
   ///              containers, when managed memory is enabled                
   ///   @return the number of reserved bytes                                 
   LANGULUS(ALWAYSINLINE)
   constexpr Size Block::GetReservedSize() const noexcept {
      return mType ? mReserved * mType->mSize : 0;
   }
   
   /// Get the number of sub-blocks (this one included)                       
   ///   @return the number of contained blocks, including this one           
   inline Count Block::GetCountDeep() const noexcept {
      Count counter {1};
      ForEach([&counter](const Block& block) noexcept {
         counter += block.GetCountDeep();
      });
      return counter;
   }

   /// Get the sum of initialized non-deep elements in all sub-blocks         
   ///   @return the number of contained non-deep elements                    
   inline Count Block::GetCountElementsDeep() const noexcept {
      if (!mType)
         return 0;
      if (!IsDeep())
         return mCount;

      Count counter {};
      ForEach([&counter](const Block& block) noexcept {
         counter += block.GetCountElementsDeep();
      });
      return counter;
   }

} // namespace Langulus::Anyness