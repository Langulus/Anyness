///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedMap.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   /// A highly optimized unordered hashmap implementation, using the Robin   
   /// Hood algorithm                                                         
   ///                                                                        
   template<CT::Data K, CT::Data V>
   class TUnorderedMap : public UnorderedMap {
   public:
      friend class BlockMap;
      static_assert(CT::Inner::Comparable<K>,
         "Map's key type must be equality-comparable to itself");

      using Key = K;
      using Value = V;
      using Self = TUnorderedMap<K, V>;
      using Pair = TPair<K, V>;
      using PairRef = TPair<K&, V&>;
      using PairConstRef = TPair<const K&, const V&>;

      LANGULUS(TYPED) Pair;

      static constexpr Count MinimalAllocation = 8;
      static constexpr bool Ordered = false;

      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

   public:
      constexpr TUnorderedMap();
      TUnorderedMap(const TUnorderedMap&);
      TUnorderedMap(TUnorderedMap&&) noexcept;

      TUnorderedMap(const CT::NotSemantic auto&);
      TUnorderedMap(CT::NotSemantic auto&);
      TUnorderedMap(CT::NotSemantic auto&&);
      TUnorderedMap(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      TUnorderedMap(T1&&, T2&&, TAIL&&...);

      ~TUnorderedMap();

      TUnorderedMap& operator = (const TUnorderedMap&);
      TUnorderedMap& operator = (TUnorderedMap&&) noexcept;

      TUnorderedMap& operator = (const CT::NotSemantic auto&);
      TUnorderedMap& operator = (CT::NotSemantic auto&);
      TUnorderedMap& operator = (CT::NotSemantic auto&&);
      TUnorderedMap& operator = (CT::Semantic auto&&);

   public:
      NOD() DMeta GetKeyType() const;
      NOD() DMeta GetValueType() const;

      template<class>
      NOD() constexpr bool KeyIs() const noexcept;
      template<class>
      NOD() constexpr bool ValueIs() const noexcept;

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

      NOD() constexpr Size GetBytesize() const noexcept;

      void Reserve(const Count&);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const TUnorderedMap&) const requires (CT::Inner::Comparable<V>);

      NOD() bool ContainsKey(const K&) const;
      NOD() bool ContainsValue(const V&) const requires (CT::Inner::Comparable<V>);
      NOD() bool ContainsPair(const Pair&) const requires (CT::Inner::Comparable<V>);

      NOD() Index Find(const K&) const;
      NOD() Iterator FindIt(const K&);
      NOD() ConstIterator FindIt(const K&) const;

      NOD() decltype(auto) At(const K&);
      NOD() decltype(auto) At(const K&) const;

      NOD() decltype(auto) operator[] (const K&);
      NOD() decltype(auto) operator[] (const K&) const;

      NOD()       K& GetKey(const CT::Index auto&);
      NOD() const K& GetKey(const CT::Index auto&) const;

      NOD()       V& GetValue(const CT::Index auto&);
      NOD() const V& GetValue(const CT::Index auto&) const;

      NOD()      PairRef GetPair(const CT::Index auto&);
      NOD() PairConstRef GetPair(const CT::Index auto&) const;

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
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const K&, const V&);
      Count Insert(const K&,       V&);
      Count Insert(const K&,       V&&);
      Count Insert(const K&, CT::Semantic auto&&);

      Count Insert(K&, const V&);
      Count Insert(K&,       V&);
      Count Insert(K&,       V&&);
      Count Insert(K&, CT::Semantic auto&&);
                   
      Count Insert(K&&, const V&);
      Count Insert(K&&,       V&);
      Count Insert(K&&,       V&&);
      Count Insert(K&&, CT::Semantic auto&&);

      Count Insert(CT::Semantic auto&&, const V&);
      Count Insert(CT::Semantic auto&&,       V&);
      Count Insert(CT::Semantic auto&&,       V&&);
      Count Insert(CT::Semantic auto&&, CT::Semantic auto&&);

      Count InsertBlock(CT::Semantic auto&&, CT::Semantic auto&&);

      Count InsertPair(const CT::Pair auto&);
      Count InsertPair(      CT::Pair auto&);
      Count InsertPair(      CT::Pair auto&&);
      Count InsertPair(CT::Semantic   auto&&);

      Count InsertPairBlock(CT::Semantic auto&&);

      TUnorderedMap& operator << (const CT::Pair auto&);
      TUnorderedMap& operator << (      CT::Pair auto&);
      TUnorderedMap& operator << (      CT::Pair auto&&);
      TUnorderedMap& operator << (CT::Semantic   auto&&);

      TUnorderedMap& operator += (const TUnorderedMap&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count RemoveKey(const K&);
      Count RemoveValue(const V&);
      Count RemovePair(const Pair&);
      Iterator RemoveIt(const Iterator&);

      void Clear();
      void Reset();
      void Compact();

   protected:
      void AllocateFresh(const Count&);
      template<bool REUSE>
      void AllocateData(const Count&);
      void AllocateInner(const Count&);

      void Rehash(const Count&);
      void RehashKeys(const Count&, Block&);
      void RehashValues(const Count&, Block&);

      void ClearInner();

      NOD() static Size RequestKeyAndInfoSize(Count, Offset&) noexcept;
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
   template<CT::Data K, CT::Data V>
   template<bool MUTABLE>
   struct TUnorderedMap<K, V>::TIterator {
   protected:
      friend class TUnorderedMap<K, V>;

      const InfoType* mInfo {};
      const InfoType* mSentinel {};
      const K* mKey {};
      const V* mValue {};

      TIterator(const InfoType*, const InfoType*, const K*, const V*) noexcept;

   public:
      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() PairRef operator * () const noexcept requires (MUTABLE);
      NOD() PairConstRef operator * () const noexcept requires (not MUTABLE);

      NOD() PairRef operator -> () const noexcept requires (MUTABLE);
      NOD() PairConstRef operator -> () const noexcept requires (not MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
   };

} // namespace Langulus::Anyness
