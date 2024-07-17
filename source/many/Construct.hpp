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
      static constexpr bool Ownership = true;

      constexpr Construct() noexcept = default;
      Construct(const Construct&) noexcept;
      Construct(Construct&&) noexcept;

      template<template<class> class S> requires CT::Intent<S<Construct>>
      Construct(S<Construct>&&);

      Construct(DMeta);
      Construct(DMeta, auto&&, const Charge& = {});

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         Construct(const Token&);
         Construct(const Token&, auto&&, const Charge& = {});
      #endif

      Construct& operator = (const Construct&) noexcept;
      Construct& operator = (Construct&&) noexcept;
      template<template<class> class S> requires CT::Intent<S<Construct>>
      Construct& operator = (S<Construct>&&);

   public:
      NOD() Hash GetHash() const;

      template<CT::Data, CT::Data T1, CT::Data...TN>
      NOD() static Construct From(T1&&, TN&&...);
      template<CT::Data>
      NOD() static Construct From();

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         template<CT::Data T1, CT::Data...TN>
         NOD() static Construct FromToken(const Token&, T1&&, TN&&...);
         NOD() static Construct FromToken(const Token&);
      #endif

      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Create                                    
      NOD() bool StaticCreation(Many&) const;

   public:
      NOD() bool operator == (const Construct&) const;

      template<CT::Data>
      NOD() bool CastsTo() const;
      NOD() bool CastsTo(DMeta) const;

      template<CT::Data>
      NOD() bool Is() const;
      NOD() bool Is(DMeta) const;

      template<CT::Data>
      void SetType();
      void SetType(DMeta) noexcept;

      NOD() Neat const& GetDescriptor() const noexcept;
      NOD() Neat&       GetDescriptor()       noexcept;
      NOD() Charge const& GetCharge() const noexcept;
      NOD() Charge&       GetCharge()       noexcept;

      NOD() DMeta GetType() const noexcept;
      NOD() Token GetToken() const noexcept;
      NOD() DMeta GetProducer() const noexcept;
      NOD() bool  IsExecutable() const noexcept;
      NOD() bool  IsTyped() const noexcept;
      NOD() bool  IsUntyped() const noexcept;

      void Clear();
      void Reset();
      void ResetCharge() noexcept;

      Construct& operator <<  (auto&&);
      Construct& operator <<= (auto&&);

      ///                                                                     
      ///   Conversion                                                        
      ///                                                                     
      Count Serialize(CT::Serial auto&) const;
   };

} // namespace Langulus::Anyness
