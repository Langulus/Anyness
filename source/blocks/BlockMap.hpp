///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../TAny.hpp"
#include "../TPair.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased map block, base for all map types                        
   ///                                                                        
   ///   This is an inner structure, that doesn't reference any memory,       
   /// only provides the functionality to do so. You can use BlockMap as a    
   /// lightweight intermediate structure for iteration, etc.                 
   ///                                                                        
   class BlockMap {
   protected:
      static constexpr Count MinimalAllocation = 8;
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
      using Pair = Anyness::Pair;

      static constexpr bool Ownership = false;
      static constexpr bool Sequential = false;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr BlockMap() noexcept = default;
      constexpr BlockMap(const BlockMap&) noexcept = default;
      constexpr BlockMap(BlockMap&&) noexcept = default;
      constexpr BlockMap(CT::Semantic auto&&) noexcept;

      constexpr BlockMap& operator = (const BlockMap&) noexcept = default;
      constexpr BlockMap& operator = (BlockMap&&) noexcept = default;
      constexpr BlockMap& operator = (CT::Semantic auto&&) noexcept;

   protected:
      template<class T>
      void BlockTransfer(CT::Semantic auto&&);
      template<class T>
      void BlockClone(const BlockMap&);

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetKeyType() const noexcept;
      NOD() DMeta GetValueType() const noexcept;

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
      NOD() constexpr Count GetCount() const noexcept;
      NOD() Count GetKeyCountDeep() const noexcept;
      NOD() Count GetKeyCountElementsDeep() const noexcept;
      NOD() Count GetValueCountDeep() const noexcept;
      NOD() Count GetValueCountElementsDeep() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;
      NOD() bool IsMissing() const noexcept;
      NOD() bool IsMissingDeep() const;

      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      DEBUGGERY(void Dump() const);

   protected:
      template<CT::Data K>
      NOD() const TAny<K>& GetKeys() const noexcept;
      template<CT::Data K>
      NOD() TAny<K>& GetKeys() noexcept;
      template<CT::Data V>
      NOD() const TAny<V>& GetValues() const noexcept;
      template<CT::Data V>
      NOD() TAny<V>& GetValues() noexcept;

      NOD() const InfoType* GetInfo() const noexcept;
      NOD() InfoType* GetInfo() noexcept;
      NOD() const InfoType* GetInfoEnd() const noexcept;

      NOD() Count GetCountDeep(const Block&) const noexcept;
      NOD() Count GetCountElementsDeep(const Block&) const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Block GetKey(const CT::Index auto&);
      NOD() Block GetKey(const CT::Index auto&) const;
      NOD() Block GetValue(const CT::Index auto&);
      NOD() Block GetValue(const CT::Index auto&) const;
      NOD() Pair GetPair(const CT::Index auto&);
      NOD() Pair GetPair(const CT::Index auto&) const;

   protected:
      NOD() Block GetKeyInner(const Offset&) SAFETY_NOEXCEPT();
      NOD() Block GetKeyInner(const Offset&) const SAFETY_NOEXCEPT();
      NOD() Block GetValueInner(const Offset&) SAFETY_NOEXCEPT();
      NOD() Block GetValueInner(const Offset&) const SAFETY_NOEXCEPT();
      NOD() Pair GetPairInner(const Offset&) SAFETY_NOEXCEPT();
      NOD() Pair GetPairInner(const Offset&) const SAFETY_NOEXCEPT();

      NOD() static Offset GetBucket(Offset, const CT::NotSemantic auto&) noexcept;
      NOD() static Offset GetBucketUnknown(Offset, const Block&) noexcept;

      template<CT::Data K>
      NOD() constexpr const K& GetRawKey(Offset) const noexcept;
      template<CT::Data K>
      NOD() constexpr K& GetRawKey(Offset) noexcept;
      template<CT::Data K>
      NOD() constexpr Handle<K> GetKeyHandle(Offset) const noexcept;

      template<CT::Data V>
      NOD() constexpr const V& GetRawValue(Offset) const noexcept;
      template<CT::Data V>
      NOD() constexpr V& GetRawValue(Offset) noexcept;
      template<CT::Data V>
      NOD() constexpr Handle<V> GetValueHandle(Offset) const noexcept;

   public:
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

      template<bool REVERSE = false, class F>
      Count ForEach(F&&) const;

      template<bool REVERSE = false, bool MUTABLE = true, class F>
      Count ForEachKeyElement(F&&);
      template<bool REVERSE = false, class F>
      Count ForEachKeyElement(F&&) const;

      template<bool REVERSE = false, bool MUTABLE = true, class F>
      Count ForEachValueElement(F&&);
      template<bool REVERSE = false, class F> 
      Count ForEachValueElement(F&&) const;

      template<bool REVERSE = false, bool MUTABLE = true, class... F>
      Count ForEachKey(F&&...);
      template<bool REVERSE = false, class... F>
      Count ForEachKey(F&&...) const;
      template<bool REVERSE = false, bool MUTABLE = true, class... F>
      Count ForEachValue(F&&...);
      template<bool REVERSE = false, class... F>
      Count ForEachValue(F&&...) const;
   
      template<bool REVERSE = false, bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachKeyDeep(F&&...);
      template<bool REVERSE = false, bool SKIP = true, class... F>
      Count ForEachKeyDeep(F&&...) const;
      template<bool REVERSE = false, bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachValueDeep(F&&...);
      template<bool REVERSE = false, bool SKIP = true, class... F>
      Count ForEachValueDeep(F&&...) const;

   protected:
      template<bool REVERSE, bool MUTABLE, class F>
      Count ForEachElement(Block&, F&&);
      template<class R, CT::Data A, bool REVERSE, bool MUTABLE, class F>
      Count ForEachInner(Block&, F&&);
      template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE, class F>
      Count ForEachDeepInner(Block&, F&&);

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::NotSemantic K, CT::NotSemantic V>
      void Mutate();
      void Mutate(DMeta, DMeta);
      
      template<class ALT_K>
      NOD() constexpr bool KeyIs() const noexcept;
      template<class ALT_V>
      NOD() constexpr bool ValueIs() const noexcept;

      NOD() bool IsTypeCompatibleWith(const BlockMap&) const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const BlockMap&) const;

      NOD() Hash GetHash() const;

      template<CT::NotSemantic K>
      NOD() bool ContainsKey(const K&) const;
      template<CT::NotSemantic V>
      NOD() bool ContainsValue(const V&) const;
      template<CT::NotSemantic K, CT::NotSemantic V>
      NOD() bool ContainsPair(const TPair<K, V>&) const;
      template<CT::NotSemantic K>
      NOD() Index Find(const K&) const;

   protected:
      template<class THIS = BlockMap, CT::NotSemantic K>
      NOD() Offset FindIndex(const K&) const;
      template<class THIS = BlockMap>
      NOD() Offset FindIndexUnknown(const Block&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(const Count&);

   protected:
      /// @cond show_protected                                                
      void AllocateFresh(const Count&);
      template<bool REUSE>
      void AllocateData(const Count&);
      void AllocateInner(const Count&);

      void Reference(const Count&) const noexcept;
      void Keep() const noexcept;
      template<bool DESTROY>
      void Dereference(const Count&);
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     

   protected:
      NOD() Size RequestKeyAndInfoSize(Count, Offset&) const SAFETY_NOEXCEPT();
      NOD() Size RequestValuesSize(Count) const SAFETY_NOEXCEPT();
      
      void Rehash(const Count&);
      void RehashKeys(const Count&, Block&);
      void RehashValues(const Count&, Block&);
      template<class K, class V>
      void ShiftPairs();

      template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
      Offset InsertInner(const Offset&, SK&&, SV&&);
      template<bool CHECK_FOR_MATCH, CT::Semantic SK, CT::Semantic SV>
      Offset InsertInnerUnknown(const Offset&, SK&&, SV&&);

      template<class, CT::Semantic S>
      void InsertPairInner(const Count&, S&&);

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<class THIS = BlockMap, CT::NotSemantic K>
      Count RemoveKey(const K&);
      template<class THIS = BlockMap, CT::NotSemantic V>
      Count RemoveValue(const V&);
      template<CT::NotSemantic K, CT::NotSemantic V>
      Count RemovePair(const TPair<K, V>&);
      Count RemoveIndex(const Index&);

      void Clear();
      void Reset();
      void Compact();

   protected:
      void ClearInner();
      void RemoveIndex(const Offset&) SAFETY_NOEXCEPT();

   #if LANGULUS(TESTING)
      public: NOD() constexpr const void* GetRawKeysMemory() const noexcept;
      public: NOD() constexpr const void* GetRawValuesMemory() const noexcept;
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


namespace Langulus::CT
{

   /// A reflected map type is any type that inherits BlockMap, and is        
   /// binary compatible to a BlockMap                                        
   /// Keep in mind, that sparse types are never considered CT::Map!          
   template<class... T>
   concept Map = ((DerivedFrom<T, Anyness::BlockMap>
       and sizeof(T) == sizeof(Anyness::BlockMap)) and ...);
   
   /// Check if a type is a statically typed map                              
   template<class... T>
   concept TypedMap = Map<T...> and Typed<T...>;

} // namespace Langulus::CT
