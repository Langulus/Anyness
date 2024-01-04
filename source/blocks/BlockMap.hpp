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


namespace Langulus
{
   namespace A
   {

      ///                                                                     
      /// An abstract Map structure                                           
      /// It defines the size for CT::Map concept                             
      ///                                                                     
      struct BlockMap {
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
      using Pair = Anyness::Pair;

      static constexpr bool Ownership = false;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      using A::BlockMap::BlockMap;
      using A::BlockMap::operator =;

   protected:
      template<CT::Map TO, template<class> class S, CT::Map FROM>
      requires CT::Semantic<S<FROM>>
      void BlockTransfer(S<FROM>&&);

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
      template<CT::Map>
      NOD() auto& GetKeys() const noexcept;
      template<CT::Map>
      NOD() auto& GetKeys() noexcept;
      template<CT::Map>
      NOD() auto& GetValues() const noexcept;
      template<CT::Map>
      NOD() auto& GetValues() noexcept;

      NOD() const InfoType* GetInfo() const noexcept;
      NOD()       InfoType* GetInfo() noexcept;
      NOD() const InfoType* GetInfoEnd() const noexcept;

      NOD() Count GetCountDeep(const Block&) const noexcept;
      NOD() Count GetCountElementsDeep(const Block&) const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Block GetKey  (CT::Index auto);
      NOD() Block GetKey  (CT::Index auto) const;
      NOD() Block GetValue(CT::Index auto);
      NOD() Block GetValue(CT::Index auto) const;
      NOD() Pair  GetPair (CT::Index auto);
      NOD() Pair  GetPair (CT::Index auto) const;

   protected:
      NOD() Block GetKeyInner  (Offset)       IF_UNSAFE(noexcept);
      NOD() Block GetKeyInner  (Offset) const IF_UNSAFE(noexcept);
      NOD() Block GetValueInner(Offset)       IF_UNSAFE(noexcept);
      NOD() Block GetValueInner(Offset) const IF_UNSAFE(noexcept);
      NOD() Pair  GetPairInner (Offset)       IF_UNSAFE(noexcept);
      NOD() Pair  GetPairInner (Offset) const IF_UNSAFE(noexcept);

      NOD() static Offset GetBucket(Offset, const CT::NotSemantic auto&) noexcept;
      NOD() static Offset GetBucketUnknown(Offset, const Block&) noexcept;

      template<CT::Map>
      NOD() auto& GetRawKey(Offset) const IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() auto& GetRawKey(Offset)       IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() auto  GetKeyHandle(Offset) const IF_UNSAFE(noexcept);

      template<CT::Map>
      NOD() auto& GetRawValue(Offset) const IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() auto& GetRawValue(Offset)       IF_UNSAFE(noexcept);
      template<CT::Map>
      NOD() auto  GetValueHandle(Offset) const IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<CT::Map MAP>
      struct TIterator;

