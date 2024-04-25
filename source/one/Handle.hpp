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
         LANGULUS(UNALLOCATABLE) true;
         LANGULUS(REFLECTABLE) false;
      };

   } // namespace Langulus::A

   namespace CT
   {

      /// Concept for differentiating Handle types                            
      template<class...T>
      concept Handle = (DerivedFrom<T, A::Handle> and ...);

      /// Opposite of a CT::Handle                                            
      template<class...T>
      concept NotHandle = ((not Handle<T>) and ...);

   } // namespace Langulus::CT

} // namespace Langulus

namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   An element & allocation pair                                         
   ///                                                                        
   ///   Used as intermediate type when managed memory is enabled, to keep    
   /// track of pointers inserted or accessed to/from containers. This does   
   /// not have ownership, and can be used as iterator only when EMBEDed.     
   ///                                                                        
   template<class T, bool EMBED>
   struct Handle : A::Handle {
      LANGULUS(TYPED) T;
      LANGULUS(ABSTRACT) false;
      LANGULUS_BASES(A::Handle);

   public:
      static_assert(CT::NotHandle<T>, "Handles can't be nested");
      static constexpr bool Embedded   = EMBED;
      static constexpr bool TypeErased = CT::TypeErased<T>;
      static constexpr bool Sparse     = CT::Sparse<T>;
      static constexpr bool Mutable    = CT::Mutable<T>;
      static_assert(Embedded or not TypeErased,
         "Can't have a type-erased local handle, unless it is sparse");

      //using Type      = Conditional<TypeErased, Byte*, T>;
      using AllocType = const Allocation*;
      using ValueType = Conditional<Embedded,
            Conditional<TypeErased, Byte*, T*>,
            T>;
      using EntryType = Conditional<Embedded and Sparse,
            Conditional<Mutable, AllocType*, AllocType const*>,
            AllocType>;

      friend struct Block<T>;

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

      constexpr Handle(ValueType) noexcept requires Embedded;
      constexpr Handle(ValueType, EntryType = nullptr) noexcept requires Embedded;

      // Mutable constructors                                           
      /*constexpr Handle(T&, AllocType&) IF_UNSAFE(noexcept)
      requires (Embedded and Sparse and Mutable);

      constexpr Handle(T&, AllocType) IF_UNSAFE(noexcept)
      requires (Embedded and not Sparse and Mutable);
      constexpr Handle(T&) IF_UNSAFE(noexcept)
      requires (Embedded and not Sparse and Mutable);

      // Constant constructors                                          
      constexpr Handle(const T&, const AllocType&) IF_UNSAFE(noexcept)
      requires (EMBED and CT::Sparse<T> and not CT::Mutable<T>);

      constexpr Handle(const T&, AllocType) IF_UNSAFE(noexcept)
      requires (EMBED and CT::Dense<T> and not CT::Mutable<T>);
      constexpr Handle(const T&) IF_UNSAFE(noexcept)
      requires (EMBED and CT::Dense<T> and not CT::Mutable<T>);*/

      // Construct from handle                                          
      template<template<class> class S, CT::Handle H>
      requires CT::SemanticMakable<S, T>
      constexpr Handle(S<H>&&);

      // Construct from Ref                                             
      /*template<template<class> class S>
      requires (CT::Sparse<T> and CT::Semantic<S<Ref<Deptr<T>>>>)
      constexpr Handle(S<Ref<Deptr<T>>>&&);

      // Construct from Own                                             
      template<template<class> class S>
      requires (CT::Dense<T> and CT::Semantic<S<Own<T>>>)
      constexpr Handle(S<Own<T>>&&);*/

      // Construct locally, if not embedded                             
      //template<class T1> requires (not EMBED and CT::MakableFrom<T, T1>)
      //constexpr Handle(T1&&, AllocType = nullptr);

      ~Handle();

      constexpr Handle& operator = (const Handle&) noexcept = default;
      constexpr Handle& operator = (Handle&&) noexcept = default;

      /*template<CT::NotHandle RHS> requires CT::Comparable<T, RHS>
      constexpr bool operator == (const RHS&) const noexcept;
      template<CT::Handle RHS> requires CT::Comparable<T, TypeOf<RHS>>
      constexpr bool operator == (const RHS&) const noexcept;*/

      NOD() decltype(auto) Get() const noexcept requires (not TypeErased);
      NOD() decltype(auto) GetEntry() const noexcept;

      /*void Create(T,        AllocType = nullptr) noexcept requires CT::Sparse<T>;
      void Create(T const&, AllocType = nullptr) noexcept requires CT::Dense<T>;
      void Create(T&&,      AllocType = nullptr) noexcept requires CT::Dense<T>;*/

      void CreateSemantic(DMeta, auto&&);

      /*template<template<class> class S, class T1> requires CT::Semantic<S<T1>>
      void CreateSemanticUnknown(DMeta, S<T1>&&);*/

      void AssignSemantic(DMeta, auto&&) requires Mutable;

      /*template<template<class> class S, class T1> requires (CT::Semantic<S<T1>> and CT::Mutable<T>)
      void AssignSemanticUnknown(DMeta, S<T1>&&);*/

      void Swap(DMeta, CT::Handle auto&) requires Mutable;

      NOD() bool Compare(DMeta, const auto&) const;

      /*template<class T1> requires CT::Comparable<T, T1>
      NOD() bool Compare(const T1&) const;
      template<class T1, bool RHS_EMBED> requires CT::Comparable<T, T1>
      NOD() bool Compare(const Handle<T1, RHS_EMBED>&) const;*/

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
      void Destroy(DMeta) const;

      /*template<bool RESET = false, bool DEALLOCATE = true>
      void DestroyUnknown(DMeta) const;*/
   };
   
   template<CT::NotHandle T>
   using HandleLocal = Handle<T, false>;

   /// Deduction guides                                                       
   /*template<CT::Sparse T>
   Handle(T&, const Allocation*&) -> Handle<T, true>;

   template<CT::Sparse T>
   Handle(const T&, const Allocation*&) -> Handle<const T, true>;

   template<CT::Dense T>
   Handle(T&, const Allocation*) -> Handle<T, true>;

   template<CT::Dense T>
   Handle(const T&, const Allocation*) -> Handle<const T, true>;

   template<CT::Dense T>
   Handle(T&) -> Handle<T, true>;

   template<CT::Dense T>
   Handle(const T&) -> Handle<const T, true>;*/

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
