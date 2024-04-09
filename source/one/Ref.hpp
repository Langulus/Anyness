///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Own.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   A shared pointer                                                     
   ///                                                                        
   ///   Provides ownership for a single pointer. Also, for single-element    
   /// containment, it is a lot more efficient than TAny. So, essentially     
   /// it's equivalent to std::shared_ptr                                     
   ///                                                                        
   template<class T>
   class Ref : public Own<T*> {
   protected:
      using Base = Own<T*>;
      using Self = Ref<T>;
      using Type = TypeOf<Base>;

      using Base::mValue;
      const Allocation* mEntry {};
   
      void ResetInner();

   public:
      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Ref() noexcept;
      constexpr Ref(const Ref&);
      constexpr Ref(Ref&&);

      template<template<class> class S>
      requires CT::Inner::SemanticMakable<S, T*>
      constexpr Ref(S<Ref>&&);

      template<class A> requires CT::MakableFrom<T*, A>
      constexpr Ref(A&&);

      ~Ref();

      template<class...A> requires ::std::constructible_from<T, A...>
      void New(A&&...);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      constexpr Ref& operator = (const Ref&);
      constexpr Ref& operator = (Ref&&);

      template<template<class> class S>
      requires CT::Inner::SemanticAssignable<S, T*>
      Ref& operator = (S<Ref>&&);

      template<CT::NotOwned A> requires CT::AssignableFrom<T*, A>
      Ref& operator = (A&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() auto GetHandle() const;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;
      
      using Base::operator bool;
      using Base::operator ->;
      using Base::operator *;

      /// Makes Own CT::Resolvable                                            
      NOD() Block GetBlock() const;

      ///                                                                     
      ///   Services                                                          
      ///                                                                     
      void Reset();

      NOD() operator Ref<const T>() const noexcept requires CT::Mutable<T>;
      //NOD() operator T* () const noexcept;
   };


   /// Deduction guides                                                       
   template<CT::Sparse T>
   Ref(T&&) -> Ref<Deptr<T>>;

} // namespace Langulus::Anyness