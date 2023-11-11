///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TUnorderedMap.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   /// A highly optimized ordered hashmap implementation, using the Robin     
   /// Hood algorithm                                                         
   ///                                                                        
   template<CT::Data K, CT::Data V>
   class TOrderedMap : public TUnorderedMap<K, V> {
   public:
      using Self = TOrderedMap<K, V>;
      using Base = TUnorderedMap<K, V>;
      using typename Base::Iterator;
      using typename Base::ConstIterator;
      using Base::mKeys;
      using Base::mValues;
      using Base::mInfo;
      using Base::MinimalAllocation;
      using Base::InvalidOffset;

      static constexpr bool Ordered = true;

   public:
      constexpr TOrderedMap();
      TOrderedMap(const TOrderedMap&);
      TOrderedMap(TOrderedMap&&) noexcept;

      TOrderedMap(const CT::NotSemantic auto&);
      TOrderedMap(CT::NotSemantic auto&);
      TOrderedMap(CT::NotSemantic auto&&);
      TOrderedMap(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      TOrderedMap(T1&&, T2&&, TAIL&&...);

      TOrderedMap& operator = (const TOrderedMap&);
      TOrderedMap& operator = (TOrderedMap&&) noexcept;

      TOrderedMap& operator = (const CT::NotSemantic auto&);
      TOrderedMap& operator = (CT::NotSemantic auto&);
      TOrderedMap& operator = (CT::NotSemantic auto&&);
      TOrderedMap& operator = (CT::Semantic auto&&);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      NOD() bool ContainsKey(const K&) const;
      NOD() bool ContainsValue(const V&) const requires CT::Inner::Comparable<V>;
      NOD() bool ContainsPair(const Pair&) const requires CT::Inner::Comparable<V>;

      NOD() Index Find(const K&) const;
      NOD() Iterator FindIt(const K&);
      NOD() ConstIterator FindIt(const K&) const;

      NOD() decltype(auto) At(const K&);
      NOD() decltype(auto) At(const K&) const;

      NOD() decltype(auto) operator[] (const K&);
      NOD() decltype(auto) operator[] (const K&) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const K&, const V&);
      Count Insert(const K&,       V&&);
      Count Insert(const K&, CT::Semantic auto&&);
                   
      Count Insert(K&&, const V&);
      Count Insert(K&&,       V&&);
      Count Insert(K&&, CT::Semantic auto&&);

      Count Insert(CT::Semantic auto&&, const V&);
      Count Insert(CT::Semantic auto&&,       V&&);
      Count Insert(CT::Semantic auto&&, CT::Semantic auto&&);

      Count InsertBlock(CT::Semantic auto&&, CT::Semantic auto&&);

      Count InsertPair(const CT::Pair auto&);
      Count InsertPair(      CT::Pair auto&&);
      Count InsertPair(CT::Semantic   auto&&);

      Count InsertPairBlock(CT::Semantic auto&&);

      TOrderedMap& operator << (const CT::Pair auto&);
      TOrderedMap& operator << (      CT::Pair auto&&);
      TOrderedMap& operator << (CT::Semantic   auto&&);

      TOrderedMap& operator += (const TOrderedMap&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count RemoveKey(const K&);
      Count RemoveValue(const V&);
   };

} // namespace Langulus::Anyness
