///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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
   /// (by checking result of doing something like pack.Is<my type>())        
   /// you can then directly reinterpret_cast that Any to an equivalent       
   /// TAny<of the type you checked for>, essentially converting your         
   /// type-erased container to a statically-optimized equivalent.            
   ///                                                                        
   template<CT::Data T>
   class TAny : public Any {
      LANGULUS(DEEP) true;
      LANGULUS_BASES(Any);

   public:
      static constexpr bool Ownership = true;

      static_assert(CT::Dense<T> || !CT::Same<T, KnownPointer>,
         "Can only insert dense KnownPointer(s)");
      static_assert(CT::Sparse<T> || CT::Insertable<T>,
         "Dense contained type is not insertable");

      friend struct ::Langulus::Flow::Serializer;
      template<CT::Data, CT::Data>
      friend class TUnorderedMap;
      friend class Any;
      friend class Block;

      /// Makes the TAny CT::Typed                                            
      using MemberType = T;

      /// Internal representation of a sparse element                         
      class KnownPointer;
      using TypeInner = Conditional<CT::Dense<T>, T, KnownPointer>;

      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

   public:
      constexpr TAny();
      
      TAny(const TAny&);
      TAny(TAny&&) noexcept;

      template<CT::Deep ALT_T>
      TAny(const ALT_T&);
      template<CT::Deep ALT_T>
      TAny(ALT_T&);
      template<CT::Deep ALT_T>
      TAny(ALT_T&&);
      template<CT::Semantic S>
      TAny(S&&) requires (CT::Deep<TypeOf<S>>);

      TAny(const T&) requires CT::CustomData<T>;
      TAny(T&&) requires CT::CustomData<T>;
      template<CT::Semantic S>
      TAny(S&&) requires (CT::CustomData<TypeOf<S>>);

      template<CT::Data HEAD, CT::Data... TAIL>
      TAny(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      ~TAny();

      TAny& operator = (const TAny&);
      TAny& operator = (TAny&&);

      template<CT::Deep ALT_T>
      TAny& operator = (const ALT_T&);
      template<CT::Deep ALT_T>
      TAny& operator = (ALT_T&);
      template<CT::Deep ALT_T>
      TAny& operator = (ALT_T&&);
      template<CT::Semantic S>
      TAny& operator = (S&&) requires (CT::Deep<TypeOf<S>>);

      TAny& operator = (const T&) requires CT::CustomData<T>;
      TAny& operator = (T&) requires CT::CustomData<T>;
      TAny& operator = (T&&) requires CT::CustomData<T>;
      template<CT::Semantic S>
      TAny& operator = (S&&) requires (CT::CustomData<TypeOf<S>>);

   public:
      template<CT::Semantic S>
      NOD() static TAny From(S&&, const Count&) requires (CT::Sparse<TypeOf<S>>);

      template<CT::Data... LIST_T>
      NOD() static TAny Wrap(LIST_T&&...);
   
      void Null(const Count&);
      void TakeAuthority();

      void Reserve(Count);
      NOD() TAny Clone() const;

      NOD() DMeta GetType() const noexcept;
      NOD() auto GetRaw() const noexcept;
      NOD() auto GetRaw() noexcept;
      NOD() auto GetRawEnd() const noexcept;
      NOD() auto GetRawEnd() noexcept;
      NOD() auto GetRawSparse() noexcept;
      NOD() auto GetRawSparse() const noexcept;

      NOD() decltype(auto) Last() const;
      NOD() decltype(auto) Last();

      template<CT::Data = T>
      NOD() decltype(auto) Get(const Offset&) const noexcept;
      template<CT::Data = T>
      NOD() decltype(auto) Get(const Offset&) noexcept;

      template<CT::Index IDX>
      NOD() decltype(auto) operator [] (const IDX&) const;
      template<CT::Index IDX>
      NOD() decltype(auto) operator [] (const IDX&);

      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsDefaultable() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr bool IsPOD() const noexcept;
      NOD() constexpr bool IsResolvable() const noexcept;
      NOD() constexpr bool IsNullifiable() const noexcept;

      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr Size GetByteSize() const noexcept;
      NOD() auto RequestSize(const Count&) const noexcept;

      NOD() bool CastsToMeta(DMeta) const;
      NOD() bool CastsToMeta(DMeta, Count) const;

      template<CT::Data>
      NOD() bool CastsTo() const;
      template<CT::Data>
      NOD() bool CastsTo(Count) const;

      NOD() bool Is(DMeta) const noexcept;
      template<CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      template<CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;

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

      template<CT::Index IDX = Offset>
      Count MergeAt(const T*, const T*, const IDX&);
      template<CT::Index IDX = Offset>
      Count MergeAt(const T&, const IDX&);
      template<CT::Index IDX = Offset>
      Count MergeAt(T&&, const IDX&);
      template<CT::Semantic S, CT::Index IDX = Offset>
      Count MergeAt(S&&, const IDX&) requires (CT::Exact<TypeOf<S>, T>);

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

      template<CT::Data ALT_T = T>
      bool operator == (const TAny<ALT_T>&) const noexcept;
      bool operator == (const Any&) const noexcept;

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false, bool BY_ADDRESS_ONLY = false, CT::Data ALT_T>
      Count RemoveValue(const ALT_T&);
      Count RemoveIndex(const Offset&, const Count&);
      Iterator RemoveIndex(const Iterator&);

      NOD() TAny& Trim(const Count&);
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
      template<bool REVERSE = false, bool BY_ADDRESS_ONLY = false, CT::Data ALT_T>
      NOD() Index Find(const ALT_T&, const Offset& = 0) const;

      NOD() bool Compare(const TAny&) const noexcept;
      NOD() bool CompareLoose(const TAny&) const noexcept requires CT::Character<T>;
      NOD() Count Matches(const TAny&) const noexcept;
      NOD() Count MatchesLoose(const TAny&) const noexcept requires CT::Character<T>;

      template<bool ASCEND = false>
      void Sort();

      template<CT::Block WRAPPER = TAny>
      NOD() WRAPPER Extend(const Count&);

      void Swap(const Offset&, const Offset&);
      void Swap(const Index&, const Index&);

      Count GatherFrom(const Block&, Index = IndexFront);
      Count GatherFrom(const Block&, DataState, Index = IndexFront);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() TAny operator + (const TAny&) const;
      NOD() TAny operator + (TAny&&) const;
      template<CT::Semantic S>
      NOD() TAny operator + (S&& rhs) const requires (CT::Exact<TypeOf<S>, TAny>) {
         return Concatenate<TAny>(rhs.Forward());
      }

      TAny& operator += (const TAny&);
      TAny& operator += (TAny&&);
      template<CT::Semantic S>
      TAny& operator += (S&& rhs) requires (CT::Exact<TypeOf<S>, TAny>) {
         InsertBlock(rhs.Forward());
         return *this;
      }

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;

   protected:
      template<bool CREATE = false, bool SETSIZE = false>
      void AllocateMore(Count);
      void AllocateLess(Count);

      constexpr void ResetState() noexcept;
      constexpr void ResetType() noexcept;

      template<bool OVERWRITE_STATE, bool OVERWRITE_ENTRY>
      void CopyProperties(const Block&) noexcept;

   private:
      using Any::FromMeta;
      using Any::FromBlock;
      using Any::FromState;
      using Any::From;
      using Any::Wrap;
      using Any::WrapAs;
   };


   ///                                                                        
   ///   A sparse element access that dereferences on overwrite               
   ///                                                                        
   template<CT::Data T>
   class TAny<T>::KnownPointer {
      static_assert(CT::Sparse<T>, "T must be a pointer");
      friend class TAny<T>;

   private: TESTING(public:)
      T mPointer {};
      Inner::Allocation* mEntry {};

      void Free();

   public:
      constexpr KnownPointer() noexcept = default;

      KnownPointer(const KnownPointer&) noexcept;
      KnownPointer(KnownPointer&&) noexcept;
      KnownPointer(Disowned<KnownPointer>&&) noexcept;
      KnownPointer(Abandoned<KnownPointer>&&) noexcept;

      explicit KnownPointer(const T&);
      KnownPointer(Disowned<T>&&) noexcept;
      ~KnownPointer();

      KnownPointer& operator = (const KnownPointer&) noexcept;
      KnownPointer& operator = (KnownPointer&&) noexcept;
      KnownPointer& operator = (Disowned<KnownPointer>&&) noexcept;
      KnownPointer& operator = (Abandoned<KnownPointer>&&) noexcept;

      KnownPointer& operator = (const T&);
      KnownPointer& operator = (Disowned<T>&&);
      KnownPointer& operator = (::std::nullptr_t);

      bool operator == (const KnownPointer&) const noexcept;

      NOD() Hash GetHash() const;

      auto operator -> () const;
      auto operator -> ();

      decltype(auto) operator * () const;
      decltype(auto) operator * ();

      operator T () const noexcept {
         return mPointer;
      }
   };


   ///                                                                        
   ///   TAny iterator                                                        
   ///                                                                        
   template<CT::Data T>
   template<bool MUTABLE>
   struct TAny<T>::TIterator {
      LANGULUS(UNINSERTABLE) true;

   protected:
      using MemberType = typename TAny<T>::MemberType;
      using TypeInner = typename TAny<T>::TypeInner;
      friend class TAny<T>;

      const TypeInner* mElement;

      TIterator(const TypeInner*) noexcept;

   public:
      NOD() bool operator == (const TIterator&) const noexcept;

      operator TypeInner& () const noexcept requires (MUTABLE);
      operator const MemberType& () const noexcept requires (!MUTABLE);

      NOD() decltype(auto) operator * () const noexcept requires (MUTABLE);
      NOD() decltype(auto) operator * () const noexcept requires (!MUTABLE);

      NOD() decltype(auto) operator -> () const noexcept requires (MUTABLE);
      NOD() decltype(auto) operator -> () const noexcept requires (!MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };


   /// Concatenate anything with TAny container                               
   template<class T, class LHS, class WRAPPER = TAny<T>>
   NOD() WRAPPER operator + (const LHS& lhs, const TAny<T>& rhs) requires (!CT::DerivedFrom<LHS, TAny<T>>) {
      if constexpr (CT::Sparse<LHS>)
         return operator + (*lhs, rhs);
      else if constexpr (CT::Convertible<LHS, WRAPPER>) {
         auto result = static_cast<WRAPPER>(lhs);
         result += rhs;
         return result;
      }
      else LANGULUS_ERROR("Can't concatenate - LHS is not convertible to WRAPPER");
   }

} // namespace Langulus::Anyness

#include "TAny.inl"
