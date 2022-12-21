///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedSet.hpp"

namespace Langulus::Anyness
{

   ///                                                                        
   /// A highly optimized unordered hashset implementation, using the Robin   
   /// Hood algorithm                                                         
   ///                                                                        
   template<CT::Data T>
   class TUnorderedSet : public UnorderedSet {
   public:
      static_assert(CT::Comparable<T>, "Can't compare elements for map");
      static_assert(CT::NotSemantic<T>, "T can't be semantic");

      using Value = T;
      using ValueInner = typename TAny<T>::TypeInner;
      using Self = TUnorderedSet<T>;
      using Allocator = Inner::Allocator;

      /// Makes TUnorderedSet CT::Typed                                       
      using MemberType = ValueInner;

      static constexpr Count MinimalAllocation = 8;
      static constexpr bool Ordered = false;

      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

   public:
      constexpr TUnorderedSet();
      TUnorderedSet(::std::initializer_list<T>);
      TUnorderedSet(const TUnorderedSet&);
      TUnorderedSet(TUnorderedSet&&) noexcept;

      template<CT::Semantic S>
      constexpr TUnorderedSet(S&&) noexcept requires (S::template Exact<TUnorderedSet<T>>);

      ~TUnorderedSet();

      TUnorderedSet& operator = (const TUnorderedSet&);
      TUnorderedSet& operator = (TUnorderedSet&&) noexcept;
      template<CT::Semantic S>
      TUnorderedSet& operator = (S&&) noexcept requires (S::template Exact<TUnorderedSet<T>>);

      TUnorderedSet& operator = (const T&);
      TUnorderedSet& operator = (T&&) noexcept;
      template<CT::Semantic S>
      TUnorderedSet& operator = (S&&) noexcept requires (S::template Exact<T>);

   public:
      NOD() DMeta GetType() const;

      template<class>
      NOD() constexpr bool Is() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsConstructible() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr Size GetByteSize() const noexcept;

      void Allocate(const Count&);

      NOD() TUnorderedSet Clone() const;

      bool operator == (const TUnorderedSet&) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const T&);
      Count Insert(T&&);
      template<CT::Semantic S>
      Count Insert(S&&) requires (S::template Exact<T>);

      TUnorderedSet& operator << (const T&);
      TUnorderedSet& operator << (T&&);
      template<CT::Semantic S>
      TUnorderedSet& operator << (S&&) requires (S::template Exact<T>);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count RemoveValue(const T&);
      Count RemoveIndex(const Index&);
      Iterator RemoveIndex(const Iterator&);

      void Clear();
      void Reset();
      void Compact();

      ///                                                                     
      ///   Search                                                            
      ///                                                                     
      NOD() bool Contains(const T&) const;
      NOD() Index Find(const T&) const;

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;

      Count ForEachElement(TFunctor<bool(const Block&)>&&) const;
      Count ForEachElement(TFunctor<bool(Block&)>&&);
      Count ForEachElement(TFunctor<void(const Block&)>&&) const;
      Count ForEachElement(TFunctor<void(Block&)>&&);

   protected:
      template<bool REUSE>
      void AllocateKeys(const Count&);
      void AllocateInner(const Count&);
      void Rehash(const Count&, const Count&);
      template<bool CHECK_FOR_MATCH, CT::Semantic S>
      Offset InsertInner(const Offset&, S&&);

      void ClearInner();

      template<class ALT_T>
      void CloneInner(const ALT_T&, ALT_T&) const;

      template<class ALT_T>
      static void RemoveInner(ALT_T*) noexcept;

      template<class ALT_T>
      static void Overwrite(ALT_T&&, ALT_T&) noexcept;

      NOD() static Size RequestKeyAndInfoSize(Count, Offset&) noexcept;

      void RemoveIndex(const Offset&) noexcept;

      NOD() const TAny<T>& GetValues() const noexcept;
      NOD() TAny<T>& GetValues() noexcept;

      NOD() decltype(auto) Get(const Index&) const;
      NOD() decltype(auto) Get(const Index&);
      NOD() decltype(auto) Get(const Offset&) const noexcept;
      NOD() decltype(auto) Get(const Offset&) noexcept;

      NOD() Offset GetBucket(const T&) const noexcept;
      NOD() Offset FindIndex(const T&) const;

   TESTING(public:)
      NOD() constexpr auto GetRaw() const noexcept;
      NOD() constexpr auto GetRaw() noexcept;
      NOD() constexpr auto GetRawEnd() const noexcept;
   };


   ///                                                                        
   ///   Unordered set iterator                                               
   ///                                                                        
   template<CT::Data T>
   template<bool MUTABLE>
   struct TUnorderedSet<T>::TIterator {
   protected:
      friend class TUnorderedSet<T>;

      const InfoType* mInfo {};
      const InfoType* mSentinel {};
      const ValueInner* mValue {};

      TIterator(const InfoType*, const InfoType*, const ValueInner*) noexcept;

   public:
      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() ValueInner& operator * () const noexcept requires (MUTABLE);
      NOD() const ValueInner& operator * () const noexcept requires (!MUTABLE);

      NOD() ValueInner& operator -> () const noexcept requires (MUTABLE);
      NOD() const ValueInner& operator -> () const noexcept requires (!MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Anyness

#include "TUnorderedSet.inl"
