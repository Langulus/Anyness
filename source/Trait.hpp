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

      /// A reflected trait type is any type that inherits Trait, is not Trait
      /// itself, and is binary compatible to a Trait                         
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
   /// global objects, or supply paremeters for content desciptors, such as   
   /// Construct, as well as parameters for any Flow::Verb call               
   ///                                                                        
   struct Trait : A::Trait {
      LANGULUS(ABSTRACT) false;
      LANGULUS_BASES(A::Trait);

   public:
      constexpr Trait() noexcept = default;
      Trait(const Trait&);
      Trait(Trait&&) noexcept;

      template<class T1, class...TAIL>
      requires CT::Inner::UnfoldInsertable<T1, TAIL...>
      Trait(T1&&, TAIL&&...);

      Trait& operator = (const Trait&);
      Trait& operator = (Trait&&);
      Trait& operator = (CT::Inner::UnfoldInsertable auto&&);

   public:
      template<CT::Trait, CT::Data>
      NOD() static Trait From();
      NOD() static Trait FromMeta(TMeta, DMeta);

      template<CT::Trait>
      NOD() static Trait From(auto&&);
      NOD() static Trait From(TMeta, auto&&);

   public:
      template<CT::Trait>
      void SetTrait() noexcept;
      void SetTrait(TMeta) noexcept;

      template<CT::Trait, CT::Trait...>
      NOD() bool TraitIs() const;

      template<class T1, class...TN>
      requires CT::Exact<TMeta, T1, TN...>
      NOD() bool TraitIs(T1, TN...) const;

      NOD() TMeta GetTrait() const noexcept;

      NOD() bool IsTraitValid() const noexcept;
      NOD() bool IsTraitSimilar(const CT::TraitBased auto&) const noexcept;
      NOD() bool HasCorrectData() const;

      NOD() bool operator == (const auto&) const;

   public:
      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Trait operator + (CT::Inner::UnfoldInsertable auto&&) const;
      Trait& operator += (CT::Inner::UnfoldInsertable auto&&);
   };


   ///                                                                        
   ///   A statically named trait, used for integrating any custom trait by   
   /// using it as a CRTP                                                     
   ///                                                                        
   template<class TRAIT>
   struct StaticTrait : Trait {
      LANGULUS(TRAIT) RTTI::LastCppNameOf<TRAIT>();
      LANGULUS_BASES(Trait);

      using TraitType = TRAIT;

   public:
      using Trait::Trait;
      StaticTrait(const StaticTrait&);
      StaticTrait(StaticTrait&&);

      template<CT::Data>
      NOD() static TRAIT OfType();

      using Trait::operator =;
      StaticTrait& operator = (const StaticTrait&);
      StaticTrait& operator = (StaticTrait&&);

   public:
      template<CT::Trait, CT::Trait...>
      NOD() constexpr bool TraitIs() const;

      template<class T1, class...TN>
      requires CT::Exact<TMeta, T1, TN...>
      NOD() constexpr bool TraitIs(T1, TN...) const;

      NOD() TMeta GetTrait() const noexcept;

      NOD() constexpr bool IsTraitValid() const noexcept;
      NOD() constexpr bool IsTraitSimilar(const CT::TraitBased auto&) const noexcept;
      NOD() constexpr bool HasCorrectData() const;

   private:
      using Trait::SetTrait;
   };

} // namespace Langulus::Anyness


#define LANGULUS_DEFINE_TRAIT(T, INFOSTRING) \
   namespace Langulus::Traits \
   { \
      struct T : Anyness::StaticTrait<T> { \
         LANGULUS(INFO) INFOSTRING; \
         using StaticTrait::StaticTrait; \
         using StaticTrait::operator =; \
         using StaticTrait::operator ==; \
         using StaticTrait::operator +; \
         using StaticTrait::operator +=; \
      }; \
   }

#define LANGULUS_DEFINE_TRAIT_WITH_PROPERTIES(T, INFOSTRING, PROPERTIES) \
   namespace Langulus::Traits \
   { \
      struct T : Anyness::StaticTrait<T> { \
         LANGULUS(INFO) INFOSTRING; \
         using StaticTrait::StaticTrait; \
         using StaticTrait::operator =; \
         using StaticTrait::operator ==; \
         using StaticTrait::operator +; \
         using StaticTrait::operator +=; \
         PROPERTIES; \
      }; \
   }

LANGULUS_DEFINE_TRAIT(Logger,
   "Logger trait, used to access the logger instance");
LANGULUS_DEFINE_TRAIT(Count,
   "Count trait, used to access container size, or other similar properties");
LANGULUS_DEFINE_TRAIT(Name,
   "Name trait, used to access names, or other similar properties");
LANGULUS_DEFINE_TRAIT(Path,
   "Path trait, used to access files and folders, or other file-system related stuff");
LANGULUS_DEFINE_TRAIT(Data,
   "Raw data trait, used to access raw container data, or other similar properties");
LANGULUS_DEFINE_TRAIT(Index,
   "Index trait, used to access the index of elements, or other similar properties");
LANGULUS_DEFINE_TRAIT(Context,
   "Context trait, used to access verb source, the current environment, or other similar properties");
LANGULUS_DEFINE_TRAIT(Trait, 
   "Accesses traits (static or dynamic variables) of an instantiated object of any kind");
LANGULUS_DEFINE_TRAIT(State, 
   "State trait, used to access the state of an object");
LANGULUS_DEFINE_TRAIT(Child,
   "Accesses children in any kind of hierarchy");
LANGULUS_DEFINE_TRAIT(Parent,
   "Accesses parents in any kind of hierarchy");
LANGULUS_DEFINE_TRAIT(Clipboard,
   "Accesses the system clipboard");
LANGULUS_DEFINE_TRAIT(Color,
   "Accesses associated color properties");
LANGULUS_DEFINE_TRAIT(Min,
   "Accesses smallest element in a container, or in other similar contexts");
LANGULUS_DEFINE_TRAIT(Max,
   "Accesses biggest element in a container, or in other similar contexts");
LANGULUS_DEFINE_TRAIT(Input,
   "For accessing verb arguments, or general inputs of some operation");
LANGULUS_DEFINE_TRAIT(Output,
   "For accessing the outputs of a verb, or general output of some operation");
