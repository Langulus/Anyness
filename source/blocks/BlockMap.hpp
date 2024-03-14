///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../many/TAny.hpp"
#include "../pairs/TPair.hpp"


namespace Langulus
{
   namespace A
   {

      ///                                                                     
      /// An abstract Map structure                                           
      /// It defines the size for CT::Map concept                             
      ///                                                                     
      struct BlockMap {
         LANGULUS(ABSTRACT) true;
         LANGULUS(POD) true;

         using InfoType = ::std::uint8_t;
         using OrderType = Offset;

         static constexpr bool Sequential = false;
         static constexpr Offset InvalidOffset = -1;
         static constexpr Count MinimalAllocation = 8;

      protected:
         // A precomputed pointer for the info/ordering bytes           
         // Points to an offset inside mKeys allocation                 
         // Each byte represents a pair, and can be three things:       
         //    0 - the index is not used, data is not initialized       
         //    1 - the index is used, and key is where it should be     
         //   2+ - the index is used, but bucket is info-1 buckets to   
         //         the right of this index                             
         InfoType* mInfo {};

         // The block that contains the keys and info bytes             
         // Also keeps track of count and reserve                       
         Anyness::Block mKeys;

         // The block that contains the values                          
         // Count and reserve in this block are redundant and shouldn't 
         // be used for any purpose. The benefit is, that we can access 
         // the values block without any cost via pointer arithmetic,   
         // instead of generating Block instances at runtime            
         // This incurs 8 bytes or 16 bytes of memory overhead per map, 
         // depending on architecture. Optimizing this in the future    
         // will definitely break binary compatibility, and would       
         // involve a lot of boilerplate code that duplicates Block     
         // functionality. I've decided to make the sacrifice...        
         Anyness::Block mValues;

      public:
         constexpr BlockMap() noexcept = default;
         constexpr BlockMap(const BlockMap&) noexcept = default;
         constexpr BlockMap(BlockMap&&) noexcept = default;

         constexpr BlockMap& operator = (const BlockMap&) noexcept = default;
         constexpr BlockMap& operator = (BlockMap&&) noexcept = default;
      };

   } // namespace Langulus::A

   namespace CT
   {

      /// A reflected map type is any type that inherits BlockMap, and is     
      /// binary compatible to a BlockMap                                     
      /// Keep in mind, that sparse types are never considered CT::Map!       
      template<class... T>
      concept Map = ((DerivedFrom<T, A::BlockMap>
          and sizeof(T) == sizeof(A::BlockMap)) and ...);

      /// Check if a type is a statically typed map                           
      template<class... T>
      concept TypedMap = Map<T...> and Typed<T...>;

      /// Check if a type is a type-erased map                                
      template<class... T>
      concept TypeErasedMap = Map<T...> and ((not Typed<T>) and ...);

   } // namespace Langulus::CT

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased map block, base for all map containers                   
   ///                                                                        
   ///   This is an inner structure, that doesn't reference any memory,       
   /// only provides the functionality to do so. You can use BlockMap as a    
   /// lightweight intermediate structure for iteration of maps - it is       
   /// binary compatible with any other map, be it type-erased or not.        
   ///   Unlike std::map, accessing elements via the subscript operator []    
   /// doesn't implicitly add element, if map is mutable. This has always     
   /// been a source of many subtle bugs, and generally the idea of           
   /// completely changing the behavior of a program, by simply removing a    
   /// 'const' qualifier doesn't seem like a sound design decision in my book 
   ///                                                                        
   struct BlockMap : A::BlockMap {
      LANGULUS(ABSTRACT) false;

      using Pair = Anyness::Pair;

      static constexpr bool Ownership = false;
      static constexpr bool Ordered = false;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      using A::BlockMap::BlockMap;
      using A::BlockMap::operator =;

   protected:
      template<CT::Map TO, template<class> class S, CT::Map FROM>
      requires CT::Semantic<S<FROM>>
      void BlockTransfer(S<FROM>&&);

      template<template<class> class S, CT::Map FROM>
      requires CT::Semantic<S<FROM>>
      void CloneValuesInner(S<FROM>&&);

      template<template<class> class S, CT::Map FROM>
      requires CT::Semantic<S<FROM>>
      void CloneValuesReinsertInner(CT::Block auto&, S<FROM>&&);

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      template<CT::Map = UnorderedMap>
      NOD() DMeta GetKeyType() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() DMeta GetValueType() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsKeyTyped() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsValueTyped() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsKeyUntyped() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsValueUntyped() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsKeyTypeConstrained() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsValueTypeConstrained() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsKeyDeep() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsValueDeep() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsKeySparse() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsValueSparse() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsKeyDense() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsValueDense() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() constexpr Size GetKeyStride() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr Size GetValueStride() const noexcept;

