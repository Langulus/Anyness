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
   #include "memory/NoAllocator.hpp"

   using Allocator = ::Langulus::Anyness::Allocator;
   using Allocation = ::Langulus::Anyness::Allocation;
#endif

/// Make the rest of the code aware, that Langulus::Anyness has been included 
#define LANGULUS_LIBRARY_ANYNESS() 1


namespace Langulus
{

   /// Loop controls from inside ForEach lambdas when iterating containers    
   struct LoopControl {
      enum Command : int {
         Break = 0,     // Break the loop                                  
         Continue = 1,  // Continue the loop                               
         Discard = 2,   // Remove the current element                      
         NextLoop = 3   // Skip to next function in the visitor pattern    
      } mControl;

      LoopControl() = delete;

      constexpr LoopControl(bool a) noexcept
         : mControl {static_cast<Command>(a)} {}
      constexpr LoopControl(Command a) noexcept
         : mControl {a} {}

      explicit constexpr operator bool() const noexcept {
         return mControl == Continue;
      }

      constexpr bool operator == (const LoopControl& rhs) const noexcept {
         return mControl == rhs.mControl;
      }
   };

   namespace Loop
   {
      constexpr LoopControl Break      = LoopControl::Break;
      constexpr LoopControl Continue   = LoopControl::Continue;
      constexpr LoopControl Discard    = LoopControl::Discard;
      constexpr LoopControl NextLoop   = LoopControl::NextLoop;
   }

   using RTTI::DMeta;
   using RTTI::CMeta;
   using RTTI::TMeta;
   using RTTI::VMeta;
   using RTTI::AMeta;

   namespace Anyness
   {
      using RTTI::AllocationRequest;

      template<CT::Data, bool EMBED = true>
      struct Handle;

      class Any;
      template<CT::Data>
      class TAny;

      struct BlockMap;

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

      struct BlockSet;

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

      struct Bytes;
      struct Text;
      struct Path;

      template<CT::Data>
      class TOwned;
      template<class>
      class TPointer;

      /// A shared pointer, that provides ownership and more reference counting  
      /// Referencing comes first from the block of memory that the pointer      
      /// points to, and second - the instance's individual reference counter    
      /// Useful for keeping track not only of the memory, but of the individual 
      /// element inside the memory block. Used to keep track of elements inside 
      /// THive and Hive (component factories for example)                       
      template<class T>
      using Ref = TPointer<T>;

      class Construct;
      using Messy = Any;
      class Neat;

   } // namespace Langulus::Anyness

} // namespace Langulus
