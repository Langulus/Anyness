///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../blocks/Block.hpp"


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
      using ValueType = Conditional<Embedded, T*, T>;
      using EntryType = Conditional<Embedded and CT::Sparse<T>,
         const Allocation**, const Allocation*>;

      friend struct Block;
      /// @cond show_protected                                                
      // The value                                                      
      ValueType mValue;
      // The entry                                                      
      EntryType mEntry;
      /// @endcond show_protected                                             

   public:
      Handle() = delete;

      constexpr Handle(const Handle&) noexcept = default;
      constexpr Handle(Handle&&) noexcept = default;

      constexpr Handle(T&, const Allocation*&) IF_UNSAFE(noexcept)
      requires (Embedded and CT::Sparse<T>);

      constexpr Handle(T&, const Allocation*) IF_UNSAFE(noexcept)
      requires (Embedded and CT::Dense<T>);

      template<CT::NotHandle T1>
      constexpr Handle(T1&&, const Allocation* = nullptr)
      requires (not Embedded and CT::Inner::MakableFrom<T, T1>);

      template<template<class> class S, CT::Handle H>
      constexpr Handle(S<H>&&)
      requires (CT::Inner::MakableFrom<T, S<TypeOf<H>>>);

      constexpr Handle& operator = (const Handle&) noexcept = default;
      constexpr Handle& operator = (Handle&&) noexcept = default;

      constexpr bool operator == (const T*) const noexcept requires Embedded;
      constexpr bool operator == (const Handle&) const noexcept requires Embedded;

      NOD() T& Get() const noexcept;
      NOD() const Allocation*& GetEntry() const noexcept;

      void New(T,        const Allocation* = nullptr) noexcept requires CT::Sparse<T>;
      void New(const T&, const Allocation* = nullptr) noexcept requires CT::Dense<T>;
      void New(T&&,      const Allocation* = nullptr) noexcept requires CT::Dense<T>;
      void New(CT::Semantic auto&&);

      void Assign(CT::Semantic auto&&);

      template<bool RHS_EMBED>
      void Swap(Handle<T, RHS_EMBED>&);

      NOD() bool Compare(const T&) const;
      template<bool RHS_EMBED>
      NOD() bool Compare(const Handle<T, RHS_EMBED>&) const;

      // Prefix operators                                               
      Handle& operator ++ () noexcept requires Embedded;
      Handle& operator -- () noexcept requires Embedded;

      // Suffix operators                                               
      NOD() Handle operator ++ (int) noexcept requires Embedded;
      NOD() Handle operator -- (int) noexcept requires Embedded;
      NOD() Handle operator + (Offset) noexcept requires Embedded;
      NOD() Handle operator - (Offset) noexcept requires Embedded;
      Handle& operator += (Offset) noexcept requires Embedded;
      Handle& operator -= (Offset) noexcept requires Embedded;

      template<bool RESET = false>
      void Destroy() const;
      template<bool RESET = false>
      void DestroyUnknown(DMeta) const;
   };
   
   template<CT::NotHandle T>
   using HandleLocal = Handle<T, false>;

   /// A handle that has void T is just a type-erased Block                   
   template<>
   struct Handle<void> : Block {};

} // namespace Langulus::Anyness