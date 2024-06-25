///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../DataState.hpp"
#include "../Compare.hpp"
#include "../Index.hpp"
#include "../Iterator.hpp"
#include "../one/Handle.hpp"
#include "../one/Own.hpp"


namespace Langulus
{
   namespace A
   {

      ///                                                                     
      /// An abstract Block structure                                         
      /// It defines the size for CT::Block and CT::Deep concepts             
      ///                                                                     
      struct Block {
         LANGULUS(ABSTRACT) true;
         LANGULUS(DEEP) true;
         LANGULUS(POD) true;
         static constexpr bool CTTI_Container = true;

      protected:
         using DMeta = Anyness::DMeta;
         using CMeta = Anyness::CMeta;
         using Allocation = Anyness::Allocation;
         using Allocator  = Anyness::Allocator;

         union {
         #if LANGULUS(DEBUG)
            char* mRawChar;
         #endif
            // Raw pointer to first element inside the memory block     
            Byte*  mRaw {};
            Byte** mRawSparse;
         };
   
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
         // The data state                                              
         DataState mState {DataState::Default};

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

      template<class...T>
      concept Iteratable = requires (T...a) {
         {(a.begin(), ...)} -> Data;
         {(a.end(),   ...)} -> Data;
      };

      template<class...T>
      concept IteratableInReverse = requires (T...a) {
         {(a.rbegin(), ...)} -> Data;
         {(a.rend(),   ...)} -> Data;
      };

      /// Any origin type that inherits A::Block                              
      template<class...T>
      concept BlockBased = (DerivedFrom<T, A::Block> and ...);

      /// A reflected block type is any type that is BlockBased, and is       
      /// binary compatible to a Block - this is a mandatory requirement for  
      /// any CT::Deep type                                                   
      /// Keep in mind, that sparse types are never considered Block!         
      template<class...T>
      concept Block = BlockBased<T...>
          and ((sizeof(T) == sizeof(A::Block)) and ...);

      template<class...T>
      concept NotBlock = ((not Block<T>) and ...);

      template<class...T>
      concept TypedBlock = Block<T...> and Typed<T...>;

      template<class...T>
      concept UntypedBlock = Block<T...> and ((not Typed<T>) and ...);

      /// A deep type is any type with a true static member T::CTTI_Deep,     
      /// is binary compatible with Block, and having the same interface      
      /// If no such member/base exists, the type is assumed NOT deep by      
      /// default. Deep types are considered iteratable, and verbs are        
      /// executed in each of their elements/members, instead on the type     
      /// itself. Use LANGULUS(DEEP) macro as member to tag deep types        
      /// Keep in mind, that sparse types are never considered Deep!          
      template<class...T>
      concept Deep = ((Block<T> and Decay<T>::CTTI_Deep) and ...);

      /// Check if Ts can be deepened 'WITH' the provided type                
      template<class WITH, class...T>
      concept CanBeDeepened = Deep<T...> and not CT::Void<WITH>
          and ((not CT::Typed<T> or CT::Similar<WITH, TypeOf<T>>) and ...);

      /// Type that is not deep, see CT::Deep                                 
      template<class...T>
      concept Flat = ((not Deep<T>) and ...);

      /// Check if origin of T(s) are Neat(s)                                 
      template<class...T>
      concept Neat = ((Exact<Decay<T>, Anyness::Neat>) and ...);

      /// Check if origin of T(s) aren't Neat(s)                              
      template<class...T>
      concept Messy = ((not Neat<T>) and ...);

      /// Check if origin of T(s) are Construct(s)                            
      template<class...T>
      concept Construct = ((Exact<Decay<T>, Anyness::Construct>) and ...);

      /// Check if origin of T(s) aren't Construct(s)                         
      template<class...T>
      concept NotConstruct = ((not Construct<T>) and ...);

      /// A serializer is any Block type, that has an inner type called       
      /// SerializationRules, which holds settings on how data is assembled   
      template<class...T>
      concept Serial = Block<T...> and ((requires {
         typename T::SerializationRules; }) and ...);

      
      namespace Inner
      {

