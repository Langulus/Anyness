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

      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

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
      constexpr void Mutate() noexcept;
      void Mutate(DMeta, DMeta);

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const TMap&) const
      requires CT::Inner::Comparable<V>;

      NOD() bool ContainsKey(const K&) const;

      NOD() bool ContainsValue(const V&) const
      requires CT::Inner::Comparable<V>;

      NOD() bool ContainsPair(const Pair&) const
      requires CT::Inner::Comparable<V>;

      NOD() Index Find(const K&) const;
      NOD() Iterator FindIt(const K&);
      NOD() ConstIterator FindIt(const K&) const;

      NOD() decltype(auto) At(const K&);
      NOD() decltype(auto) At(const K&) const;

      NOD() decltype(auto) operator[] (const K&);
      NOD() decltype(auto) operator[] (const K&) const;

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
      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;
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
      Iterator RemoveIt(const Iterator&);

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


   ///                                                                        
   ///   Unordered map iterator                                               
   ///                                                                        
   template<CT::Data K, CT::Data V, bool ORDERED> template<bool MUTABLE>
   struct TMap<K, V, ORDERED>::TIterator {
   protected:
      template<CT::Data, CT::Data, bool>
      friend struct TMap;

      using InfoType = TMap<K, V, ORDERED>::InfoType;
      const InfoType* mInfo {};
      const InfoType* mSentinel {};
      const K* mKey {};
      const V* mValue {};

      TIterator(const InfoType*, const InfoType*, const K*, const V*) noexcept;

   public:
      TIterator(const TIterator<true>&) noexcept;
      TIterator& operator = (const TIterator&) noexcept;

      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() PairRef      operator *  () const noexcept requires (MUTABLE);
      NOD() PairConstRef operator *  () const noexcept requires (not MUTABLE);

      NOD() PairRef      operator -> () const noexcept requires (MUTABLE);
      NOD() PairConstRef operator -> () const noexcept requires (not MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
   };

} // namespace Langulus::Anyness