      template<CT::Map = UnorderedMap>
      NOD() Count GetKeyCountDeep() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() Count GetKeyCountElementsDeep() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() Count GetValueCountDeep() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() Count GetValueCountElementsDeep() const noexcept;

      NOD() constexpr DataState GetKeyState() const noexcept;
      NOD() constexpr DataState GetValueState() const noexcept;
      NOD() constexpr bool IsKeyCompressed() const noexcept;
      NOD() constexpr bool IsValueCompressed() const noexcept;
      NOD() constexpr bool IsKeyEncrypted() const noexcept;
      NOD() constexpr bool IsValueEncrypted() const noexcept;
      NOD() constexpr bool IsKeyConstant() const noexcept;
      NOD() constexpr bool IsValueConstant() const noexcept;
      NOD() constexpr Count GetCount() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsValid() const noexcept;
      NOD() constexpr bool IsInvalid() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;

      NOD() bool IsKeyMissing() const noexcept;
      NOD() bool IsValueMissing() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() bool IsKeyMissingDeep() const;
      template<CT::Map = UnorderedMap>
      NOD() bool IsValueMissingDeep() const;

      template<CT::Map = UnorderedMap>
      NOD() bool IsKeyExecutable() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() bool IsValueExecutable() const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() bool IsKeyExecutableDeep() const;
      template<CT::Map = UnorderedMap>
      NOD() bool IsValueExecutableDeep() const;

      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      DEBUGGERY(template<CT::Map> void Dump() const);

   protected:
      template<CT::Map>
      NOD() auto& GetKeys() const noexcept;
      template<CT::Map>
      NOD() auto& GetKeys() noexcept;
      template<CT::Map>
      NOD() auto& GetVals() const noexcept;
      template<CT::Map>
      NOD() auto& GetVals() noexcept;

      NOD() const InfoType* GetInfo() const noexcept;
      NOD()       InfoType* GetInfo() noexcept;
      NOD() const InfoType* GetInfoEnd() const noexcept;

      NOD() Count GetCountDeep(const CT::Block auto&) const noexcept;
      NOD() Count GetCountElementsDeep(const CT::Block auto&) const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      template<CT::Map>
      NOD() decltype(auto) GetKey(CT::Index auto);
      template<CT::Map>
      NOD() decltype(auto) GetKey(CT::Index auto) const;
      template<CT::Map>
      NOD() decltype(auto) GetValue(CT::Index auto);
      template<CT::Map>
      NOD() decltype(auto) GetValue(CT::Index auto) const;
      template<CT::Map>
      NOD() auto GetPair (CT::Index auto);
      template<CT::Map>
      NOD() auto GetPair (CT::Index auto) const;

   protected:
      template<CT::Map, CT::Index INDEX>
      NOD() Offset SimplifyIndex(INDEX) const
      noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>);

      NOD() static Offset GetBucket(Offset, const CT::NotSemantic auto&) noexcept;
      NOD() static Offset GetBucketUnknown(Offset, const Block&) noexcept;

      template<CT::Map>
      NOD() decltype(auto) GetRawKey(Offset) const IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() decltype(auto) GetRawKey(Offset)       IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() decltype(auto) GetKeyRef(Offset) const IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() decltype(auto) GetKeyRef(Offset)       IF_UNSAFE(noexcept);

      template<CT::Map>
      NOD() decltype(auto) GetRawVal(Offset) const IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() decltype(auto) GetRawVal(Offset)       IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() decltype(auto) GetValRef(Offset) const IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() decltype(auto) GetValRef(Offset)       IF_UNSAFE(noexcept);

      template<CT::Map>
      NOD() auto GetKeyHandle(Offset) const IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() auto GetValHandle(Offset) const IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<class MAP>
      struct Iterator;

      template<CT::Map MAP>
      NOD() Iterator<MAP> begin() noexcept;
      template<CT::Map MAP>
      NOD() Iterator<const MAP> begin() const noexcept;

      template<CT::Map MAP>
      NOD() Iterator<MAP> last() noexcept;
      template<CT::Map MAP>
      NOD() Iterator<const MAP> last() const noexcept;

      constexpr A::IteratorEnd end() const noexcept { return {}; }

      template<bool REVERSE = false, CT::Map>
      Count ForEach(auto&&) const;

      template<bool REVERSE = false, CT::Map>
      Count ForEachKeyElement(auto&&) const;

      template<bool REVERSE = false, CT::Map>
      Count ForEachValueElement(auto&&) const;

      template<bool REVERSE = false, CT::Map>
      Count ForEachKey(auto&&...) const;

