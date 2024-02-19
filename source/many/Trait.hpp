///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Any.hpp"


namespace Langulus
{
   namespace A
   {

      ///                                                                     
      /// An abstract Trait structure                                         
      /// It defines the size for CT::Trait and CT::TraitBased concepts       
      ///                                                                     
      struct Trait : Anyness::Any {
         LANGULUS(ABSTRACT) true;
         LANGULUS(DEEP) false;
         LANGULUS_BASES(Any);

      protected:
         mutable TMeta mTraitType {};
      };

   } // namespace Langulus::A

   namespace CT
   {

      /// A TraitBased type is any type that inherits A::Trait, and is binary 
      /// compatible to it                                                    
      template<class...T>
      concept TraitBased = (DerivedFrom<T, A::Trait> and ...);

      /// A reflected trait type is any type that inherits Trait, is not      
      /// Trait itself, and is binary compatible to a Trait                   
      template<class...T>
      concept Trait = TraitBased<T...> and ((
            sizeof(T) == sizeof(A::Trait)
            and requires { {Decay<T>::CTTI_Trait} -> Similar<Token>; }
         ) and ...);

   } // namespace Langulus::CT

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Trait                                                                
   ///                                                                        
   ///   A named container, used to give containers a standard intent of use  
   ///   A count is a count, no matter how you call it. So when your type     
   /// contains a count variable, you can tag it with a Traits::Count tag     
   ///   Traits are used to access members of objects at runtime, or access   
   /// global objects, or supply paremeters                                   
   ///                                                                        
   struct Trait : A::Trait {
      LANGULUS(ABSTRACT) false;
      LANGULUS_BASES(A::Trait);

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr Trait() noexcept = default;
      Trait(const Trait&);
      Trait(Trait&&) noexcept;

      template<class T1, class...TN>
      requires CT::Inner::UnfoldInsertable<T1, TN...>
      Trait(T1&&, TN&&...);

      Trait& operator = (const Trait&);
      Trait& operator = (Trait&&);
      Trait& operator = (CT::Inner::UnfoldInsertable auto&&);

      template<CT::Trait, CT::Data>
      NOD() static Trait From();
      NOD() static Trait FromMeta(TMeta, DMeta);

      template<CT::Trait>
      NOD() static Trait From(auto&&);
      NOD() static Trait From(TMeta, auto&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      template<CT::Trait>
      void SetTrait() noexcept;
      void SetTrait(TMeta) noexcept;

      template<CT::Trait, CT::TraitBased = Trait>
      NOD() constexpr bool IsTrait() const;

      template<CT::TraitBased = Trait, class...TN>
      requires CT::Exact<TMeta, TMeta, TN...>
      NOD() bool IsTrait(TMeta, TN...) const;

      template<CT::TraitBased = Trait>
      NOD() TMeta GetTrait() const noexcept;

      template<CT::TraitBased = Trait>
      NOD() bool IsTraitValid() const noexcept;

      template<CT::TraitBased = Trait>
      NOD() bool IsTraitSimilar(const CT::TraitBased auto&) const noexcept;

      template<CT::TraitBased = Trait>
      NOD() bool HasCorrectData() const;

      ///                                                                     
      ///   Compare                                                           
      ///                                                                     
      template<CT::TraitBased = Trait>
      NOD() bool operator == (const CT::NotSemantic auto&) const;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<CT::TraitBased THIS = Trait>
      NOD() THIS operator + (CT::Inner::UnfoldInsertable auto&&) const;

      template<CT::TraitBased THIS = Trait>
      THIS& operator += (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Conversion                                                        
      ///                                                                     
      template<CT::TraitBased = Trait>
      Count Serialize(CT::Serial auto&) const;
   };

} // namespace Langulus::Anyness