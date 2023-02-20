///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../TAny.hpp"
#include "../TPair.hpp"

namespace Langulus::Anyness
{
   class BlockMap;
}

namespace Langulus::CT
{

   /// A reflected map type is any type that inherits BlockMap, and is        
   /// binary compatible to a BlockMap                                        
   /// Keep in mind, that sparse types are never considered CT::Map!          
   template<class... T>
   concept Map = ((DerivedFrom<T, Anyness::BlockMap>
      && sizeof(T) == sizeof(Anyness::BlockMap)) && ...);

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased map block, base for all map types                        
   ///                                                                        
   class BlockMap {
      using Allocator = Inner::Allocator;
      static constexpr Count MinimalAllocation = 8;
   protected:
      using InfoType = ::std::uint8_t;

      // A precomputed pointer for the info bytes                       
      // Points to an offset inside mKeys allocation                    
      // Each byte represents a pair, and can be three things:          
      //    0 - the index is not used, data is not initialized          
      //    1 - the index is used, and key is exactly where it should be
      //   2+ - the index is used, but bucket is info-1 buckets to      
      //         the right of this index                                
      InfoType* mInfo {};

      // The block that contains the keys and info bytes                
      Block mKeys;

      // The block that contains the values                             
      // It's size and reserve also used for the keys and tombstones    
      // The redundant data inside mKeys is required for binary         
      // compatibility with the type-erased equivalents                 
      Block mValues;

   public:
      static constexpr bool Ownership = true;
      static constexpr bool Sequential = false;

      constexpr BlockMap() = default;

      BlockMap(const BlockMap&);
      BlockMap(BlockMap&&) noexcept;
      template<CT::Semantic S>
      constexpr BlockMap(S&&) noexcept requires (CT::Map<TypeOf<S>>);

      template<CT::Data K, CT::Data V>
      BlockMap(::std::initializer_list<TPair<K, V>>);

      ~BlockMap();

      BlockMap& operator = (const BlockMap&);
      BlockMap& operator = (BlockMap&&) noexcept;

      BlockMap& operator = (const Pair&);
      BlockMap& operator = (Pair&&) noexcept;

      template<CT::Data K, CT::Data V>
      BlockMap& operator = (const TPair<K, V>&);
      template<CT::Data K, CT::Data V>
      BlockMap& operator = (TPair<K, V>&&) noexcept;

   public:
      NOD() DMeta GetKeyType() const noexcept;
      NOD() DMeta GetValueType() const noexcept;

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
      NOD() constexpr Count GetCount() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;

      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

      NOD() Hash GetHash() const;

      template<CT::NotSemantic K, CT::NotSemantic V>
      void Mutate();
      void Mutate(DMeta, DMeta);
      void Allocate(const Count&);

      NOD() BlockMap Clone() const;

      bool operator == (const BlockMap&) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::NotSemantic K, CT::NotSemantic V>
      Count Insert(const K&, const V&);
      template<CT::NotSemantic K, CT::NotSemantic V>
      Count Insert(K&&, const V&);
      template<CT::NotSemantic K, CT::NotSemantic V>
      Count Insert(const K&, V&&);
      template<CT::NotSemantic K, CT::NotSemantic V>
      Count Insert(K&&, V&&);

      template<CT::Semantic SK, CT::Semantic SV>
      Count Insert(SK&&, SV&&);

      template<CT::Data K, CT::Data V>
      BlockMap& operator << (const TPair<K, V>&);
      template<CT::Data K, CT::Data V>
      BlockMap& operator << (TPair<K, V>&&);
      BlockMap& operator << (const Pair&);
      BlockMap& operator << (Pair&&);

      template<CT::Semantic S>
      BlockMap& operator << (S&&) requires (CT::Pair<TypeOf<S>>);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::NotSemantic K>
      Count RemoveKey(const K&);
      template<CT::NotSemantic V>
      Count RemoveValue(const V&);
      template<CT::NotSemantic K, CT::NotSemantic V>
      Count RemovePair(const TPair<K, V>&);
      Count RemoveIndex(const Index&);

      void Clear();
      void Reset();
      void Compact();

      ///                                                                     
      ///   Search                                                            
      ///                                                                     
      template<CT::NotSemantic K>
      NOD() bool ContainsKey(const K&) const;
      template<CT::NotSemantic V>
      NOD() bool ContainsValue(const V&) const;
      template<CT::NotSemantic K, CT::NotSemantic V>
      NOD() bool ContainsPair(const TPair<K, V>&) const;
      template<CT::NotSemantic K>
      NOD() Index FindKeyIndex(const K&) const;

      template<CT::NotSemantic K>
      NOD() Block At(const K&);
      template<CT::NotSemantic K>
      NOD() Block At(const K&) const;

      template<CT::NotSemantic K>
      NOD() Block operator[] (const K&) const;
      template<CT::NotSemantic K>
      NOD() Block operator[] (const K&);

      NOD() Block GetKey(const Index&) const;
      NOD() Block GetKey(const Index&);
      NOD() Block GetValue(const Index&) const;
      NOD() Block GetValue(const Index&);
      NOD() Pair GetPair(const Index&) const;
      NOD() Pair GetPair(const Index&);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;

