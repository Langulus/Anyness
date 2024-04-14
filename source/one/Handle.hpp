///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Config.hpp"


namespace Langulus
{
   namespace A
   {

      /// An abstract handle                                                  
      struct Handle {
         LANGULUS(ABSTRACT) true;
         LANGULUS(UNINSERTABLE) true;
         LANGULUS(UNALLOCATABLE) true;
      };

   } // namespace Langulus::A

   namespace CT
   {

      template<class...T>
      concept Handle = (DerivedFrom<T, A::Handle> and ...);

      template<class...T>
      concept NotHandle = ((not Handle<T>) and ...);

   } // namespace Langulus::CT

} // namespace Langulus

namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   An element & allocation pair                                         
   ///                                                                        
   /// Used as intermediate type when managed memory is enabled, to keep      
   /// track of pointers inserted to containers. This does not have ownership 
   /// and can be used as iterator only when EMBEDed.                         
   ///                                                                        
   template<CT::Data T, bool EMBED>
   struct Handle : A::Handle {
      LANGULUS(TYPED) T;
      LANGULUS(ABSTRACT) false;
      LANGULUS(UNINSERTABLE) true;
      LANGULUS(UNALLOCATABLE) true;
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
      requires (EMBED and CT::Sparse<T>);

      constexpr Handle(T&, const Allocation*) IF_UNSAFE(noexcept)
      requires (EMBED and CT::Dense<T>);

      template<template<class> class S, CT::Handle H>
      requires CT::SemanticMakable<S, T>
      constexpr Handle(S<H>&&);

      template<class T1> requires (not EMBED and CT::MakableFrom<T, T1>)
      constexpr Handle(T1&&, const Allocation* = nullptr);

      constexpr Handle& operator = (const Handle&) noexcept = default;
      constexpr Handle& operator = (Handle&&) noexcept = default;

      template<class T1> requires CT::Comparable<T, T1>
      constexpr bool operator == (const T1&) const noexcept;
      template<class T1, bool EMBED1> requires CT::Comparable<T, T1>
      constexpr bool operator == (const Handle<T1, EMBED1>&) const noexcept;

      NOD() T& Get() const noexcept;
      NOD() const Allocation*& GetEntry() const noexcept;

      void Create(T,        const Allocation* = nullptr) noexcept requires CT::Sparse<T>;
      void Create(T const&, const Allocation* = nullptr) noexcept requires CT::Dense<T>;
      void Create(T&&,      const Allocation* = nullptr) noexcept requires CT::Dense<T>;

      void CreateSemantic(auto&&);
      template<template<class> class S, class T1> requires CT::Semantic<S<T1>>
      void CreateSemanticUnknown(DMeta, S<T1>&&);

      template<template<class> class S, class T1> requires CT::Semantic<S<T1>>
      void AssignSemantic(S<T1>&&);
      template<template<class> class S, class T1> requires CT::Semantic<S<T1>>
      void AssignSemanticUnknown(DMeta, S<T1>&&);

      template<bool RHS_EMBED>
      void Swap(Handle<T, RHS_EMBED>&);

      template<class T1> requires CT::Comparable<T, T1>
      NOD() bool Compare(const T1&) const;
      template<class T1, bool RHS_EMBED> requires CT::Comparable<T, T1>
      NOD() bool Compare(const Handle<T1, RHS_EMBED>&) const;

      // Prefix operators                                               
      Handle& operator ++ () noexcept requires Embedded;
      Handle& operator -- () noexcept requires Embedded;

      // Suffix operators                                               
      NOD() Handle operator ++ (int) const noexcept requires Embedded;
      NOD() Handle operator -- (int) const noexcept requires Embedded;

      NOD() Handle operator + (Offset) const noexcept requires Embedded;
      NOD() Handle operator - (Offset) const noexcept requires Embedded;
      Handle& operator += (Offset) noexcept requires Embedded;
      Handle& operator -= (Offset) noexcept requires Embedded;

      template<bool RESET = false, bool DEALLOCATE = true>
      void Destroy() const;
      template<bool RESET = false, bool DEALLOCATE = true>
      void DestroyUnknown(DMeta) const;
   };
   
   template<CT::NotHandle T>
   using HandleLocal = Handle<T, false>;

   /// Deduction guides                                                       
   template<CT::Sparse T>
   Handle(T&, const Allocation*&) -> Handle<T, true>;

   template<CT::Dense T>
   Handle(T&, const Allocation*) -> Handle<T, true>;

} // namespace Langulus::Anyness

namespace Langulus
{
   namespace CT::Inner
   {

      /// Unfolds T, if it is a bounded array, or std::range, and returns     
      /// a nullptr pointer of the type, contained inside. Nested for ranges  
      /// containing other ranges, or arrays containing ranges. Removes       
      /// semantics and handles, too                                          
      ///   @tparam T - type to unfold                                        
      ///   @return a pointer of the most inner type                          
      template<class T>
      consteval auto Unfold() noexcept {
         if constexpr (CT::Sparse<T>) {
            if constexpr (CT::Array<T>)
               return Unfold<Deext<T>>();
            else
               return (Deref<T>*) nullptr;
         }
         else if constexpr (CT::Handle<Desem<T>>)
            return (TypeOf<Desem<T>>*) nullptr;
         else if constexpr (::std::ranges::range<T>)
            return Unfold<TypeOf<T>>();
         else
            return (Deref<T>*) nullptr;
      }

   } // namespace Langulus::CT::Inner

   /// Nest-unfold any bounded array or std::range, and get most inner type   
   template<class T>
   using Unfold = Deptr<decltype(CT::Inner::Unfold<T>())>;

   namespace CT
   {

      /// Check if T is constructible with each of the provided arguments,    
      /// either directly, or by unfolding that argument                      
      template<class T, class...A>
      concept UnfoldMakableFrom = ((
               ::std::constructible_from<T, A>
            or ::std::constructible_from<T, Langulus::Unfold<A>>
         ) and ...);

      /// Check if T is insertable to containers, either directly, or while   
      /// wrapped in a semantic                                               
      template<class T1, class...TN>
      concept UnfoldInsertable = Insertable<Desem<T1>, Desem<TN>...>;

   } // namespace Langulus::CT

} // namespace Langulus
