///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../inner/DataState.hpp"
#include "../inner/Compare.hpp"
#include "../inner/Index.hpp"
#include "../inner/Iterator.hpp"


namespace Langulus::Flow
{

   /// Just syntax sugar, for breaking ForEach loop                           
   constexpr bool Break = false;
   /// Just syntax sugar, for continuing ForEach loop                         
   constexpr bool Continue = true;

} // namespace Langulus::Flow

namespace Langulus
{
   namespace A
   {
      struct Handle {};
   }

   namespace CT
   {
      namespace Inner
      {
         template<class T>
         concept Iteratable = requires (T a) {
            {a.begin()} -> Data;
            {a.end()} -> Data;
         };

         template<class T>
         concept IteratableInReverse = requires (T a) {
            {a.rbegin()} -> Data;
            {a.rend()} -> Data;
         };
      }

      template<class... T>
      concept Handle = (DerivedFrom<T, A::Handle> and ...);

      template<class... T>
      concept NotHandle = not Handle<T...>;

      template<class... T>
      concept Iteratable = (Inner::Iteratable<T> and ...);

      template<class... T>
      concept IteratableInReverse = (Inner::IteratableInReverse<T> and ...);
   }
}

namespace Langulus::Anyness
{

   using RTTI::AllocationRequest;
   using RTTI::MetaData;
   using RTTI::MetaConst;
   using RTTI::MetaTrait;
   using RTTI::MetaVerb;
   using RTTI::DMeta;
   using RTTI::CMeta;
   using RTTI::TMeta;
   using RTTI::VMeta;

   template<class, bool EMBED = true>
   struct Handle;

   class Any;
   template<CT::Data>
   class TAny;
   
   class BlockMap;
   class OrderedMap;
   class UnorderedMap;
   template<CT::Data, CT::Data>
   class TOrderedMap;
   template<CT::Data, CT::Data>
   class TUnorderedMap;
   
   class BlockSet;
   class OrderedSet;
   class UnorderedSet;
   template<CT::Data>
   class TOrderedSet;
   template<CT::Data>
   class TUnorderedSet;
   
   class Bytes;
   class Text;
   struct Path;
   
   template<CT::Data>
   class TOwned;
   template<class, bool>
   class TPointer;
   
   class Construct;
   using Messy = Any;
   class Neat;

#if LANGULUS_FEATURE(COMPRESSION)
   /// Compression types, analogous to zlib's                                 
   enum class Compression {
      None = 0,
      Fastest = 1,
      Balanced = 5,
      Smallest = 9,
      
      Default = Fastest
   };
#endif

   
   ///                                                                        
   ///	Memory block                                                         
   ///                                                                        
   ///   Wraps an allocated memory block; acts as base to all containers.     
   ///   This is an inner structure, that doesn't reference any memory,       
   /// only provides the functionality to do so. You can use Block as a       
   /// lightweight intermediate structure for iteration, etc.                 
   ///                                                                        
   class Block {
   public:
      LANGULUS(DEEP) true;
      LANGULUS(POD) true;

      static constexpr bool Ownership = false;
      static constexpr bool Sequential = true;

      friend class Any;
      template<CT::Data>
      friend class TAny;

      friend class BlockMap;
      friend class OrderedMap;
      friend class UnorderedMap;
      template<CT::Data, CT::Data>
      friend class TOrderedMap;
      template<CT::Data, CT::Data>
      friend class TUnorderedMap;

      friend class BlockSet;
      friend class OrderedSet;
      friend class UnorderedSet;
      template<CT::Data>
      friend class TOrderedSet;
      template<CT::Data>
      friend class TUnorderedSet;

      friend class Bytes;
      friend class Text;
      friend struct Path;

      template<CT::Data>
      friend class TOwned;
      template<class, bool>
      friend class TPointer;

      friend class Neat;
      friend class Construct;

      friend class ::Langulus::Flow::Verb;
      template<class>
      friend struct ::Langulus::Flow::StaticVerb;
      template<class, bool>
      friend struct ::Langulus::Flow::ArithmeticVerb;