      NOD() auto begin()       noexcept;
      NOD() auto begin() const noexcept;
      NOD() auto end()       noexcept;
      NOD() auto end() const noexcept;
      NOD() auto last()       noexcept;
      NOD() auto last() const noexcept;

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
      template<CT::Data, CT::Data...>
      NOD() bool KeyIs() const noexcept;
      NOD() bool KeyIs(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool KeyIsSimilar() const noexcept;
      NOD() bool KeyIsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool KeyIsExact() const noexcept;
      NOD() bool KeyIsExact(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool ValueIs() const noexcept;
      NOD() bool ValueIs(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool ValueIsSimilar() const noexcept;
      NOD() bool ValueIsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool ValueIsExact() const noexcept;
      NOD() bool ValueIsExact(DMeta) const noexcept;

   protected:
      template<CT::Map = UnorderedMap, CT::NotSemantic, CT::NotSemantic>
      void Mutate();
      template<CT::Map = UnorderedMap>
      void Mutate(DMeta, DMeta);

      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsTypeCompatibleWith(CT::Map  auto const&) const noexcept;
      template<CT::Map = UnorderedMap>
      NOD() constexpr bool IsTypeCompatibleWith(CT::Pair auto const&) const noexcept;

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::Map = UnorderedMap>
      bool operator == (CT::Map  auto const&) const;
      template<CT::Map = UnorderedMap>
      bool operator == (CT::Pair auto const&) const;

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
      NOD() TIterator<THIS> FindIt(const CT::NotSemantic auto&);
      template<CT::Map THIS = UnorderedMap>
      NOD() TIterator<const THIS> FindIt(const CT::NotSemantic auto&) const;

      template<CT::Map = UnorderedMap>
      NOD() Block At(const CT::NotSemantic auto&);
      template<CT::Map = UnorderedMap>
      NOD() Block At(const CT::NotSemantic auto&) const;

      NOD() Block operator[] (const CT::NotSemantic auto&);
      NOD() Block operator[] (const CT::NotSemantic auto&) const;

   protected:
      template<CT::Map>
      NOD() Offset FindInner(const CT::NotSemantic auto&) const;
      NOD() Offset FindInnerUnknown(const Block&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<CT::Map = UnorderedMap>
      void Reserve(Count);

   protected:
      /// @cond show_protected                                                
      template<CT::Map>
      void AllocateFresh(Count);
      template<CT::Map, bool REUSE>
      void AllocateData(Count);
      template<CT::Map>
      void AllocateInner(Count);

      void Reference(Count) const noexcept;
      void Keep() const noexcept;
      template<CT::Map>
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::Map = UnorderedMap>
      Count Insert(auto&&, auto&&);

      template<CT::Map = UnorderedMap, class T1, class T2>
      requires CT::Block<Desem<T1>, Desem<T2>>
      Count InsertBlock(T1&&, T2&&);

      template<CT::Map = UnorderedMap, class T1, class...TAIL>
      Count InsertPair(T1&&, TAIL&&...);

   protected:
      template<CT::Map>
      NOD() Size RequestKeyAndInfoSize(Count, Offset&) const IF_UNSAFE(noexcept);
      NOD() Size RequestValuesSize(Count) const IF_UNSAFE(noexcept);
      
      template<CT::Map>
      void Rehash(Count);
      template<CT::Map>
      void RehashKeys(Count, Block&);
      template<CT::Map>
      void RehashValues(Count, Block&);
      template<CT::Map>
      void ShiftPairs();

      template<CT::Map, bool CHECK_FOR_MATCH>
      Offset InsertInner(Offset, CT::Semantic auto&&, CT::Semantic auto&&);

      template<CT::Map, bool CHECK_FOR_MATCH, template<class> class S1, template<class> class S2, CT::Block T>
      requires CT::Semantic<S1<T>, S2<T>>
      Offset InsertInnerUnknown(Offset, S1<T>&&, S2<T>&&);

      template<CT::Map, bool CHECK_FOR_MATCH, template<class> class S, CT::Pair T>
      requires CT::Semantic<S<T>>
      void InsertPairInner(Count, S<T>&&);

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::Map = UnorderedMap>
      Count RemoveKey(const CT::NotSemantic auto&);
      template<CT::Map = UnorderedMap>
      Count RemoveValue(const CT::NotSemantic auto&);
      template<CT::Map = UnorderedMap>
      Count RemovePair(const CT::Pair auto&);

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
      Count RemoveValueInner(const CT::NotSemantic auto&);
      template<CT::Map>
      Count RemovePairInner(const CT::Pair auto&);

      template<CT::Map>
      void ClearInner();
      template<CT::Map>
      void RemoveInner(Offset) IF_UNSAFE(noexcept);

   #if LANGULUS(TESTING)
      public: NOD() constexpr const void* GetRawKeysMemory() const noexcept;
      public: NOD() constexpr const void* GetRawValuesMemory() const noexcept;
   #endif
   };


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        
   template<CT::Map MAP>
   struct BlockMap::TIterator {
      static constexpr bool Mutable = CT::Constant<MAP>;
      static constexpr bool Typed = CT::Typed<MAP>;
      using Key   = Conditional<Typed, typename MAP::Key,   void>;
      using Value = Conditional<Typed, typename MAP::Value, void>;
      using Pair  = Conditional<Typed
         , TPair<Conditional<Mutable, Key&, const Key&>, 
                 Conditional<Mutable, Value&, const Value&>>
         , Anyness::Pair>;

      LANGULUS(UNINSERTABLE) true;
      LANGULUS(UNALLOCATABLE) true;
      LANGULUS(TYPED) Conditional<Typed, Pair, void>;

   protected:
      friend struct BlockMap;
      using K = Conditional<Typed, const Key*,   Block>;
      using V = Conditional<Typed, const Value*, Block>;

      const InfoType* mInfo {};
      const InfoType* mSentinel {};
      K mKey;
      V mValue;

      TIterator(const InfoType*, const InfoType*, K&&, V&&) noexcept;

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

      constexpr explicit operator bool() const noexcept;
   };

} // namespace Langulus::Anyness