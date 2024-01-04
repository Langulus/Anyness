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
#include "TPair.hpp"


namespace Langulus::CT
{

   /// Concept for recognizing arguments, with which a statically typed       
   /// map can be constructed                                                 
   template<class K, class V, class...A>
   concept DeepMapMakable = Inner::UnfoldMakableFrom<Anyness::TPair<K, V>, A...>
        or (sizeof...(A) == 1 and Map<Desem<FirstOf<A...>>>
           and (SemanticOf<FirstOf<A...>>::Shallow
             or Inner::SemanticMakableAlt<typename SemanticOf<FirstOf<A...>>::template As<Anyness::TPair<K, V>>>
        ));

   /// Concept for recognizing argument, with which a statically typed        
   /// map can be assigned                                                    
   template<class K, class V, class A>
   concept DeepMapAssignable = Inner::UnfoldMakableFrom<Anyness::TPair<K, V>, A> or (Map<Desem<A>>
       and (SemanticOf<A>::Shallow
         or Inner::SemanticAssignableAlt<typename SemanticOf<A>::template As<Anyness::TPair<K, V>>>
       ));

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   /// A hashmap implementation, using the Robin Hood algorithm               
   ///                                                                        
   template<CT::Data K, CT::Data V, bool ORDERED>
   struct TMap : Map<ORDERED> {
      friend struct BlockMap;
      using Key = K;
      using Value = V;
      using Base = Map<ORDERED>;
      using Self = TMap<K, V, ORDERED>;
      using Pair = TPair<K, V>;
      using PairRef = TPair<K&, V&>;
      using PairConstRef = TPair<const K&, const V&>;

      LANGULUS(TYPED) Pair;

   protected:
      static_assert(CT::Inner::Comparable<K>,
         "Map's key type must be equality-comparable to itself");

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

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetKeyType() const;
      NOD() DMeta GetValueType() const;

      NOD() constexpr bool IsKeyUntyped() const noexcept;
      NOD() constexpr bool IsValueUntyped() const noexcept;

      NOD() constexpr bool IsKeyTypeConstrained() const noexcept;
      NOD() constexpr bool IsValueTypeConstrained() const noexcept;

      NOD() constexpr bool IsKeyAbstract() const noexcept;
      NOD() constexpr bool IsValueAbstract() const noexcept;

      NOD() constexpr bool IsKeyConstructible() const noexcept;
      NOD() constexpr bool IsValueConstructible() const noexcept;

      NOD() constexpr bool IsKeyDeep() const noexcept;
      NOD() constexpr bool IsValueDeep() const noexcept;

      NOD() constexpr bool IsKeySparse() const noexcept;
      NOD() constexpr bool IsValueSparse() const noexcept;

      NOD() constexpr bool IsKeyDense() const noexcept;
      NOD() constexpr bool IsValueDense() const noexcept;

      NOD() constexpr Size GetKeyStride() const noexcept;
      NOD() constexpr Size GetValueStride() const noexcept;

      using Base::GetCount;
      using Base::GetReserved;
      using Base::GetInfo;
      using Base::GetInfoEnd;
      using Base::IsEmpty;

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool KeyIs() const noexcept;
      NOD() constexpr bool KeyIs(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool KeyIsSimilar() const noexcept;
      NOD() constexpr bool KeyIsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool KeyIsExact() const noexcept;
      NOD() constexpr bool KeyIsExact(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool ValueIs() const noexcept;
      NOD() constexpr bool ValueIs(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool ValueIsSimilar() const noexcept;
      NOD() constexpr bool ValueIsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool ValueIsExact() const noexcept;
      NOD() constexpr bool ValueIsExact(DMeta) const noexcept;

   protected:
      template<CT::NotSemantic, CT::NotSemantic>
      void Mutate() noexcept;
      void Mutate(DMeta, DMeta);

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (CT::Map  auto const&) const requires CT::Inner::Comparable<V>;
      bool operator == (CT::Pair auto const&) const requires CT::Inner::Comparable<V>;

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() bool ContainsKey(K1 const&) const;

      template<CT::NotSemantic V1>
      requires ::std::equality_comparable_with<V, V1>
      NOD() bool ContainsValue(V1 const&) const;

      template<CT::Pair P>
      requires ::std::equality_comparable_with<TPair<K, V>, P>
      NOD() bool ContainsPair(P const&) const;

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() Index Find(K1 const&) const;

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() auto FindIt(K1 const&);

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() auto FindIt(K1 const&) const;

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() decltype(auto) At(K1 const&);

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() decltype(auto) At(K1 const&) const;

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() decltype(auto) operator[] (K1 const&);

      template<CT::NotSemantic K1>
      requires ::std::equality_comparable_with<K, K1>
      NOD() decltype(auto) operator[] (K1 const&) const;

      NOD()       K& GetKey(CT::Index auto);
      NOD() const K& GetKey(CT::Index auto) const;

      NOD()       V& GetValue(CT::Index auto);
      NOD() const V& GetValue(CT::Index auto) const;

      NOD()      PairRef GetPair(CT::Index auto);
      NOD() PairConstRef GetPair(CT::Index auto) const;

   protected:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      using Base::GetBucket;
      using Base::GetBucketUnknown;

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<CT::Map MAP>
      using TIterator = typename Base::template TIterator<MAP>;

      NOD() TIterator<TMap> begin() noexcept;
      NOD() TIterator<TMap> end() noexcept;
      NOD() TIterator<TMap> last() noexcept;
      NOD() TIterator<const TMap> begin() const noexcept;
      NOD() TIterator<const TMap> end() const noexcept;
      NOD() TIterator<const TMap> last() const noexcept;
      NOD() decltype(auto) Last() const;
      NOD() decltype(auto) Last();

      template<class F>
      Count ForEachKeyElement(F&&) const;
      template<class F>
      Count ForEachKeyElement(F&&);

      template<class F>
      Count ForEachValueElement(F&&) const;
      template<class F>
      Count ForEachValueElement(F&&);

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class K1, class V1>
      requires (CT::Inner::MakableFrom<K, K1> and CT::Inner::MakableFrom<V, V1>)
      Count Insert(K1&&, V1&&);

      template<class T1, class...TAIL>
      requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1, TAIL...>
      Count InsertPair(T1&&, TAIL&&...);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1>
      TMap& operator << (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<TPair<K, V>, T1>
      TMap& operator >> (T1&&);

      Count InsertBlock(auto&&, auto&&);
      Count InsertPairBlock(auto&&);

      TMap& operator += (const TMap&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count RemoveKey(const K&);
      Count RemoveValue(const V&);
      TIterator<TMap> RemoveIt(const TIterator<TMap>&);

      void Clear();
      void Reset();
      void Compact();

   protected:
      NOD() static Size RequestValuesSize(Count) noexcept;

      NOD()       TAny<K>& GetKeys() noexcept;
      NOD() const TAny<K>& GetKeys() const noexcept;
      NOD()       TAny<V>& GetValues() noexcept;
      NOD() const TAny<V>& GetValues() const noexcept;

   IF_LANGULUS_TESTING(public:)
      NOD() constexpr       K&  GetRawKey(Offset) noexcept;
      NOD() constexpr const K&  GetRawKey(Offset) const noexcept;
      NOD() constexpr Handle<K> GetKeyHandle(Offset) noexcept;

      NOD() constexpr       V&  GetRawValue(Offset) noexcept;
      NOD() constexpr const V&  GetRawValue(Offset) const noexcept;
      NOD() constexpr Handle<V> GetValueHandle(Offset) noexcept;
   };

} // namespace Langulus::Anyness
