///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Map.hpp"
#include "../pairs/TPair.hpp"


namespace Langulus::CT
{

   /// Concept for recognizing arguments, with which a statically typed       
   /// map can be constructed                                                 
   template<class K, class V, class...A>
   concept DeepMapMakable = Inner::UnfoldMakableFrom<Anyness::TPair<K, V>, A...>
        or (sizeof...(A) == 1
           and Map<Desem<FirstOf<A...>>>
           and Inner::SemanticMakableAlt<
              typename SemanticOf<FirstOf<A...>>::template As<Anyness::TPair<K, V>>>
        );

   /// Concept for recognizing argument, with which a statically typed        
   /// map can be assigned                                                    
   template<class K, class V, class A>
   concept DeepMapAssignable = Inner::UnfoldMakableFrom<Anyness::TPair<K, V>, A>
        or (Map<Desem<A>> and Inner::SemanticAssignableAlt<
           typename SemanticOf<A>::template As<Anyness::TPair<K, V>>>);

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   /// A hashmap implementation, using the Robin Hood algorithm               
   ///                                                                        
   template<CT::Data K, CT::Data V, bool ORDERED>
   struct TMap : Map<ORDERED> {
      using Key = K;
      using Value = V;
      using Base = Map<ORDERED>;
      using Self = TMap<K, V, ORDERED>;
      using Pair = TPair<K, V>;
      using PairRef = TPair<const K&, V&>;
      using PairConstRef = TPair<const K&, const V&>;

      LANGULUS(POD) false;
      LANGULUS(TYPED) Pair;
      LANGULUS_BASES(Map<ORDERED>);

   protected:
      static_assert(CT::Inner::Comparable<K>,
         "Map's key type must be equality-comparable to itself");

      friend struct BlockMap;
      using typename Base::InfoType;
      using Base::InvalidOffset;
      using Base::mInfo;
      using Base::mKeys;
      using Base::mValues;

   public:
      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr TMap();
      TMap(const TMap&);
      TMap(TMap&&) noexcept;

      template<class T1, class...TAIL>
      requires CT::DeepMapMakable<K, V, T1, TAIL...>
      TMap(T1&&, TAIL&&...);

      ~TMap();

      TMap& operator = (const TMap&);
      TMap& operator = (TMap&&);

