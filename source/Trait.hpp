///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Any.hpp"

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
   /// Flow::Construct, as well as parameters for any Flow::Verb call         
   ///                                                                        
   class Trait : public Any {
      LANGULUS(DEEP) false;
      LANGULUS_BASES(Any);

   private:
      TMeta mTraitType {};

   protected:
      template<class T>
      static constexpr bool NotRelated = CT::Sparse<T> || !CT::DerivedFrom<T, Trait>;
      template<class T>
      static constexpr bool Related = CT::Dense<T> && CT::DerivedFrom<T, Trait>;

   public:
      constexpr Trait() noexcept = default;

      Trait(const Trait&);
      Trait(Trait&&) noexcept;

      template<CT::NotSemantic T>
      Trait(const T&) requires Related<T>;
      template<CT::NotSemantic T>
      Trait(T&) requires Related<T>;
      template<CT::NotSemantic T>
      Trait(T&&) requires Related<T>;

      template<CT::Semantic S>
      Trait(S&&) requires Related<TypeOf<S>>;

      template<CT::NotSemantic T>
      Trait(const T&) requires NotRelated<T>;
      template<CT::NotSemantic T>
      Trait(T&) requires NotRelated<T>;
      template<CT::NotSemantic T>
      Trait(T&&) requires NotRelated<T>;

      template<CT::Semantic S>
      Trait(S&&) requires NotRelated<TypeOf<S>>;

      template<CT::Data HEAD, CT::Data... TAIL>
      Trait(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      Trait& operator = (const Trait&);
      Trait& operator = (Trait&&) noexcept;

      template<CT::NotSemantic T>
      Trait& operator = (const T&);
      template<CT::NotSemantic T>
      Trait& operator = (T&);
      template<CT::NotSemantic T>
      Trait& operator = (T&&);

      template<CT::Semantic S>
      Trait& operator = (S&&);

   public:
      template<CT::Data TRAIT, CT::Data DATA>
      NOD() static Trait From();
      template<CT::Data DATA>
      NOD() static Trait From(TMeta, const DATA&);
      template<CT::Data DATA>
      NOD() static Trait From(TMeta, DATA&&);

      template<CT::Data TRAIT, CT::Data DATA>
      NOD() static Trait From(const DATA&);
      template<CT::Data TRAIT, CT::Data DATA>
      NOD() static Trait From(DATA&&);

      NOD() static Trait FromMeta(TMeta, DMeta);

   public:
      template<CT::Data T>
      void SetTrait() noexcept;
      constexpr void SetTrait(TMeta) noexcept;

      template<CT::Data T>
      NOD() bool TraitIs() const;
      NOD() bool TraitIs(TMeta) const;

      NOD() TMeta GetTrait() const noexcept;

      NOD() bool IsTraitValid() const noexcept;
      NOD() bool IsSimilar(const Trait&) const noexcept;
      NOD() bool HasCorrectData() const;

      template<CT::Data T>
      NOD() bool operator == (const T&) const;
   };


   ///                                                                        
   ///   A statically named trait, used for integrating any custom trait by   
   /// using it as a CRTP                                                     
   ///                                                                        
   template<class TRAIT>
   struct StaticTrait : public Trait {
   public:
      LANGULUS(TRAIT) RTTI::LastNameOf<TRAIT>();
      LANGULUS_BASES(Trait);

      using TraitType = TRAIT;

      StaticTrait();

      StaticTrait(const StaticTrait&);
      StaticTrait(StaticTrait&&);

      template<CT::NotSemantic T>
      StaticTrait(const T&);
      template<CT::NotSemantic T>
      StaticTrait(T&);
      template<CT::NotSemantic T>
      StaticTrait(T&&);

      template<CT::Semantic S>
      StaticTrait(S&&);

      template<CT::Data HEAD, CT::Data... TAIL>
      StaticTrait(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      StaticTrait& operator = (const StaticTrait&);
      StaticTrait& operator = (StaticTrait&&);

      template<CT::NotSemantic T>
      StaticTrait& operator = (const T&);
      template<CT::NotSemantic T>
      StaticTrait& operator = (T&);
      template<CT::NotSemantic T>
      StaticTrait& operator = (T&&);

      template<CT::Semantic S>
      StaticTrait& operator = (S&&);

      TRAIT operator + (const Trait&) const;
      template<CT::Deep T>
      TRAIT operator + (const T&) const;

      TRAIT& operator += (const Trait&);
      template<CT::Deep T>
      TRAIT& operator += (const T&);

      template<CT::Data T>
      NOD() bool operator == (const T&) const;
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{

   /// A reflected trait type is any type that inherits Trait, and is         
   /// binary compatible to a Trait                                           
   template<class... T>
   concept Trait = ((DerivedFrom<T, ::Langulus::Anyness::Trait>
      && sizeof(T) == sizeof(::Langulus::Anyness::Trait)) && ...);

} // namespace Langulus::CT

#define LANGULUS_DEFINE_TRAIT(T, INFOSTRING) \
   namespace Langulus::Traits \
   { \
      struct T : public ::Langulus::Anyness::StaticTrait<T> { \
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
      struct T : public ::Langulus::Anyness::StaticTrait<T> { \
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
   "Count trait, used all over the place");
LANGULUS_DEFINE_TRAIT(Name,
   "Name trait, used all over the place");
LANGULUS_DEFINE_TRAIT(Index,
   "Index trait, used all over the place");
LANGULUS_DEFINE_TRAIT(Context,
   "Context trait, used to access the current environment");
LANGULUS_DEFINE_TRAIT(Trait, 
   "Accesses traits (static or dynamic) of an instantiated object of any kind");
LANGULUS_DEFINE_TRAIT(State, 
   "State trait, used all over the place");
LANGULUS_DEFINE_TRAIT(Child,
   "Accesses children in any kind of hierarchies");
LANGULUS_DEFINE_TRAIT(Parent,
   "Accesses parents in any kind of hierarchies");
LANGULUS_DEFINE_TRAIT(Clipboard,
   "Accesses clipboard");

#include "Trait.inl"