         /// Unfolds T, if it is a bounded array, or std::range, and returns  
         /// a nullptr pointer of the type contained inside. Nested for       
         /// ranges containing other ranges, or arrays containing ranges.     
         /// Removes semantics and handles, too.                              
         ///   @tparam T - type to unfold                                     
         ///   @tparam UNLESS - stop unfolding if the type is similar         
         ///      useful in cases when you actually want to insert a std::map 
         ///      for example, and not unfold it down to pairs                
         ///   @return a pointer of the most inner type                       
         template<class T, class UNLESS = void>
         consteval auto Unfold() {
            if constexpr (CT::Similar<T, UNLESS>)
               return (Deref<Desem<T>>*) nullptr;
            else if constexpr (CT::Sparse<Desem<T>>) {
               if constexpr (CT::Array<Desem<T>>)
                  return Unfold<Deext<Desem<T>>>();
               else
                  return (Deref<Desem<T>>*) nullptr;
            }
            else if constexpr (CT::Handle<Desem<T>>)
               return (TypeOf<Desem<T>>*) nullptr;
            else if constexpr (::std::ranges::range<Desem<T>>)
               return Unfold<TypeOf<Desem<T>>>();
            else
               return (Deref<Desem<T>>*) nullptr;
         }

      } // namespace Langulus::CT::Inner
      
      /// Nest-unfold any bounded array or std::range, and get most inner type
      ///   @tparam T - type to unfold                                        
      ///   @tparam UNLESS - stop unfolding if the type is similar            
      ///      useful in cases when you actually want to insert a std::map    
      ///      for example, and not unfold it down to pairs                   
      template<class T, class UNLESS = void>
      using Unfold = Deptr<decltype(Inner::Unfold<T, UNLESS>())>;

      /// Check if T is constructible with each of the provided arguments,    
      /// either directly, or by unfolding that argument                      
      template<class T, class...A>
      concept UnfoldMakableFrom = ((not Same<A, Describe> and (
               ::std::constructible_from<T, A>
            or ::std::constructible_from<T, Unfold<A>>
         )) and ...) or DescriptorMakable<T>;

      /// Check if T is assignable with the provided argument,                
      /// either directly, or by unfolding that argument                      
      template<class T, class A>
      concept UnfoldAssignableFrom = not Same<A, Describe> and (
              ::std::assignable_from<T&, A>
           or ::std::assignable_from<T&, Unfold<A>>);

      /// Check if T is insertable to containers, either directly, or while   
      /// wrapped in a semantic                                               
      template<class...TN>
      concept UnfoldInsertable = Insertable<Desem<TN>...>;

      namespace Inner
      {

         /// Test whether a TMany is constructible with the given arguments   
         ///   @tparam T - the contained type in TMany<T>                     
         ///   @tparam ...A - the arguments to test                           
         ///   @return true if TMany<T> is constructible using {A...}         
         template<class T, class...A>
         consteval bool DeepMakable() noexcept {
            using FA = FirstOf<A...>;
            using SA = SemanticOf<FA>;

            if constexpr (TypeErased<T>) {
               // Type-erased containers accept almost any type - they  
               // will report errors at runtime instead, if any         
               return UnfoldInsertable<A...>;
            }
            else if constexpr (sizeof...(A) == 1 and CT::Block<Desem<FA>>) {
               // If only one A provided, it HAS to be a CT::Block      
               if constexpr (SA::Shallow) {
                  // Generally, shallow semantics are always supported, 
                  // but copying will call element constructors, so we  
                  // have to check if the contained type supports it    
                  if constexpr (CT::Copied<SA>)
                     return ReferMakable<T>;
                  else
                     return true;
               }
               else {
                  // Cloning always calls element constructors, and     
                  // we have to check whether contained elements can    
                  // do it                                              
                  return SemanticMakable<Langulus::Cloned, T>;
               }
            }
            else return UnfoldMakableFrom<T, A...>;
         };

