///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "../Any.hpp"
#include "../Bytes.hpp"
#include "../Map.hpp"
#include "../Path.hpp"
#include "../TAny.hpp"
#include "../Text.hpp"
#include "../TPointer.hpp"
#include "../Trait.hpp"
#include "../TOrderedMap.hpp"
#include "../TUnorderedMap.hpp"
#include "../TUnorderedSet.hpp"
#include "../TFactory.hpp"

#if LANGULUS_FEATURE(MANAGED_MEMORY)
#ifdef LANGULUS_EXPOSE_PRIVATE_HEADERS
	#include "../inner/Pool.hpp"
#endif
#endif

#define LANGULUS_MODULE_ANYNESS()
