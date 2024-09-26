///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../many/Many.hpp"
#include "../Index.hpp"
#include "../Charge.hpp"
#include "VerbState.hpp"


namespace Langulus::A
{

   ///                                                                        
   /// Abstract verb, dictating canonical verb size, used in various concepts 
   ///                                                                        
   struct Verb : Anyness::Many, Anyness::Charge {
      LANGULUS(NAME) "AVerb";
      LANGULUS(POD) false;
      LANGULUS(NULLIFIABLE) false;
      LANGULUS(DEEP) false;
      LANGULUS_BASES(Anyness::Many, Anyness::Charge);
      LANGULUS_CONVERTS_TO(Anyness::Text);
      static constexpr bool CTTI_Container = true;

   protected:
      using Real      = Langulus::Real;
      using Charge    = Anyness::Charge;
      using VMeta     = Anyness::VMeta;
      using VerbState = Anyness::VerbState;
      using Many      = Anyness::Many;

      // Verb meta, mass, rate, time and priority                       
      mutable VMeta mVerb {};
      // The number of successful executions                            
      Count mSuccesses {};
      // Verb short-circuiting                                          
      VerbState mState {};
      // Verb context                                                   
      Many mSource;
      // The container where output goes after execution                
      Many mOutput;

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

      template<template<class> class S> requires CT::Intent<S<Verb>>
      Verb(S<Verb>&&);

      ~Verb() = default;

      Verb& operator = (const Verb&);
      Verb& operator = (Verb&&);

      template<template<class> class S> requires CT::Intent<S<Verb>>
      Verb& operator = (S<Verb>&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() auto GetVerb() const noexcept -> VMeta;
      NOD() auto GetHash() const -> Hash;
      NOD() auto GetCharge() const noexcept -> const Charge&;
      NOD() auto GetMass() const noexcept -> Real;
      NOD() auto GetRate() const noexcept -> Real;
      NOD() auto GetTime() const noexcept -> Real;
      NOD() auto GetPriority() const noexcept -> Real;

      NOD() auto GetSource() noexcept -> Many&;
      NOD() auto GetSource() const noexcept -> Many const&;

      NOD() auto GetArgument() noexcept -> Many&;
      NOD() auto GetArgument() const noexcept -> Many const&;

      NOD() auto GetOutput() noexcept -> Many&;
      NOD() auto GetOutput() const noexcept -> Many const&;

      NOD() auto operator -> () noexcept -> Many*;
      NOD() auto operator -> () const noexcept -> Many const*;
      
      NOD() Count GetSuccesses() const noexcept;
      NOD() auto GetVerbState() const noexcept -> VerbState;
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
   concept VerbMakable = UnfoldInsertable<T1, TN...>
        or (sizeof...(TN) == 0 and VerbBased<Deint<T1>>);

   /// Concept for recognizing argument, with which a verb can be assigned    
   template<class A>
   concept VerbAssignable = VerbMakable<A>;

} // namespace Langulus::CT