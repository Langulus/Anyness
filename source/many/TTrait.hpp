///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Trait.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   A statically named trait, used for integrating any custom trait by   
   /// using it as a CRTP                                                     
   ///                                                                        
   template<class TRAIT>
   struct TTrait : Trait {
      LANGULUS(TRAIT) RTTI::LastCppNameOf<TRAIT>();
      LANGULUS_BASES(Trait);

      using TraitType = TRAIT;

   public:
      using Trait::Trait;
      TTrait(const TTrait&);
      TTrait(TTrait&&);

      template<CT::Data>
      NOD() static TRAIT OfType();
      NOD() static TRAIT OfType(DMeta);

      using Trait::operator =;
      TTrait& operator = (const TTrait&);
      TTrait& operator = (TTrait&&);

   public:
      template<CT::Trait, CT::Trait...>
      NOD() constexpr bool IsTrait() const;
      template<CT::Trait...TN>
      NOD() constexpr bool IsTrait(TMeta, TN...) const;

      NOD() TMeta GetTrait() const noexcept;

      NOD() constexpr bool IsTraitValid() const noexcept;
      NOD() constexpr bool IsTraitSimilar(const CT::TraitBased auto&) const noexcept;
      NOD() constexpr bool HasCorrectData() const;

      ///                                                                     
      ///   Compare                                                           
      ///                                                                     
      NOD() bool operator == (const CT::NotSemantic auto&) const;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() TRAIT  operator +  (CT::Inner::UnfoldInsertable auto&&) const;
            TRAIT& operator += (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Conversion                                                        
      ///                                                                     
      Count Serialize(CT::Serial auto&) const;

   private:
      using Trait::From;
      using Trait::FromMeta;
      using Trait::SetTrait;
   };

} // namespace Langulus::Anyness


#define LANGULUS_DEFINE_TRAIT(T, INFOSTRING) \
   namespace Langulus::Traits \
   { \
      struct T : Anyness::TTrait<T> { \
         LANGULUS(INFO) INFOSTRING; \
         using TTrait::TTrait; \
         using TTrait::operator =; \
         using TTrait::operator ==; \
         using TTrait::operator +; \
         using TTrait::operator +=; \
      }; \
   }

#define LANGULUS_DEFINE_TRAIT_WITH_PROPERTIES(T, INFOSTRING, PROPERTIES) \
   namespace Langulus::Traits \
   { \
      struct T : Anyness::TTrait<T> { \
         LANGULUS(INFO) INFOSTRING; \
         using TTrait::TTrait; \
         using TTrait::operator =; \
         using TTrait::operator ==; \
         using TTrait::operator +; \
         using TTrait::operator +=; \
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
