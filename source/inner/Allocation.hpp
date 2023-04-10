///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Config.hpp"

namespace Langulus::Anyness
{

   using RTTI::MetaData;
   using RTTI::MetaTrait;
   using RTTI::MetaVerb;
   using RTTI::DMeta;
   using RTTI::TMeta;
   using RTTI::VMeta;

   /// Wrapper for memcpy                                                     
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @tparam FROM - source memory type (deducible)                        
   ///   @param to - [out] destination memory                                 
   ///   @param from - source of data to copy                                 
   template<class TO, class FROM>
   LANGULUS(INLINED)
   void CopyMemory(TO* to, const FROM* from) noexcept {
      static_assert(CT::Void<TO> || CT::Sparse<TO> || CT::POD<TO> || ::std::is_trivial_v<TO>, 
         "TO must be either pointer, reflected as POD, or trivial. "
         "(you can suppress this error by casting pointer to void*)");

      static_assert(CT::Void<TO> || (CT::Same<TO, FROM> && CT::Sparse<TO> == CT::Sparse<FROM>),
         "TO and FROM must be the exact same types"
         "(you can suppress this error by casting pointer to void*)");

      if constexpr (CT::Void<TO>)
         LANGULUS_ERROR("Bytecount not specified when copying void pointers");

      ::std::memcpy(
         static_cast<void*>(to),
         static_cast<const void*>(from),
         sizeof(TO)
      );
   }

   /// Wrapper for memcpy                                                     
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @tparam FROM - source memory type (deducible)                        
   ///   @param to - [out] destination memory                                 
   ///   @param from - source of data to copy                                 
   ///   @param count - number of elements to copy                            
   ///   @attention count becomes bytecount, when TO is void                  
   template<class TO, class FROM>
   LANGULUS(INLINED)
   void CopyMemory(TO* to, const FROM* from, const Count& count) noexcept {
      static_assert(CT::Void<TO> || CT::Sparse<TO> || CT::POD<TO> || ::std::is_trivial_v<TO>, 
         "TO must be either pointer, reflected as POD, or trivial. "
         "(you can suppress this error by casting pointer to void*)");

      static_assert(CT::Void<TO> || (CT::Same<TO, FROM> && CT::Sparse<TO> == CT::Sparse<FROM>),
         "TO and FROM must be the exact same types"
         "(you can suppress this error by casting pointer to void*)");

      if constexpr (CT::Void<TO>) {
         ::std::memcpy(
            static_cast<void*>(to),
            static_cast<const void*>(from),
            count
         );
      }
      else {
         ::std::memcpy(
            static_cast<void*>(to),
            static_cast<const void*>(from),
            sizeof(TO) * count
         );
      }
   }
   
   /// Wrapper for memset                                                     
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @tparam FILLER - value to fill in with                               
   ///   @param to - [out] destination memory                                 
   template<int FILLER, class TO>
   LANGULUS(INLINED)
   void FillMemory(TO* to) noexcept {
      static_assert(CT::Void<TO> || CT::Sparse<TO> || CT::POD<TO> || ::std::is_trivial_v<TO>,
         "TO must be either pointer, reflected as POD, or trivial. "
         "(you can suppress this error by casting to void*)");

      static_assert(FILLER || CT::Nullifiable<TO> || CT::Void<TO> || CT::Sparse<TO> || CT::Fundamental<TO>,
         "Filling with zeroes requires the type to be reflected as nullifiable, "
         "or be a pointer/fundamental (you can suppress this error by casting to void*)");

      if constexpr (CT::Void<TO>)
         LANGULUS_ERROR("Bytecount not specified when filling void pointer");
      
      ::std::memset(static_cast<void*>(to), FILLER, sizeof(TO));
   }
   
   /// Wrapper for memset                                                     
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @tparam FILLER - value to fill in with                               
   ///   @param to - [out] destination memory                                 
   ///   @param count - number of elements to fill                            
   ///   @attention count becomes bytecount, when TO is void                  
   template<int FILLER, class TO>
   LANGULUS(INLINED)
   void FillMemory(TO* to, const Count& count) noexcept {
      static_assert(CT::Void<TO> || CT::Sparse<TO> || CT::POD<TO> || ::std::is_trivial_v<TO>,
         "TO must be either pointer, reflected as POD, or trivial. "
         "(you can suppress this error by casting to void*)");

      static_assert(FILLER || CT::Nullifiable<TO> || CT::Void<TO> || CT::Sparse<TO> || CT::Fundamental<TO>,
         "Filling with zeroes requires the type to be reflected as nullifiable, "
         "or be a pointer/fundamental (you can suppress this error by casting to void*)");

      if constexpr (CT::Void<TO>)
         ::std::memset(static_cast<void*>(to), FILLER, count);
      else
         ::std::memset(static_cast<void*>(to), FILLER, sizeof(TO) * count);
   }

