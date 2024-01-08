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
       and Inner::SemanticMakableAlt<typename SemanticOf<A>::template As<K>>
       and Inner::SemanticMakableAlt<typename SemanticOf<A>::template As<V>>;

   /// Concept for recognizing argument, with which a statically typed        
   /// pair can be assigned                                                   
   template<class K, class V, class A>
   concept PairAssignable = Pair<Desem<A>>
       and Inner::SemanticAssignableAlt<typename SemanticOf<A>::template As<K>>
       and Inner::SemanticAssignableAlt<typename SemanticOf<A>::template As<V>>;

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   ///   A helper structure for pairing keys and values of any type           
   ///                                                                        
   template<CT::NotSemantic K, CT::NotSemantic V>
   struct TPair : A::Pair {
      using Key = K;
      using Value = V;

      LANGULUS_ABSTRACT() false;
      LANGULUS(TYPED) TPair<K, V>;

      Key   mKey;
      Value mValue;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      TPair() = default;
      TPair(TPair const&) = default;
      TPair(TPair&&) = default;

      template<class P> requires CT::PairMakable<K, V, P>
      TPair(P&&);

      template<class K1, class V1>
      requires (CT::Inner::MakableFrom<K, K1> and CT::Inner::MakableFrom<V, V1>)
      TPair(K1&&, V1&&);

      TPair& operator = (TPair const&) = default;
      TPair& operator = (TPair&&) = default;
      template<class P> requires CT::PairAssignable<K, V, P>
      TPair& operator = (P&&);

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

      operator TPair<const Deref<K>&, const Deref<V>&>() const noexcept
      requires (::std::is_reference_v<K> and ::std::is_reference_v<V>);
   };

} // namespace Langulus::Anyness
