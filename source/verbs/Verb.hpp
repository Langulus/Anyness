///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../many/Any.hpp"
#include "../Index.hpp"
#include "../Charge.hpp"
#include "VerbState.hpp"


namespace Langulus::A
{

   ///                                                                        
   /// Abstract verb, dictating canonical verb size, used in various concepts 
   ///                                                                        
   struct Verb : Anyness::Any, Anyness::Charge {
      LANGULUS(NAME) "AVerb";
      LANGULUS(POD) false;
      LANGULUS(NULLIFIABLE) false;
      LANGULUS(DEEP) false;
      LANGULUS_BASES(Anyness::Any, Anyness::Charge);
      LANGULUS_CONVERTS_TO(Anyness::Text);

   protected:
      // Verb meta, mass, rate, time and priority                       
      mutable VMeta mVerb {};
      // The number of successful executions                            
      Count mSuccesses {};
      // Verb short-circuiting                                          
      Anyness::VerbState mState {};
      // Verb context                                                   
      Anyness::Any mSource;
      // The container where output goes after execution                
      Anyness::Any mOutput;

      LANGULUS_MEMBERS(
         &Verb::mVerb,
         &Verb::mState,
         &Verb::mSource
      );

   public:
      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr Verb() noexcept = default;
      Verb(const Verb&);
      Verb(Verb&&);

      template<template<class> class S>
      requires CT::Semantic<S<Verb>>
      Verb(S<Verb>&&);

      ~Verb() {}

      Verb& operator = (const Verb&);
      Verb& operator = (Verb&&);

      template<template<class> class S>
      requires CT::Semantic<S<Verb>>
      Verb& operator = (S<Verb>&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() Hash GetHash() const;
      NOD() const Anyness::Charge& GetCharge() const noexcept;
      NOD() Langulus::Real GetMass() const noexcept;
      NOD() Langulus::Real GetRate() const noexcept;
      NOD() Langulus::Real GetTime() const noexcept;
      NOD() Langulus::Real GetPriority() const noexcept;

      NOD() Anyness::Any&       GetSource() noexcept;
      NOD() Anyness::Any const& GetSource() const noexcept;

      NOD() Anyness::Any&       GetArgument() noexcept;
      NOD() Anyness::Any const& GetArgument() const noexcept;

      NOD() Anyness::Any&       GetOutput() noexcept;
      NOD() Anyness::Any const& GetOutput() const noexcept;

      NOD() Anyness::Any*       operator -> () noexcept;
      NOD() Anyness::Any const* operator -> () const noexcept;
      
      NOD() Count GetSuccesses() const noexcept;
      NOD() Anyness::VerbState GetVerbState() const noexcept;
      NOD() bool IsDone() const noexcept;

      NOD() constexpr bool IsMulticast() const noexcept;
      NOD() constexpr bool IsMonocast() const noexcept;
      NOD() constexpr bool IsShortCircuited() const noexcept;
      NOD() constexpr bool IsLongCircuited() const noexcept;

      NOD() bool IsMissing() const noexcept;
      NOD() bool IsMissingDeep() const noexcept;
      NOD() bool Validate(Anyness::Index) const noexcept;

      void Done(Count) noexcept;
      void Done() noexcept;
      void Undo() noexcept;

      NOD() explicit operator Anyness::Text() const;

   protected:
      void SerializeVerb(CT::Serial auto&) const;

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      NOD() bool operator == (const Verb&) const;
      NOD() bool operator == (VMeta) const noexcept;

      NOD() bool operator <  (const Verb&) const noexcept;
      NOD() bool operator >  (const Verb&) const noexcept;

      NOD() bool operator <= (const Verb&) const noexcept;
      NOD() bool operator >= (const Verb&) const noexcept;

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      void Reset();
   };

} // namespace Langulus::A


namespace Langulus::CT
{
   
   /// A VerbBased type is any type that inherits A::Verb                     
   template<class...T>
   concept VerbBased = (DerivedFrom<T, A::Verb> and ...);

   /// A reflected verb type is any type that inherits A::Verb, is binary     
   /// compatible to it, and is reflected as a verb                           
   template<class...T>
   concept Verb = VerbBased<T...> and ((
      sizeof(T) == sizeof(A::Verb) and (
         requires {
            {Decay<T>::CTTI_Verb} -> Similar<Token>;
         } or requires {
            {Decay<T>::CTTI_PositiveVerb} -> Similar<Token>;
            {Decay<T>::CTTI_NegativeVerb} -> Similar<Token>;
         }
      )) and ...);

   /// Concept for recognizing arguments, with which a verb can be constructed
   template<class T1, class...TN>
   concept VerbMakable = Inner::UnfoldInsertable<T1, TN...>
        or (sizeof...(TN) == 0 and VerbBased<Desem<T1>>);

   /// Concept for recognizing argument, with which a verb can be assigned    
   template<class A>
   concept VerbAssignable = VerbMakable<A>;

} // namespace Langulus::CT