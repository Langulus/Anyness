///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../source/Any.inl"
#include "../source/Bytes.inl"
#include "../source/Path.hpp"
#include "../source/TAny.inl"
#include "../source/Text.inl"
#include "../source/TPointer.inl"
#include "../source/TPair.inl"
#include "../source/Trait.inl"

#include "../source/TOrderedMap.inl"
#include "../source/TUnorderedMap.inl"

#include "../source/TOrderedSet.inl"
#include "../source/TUnorderedSet.inl"

#include "../source/OrderedSet.inl"
#include "../source/UnorderedSet.inl"

#include "../source/OrderedMap.inl"
#include "../source/UnorderedMap.inl"

#include "../source/Referenced.hpp"

#if LANGULUS_FEATURE(MANAGED_MEMORY)
#ifdef LANGULUS_EXPOSE_PRIVATE_HEADERS
   #include "../source/inner/Pool.hpp"
#endif
#endif

#define LANGULUS_LIBRARY_ANYNESS() 1
