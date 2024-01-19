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

   namespace CT
   {

      /// Check if T is a pair type                                           
      ///	@attention not a test for binary compatibility!                   
      template<class...T>
      concept Pair = (DerivedFrom<T, A::Pair> and ...);

      /// Check if a type is a statically typed pair                          
      template<class...T>
      concept TypedPair = Pair<T...> and Typed<T...>;

   } // namespace Langulus::CT

   namespace Anyness
   {

      ///                                                                     
      ///   Type-erased pair                                                  
      ///                                                                     
      struct Pair : A::Pair {
         LANGULUS_ABSTRACT() false;

         Any mKey;
         Any mValue;

         ///                                                                  
         ///   Construction & Assignment                                      
         ///                                                                  
         constexpr Pair() = default;
         Pair(Pair const&) = default;
         Pair(Pair&&) noexcept = default;

         template<class P> requires CT::Pair<Desem<P>>
         Pair(P&&);

         template<class K, class V>
         requires CT::Inner::UnfoldInsertable<K, V>
         Pair(K&&, V&&);

         Pair& operator = (Pair const&) = default;
         Pair& operator = (Pair&&) noexcept = default;
         template<class P> requires CT::Pair<Desem<P>>
         Pair& operator = (P&&);

         ///                                                                  
         ///   Capsulation                                                    
         ///                                                                  
         NOD() Hash  GetHash() const;
         NOD() DMeta GetKeyType() const noexcept;
         NOD() DMeta GetValueType() const noexcept;

         ///                                                                  
         ///   Comparison                                                     
         ///                                                                  
         bool operator == (CT::Pair auto const&) const;
      };

   } // namespace Langulus::Anyness
} // namespace Langulus
