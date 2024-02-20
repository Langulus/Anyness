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
#include "VerbState.inl"


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
      LANGULUS_BASES(Any, Charge);

   protected:
      // Verb meta, mass, rate, time and priority                       
      mutable VMeta mVerb {};
      // The number of successful executions                            
      Count mSuccesses {};
      // Verb short-circuiting                                          
      Anyness::VerbState mState {};
      // Verb context                                                   
      Any mSource;
      // The container where output goes after execution                
      Any mOutput;

      LANGULUS_MEMBERS(
         &Verb::mVerb,
         &Verb::mState,
         &Verb::mSource
      );
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