///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../inner/Index.hpp"
#include "../inner/Allocator.hpp"
#include <utility>

namespace Langulus::Anyness
{
   
   /// Predeclarations                                                        
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
   class Path;
   
   template<CT::Data>
   class TOwned;
   template<class, bool>
   class TPointer;

   /// Compression types, analogous to zlib's                                 
   enum class Compression {
      None = 0,
      Fastest = 1,
      Balanced = 5,
      Smallest = 9,
      
      Default = Fastest
   };

   
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
      LANGULUS(UNINSERTABLE) true;
      static constexpr bool Ownership = false;

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
      friend class Path;

      template<CT::Data>
      friend class TOwned;
      template<class, bool REFERENCED>
      friend class TPointer;

      friend class ::Langulus::Flow::Verb;
      template<class VERB>
      friend struct ::Langulus::Flow::StaticVerb;
      template<class VERB, bool NOEXCEPT>
      friend struct ::Langulus::Flow::ArithmeticVerb;

      /// A structure used to represent an element of a sparse container      
      struct KnownPointer;

   private:
   /// @cond show_protected                                                   
   TESTING(public:)
      union {
         #if LANGULUS_DEBUG()
            char* mRawChar;
         #endif
         // Raw pointer to first element inside the memory block        
         Byte* mRaw {};
         KnownPointer* mRawSparse;
      };
   
      // The data state                                                 
      DataState mState {DataState::Default};
      // Number of initialized elements inside memory block             
      Count mCount {};
      // Number of allocated elements in the memory block               
      Count mReserved {};
      // Meta data about the elements inside the memory block           
      DMeta mType {};
      // Pointer to the allocated block                                 
      // If entry is zero, then data is static                          
      Inner::Allocation* mEntry {};
   /// @endcond                                                               

   public:
      constexpr Block() noexcept = default;
      constexpr Block(const Block&) noexcept = default;
      constexpr Block(Block&&) noexcept = default;

      constexpr Block(Disowned<Block>&&) noexcept;
      constexpr Block(Abandoned<Block>&&) noexcept;
         
      explicit constexpr Block(DMeta) noexcept;
      constexpr Block(const DataState&, DMeta) noexcept;

      Block(const DataState&, DMeta, Count, const void*) SAFETY_NOEXCEPT();
      Block(const DataState&, DMeta, Count, void*) SAFETY_NOEXCEPT();
      constexpr Block(const DataState&, DMeta, Count, const void*, Inner::Allocation*) noexcept;
      constexpr Block(const DataState&, DMeta, Count, void*, Inner::Allocation*) noexcept;
   
      template<CT::Data T, bool CONSTRAIN = false>
      NOD() static Block From(T) requires CT::Sparse<T>;
      template<CT::Data T, bool CONSTRAIN = false>
      NOD() static Block From(T, Count) requires CT::Sparse<T>;
      template<CT::Data T, bool CONSTRAIN = false>
      NOD() static Block From(T&) requires CT::Dense<T>;
      template<CT::Data T, bool CONSTRAIN = false>
      NOD() static Block From();

      constexpr Block& operator = (const Block&) noexcept = default;
      constexpr Block& operator = (Block&&) noexcept = default;
         
      void TakeAuthority();
      void Optimize();

   public:
      //                                                                
      // Capsulation and access                                         
      //                                                                
      constexpr void SetState(DataState) noexcept;
      constexpr void AddState(DataState) noexcept;
      constexpr void RemoveState(DataState) noexcept;
   
