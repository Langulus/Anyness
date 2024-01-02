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


namespace Langulus
{
   namespace A
   {

      struct Handle {};

      ///                                                                     
      /// An abstract Block structure                                         
      /// It defines the size for CT::Block and CT::Deep concepts             
      ///                                                                     
      struct Block {
         LANGULUS(ABSTRACT) true;
         LANGULUS(DEEP) true;
         LANGULUS(POD) true;
      protected:
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
         // Pointer to the allocated block. If entry is zero, then data 
         // is static, or we simply have no authority over it (just a   
         // view)                                                       
         const Allocation* mEntry {};

      public:
         constexpr Block() noexcept = default;
         constexpr Block(const Block&) noexcept = default;
         constexpr Block(Block&&) noexcept = default;

         constexpr Block(DMeta) noexcept;
         constexpr Block(const DataState&, DMeta) noexcept;

         Block(const DataState&, CMeta) IF_UNSAFE(noexcept);
         Block(const DataState&, DMeta, Count, const void*) IF_UNSAFE(noexcept);
         Block(const DataState&, DMeta, Count, void*) IF_UNSAFE(noexcept);
         Block(const DataState&, DMeta, Count, const void*, const Allocation*) IF_UNSAFE(noexcept);
         Block(const DataState&, DMeta, Count, void*, const Allocation*) IF_UNSAFE(noexcept);

         constexpr Block& operator = (const Block&) noexcept = default;
         constexpr Block& operator = (Block&&) noexcept = default;
      };

   } // namespace Langulus::A

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

      /// Any origin type that inherits A::Block                              
      template<class... T>
      concept BlockBased = (DerivedFrom<T, A::Block> and ...);

      /// A reflected block type is any type that is BlockBased, and is       
      /// binary compatible to a Block - this is a mandatory requirement for  
      /// any CT::Deep type                                                   
      /// Keep in mind, that sparse types are never considered Block!         
      template<class... T>
      concept Block = BlockBased<T...>
          and ((sizeof(T) == sizeof(A::Block)) and ...);

      /// A deep type is any type with a true static member T::CTTI_Deep,     
      /// is binary compatible with Block, and having the same interface      
      /// If no such member/base exists, the type is assumed NOT deep by      
      /// default. Deep types are considered iteratable, and verbs are        
      /// executed in each of their elements/members, instead on the type     
      /// itself. Use LANGULUS(DEEP) macro as member to tag deep types        
      /// Keep in mind, that sparse types are never considered Deep!          
      template<class... T>
      concept Deep = ((Block<T> and Decay<T>::CTTI_Deep) and ...);

      /// Check if Ts can be deepened 'WITH' the provided type                
      template<class WITH, class... T>
      concept CanBeDeepened = Deep<T...> and not CT::Void<WITH>
          and ((not CT::Typed<T> or CT::Similar<WITH, TypeOf<T>>) and ...);

      /// Type that is not deep, see CT::Deep                                 
      template<class... T>
      concept Flat = ((not Deep<T>) and ...);

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

} // namespace Langulus

namespace Langulus::Anyness
{

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
   class Block : public A::Block {
   public:
      LANGULUS(ABSTRACT) false;

      static constexpr bool Ownership = false;
      static constexpr bool Sequential = true;

      friend class Any;
      template<CT::Data>
      friend class TAny;

      friend class BlockMap;
      template<bool>
      friend struct Map;
      template<CT::Data, CT::Data, bool>
      friend struct TMap;

      friend class BlockSet;
      template<bool>
      friend struct Set;
      template<CT::Data, bool>
      friend struct TSet;

      friend class Bytes;
      friend class Text;
      friend struct Path;

      template<CT::Data>
      friend class TOwned;
      template<class, bool>
      friend class TPointer;

      friend class Neat;
      friend class Construct;

      friend struct ::Langulus::Flow::Verb;
      template<class>
      friend struct ::Langulus::Flow::StaticVerb;
      template<class, bool>
      friend struct ::Langulus::Flow::ArithmeticVerb;

   public:
      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      using A::Block::Block;
   
      template<bool CONSTRAIN_TYPE = false>
      NOD() static Block From(auto&&, Count = 1);

      template<CT::Data, bool CONSTRAIN_TYPE = false>
      NOD() static Block From();

      using A::Block::operator =;
         
