#pragma once
#include <Langulus.Core.hpp>

/// Replace the default new-delete operators with one that use Allocator		
/// No overhead, no dependencies																
#define LANGULUS_FEATURE_NEWDELETE() 0

/// Use the utfcpp library to convert between utf types								
/// No overhead, requires utfcpp to be linked											
#define LANGULUS_FEATURE_UTFCPP() 0

/// Enable memory compression via the use of zlib										
/// Gives a tiny runtime overhead, requires zlib to be linked						
#define LANGULUS_FEATURE_ZLIB() 0

/// Enable memory encryption and decryption												
/// Gives a tiny runtime overhead, no dependencies										
#define LANGULUS_FEATURE_ENCRYPTION() 0

/// Memory allocations will be pooled, authority will be tracked,					
/// memory will be reused whenever possible, and you can also tweak				
/// runtime allocation strategies on per-type basis									
/// Significantly improves performance, no dependencies								
#define LANGULUS_FEATURE_MANAGED_MEMORY() 0

/// Reflections will be registered in a centralized location, allowing for		
/// runtime type modification. Meta primitives will always be in the same		
/// place in memory regardless of translation unit, which significantly			
/// speeds up meta definition comparisons.												
/// Naming collisions will be detected upon type registration						
/// Gives a significant overhead on program launch, no dependencies				
#define LANGULUS_FEATURE_MANAGED_REFLECTION() 0