      NOD() constexpr const DMeta& GetType() const noexcept;
      NOD() constexpr const Count& GetCount() const noexcept;
      NOD() constexpr const Count& GetReserved() const noexcept;
      NOD() constexpr Size GetReservedSize() const noexcept;
      NOD() Count GetCountDeep() const noexcept;
      NOD() Count GetCountElementsDeep() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;
      NOD() constexpr bool IsPast() const noexcept;
      NOD() constexpr bool IsFuture() const noexcept;
      NOD() constexpr bool IsNow() const noexcept;
      NOD() constexpr bool IsMissing() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsEncrypted() const noexcept;
      NOD() constexpr bool IsCompressed() const noexcept;
      NOD() constexpr bool IsConstant() const noexcept;
      NOD() constexpr bool IsMutable() const noexcept;
      NOD() constexpr bool IsStatic() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsDefaultable() const noexcept;
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
      NOD() constexpr bool CanFitPhase(const Block&) const noexcept;
      NOD() constexpr bool CanFitState(const Block&) const noexcept;
      NOD() constexpr bool CanFitOrAnd(const Block&) const noexcept;
      NOD() constexpr Count GetByteSize() const noexcept;
      NOD() constexpr Token GetToken() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr const DataState& GetState() const noexcept;
      NOD() constexpr DataState GetUnconstrainedState() const noexcept;
      NOD() constexpr Byte* GetRaw() noexcept;
      NOD() constexpr const Byte* GetRaw() const noexcept;
      NOD() constexpr Byte* GetRawEnd() noexcept;
      NOD() constexpr const Byte* GetRawEnd() const noexcept;
      NOD() constexpr KnownPointer* GetRawSparse() noexcept;
      NOD() constexpr const KnownPointer* GetRawSparse() const noexcept;
      
      NOD() constexpr bool IsMissingDeep() const;
      
      template<CT::Data T>
      NOD() T* GetRawAs() noexcept;
      template<CT::Data T>
      NOD() const T* GetRawAs() const noexcept;
      
      template<CT::Data T>
      NOD() T* GetRawEndAs() noexcept;
      template<CT::Data T>
      NOD() const T* GetRawEndAs() const noexcept;

      NOD() bool IsConcatable(const Block&) const noexcept;
      
      NOD() bool IsInsertable(DMeta) const noexcept;
      template<CT::Data>
      NOD() bool IsInsertable() const noexcept;
   
      NOD() Byte* At(const Offset& = 0) SAFETY_NOEXCEPT();
      NOD() const Byte* At(const Offset& = 0) const SAFETY_NOEXCEPT();
   
      template<CT::Data>
      NOD() decltype(auto) Get(const Offset& = 0, const Offset& = 0) SAFETY_NOEXCEPT();
      template<CT::Data>
      NOD() decltype(auto) Get(const Offset& = 0, const Offset& = 0) const SAFETY_NOEXCEPT();
   
      template<CT::Data, CT::Index IDX = Offset>
      NOD() decltype(auto) As(const IDX& = {});
      template<CT::Data, CT::Index IDX = Offset>
      NOD() decltype(auto) As(const IDX& = {}) const;
   
      template<CT::Data T, bool FATAL_FAILURE = true>
      NOD() T AsCast(Index) const;
      template<CT::Data T, bool FATAL_FAILURE = true>
      NOD() T AsCast() const;
   
      NOD() Block GetElementDense(Offset);
      NOD() const Block GetElementDense(Offset) const;
   
      NOD() Block GetElementResolved(Offset);
      NOD() const Block GetElementResolved(Offset) const;
   
      NOD() Block GetElement(Offset) noexcept;
      NOD() const Block GetElement(Offset) const noexcept;
   
      NOD() Block GetElement() noexcept;
      NOD() const Block GetElement() const noexcept;
   
      NOD() Block* GetBlockDeep(Offset) noexcept;
      NOD() const Block* GetBlockDeep(Offset) const noexcept;
   
      NOD() Block GetElementDeep(Offset) noexcept;
      NOD() const Block GetElementDeep(Offset) const noexcept;

      NOD() Block GetResolved() noexcept;
      NOD() const Block GetResolved() const noexcept;

      NOD() Block GetDense() noexcept;
      NOD() const Block GetDense() const noexcept;

