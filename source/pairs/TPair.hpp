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
#include "../one/Own.hpp"
#include "../one/Ref.hpp"


namespace Langulus::CT
{

   /// Concept for recognizing arguments, with which a statically typed       
   /// pair can be constructed                                                
   template<class K, class V, class A>
   concept PairMakable = Pair<Desem<A>> and NotReference<K, V>
       and (SemanticOf<A>::Shallow or (
            SemanticMakableAlt<typename SemanticOf<A>::template As<K>>
        and SemanticMakableAlt<typename SemanticOf<A>::template As<V>>));

   /// Concept for recognizing argument, with which a statically typed        
   /// pair can be assigned                                                   
   template<class K, class V, class A>
   concept PairAssignable = Pair<Desem<A>> and NotReference<K, V>
       and (SemanticOf<A>::Shallow or (
            SemanticAssignableAlt<typename SemanticOf<A>::template As<K>>
        and SemanticAssignableAlt<typename SemanticOf<A>::template As<V>>));

   /// Concept for recognizing argument, against which a pair can be compared 
   template<class K, class V, class A>
   concept PairComparable = Pair<A>
       and Comparable<K, typename A::Key>
       and Comparable<V, typename A::Value>;

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   ///   A helper structure for pairing keys and values of any type           
   ///                                                                        
   ///   This is the statically typed pair, and it can be used with           
   /// references, as well as dense or sparse values. When key or value types 
   /// are references, the TPair acts as a simple intermediate type, often    
   /// used to access elements inside maps.                                   
   ///   @attention TPair is not binary-compatible with its type-erased       
   ///      counterpart Pair                                                  
   ///                                                                        
   template<class K, class V>
   struct TPair : A::Pair {
      using Key = K;
      using Value = V;

      LANGULUS_ABSTRACT() false;
      LANGULUS(TYPED) TPair<K, V>;

      Conditional<CT::Reference<K> or CT::Dense<K>, K, Ref<Deptr<K>>> mKey;
      Conditional<CT::Reference<V> or CT::Dense<V>, V, Ref<Deptr<V>>> mValue;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      TPair() = default;
      TPair(TPair const&) = default;
      TPair(TPair&&) = default;

      template<class P> requires CT::PairMakable<K, V, P>
      TPair(P&&);

      template<class K1, class V1>
      requires (CT::MakableFrom<K, K1> and CT::MakableFrom<V, V1>
           and  CT::NotReference<K, V>)
      TPair(K1&&, V1&&);

      TPair(K&&, V&&) noexcept requires CT::Reference<K, V>;

      TPair& operator = (TPair const&) = default;
      TPair& operator = (TPair&&) = default;
      template<class P> requires CT::PairAssignable<K, V, P>
      TPair& operator = (P&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() Hash GetHash() const requires CT::Hashable<K, V>;

      Block GetKey() const noexcept;
      Block GetKey() noexcept;
      Block GetValue() const noexcept;
      Block GetValue() noexcept;

      Handle<K> GetKeyHandle();
      Handle<V> GetValueHandle();
      Handle<const K> GetKeyHandle() const;
      Handle<const V> GetValueHandle() const;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<class P> requires CT::PairComparable<K, V, P>
      bool operator == (const P&) const;

      operator TPair<const Deref<K>&, const Deref<V>&>() const noexcept
      requires CT::Reference<K, V>;

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      void Clear();
      void Reset();
   };

   /// Deduction guides                                                       
   template<class K, class V>
   TPair(K&&, V&&) -> TPair<Deref<K>, Deref<V>>;

} // namespace Langulus::Anyness
