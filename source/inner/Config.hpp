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
#include <RTTI/Meta.hpp>


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

   
namespace Langulus::Flow
{

   /// Just syntax sugar, for breaking ForEach loop                           
   constexpr bool Break = false;
   /// Just syntax sugar, for continuing ForEach loop                         
   constexpr bool Continue = true;

} // namespace Langulus::Flow

namespace Langulus
{

   using RTTI::DMeta;
   using RTTI::CMeta;
   using RTTI::TMeta;
   using RTTI::VMeta;
   using RTTI::AMeta;

   namespace Anyness
   {
      using RTTI::AllocationRequest;

      template<class, bool EMBED = true>
      struct Handle;

      class Any;
      template<CT::Data>
      class TAny;

      class BlockMap;

      template<bool>
      struct Map;
      using UnorderedMap = Map<false>;
      using OrderedMap = Map<true>;

      template<CT::Data, CT::Data, bool>
      struct TMap;
      template<CT::Data K, CT::Data V>
      using TOrderedMap = TMap<K, V, true>;
      template<CT::Data K, CT::Data V>
      using TUnorderedMap = TMap<K, V, false>;

      class BlockSet;

      template<bool>
      struct Set;
      using UnorderedSet = Set<false>;
      using OrderedSet = Set<true>;

      template<CT::Data, bool>
      struct TSet;
      template<CT::Data T>
      using TOrderedSet = TSet<T, true>;
      template<CT::Data T>
      using TUnorderedSet = TSet<T, false>;

      class Bytes;
      class Text;
      struct Path;

      template<CT::Data>
      class TOwned;
      template<class, bool>
      class TPointer;

      /// A shared pointer, that provides ownership and basic reference counting 
      /// Referencing comes from the block of memory that the pointer points to  
      template<class T>
      using Ptr = TPointer<T, false>;

      /// A shared pointer, that provides ownership and more reference counting  
      /// Referencing comes first from the block of memory that the pointer      
      /// points to, and second - the instance's individual reference counter    
      /// Useful for keeping track not only of the memory, but of the individual 
      /// element inside the memory block. Used to keep track of elements inside 
      /// THive and Hive (component factories for example)                       
      template<class T>
      using Ref = TPointer<T, true>;

      class Construct;
      using Messy = Any;
      class Neat;

   } // namespace Langulus::Anyness

} // namespace Langulus
