///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <Core/Common.hpp>

#if defined(LANGULUS_EXPORT_ALL) or defined(LANGULUS_EXPORT_ANYNESS)
   #define LANGULUS_API_ANYNESS() LANGULUS_EXPORT()
#else
   #define LANGULUS_API_ANYNESS() LANGULUS_IMPORT()
#endif

/// Enable memory manager                                                     
#if LANGULUS_FEATURE(MANAGED_MEMORY)
   #include <Fractalloc/Allocator.hpp>

   using Allocator = ::Langulus::Fractalloc::Allocator;
   using Allocation = ::Langulus::Fractalloc::Allocation;
#else
   #include "NoAllocator.hpp"

   using Allocator = ::Langulus::Anyness::Allocator;
   using Allocation = ::Langulus::Anyness::Allocation;
#endif

/// Make the rest of the code aware, that Langulus::Anyness has been included 
#define LANGULUS_LIBRARY_ANYNESS() 1
