///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Neat.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Construct                                                            
   ///                                                                        
   ///   Used to contain constructor arguments for any type. It is just a     
   /// type-erased Neat, but also carries a charge and a type. It is often    
   /// used in Verbs::Create to provide instructions on how to instantiate a  
   /// data type.                                                             
   ///                                                                        
   class Construct {
   private:
      // What are we constructing?                                      
      DMeta  mType {};
      // What properties does the thing have?                           
      Neat   mDescriptor;
      // How many things, when, at what frequency/priority?             
      Charge mCharge;

   public:
      constexpr Construct() noexcept = default;
      Construct(const Construct&) noexcept;
      Construct(Construct&&) noexcept;

      template<template<class> class S>
      Construct(S<Construct>&&) requires CT::Semantic<S<Construct>>;

      Construct(DMeta);

      template<CT::NotSemantic T = Any>
      Construct(DMeta, const T&, const Charge& = {});
      template<CT::NotSemantic T = Any>
      Construct(DMeta, T&, const Charge& = {});
      template<CT::NotSemantic T = Any>
      Construct(DMeta, T&&, const Charge& = {});
      template<CT::Semantic S>
      Construct(DMeta, S&&, const Charge& = {});

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         Construct(const Token&);
         template<CT::NotSemantic T = Any>
         Construct(const Token&, const T&, const Charge& = {});
         template<CT::NotSemantic T = Any>
         Construct(const Token&, T&, const Charge& = {});
         template<CT::NotSemantic T = Any>
         Construct(const Token&, T&&, const Charge& = {});
         Construct(const Token&, CT::Semantic auto&&, const Charge& = {});
      #endif

      Construct& operator = (const Construct&) noexcept;
      Construct& operator = (Construct&&) noexcept;
      template<template<class> class S>
      Construct& operator = (S<Construct>&&) requires CT::Semantic<S<Construct>>;

   public:
      NOD() Hash GetHash() const;

      template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
      NOD() static Construct From(HEAD&&, TAIL&&...);
      template<CT::Data T>
      NOD() static Construct From();

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         template<CT::Data HEAD, CT::Data... TAIL>
         NOD() static Construct FromToken(const Token&, HEAD&&, TAIL&&...);
         NOD() static Construct FromToken(const Token&);
      #endif

      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Create                                    
      NOD() bool StaticCreation(Any&) const;

   public:
      NOD() bool operator == (const Construct&) const;

      NOD() bool CastsTo(DMeta type) const;
      template<CT::Data T>
      NOD() bool CastsTo() const;

      NOD() bool Is(DMeta) const;
      template<CT::Data T>
      NOD() bool Is() const;

      NOD() const Neat& GetDescriptor() const noexcept;
      NOD()       Neat& GetDescriptor()       noexcept;
      NOD() const Charge& GetCharge() const noexcept;
      NOD()       Charge& GetCharge()       noexcept;

      NOD() DMeta GetType() const noexcept;
      NOD() Token GetToken() const noexcept;
      NOD() DMeta GetProducer() const noexcept;

      void Clear();
      void Reset();
      void ResetCharge() noexcept;

      // Intentionally left undefined                                   
      template<CT::TextBased T>
      NOD() T SerializeAs() const;

      // Intentionally left undefined                                   
      NOD() explicit operator Text() const;

      Construct& operator << (auto&&);
      Construct& operator <<= (auto&&);
   };

} // namespace Langulus::Anyness

LANGULUS_DEFINE_TRAIT(Mass,
   "Mass of anything with charge, amplitude, or literally physical mass");
LANGULUS_DEFINE_TRAIT(Rate,
   "Rate of anything with charge, or with physical frequency");
LANGULUS_DEFINE_TRAIT(Time,
   "Time of anything with charge, or with a temporal component");
LANGULUS_DEFINE_TRAIT(Priority,
   "Priority of anything with charge, or some kind of priority");
