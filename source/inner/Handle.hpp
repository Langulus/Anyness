///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../blocks/Block.hpp"

namespace Langulus::A
{
   struct Handle {};
}

namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   An element & allocation pair                                         
   ///                                                                        
   /// Used as intermediate type when managed memory is enabled, to keep      
   /// track of pointers inserted to containers.                              
   ///                                                                        
   template<CT::Sparse T, bool EMBED>
   struct Handle : A::Handle {
      LANGULUS(TYPED) T;
      LANGULUS_BASES(A::Handle);

      static_assert(CT::Allocatable<Deptr<T>>,
         "Handle to unallocatable T is pointless");

   protected: TESTING(public:)
      friend class Block;

      /// @cond show_protected                                                
      // The value                                                      
      Conditional<EMBED, T*, T> mValue;
      // The entry                                                      
      Conditional<EMBED, Inner::Allocation**, Inner::Allocation*> mEntry;
      /// @endcond show_protected                                             

   public:
      Handle() = delete;

      constexpr Handle(const Handle&) noexcept = default;
      constexpr Handle(Handle&&) noexcept = default;
      constexpr Handle(decltype(mValue), decltype(mEntry)) SAFETY_NOEXCEPT();

      constexpr Handle& operator = (const Handle&) noexcept = default;
      constexpr Handle& operator = (Handle&&) noexcept = default;
      
      // Prefix operator                                                
      Handle& operator ++ () noexcept requires EMBED;

      // Suffix operator                                                
      NOD() Handle operator ++ (int) noexcept requires EMBED;

      operator T () const noexcept;

      template<bool RESET>
      void Destroy() const;
   };
   
} // namespace Langulus::Anyness

namespace Langulus::CT
{
   template<class... T>
   concept Handle = (DerivedFrom<T, A::Handle> && ...);
}

#include "Handle.inl"