      template<bool REVERSE = false, CT::Map>
      Count ForEachValue(auto&&...) const;
   
      template<bool REVERSE = false, bool SKIP = true, CT::Map>
      Count ForEachKeyDeep(auto&&...) const;

      template<bool REVERSE = false, bool SKIP = true, CT::Map>
      Count ForEachValueDeep(auto&&...) const;

   protected:
      template<CT::Map, bool REVERSE>
      LoopControl ForEachElementInner(const CT::Block auto&, auto&&, Count&) const;

      template<CT::Map, bool REVERSE>
      LoopControl ForEachInner(const CT::Block auto&, auto&&, Count&) const;

      template<CT::Map, bool REVERSE, bool SKIP>
      LoopControl ForEachDeepInner(const CT::Block auto&, auto&&, Count&) const;

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Map THIS, CT::Data, CT::Data...>
      NOD() constexpr bool IsKey() const noexcept;
      template<CT::Map THIS>
      NOD() bool IsKey(DMeta) const noexcept;

      template<CT::Map THIS, CT::Data, CT::Data...>
      NOD() constexpr bool IsKeySimilar() const noexcept;
      template<CT::Map THIS>
      NOD() bool IsKeySimilar(DMeta) const noexcept;

      template<CT::Map THIS, CT::Data, CT::Data...>
      NOD() constexpr bool IsKeyExact() const noexcept;
      template<CT::Map THIS>
      NOD() bool IsKeyExact(DMeta) const noexcept;

      template<CT::Map THIS, CT::Data, CT::Data...>
      NOD() constexpr bool IsValue() const noexcept;
      template<CT::Map THIS>
      NOD() bool IsValue(DMeta) const noexcept;

      template<CT::Map THIS, CT::Data, CT::Data...>
      NOD() constexpr bool IsValueSimilar() const noexcept;
      template<CT::Map THIS>
      NOD() bool IsValueSimilar(DMeta) const noexcept;

      template<CT::Map THIS, CT::Data, CT::Data...>
      NOD() constexpr bool IsValueExact() const noexcept;
      template<CT::Map THIS>
      NOD() bool IsValueExact(DMeta) const noexcept;

   protected:
      template<CT::Map, CT::NotSemantic, CT::NotSemantic>
      void Mutate();
      template<CT::Map>
      void Mutate(DMeta, DMeta);

      template<CT::Map>
      NOD() constexpr bool IsTypeCompatibleWith(CT::Map  auto const&) const noexcept;
      template<CT::Map>
      NOD() constexpr bool IsTypeCompatibleWith(CT::Pair auto const&) const noexcept;

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::Map = UnorderedMap>
      bool operator == (CT::Map  auto const&) const;
      template<CT::Map = UnorderedMap>
      bool operator == (CT::Pair auto const&) const;

      template<CT::Map = UnorderedMap>
      NOD() Hash GetHash() const;

      template<CT::Map = UnorderedMap>
      NOD() bool ContainsKey(const CT::NotSemantic auto&) const;
      template<CT::Map = UnorderedMap>
      NOD() bool ContainsValue(const CT::NotSemantic auto&) const;
      template<CT::Map = UnorderedMap>
      NOD() bool ContainsPair(const CT::Pair auto&) const;

      template<CT::Map = UnorderedMap>
      NOD() Index Find(const CT::NotSemantic auto&) const;
      template<CT::Map THIS = UnorderedMap>
      NOD() Iterator<THIS> FindIt(const CT::NotSemantic auto&);
      template<CT::Map THIS = UnorderedMap>
      NOD() Iterator<const THIS> FindIt(const CT::NotSemantic auto&) const;

      template<CT::Map = UnorderedMap>
      NOD() decltype(auto) At(const CT::NotSemantic auto&);
      template<CT::Map = UnorderedMap>
      NOD() decltype(auto) At(const CT::NotSemantic auto&) const;

      template<CT::Map = UnorderedMap>
      NOD() decltype(auto) operator[] (const CT::NotSemantic auto&);
      template<CT::Map = UnorderedMap>
      NOD() decltype(auto) operator[] (const CT::NotSemantic auto&) const;

