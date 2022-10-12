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

         // Needed so that inherited pairs can have default operator ==	
         //TODO huh?
         constexpr bool operator == (const Pair&) const noexcept {
            return true;
         }
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

         template<class K, class V>
         Pair(const K& key, const V& value)
            : mKey {key}
            , mValue {value} {}

         template<class K, class V>
         Pair(K&& key, V&& value)
            : mKey {Forward<K>(key)}
            , mValue {Forward<V>(value)} {}
      };

   } // namespace Langulus::Anyness

   namespace CT
   {

      /// Check if T is binary compatible to a pair                           
      template<class T>
      concept Pair = sizeof(T) == sizeof(Anyness::Pair)
         && DerivedFrom<T, A::Pair>;

   } // namespace Langulus::CT

} // namespace Langulus
