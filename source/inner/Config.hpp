///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <Core/Config.hpp>

#if defined(LANGULUS_EXPORT_ALL) || defined(LANGULUS_EXPORT_ANYNESS)
   #define LANGULUS_API_ANYNESS() LANGULUS_EXPORT()
#else
   #define LANGULUS_API_ANYNESS() LANGULUS_IMPORT()
#endif

/// Enables utf support and utilities for Text container                      
/// No runtime overhead                                                       
#ifdef LANGULUS_ENABLE_FEATURE_UNICODE
   #define LANGULUS_FEATURE_UNICODE() 1
   #define IF_LANGULUS_UNICODE(a) a
   #define IF_NOT_LANGULUS_UNICODE(a)
#else
   #define LANGULUS_FEATURE_UNICODE() 0
   #define IF_LANGULUS_UNICODE(a) 
   #define IF_NOT_LANGULUS_UNICODE(a) a
#endif

/// Enable memory compression utilities for containers                        
/// Gives a bit of general runtime overhead, zstd will be linked              
#ifdef LANGULUS_ENABLE_FEATURE_COMPRESSION
   #define LANGULUS_FEATURE_COMPRESSION() 1
   #define IF_LANGULUS_COMPRESSION(a) a
   #define IF_NOT_LANGULUS_COMPRESSION(a)
#else
   #define LANGULUS_FEATURE_COMPRESSION() 0
   #define IF_LANGULUS_COMPRESSION(a) 
   #define IF_NOT_LANGULUS_COMPRESSION(a) a
#endif

/// Enable memory encryption and decryption                                   
/// Gives a tiny runtime overhead, no dependencies                            
#ifdef LANGULUS_ENABLE_FEATURE_ENCRYPTION
   #define LANGULUS_FEATURE_ENCRYPTION() 1
   #define IF_LANGULUS_ENCRYPTION(a) a
   #define IF_NOT_LANGULUS_ENCRYPTION(a)
#else
   #define LANGULUS_FEATURE_ENCRYPTION() 0
   #define IF_LANGULUS_ENCRYPTION(a) 
   #define IF_NOT_LANGULUS_ENCRYPTION(a) a
#endif

/// Enable memory manager                                                     
#ifdef LANGULUS_ENABLE_FEATURE_MANAGED_MEMORY
   #define LANGULUS_FEATURE_MANAGED_MEMORY() 1
   #define IF_LANGULUS_MANAGED_MEMORY(a) a
   #define IF_NOT_LANGULUS_MANAGED_MEMORY(a)
   #include <Fractalloc/Allocator.hpp>

   using Allocator = ::Langulus::Fractalloc::Allocator;
   using Allocation = ::Langulus::Fractalloc::Allocation;
#else
   #define LANGULUS_FEATURE_MANAGED_MEMORY() 0
   #define IF_LANGULUS_MANAGED_MEMORY(a) 
   #define IF_NOT_LANGULUS_MANAGED_MEMORY(a) a
   #include "NoAllocator.hpp"
#endif

/// Make the rest of the code aware, that Langulus::Anyness has been included 
#define LANGULUS_LIBRARY_ANYNESS() 1