   protected:
      template<CT::Map>
      NOD() Offset FindInner(const CT::NotSemantic auto&) const;
      template<CT::Map>
      NOD() Offset FindBlockInner(const Block&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<CT::Map>
      void Reserve(Count);

   protected:
      /// @cond show_protected                                                
      template<CT::Map>
      void AllocateFresh(Count);
      template<CT::Map, bool REUSE>
      void AllocateData(Count);
      template<CT::Map>
      void AllocateInner(Count);

      void Keep() const noexcept;
      template<CT::Map>
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::Map>
      Count Insert(auto&&, auto&&);

      template<CT::Map, class T1, class T2>
      requires CT::Block<Desem<T1>, Desem<T2>>
      Count InsertBlock(T1&&, T2&&);

      template<CT::Map, class T1, class...TAIL>
      Count InsertPair(T1&&, TAIL&&...);

   protected:
      template<CT::Map THIS>
      auto CreateKeyHandle(CT::Semantic auto&&);
      template<CT::Map THIS>
      auto CreateValHandle(CT::Semantic auto&&);

      template<CT::Map>
      NOD() Size RequestKeyAndInfoSize(Count, Offset&) const IF_UNSAFE(noexcept);
      NOD() Size RequestValuesSize(Count) const IF_UNSAFE(noexcept);
      
      template<CT::Map>
      void Rehash(Count);
      template<CT::Map>
      void RehashKeys(BlockMap&);
      template<CT::Map>
      void RehashVals(BlockMap&);
      template<CT::Map>
      void ShiftPairs();

      template<CT::Map, bool CHECK_FOR_MATCH>
      Offset InsertInner(Offset, CT::Semantic auto&&, CT::Semantic auto&&);

      template<CT::Map, bool CHECK_FOR_MATCH, template<class> class S1, template<class> class S2, CT::Block T>
      requires CT::Semantic<S1<T>, S2<T>>
      Offset InsertBlockInner(Offset, S1<T>&&, S2<T>&&);

      template<CT::Map, bool CHECK_FOR_MATCH, template<class> class S, CT::Pair T>
      requires CT::Semantic<S<T>>
      Count InsertPairInner(Count, S<T>&&);

      template<CT::Map>
      Count UnfoldInsert(auto&&);

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::Map>
      Count RemoveKey(const CT::NotSemantic auto&);
      template<CT::Map>
      Count RemoveValue(const CT::NotSemantic auto&);
      template<CT::Map>
      Count RemovePair(const CT::Pair auto&);
      template<CT::Map THIS>
      Iterator<THIS> RemoveIt(const Iterator<THIS>&);

      template<CT::Map>
      void Clear();
      template<CT::Map>
      void Reset();
      template<CT::Map>
      void Compact();

   protected:
      template<CT::Map>
      Count RemoveKeyInner(const CT::NotSemantic auto&);
      template<CT::Map>
      Count RemoveValInner(const CT::NotSemantic auto&);
      template<CT::Map>
      Count RemovePairInner(const CT::Pair auto&);

      template<CT::Map>
      void RemoveInner(Offset);
      template<CT::Map, bool FORCE = true>
      void ClearPartInner(CT::Block auto&);

   #if LANGULUS(TESTING)
      public: NOD() constexpr const void* GetRawKeysMemory() const noexcept;
      public: NOD() constexpr const void* GetRawValsMemory() const noexcept;
   #endif
   };


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        
   template<class MAP>
   struct BlockMap::Iterator : A::Iterator {
      static_assert(CT::Map<MAP>, "MAP must be a CT::Map type");
      static constexpr bool Mutable = CT::Mutable<MAP>;

      // Key type is always constant, because changing it will mean     
      // rehashing the entire table, so we forbid it while iterating    
      using Key   = const typename MAP::Key;
      using Value = Conditional<Mutable, typename MAP::Value,
                                         const typename MAP::Value>;
      using Pair  = Conditional<Mutable, typename MAP::PairRef,
                                         typename MAP::PairConstRef>;
      using KA = Conditional<CT::TypeErased<Key>,   Block, Key*>;
      using VA = Conditional<CT::TypeErased<Value>, Block, Value*>;

      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED)    Pair;

      KA mKey;
      VA mValue;

   protected:
      friend struct BlockMap;
      const InfoType* mInfo;
      const InfoType* mSentinel;

      constexpr Iterator(const InfoType*, const InfoType*, const KA&, const VA&) noexcept;

   public:
      Iterator() noexcept = delete;
      constexpr Iterator(const Iterator&) noexcept = default;
      constexpr Iterator(Iterator&&) noexcept = default;
      constexpr Iterator(const A::IteratorEnd&) noexcept;

      constexpr Iterator& operator = (const Iterator&) noexcept = default;
      constexpr Iterator& operator = (Iterator&&) noexcept = default;

      NOD() constexpr bool operator == (const Iterator&) const noexcept;
      NOD() constexpr bool operator == (const A::IteratorEnd&) const noexcept;

      NOD() constexpr auto operator * () const;

      // Prefix operator                                                
      constexpr Iterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() constexpr Iterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
      constexpr operator Iterator<const MAP>() const noexcept requires Mutable {
         return {mInfo, mSentinel, mKey, mValue};
      }
   };

} // namespace Langulus::Anyness