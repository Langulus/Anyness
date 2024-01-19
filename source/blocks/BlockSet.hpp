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


namespace Langulus
{
   namespace A
   {

      ///                                                                     
      /// An abstract Set structure                                           
      /// It defines the size for CT::Set concept                             
      ///                                                                     
      struct BlockSet {
         LANGULUS(ABSTRACT) true;
         LANGULUS(POD) true;

         using InfoType = ::std::uint8_t;
         using OrderType = Offset;

         static constexpr bool Sequential = false;
         static constexpr Offset InvalidOffset = -1;
         static constexpr Count MinimalAllocation = 8;

      protected:
         // A precomputed pointer for the info (and ordering) bytes     
         // Points to an offset inside mKeys allocation                 
         // Each byte represents an entry, and can be three things:     
         //    0 - the index is not used, data is not initialized       
         //    1 - the index is used, key is exactly where it should be 
         //   2+ - the index is used, but bucket is info-1 buckets to   
         //         the right of this index                             
         InfoType* mInfo {};

         // The block that contains the keys and info bytes             
         Anyness::Block mKeys;

      public:
         constexpr BlockSet() noexcept = default;
         constexpr BlockSet(const BlockSet&) noexcept = default;
         constexpr BlockSet(BlockSet&&) noexcept = default;

         constexpr BlockSet& operator = (const BlockSet&) noexcept = default;
         constexpr BlockSet& operator = (BlockSet&&) noexcept = default;
      };

   } // namespace Langulus::A

   namespace CT
   {

      /// A reflected set type is any type that publicly inherits A::BlockSet 
      /// and is binary compatible to it                                      
      /// Keep in mind, that sparse types are never considered CT::Set!       
      template<class...T>
      concept Set = ((DerivedFrom<T, A::BlockSet>
          and sizeof(T) == sizeof(A::BlockSet)) and ...);

      /// Check if a type is a statically typed set                           
      template<class...T>
      concept TypedSet = Set<T...> and Typed<T...>;

   } // namespace Langulus::CT

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased set block, base for all set types                        
   ///                                                                        
   ///   This is an inner structure, that doesn't reference any memory,       
   /// only provides the functionality to do so. You can use BlockSet as a    
   /// lightweight intermediate structure for iteration, etc.                 
   ///                                                                        
   struct BlockSet : A::BlockSet {
      LANGULUS(ABSTRACT) false;

      static constexpr bool Ownership = false;
      static constexpr bool Ordered = false;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      using A::BlockSet::BlockSet;
      using A::BlockSet::operator =;

   protected:
      template<CT::Set TO, template<class> class S, CT::Set FROM>
      requires CT::Semantic<S<FROM>>
      void BlockTransfer(S<FROM>&&);

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      template<CT::Set = UnorderedSet>
      NOD() DMeta GetType() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsUntyped() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsDeep() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsSparse() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsDense() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr DataState GetState() const noexcept;
      NOD() constexpr Count GetCount() const noexcept;
      NOD() Count GetCountDeep() const noexcept;
      NOD() Count GetCountElementsDeep() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;
      NOD() bool IsMissing() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() bool IsMissingDeep() const;
      template<CT::Set>
      NOD() bool IsOrdered() const noexcept;

      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      DEBUGGERY(template<CT::Set> void Dump() const);

   protected:
      template<CT::Set>
      NOD() auto& GetValues() const noexcept;
      template<CT::Set>
      NOD() auto& GetValues() noexcept;

      NOD() InfoType const* GetInfo() const noexcept;
      NOD() InfoType*       GetInfo() noexcept;
      NOD() InfoType const* GetInfoEnd() const noexcept;