   public:
      union {
         #if LANGULUS_DEBUG()
            char* mRawChar;
         #endif
         // Raw pointer to first element inside the memory block        
         Byte* mRaw {};
         Byte** mRawSparse;
      };
   
      // The data state                                                 
      DataState mState {DataState::Default};
      // Number of initialized elements inside memory block             
      Count mCount {};
      // Number of allocated elements in the memory block               
      Count mReserved {};
      // Meta data about the elements inside the memory block           
      mutable DMeta mType {};
      // Pointer to the allocated block. If entry is zero, then data is 
      // static, or we simply have no authority over it (just a view)   
      const Allocation* mEntry {};

   public:
      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr Block() noexcept = default;
      constexpr Block(const Block&) noexcept = default;
      constexpr Block(Block&&) noexcept = default;
      constexpr Block(CT::Semantic auto&&) noexcept;
         
      constexpr Block(DMeta) noexcept;
      constexpr Block(const DataState&, DMeta) noexcept;

      Block(const DataState&, CMeta) IF_UNSAFE(noexcept);
      Block(const DataState&, DMeta, Count, const void*) IF_UNSAFE(noexcept);
      Block(const DataState&, DMeta, Count, void*) IF_UNSAFE(noexcept);
      Block(const DataState&, DMeta, Count, const void*, const Allocation*) IF_UNSAFE(noexcept);
      Block(const DataState&, DMeta, Count, void*, const Allocation*) IF_UNSAFE(noexcept);
   
      template<bool CONSTRAIN = false, CT::Data T>
      NOD() static Block From(T) requires CT::Sparse<T>;
      template<bool CONSTRAIN = false, CT::Data T>
      NOD() static Block From(T, Count) requires CT::Sparse<T>;
      template<bool CONSTRAIN = false, CT::Data T>
      NOD() static Block From(T&) requires CT::Dense<T>;
      template<CT::Data T, bool CONSTRAIN = false>
      NOD() static Block From();

      constexpr Block& operator = (const Block&) noexcept = default;
      constexpr Block& operator = (Block&&) noexcept = default;
      constexpr Block& operator = (CT::Semantic auto&&) noexcept;
         
   protected:
      template<class TO>
      void BlockTransfer(CT::Semantic auto&&);
      void CreateFrom(CT::Semantic auto&&);
      void SwapUnknown(CT::Semantic auto&&);
      template<CT::Data>
      void SwapKnown(Block&);

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      constexpr void SetState(DataState) noexcept;
      constexpr void AddState(DataState) noexcept;
      constexpr void RemoveState(DataState) noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      NOD() bool Owns(const void*) const noexcept;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;
      NOD() constexpr DMeta GetType() const noexcept;
      NOD() constexpr Count GetCount() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr Size GetReservedSize() const noexcept;
      NOD() Count GetCountDeep() const noexcept;
      NOD() Count GetCountElementsDeep() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;
      NOD() constexpr bool IsPast() const noexcept;
      NOD() constexpr bool IsFuture() const noexcept;
      NOD() constexpr bool IsNow() const noexcept;
      NOD() constexpr bool IsMissing() const noexcept;
      NOD() constexpr bool IsTyped() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsEncrypted() const noexcept;
      NOD() constexpr bool IsCompressed() const noexcept;
      NOD() constexpr bool IsConstant() const noexcept;
      NOD() constexpr bool IsMutable() const noexcept;
      NOD() constexpr bool IsStatic() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsOr() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsValid() const noexcept;
      NOD() constexpr bool IsInvalid() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsPOD() const noexcept;
      NOD() constexpr bool IsResolvable() const noexcept;
      NOD() constexpr bool IsNullifiable() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsBlock() const noexcept;
      NOD() constexpr bool CanFitPhase(const Block&) const noexcept;
      NOD() constexpr bool CanFitState(const Block&) const noexcept;
      NOD() constexpr bool CanFitOrAnd(const Block&) const noexcept;
      NOD() constexpr Count GetBytesize() const noexcept;
      NOD() constexpr Token GetToken() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr DataState GetState() const noexcept;
      NOD() constexpr DataState GetUnconstrainedState() const noexcept;
      NOD() constexpr bool IsMissingDeep() const;
      NOD() constexpr bool IsConcatable(const Block&) const noexcept;
      NOD() constexpr bool IsInsertable(DMeta) const noexcept;
      template<CT::Data>
      NOD() constexpr bool IsInsertable() const noexcept;