   /// Wrapper for memset 0                                                   
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @param to - [out] destination memory                                 
   template<class TO>
   LANGULUS(INLINED)
   void ZeroMemory(TO* to) noexcept {
      return FillMemory<0>(to);
   }
      
   /// Wrapper for memset 0                                                   
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @param to - [out] destination memory                                 
   ///   @param count - number of elements to fill                            
   ///   @attention count becomes bytecount, when TO is void                  
   template<class TO>
   LANGULUS(INLINED)
   void ZeroMemory(TO* to, const Count& count) noexcept {
      return FillMemory<0>(to, count);
   }
      
   /// Wrapper for memmove                                                    
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @tparam FROM - source memory type (deducible)                        
   ///   @param to - [out] destination memory                                 
   ///   @param from - source of data to move                                 
   template<class TO, class FROM>
   LANGULUS(INLINED)
   void MoveMemory(TO* to, const FROM* from) noexcept {
      static_assert(CT::Void<TO> || CT::Sparse<TO> || CT::POD<TO> || ::std::is_trivial_v<TO>,
         "TO must be either pointer, reflected as POD, or trivial. "
         "(You can suppress this error by casting pointer to void*)");

      static_assert(CT::Void<TO> || (CT::Same<TO, FROM> && CT::Sparse<TO> == CT::Sparse<FROM>),
         "TO and FROM must be the exact same types"
         "(you can suppress this error by casting pointer to void*)");

      if constexpr (CT::Void<TO>)
         LANGULUS_ERROR("Bytecount not specified when filling void pointer");

      ::std::memmove(
         static_cast<void*>(to),
         static_cast<const void*>(from),
         sizeof(TO)
      );

      #if LANGULUS(PARANOID)
         TODO() // zero old memory, but beware - `from` and `to` might overlap
      #endif
   }

   /// Wrapper for memmove                                                    
   ///   @tparam TO - destination memory type (deducible)                     
   ///   @tparam FROM - source memory type (deducible)                        
   ///   @param to - [out] destination memory                                 
   ///   @param from - source of data to move                                 
   ///   @param count - number of elements to move                            
   ///   @attention count becomes bytecount, when TO is void                  
   template<class TO, class FROM>
   LANGULUS(INLINED)
   void MoveMemory(TO* to, const FROM* from, const Count& count) noexcept {
      static_assert(CT::Void<TO> || CT::Sparse<TO> || CT::POD<TO> || ::std::is_trivial_v<TO>,
         "TO must be either pointer, reflected as POD, or trivial. "
         "(You can suppress this error by casting pointer to void*)");

      static_assert(CT::Void<TO> || (CT::Same<TO, FROM> && CT::Sparse<TO> == CT::Sparse<FROM>),
         "TO and FROM must be the exact same types"
         "(you can suppress this error by casting pointer to void*)");

      if constexpr (CT::Void<TO>) {
         ::std::memmove(
            static_cast<void*>(to),
            static_cast<const void*>(from),
            count
         );
      }
      else {
         ::std::memmove(
            static_cast<void*>(to),
            static_cast<const void*>(from),
            sizeof(TO) * count
         );
      }

      #if LANGULUS(PARANOID)
         TODO() // zero old memory, but beware - `from` and `to` might overlap
      #endif
   }
   
} // namespace Langulus::Anyness

namespace Langulus::Anyness::Inner
{

   template<class T>
   concept AllocationPrimitive = requires(T a) { 
      {T::GetNewAllocationSize(Size {})} -> CT::Same<Size>;
   };
            
   template<AllocationPrimitive T>
   T* AlignedAllocate(const Size& size) SAFETY_NOEXCEPT();


   NOD() constexpr Size FastLog2(Size) noexcept;
   NOD() constexpr Size LSB(const Size&) noexcept;


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

} // namespace Langulus::Anyness::Inner

#include "Allocation.inl"