      NOD() auto RequestSize(const Count&) const noexcept;

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<CT::Index IDX = Offset>
      NOD() Block operator[] (const IDX&) const;
      template<CT::Index IDX = Offset>
      NOD() Block operator[] (const IDX&);

      template<bool MUTABLE = true, class F>
      Count ForEachElement(F&&);
      template<class F>
      Count ForEachElement(F&&) const;
      
      template<bool MUTABLE = true, class... F>
      Count ForEach(F&&...);
      template<class... F>
      Count ForEach(F&&...) const;
   
      template<bool MUTABLE = true, class... F>
      Count ForEachRev(F&&...);
      template<class... F>
      Count ForEachRev(F&&...) const;
   
      template<bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachDeep(F&&...);
      template<bool SKIP = true, class... F>
      Count ForEachDeep(F&&...) const;
   
      template<bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachDeepRev(F&&...);
      template<bool SKIP = true, class... F>
      Count ForEachDeepRev(F&&...) const;
      
   private:
      template<bool MUTABLE, bool REVERSE, class F>
      Count ForEachSplitter(F&&);
      template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
      Count ForEachDeepSplitter(F&&);

      template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
      Count ForEachInner(TFunctor<R(A)>&&);
      template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
      Count ForEachDeepInner(TFunctor<R(A)>&&);
   
      NOD() Block CropInner(const Offset&, const Count&, const Count&) const noexcept;
   
   public:
      NOD() bool Owns(const void*) const noexcept;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;
      NOD() Block Crop(const Offset&, const Count&);
      NOD() Block Crop(const Offset&, const Count&) const;
   
      NOD() Block GetMember(const RTTI::Member&) const;
      NOD() Block GetMember(const RTTI::Member&);
      NOD() Block GetMember(TMeta) const;
      NOD() Block GetMember(TMeta);
      NOD() Block GetMember(DMeta) const;
      NOD() Block GetMember(DMeta);
      NOD() Block GetMember(::std::nullptr_t) const;
      NOD() Block GetMember(::std::nullptr_t);