      template<class T1>
      requires CT::DeepMapAssignable<K, V, T1>
      TMap& operator = (T1&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetKeyType() const noexcept;
      NOD() DMeta GetValueType() const noexcept;
      NOD() constexpr bool IsKeyUntyped() const noexcept;
      NOD() constexpr bool IsValueUntyped() const noexcept;
      NOD() constexpr bool IsKeyTypeConstrained() const noexcept;
      NOD() constexpr bool IsValueTypeConstrained() const noexcept;
      NOD() constexpr bool IsKeyDeep() const noexcept;
      NOD() constexpr bool IsValueDeep() const noexcept;
      NOD() constexpr bool IsKeySparse() const noexcept;
      NOD() constexpr bool IsValueSparse() const noexcept;
      NOD() constexpr bool IsKeyDense() const noexcept;
      NOD() constexpr bool IsValueDense() const noexcept;
      NOD() constexpr Size GetKeyStride() const noexcept;
      NOD() constexpr Size GetValueStride() const noexcept;
      NOD() Count GetKeyCountDeep() const noexcept;
      NOD() Count GetKeyCountElementsDeep() const noexcept;
      NOD() Count GetValueCountDeep() const noexcept;
      NOD() Count GetValueCountElementsDeep() const noexcept;
      NOD() bool IsMissingDeep() const;

      using Base::GetCount;
      using Base::GetReserved;
      using Base::GetInfo;
      using Base::GetInfoEnd;
      using Base::IsEmpty;

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsKey() const noexcept;
      NOD() bool IsKey(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsKeySimilar() const noexcept;
      NOD() bool IsKeySimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsKeyExact() const noexcept;
      NOD() bool IsKeyExact(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsValue() const noexcept;
      NOD() bool IsValue(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsValueSimilar() const noexcept;
      NOD() bool IsValueSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsValueExact() const noexcept;
      NOD() bool IsValueExact(DMeta) const noexcept;

   protected:
      template<CT::NotSemantic, CT::NotSemantic>
      void Mutate() noexcept;
      void Mutate(DMeta, DMeta);

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() K&           GetKey(CT::Index auto);
      NOD() K const&     GetKey(CT::Index auto) const;
      NOD() V&           GetValue(CT::Index auto);
      NOD() V const&     GetValue(CT::Index auto) const;
      NOD() PairRef      GetPair(CT::Index auto);
      NOD() PairConstRef GetPair(CT::Index auto) const;

   protected:
      using Base::GetBucket;
      using Base::GetBucketUnknown;

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator = BlockMap::Iterator<TMap>;
      using ConstIterator = BlockMap::Iterator<const TMap>;

      NOD() Iterator begin() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator last() const noexcept;

      template<bool REVERSE = false>
      Count ForEach(auto&&) const;
      template<bool REVERSE = false>
      Count ForEach(auto&&);

      template<bool REVERSE = false>
      Count ForEachKeyElement(auto&&) const;
      template<bool REVERSE = false>
      Count ForEachKeyElement(auto&&);

      template<bool REVERSE = false>
      Count ForEachValueElement(auto&&) const;
      template<bool REVERSE = false>
      Count ForEachValueElement(auto&&);

      template<bool REVERSE = false>
      Count ForEachKey(auto&&...) const;
      template<bool REVERSE = false>
      Count ForEachKey(auto&&...);

      template<bool REVERSE = false>
      Count ForEachValue(auto&&...) const;
      template<bool REVERSE = false>
      Count ForEachValue(auto&&...);

      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachKeyDeep(auto&&...) const;
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachKeyDeep(auto&&...);

      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachValueDeep(auto&&...) const;
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachValueDeep(auto&&...);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (CT::Map  auto const&) const requires CT::Inner::Comparable<V>;
      bool operator == (CT::Pair auto const&) const requires CT::Inner::Comparable<V>;

      NOD() Hash GetHash() const;

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() bool ContainsKey(K1 const&) const;

      template<CT::NotSemantic V1> requires CT::Inner::Comparable<V, V1>
      NOD() bool ContainsValue(V1 const&) const;

      template<CT::Pair P> requires CT::Inner::Comparable<TPair<K, V>, P>
      NOD() bool ContainsPair(P const&) const;

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() Index Find(K1 const&) const;

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() Iterator FindIt(K1 const&);

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() ConstIterator FindIt(K1 const&) const;

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() decltype(auto) At(K1 const&);

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() decltype(auto) At(K1 const&) const;

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() decltype(auto) operator[] (K1 const&);

      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      NOD() decltype(auto) operator[] (K1 const&) const;

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class K1, class V1>
      requires (CT::MakableFrom<K, K1> and CT::MakableFrom<V, V1>)
      Count Insert(K1&&, V1&&);

      Count InsertBlock(auto&&, auto&&);

      template<class T1, class...TAIL>
      requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1, TAIL...>
      Count InsertPair(T1&&, TAIL&&...);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1>
      TMap& operator << (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1>
      TMap& operator >> (T1&&);

      TMap& operator += (const TMap&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::NotSemantic K1> requires CT::Inner::Comparable<K, K1>
      Count RemoveKey(const K1&);
      template<CT::NotSemantic V1> requires CT::Inner::Comparable<V, V1>
      Count RemoveValue(const V1&);
      template<CT::Pair P> requires CT::Inner::Comparable<TPair<K, V>, P>
      Count RemovePair(const P&);

      Iterator RemoveIt(const Iterator&);

      void Clear();
      void Reset();
      void Compact();

   protected:
      NOD() static Size RequestValuesSize(Count) noexcept;
   };

} // namespace Langulus::Anyness
