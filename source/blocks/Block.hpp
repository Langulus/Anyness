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
         Block(const DataState&, DMeta, Count) IF_UNSAFE(noexcept);
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
   struct Block : A::Block {
      LANGULUS(ABSTRACT) false;

      static constexpr bool Ownership = false;
      static constexpr bool Sequential = true;

      friend class Any;
      template<CT::Data>
      friend class TAny;

      friend struct BlockMap;
      template<bool>
      friend struct Map;
      template<CT::Data, CT::Data, bool>
      friend struct TMap;

      friend struct BlockSet;
      template<bool>
      friend struct Set;
      template<CT::Data, bool>
      friend struct TSet;

      friend class Bytes;
      friend struct Text;
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

      template<CT::BlockBased = Any>
      NOD() bool Owns(const void*) const noexcept;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr DMeta GetType() const noexcept;
      NOD() constexpr Count GetCount() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr Size GetReservedSize() const noexcept;
      template<CT::Block = Any>
      NOD() Count GetCountDeep() const noexcept;
      template<CT::Block = Any>
      NOD() Count GetCountElementsDeep() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;
      NOD() constexpr bool IsPast() const noexcept;
      NOD() constexpr bool IsFuture() const noexcept;
      NOD() constexpr bool IsNow() const noexcept;
      NOD() constexpr bool IsMissing() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsTyped() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsUntyped() const noexcept;
      template<CT::BlockBased = Any>
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
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsDense() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsSparse() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsPOD() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsResolvable() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsDeep() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool IsBlock() const noexcept;
      NOD() constexpr bool CanFitPhase(const Block&) const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr bool CanFitState(const Block&) const noexcept;
      NOD() constexpr bool CanFitOrAnd(const Block&) const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr Count GetBytesize() const noexcept;
      NOD() constexpr Token GetToken() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr DataState GetState() const noexcept;
      NOD() constexpr DataState GetUnconstrainedState() const noexcept;
      template<CT::Block = Any>
      NOD() constexpr bool IsMissingDeep() const;
      template<CT::Block = Any>
      NOD() constexpr bool IsConcatable(const CT::Block auto&) const noexcept;
      template<CT::Block = Any>
      NOD() constexpr bool IsInsertable(DMeta) const noexcept;
      template<CT::Data, CT::Block = Any>
      NOD() constexpr bool IsInsertable() const noexcept;

      constexpr void MakeStatic(bool enable = true) noexcept;
      constexpr void MakeConst(bool enable = true) noexcept;
      constexpr void MakeTypeConstrained(bool enable = true) noexcept;
      constexpr void MakeOr() noexcept;
      constexpr void MakeAnd() noexcept;
      constexpr void MakePast() noexcept;
      constexpr void MakeFuture() noexcept;
      constexpr void MakeNow() noexcept;

      template<CT::Block = Any>
      constexpr void ResetState() noexcept;

   protected: IF_LANGULUS_TESTING(public:)
      template<CT::BlockBased = Any>
      NOD() constexpr auto GetRaw() noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr auto GetRaw() const noexcept;
      template<CT::BlockBased = Any>
      NOD() constexpr auto GetRawEnd() const noexcept;

      template<CT::BlockBased = Any>
      NOD() auto GetRawSparse()       IF_UNSAFE(noexcept);
      template<CT::BlockBased = Any>
      NOD() auto GetRawSparse() const IF_UNSAFE(noexcept);

      template<CT::Data T, CT::BlockBased = Any>
      NOD() T*       GetRawAs() noexcept;
      template<CT::Data T, CT::BlockBased = Any>
      NOD() T const* GetRawAs() const noexcept;
      template<CT::Data T, CT::BlockBased = Any>
      NOD() T const* GetRawEndAs() const noexcept;

      template<CT::Data T, CT::BlockBased = Any>
      NOD() T**             GetRawSparseAs()       IF_UNSAFE(noexcept);
      template<CT::Data T, CT::BlockBased = Any>
      NOD() T const* const* GetRawSparseAs() const IF_UNSAFE(noexcept);

      template<CT::BlockBased = Any>
      NOD() const Allocation* const* GetEntries() const IF_UNSAFE(noexcept);
      template<CT::BlockBased = Any>
      NOD() const Allocation**       GetEntries()       IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Block operator[] (CT::Index auto);
      NOD() Block operator[] (CT::Index auto) const;

      template<CT::Data> NOD() IF_UNSAFE(constexpr)
      decltype(auto) Get(Offset = 0, Offset = 0) IF_UNSAFE(noexcept);
      template<CT::Data> NOD() IF_UNSAFE(constexpr)
      decltype(auto) Get(Offset = 0, Offset = 0) const IF_UNSAFE(noexcept);
   
      template<CT::Data>
      NOD() decltype(auto) As(CT::Index auto);
      template<CT::Data>
      NOD() decltype(auto) As(CT::Index auto) const;

      template<CT::Data T>
      NOD() LANGULUS(INLINED) decltype(auto) As() {
         return As<T>(0);
      }

      template<CT::Data T>
      NOD() LANGULUS(INLINED) decltype(auto) As() const {
         return As<T>(0);
      }

      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      // If you receive missing externals, include the following:       
      //    #include <Flow/Verbs/Interpret.hpp>                         
      template<CT::Data T, bool FATAL_FAILURE = true, CT::Block = Any>
      NOD() T AsCast(CT::Index auto) const;

      template<CT::Data T, bool FATAL_FAILURE = true, CT::Block THIS = Any>
      NOD() LANGULUS(INLINED) T AsCast() const {
         return AsCast<T, FATAL_FAILURE, THIS>(0);
      }
   
      template<CT::Block THIS> NOD() IF_UNSAFE(constexpr)
      THIS Crop(Offset, Count) IF_UNSAFE(noexcept);
      template<CT::Block THIS> NOD() IF_UNSAFE(constexpr)
      THIS Crop(Offset, Count) const IF_UNSAFE(noexcept);

      NOD() Block GetElementDense(Offset = 0);
      NOD() Block GetElementDense(Offset = 0) const;
   
      NOD() Block GetElementResolved(Offset = 0);
      NOD() Block GetElementResolved(Offset = 0) const;
   
      NOD() Block GetElement(Offset = 0)       IF_UNSAFE(noexcept);
      NOD() Block GetElement(Offset = 0) const IF_UNSAFE(noexcept);
   
      template<CT::Block = Any>
      NOD() Block*       GetBlockDeep(Offset) noexcept;
      template<CT::Block = Any>
      NOD() Block const* GetBlockDeep(Offset) const noexcept;
   
      template<CT::Block = Any>
      NOD() Block GetElementDeep(Offset) noexcept;
      template<CT::Block = Any>
      NOD() Block GetElementDeep(Offset) const noexcept;

      template<CT::BlockBased = Any>
      NOD() Any GetResolved();
      template<CT::BlockBased = Any>
      NOD() Any GetResolved() const;

      template<Count = CountMax>
      NOD() Any GetDense();
      template<Count = CountMax>
      NOD() Any GetDense() const;

      template<CT::Block = Any>
      void SwapIndices(CT::Index auto, CT::Index auto);

      template<CT::Block = Any, class T> requires CT::Block<Desem<T>>
      void Swap(T&&);

      template<Index INDEX, CT::Block>
      NOD() Index GetIndex() const IF_UNSAFE(noexcept);

      template<CT::Block>
      NOD() Index GetIndexMode(Count&) const IF_UNSAFE(noexcept);

      template<CT::Data, bool ASCEND = false>
      void Sort() noexcept;

   protected:
      NOD() IF_UNSAFE(constexpr)
      Byte* At(Offset = 0) IF_UNSAFE(noexcept);
      NOD() IF_UNSAFE(constexpr)
      const Byte* At(Offset = 0) const IF_UNSAFE(noexcept);
   
      template<CT::Block>
      NOD() Index Constrain(Index) const IF_UNSAFE(noexcept);

      template<CT::Data T, CT::Block = Any>
      NOD() Handle<T> GetHandle(Offset) const IF_UNSAFE(noexcept);

      NOD() Block CropInner(Offset, Count) const IF_UNSAFE(noexcept);

      void Next() IF_UNSAFE(noexcept);
      void Prev() IF_UNSAFE(noexcept);
      NOD() Block Next() const IF_UNSAFE(noexcept);
      NOD() Block Prev() const IF_UNSAFE(noexcept);

      template<CT::Block, CT::Index INDEX>
      Offset SimplifyIndex(INDEX) const
      noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>);

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<class BLOCK>
      struct Iterator;

      template<CT::Block BLOCK = Any>
      NOD() constexpr Iterator<BLOCK> begin() noexcept;
      template<CT::Block BLOCK = Any>
      NOD() constexpr Iterator<const BLOCK> begin() const noexcept;

      template<CT::Block BLOCK = Any>
      NOD() constexpr Iterator<BLOCK> last() noexcept;
      template<CT::Block BLOCK = Any>
      NOD() constexpr Iterator<const BLOCK> last() const noexcept;

      constexpr A::IteratorEnd end() const noexcept { return {}; }

      template<bool REVERSE = false, CT::Block = Any>
      Count ForEachElement(auto&&) const;
      
      template<bool REVERSE = false, CT::Block = Any>
      Count ForEach(auto&&...) const;
   
      template<bool REVERSE = false, bool SKIP = true, CT::Block = Any>
      Count ForEachDeep(auto&&...) const;
      
      template<CT::Block = Any>
      Count ForEachElementRev(auto&&...) const;
      
      template<CT::Block = Any>
      Count ForEachRev(auto&&...) const;

      template<bool SKIP = true, CT::Block = Any>
      Count ForEachDeepRev(auto&&...) const;

   protected:
      template<class F>
      static constexpr bool NoexceptIterator = not LANGULUS_SAFE()
         and noexcept(Fake<F&&>().operator() (Fake<ArgumentOf<F>>()));

      template<CT::Block, class R, CT::Data A, bool REVERSE>
      Count ForEachInner(auto&& f) const noexcept(NoexceptIterator<decltype(f)>);

      template<CT::Block, class R, CT::Data A, bool REVERSE, bool SKIP>
      Count ForEachDeepInner(auto&&) const;

      template<CT::Block, class R, CT::Data A, bool REVERSE = false>
      void IterateInner(Count, auto&& f) const noexcept(NoexceptIterator<decltype(f)>);

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Block, CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      template<CT::Block = Any>
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Block, CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      template<CT::Block = Any>
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Block, CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      template<CT::Block = Any>
      NOD() bool IsExact(DMeta) const noexcept;

      template<bool BINARY_COMPATIBLE = false, CT::Block = Any>
      NOD() bool CastsToMeta(DMeta) const;
      template<bool BINARY_COMPATIBLE = false, CT::Block = Any>
      NOD() bool CastsToMeta(DMeta, Count) const;

      template<CT::Data, bool BINARY_COMPATIBLE = false, CT::Block = Any>
      NOD() bool CastsTo() const;
      template<CT::Data, bool BINARY_COMPATIBLE = false, CT::Block = Any>
      NOD() bool CastsTo(Count) const;

      template<CT::Block = Any>
      NOD() auto    ReinterpretAs(const CT::Block auto&) const;
      template<CT::Data T, CT::Block = Any>
      NOD() TAny<T> ReinterpretAs() const;

      template<CT::Block = Any>
      NOD() Block GetMember(const RTTI::Member&, CT::Index auto);
      template<CT::Block = Any>
      NOD() Block GetMember(const RTTI::Member&, CT::Index auto) const;
   
      template<bool CONSTRAIN = false, CT::Block = Any>
      void SetType(DMeta);
      template<CT::Data, bool CONSTRAIN = false, CT::Block = Any>
      void SetType();

   protected:
      template<CT::Block, CT::Data, class FORCE = Any>
      bool Mutate();
      template<CT::Block, class FORCE = Any>
      bool Mutate(DMeta);

      template<CT::Block>
      constexpr void ResetType() noexcept;

   IF_LANGULUS_TESTING(public:)
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&);
      NOD() Block GetBaseMemory(DMeta, const RTTI::Base&) const;
      NOD() Block GetBaseMemory(const RTTI::Base&);
      NOD() Block GetBaseMemory(const RTTI::Base&) const;

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::Block = Any>
      bool operator == (const CT::NotSemantic auto&) const;

      template<bool RESOLVE = true, CT::Block = Any>
      NOD() bool Compare(const CT::Block auto&) const;
      template<CT::Block = Any>
      NOD() Hash GetHash() const;

      template<bool REVERSE = false, CT::Block = Any>
      NOD() Index Find(const CT::NotSemantic auto&, Offset = 0) const noexcept;
      template<CT::Block THIS = Any>
      NOD() Iterator<THIS> FindIt(const CT::NotSemantic auto&);
      template<CT::Block THIS = Any>
      NOD() Iterator<const THIS> FindIt(const CT::NotSemantic auto&) const;

      template<bool REVERSE = false, CT::Block = Any>
      NOD() Index FindBlock(const CT::Block auto&, Offset = 0) const noexcept;

      template<bool ASCEND = false, CT::Block = Any>
      void Sort();

      template<CT::Block>
      NOD() bool CompareLoose(const CT::Block auto&) const noexcept;
      template<CT::Block>
      NOD() Count Matches(const CT::Block auto&) const noexcept;
      template<CT::Block>
      NOD() Count MatchesLoose(const CT::Block auto&) const noexcept;

   protected:
      template<CT::Block>
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
      template<bool SETSIZE = false, CT::Block = Any>
      void Reserve(Count);

   protected:
      /// @cond show_protected                                                
      template<CT::Block>
      NOD() AllocationRequest RequestSize(Count) const IF_UNSAFE(noexcept);

      template<CT::Block, bool CREATE = false, bool SETSIZE = false>
      void AllocateMore(Count);
      template<CT::Block>
      void AllocateLess(Count);

      template<CT::Block>
      void TakeAuthority();
      template<CT::Block, bool CREATE = false>
      void AllocateInner(Count);
      template<CT::Block>
      void AllocateFresh(const AllocationRequest&);

      void Reference(Count) const noexcept;
      void Keep() const noexcept;
      template<CT::Block = Any>
      void Free();

      void SetMemory(const DataState&, DMeta, Count) IF_UNSAFE(noexcept);
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
      template<CT::Block = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T1, class...TAIL>
      Count Insert(CT::Index auto, T1&&, TAIL&&...);

      template<CT::Block = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T>
      requires CT::Block<Desem<T>>
      Count InsertBlock(CT::Index auto, T&&);

      template<CT::Block = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T1, class...TAIL>
      Count Merge(CT::Index auto, T1&&, TAIL&&...);

      template<CT::Block = Any, class FORCE = Any, bool MOVE_ASIDE = true, class T>
      requires CT::Block<Desem<T>>
      Count MergeBlock(CT::Index auto, T&&);
   
      template<CT::Block = Any, bool MOVE_ASIDE = true, class...A>
      Count Emplace(CT::Index auto, A&&...);

      template<CT::Block = Any, class...A>
      Count New(Count, A&&...);

      template<CT::Block>
      Count New(Count);

      template<bool CONCAT = true, class FORCE = Any, CT::Block THIS = Any>
      Count SmartPush(CT::Index auto, auto&&, DataState = {});

      template<CT::Deep T, bool TRANSFER_OR = true, CT::Block THIS>
      requires CT::CanBeDeepened<T, THIS>
      T& Deepen();

      template<CT::Block>
      void Null(Count);

      template<CT::Block THIS>
      NOD() THIS Extend(Count);

   protected:
      template<CT::Block, class FORCE, bool MOVE_ASIDE>
      void InsertInner(CT::Index auto, auto&&);

      template<CT::Block, class FORCE, bool MOVE_ASIDE, class T = void, template<class> class S>
      requires CT::Semantic<S<Block>>
      void InsertBlockInner(CT::Index auto, S<Block>&&);

      template<CT::Block, class FORCE, bool MOVE_ASIDE>
      Count UnfoldInsert(CT::Index auto, auto&&);
      template<CT::Block, class FORCE, bool MOVE_ASIDE>
      Count UnfoldMerge(CT::Index auto, auto&&);

      template<CT::Block, class FORCE, template<class> class S, CT::Deep T>
      requires CT::Semantic<S<T>>
      Count SmartConcat(const CT::Index auto, bool, S<T>&&, DataState);

      template<CT::Block, class FORCE, template<class> class S, class T>
      requires CT::Semantic<S<T>>
      Count SmartPushInner(const CT::Index auto, S<T>&&, DataState);

      template<CT::Block THIS, template<class> class S, CT::Block T>
      requires CT::Semantic<S<T>>
      THIS ConcatBlock(S<T>&&) const;

      template<CT::Block>
      void CreateDefault() const;

      template<CT::Block, class...A>
      void CreateDescribe(A&&...) const;

      template<CT::Block, class...A>
      void Create(A&&...) const;

      template<CT::Block = Any, bool REVERSE = false, template<class> class S, CT::Block T>
      requires CT::Semantic<S<T>>
      void CreateSemantic(S<T>&&) const;

      template<template<class> class S, CT::Block T>
      requires CT::Semantic<S<T>>
      void ShallowBatchPointerConstruction(S<T>&&) const;

   public:
      template<CT::Block = Any, template<class> class S, CT::Block T>
      requires CT::Semantic<S<T>>
      void AssignSemantic(S<T>&&) const;

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false, CT::Block = Any>
      Count Remove(const CT::NotSemantic auto&);
      template<CT::Block = Any>
      Count RemoveIndex(CT::Index auto, Count = 1);
      template<CT::Block = Any>
      Count RemoveIndexDeep(CT::Index auto);
      template<CT::Block THIS = Any>
      Iterator<THIS> RemoveIt(const Iterator<THIS>&, Count = 1);

      template<CT::Block = Any>
      void Trim(Count);
      template<CT::Block = Any>
      void Optimize();
      template<CT::Block = Any>
      void Clear();
      template<CT::Block = Any>
      void Reset();

   protected:
      template<CT::Block = Any>
      void Destroy() const;

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
      Size Encrypt(Block&, const ::std::size_t*, Count) const;
      Size Decrypt(Block&, const ::std::size_t*, Count) const;

      ///                                                                     
      ///   Serialization                                                     
      ///                                                                     
      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      template<bool ENSCOPE = true, CT::Block TO, CT::Block TO_ORIGINAL = TO, CT::Block THIS>
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
      /// Use like this: for(auto i : Reverse(container)), where              
      /// 'container' can be any CT::IteratableInReverse type                 
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
      /// type - counteract this, and make 'i' be the iterator type instead   
      /// Use like this: for(auto i : KeepIterator(container)), where         
      /// 'container' can be any CT::Iteratable type                          
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
   

   ///                                                                        
   ///   Contiguous block iterator                                            
   ///                                                                        
   template<class BLOCK>
   struct Block::Iterator : A::Iterator {
      static_assert(CT::Block<BLOCK>, "BLOCK must be a CT::Block type");
      static constexpr bool Mutable = CT::Mutable<BLOCK>;

      using Type = Conditional<CT::Typed<BLOCK>
         , Conditional<Mutable, TypeOf<BLOCK>, const TypeOf<BLOCK>>
         , void>;

      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED) Type;

   protected:
      friend struct Block;
      using TypeInner = Conditional<CT::Typed<BLOCK>, Type*, Block>;

      // Current iterator position pointer                              
      TypeInner mValue;
      // Iterator position which is considered the 'end' iterator       
      Byte const* mEnd;

      constexpr Iterator(TypeInner, Byte const*) noexcept;

   public:
      Iterator() noexcept = delete;
      constexpr Iterator(const Iterator&) noexcept = default;
      constexpr Iterator(Iterator&&) noexcept = default;
      constexpr Iterator(const A::IteratorEnd&) noexcept;

      constexpr Iterator& operator = (const Iterator&) noexcept = default;
      constexpr Iterator& operator = (Iterator&&) noexcept = default;

      NOD() constexpr bool operator == (const Iterator&) const noexcept;
      NOD() constexpr bool operator == (const A::IteratorEnd&) const noexcept;

      NOD() constexpr decltype(auto) operator *  () const noexcept;
      NOD() constexpr decltype(auto) operator -> () const noexcept;

      // Prefix operator                                                
      constexpr Iterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() constexpr Iterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
      constexpr operator Iterator<const BLOCK>() const noexcept requires Mutable;
   };

} // namespace Langulus::Anyness