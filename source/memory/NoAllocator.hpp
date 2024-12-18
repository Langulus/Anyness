///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Allocation.hpp"


namespace Langulus::Anyness
{
   
   /// MSVC will likely never support std::aligned_alloc, so we use           
   /// a custom portable routine that's almost the same                       
   /// https://stackoverflow.com/questions/62962839                           
   ///                                                                        
   /// Each allocation has the following prefixed bytes:                      
   /// [padding][T::GetSize()][client bytes...]                               
   ///                                                                        
   ///   @param size - the number of client bytes to allocate                 
   ///   @return a newly allocated memory that is correctly aligned           
   template<AllocationPrimitive T>
   T* AlignedAllocate(Offset size) noexcept {
      const auto finalSize = T::GetNewAllocationSize(size) + Alignment;
      const auto base = ::std::malloc(finalSize);
      if (not base) UNLIKELY()
         return nullptr;

      // Align pointer to the alignment LANGULUS was built with         
      auto ptr = reinterpret_cast<T*>(
         (reinterpret_cast<Offset>(base) + Alignment)
         & ~(Alignment - Offset {1})
      );

      // Place the entry there                                          
      new (ptr) T {size, base};
      return ptr;
   }


   ///                                                                        
   ///   A mockup of a memory manager                                         
   ///                                                                        
   struct Allocator {
      /// No state when MANAGED_MEMORY feature is disabled                    
      struct State {
         consteval bool Assert() const noexcept { return true; }
      };

      NOD() LANGULUS(INLINED)
      static Allocation* Allocate(DMeta, Offset size) IF_UNSAFE(noexcept) {
         LANGULUS_ASSUME(DevAssumes, size, "Zero allocation is not allowed");
         return AlignedAllocate<Allocation>(size);
      }

      NOD() LANGULUS(INLINED)
      static Allocation* Reallocate(Offset size, UNUSED() Allocation* previous) IF_UNSAFE(noexcept) {
         LANGULUS_ASSUME(DevAssumes, previous,
            "Reallocating nullptr");
         LANGULUS_ASSUME(DevAssumes, size != previous->GetAllocatedSize(),
            "Reallocation suboptimal - size is same as previous");
         LANGULUS_ASSUME(DevAssumes, size,
            "Zero reallocation is not allowed");
         LANGULUS_ASSUME(DevAssumes, previous->mReferences,
            "Deallocating an unused allocation");

         return Allocator::Allocate(nullptr, size);
      }

      LANGULUS(INLINED)
      static void Deallocate(Allocation* entry) IF_UNSAFE(noexcept) {
         LANGULUS_ASSUME(DevAssumes, entry,
            "Deallocating nullptr");
         LANGULUS_ASSUME(DevAssumes, entry->GetAllocatedSize(),
            "Deallocating an empty allocation");
         LANGULUS_ASSUME(DevAssumes, entry->mReferences,
            "Deallocating an unused allocation");
         LANGULUS_ASSUME(DevAssumes, entry->mReferences == 1,
            "Deallocating an allocation used from multiple places");

         ::std::free(entry->mPool);
      }

      static constexpr const Allocation* Find(DMeta, const void*) noexcept {
         return nullptr;
      }

      static constexpr bool CheckAuthority(DMeta, const void*) noexcept {
         return false;
      }

      static consteval bool CollectGarbage() noexcept {
         return false;
      }

      #if LANGULUS_FEATURE(MEMORY_STATISTICS)
         static consteval void DumpPools() noexcept {}
      #endif
   };

} // namespace Langulus::Anyness