         /// Test whether a TMany is assignable with the given argument       
         ///   @tparam T - the contained type in TMany<T>                     
         ///   @tparam A - the argument to test                               
         ///   @return true if TMany<T> is assignable using = A               
         template<class T, class A>
         consteval bool DeepAssignable() noexcept {
            using SA = SemanticOf<A>;

            if constexpr (TypeErased<T>) {
               // Type-erased containers accept almost any type - they  
               // will report errors at runtime instead, if any         
               return UnfoldInsertable<A>;
            }
            else if constexpr (CT::Block<Desem<A>>) {
               if constexpr (SA::Shallow) {
                  // Generally, shallow semantics are always supported, 
                  // but copying will call element assigners, so we     
                  // have to check if the contained type supports it    
                  if constexpr (CT::Copied<SA>)
                     return ReferAssignable<T>;
                  else
                     return true;
               }
               else {
                  // Cloning always calls element assigners, and we     
                  // have to check whether contained elements can do it 
                  return SemanticAssignable<Langulus::Cloned, T>;
               }
            }
            else return UnfoldAssignableFrom<T, A>;
         };

      } // namespace Langulus::CT::Inner

      /// Concept for recognizing arguments, with which a statically typed    
      /// container can be constructed                                        
      template<class T, class...A>
      concept DeepMakable = Inner::DeepMakable<T, A...>();

      /// Concept for recognizing argument, with which a statically typed     
      /// container can be assigned                                           
      template<class T, class A>
      concept DeepAssignable = Inner::DeepAssignable<T, A>();

   } // namespace Langulus::CT

} // namespace Langulus


namespace Langulus::Anyness
{

   template<class>
   struct TBlockIterator;

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
   template<class TYPE>
   struct Block : A::Block {
      LANGULUS(TYPED)    TYPE;
      LANGULUS(ABSTRACT) false;
      LANGULUS_BASES(A::Block);

      static constexpr bool Ownership  = false;
      static constexpr bool Sequential = true;
      static constexpr bool TypeErased = CT::TypeErased<TYPE>;
      static constexpr bool Sparse     = CT::Sparse<TYPE>;
      static constexpr bool Dense      = not Sparse;

      static_assert(TypeErased or CT::Insertable<TYPE>,
         "Contained type must be insertable");
      static_assert(TypeErased or CT::Allocatable<TYPE>,
         "Contained type must be allocatable");
      static_assert(not CT::Reference<TYPE>,
         "Contained type can't be a reference");

      template<class>
      friend struct Block;

      template<class>
      friend struct TBlockIterator;

      friend class Many;
      template<CT::Data>
      friend class TMany;

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

      friend struct Bytes;
      friend struct Text;
      friend struct Path;

      template<CT::Data>
      friend class Own;
      template<class>
      friend class Ref;

      friend class Neat;
      friend class Construct;

      friend struct ::Langulus::Flow::Verb;
      template<class>
      friend struct ::Langulus::Flow::TVerb;
      template<class, bool>
      friend struct ::Langulus::Flow::ArithmeticVerb;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      using A::Block::Block;

      constexpr Block(const A::Block&) noexcept;
      constexpr Block& operator = (const A::Block&) noexcept;
               
   protected:
      template<class T1, class...TN> requires CT::DeepMakable<TYPE, T1, TN...>
      void BlockCreate(T1&&, TN&&...);

      template<class B> requires CT::Block<Desem<B>>
      void BlockTransfer(B&&);

      template<CT::Block THIS, class T1>
      THIS& BlockAssign(T1&&) requires CT::DeepAssignable<TYPE, T1>;

      void BranchOut();

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      constexpr void SetState(DataState) noexcept;
      constexpr void AddState(DataState) noexcept;
      constexpr void RemoveState(DataState) noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      NOD() bool Owns(const void*) const noexcept;
      NOD() constexpr auto GetAllocation() const noexcept -> const Allocation*;
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
      NOD() constexpr bool CanFitState(const CT::Block auto&) const noexcept;
      NOD() constexpr Size GetBytesize() const noexcept;
      NOD() constexpr Token GetToken() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr DataState GetState() const noexcept;
      NOD() constexpr DataState GetUnconstrainedState() const noexcept;
      NOD() constexpr bool IsMissingDeep() const;
      NOD() constexpr bool IsConcatable(const CT::Block auto&) const noexcept;

      template<CT::Data>
      NOD() constexpr bool IsInsertable() const noexcept;
      NOD() constexpr bool IsInsertable(DMeta) const noexcept;

      NOD() bool IsExecutable() const noexcept;
      NOD() bool IsExecutableDeep() const noexcept;

