///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Trait.hpp"

namespace Langulus::Anyness
{
   
   
   ///                                                                        
   ///   Charge, carrying the four verb dimensions                            
   ///                                                                        
   class Charge {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) false;

      // Mass of the verb                                               
      Real mMass = DefaultMass;
      // Frequency of the verb                                          
      Real mRate = DefaultRate;
      // Time of the verb                                               
      Real mTime = DefaultTime;
      // Priority of the verb                                           
      Real mPriority = DefaultPriority;

   public:
      static constexpr Real DefaultMass {1};
      static constexpr Real DefaultRate {0};
      static constexpr Real DefaultTime {0};

      static constexpr Real DefaultPriority {0};
      static constexpr Real MinPriority {-10000};
      static constexpr Real MaxPriority {+10000};

      constexpr Charge(
         Real = DefaultMass,
         Real = DefaultRate,
         Real = DefaultTime,
         Real = DefaultPriority
      ) noexcept;

      NOD() constexpr bool operator == (const Charge&) const noexcept;

      NOD() constexpr Charge operator * (const Real&) const noexcept;
      NOD() constexpr Charge operator ^ (const Real&) const noexcept;

      NOD() constexpr Charge& operator *= (const Real&) noexcept;
      NOD() constexpr Charge& operator ^= (const Real&) noexcept;

      NOD() constexpr bool IsDefault() const noexcept;
      NOD() constexpr bool IsFlowDependent() const noexcept;
      NOD() Hash GetHash() const noexcept;
      void Reset() noexcept;
   };


   ///                                                                        
   ///   Construct                                                            
   ///                                                                        
   ///   Used to contain constructor arguments for any type. It is just a     
   /// type-erased Any, but also carries a charge and a type. Also, unlike    
   /// Any, this one caches and updates its hash value on any change,         
   /// speeding up comparisons a lot.                                         
   ///                                                                        
   class Construct : public Any, public Charge {
      LANGULUS(POD) false;
      LANGULUS(NULLIFIABLE) false;
      LANGULUS(DEEP) false;
      LANGULUS_BASES(Any, Charge);

   private:
      DMeta mType {};
      mutable Hash mHash {};

   public:
      constexpr Construct() noexcept = default;
      Construct(const Construct&) noexcept;
      Construct(Construct&&) noexcept;

      template<CT::Semantic S>
      Construct(S&&) requires (CT::Exact<TypeOf<S>, Construct>);

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
         template<CT::Semantic S>
         Construct(const Token&, S&&, const Charge& = {});
      #endif

      Construct& operator = (const Construct&) noexcept;
      Construct& operator = (Construct&&) noexcept;
      template<CT::Semantic S>
      Construct& operator = (S&&) requires (CT::Exact<TypeOf<S>, Construct>);

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

   private:
      // Omit these inherited from Any                                  
      using Any::FromMeta;
      using Any::FromBlock;
      using Any::FromState;

   public:
      NOD() bool operator == (const Construct&) const;

      NOD() bool CastsTo(DMeta type) const;
      template<CT::Data T>
      NOD() bool CastsTo() const;

      NOD() bool Is(DMeta) const;
      template<CT::Data T>
      NOD() bool Is() const;

      NOD() const Any& GetArgument() const noexcept;
      NOD() Any& GetArgument() noexcept;

      NOD() const Charge& GetCharge() const noexcept;
      NOD() Charge& GetCharge() noexcept;

      NOD() DMeta GetType() const noexcept;
      NOD() Token GetToken() const noexcept;
      NOD() DMeta GetProducer() const noexcept;

      void Clear();
      void ResetCharge() noexcept;

      template<CT::Data T>
      Construct& operator << (const T&);
      template<CT::Data T>
      Construct& operator << (T&&);
      template<CT::Data T>
      Construct& operator >> (const T&);
      template<CT::Data T>
      Construct& operator >> (T&&);

      template<CT::Data T>
      Construct& operator <<= (const T&);
      template<CT::Data T>
      Construct& operator <<= (T&&);
      template<CT::Data T>
      Construct& operator >>= (const T&);
      template<CT::Data T>
      Construct& operator >>= (T&&);

      Construct& Set(const Trait&, const Offset& = 0);
      template<CT::Trait T, CT::Semantic S>
      void Set(S&&) const;

      NOD() const Trait* Get(TMeta, const Offset& = 0) const;
      template<CT::Trait T>
      NOD() const Trait* Get(const Offset& = 0) const;
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

#include "Construct.inl"
