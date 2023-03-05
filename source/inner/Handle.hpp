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
   /// track of pointers inserted to containers. This does not have ownership 
   /// and can be used as iterator only when EMBEDed.                         
   ///                                                                        
   template<class T, bool EMBED>
   struct Handle : A::Handle {
      LANGULUS(TYPED) T;
      LANGULUS_BASES(A::Handle);

   public:
      static constexpr bool Embedded = EMBED;

      friend class Block;
      /// @cond show_protected                                                
      // The value                                                      
      Conditional<EMBED, T*, T> mValue;

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         // The entry, enabled only when managed memory is enabled      
         Conditional<EMBED && CT::Sparse<T>, Inner::Allocation**, Inner::Allocation*> mEntry;
      #endif
      /// @endcond show_protected                                             

   public:
      Handle() = delete;

      constexpr Handle(const Handle&) noexcept = default;
      constexpr Handle(Handle&&) noexcept = default;

      template<CT::Semantic S>
      constexpr Handle(S&&) noexcept requires (!EMBED);

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         constexpr Handle(T&, Inner::Allocation*&) SAFETY_NOEXCEPT() requires (EMBED && CT::Sparse<T>);
         constexpr Handle(T&, Inner::Allocation*) SAFETY_NOEXCEPT() requires (EMBED && CT::Dense<T>);
         constexpr Handle(T&&, Inner::Allocation* = nullptr) SAFETY_NOEXCEPT() requires (!EMBED);
      #else
         constexpr Handle(T&) noexcept requires (EMBED);
      #endif

      constexpr Handle& operator = (const Handle&) noexcept = default;
      constexpr Handle& operator = (Handle&&) noexcept = default;

      constexpr bool operator == (const T*) const noexcept requires (EMBED);

      NOD() T& Get() const noexcept;

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         NOD() Inner::Allocation*& GetEntry() const noexcept;
      #endif

      void New(T, Inner::Allocation* = nullptr) noexcept requires CT::Sparse<T>;
      void New(T&&, Inner::Allocation* = nullptr) noexcept requires CT::Dense<T>;
      template<CT::Semantic S>
      void New(S&&);
      template<CT::Semantic S>
      void NewUnknown(DMeta, S&&);

      template<CT::Semantic S>
      void Assign(S&&);

      template<bool RHS_EMBED>
      void Swap(Handle<T, RHS_EMBED>&);

      NOD() bool Compare(const T&) const;

      // Prefix operators                                               
      Handle& operator ++ () noexcept requires (EMBED);
      Handle& operator -- () noexcept requires (EMBED);

      // Suffix operators                                               
      NOD() Handle operator ++ (int) noexcept requires (EMBED);
      NOD() Handle operator -- (int) noexcept requires (EMBED);
      NOD() Handle operator + (Offset) noexcept requires (EMBED);
      NOD() Handle operator - (Offset) noexcept requires (EMBED);
      Handle& operator += (Offset) noexcept requires (EMBED);
      Handle& operator -= (Offset) noexcept requires (EMBED);

      template<bool RESET = false>
      void Destroy() const;
   };
   
   template<class T>
   using HandleLocal = Handle<T, false>;

} // namespace Langulus::Anyness

namespace Langulus::CT
{

   template<class... T>
   concept Handle = (DerivedFrom<T, A::Handle> && ...);

} // namespace Langulus::CT

#include "Handle.inl"