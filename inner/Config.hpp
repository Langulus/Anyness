#pragma once
#include <Langulus.Logger.hpp>

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

/// Memory allocations will be pooled, authority will be tracked,					
/// memory will be reused whenever possible, and you can also tweak				
/// runtime allocation strategies on per-type basis									
/// Significantly improves performance, no dependencies								
#ifdef LANGULUS_ENABLE_FEATURE_MANAGED_MEMORY
	#define LANGULUS_FEATURE_MANAGED_MEMORY() 1
#else
	#define LANGULUS_FEATURE_MANAGED_MEMORY() 0
#endif

/// Memory manager shall keep track of statistics										
/// Some overhead upon allocation/deallocation/reallocation							
#ifdef LANGULUS_ENABLE_FEATURE_MEMORY_STATISTICS
	#define LANGULUS_FEATURE_MEMORY_STATISTICS() 1
#else
	#define LANGULUS_FEATURE_MEMORY_STATISTICS() 0
#endif

/// The following features can be enabled only if managed memory is				
#define LANGULUS_FEATURE_MANAGED_REFLECTION() 0
#define LANGULUS_FEATURE_NEWDELETE() 0

#if LANGULUS_FEATURE_MANAGED_MEMORY()
	/// Reflections will be registered in a centralized location, allowing for	
	/// runtime type modification. Meta primitives will always be in the same	
	/// place in memory regardless of translation unit, which significantly		
	/// speeds up meta definition comparisons.											
	/// Naming collisions will be detected upon type registration					
	/// Gives a significant overhead on program launch, no dependencies			
	#ifdef LANGULUS_ENABLE_MANAGED_REFLECTION
		#undef LANGULUS_FEATURE_MANAGED_REFLECTION
		#define LANGULUS_FEATURE_MANAGED_REFLECTION() 1
	#endif

	/// Replace the default new-delete operators with one that use Allocator	
	/// No overhead, no dependencies															
	#ifdef LANGULUS_ENABLE_FEATURE_NEWDELETE
		#undef LANGULUS_FEATURE_NEWDELETE
		#define LANGULUS_FEATURE_NEWDELETE() 1
	#endif
#endif