      constexpr void MakeStatic(bool enable = true) noexcept;
      constexpr void MakeConst(bool enable = true) noexcept;
      constexpr void MakeTypeConstrained(bool enable = true) noexcept;
      constexpr void MakeOr() noexcept;
      constexpr void MakeAnd() noexcept;
      constexpr void MakePast() noexcept;
      constexpr void MakeFuture() noexcept;
      constexpr void MakeNow() noexcept;
      constexpr void ResetState() noexcept;
	  
      template<class T = TYPE>
      NOD() T*       GetRaw()          IF_UNSAFE(noexcept);
      template<class T = TYPE>
      NOD() T const* GetRaw()    const IF_UNSAFE(noexcept);
      template<class T = TYPE>
      NOD() T const* GetRawEnd() const IF_UNSAFE(noexcept);
      
      NOD() Allocation const* const* GetEntries() const IF_UNSAFE(noexcept);
      NOD() Allocation const**       GetEntries()       IF_UNSAFE(noexcept);

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() decltype(auto) operator[] (CT::Index auto);
      NOD() decltype(auto) operator[] (CT::Index auto) const;

      template<CT::Data>
      NOD() decltype(auto) As(CT::Index auto);
      template<CT::Data>
      NOD() decltype(auto) As(CT::Index auto) const;

      template<CT::Data T>
      NOD() LANGULUS(ALWAYS_INLINED) decltype(auto) As() {
         return As<T>(0);
      }

      template<CT::Data T>
      NOD() LANGULUS(ALWAYS_INLINED) decltype(auto) As() const {
         return As<T>(0);
      }

      decltype(auto) Last();
      decltype(auto) Last() const;

      // Intentionally undefined, because it requires Langulus::Flow    
      // and relies on Verbs::Interpret                                 
      // If you receive missing externals, include the following:       
      //    #include <Flow/Verbs/Interpret.hpp>                         
      template<CT::Data T, bool FATAL_FAILURE = true>
      NOD() T AsCast(CT::Index auto) const;

      template<CT::Data T, bool FATAL_FAILURE = true>
      NOD() LANGULUS(INLINED) T AsCast() const {
         return AsCast<T, FATAL_FAILURE>(0);
      }
   
      template<CT::Block THIS> NOD() IF_UNSAFE(constexpr)
      THIS Select(Offset, Count) IF_UNSAFE(noexcept);
      template<CT::Block THIS> NOD() IF_UNSAFE(constexpr)
      THIS Select(Offset, Count) const IF_UNSAFE(noexcept);

      template<Count = CountMax>
      NOD() Block<> GetElementDense(Offset = 0);
      template<Count = CountMax>
      NOD() Block<> GetElementDense(Offset = 0) const;
   
      NOD() Block<> GetElementResolved(Offset = 0);
      NOD() Block<> GetElementResolved(Offset = 0) const;
   
      NOD() Block<> GetElement(Offset = 0)       IF_UNSAFE(noexcept);
      NOD() Block<> GetElement(Offset = 0) const IF_UNSAFE(noexcept);
   
      NOD() Block<>*       GetBlockDeep(Offset) noexcept;
      NOD() Block<> const* GetBlockDeep(Offset) const noexcept;
   
      NOD() Block<> GetElementDeep(Offset) noexcept;
      NOD() Block<> GetElementDeep(Offset) const noexcept;

      NOD() Block<> GetResolved();
      NOD() Block<> GetResolved() const;

      template<Count = CountMax>
      NOD() Block<> GetDense();
      template<Count = CountMax>
      NOD() Block<> GetDense() const;

      NOD() Block<> operator * ();
      NOD() Block<> operator * () const;

      void SwapIndices(CT::Index auto, CT::Index auto);
      template<class T> requires CT::Block<Desem<T>>
      void Swap(T&&);

      template<bool REVERSE = false>
      Count GatherFrom(const CT::Block auto&);
      template<bool REVERSE = false>
      Count GatherFrom(const CT::Block auto&, DataState);

      template<Index>
      NOD() Index GetIndex() const IF_UNSAFE(noexcept);
      NOD() Index GetIndexMode(Count&) const IF_UNSAFE(noexcept);
	  
