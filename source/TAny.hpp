///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Any.hpp"


namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   TAny                                                                 
   ///                                                                        
   ///   Unlike Any, this one is statically optimized to perform faster, due  
   /// to not being type-erased. In that sense, this container is equivalent  
   /// to std::vector.                                                        
   ///   Don't forget that all Any containers are binary-compatible with each 
   /// other, so after you've asserted, that an Any is of a specific type,    
   /// (by checking result of doing something like pack.IsExact<my type>())   
   /// you can then directly reinterpret_cast that Any to an equivalent       
   /// TAny<of the type you checked for>, essentially converting your         
   /// type-erased container to a statically-optimized equivalent.            
   ///                                                                        
   template<CT::Data T>
   class TAny : public Any {
      LANGULUS(DEEP) true;
      LANGULUS(POD) false;
      LANGULUS(TYPED) T;
      LANGULUS_BASES(Any);

   public:
      static constexpr bool Ownership = true;

      template<CT::Data, CT::Data>
      friend class TUnorderedMap;
      friend class Any;
      friend class Block;

      template<bool MUTABLE>
      struct TIterator;
      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

      template<bool MUTABLE>
      struct TIteratorEnd;
      using IteratorEnd = TIteratorEnd<true>;
      using ConstIteratorEnd = TIteratorEnd<false>;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr TAny();
      
      TAny(const TAny&);
      TAny(TAny&&) noexcept;

      TAny(const CT::NotSemantic auto&);
      TAny(CT::NotSemantic auto&);
      TAny(CT::NotSemantic auto&&);
      TAny(CT::ShallowSemantic auto&&);
      TAny(CT::DeepSemantic auto&&) requires CT::CloneMakable<T>;

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      TAny(T1&&, T2&&, TAIL&&...);

      ~TAny();

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      TAny& operator = (const TAny&);
      TAny& operator = (TAny&&);

      TAny& operator = (const CT::NotSemantic auto&);
      TAny& operator = (CT::NotSemantic auto&);
      TAny& operator = (CT::NotSemantic auto&&);
      TAny& operator = (CT::ShallowSemantic auto&&);
      TAny& operator = (CT::DeepSemantic auto&&) requires CT::CloneAssignable<T>;

   private:
      void ConstructFrom(CT::Semantic auto&&);
      TAny& AssignFrom(CT::Semantic auto&&);

   public:
      NOD() static TAny From(const T*, const Count& = 1);
      NOD() static TAny From(CT::Semantic auto&&, const Count& = 1);

      template<CT::Data... LIST_T>
      NOD() static TAny Wrap(LIST_T&&...);
   
      void Null(const Count&);

      NOD() DMeta GetType() const noexcept;
      NOD() const T* GetRaw() const noexcept;
      NOD() T* GetRaw() noexcept;
      NOD() const T* GetRawEnd() const noexcept;
      NOD() decltype(auto) GetHandle(Offset) IF_UNSAFE(noexcept);
      NOD() decltype(auto) GetHandle(Offset) const IF_UNSAFE(noexcept);


   private: IF_LANGULUS_TESTING(public:)
      using Any::GetRawSparse;

   public:
      NOD() const T& Last() const;
      NOD()       T& Last();

      template<CT::Data = T>
      NOD() decltype(auto) Get(const Offset&) const noexcept;
      template<CT::Data = T>
      NOD() decltype(auto) Get(const Offset&) noexcept;

      NOD() const T& operator [] (const CT::Index auto&) const;
      NOD()       T& operator [] (const CT::Index auto&);

      NOD() constexpr bool IsTyped() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr bool IsPOD() const noexcept;
      NOD() constexpr bool IsResolvable() const noexcept;
      NOD() constexpr bool IsNullifiable() const noexcept;

      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr Size GetBytesize() const noexcept;

      NOD() bool CastsToMeta(DMeta) const;
      NOD() bool CastsToMeta(DMeta, Count) const;

      template<CT::Data>
      NOD() bool CastsTo() const;
      template<CT::Data>
      NOD() bool CastsTo(Count) const;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      NOD() bool IsExact(DMeta) const noexcept;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      NOD() AllocationRequest RequestSize(const Count&) const noexcept;
      void Reserve(Count);
      template<bool CREATE = false, bool SETSIZE = false>
      void AllocateMore(Count);
      void AllocateLess(Count);
      void TakeAuthority();

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::Index IDX = Offset>
      Count InsertAt(const T*, const T*, const IDX&);
      template<CT::Index IDX = Offset>
      Count InsertAt(const T&, const IDX&);
      template<CT::Index IDX = Offset>
      Count InsertAt(T&&, const IDX&);
      template<CT::Semantic S, CT::Index IDX = Offset>
      Count InsertAt(S&&, const IDX&) requires (CT::Exact<TypeOf<S>, T>);

      template<Index = IndexBack, bool MUTABLE = false>
      Count Insert(const T*, const T*);
      template<Index = IndexBack>
      Count Insert(const T&);
      template<Index = IndexBack>
      Count Insert(T&&);
      template<Index = IndexBack, CT::Semantic S>
      Count Insert(S&&) requires (CT::Exact<TypeOf<S>, T>);

      template<CT::Index IDX = Offset, class... A>
      Count EmplaceAt(const IDX&, A&&...);
      template<Index = IndexBack, class... A>
      Count Emplace(A&&...);
      Count New(Count = 1);
      template<class... A>
      Count New(Count, A&&...);

      TAny& operator << (const T&);
      TAny& operator << (T&&);
      template<CT::Semantic S>
      TAny& operator << (S&&) requires (CT::Exact<TypeOf<S>, T>);

      TAny& operator >> (const T&);
      TAny& operator >> (T&&);
      template<CT::Semantic S>
      TAny& operator >> (S&&) requires (CT::Exact<TypeOf<S>, T>);

      Count MergeAt(const T*, const T*, const CT::Index auto&);
      Count MergeAt(const T&, const CT::Index auto&);
      Count MergeAt(T&&, const CT::Index auto&);
      template<CT::Semantic S>
      Count MergeAt(S&&, const CT::Index auto&) requires (CT::Exact<TypeOf<S>, T>);

      template<Index = IndexBack>
      Count Merge(const T*, const T*);
      template<Index = IndexBack>
      Count Merge(const T&);
      template<Index = IndexBack>
      Count Merge(T&&);
      template<Index = IndexBack, CT::Semantic S>
      Count Merge(S&&) requires (CT::Exact<TypeOf<S>, T>);

      TAny& operator <<= (const T&);
      TAny& operator <<= (T&&);
      template<CT::Semantic S>
      TAny& operator <<= (S&&) requires (CT::Exact<TypeOf<S>, T>);

      TAny& operator >>= (const T&);
      TAny& operator >>= (T&&);
      template<CT::Semantic S>
      TAny& operator >>= (S&&) requires (CT::Exact<TypeOf<S>, T>);

   private:
      // Disable these inherited functions                              
      using Any::SmartPushAt;
      using Any::SmartPush;

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false>
      Count Remove(const CT::Data auto&);
      Count RemoveIndex(const CT::Index auto&, Count = 1);
      Iterator RemoveIt(const Iterator&, Count = 1);

      void Trim(const Count&);
      template<CT::Block WRAPPER = TAny>
      NOD() WRAPPER Crop(const Offset&, const Count&) const;
      template<CT::Block WRAPPER = TAny>
      NOD() WRAPPER Crop(const Offset&, const Count&);

      void Clear();
      void Reset();
      void Free();

      ///                                                                     
      ///   Search                                                            
      ///                                                                     
      template<bool REVERSE = false>
      NOD() Index Find(const CT::Data auto&, const Offset& = 0) const noexcept;

      template<CT::Data ALT_T = T>
      bool operator == (const TAny<ALT_T>&) const noexcept
      requires (CT::Inner::Comparable<T, ALT_T>);

      bool operator == (const Any&) const noexcept
      requires (CT::Inner::Comparable<T>);

      NOD() bool Compare(const TAny&) const noexcept;
      NOD() bool CompareLoose(const TAny&) const noexcept;
      NOD() Count Matches(const TAny&) const noexcept;
      NOD() Count MatchesLoose(const TAny&) const noexcept;
      NOD() Hash GetHash() const requires CT::Hashable<T>;

      template<bool ASCEND = false>
      void Sort();

      template<CT::Block WRAPPER = TAny>
      NOD() WRAPPER Extend(const Count&);

      void Swap(const Offset&, const Offset&);
      void Swap(const Index&, const Index&);

      template<bool REVERSE = false>
      Count GatherFrom(const Block&);
      template<bool REVERSE = false>
      Count GatherFrom(const Block&, DataState);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() TAny operator + (const CT::NotSemantic auto&) const;
      NOD() TAny operator + (CT::NotSemantic auto&) const;
      NOD() TAny operator + (CT::NotSemantic auto&&) const;
      template<template<class> class S, CT::NotSemantic ALT> requires CT::Semantic<S<ALT>>
      NOD() TAny operator + (S<ALT>&&) const;

      TAny& operator += (const CT::NotSemantic auto&);
      TAny& operator += (CT::NotSemantic auto&);
      TAny& operator += (CT::NotSemantic auto&&);
      template<template<class> class S, CT::NotSemantic ALT> requires CT::Semantic<S<ALT>>
      TAny& operator += (S<ALT>&&);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      NOD() Iterator begin() noexcept;
      NOD() IteratorEnd end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIteratorEnd end() const noexcept;
      NOD() ConstIterator last() const noexcept;

      ///                                                                     
      ///   Flow                                                              
      ///                                                                     
      // Intentionally undefined, because it requires Langulus::Flow    
      void Run(Flow::Verb&) const;
      // Intentionally undefined, because it requires Langulus::Flow    
      void Run(Flow::Verb&);

   protected:
      void AllocateFresh(const AllocationRequest&);

      constexpr void ResetState() noexcept;
      constexpr void ResetType() noexcept;

      template<bool OVERWRITE_STATE, bool OVERWRITE_ENTRY>
      void CopyProperties(const Block&) noexcept;

   private:
      using Any::FromMeta;
      using Any::FromBlock;
      using Any::FromState;
      using Any::From;
      using Any::WrapAs;
   };


   ///                                                                        
   ///   TAny iterator                                                        
   ///                                                                        
   template<CT::Data T>
   template<bool MUTABLE>
   struct TAny<T>::TIterator : A::Iterator {
      LANGULUS(UNINSERTABLE) true;
      LANGULUS(TYPED) T;

   protected:
      friend class TAny<T>;
      template<bool>
      friend struct TAny<T>::TIteratorEnd;

      const T* mElement;

      constexpr TIterator(const T*) noexcept;

   public:
      template<bool RHS_MUTABLE>
      NOD() constexpr bool operator == (const TIteratorEnd<RHS_MUTABLE>&) const noexcept;
      NOD() constexpr bool operator == (const TIterator&) const noexcept;

      operator T& () const noexcept requires (MUTABLE);
      operator const T& () const noexcept requires (not MUTABLE);

      NOD() T& operator * () const noexcept requires (MUTABLE);
      NOD() const T& operator * () const noexcept requires (not MUTABLE);

      NOD() T& operator -> () const noexcept requires (MUTABLE);
      NOD() const T& operator -> () const noexcept requires (not MUTABLE);

      // Prefix operator                                                
      constexpr TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() constexpr TIterator operator ++ (int) noexcept;
   };
   

   ///                                                                        
   ///   TAny iterator end marker                                             
   ///                                                                        
   template<CT::Data T>
   template<bool MUTABLE>
   struct TAny<T>::TIteratorEnd : A::Iterator {
      LANGULUS(UNINSERTABLE) true;
      LANGULUS(TYPED) T;

   protected:
      friend class TAny<T>;
      template<bool>
      friend struct TAny<T>::TIterator;

      const T* mEndMarker;

      constexpr TIteratorEnd(const T*) noexcept;

   public:
      template<bool RHS_MUTABLE>
      NOD() constexpr bool operator == (const TIterator<RHS_MUTABLE>&) const noexcept;
      NOD() constexpr bool operator == (const TIteratorEnd&) const noexcept;
   };

} // namespace Langulus::Anyness
