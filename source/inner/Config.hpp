///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <LangulusRTTI.hpp>

#if defined(LANGULUS_EXPORT_ALL) || defined(LANGULUS_EXPORT_ANYNESS)
   #define LANGULUS_API_ANYNESS() LANGULUS_EXPORT()
#else
   #define LANGULUS_API_ANYNESS() LANGULUS_IMPORT()
#endif

/// Use the utfcpp library to convert between utf types                       
/// No overhead, requires utfcpp to be linked                                 
#ifdef LANGULUS_ENABLE_FEATURE_UTFCPP
   #define LANGULUS_FEATURE_UTFCPP() 1
#else
   #define LANGULUS_FEATURE_UTFCPP() 0
#endif

/// Enable memory compression via the use of zlib                             
/// Gives a tiny runtime overhead, requires zlib to be linked                 
#ifdef LANGULUS_ENABLE_FEATURE_ZLIB
   #define LANGULUS_FEATURE_ZLIB() 1
#else
   #define LANGULUS_FEATURE_ZLIB() 0
#endif

/// Enable memory encryption and decryption                                   
/// Gives a tiny runtime overhead, no dependencies                            
#ifdef LANGULUS_ENABLE_FEATURE_ENCRYPTION
   #define LANGULUS_FEATURE_ENCRYPTION() 1
#else
   #define LANGULUS_FEATURE_ENCRYPTION() 0
#endif

/// Memory manager shall keep track of statistics                             
/// Some overhead upon allocation/deallocation/reallocation                   
#ifdef LANGULUS_ENABLE_FEATURE_MEMORY_STATISTICS
   #define LANGULUS_FEATURE_MEMORY_STATISTICS() 1
#else
   #define LANGULUS_FEATURE_MEMORY_STATISTICS() 0
#endif

/// Replace the default new-delete operators with custom ones                 
/// No overhead, no dependencies                                              
#ifdef LANGULUS_ENABLE_FEATURE_NEWDELETE
   #undef LANGULUS_FEATURE_NEWDELETE
   #define LANGULUS_FEATURE_NEWDELETE() 1
#else
   #define LANGULUS_FEATURE_NEWDELETE() 0
#endif
