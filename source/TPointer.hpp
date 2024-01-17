///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOwned.hpp"
#include "inner/Handle.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   A shared pointer                                                     
   ///                                                                        
   ///   Provides ownership and referencing. Also, for single-element         
   /// containment, it is a lot more efficient than TAny. So, essentially     
   /// it's equivalent to std::shared_ptr                                     
   ///                                                                        
   template<class T, bool DR>
   class TPointer : public TOwned<T*> {
   protected:
      using Base = TOwned<T*>;
      using Self = TPointer<T, DR>;
      using Type = TypeOf<Base>;

      using Base::mValue;
      const Allocation* mEntry {};
   
      void ResetInner();

   public:
      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr TPointer() noexcept;
      constexpr TPointer(const TPointer&);
      constexpr TPointer(TPointer&&);

      template<template<class> class S>
      requires CT::Inner::SemanticMakable<S, T*>
      constexpr TPointer(S<TPointer>&&);

      template<class A> requires CT::Inner::MakableFrom<T*, A>
      constexpr TPointer(A&&);

      ~TPointer();

      template<class...A> requires ::std::constructible_from<T, A...>
      void New(A&&...);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      constexpr TPointer& operator = (const TPointer&);
      constexpr TPointer& operator = (TPointer&&);

      template<template<class> class S>
      requires CT::Inner::SemanticAssignable<S, T*>
      TPointer& operator = (S<TPointer>&&);

      template<CT::NotOwned A> requires CT::Inner::AssignableFrom<T*, A>
      TPointer& operator = (A&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() auto GetHandle() const;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;
      
      using Base::operator bool;
      using Base::operator ->;
      using Base::operator *;

      /// Makes TOwned CT::Resolvable                                         
      NOD() Block GetBlock() const;

      ///                                                                     
      ///   Services                                                          
      ///                                                                     
      void Reset();

      NOD() operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T>;
      NOD() operator const T& () const noexcept;
   };

} // namespace Langulus::Anyness

/*namespace fmt
{
   
   ///                                                                        
   /// Extend FMT to be capable of logging any shared pointers                
   ///                                                                        
   template<Langulus::CT::Pointer T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         if (not element) {
            const auto type = element.GetType();
            if (type)
               return fmt::format_to(ctx.out(), "{}(null)", type);
            else
               return fmt::format_to(ctx.out(), "null");
         }
         else return fmt::format_to(ctx.out(), "{}", *element.Get());
      }
   };

} */// namespace fmt