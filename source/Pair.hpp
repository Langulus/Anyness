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
      ///   An abstract pair                                                  
      ///                                                                     
      struct Pair {
         LANGULUS_ABSTRACT() true;
      };

   } // namespace Langulus::A

   namespace Anyness
   {

      ///                                                                     
      ///   Type-erased pair                                                  
      ///                                                                     
      struct Pair : A::Pair {
         LANGULUS_ABSTRACT() false;

         Any mKey;
         Any mValue;

      public:
         constexpr Pair();

         template<CT::NotSemantic K, CT::NotSemantic V>
         Pair(const K&, const V&);
         template<CT::NotSemantic K, CT::NotSemantic V>
         Pair(const K&, V&&);
         template<CT::NotSemantic K, CT::NotSemantic V>
         Pair(K&&, const V&);
         template<CT::NotSemantic K, CT::NotSemantic V>
         Pair(K&&, V&&);

         template<CT::Semantic SK, CT::Semantic SV>
         Pair(SK&&, SV&&);

         NOD() Hash GetHash() const;
         NOD() DMeta GetKeyType() const noexcept;
         NOD() DMeta GetValueType() const noexcept;
      };

   } // namespace Langulus::Anyness

   namespace CT
   {

      /// Check if T is a pair type                                           
      ///	@attention not a test for binary compatibility!                   
      template<class... T>
      concept Pair = ((Dense<T> and DerivedFrom<T, A::Pair>) and ...);

      /// Check if a type is a statically typed pair                          
      template<class... T>
      concept TypedPair = ((Pair<T> and requires { typename T::Key; typename T::Value; }) and ...);

   } // namespace Langulus::CT

} // namespace Langulus