      NOD() constexpr Byte* GetRaw() noexcept;
      NOD() constexpr const Byte* GetRaw() const noexcept;
      NOD() constexpr const Byte* GetRawEnd() const noexcept;

      NOD() IF_UNSAFE(constexpr)
      Byte** GetRawSparse() IF_UNSAFE(noexcept);
      NOD() IF_UNSAFE(constexpr)
      const Byte* const* GetRawSparse() const IF_UNSAFE(noexcept);

      template<CT::Data T>
      NOD() T* GetRawAs() noexcept;
      template<CT::Data T>
      NOD() const T* GetRawAs() const noexcept;
      template<CT::Data T>
      NOD() const T* GetRawEndAs() const noexcept;

      constexpr void MakeStatic(bool enable = true) noexcept;
      constexpr void MakeConst(bool enable = true) noexcept;
      constexpr void MakeTypeConstrained(bool enable = true) noexcept;
      constexpr void MakeOr() noexcept;
      constexpr void MakeAnd() noexcept;
      constexpr void MakePast() noexcept;
      constexpr void MakeFuture() noexcept;
      constexpr void MakeNow() noexcept;

   protected: IF_LANGULUS_TESTING(public:)
      NOD() const Allocation* const* GetEntries() const IF_UNSAFE(noexcept);
      NOD() const Allocation**       GetEntries()       IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      template<bool COUNT_CONSTRAINED = true>
      NOD() constexpr Index Constrain(const Index&) const noexcept;
      template<CT::Data, bool COUNT_CONSTRAINED = true>
      NOD() Index ConstrainMore(const Index&) const IF_UNSAFE(noexcept);

      NOD() IF_UNSAFE(constexpr)
      Byte* At(const Offset& = 0) IF_UNSAFE(noexcept);
      NOD() IF_UNSAFE(constexpr)
      const Byte* At(const Offset& = 0) const IF_UNSAFE(noexcept);
   
      template<CT::Index IDX = Offset>
      NOD() Block operator[] (const IDX&);
      template<CT::Index IDX = Offset>
      NOD() Block operator[] (const IDX&) const;

      template<CT::Data>
      NOD() IF_UNSAFE(constexpr)
      decltype(auto) Get(const Offset& = 0, const Offset& = 0) IF_UNSAFE(noexcept);
      template<CT::Data>
      NOD() IF_UNSAFE(constexpr)
      decltype(auto) Get(const Offset& = 0, const Offset& = 0) const IF_UNSAFE(noexcept);
   
      template<CT::Data, CT::Index IDX = Offset>
      NOD() decltype(auto) As(const IDX& = {});
      template<CT::Data, CT::Index IDX = Offset>
      NOD() decltype(auto) As(const IDX& = {}) const;
   
      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      // If you receive missing externals, include the following:       
      //    #include <Flow/Verbs/Interpret.hpp>                         
      template<CT::Data T, bool FATAL_FAILURE = true, CT::Index IDX = Offset>
      NOD() T AsCast(const IDX& = {}) const;
   
      NOD() IF_UNSAFE(constexpr)
      Block Crop(const Offset&, const Count&) IF_UNSAFE(noexcept);
      NOD() IF_UNSAFE(constexpr)
      Block Crop(const Offset&, const Count&) const IF_UNSAFE(noexcept);

      NOD() Block GetElementDense(Offset);
      NOD() Block GetElementDense(Offset) const;
   