   protected:
      template<CT::Block TO, template<class> class S, CT::Block FROM>
      requires CT::Semantic<S<FROM>>
      void BlockTransfer(S<FROM>&&);
      void CreateFrom(CT::Semantic auto&&);

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
      NOD() constexpr bool IsOr() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsValid() const noexcept;
      NOD() constexpr bool IsInvalid() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsPOD() const noexcept;
      NOD() constexpr bool IsResolvable() const noexcept;
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

      NOD() constexpr void* GetRaw() noexcept;
      NOD() constexpr const void* GetRaw() const noexcept;
      NOD() constexpr const void* GetRawEnd() const noexcept;

      NOD() void** GetRawSparse() IF_UNSAFE(noexcept);
      NOD() const void* const* GetRawSparse() const IF_UNSAFE(noexcept);

      template<CT::Data T>
      NOD() T* GetRawAs() noexcept;
      template<CT::Data T>
      NOD() const T* GetRawAs() const noexcept;
      template<CT::Data T>
      NOD() const T* GetRawEndAs() const noexcept;

      template<CT::Data T>
      NOD() T** GetRawSparseAs() IF_UNSAFE(noexcept);
      template<CT::Data T>
      NOD() const T* const* GetRawSparseAs() const IF_UNSAFE(noexcept);

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

      template<CT::Data> NOD() IF_UNSAFE(constexpr)
      decltype(auto) Get(const Offset& = 0, const Offset& = 0) IF_UNSAFE(noexcept);
      template<CT::Data> NOD() IF_UNSAFE(constexpr)
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
   
      template<CT::Block THIS> NOD() IF_UNSAFE(constexpr)
      THIS Crop(const Offset&, const Count&) IF_UNSAFE(noexcept);
      template<CT::Block THIS> NOD() IF_UNSAFE(constexpr)
      THIS Crop(const Offset&, const Count&) const IF_UNSAFE(noexcept);

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
      requires CT::Sortable<T, T>
      NOD() Index GetIndex() const IF_UNSAFE(noexcept);

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
      Offset SimplifyIndex(const INDEX&) const
      noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>);

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
      void IterateInner(const Count&, auto&& f) noexcept(NoexceptIterator<decltype(f)>);

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

