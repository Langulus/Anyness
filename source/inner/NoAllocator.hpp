///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Allocation.hpp"
#include <unordered_set>

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
   T* AlignedAllocate(const Size& size) noexcept {
      const auto finalSize = T::GetNewAllocationSize(size) + Alignment;
      const auto base = ::std::malloc(finalSize);
      if (!base) UNLIKELY()
         return nullptr;

      // Align pointer to the alignment LANGULUS was built with         
      auto ptr = reinterpret_cast<T*>(
         (reinterpret_cast<Size>(base) + Alignment)
         & ~(Alignment - Size {1})
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
         constexpr bool Assert() const noexcept { return true; }
      };

      LANGULUS(INLINED)
      static NOD() Allocation* Allocate(RTTI::DMeta, const Size& size) SAFETY_NOEXCEPT() {
         LANGULUS_ASSUME(DevAssumes, size, "Zero allocation is not allowed");
         return AlignedAllocate<Allocation>(size);
      }

      LANGULUS(INLINED)
      static NOD() Allocation* Reallocate(const Size& size, Allocation* previous) SAFETY_NOEXCEPT() {
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
      static void Deallocate(Allocation* entry) SAFETY_NOEXCEPT() {
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

      LANGULUS(INLINED)
      static constexpr NOD() Allocation* Find(RTTI::DMeta, const void*) noexcept {
         return nullptr;
      }

      LANGULUS(INLINED)
      static constexpr NOD() bool CheckAuthority(RTTI::DMeta, const void*) noexcept {
         return false;
      }

      #if LANGULUS_FEATURE(MEMORY_STATISTICS)
         LANGULUS(INLINED)
         static void DumpPools() noexcept {}
      #endif
   };

} // namespace Langulus::Anyness