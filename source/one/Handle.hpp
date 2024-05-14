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
      static constexpr bool TypeErased = CT::TypeErased<Decay<T>>;
      static constexpr bool Sparse     = CT::Sparse<T>;
      static constexpr bool Dense      = not Sparse;
      static constexpr bool Mutable    = CT::Mutable<T>;
      static constexpr bool CTTI_Container = true;

      static_assert(Embedded or not TypeErased or Sparse,
         "Can't have a type-erased local handle, unless it is sparse");

      using Type = Conditional<TypeErased and Dense, Byte, T>;
      using AllocType = const Allocation*;
      using ValueType = Conditional<Embedded, Type*, Type>;
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

      constexpr Handle(ValueType) noexcept requires (Embedded or Sparse);
      constexpr Handle(ValueType, EntryType) noexcept requires (Embedded or Sparse);

      constexpr Handle(auto&&) noexcept requires (not Embedded);

      // Construct from handle                                          
      template<template<class> class S, CT::Handle H>
      requires CT::SemanticMakable<S, T>
      constexpr Handle(S<H>&&);

      ~Handle();

      NOD() Handle<const T, EMBED> MakeConst() const noexcept {
         return {mValue, mEntry};
      }

      constexpr Handle& operator = (const Handle&) noexcept = default;
      constexpr Handle& operator = (Handle&&) noexcept = default;

      constexpr bool operator == (const auto&) const noexcept requires (not TypeErased or Sparse);

      NOD() Type&       Get() noexcept;
      NOD() Type const& Get() const noexcept;

      NOD() AllocType&       GetEntry() noexcept;
      NOD() AllocType const& GetEntry() const noexcept;

      void CreateSemantic(auto&&, DMeta = {});
      void Assign(const Type&, AllocType = nullptr) noexcept requires (Embedded and Mutable);
      void AssignSemantic(auto&&, DMeta = {}) requires Mutable;
      void Swap(CT::Handle auto&, DMeta = {}) requires Mutable;
      NOD() bool Compare(const auto&, DMeta = {}) const;
      template<bool RESET = false, bool DEALLOCATE = true>
      void Destroy(DMeta = {}) const requires Mutable;

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
   };
   
   template<CT::NotHandle T>
   using HandleLocal = Handle<T, false>;

} // namespace Langulus::Anyness
