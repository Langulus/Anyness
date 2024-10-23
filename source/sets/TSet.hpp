///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Set.hpp"


namespace Langulus::CT
{

   /// Concept for recognizing arguments, with which a statically typed       
   /// set can be constructed                                                 
   template<class T, class...A>
   concept DeepSetMakable = UnfoldMakableFrom<T, A...>
        or (sizeof...(A) == 1 and Set<Deint<FirstOf<A...>>> and (
              IntentOf<FirstOf<A...>>::Shallow
           or IntentMakableAlt<typename IntentOf<FirstOf<A...>>::template As<T>>
        ));

   /// Concept for recognizing argument, with which a statically typed        
   /// set can be assigned                                                    
   template<class T, class A>
   concept DeepSetAssignable = UnfoldMakableFrom<T, A> or (Set<Deint<A>> and (
            IntentOf<A>::Shallow
         or IntentAssignableAlt<typename IntentOf<A>::template As<T>>)
      );

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   /// A hashset implementation, using the Robin Hood algorithm               
   ///                                                                        
   template<CT::Data T, bool ORDERED = false>
   struct TSet : Set<ORDERED> {
      using Value = T;
      using Base = Set<ORDERED>;
      using Self = TSet<T, ORDERED>;
      using BlockType = TMany<T>;

      LANGULUS(POD) false;
      LANGULUS(TYPED) T;
      LANGULUS_BASES(Set<ORDERED>);

   protected:
      static_assert(CT::Comparable<T, T>,
         "Set's type must be equality-comparable to itself");

      friend struct BlockSet;
      using typename Base::InfoType;
      using Base::InvalidOffset;
      using Base::MinimalAllocation;
      using Base::mKeys;
      using Base::mInfo;

   public:
      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr TSet();
      TSet(const TSet&);
      TSet(TSet&&);

      template<class T1, class...TN>
      requires CT::DeepSetMakable<T, T1, TN...>
      TSet(T1&&, TN&&...);

      ~TSet();

      TSet& operator = (const TSet&);
      TSet& operator = (TSet&&);

      template<class T1> requires CT::DeepSetAssignable<T, T1>
      TSet& operator = (T1&&);

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetType() const;
      NOD() constexpr bool IsTyped() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;

      NOD() constexpr bool IsInsertable(DMeta) const noexcept;
      template<CT::Data>
      NOD() constexpr bool IsInsertable() const noexcept;

      NOD() bool IsMissingDeep() const;

      NOD() bool IsExecutable() const;
      NOD() bool IsExecutableDeep() const;

      using Base::GetReserved;
      using Base::GetInfo;
      using Base::GetInfoEnd;
      using Base::IsAllocated;
      using Base::GetUses;
      using Base::GetCount;
      using Base::IsEmpty;

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() T const& Get(CT::Index auto) const;

      NOD() T const& operator[] (CT::Index auto) const;

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator = BlockSet::Iterator<TSet>;
      using ConstIterator = BlockSet::Iterator<const TSet>;

      NOD() auto begin() noexcept -> Iterator;
      NOD() auto last()  noexcept -> Iterator;
      NOD() auto begin() const noexcept -> ConstIterator;
      NOD() auto last()  const noexcept -> ConstIterator;
      using Base::end;

      template<bool REVERSE = false>
      Count ForEach(auto&&...) const;
      template<bool REVERSE = false>
      Count ForEach(auto&&...);

      template<bool REVERSE = false>
      Count ForEachElement(auto&&) const;
      template<bool REVERSE = false>
      Count ForEachElement(auto&&);

      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...) const;
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...);

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      NOD() bool IsExact(DMeta) const noexcept;

   protected:
      template<CT::NoIntent>
      constexpr void Mutate() noexcept;
      void Mutate(DMeta);

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::NoIntent T1> requires (CT::Set<T1> or CT::Comparable<T, T1>)
      bool operator == (const T1&) const;

      template<CT::NoIntent T1> requires CT::Comparable<T, T1>
      NOD() bool Contains(T1 const&) const;
      template<CT::NoIntent T1> requires (not CT::Comparable<T, T1>)
      static consteval bool Contains(T1 const&) { return false; }

      template<CT::NoIntent T1> requires CT::Comparable<T, T1>
      NOD() auto Find(T1 const&) const -> Index;

      template<CT::NoIntent T1> requires CT::Comparable<T, T1>
      NOD() auto FindIt(T1 const&) -> Iterator;

      template<CT::NoIntent T1> requires CT::Comparable<T, T1>
      NOD() auto FindIt(T1 const&) const ->ConstIterator;

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class T1, class...TN>
      requires CT::UnfoldMakableFrom<T, T1, TN...>
      Count Insert(T1&&, TN&&...);

      template<class T1> requires CT::Set<Deint<T1>>
      Count InsertBlock(T1&&);

      template<class T1> requires CT::Block<Deint<T1>>
      Count InsertBlock(T1&&);

      template<class T1> requires CT::UnfoldMakableFrom<T, T1>
      TSet& operator << (T1&&);

      template<class T1> requires CT::UnfoldMakableFrom<T, T1>
      TSet& operator >> (T1&&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count Remove(const T&);
      Iterator RemoveIt(const Iterator&);

      void Clear();
      void Reset();
      void Compact();
   };

} // namespace Langulus::Anyness
