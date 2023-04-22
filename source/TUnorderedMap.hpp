///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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
         "Can't compare keys for map");

      using Key = K;
      using Value = V;
      using Self = TUnorderedMap<K, V>;
      using Pair = TPair<K, V>;
      using PairRef = TPair<K&, V&>;
      using PairConstRef = TPair<const K&, const V&>;
      using Allocator = Inner::Allocator;

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

      template<CT::Data HEAD, CT::Data... TAIL>
      TUnorderedMap(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

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

      NOD() constexpr Size GetByteSize() const noexcept;

      void Allocate(const Count&);

      bool operator == (const TUnorderedMap&) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const K&, const V&);
      Count Insert(K&&, const V&);
      Count Insert(const K&, V&&);
      Count Insert(K&&, V&&);
      template<CT::Semantic SK, CT::Semantic SV>
      Count Insert(SK&&, SV&&) noexcept requires (CT::Exact<TypeOf<SK>, K> && CT::Exact<TypeOf<SV>, V>);

      TUnorderedMap& operator << (const TPair<K, V>&);
      TUnorderedMap& operator << (TPair<K, V>&&);
      template<CT::Semantic S>
      TUnorderedMap& operator << (S&&) noexcept requires (CT::Pair<TypeOf<S>>);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count RemoveKey(const K&);
      Count RemoveValue(const V&);
      Count RemovePair(const Pair&);
      Count RemoveIndex(const Index&);
      Iterator RemoveIndex(const Iterator&);

      void Clear();
      void Reset();
      void Compact();

      ///                                                                     
      ///   Search                                                            
      ///                                                                     
      NOD() bool ContainsKey(const K&) const;
      NOD() bool ContainsValue(const V&) const;
      NOD() bool ContainsPair(const Pair&) const;
      NOD() Index FindKeyIndex(const K&) const;

      NOD() decltype(auto) At(const K&);
      NOD() decltype(auto) At(const K&) const;

      NOD() decltype(auto) operator[] (const K&) const;
      NOD() decltype(auto) operator[] (const K&);

      NOD() const K& GetKey(const CT::Index auto&) const;
      NOD() K& GetKey(const CT::Index auto&);
      NOD() const V& GetValue(const CT::Index auto&) const;
      NOD() V& GetValue(const CT::Index auto&);
      NOD() PairConstRef GetPair(const CT::Index auto&) const;
      NOD() PairRef GetPair(const CT::Index auto&);

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

      Count ForEachKeyElement(TFunctor<bool(const Block&)>&&) const;
      Count ForEachKeyElement(TFunctor<bool(Block&)>&&);
      Count ForEachKeyElement(TFunctor<void(const Block&)>&&) const;
      Count ForEachKeyElement(TFunctor<void(Block&)>&&);

      Count ForEachValueElement(TFunctor<bool(const Block&)>&&) const;
      Count ForEachValueElement(TFunctor<bool(Block&)>&&);
      Count ForEachValueElement(TFunctor<void(const Block&)>&&) const;
      Count ForEachValueElement(TFunctor<void(Block&)>&&);

   protected:
      void AllocateFresh(const Count&);
      template<bool REUSE>
      void AllocateData(const Count&);
      void AllocateInner(const Count&);
      void Rehash(const Count&, const Count&);

      template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
      Offset InsertInner(const Offset&, SK&&, SV&&);

      void ClearInner();

      template<class T>
      static void Overwrite(T&&, T&) noexcept;

      NOD() static Size RequestKeyAndInfoSize(Count, Offset&) noexcept;
      NOD() static Size RequestValuesSize(Count) noexcept;

      void RemoveIndex(const Offset&) SAFETY_NOEXCEPT();

      NOD() const TAny<K>& GetKeys() const noexcept;
      NOD() TAny<K>& GetKeys() noexcept;
      NOD() const TAny<V>& GetValues() const noexcept;
      NOD() TAny<V>& GetValues() noexcept;

      NOD() Offset FindIndex(const K&) const;

   TESTING(public:)
      NOD() constexpr const K& GetRawKey(Offset) const noexcept;
      NOD() constexpr K& GetRawKey(Offset) noexcept;
      NOD() constexpr Handle<K> GetKeyHandle(Offset) noexcept;

      NOD() constexpr const V& GetRawValue(Offset) const noexcept;
      NOD() constexpr V& GetRawValue(Offset) noexcept;
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
      NOD() PairConstRef operator * () const noexcept requires (!MUTABLE);

      NOD() PairRef operator -> () const noexcept requires (MUTABLE);
      NOD() PairConstRef operator -> () const noexcept requires (!MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Anyness

#include "TUnorderedMap.inl"
