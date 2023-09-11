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
      TPair(const TPair&) = default;
      TPair(TPair&&) noexcept = default;

      constexpr TPair(K, V) noexcept requires (not CT::Decayed<K, V>);
      constexpr TPair(K&&, V&&) requires (CT::Decayed<K, V> and CT::MoveMakable<K, V>);
      constexpr TPair(const K&, const V&) requires (CT::Decayed<K, V> and CT::CopyMakable<K, V>);

      constexpr void Swap(TPair&) noexcept requires CT::SwappableNoexcept<K, V>;
      constexpr void Swap(TPair&) requires CT::Swappable<K, V>;

      TPair Clone() const;

      bool operator == (const TPair&) const;

      const TPair* operator -> () const noexcept { return this; }
      TPair* operator -> () noexcept { return this; }

      TPair& operator = (const TPair&) = default;
      TPair& operator = (TPair&&) noexcept = default;

      NOD() Hash GetHash() const;
      NOD() DMeta GetKeyType() const noexcept;
      NOD() DMeta GetValueType() const noexcept;
   };

} // namespace Langulus::Anyness
