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
      friend class BlockSet;
      static_assert(CT::Comparable<T>, "Can't compare elements for set");

      using Value = T;
      using Self = TUnorderedSet<T>;
      using Allocator = Inner::Allocator;

      LANGULUS(TYPED) T;

      static constexpr Count MinimalAllocation = 8;
      static constexpr bool Ordered = false;

      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

   public:
      constexpr TUnorderedSet();
      TUnorderedSet(const TUnorderedSet&);
      TUnorderedSet(TUnorderedSet&&) noexcept;

      TUnorderedSet(::std::initializer_list<T>);

      template<CT::Semantic S>
      TUnorderedSet(S&&) noexcept;

      ~TUnorderedSet();

      TUnorderedSet& operator = (const TUnorderedSet&);
      TUnorderedSet& operator = (TUnorderedSet&&) noexcept;

      TUnorderedSet& operator = (const CT::NotSemantic auto&);
      TUnorderedSet& operator = (CT::NotSemantic auto&&) noexcept;
      template<CT::Semantic S>
      TUnorderedSet& operator = (S&&) noexcept;

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

      bool operator == (const TUnorderedSet&) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const T&);
      Count Insert(T&&);
      template<CT::Semantic S>
      Count Insert(S&&) requires (CT::Exact<TypeOf<S>, T>);

      TUnorderedSet& operator << (const T&);
      TUnorderedSet& operator << (T&&);
      template<CT::Semantic S>
      TUnorderedSet& operator << (S&&) requires (CT::Exact<TypeOf<S>, T>);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count Remove(const T&);
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

      NOD() const T& Get(const CT::Index auto&) const;
      NOD() T& Get(const CT::Index auto&);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;
      NOD() const T& Last() const;
      NOD() T& Last();

      Count ForEachElement(TFunctor<bool(const Block&)>&&) const;
      Count ForEachElement(TFunctor<bool(Block&)>&&);
      Count ForEachElement(TFunctor<void(const Block&)>&&) const;
      Count ForEachElement(TFunctor<void(Block&)>&&);

   protected:
      void AllocateFresh(const Count&);
      template<bool REUSE>
      void AllocateData(const Count&);
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

      void RemoveIndex(const Offset&) SAFETY_NOEXCEPT();

      NOD() const TAny<T>& GetValues() const noexcept;
      NOD() TAny<T>& GetValues() noexcept;

      NOD() Offset GetBucket(const T&) const noexcept;
      NOD() Offset FindIndex(const T&) const;

   TESTING(public:)
      NOD() constexpr const T& GetRaw(Offset) const noexcept;
      NOD() constexpr T& GetRaw(Offset) noexcept;
      NOD() constexpr Handle<T> GetHandle(Offset) noexcept;
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
      const T* mValue {};

      TIterator(const InfoType*, const InfoType*, const T*) noexcept;

   public:
      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() T& operator * () const noexcept requires (MUTABLE);
      NOD() const T& operator * () const noexcept requires (!MUTABLE);

      NOD() T& operator -> () const noexcept requires (MUTABLE);
      NOD() const T& operator -> () const noexcept requires (!MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Anyness

#include "TUnorderedSet.inl"