      template<CT::Data = TYPE> NOD() IF_UNSAFE(constexpr)
      decltype(auto) Get(Offset = 0)       IF_UNSAFE(noexcept);
      template<CT::Data = TYPE> NOD() IF_UNSAFE(constexpr)
      decltype(auto) Get(Offset = 0) const IF_UNSAFE(noexcept);
   
      NOD() IF_UNSAFE(constexpr)
      decltype(auto) GetDeep(Offset = 0)       IF_UNSAFE(noexcept);
      NOD() IF_UNSAFE(constexpr)
      decltype(auto) GetDeep(Offset = 0) const IF_UNSAFE(noexcept);

   protected: 
      NOD() Block<> GetElementInner(Offset = 0)       IF_UNSAFE(noexcept);
      NOD() Block<> GetElementInner(Offset = 0) const IF_UNSAFE(noexcept);

      NOD() IF_UNSAFE(constexpr)
      Byte* At(Offset = 0) IF_UNSAFE(noexcept);
      NOD() IF_UNSAFE(constexpr)
      Byte const* At(Offset = 0) const IF_UNSAFE(noexcept);
   
      NOD() Index Constrain(Index) const IF_UNSAFE(noexcept);
      NOD() Block CropInner(Offset, Count) const IF_UNSAFE(noexcept);

      template<bool SAFE = true, CT::Index INDEX>
      Offset SimplifyIndex(INDEX) const
      noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>);

   public:
      template<class = TYPE> NOD()
      auto GetHandle(Offset = 0)       IF_UNSAFE(noexcept);
      template<class = TYPE> NOD()
      auto GetHandle(Offset = 0) const IF_UNSAFE(noexcept);
   
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator      = TBlockIterator<Block>;
      using ConstIterator = TBlockIterator<const Block>;

      NOD() constexpr Iterator      begin() noexcept;
      NOD() constexpr ConstIterator begin() const noexcept;
      NOD() constexpr Iterator      last() noexcept;
      NOD() constexpr ConstIterator last() const noexcept;

      constexpr A::IteratorEnd end() const noexcept { return {}; }

      template<bool REVERSE = false, bool MUTABLE = false>
      Count ForEachElement(auto&&) const;
      template<bool REVERSE = false>
      Count ForEachElement(auto&&);

      template<bool MUTABLE = false>
      Count ForEachElementRev(auto&&...) const;
      Count ForEachElementRev(auto&&...);

      template<bool REVERSE = false, bool MUTABLE = false>
      Count ForEach(auto&&...) const;
      template<bool REVERSE = false>
      Count ForEach(auto&&...);

      template<bool MUTABLE = false>
      Count ForEachRev(auto&&...) const;
      Count ForEachRev(auto&&...);

      template<bool REVERSE = false, bool SKIP = true, bool MUTABLE = false>
      Count ForEachDeep(auto&&...) const;
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...);

      template<bool SKIP = true, bool MUTABLE = false>
      Count ForEachDeepRev(auto&&...) const;
      template<bool SKIP = true>
      Count ForEachDeepRev(auto&&...);

   protected:
      template<class F>
      static constexpr bool NoexceptIterator = not LANGULUS_SAFE()
         and noexcept(Fake<F&&>().operator() (Fake<ArgumentOf<F>>()));

      template<bool MUTABLE, bool REVERSE>
      LoopControl ForEachInner(auto&& f, Count&) const noexcept(NoexceptIterator<decltype(f)>);

      template<bool MUTABLE, bool REVERSE, bool SKIP>
      LoopControl ForEachDeepInner(auto&&, Count&) const;

      template<bool MUTABLE, bool REVERSE>
      LoopControl IterateInner(Count, auto&& f) const noexcept(NoexceptIterator<decltype(f)>);

      // Prefix operators                                               
      Block&       operator ++ ()       IF_UNSAFE(noexcept);
      Block const& operator ++ () const IF_UNSAFE(noexcept);
      Block&       operator -- ()       IF_UNSAFE(noexcept);
      Block const& operator -- () const IF_UNSAFE(noexcept);

      // Suffix operators                                               
      NOD() Block  operator ++ (int) const IF_UNSAFE(noexcept);
      NOD() Block  operator -- (int) const IF_UNSAFE(noexcept);
                   