      NOD() Block GetElementResolved(Offset);
      NOD() Block GetElementResolved(Offset) const;
   
      NOD() Block GetElement(Offset) IF_UNSAFE(noexcept);
      NOD() Block GetElement(Offset) const IF_UNSAFE(noexcept);
   
      NOD() Block GetElement() IF_UNSAFE(noexcept);
      NOD() Block GetElement() const IF_UNSAFE(noexcept);
   
      NOD() Block* GetBlockDeep(Offset) noexcept;
      NOD() const Block* GetBlockDeep(Offset) const noexcept;
   
      NOD() Block GetElementDeep(Offset) noexcept;
      NOD() Block GetElementDeep(Offset) const noexcept;

      NOD() Block GetResolved();
      NOD() Block GetResolved() const;

      template<Count COUNT = CountMax>
      NOD() Block GetDense();
      template<Count COUNT = CountMax>
      NOD() Block GetDense() const;

      template<CT::Data>
      void Swap(CT::Index auto, CT::Index auto);

      template<CT::Data T, Index INDEX>
      NOD() Index GetIndex() const IF_UNSAFE(noexcept) requires (CT::Sortable<T, T>);
      template<CT::Data>
      NOD() Index GetIndexMode(Count&) const IF_UNSAFE(noexcept);

      template<CT::Data, bool ASCEND = false>
      void Sort() noexcept;

   protected:
      template<CT::Data T>
      NOD() Handle<T> GetHandle(Offset) const IF_UNSAFE(noexcept);

      NOD() Block CropInner(const Offset&, const Count&) const IF_UNSAFE(noexcept);

      void Next() IF_UNSAFE(noexcept);
      void Prev() IF_UNSAFE(noexcept);
      NOD() Block Next() const IF_UNSAFE(noexcept);
      NOD() Block Prev() const IF_UNSAFE(noexcept);

      template<class, bool COUNT_CONSTRAINED = true, CT::Index INDEX>
      Offset SimplifyIndex(const INDEX&) const noexcept(not LANGULUS_SAFE() and CT::Unsigned<INDEX>);

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool REVERSE = false, bool MUTABLE = true>
      Count ForEachElement(auto&&);
      template<bool REVERSE = false>
      Count ForEachElement(auto&&) const;
      
      template<bool REVERSE = false, bool MUTABLE = true, class... F>
      Count ForEach(F&&...);
      template<bool REVERSE = false, class... F>
      Count ForEach(F&&...) const;
   
      template<bool REVERSE = false, bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachDeep(F&&...);
      template<bool REVERSE = false, bool SKIP = true, class... F>
      Count ForEachDeep(F&&...) const;
      
      template<bool MUTABLE = true, class... F>
      Count ForEachElementRev(F&&...);
      template<class... F>
      Count ForEachElementRev(F&&...) const;
      
      template<bool MUTABLE = true, class... F>
      Count ForEachRev(F&&...);
      template<class... F>
      Count ForEachRev(F&&...) const;

      template<bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachDeepRev(F&&...);
      template<bool SKIP = true, class... F>
      Count ForEachDeepRev(F&&...) const;

   protected:
      template<class F>
      static constexpr bool NoexceptIterator = not LANGULUS_SAFE()
         and noexcept(Fake<F&&>().operator() (Fake<ArgumentOf<F>>()));

      template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
      Count ForEachInner(auto&& f) noexcept(NoexceptIterator<decltype(f)>);
      template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
      Count ForEachDeepInner(auto&&);

      template<class R, CT::Data A, bool REVERSE = false, bool MUTABLE = false>
      void IterateInner(auto&& f) noexcept(NoexceptIterator<decltype(f)>);

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() bool Is() const;
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool IsSimilar() const;
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool IsExact() const;
      NOD() bool IsExact(DMeta) const noexcept;

      template<bool BINARY_COMPATIBLE = false>
      NOD() bool CastsToMeta(DMeta) const;
      template<bool BINARY_COMPATIBLE = false>
      NOD() bool CastsToMeta(DMeta, Count) const;

