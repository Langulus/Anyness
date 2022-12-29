///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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
      ///   A helper structure for pairing keys and values of any type        
      ///                                                                     
      struct Pair : public A::Pair {
         LANGULUS_ABSTRACT() false;

         Any mKey;
         Any mValue;

      public:
         template<class K, class V>
         Pair(const K&, const V&);

         template<class K, class V>
         Pair(K&&, V&&);

         NOD() Hash GetHash() const;
      };

   } // namespace Langulus::Anyness

   namespace CT
   {

      /// Check if T is a pair                           
      ///	@attention not a test for binary compatibility!
      template<class T>
      concept Pair = Dense<T> && DerivedFrom<T, A::Pair>;

   } // namespace Langulus::CT

} // namespace Langulus

#include "Pair.inl"