      NOD() Block  operator +  (Offset) const IF_UNSAFE(noexcept);
      NOD() Block  operator -  (Offset) const IF_UNSAFE(noexcept);

      Block&       operator += (Offset)       IF_UNSAFE(noexcept);
      Block const& operator += (Offset) const IF_UNSAFE(noexcept);
      Block&       operator -= (Offset)       IF_UNSAFE(noexcept);
      Block const& operator -= (Offset) const IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      NOD() bool Is(DMeta) const noexcept;
      NOD() bool Is(const CT::Block auto&) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      NOD() bool IsSimilar(DMeta) const noexcept;
      NOD() bool IsSimilar(const CT::Block auto&) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      NOD() bool IsExact(DMeta) const noexcept;
      NOD() bool IsExact(const CT::Block auto&) const noexcept;

      template<bool BINARY_COMPATIBLE = false, bool ADVANCED = false>
      NOD() bool CastsToMeta(DMeta) const;
      template<bool BINARY_COMPATIBLE = false>
      NOD() bool CastsToMeta(DMeta, Count) const;

      template<CT::Data, bool BINARY_COMPATIBLE = false, bool ADVANCED = false>
      NOD() bool CastsTo() const;
      template<CT::Data, bool BINARY_COMPATIBLE = false>
      NOD() bool CastsTo(Count) const;

      template<CT::Block B>
      NOD() B ReinterpretAs(const B&) const;
      template<CT::Data T>
      NOD() TMany<T> ReinterpretAs() const;

      NOD() Block<> GetMember(const RTTI::Member&, CT::Index auto);
      NOD() Block<> GetMember(const RTTI::Member&, CT::Index auto) const;
   
      template<bool CONSTRAIN = false>
      void SetType(DMeta) requires TypeErased;
      template<CT::Data, bool CONSTRAIN = false>
      void SetType() requires TypeErased;

   protected:
      template<CT::Data, class FORCE = Many>
      bool Mutate();
      template<class FORCE = Many>
      bool Mutate(DMeta);

      constexpr void ResetType() noexcept;

