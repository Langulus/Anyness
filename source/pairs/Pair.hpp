///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../many/Many.hpp"


namespace Langulus
{
   namespace A
   {

      ///                                                                     
      ///   An abstract pair                                                  
      ///                                                                     
      struct Pair {
         LANGULUS_ABSTRACT() true;
         static constexpr bool CTTI_Container = true;
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
      ///   A helper structure for pairing keys and values of any type           
      ///                                                                        
      ///   This is the type-erased pair, and it can contain dense or sparse     
      /// values.                                                                
      ///   @attention Pair is not binary-compatible with its statically typed   
      ///      counterpart TPair                                                 
      ///                                                                        
      struct Pair : A::Pair {
         LANGULUS_ABSTRACT() false;

         Many mKey;
         Many mValue;

         ///                                                                  
         ///   Construction & Assignment                                      
         ///                                                                  
         constexpr Pair() = default;
         Pair(Pair const&) = default;
         Pair(Pair&&) noexcept = default;

         template<class P> requires CT::Pair<Deint<P>>
         Pair(P&&);

         template<class K, class V> requires CT::UnfoldInsertable<K, V>
         Pair(K&&, V&&);

         Pair& operator = (Pair const&) = default;
         Pair& operator = (Pair&&) noexcept = default;
         template<class P> requires CT::Pair<Deint<P>>
         Pair& operator = (P&&);

         ///                                                                  
         ///   Capsulation                                                    
         ///                                                                  
         NOD() Hash GetHash() const;

         Many const& GetKeyBlock() const noexcept;
         Many&       GetKeyBlock() noexcept;
         Many const& GetValueBlock() const noexcept;
         Many&       GetValueBlock() noexcept;

         ///                                                                  
         ///   Comparison                                                     
         ///                                                                  
         bool operator == (CT::Pair auto const&) const;

         ///                                                                  
         ///   Removal                                                        
         ///                                                                  
         void Clear();
         void Reset();
      };

   } // namespace Langulus::Anyness
} // namespace Langulus