      template<bool MUTABLE = true, class F>
      Count ForEachKeyElement(F&&);
      template<class F>
      Count ForEachKeyElement(F&&) const;

      template<bool MUTABLE = true, class F>
      Count ForEachValueElement(F&&);
      template<class F>
      Count ForEachValueElement(F&&) const;

      template<bool MUTABLE = true, class... F>
      Count ForEachKey(F&&...);
      template<class... F>
      Count ForEachKey(F&&...) const;
      template<bool MUTABLE = true, class... F>
      Count ForEachValue(F&&...);
      template<class... F>
      Count ForEachValue(F&&...) const;
   
      template<bool MUTABLE = true, class... F>
      Count ForEachKeyRev(F&&...);
      template<class... F>
      Count ForEachKeyRev(F&&...) const;
      template<bool MUTABLE = true, class... F>
      Count ForEachValueRev(F&&...);
      template<class... F>
      Count ForEachValueRev(F&&...) const;
   
      template<bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachKeyDeep(F&&...);
      template<bool SKIP = true, class... F>
      Count ForEachKeyDeep(F&&...) const;
      template<bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachValueDeep(F&&...);
      template<bool SKIP = true, class... F>
      Count ForEachValueDeep(F&&...) const;
   
      template<bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachKeyDeepRev(F&&...);
      template<bool SKIP = true, class... F>
      Count ForEachKeyDeepRev(F&&...) const;
      template<bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachValueDeepRev(F&&...);
      template<bool SKIP = true, class... F>
      Count ForEachValueDeepRev(F&&...) const;

   protected:
      template<bool MUTABLE, bool REVERSE, class F>
      Count ForEachSplitter(Block&, F&&);
      template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
      Count ForEachDeepSplitter(Block&, F&&);
      template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
      Count ForEachInner(Block&, TFunctor<R(A)>&&);
      template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
      Count ForEachDeepInner(Block&, TFunctor<R(A)>&&);
      template<bool MUTABLE, class F>
      Count ForEachElement(Block&, F&&);

      template<bool REUSE>
      void AllocateKeys(const Count&);
      void AllocateInner(const Count&);

      void Rehash(const Count&, const Count&);

      template<CT::Semantic SK, CT::Semantic SV>
      Count InsertUnknown(SK&&, SV&&);

      template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
      Offset InsertInnerUnknown(const Offset&, SK&&, SV&&);
      template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
      Offset InsertInner(const Offset&, SK&&, SV&&);

      void ClearInner();

      NOD() Size RequestKeyAndInfoSize(Count, Offset&) const SAFETY_NOEXCEPT();

      void RemoveIndex(const Offset&) noexcept;

      template<CT::Data K>
      NOD() const TAny<K>& GetKeys() const noexcept;
      template<CT::Data K>
      NOD() TAny<K>& GetKeys() noexcept;
      template<CT::Data V>
      NOD() const TAny<V>& GetValues() const noexcept;
      template<CT::Data V>
      NOD() TAny<V>& GetValues() noexcept;

      NOD() Block GetKey(const Offset&) const noexcept;
      NOD() Block GetKey(const Offset&) noexcept;
      NOD() Block GetValue(const Offset&) const noexcept;
      NOD() Block GetValue(const Offset&) noexcept;
      NOD() Pair GetPair(const Offset&) const noexcept;
      NOD() Pair GetPair(const Offset&) noexcept;

      template<CT::NotSemantic K>
      NOD() Offset GetBucket(const K&) const noexcept;
      template<CT::NotSemantic K>
      NOD() Offset FindIndex(const K&) const;
      NOD() Offset FindIndexUnknown(const Block&) const;

   TESTING(public:)
      NOD() const InfoType* GetInfo() const noexcept;
      NOD() InfoType* GetInfo() noexcept;
      NOD() const InfoType* GetInfoEnd() const noexcept;

      template<CT::Data K>
      NOD() constexpr const K* GetRawKeys() const noexcept;
      template<CT::Data K>
      NOD() constexpr K* GetRawKeys() noexcept;
      template<CT::Data K>
      NOD() constexpr const K* GetRawKeysEnd() const noexcept;

      template<CT::Data V>
      NOD() constexpr const V* GetRawValues() const noexcept;
      template<CT::Data V>
      NOD() constexpr V* GetRawValues() noexcept;
      template<CT::Data V>
      NOD() constexpr const V* GetRawValuesEnd() const noexcept;

   #ifdef LANGULUS_ENABLE_TESTING
      NOD() constexpr const void* GetRawKeysMemory() const noexcept;
      NOD() constexpr const void* GetRawValuesMemory() const noexcept;
   #endif
   };


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        
   template<bool MUTABLE>
   struct BlockMap::TIterator {
   protected:
      friend class BlockMap;

      const InfoType* mInfo {};
      const InfoType* mSentinel {};
      Block mKey;
      Block mValue;

      TIterator(const InfoType*, const InfoType*, const Block&, const Block&) noexcept;

   public:
      TIterator() noexcept = default;
      TIterator(const TIterator&) noexcept = default;
      TIterator(TIterator&&) noexcept = default;

      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() Pair operator * () const noexcept;

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Anyness

#include "BlockMap.inl"
