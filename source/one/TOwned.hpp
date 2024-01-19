///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Text.hpp"


namespace Langulus
{
   namespace A
   {
      /// An abstract owned value                                             
      struct Owned {
         LANGULUS(ABSTRACT) true;
      };
   }

   namespace CT
   {
      /// Anything derived from A::Owned                                      
      template<class... T>
      concept Owned = (DerivedFrom<T, A::Owned> and ...);

      /// Anything not derived from A::Owned                                  
      template<class... T>
      concept NotOwned = CT::Data<T...> and not Owned<T...>;

      /// Any owned pointer                                                   
      template<class... T>
      concept Pointer = Owned<T...> and Sparse<TypeOf<T>...>;

      /// Anything usable to initialize a shared pointer                      
      template<class... T>
      concept PointerRelated = ((Pointer<T> or Sparse<T> or Nullptr<T>) and ...);
   }

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   An owned value, dense or sparse                                      
   ///                                                                        
   /// Provides ownership and semantics, for when you need to cleanup after a 
   /// move, for example. By default, fundamental types are not reset after a 
   /// move - wrapping them inside this ensures they are.                     
   ///   @attention this container is suboptimal for pointers, because it     
   ///              will constantly search the allocation corresponding to    
   ///              them, as it doesn't cache it in order to minimize size.   
   ///              For pointers, use Ptr or Ref instead. This doesn't really 
   ///              matter, if built without the MANAGED_MEMORY feature       
   ///                                                                        
   template<CT::Data T>
   class TOwned : public A::Owned {
   protected:
      T mValue;

   public:
      LANGULUS(NULLIFIABLE) CT::Inner::Nullifiable<T>;
      LANGULUS(POD) not CT::Sparse<T> and CT::Inner::POD<T>;
      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED) T;

      static constexpr bool Ownership = true;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr TOwned() requires CT::Inner::Defaultable<T>;
      constexpr TOwned(const TOwned&) requires CT::Inner::CopyMakable<T>;
      constexpr TOwned(TOwned&&) requires CT::Inner::MoveMakable<T>;

      template<template<class> class S>
      requires CT::Inner::SemanticMakable<S, T>
      constexpr TOwned(S<TOwned>&&);

      template<CT::NotOwned...A>
      requires ::std::constructible_from<T, A...>
      constexpr TOwned(A&&...);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      constexpr TOwned& operator = (const TOwned&) requires CT::Inner::CopyAssignable<T>;
      constexpr TOwned& operator = (TOwned&&) requires CT::Inner::MoveAssignable<T>;

      template<template<class> class S>
      requires CT::Inner::SemanticAssignable<S, T>
      constexpr TOwned& operator = (S<TOwned>&&);

      template<CT::NotOwned A> requires ::std::assignable_from<T, A>
      constexpr TOwned& operator = (A&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetType() const;
      NOD() Hash  GetHash() const requires CT::Hashable<T>;
      NOD() constexpr T const& Get() const noexcept;
      NOD() constexpr T&       Get()       noexcept;

      template<class>
      NOD() auto As() const noexcept requires CT::Sparse<T>;

      NOD() constexpr auto operator -> () const;
      NOD() constexpr auto operator -> ();

      NOD() constexpr auto& operator * () const IF_UNSAFE(noexcept)
         requires (CT::Sparse<T> and not CT::Void<Decay<T>>);
      NOD() constexpr auto& operator * ()       IF_UNSAFE(noexcept)
         requires (CT::Sparse<T> and not CT::Void<Decay<T>>);

      /// Makes TOwned CT::Resolvable                                         
      NOD() constexpr Block GetBlock() const;

      ///                                                                     
      ///   Services                                                          
      ///                                                                     
      void Reset();

      NOD() explicit constexpr operator bool() const noexcept;
      NOD() constexpr operator const T&() const noexcept;
      NOD() constexpr operator T&() noexcept;
   };


   /// Just a short handle for value with ownership                           
   /// If sparse/fundamental, value will be explicitly nulled after a move    
   template<class T>
   using Own = TOwned<T>;

   template<CT::Data T1, CT::Data T2>
   requires CT::Inner::Comparable<T1, T2> LANGULUS(INLINED)
   constexpr bool operator == (const TOwned<T1>& lhs, const TOwned<T2>& rhs) noexcept {
      return lhs.Get() == rhs.Get();
   }

   template<CT::Data T1, CT::NotOwned T2>
   requires CT::Inner::Comparable<T1, T2> LANGULUS(INLINED)
   constexpr bool operator == (const TOwned<T1>& lhs, const T2& rhs) noexcept {
      return lhs.Get() == rhs;
   }

   template<CT::Data T1, CT::NotOwned T2>
   requires CT::Inner::Comparable<T2, T1> LANGULUS(INLINED)
   constexpr bool operator == (const T2& lhs, const TOwned<T1>& rhs) noexcept {
      return lhs == rhs.Get();
   }

   template<CT::Sparse T> LANGULUS(INLINED)
   constexpr bool operator == (const TOwned<T>& lhs, ::std::nullptr_t) noexcept {
      return lhs.Get() == nullptr;
   }

   template<CT::Sparse T> LANGULUS(INLINED)
   constexpr bool operator == (::std::nullptr_t, const TOwned<T>& rhs) noexcept {
      return rhs.Get() == nullptr;
   }

} // namespace Langulus::Anyness

namespace fmt
{

   ///                                                                        
   /// Extend FMT to be capable of logging any owned values                   
   ///                                                                        
   template<Langulus::CT::Owned T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT> LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace Langulus;

         if constexpr (CT::Sparse<TypeOf<T>>) {
            if (element == nullptr) {
               const auto type = element.GetType();
               if (type)
                  return fmt::format_to(ctx.out(), "{}(null)", *type);
               else
                  return fmt::format_to(ctx.out(), "null");
            }
            else return fmt::format_to(ctx.out(), "{}", *element.Get());
         }
         else {
            static_assert(CT::Dense<decltype(element.Get())>,
               "T not dense, but not sparse either????");
            return fmt::format_to(ctx.out(), "{}", element.Get());
         }
      }
   };

} // namespace fmt