///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../inner/Allocation.hpp"
#include "Pool.hpp"
#include <unordered_set>

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Memory allocator                                                     
   ///                                                                        
   /// The lowest-level memory management interface                           
   /// Basically an overcomplicated wrapper for malloc/free                   
   ///                                                                        
   struct Allocator {
      #if LANGULUS_FEATURE(MEMORY_STATISTICS)
         struct Statistics {
            // The real allocated bytes, provided by malloc in backend  
            Size mBytesAllocatedByBackend {};
            // The bytes allocated by the frontend                      
            Size mBytesAllocatedByFrontend {};
            // Number of registered entries                             
            Count mEntries {};
            // Number of registered pools                               
            Count mPools {};

            #if LANGULUS_FEATURE(MANAGED_REFLECTION)
               // Number of registered meta datas                       
               Count mDataDefinitions {};
               // Number of registered meta traits                      
               Count mTraitDefinitions {};
               // Number of registered meta verbs                       
               Count mVerbDefinitions {};
            #endif

            bool operator == (const Statistics&) const noexcept = default;

            void AddPool(const Pool*) noexcept;
            void DelPool(const Pool*) noexcept;
         };
      
      private:
         Statistics mStatistics {};
      #endif

   private:
      // Default pool chain                                             
      Pool* mDefaultPoolChain {};
      // The last succesfull Find() result in default pool chain        
      mutable Pool* mLastFoundPool {};

      // Pool chains for types that use PoolTactic::Size                
      static constexpr Count SizeBuckets = sizeof(Size) * 8;
      Pool* mSizePoolChain[SizeBuckets] {};

      ::std::unordered_set<DMeta> mInstantiatedTypes;

   private:
      LANGULUS_API(ANYNESS)
      void DumpPool(Offset, const Pool*) const noexcept;

      LANGULUS_API(ANYNESS)
      void CollectGarbageChain(Pool*&);

      Allocation* FindInChain(const void*, Pool*) const SAFETY_NOEXCEPT();
      bool ContainedInChain(const void*, Pool*) const SAFETY_NOEXCEPT();

   public:
      //                                                                
      // Standard functionality                                         
      //                                                                
      NOD() LANGULUS_API(ANYNESS)
      Allocation* Allocate(RTTI::DMeta, const Size&) SAFETY_NOEXCEPT();

      NOD() LANGULUS_API(ANYNESS)
      Allocation* Reallocate(const Size&, Allocation*) SAFETY_NOEXCEPT();

      LANGULUS_API(ANYNESS)
      void Deallocate(Allocation*) SAFETY_NOEXCEPT();

      NOD() LANGULUS_API(ANYNESS)
      Allocation* Find(RTTI::DMeta, const void*) const SAFETY_NOEXCEPT();

      NOD() LANGULUS_API(ANYNESS)
      bool CheckAuthority(RTTI::DMeta, const void*) const SAFETY_NOEXCEPT();

      NOD() LANGULUS_API(ANYNESS)
      Pool* AllocatePool(DMeta, const Size&) SAFETY_NOEXCEPT();

      LANGULUS_API(ANYNESS)
      void DeallocatePool(Pool*) SAFETY_NOEXCEPT();

      LANGULUS_API(ANYNESS)
      void CollectGarbage();

      LANGULUS_API(ANYNESS)
      Count CheckBoundary(const Token&) const noexcept;

      #if LANGULUS_FEATURE(MEMORY_STATISTICS)
         NOD() LANGULUS_API(ANYNESS)
         const Statistics& GetStatistics() const noexcept;

         LANGULUS_API(ANYNESS)
         void DumpPools() const noexcept;
      #endif
   };


   ///                                                                        
   ///   The global memory manager                                            
   ///                                                                        
   LANGULUS_API(ANYNESS) extern Allocator Fractalloc;

} // namespace Langulus::Anyness