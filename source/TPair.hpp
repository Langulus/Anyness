///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Pair.hpp"


namespace Langulus::CT
{

   /// Concept for recognizing arguments, with which a statically typed       
   /// pair can be constructed                                                
   template<class K, class V, class A>
   concept PairMakable = Pair<Desem<A>>
       and (SemanticOf<A>::Shallow or (
               Inner::SemanticMakableAlt<typename SemanticOf<A>::template As<K>>
           and Inner::SemanticMakableAlt<typename SemanticOf<A>::template As<V>>
       ));

   /// Concept for recognizing argument, with which a statically typed        
   /// pair can be assigned                                                   
   template<class K, class V, class A>
   concept PairAssignable = Pair<Desem<A>>
       and (SemanticOf<A>::Shallow or (
               Inner::SemanticAssignableAlt<typename SemanticOf<A>::template As<K>>
           and Inner::SemanticAssignableAlt<typename SemanticOf<A>::template As<V>>
       ));

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   ///   A helper structure for pairing keys and values of any type           
   ///                                                                        
   template<CT::Data K, CT::Data V>
   struct TPair : A::Pair {
      using Key = K;
      using Value = V;

      LANGULUS(TYPED) TPair<K, V>;

      Key   mKey;
      Value mValue;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      TPair() = default;
      TPair(TPair const&) = default;
      TPair(TPair&&) noexcept = default;

      template<class P> requires CT::PairMakable<K, V, P>
      TPair(P&&);

      template<class K1, class V1>
      requires (CT::Inner::MakableFrom<K, K1> and CT::Inner::MakableFrom<V, V1>)
      TPair(K1&&, V1&&);

      TPair& operator = (TPair const&) = default;
      TPair& operator = (TPair&&) noexcept = default;
      template<class P> requires CT::PairAssignable<K, V, P>
      TPair& operator = (P&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() Hash  GetHash() const;
      NOD() DMeta GetKeyType() const noexcept;
      NOD() DMeta GetValueType() const noexcept;

      TPair const* operator -> () const noexcept { return this; }
      TPair*       operator -> () noexcept       { return this; }

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (CT::Pair auto const&) const;
   };

} // namespace Langulus::Anyness
