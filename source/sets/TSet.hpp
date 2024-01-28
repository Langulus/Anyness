///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Set.hpp"


namespace Langulus::CT
{

   /// Concept for recognizing arguments, with which a statically typed       
   /// set can be constructed                                                 
   template<class T, class...A>
   concept DeepSetMakable = Inner::UnfoldMakableFrom<T, A...>
        or (sizeof...(A) == 1 and Set<Desem<FirstOf<A...>>> and (
              SemanticOf<FirstOf<A...>>::Shallow
           or Inner::SemanticMakableAlt<typename SemanticOf<FirstOf<A...>>::template As<T>>
        ));

   /// Concept for recognizing argument, with which a statically typed        
   /// set can be assigned                                                    
   template<class T, class A>
   concept DeepSetAssignable = Inner::UnfoldMakableFrom<T, A> or (Set<Desem<A>> and (
            SemanticOf<A>::Shallow
         or Inner::SemanticAssignableAlt<typename SemanticOf<A>::template As<T>>)
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

      LANGULUS(POD) false;
      LANGULUS(TYPED) T;
      LANGULUS_BASES(Set<ORDERED>);

   protected:
      static_assert(CT::Inner::Comparable<T>,
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

      template<class T1, class...TAIL>
      requires CT::DeepSetMakable<T, T1, TAIL...>
      TSet(T1&&, TAIL&&...);

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
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr bool IsInsertable(DMeta) const noexcept;
      template<CT::Data>
      NOD() constexpr bool IsInsertable() const noexcept;

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

      NOD() Iterator begin() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator last() const noexcept;
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
      template<CT::NotSemantic>
      constexpr void Mutate() noexcept;
      void Mutate(DMeta);

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::NotSemantic T1>
      requires (CT::Set<T1> or CT::Inner::Comparable<T, T1>)
      bool operator == (const T1&) const;

      template<CT::NotSemantic T1> requires CT::Inner::Comparable<T, T1>
      NOD() bool Contains(T1 const&) const;

      template<CT::NotSemantic T1> requires CT::Inner::Comparable<T, T1>
      NOD() Index Find(T1 const&) const;

      template<CT::NotSemantic T1> requires CT::Inner::Comparable<T, T1>
      NOD() Iterator FindIt(T1 const&);

      template<CT::NotSemantic T1> requires CT::Inner::Comparable<T, T1>
      NOD() ConstIterator FindIt(T1 const&) const;

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class T1, class...TAIL>
      requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...>
      Count Insert(T1&&, TAIL&&...);

      template<class T1> requires CT::Set<Desem<T1>>
      Count InsertBlock(T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TSet& operator << (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
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
