///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
#else
   #include "memory/NoAllocator.hpp"
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
         Repeat = 2,    // Repeat the current element                   
         Discard = 3,   // Remove the current element                   
         NextLoop = 4   // Skip to next function in the visitor pattern 
      } mControl;

      LoopControl() = delete;

      constexpr LoopControl(bool a) noexcept
         : mControl {static_cast<Command>(a)} {}
      constexpr LoopControl(Command a) noexcept
         : mControl {a} {}

      explicit constexpr operator bool() const noexcept {
         return mControl == Continue or mControl == Repeat;
      }

      constexpr bool operator == (const LoopControl& rhs) const noexcept {
         return mControl == rhs.mControl;
      }
   };

   namespace Loop
   {

      constexpr LoopControl Break      = LoopControl::Break;
      constexpr LoopControl Continue   = LoopControl::Continue;
      constexpr LoopControl Repeat     = LoopControl::Repeat;
      constexpr LoopControl Discard    = LoopControl::Discard;
      constexpr LoopControl NextLoop   = LoopControl::NextLoop;

   } // namespace Langulus::Loop

   namespace CT
   {

      /// The ultimate Anyness container tag                                  
      /// Checks if all T are marked as Anyness containers                    
      template<class...T>
      concept Container = (requires { Decay<T>::CTTI_Container; } and ...);

      /// Checks if none of the Ts are marked as Anyness containers           
      template<class...T>
      concept NotContainer = ((not Container<T>) and ...);

   } // namespace Langulus::CT

   namespace Anyness
   {
      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         using Allocator  = ::Langulus::Fractalloc::Allocator;
         using Allocation = ::Langulus::Fractalloc::Allocation;
      #else
         using Allocator  = ::Langulus::Anyness::Allocator;
         using Allocation = ::Langulus::Anyness::Allocation;
      #endif

      using RTTI::DMeta;
      using RTTI::CMeta;
      using RTTI::TMeta;
      using RTTI::VMeta;
      using RTTI::AMeta;
      using RTTI::AllocationRequest;

      template<class, bool EMBED = true>
      struct Handle;

      class Many;
      using Messy = Many;
      template<CT::Data>
      class TMany;

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
      class Own;
      template<class>
      class Ref;

      class Construct;
      class Neat;

   } // namespace Langulus::Anyness

} // namespace Langulus