      template<CT::Data, bool BINARY_COMPATIBLE = false>
      NOD() bool CastsTo() const;
      template<CT::Data, bool BINARY_COMPATIBLE = false>
      NOD() bool CastsTo(Count) const;

      NOD() Block ReinterpretAs(const Block&) const;
      template<CT::Data>
      NOD() Block ReinterpretAs() const;

      NOD() Block GetMember(const RTTI::Member&);
      NOD() Block GetMember(const RTTI::Member&) const;
      NOD() Block GetMember(TMeta);
      NOD() Block GetMember(TMeta) const;
      NOD() Block GetMember(DMeta);
      NOD() Block GetMember(DMeta) const;
      NOD() Block GetMember(::std::nullptr_t);
      NOD() Block GetMember(::std::nullptr_t) const;

      NOD() Block GetMember(TMeta, const CT::Index auto&);
      NOD() Block GetMember(TMeta, const CT::Index auto&) const;
      NOD() Block GetMember(DMeta, const CT::Index auto&);
      NOD() Block GetMember(DMeta, const CT::Index auto&) const;
      NOD() Block GetMember(::std::nullptr_t, const CT::Index auto&);
      NOD() Block GetMember(::std::nullptr_t, const CT::Index auto&) const;
   
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&);
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&) const;
      NOD() Block GetBaseMemory(const RTTI::Base&);
      NOD() Block GetBaseMemory(const RTTI::Base&) const;
   
      template<bool CONSTRAIN = false>
      void SetType(DMeta);
      template<CT::Data, bool CONSTRAIN = false>
      void SetType();

   protected:
      template<CT::Data, bool ALLOW_DEEPEN, CT::Data = Any>
      bool Mutate();
      template<bool ALLOW_DEEPEN, CT::Data = Any>
      bool Mutate(DMeta);

      constexpr void ResetType() noexcept;

      template<CT::Data>
      void CheckType() const;

      template<CT::Index INDEX>
      Offset SimplifyMemberIndex(const INDEX&) const
      noexcept(not LANGULUS_SAFE() and CT::Unsigned<INDEX>);

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const CT::NotSemantic auto&) const;

      template<bool RESOLVE = true>
      NOD() bool Compare(const Block&) const;
      NOD() Hash GetHash() const;

      template<bool REVERSE = false>
      NOD() Index FindKnown(const CT::NotSemantic auto&, const Offset& = 0) const;
      template<bool REVERSE = false>
      NOD() Index FindUnknown(const Block&, const Offset& = 0) const;
      template<bool REVERSE = false>
      NOD() Index FindDeep(const CT::NotSemantic auto&, Offset = 0) const;

   protected:
      NOD() bool CompareSingleValue(const CT::NotSemantic auto&) const;
      NOD() bool CompareStates(const Block&) const noexcept;
      NOD() bool CompareTypes(const Block&, RTTI::Base&) const;
      NOD() bool CallComparer(const Block&, const RTTI::Base&) const;

      template<bool REVERSE = false>
      Count GatherInner(const Block&, CT::Data auto&);
      template<bool REVERSE = false>
      Count GatherPolarInner(DMeta, const Block&, CT::Data auto&, DataState);

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      NOD() AllocationRequest RequestSize(const Count&) const IF_UNSAFE(noexcept);
      void Reserve(Count);
      template<bool CREATE = false, bool SETSIZE = false>
      void AllocateMore(Count);
      void AllocateLess(Count);
      void TakeAuthority();

   protected:
      /// @cond show_protected                                                
      template<bool CREATE = false>
      void AllocateInner(const Count&);
      void AllocateFresh(const AllocationRequest&);
      void AllocateRegion(const Block&, Offset, Block&);
      void Reference(const Count&) const noexcept;
      void Keep() const noexcept;
      template<bool DESTROY>
      void Dereference(const Count&);
      void Free();
      void SetMemory(const DataState&, DMeta, Count, const void*) IF_UNSAFE(noexcept);
      void SetMemory(const DataState&, DMeta, Count, void*) IF_UNSAFE(noexcept);
      IF_UNSAFE(constexpr)
      void SetMemory(const DataState&, DMeta, Count, const void*, const Allocation*);
      IF_UNSAFE(constexpr)
      void SetMemory(const DataState&, DMeta, Count, void*, const Allocation*);
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count InsertAt(const T*, const T*, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count InsertAt(const CT::NotSemantic auto&, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count InsertAt(CT::NotSemantic auto&, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count InsertAt(CT::NotSemantic auto&&, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count InsertAt(CT::Semantic auto&&, CT::Index auto);

      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Insert(const T*, const T*);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Insert(const CT::NotSemantic auto&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Insert(CT::NotSemantic auto&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Insert(CT::NotSemantic auto&&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Insert(CT::Semantic auto&&);

      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count MergeAt(const T*, const T*, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count MergeAt(const CT::NotSemantic auto&, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count MergeAt(CT::NotSemantic auto&, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count MergeAt(CT::NotSemantic auto&&, CT::Index auto);
      template<bool MUTABLE = true, CT::Data = Any>
      Count MergeAt(CT::Semantic auto&&, CT::Index auto);

      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Merge(const T*, const T*);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Merge(const CT::NotSemantic auto&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Merge(CT::NotSemantic auto&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Merge(CT::NotSemantic auto&&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any>
      Count Merge(CT::Semantic auto&&);

      Count InsertBlockAt(const CT::NotSemantic auto&,  CT::Index auto);
      Count InsertBlockAt(      CT::NotSemantic auto&,  CT::Index auto);
      Count InsertBlockAt(      CT::NotSemantic auto&&, CT::Index auto);
      Count InsertBlockAt(      CT::Semantic    auto&&, CT::Index auto);

      template<Index = IndexBack>
      Count InsertBlock(const CT::NotSemantic auto&);
      template<Index = IndexBack>
      Count InsertBlock(CT::NotSemantic auto&);
      template<Index = IndexBack>
      Count InsertBlock(CT::NotSemantic auto&&);
      template<Index = IndexBack>
      Count InsertBlock(CT::Semantic auto&&);

      Count MergeBlockAt(const CT::NotSemantic auto&,  CT::Index auto);
      Count MergeBlockAt(      CT::NotSemantic auto&,  CT::Index auto);
      Count MergeBlockAt(      CT::NotSemantic auto&&, CT::Index auto);
      Count MergeBlockAt(      CT::Semantic    auto&&, CT::Index auto);
   
      template<Index = IndexBack>
      Count MergeBlock(const CT::NotSemantic auto&);
      template<Index = IndexBack>
      Count MergeBlock(CT::NotSemantic auto&);
      template<Index = IndexBack>
      Count MergeBlock(CT::NotSemantic auto&&);
      template<Index = IndexBack>
      Count MergeBlock(CT::Semantic auto&&);
   
      template<CT::Index IDX = Offset, class... A>
      Count EmplaceAt(const IDX&, A&&...);
      template<Index = IndexBack, class... A>
      Count Emplace(A&&...);
      template<class... A>
      Count New(Count, A&&...);

      template<CT::Data T, bool MOVE_STATE = true>
      T& Deepen();

      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPushAt(const CT::NotSemantic auto&, CT::Index auto, DataState = {});
      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPushAt(CT::NotSemantic auto&, CT::Index auto, DataState = {});
      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPushAt(CT::NotSemantic auto&&, CT::Index auto, DataState = {});
      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPushAt(CT::Semantic auto&&, CT::Index auto, DataState = {});

      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPush(const CT::NotSemantic auto&, DataState = {});
      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPush(CT::NotSemantic auto&, DataState = {});
      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPush(CT::NotSemantic auto&&, DataState = {});
      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any>
      Count SmartPush(CT::Semantic auto&&, DataState = {});

   protected:
      template<template<class> class S, CT::NotSemantic T> requires CT::Semantic<S<T>>
      void InsertInner(const T*, const T*, Offset);
      void InsertInner(CT::Semantic auto&&, Offset);
      template<class... A>
      void EmplaceInner(const Block&, Count, A&&... arguments);

      template<bool ALLOW_DEEPEN, CT::Data = Any>
      Count SmartConcatAt(const bool&, CT::Semantic auto&&, const DataState&, const CT::Index auto&);
      template<bool ALLOW_DEEPEN, Index INDEX = IndexBack, CT::Data = Any>
      Count SmartConcat(const bool&, CT::Semantic auto&&, const DataState&);

      template<bool ALLOW_DEEPEN, CT::Data = Any>
      Count SmartPushAtInner(CT::Semantic auto&&, const DataState&, const CT::Index auto&);
      template<bool ALLOW_DEEPEN, Index INDEX = IndexBack, CT::Data = Any>
      Count SmartPushInner(CT::Semantic auto&&, const DataState&);

      void CallUnknownDefaultConstructors(Count) const;
      template<CT::Data T>
      void CallKnownDefaultConstructors(Count) const;

      void CallUnknownDescriptorConstructors(Count, const Neat&) const;
      template<CT::Data>
      void CallKnownDescriptorConstructors(Count, const Neat&) const;

      template<CT::Data, class... A>
      void CallKnownConstructors(Count, A&&...) const;

      template<bool REVERSE = false>
      void CallUnknownSemanticConstructors(Count, CT::Semantic auto&&) const;
      template<CT::Data, bool REVERSE = false>
      void CallKnownSemanticConstructors(Count, CT::Semantic auto&&) const;
      void ShallowBatchPointerConstruction(Count, CT::Semantic auto&&) const;

      template<CT::Data>
      void CallKnownSemanticAssignment(Count, CT::Semantic auto&&) const;

   public:
      void CallUnknownSemanticAssignment(Count, CT::Semantic auto&&) const;

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false>
      Count Remove(const CT::Data auto&);
      Count RemoveIndex(const CT::Index auto&, Count = 1);
      Count RemoveIndexDeep(CT::Index auto);
   
      void Trim(Count);
      void Optimize();
      void Clear();
      void Reset();
      constexpr void ResetState() noexcept;

   protected:
      void CallUnknownDestructors() const;
      template<CT::Data>
      void CallKnownDestructors() const;

      constexpr void ClearInner() noexcept;
      constexpr void ResetMemory() noexcept;

   public:
      ///                                                                     
      ///   Compression                                                       
      ///                                                                     
      #if LANGULUS_FEATURE(COMPRESSION)
         Size Compress(Block&, Compression = Compression::Default) const;
         Size Decompress(Block&) const;
      #endif

      ///                                                                     
      ///   Encryption                                                        
      ///                                                                     
      Size Encrypt(Block&, const ::std::size_t*, const Count&) const;
      Size Decrypt(Block&, const ::std::size_t*, const Count&) const;

      ///                                                                     
      ///   Serialization                                                     
      ///                                                                     
      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      template<bool ENSCOPE = true, class TO, class TO_ORIGINAL = TO>
      NOD() Count Serialize(TO&) const;

      ///                                                                     
      ///   Flow                                                              
      ///                                                                     
      // Intentionally undefined, because it requires Langulus::Flow    
      void Run(Flow::Verb&) const;
      // Intentionally undefined, because it requires Langulus::Flow    
      void Run(Flow::Verb&);
   };

   namespace Inner
   {

      ///                                                                     
      ///   Reverse iteration adapter                                         
      ///                                                                     
      template<CT::IteratableInReverse T>
      class TReverse {
         T& mContainer;

      public:
         auto begin() { return mContainer.rbegin(); }
         auto end()   { return mContainer.rend();   }
      };


      ///                                                                     
      ///   Keep iterator when using ranged-for                               
      ///                                                                     
      /// When doing for(auto i : container), the statement always            
      /// dereferences the iterator and 'i' always ends up with the contained 
      /// type. This counteracts this, and makes 'i' be the iterator type.    
      ///                                                                     
      template<CT::Iteratable T>
      class TKeepIterator {
         T& mContainer;

      public:
         TKeepIterator() = delete;
         TKeepIterator(T& a) : mContainer {a} {}

         struct WrapEnd;

         struct WrapBegin {
         protected:
            friend struct WrapEnd;
            using Type = decltype(Fake<T&>().begin());
            Type mIt;

         public:
            WrapBegin(const Type& it) : mIt {it} {}

            bool operator == (const WrapBegin& rhs) const noexcept {
               return mIt == rhs.mIt;
            }

            decltype(auto) operator *  () const noexcept { return mIt; }
            decltype(auto) operator -> () const noexcept { return mIt; }

            // Prefix operator                                          
            WrapBegin& operator ++ () noexcept { ++mIt; return *this; }

            // Suffix operator                                          
            WrapBegin operator ++ (int) noexcept { return mIt++; }
         };

         struct WrapEnd {
         private:
            using Type = decltype(Fake<T&>().end());
            Type mIt;

         public:
            WrapEnd(const Type& it) : mIt {it} {}

            bool operator == (const WrapBegin& rhs) const noexcept {
               return mIt == rhs.mIt;
            }
         };

      public:
         WrapBegin begin() { return mContainer.begin(); }
         WrapEnd   end  () { return mContainer.end();   }
      };

   } // namespace Langulus::Anyness::Inner

   LANGULUS(INLINED)
   constexpr auto Reverse(CT::IteratableInReverse auto&& what) noexcept {
      return Inner::TReverse<Deref<decltype(what)>> {what};
   }

   LANGULUS(INLINED)
   constexpr auto KeepIterator(CT::Iteratable auto&& what) noexcept {
      return Inner::TKeepIterator<Deref<decltype(what)>> {what};
   }

} // namespace Langulus::Anyness


namespace Langulus::CT
{

   /// Any origin type that inherits Block                                    
   template<class... T>
   concept BlockBased = (DerivedFrom<T, Anyness::Block> and ...);

   /// A reflected block type is any type that inherits Block, and is         
   /// binary compatible to a Block - this is a mandatory requirement for     
   /// any CT::Deep type                                                      
   /// Keep in mind, that sparse types are never considered Block!            
   template<class... T>
   concept Block = BlockBased<T...>
       and ((sizeof(T) == sizeof(Anyness::Block)) and ...);

   /// A deep type is any type with a true static member T::CTTI_Deep,        
   /// is binary compatible with Block, as well as having the same interface  
   /// If no such member/base exists, the type is assumed NOT deep by         
   /// default. Deep types are considered iteratable, and verbs are           
   /// executed in each of their elements/members, instead on the type        
   /// itself. Use LANGULUS(DEEP) macro as member to tag deep types           
   /// Keep in mind, that sparse types are never considered Deep!             
   template<class... T>
   concept Deep = ((Block<T> and Decay<T>::CTTI_Deep) and ...);

   /// Type that is not deep, see CT::Deep                                    
   template<class... T>
   concept Flat = ((not Deep<T>) and ...);

   /// Check if a type can be handled generically by templates, and           
   /// doesn't	require any special handling                                   
   template<class... T>
   concept CustomData = ((Data<T> and Flat<T> and NotSemantic<T>) and ...);

   /// Check if origin of T(s) are Neat(s)                                    
   template<class... T>
   concept Neat = ((Exact<Decay<T>, Anyness::Neat>) and ...);

   /// Check if origin of T(s) aren't Neat(s)                                 
   template<class... T>
   concept Messy = not Neat<T...>;

   /// Check if origin of T(s) are Construct(s)                               
   template<class... T>
   concept Construct = ((Exact<Decay<T>, Anyness::Construct>) and ...);

   /// Check if origin of T(s) aren't Construct(s)                            
   template<class... T>
   concept NotConstruct = not Construct<T...>;

} // namespace Langulus::CT