      NOD() Count GetCountDeep(const Block&) const noexcept;
      NOD() Count GetCountElementsDeep(const Block&) const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) Get(CT::Index auto) const;

      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) operator[] (CT::Index auto) const;

   protected:
      template<CT::Set, CT::Index INDEX>
      NOD() Offset SimplifyIndex(INDEX) const
      noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>);

      NOD() static Offset GetBucket(Offset, const CT::NotSemantic auto&) noexcept;
      NOD() static Offset GetBucketUnknown(Offset, const Block&) noexcept;

      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRaw(Offset)       IF_UNSAFE(noexcept);
      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRaw(Offset) const IF_UNSAFE(noexcept);

      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRef(Offset)       IF_UNSAFE(noexcept);
      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRef(Offset) const IF_UNSAFE(noexcept);

      template<CT::Set = UnorderedSet>
      NOD() auto GetHandle(Offset) const IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<class Set>
      struct Iterator;

      template<CT::Set SET>
      NOD() Iterator<SET> begin() noexcept;
      template<CT::Set SET>
      NOD() Iterator<const SET> begin() const noexcept;

      template<CT::Set SET>
      NOD() Iterator<SET> last() noexcept;
      template<CT::Set SET>
      NOD() Iterator<const SET> last() const noexcept;

      constexpr A::IteratorEnd end() const noexcept { return {}; }

      template<bool REVERSE = false, CT::Set>
      Count ForEach(auto&&...) const;

      template<bool REVERSE = false, CT::Set>
      Count ForEachElement(auto&&) const;

      template<bool REVERSE = false, bool SKIP = true, CT::Set>
      Count ForEachDeep(auto&&...) const;

   protected:
      template<class F>
      static constexpr bool NoexceptIterator = not LANGULUS_SAFE()
         and noexcept(Fake<F&&>().operator() (Fake<ArgumentOf<F>>()));

      template<CT::Set, class R, CT::Data A, bool REVERSE>
      Count ForEachInner(auto&& f) const noexcept(NoexceptIterator<decltype(f)>);

      template<CT::Set, class R, CT::Data A, bool REVERSE, bool SKIP>
      Count ForEachDeepInner(auto&&) const;

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Set, CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      template<CT::Set>
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Set, CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      template<CT::Set>
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Set, CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      template<CT::Set>
      NOD() bool IsExact(DMeta) const noexcept;

   protected:
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsTypeCompatibleWith(CT::Set auto const&) const noexcept;
      
   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::Set = UnorderedSet>
      bool operator == (const CT::NotSemantic auto&) const;

      template<CT::Set = UnorderedSet>
      NOD() Hash GetHash() const;

      template<CT::Set = UnorderedSet>
      NOD() bool Contains(const CT::NotSemantic auto&) const;

      template<CT::Set = UnorderedSet>
      NOD() Index Find(const CT::NotSemantic auto&) const;
      template<CT::Set THIS = UnorderedSet>
      NOD() Iterator<THIS> FindIt(const CT::NotSemantic auto&);
      template<CT::Set THIS = UnorderedSet>
      NOD() Iterator<const THIS> FindIt(const CT::NotSemantic auto&) const;

   protected:
      template<CT::NotSemantic>
      void Mutate();
      void Mutate(DMeta);

      template<CT::Set>
      NOD() Offset FindInner(const CT::NotSemantic auto&) const;
      template<CT::Set>
      NOD() Offset FindBlockInner(const Block&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<CT::Set = UnorderedSet>
      void Reserve(Count);

   protected:
      /// @cond show_protected                                                
      template<CT::Set>
      void AllocateFresh(Count);
      template<CT::Set, bool REUSE>
      void AllocateData(Count);
      template<CT::Set>
      void AllocateInner(Count);

      void Reference(Count) const noexcept;
      void Keep() const noexcept;
      template<CT::Set>
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::Set = UnorderedSet, class T1, class...TAIL>
      Count Insert(T1&&, TAIL&&...);

      template<CT::Set = UnorderedSet, class T>
      requires CT::Set<Desem<T>>
      Count InsertBlock(T&&);

   protected:
      template<CT::Set THIS>
      auto CreateValHandle(CT::Semantic auto&&);

      template<CT::Set>
      NOD() Size RequestKeyAndInfoSize(Count, Offset&) const IF_UNSAFE(noexcept);

      template<CT::Set>
      void Rehash(Count);
      template<CT::Set>
      void ShiftPairs();

      template<CT::Set, bool CHECK_FOR_MATCH, template<class> class S, CT::Data T>
      requires CT::Semantic<S<T>>
      Offset InsertInner(Offset, S<T>&&);

      template<CT::Set, bool CHECK_FOR_MATCH, template<class> class S>
      requires CT::Semantic<S<Block>>
      Offset InsertBlockInner(Offset, S<Block>&&);

      template<CT::Set>
      Count UnfoldInsert(auto&&);

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::Set = UnorderedSet>
      Count Remove(const CT::NotSemantic auto&);

      template<CT::Set = UnorderedSet>
      void Clear();
      template<CT::Set = UnorderedSet>
      void Reset();
      template<CT::Set = UnorderedSet>
      void Compact();

   protected:
      template<CT::Set>
      void ClearInner();

      template<CT::Set>
      void RemoveInner(Offset) IF_UNSAFE(noexcept);
      template<CT::Set>
      Count RemoveKeyInner(const CT::NotSemantic auto&);

   #if LANGULUS(TESTING)
      public: NOD() constexpr const void* GetRawMemory() const noexcept;
      public: NOD() const Allocation* GetEntry() const noexcept;
   #endif
   };


   ///                                                                        
   ///   Set iterator                                                         
   ///                                                                        
   template<class SET>
   struct BlockSet::Iterator : A::Iterator {
      static_assert(CT::Set<SET>, "SET must be a CT::Set type");
      static constexpr bool Mutable = CT::Mutable<SET>;

      using T = Conditional<CT::Typed<SET>
         , Conditional<Mutable, TypeOf<SET>, const TypeOf<SET>>
         , void>;

      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED)    T;

   protected:
      friend struct BlockSet;
      using InnerT = Conditional<CT::Typed<SET>, T*, Block>;

      const InfoType* mInfo;
      const InfoType* mSentinel;
      InnerT mKey;

      constexpr Iterator(const InfoType*, const InfoType*, const InnerT&) noexcept;

   public:
      Iterator() noexcept = delete;
      constexpr Iterator(const Iterator&) noexcept = default;
      constexpr Iterator(Iterator&&) noexcept = default;
      constexpr Iterator(const A::IteratorEnd&) noexcept;

      constexpr Iterator& operator = (const Iterator&) noexcept = default;
      constexpr Iterator& operator = (Iterator&&) noexcept = default;

      NOD() constexpr bool operator == (const Iterator&) const noexcept;
      NOD() constexpr bool operator == (const A::IteratorEnd&) const noexcept;

      NOD() constexpr decltype(auto) operator * () const;

      // Prefix operator                                                
      constexpr Iterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() constexpr Iterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
      constexpr operator Iterator<const SET>() const noexcept requires Mutable;
   };

} // namespace Langulus::Anyness
