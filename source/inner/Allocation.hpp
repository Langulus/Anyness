///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <RTTI/MetaData.hpp>
#include <RTTI/MetaTrait.hpp>
#include <RTTI/MetaVerb.hpp>


namespace Langulus::Anyness
{

   using RTTI::AllocationRequest;
   using RTTI::MetaData;
   using RTTI::MetaConst;
   using RTTI::MetaTrait;
   using RTTI::DMeta;
   using RTTI::CMeta;
   using RTTI::TMeta;

   using Pool = void;
   
   template<class T>
   concept AllocationPrimitive = requires(T a) { 
      {T::GetNewAllocationSize(Size {})} -> CT::Same<Size>;
   };


   ///                                                                        
   ///   Memory allocation                                                    
   ///                                                                        
   /// This is a single allocation record                                     
   ///                                                                        
   struct Allocation final {
   IF_LANGULUS_MANAGED_MEMORY(friend class Pool);
   friend struct Allocator;
   protected:
      // Allocated bytes for this chunk                                 
      Size mAllocatedBytes;
      // The number of references to this memory                        
      Count mReferences;
      union {
         // This pointer has two uses, depending on mReferences         
         // If mReferences > 0, it refers to the pool that owns the     
         //    allocation, or	handle for std::free() if MANAGED_MEMORY  
         //    feature is not enabled                                   
         // If mReferences == 0, it refers to the next free entry to be 
         //    reused                                                   
         Pool* mPool;
         Allocation* mNextFreeEntry;
      };

   public:
      Allocation() = delete;
      Allocation(const Allocation&) = delete;
      Allocation(Allocation&&) = delete;
      ~Allocation() = delete;

      constexpr Allocation(const Size&, Pool*) noexcept;

      NOD() static constexpr Size GetSize() noexcept;
      NOD() static constexpr Size GetNewAllocationSize(const Size&) noexcept;
      NOD() static constexpr Size GetMinAllocation() noexcept;

      NOD() constexpr const Count& GetUses() const noexcept;
      NOD() const Byte* GetBlockStart() const noexcept;
      NOD() const Byte* GetBlockEnd() const noexcept;
      NOD() Byte* GetBlockStart() noexcept;
      NOD() constexpr Size GetTotalSize() const noexcept;
      NOD() constexpr const Size& GetAllocatedSize() const noexcept;
      NOD() bool Contains(const void*) const noexcept;
      NOD() bool CollisionFree(const Allocation&) const noexcept;

      template<class T>
      NOD() T* As() const noexcept;

      constexpr void Keep() noexcept;
      constexpr void Keep(const Count&) noexcept;
      constexpr void Free() noexcept;
      constexpr void Free(const Count&) noexcept;
   };

} // namespace Langulus::Anyness

#include "Allocation.inl"