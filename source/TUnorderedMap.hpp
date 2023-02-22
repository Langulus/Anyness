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
      static_assert(CT::Comparable<K>, "Can't compare keys for map");

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
      TUnorderedMap(::std::initializer_list<Pair>);
      TUnorderedMap(const TUnorderedMap&);
      TUnorderedMap(TUnorderedMap&&) noexcept;

      //TODO defined in header due to MSVC compiler bug (02/2023)       
      // Might be fixed in the future                                   
      template<CT::Semantic S>
      constexpr TUnorderedMap(S&& other) noexcept requires (CT::Exact<TypeOf<S>, Self>)
         : UnorderedMap {other.template Forward<BlockMap>()} {}

      ~TUnorderedMap();

      TUnorderedMap& operator = (const TUnorderedMap&);
      TUnorderedMap& operator = (TUnorderedMap&&) noexcept;
      template<CT::Semantic S>
      TUnorderedMap& operator = (S&&) noexcept requires (CT::Exact<TypeOf<S>, Self>);

      TUnorderedMap& operator = (const Pair&);
      TUnorderedMap& operator = (Pair&&) noexcept;
      template<CT::Semantic S>
      TUnorderedMap& operator = (S&&) noexcept requires (CT::Pair<TypeOf<S>>);

   public:
      NOD() DMeta GetKeyType() const;
      NOD() DMeta GetValueType() const;

      template<class ALT_K>
      NOD() constexpr bool KeyIs() const noexcept;
      template<class ALT_V>
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

      NOD() TUnorderedMap Clone() const;

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

      /*NOD() decltype(auto) GetKey(const Index&) const;
      NOD() decltype(auto) GetKey(const Index&);
      NOD() decltype(auto) GetValue(const Index&) const;
      NOD() decltype(auto) GetValue(const Index&);
      NOD() decltype(auto) GetPair(const Index&) const;
      NOD() decltype(auto) GetPair(const Index&);*/

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
      template<bool REUSE>
      void AllocateData(const Count&);
      void AllocateInner(const Count&);
      void Rehash(const Count&, const Count&);

      template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
      Offset InsertInner(const Offset&, SK&&, SV&&);

      void ClearInner();

      template<class T>
      static void DestroyElement(T) noexcept;

      template<class T>
      static void Overwrite(T&&, T&) noexcept;

      NOD() static Size RequestKeyAndInfoSize(Count, Offset&) noexcept;

      void RemoveIndex(const Offset&) noexcept;

      NOD() const TAny<K>& GetKeys() const noexcept;
      NOD() TAny<K>& GetKeys() noexcept;
      NOD() const TAny<V>& GetValues() const noexcept;
      NOD() TAny<V>& GetValues() noexcept;

      NOD() Offset GetBucket(const K&) const noexcept;
      NOD() Offset FindIndex(const K&) const;

   TESTING(public:)
      NOD() constexpr const K& GetRawKey(Offset) const noexcept;
      NOD() constexpr K& GetRawKey(Offset) noexcept;
      NOD() constexpr decltype(auto) GetKeyHandle(Offset) noexcept;

      NOD() constexpr const V& GetRawValue(Offset) const noexcept;
      NOD() constexpr V& GetRawValue(Offset) noexcept;
      NOD() constexpr decltype(auto) GetValueHandle(Offset) noexcept;

      NOD() decltype(auto) GetPair(const Offset&) const noexcept;
      NOD() decltype(auto) GetPair(const Offset&) noexcept;
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