   public:
      NOD() Block<> GetBaseMemory(DMeta, const RTTI::Base&);
      NOD() Block<> GetBaseMemory(DMeta, const RTTI::Base&) const;
      NOD() Block<> GetBaseMemory(const RTTI::Base&);
      NOD() Block<> GetBaseMemory(const RTTI::Base&) const;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const CT::Block auto&) const;

      template<CT::NotBlock T1>
      bool operator == (const T1&) const
      requires (TypeErased or CT::Comparable<TYPE, T1>);

      template<bool RESOLVE = true>
      NOD() bool Compare(const CT::Block auto&) const;
      NOD() Hash GetHash() const requires (TypeErased or CT::Hashable<TYPE>);

      template<bool REVERSE = false, CT::NotSemantic T1>
      NOD() Index Find(const T1&, Offset = 0) const noexcept
      requires (TypeErased or CT::Comparable<TYPE, T1>);

      NOD() Iterator      FindIt(const CT::NotSemantic auto&);
      NOD() ConstIterator FindIt(const CT::NotSemantic auto&) const;

      template<bool REVERSE = false>
      NOD() Index FindBlock(const CT::Block auto&, CT::Index auto) const noexcept;

      template<bool ASCEND = false>
      void Sort() requires (TypeErased or CT::Sortable<TYPE, TYPE>);

      NOD() bool  CompareLoose(const CT::Block auto&) const noexcept;
      NOD() Count Matches(const CT::Block auto&) const noexcept;
      NOD() Count MatchesLoose(const CT::Block auto&) const noexcept;

   protected:
      NOD() bool CompareSingleValue(const CT::NotSemantic auto&) const;
      NOD() bool CompareStates(const Block&) const noexcept;
      NOD() bool CompareTypes(const CT::Block auto&, RTTI::Base&) const;
      NOD() bool CallComparer(const Block&, const RTTI::Base&) const;

      template<bool REVERSE = false>
      Count GatherInner(CT::Block auto&) const;
      template<bool REVERSE = false>
      Count GatherPolarInner(DMeta, CT::Block auto&, DataState) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<bool SETSIZE = false>
      void Reserve(Count);
      void TakeAuthority();

   protected:
      /// @cond show_protected                                                
      NOD() AllocationRequest RequestSize(Count) const IF_UNSAFE(noexcept);

      template<bool CREATE = false, bool SETSIZE = false>
      void AllocateMore(Count);
      void AllocateLess(Count);

      template<bool CREATE = false>
      void AllocateInner(Count);
      void AllocateFresh(const AllocationRequest&);

      void Keep() const noexcept;
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class FORCE = Many, bool MOVE_ASIDE = true, class T1, class...TN>
      Count Insert(CT::Index auto, T1&&, TN&&...)
      requires (TypeErased or CT::UnfoldMakableFrom<TYPE, T1, TN...>);

      template<class FORCE = Many, bool MOVE_ASIDE = true, class T>
      requires CT::Block<Desem<T>>
      Count InsertBlock(CT::Index auto, T&&);

      template<class FORCE = Many, bool MOVE_ASIDE = true, class T1, class...TN>
      Count Merge(CT::Index auto, T1&&, TN&&...)
      requires (TypeErased or CT::UnfoldMakableFrom<TYPE, T1, TN...>);

      template<class FORCE = Many, bool MOVE_ASIDE = true, class T>
      requires CT::Block<Desem<T>>
      Count MergeBlock(CT::Index auto, T&&);
   
      template<bool MOVE_ASIDE = true, class...A>
      decltype(auto) Emplace(CT::Index auto, A&&...)
      requires (TypeErased or ::std::constructible_from<TYPE, A...>);

      template<class...A>
      Count New(Count, A&&...)
      requires (TypeErased or ::std::constructible_from<TYPE, A...>);

      Count New(Count = 1) requires (TypeErased or CT::Defaultable<TYPE>);

      template<bool CONCAT = true, class FORCE = Many>
      Count SmartPush(CT::Index auto, auto&&, DataState = {});

      template<CT::Deep T, bool TRANSFER_OR = true>
      T& Deepen();

      void Null(Count);

      template<class A>
      void Fill(A&&) requires (TypeErased or CT::AssignableFrom<TYPE, A>);

      template<CT::Block THIS>
      NOD() THIS Extend(Count);

   protected:
      template<class FORCE, bool MOVE_ASIDE>
      void InsertInner(CT::Index auto, auto&&);

      template<class FORCE, bool MOVE_ASIDE, class T> requires CT::Block<Desem<T>>
      void InsertBlockInner(CT::Index auto, T&&);

      template<class FORCE, bool MOVE_ASIDE>
      Count UnfoldInsert(CT::Index auto, auto&&);
      template<class FORCE, bool MOVE_ASIDE>
      Count UnfoldMerge(CT::Index auto, auto&&);

      template<class FORCE, class T> requires CT::Deep<Desem<T>>
      Count SmartConcat(const CT::Index auto, bool, T&&, DataState);

      template<class FORCE>
      Count SmartPushInner(const CT::Index auto, auto&&, DataState);

      template<CT::Block THIS, class T> requires CT::Block<Desem<T>>
      THIS ConcatBlock(T&&) const;

      void CreateDefault();

      template<class...A>
      void CreateDescribe(A&&...);

      template<class...A>
      void Create(A&&...);

      template<bool REVERSE = false, class T> requires CT::Block<Desem<T>>
      void CreateSemantic(T&&);

      template<class T> requires CT::Handle<Desem<T>>
      void CreateSemantic(T&&);

      template<class T> requires CT::Block<Desem<T>>
      void ShallowBatchPointerConstruction(T&&);

   public:
      template<class T>
      void AssignSemantic(T&&) requires CT::Block<Desem<T>>;

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false>
      Count Remove(const CT::NotSemantic auto&);
      Count RemoveIndex(CT::Index auto, Count = 1);
      Count RemoveIndexDeep(CT::Index auto);
      Iterator RemoveIt(const Iterator&, Count = 1);

      void Trim(Count);
      void Optimize();
      void Clear();
      void Reset();

   protected:
      template<bool FORCE = true, class MASK = std::nullptr_t>
      void Destroy(MASK = {}) const;
      template<class MASK>
      void DestroySparse(MASK) const;

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
      ///   Conversion                                                        
      ///                                                                     
      Count Convert(CT::Block auto&) const;
      Count Serialize(CT::Serial auto&) const;

   protected:
      #pragma pack(push, 1)
      struct Header {
         enum { Default, BigEndian };

         ::std::uint8_t  mAtomSize = sizeof(Offset);
         ::std::uint8_t  mFlags    = BigEndianMachine ? BigEndian : Default;
         ::std::uint16_t mVersion  = 0;
         ::std::uint32_t mDefinitionCount = 0;
      };
      #pragma pack(pop)

      using Loader = void(*)(Block&, Count);

      template<class>
      Count SerializeToText(CT::Serial auto&) const;
      template<class>
      Count SerializeToBinary(CT::Serial auto&) const;
      template<class, class...RULES>
      Count SerializeByRules(CT::Serial auto&, Types<RULES...>) const;
      template<class, class RULE>
      Count SerializeApplyRule(CT::Serial auto&) const;

      template<class>
      Offset DeserializeBinary(CT::Block auto&, const Header&, Offset = 0, Loader = nullptr) const;
      void ReadInner(Offset, Count, Loader) const;
      NOD() Offset DeserializeAtom(Offset&, Offset, const Header&, Loader) const;
      NOD() Offset DeserializeMeta(CT::Meta auto&, Offset, const Header&, Loader) const;
   };

   template<class BLOCK = void>
   NOD() auto MakeBlock(auto&&, Count = 1);

   template<class BLOCK = void, CT::Data...TN>
   NOD() auto WrapBlock(TN&&...);

   /// Cast between block types - does only reinterpret_cast, with some       
   /// additional safety checks. Preserves qualifiers.                        
   ///   @tparam AS - what block are we casting to?                           
   ///   @param from - block we're casting from                               
   ///   @return the reinterpreted block                                      
   template<CT::Block AS>
   NOD() decltype(auto) BlockCast(CT::Block auto&& from) {
      //TODO move all kinds of checks here instead?
      //TODO utilize cast operators to type-erased references here if available - they might set type
      using DAS  = Decay<AS>;
      using FROM = Deref<decltype(from)>;
      if constexpr (CT::Mutable<FROM>)
         return reinterpret_cast<DAS&>(from);
      else
         return reinterpret_cast<const DAS&>(from);
   }

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
   template<class B>
   struct TBlockIterator : A::Iterator {
      static_assert(CT::Block<B>, "B must be a Block type");
      static constexpr bool Mutable = CT::Mutable<B>;
      using Type = Conditional<Mutable, TypeOf<B>, const TypeOf<B>>;

      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED)    Type;

   protected:
      template<class>
      friend struct Block;
      template<class>
      friend struct TBlockIterator;

      using TypeInner = Conditional<B::TypeErased, B, Type*>;

      // Current iterator position pointer                              
      TypeInner   mValue;
      // Iterator position which is considered the 'end' iterator       
      Type const* mEnd;

      constexpr TBlockIterator(const TypeInner&, Type const*) noexcept;

   public:
      TBlockIterator() noexcept = delete;

      constexpr TBlockIterator(const TBlockIterator&) noexcept = default;
      constexpr TBlockIterator(TBlockIterator&&) noexcept = default;
      constexpr TBlockIterator(A::IteratorEnd) noexcept;

      constexpr TBlockIterator& operator = (const TBlockIterator&) noexcept = default;
      constexpr TBlockIterator& operator = (TBlockIterator&&) noexcept = default;

      NOD() constexpr bool operator == (const TBlockIterator&) const noexcept;
      NOD() constexpr bool operator == (A::IteratorEnd) const noexcept;

      NOD() constexpr decltype(auto) operator *  () const noexcept;
      NOD() constexpr decltype(auto) operator -> () const noexcept;

      // Prefix operator                                                
      constexpr TBlockIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() constexpr TBlockIterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;

      // Implicit cast to a constant iterator                           
      constexpr operator TBlockIterator<const B>() const noexcept
      requires Mutable { return {mValue, mEnd}; }
   };

} // namespace Langulus::Anyness