      NOD() Block GetMember(TMeta, CT::Index auto);
      NOD() Block GetMember(TMeta, CT::Index auto) const;
      NOD() Block GetMember(DMeta, CT::Index auto);
      NOD() Block GetMember(DMeta, CT::Index auto) const;
      NOD() Block GetMember(::std::nullptr_t, CT::Index auto);
      NOD() Block GetMember(::std::nullptr_t, CT::Index auto) const;
   
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&);
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&) const;
      NOD() Block GetBaseMemory(const RTTI::Base&);
      NOD() Block GetBaseMemory(const RTTI::Base&) const;
   
      template<bool CONSTRAIN = false>
      void SetType(DMeta);
      template<CT::Data, bool CONSTRAIN = false>
      void SetType();

   protected:
      template<CT::Block THIS, CT::Data, class FORCE = Any>
      bool Mutate();
      template<CT::Block THIS, class FORCE = Any>
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
      Count GatherInner(const CT::Block auto&, CT::Block auto&);
      template<bool REVERSE = false>
      Count GatherPolarInner(DMeta, const CT::Block auto&, CT::Block auto&, DataState);

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<CT::Block THIS = Any>
      void Reserve(Count);

   protected:
      /// @cond show_protected                                                
      template<CT::Block THIS>
      NOD() AllocationRequest RequestSize(const Count&) const IF_UNSAFE(noexcept);

      template<CT::Block THIS, bool CREATE = false, bool SETSIZE = false>
      void AllocateMore(Count);
      template<CT::Block THIS>
      void AllocateLess(Count);

      template<CT::Block THIS>
      void TakeAuthority();
      template<CT::Block THIS, bool CREATE = false>
      void AllocateInner(const Count&);
      void AllocateFresh(const AllocationRequest&);

      void Reference(const Count&) const noexcept;
      void Keep() const noexcept;
      template<CT::Block THIS = Any>
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
      template<CT::Block THIS = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T1, class...TAIL>
      Count Insert(CT::Index auto, T1&&, TAIL&&...);

      template<CT::Block THIS = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T>
      requires CT::Block<Desem<T>>
      Count InsertBlock(CT::Index auto, T&&);

      template<CT::Block THIS = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T1, class...TAIL>
      Count Merge(CT::Index auto, T1&&, TAIL&&...);

      template<CT::Block THIS = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T>
      requires CT::Block<Desem<T>>
      Count MergeBlock(CT::Index auto, T&&);
   
      template<CT::Block THIS = Any, bool MOVE_ASIDE = true, class...A>
      Count Emplace(CT::Index auto, A&&...);

      template<CT::Block THIS = Any, class...A>
      Count New(Count, A&&...);

      template<bool CONCAT = true, class FORCE = Any, CT::Block THIS = Any>
      Count SmartPush(CT::Index auto, auto&&, DataState = {});

      template<CT::Deep T, bool TRANSFER_OR = true, CT::Block THIS>
      requires CT::CanBeDeepened<T, THIS>
      T& Deepen();

      template<CT::Block THIS>
      void Null(Count);

      template<CT::Block THIS>
      Count New(Count);

   protected:
      template<CT::Block THIS, template<class> class S>
      requires CT::Semantic<S<Block>>
      void SwapInner(S<Block>&&);

      template<CT::Block THIS, class FORCE, bool MOVE_ASIDE, class T = void, template<class> class S>
      requires CT::Semantic<S<Block>>
      void InsertContiguousInner(CT::Index auto, S<Block>&&);

      template<CT::Block THIS, class FORCE, bool MOVE_ASIDE>
      void InsertInner(CT::Index auto, auto&&);

      template<CT::Block THIS, class... A>
      void EmplaceInner(const Block&, Count, A&&...);

      template<CT::Block THIS, class FORCE, bool MOVE_ASIDE>
      Count UnfoldInsert(CT::Index auto, auto&&);
      template<CT::Block THIS, class FORCE, bool MOVE_ASIDE>
      Count UnfoldMerge(CT::Index auto, auto&&);

      template<CT::Block THIS, class FORCE, template<class> class S, CT::Deep T>
      requires CT::Semantic<S<T>>
      Count SmartConcat(const CT::Index auto, bool, S<T>&&, DataState);

      template<CT::Block THIS, class FORCE, template<class> class S, class T>
      requires CT::Semantic<S<T>>
      Count SmartPushInner(const CT::Index auto, S<T>&&, DataState);

      template<CT::Block THIS, template<class> class S, CT::Block T>
      requires CT::Semantic<S<T>>
      THIS ConcatBlock(S<T>&&) const;

      void CallUnknownDefaultConstructors(Count) const;
      template<CT::Data T>
      void CallKnownDefaultConstructors(Count) const;

      void CallUnknownDescriptorConstructors(Count, const Neat&) const;
      template<CT::Data>
      void CallKnownDescriptorConstructors(Count, const Neat&) const;

      template<CT::Data, class... A>
      void CallKnownConstructors(Count, A&&...) const;

      template<bool REVERSE = false, template<class> class S>
      requires CT::Semantic<S<Block>>
      void CallUnknownSemanticConstructors(Count, S<Block>&&) const;

      template<CT::Data, bool REVERSE = false, template<class> class S>
      requires CT::Semantic<S<Block>>
      void CallKnownSemanticConstructors(Count, S<Block>&&) const;

      template<template<class> class S>
      requires CT::Semantic<S<Block>>
      void ShallowBatchPointerConstruction(Count, S<Block>&&) const;

      template<CT::Data, template<class> class S>
      requires CT::Semantic<S<Block>>
      void CallKnownSemanticAssignment(Count, S<Block>&&) const;

   public:
      template<template<class> class S>
      requires CT::Semantic<S<Block>>
      void CallUnknownSemanticAssignment(Count, S<Block>&&) const;

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::Block THIS = Any, bool REVERSE = false>
      Count Remove(const CT::Data auto&);
      template<CT::Block THIS = Any>
      Count RemoveIndex(CT::Index auto, Count = 1);
      template<CT::Block THIS = Any>
      Count RemoveIndexDeep(CT::Index auto);
   
      template<CT::Block THIS = Any>
      void Trim(Count);
      template<CT::Block THIS = Any>
      void Optimize();
      template<CT::Block THIS = Any>
      void Clear();
      template<CT::Block THIS = Any>
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
      template<bool ENSCOPE = true, CT::Block TO, CT::Block TO_ORIGINAL = TO>
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