      template<CT::Index INDEX>
      NOD() Block GetMember(TMeta, const INDEX&) const;
      template<CT::Index INDEX>
      NOD() Block GetMember(TMeta, const INDEX&);
      template<CT::Index INDEX>
      NOD() Block GetMember(DMeta, const INDEX&) const;
      template<CT::Index INDEX>
      NOD() Block GetMember(DMeta, const INDEX&);
      template<CT::Index INDEX>
      NOD() Block GetMember(::std::nullptr_t, const INDEX&) const;
      template<CT::Index INDEX>
      NOD() Block GetMember(::std::nullptr_t, const INDEX&);
   
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&) const;
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&);
      NOD() Block GetBaseMemory(const RTTI::Base&) const;
      NOD() Block GetBaseMemory(const RTTI::Base&);
   
      template<CT::Data, bool ALLOW_DEEPEN, CT::Data = Any>
      bool Mutate();
      template<bool ALLOW_DEEPEN, CT::Data = Any>
      bool Mutate(DMeta);

      constexpr void MakeStatic(bool enable = true) noexcept;
      constexpr void MakeConst(bool enable = true) noexcept;
      constexpr void MakeTypeConstrained(bool enable = true) noexcept;
      constexpr void MakeOr() noexcept;
      constexpr void MakeAnd() noexcept;
      constexpr void MakePast() noexcept;
      constexpr void MakeFuture() noexcept;
      constexpr void MakeNow() noexcept;
      constexpr void MakeSparse() noexcept;
      constexpr void MakeDense() noexcept;
   
      void Reserve(Count);
      Count Copy(Block&) const;
      Count Clone(Block&) const;
   
      #if LANGULUS_FEATURE(ZLIB)
         Size Compress(Block&, Compression = Compression::Default) const;
         Size Decompress(Block&) const;
      #endif

      Size Encrypt(Block&, const ::std::size_t*, const Count&) const;
      Size Decrypt(Block&, const ::std::size_t*, const Count&) const;
   
      NOD() Hash GetHash() const;
   
      template<bool REVERSE = false, bool BY_ADDRESS_ONLY = false, CT::NotSemantic T>
      NOD() Index FindKnown(const T&, const Offset& = 0) const;
      template<bool REVERSE = false, bool BY_ADDRESS_ONLY = false>
      NOD() Index FindUnknown(const Block&, const Offset& = 0) const;
      template<bool REVERSE = false, CT::NotSemantic T>
      NOD() Index FindDeep(const T&, Offset = 0) const;
   
      template<CT::Data, CT::Index INDEX1, CT::Index INDEX2>
      void Swap(INDEX1, INDEX2);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      NOD() bool CompareStates(const Block&) const noexcept;
      NOD() bool CompareTypes(const Block&, RTTI::Base&) const noexcept;
      template<bool RESOLVE = true>
      NOD() bool Compare(const Block&) const;

      template<CT::NotSemantic T>
      bool operator == (const T&) const;

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count InsertAt(const T*, const T*, INDEX);
      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count InsertAt(const T&, INDEX);
      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count InsertAt(T&&, INDEX);
      template<bool MUTABLE = true, CT::Data = Any, CT::Semantic S, CT::Index INDEX>
      Count InsertAt(S&&, INDEX);

      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Insert(const T*, const T*);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Insert(const T&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Insert(T&&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::Semantic S>
      Count Insert(S&&);

      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count MergeAt(const T*, const T*, INDEX);
      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count MergeAt(const T&, INDEX);
      template<bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count MergeAt(T&&, INDEX);
      template<bool MUTABLE = true, CT::Data = Any, CT::Semantic S, CT::Index INDEX>
      Count MergeAt(S&&, INDEX);

      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Merge(const T*, const T*);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Merge(const T&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::NotSemantic T>
      Count Merge(T&&);
      template<Index = IndexBack, bool MUTABLE = true, CT::Data = Any, CT::Semantic S>
      Count Merge(S&&);

      template<CT::NotSemantic T, CT::Index INDEX>
      Count InsertBlockAt(const T&, INDEX);
      template<CT::NotSemantic T, CT::Index INDEX>
      Count InsertBlockAt(T&&, INDEX);
      template<CT::Semantic S, CT::Index INDEX>
      Count InsertBlockAt(S&&, INDEX);

      template<Index = IndexBack, CT::NotSemantic T>
      Count InsertBlock(const T&);
      template<Index = IndexBack, CT::NotSemantic T>
      Count InsertBlock(T&&);
      template<Index = IndexBack, CT::Semantic S>
      Count InsertBlock(S&&);

      template<CT::NotSemantic T, CT::Index INDEX>
      Count MergeBlockAt(const T&, INDEX);
      template<CT::NotSemantic T, CT::Index INDEX>
      Count MergeBlockAt(T&&, INDEX);
      template<CT::Semantic S, CT::Index INDEX>
      Count MergeBlockAt(S&&, INDEX);
   
      template<Index = IndexBack, CT::NotSemantic T>
      Count MergeBlock(const T&);
      template<Index = IndexBack, CT::NotSemantic T>
      Count MergeBlock(T&&);
      template<Index = IndexBack, CT::Semantic S>
      Count MergeBlock(S&&);
   
      template<CT::Data T, bool MOVE_STATE = true>
      T& Deepen();

      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count SmartPushAt(const T&, INDEX, DataState = {});
      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count SmartPushAt(T&, INDEX, DataState = {});
      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::NotSemantic T, CT::Index INDEX>
      Count SmartPushAt(T&&, INDEX, DataState = {});
      template<bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::Semantic S, CT::Index INDEX>
      Count SmartPushAt(S&&, INDEX, DataState = {});

      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::NotSemantic T>
      Count SmartPush(const T&, DataState = {});
      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::NotSemantic T>
      Count SmartPush(T&, DataState = {});
      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::NotSemantic T>
      Count SmartPush(T&&, DataState = {});
      template<Index = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data = Any, CT::Semantic S>
      Count SmartPush(S&&, DataState = {});

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false, bool BY_ADDRESS_ONLY = false, CT::Data T>
      Count RemoveValue(const T&);
      template<CT::Index INDEX>
      Count RemoveIndex(INDEX, Count = 1);
      template<CT::Index INDEX>
      Count RemoveIndexDeep(INDEX);
   
      Block& Trim(Offset);
   
      NOD() constexpr Index Constrain(const Index&) const noexcept;
   
      template<CT::Data>
      NOD() Index ConstrainMore(const Index&) const noexcept;
   
      template<CT::Data T>
      NOD() Index GetIndexMax() const noexcept requires CT::Sortable<T, T>;
   
      template<CT::Data T>
      NOD() Index GetIndexMin() const noexcept requires CT::Sortable<T, T>;
   
      template<CT::Data>
      NOD() Index GetIndexMode(Count&) const noexcept;
   
      template<CT::Data>
      void Sort(const Index&) noexcept;
   
      template<bool BINARY_COMPATIBLE = false>
      NOD() bool CastsToMeta(DMeta) const;
      template<bool BINARY_COMPATIBLE = false>
      NOD() bool CastsToMeta(DMeta, Count) const;

      template<CT::Data, bool BINARY_COMPATIBLE = false>
      NOD() bool CastsTo() const;
      template<CT::Data, bool BINARY_COMPATIBLE = false>
      NOD() bool CastsTo(Count) const;
   
      NOD() bool Is(DMeta) const noexcept;
      template<CT::Data...>
      NOD() bool Is() const;
      template<CT::Data...>
      NOD() bool IsExact() const;
      
      NOD() Block ReinterpretAs(const Block&) const;
      template<CT::Data>
      NOD() Block ReinterpretAs() const;
   
      void Clear();
      void Reset();

   protected:
      /// @cond show_protected                                                
      template<class TO, CT::Semantic S>
      void BlockTransfer(S&&);

      template<bool CONSTRAIN>
      void SetType(DMeta);
      template<CT::Data, bool CONSTRAIN>
      void SetType();

      template<class T>
      NOD() bool CompareSingleValue(const T&) const;

      template<class, bool COUNT_CONSTRAINED = true, CT::Index INDEX>
      Offset SimplifyIndex(const INDEX&) const;

      template<bool ALLOW_DEEPEN, CT::Data = Any, CT::Semantic S, CT::Index INDEX>
      Count SmartConcatAt(const bool&, S&&, const DataState&, const INDEX&);
      template<bool ALLOW_DEEPEN, Index INDEX = IndexBack, CT::Data = Any, CT::Semantic S>
      Count SmartConcat(const bool&, S&&, const DataState&);
      
      template<bool ALLOW_DEEPEN, CT::Data = Any, CT::Semantic S, CT::Index INDEX>
      Count SmartPushAtInner(S&&, const DataState&, const INDEX&);
      template<bool ALLOW_DEEPEN, Index INDEX = IndexBack, CT::Data = Any, CT::Semantic S>
      Count SmartPushInner(S&&, const DataState&);

      template<bool CREATE = false, bool SETSIZE = false>
      void AllocateMore(Count);
      void AllocateLess(Count);

      template<bool CREATE = false>
      void AllocateInner(const Count&);
      void AllocateFresh(const RTTI::AllocationRequest&);
      void AllocateRegion(const Block&, Offset, Block&);

      template<CT::Semantic S, CT::NotSemantic T>
      void InsertInner(const T*, const T*, Offset);
      template<CT::Semantic S>
      void InsertInner(S&&, Offset);
      template<Offset, CT::Data HEAD, CT::Data... TAIL>
      void InsertStatic(HEAD&&, TAIL&&...);

      template<CT::Semantic S>
      void Absorb(S&&, const DataState&);

      Count GatherInner(const Block&, Block&, Index);
      Count GatherPolarInner(DMeta, const Block&, Block&, Index, DataState);

      static void CopyMemory(const void*, void*, const Size&) noexcept;
      static void MoveMemory(const void*, void*, const Size&) noexcept;
      static void FillMemory(void*, Byte, const Size&) noexcept;
      NOD() static int CompareMemory(const void*, const void*, const Size&) noexcept;
      
      constexpr void ClearInner() noexcept;
      constexpr void ResetMemory() noexcept;
      constexpr void ResetState() noexcept;
      constexpr void ResetType() noexcept;
   
      void Reference(const Count&) const noexcept;
      void Keep() const noexcept;
      
      template<bool DESTROY>
      bool Dereference(const Count&);
      bool Free();
   
      void CallUnknownDefaultConstructors(Count) const;
      template<CT::Data T>
      void CallKnownDefaultConstructors(Count) const;

      void CallUnknownDescriptorConstructors(Count, const Any&) const;
      template<CT::Data>
      void CallKnownDescriptorConstructors(Count, const Any&) const;

      template<CT::Data, class... A>
      void CallKnownConstructors(Count, A&&...) const;

      template<bool REVERSE = false, CT::Semantic S>
      void CallUnknownSemanticConstructors(Count, S&&) const;
      template<CT::Data, bool REVERSE = false, CT::Semantic S>
      void CallKnownSemanticConstructors(Count, S&&) const;

      template<CT::Semantic S>
      void CallUnknownSemanticAssignment(Count, S&&) const;
      template<CT::Data, CT::Semantic S>
      void CallKnownSemanticAssignment(Count, S&&) const;

      void CallUnknownDestructors() const;
      template<CT::Data>
      void CallKnownDestructors() const;
   
      template<CT::Semantic S>
      void SwapUnknown(S&&);
      template<CT::Data>
      void SwapKnown(Block&);

      NOD() bool CallComparer(const Block&, const RTTI::Base&) const;

      void Next() noexcept;
      void Prev() noexcept;
      NOD() Block Next() const noexcept;
      NOD() Block Prev() const noexcept;
      /// @endcond                                                            
   };


   ///                                                                        
   ///   A resolved pointer inside a sparse block                             
   ///                                                                        
   struct Block::KnownPointer {
      friend class Block;

   private:
      /// @cond show_protected                                                
      TESTING(public:)
      Byte* mPointer {};
      Inner::Allocation* mEntry {};
      /// @endcond show_protected                                             

   public:
      constexpr KnownPointer() noexcept = default;
      KnownPointer(const KnownPointer&) noexcept;
      KnownPointer(KnownPointer&&) noexcept;
      KnownPointer(Disowned<KnownPointer>&&) noexcept;
      KnownPointer(Abandoned<KnownPointer>&&) noexcept;

      constexpr KnownPointer(const void* pointer, Inner::Allocation* entry) noexcept;

      template<CT::Sparse T>
      explicit KnownPointer(T);
      template<CT::Sparse T>
      KnownPointer(Disowned<T>&&) noexcept;
      ~KnownPointer() noexcept = default;

      KnownPointer& operator = (const KnownPointer&) const = delete;

      bool operator == (const void*) const noexcept;

      template<bool KEEP, bool DESTROY_OLD = true>
      void MoveAssign(DMeta, KnownPointer*);
      template<bool KEEP, bool DESTROY_OLD = true>
      void CopyAssign(DMeta, const KnownPointer*);

      template<CT::Sparse T, bool DESTROY>
      void Free() noexcept;
      template<bool DESTROY>
      void Free(DMeta) noexcept;

      template<class T>
      const Decay<T>* As() const noexcept {
         return reinterpret_cast<const Decay<T>*>(mPointer);
      }
   };


   /// Macro used to implement the standard container interface used in       
   /// range-for-statements like for (auto& item : array)                     
   /// In addition, a Reverse() function is added for reverse iteration       
   #define RANGED_FOR_INTEGRATION(containerType, iteratorType) \
      NOD() inline iteratorType* begin() noexcept { return GetRaw(); } \
      NOD() inline iteratorType* end() noexcept { return GetRawEnd(); } \
      NOD() inline iteratorType& last() noexcept { return *(GetRawEnd() - 1); } \
      NOD() inline const iteratorType* begin() const noexcept { return GetRaw(); } \
      NOD() inline const iteratorType* end() const noexcept { return GetRawEnd(); } \
      NOD() inline const iteratorType& last() const noexcept { return *(GetRawEnd() - 1); } \
      NOD() inline auto& size() const noexcept { return GetCount(); } \
      NOD() inline Inner::TReverse<const containerType, iteratorType> Reverse() const noexcept { return {*this}; } \
      NOD() inline Inner::TReverse<containerType, iteratorType> Reverse() noexcept { return {*this}; }

   namespace Inner
   {

      ///                                                                     
      ///   Reverse adaptor for ranged-for expressions                        
      ///                                                                     
      template<class T, class E>
      class TReverse {
      private:
         using ITERATOR = ::std::conditional_t<::std::is_const_v<T>, const E*, E*>;
         T& mContainer;

      public:
         TReverse() = delete;
         TReverse(const TReverse&) = delete;
         TReverse(TReverse&&) = delete;
         TReverse(T&) noexcept;

         struct PointerWrapper {
            PointerWrapper() = delete;
            PointerWrapper(const PointerWrapper&) noexcept = default;
            PointerWrapper(PointerWrapper&&) noexcept = default;
            PointerWrapper(ITERATOR) noexcept;

            bool operator == (ITERATOR) const noexcept;
            bool operator != (ITERATOR) const noexcept;
            PointerWrapper& operator ++ () noexcept;
            PointerWrapper& operator -- () noexcept;
            operator ITERATOR () const noexcept;

            ITERATOR mPointer;
         };

         NOD() PointerWrapper begin() const noexcept;
         NOD() PointerWrapper end() const noexcept;
         NOD() E& last() const noexcept;
         NOD() Count size() const noexcept;
      };

   } // namespace Langulus::Anyness::Inner

} // namespace Langulus::Anyness


namespace Langulus::CT
{

   /// A reflected block type is any type that inherits Block, and is         
   /// binary compatible to a Block - this is a mandatory requirement for     
   /// any CT::Deep type                                                      
   /// Keep in mind, that sparse types are never considered Block!            
   template<class... T>
   concept Block = ((DerivedFrom<T, ::Langulus::Anyness::Block>
      && sizeof(T) == sizeof(::Langulus::Anyness::Block)) && ...);

   /// A deep type is any type with a true static member T::CTTI_Deep,        
   /// is binary compatible with Block, as well as having the same interface  
   /// If no such member/base exists, the type is assumed NOT deep by         
   /// default. Deep types are considered iteratable, and verbs are           
   /// executed in each of their elements/members, instead on the type        
   /// itself. Use LANGULUS(DEEP) macro as member to tag deep types           
   /// Keep in mind, that sparse types are never considered Deep!             
   template<class... T>
   concept Deep = ((Block<T> && Decay<T>::CTTI_Deep) && ...);

   /// Type that is not deep, see CT::Deep                                    
   template<class... T>
   concept Flat = ((!Deep<T>) && ...);

   /// Check if a type can be handled generically by templates, and           
   /// doesn't	require any special handling                                   
   template<class... T>
   concept CustomData = ((Data<T> && Flat<T> && NotSemantic<T>) && ...);

} // namespace Langulus::CT

#include "Block.inl"
