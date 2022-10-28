///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../source/Any.hpp"
#include "../source/Bytes.hpp"
#include "../source/Path.hpp"
#include "../source/TAny.hpp"
#include "../source/Text.hpp"
#include "../source/TPointer.hpp"
#include "../source/Trait.hpp"
#include "../source/TOrderedMap.hpp"
#include "../source/OrderedMap.hpp"
#include "../source/TUnorderedMap.hpp"
#include "../source/UnorderedMap.hpp"
#include "../source/TUnorderedSet.hpp"
#include "../source/TFactory.hpp"

#if LANGULUS_FEATURE(MANAGED_MEMORY)
#ifdef LANGULUS_EXPOSE_PRIVATE_HEADERS
   #include "../source/inner/Pool.hpp"
#endif
#endif

#define LANGULUS_LIBRARY_ANYNESS() 1
