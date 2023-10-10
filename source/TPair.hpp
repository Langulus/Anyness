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


namespace Langulus::Anyness
{

   ///                                                                        
   ///   A helper structure for pairing keys and values of any type           
   ///                                                                        
   template<CT::Data K, CT::Data V>
   struct TPair : A::Pair {
      using Key = K;
      using Value = V;

      Key mKey;
      Value mValue;

      TPair() = default;
      TPair(TPair const&) = default;
      TPair(TPair&&) noexcept = default;

      TPair(K, V) requires (not CT::Decayed<K, V>);

      TPair(K const&, V const&)
      requires (CT::Decayed<K, V> and CT::CopyMakable<K, V>);

      TPair(K const&, V&&)
      requires (CT::Decayed<K, V> and CT::CopyMakable<K> and CT::MoveMakable<V>);

      TPair(K const&, CT::Semantic auto&&)
      requires (CT::Decayed<K, V> and CT::CopyMakable<K>);

      TPair(K&&, V const&)
      requires (CT::Decayed<K, V> and CT::MoveMakable<K> and CT::CopyMakable<V>);

      TPair(K&&, V&&)
      requires (CT::Decayed<K, V> and CT::MoveMakable<K, V>);

      TPair(K&&, CT::Semantic auto&&)
      requires (CT::Decayed<K, V> and CT::MoveMakable<K>);

      TPair(CT::Semantic auto&&, V const&)
      requires (CT::Decayed<K, V> and CT::CopyMakable<V>);

      TPair(CT::Semantic auto&&, V&&)
      requires (CT::Decayed<K, V> and CT::MoveMakable<V>);

      TPair(CT::Semantic auto&&, CT::Semantic auto&&) requires CT::Decayed<K, V>;

      constexpr void Swap(TPair&) noexcept requires CT::SwappableNoexcept<K, V>;
      constexpr void Swap(TPair&) requires CT::Swappable<K, V>;

      TPair Clone() const;

      bool operator == (TPair const&) const;

      const TPair* operator -> () const noexcept { return this; }
      TPair* operator -> () noexcept { return this; }

      TPair& operator = (TPair const&) = default;
      TPair& operator = (TPair&&) noexcept = default;

      /// Implicit casts to intermediate types                                
      constexpr operator TPair<Deref<K> const&, Deref<V> const&>() const noexcept;
      constexpr operator TPair<Deref<K>&,       Deref<V>&>      () const noexcept requires CT::Mutable<K, V>;
      constexpr operator TPair<Deref<K> const&, Deref<V>&>      () const noexcept requires CT::Mutable<V>;
      constexpr operator TPair<Deref<K>&,       Deref<V> const&>() const noexcept requires CT::Mutable<K>;

      NOD() Hash GetHash() const;
      NOD() DMeta GetKeyType() const noexcept;
      NOD() DMeta GetValueType() const noexcept;
   };

} // namespace Langulus::Anyness
