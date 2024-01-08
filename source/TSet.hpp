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

      using Base::GetReserved;
      using Base::GetInfo;
      using Base::GetInfoEnd;
      using Base::IsAllocated;
      using Base::GetUses;
      using Base::GetCount;

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD()       T& Get(CT::Index auto);
      NOD() const T& Get(CT::Index auto) const;

      NOD()       T& operator[] (CT::Index auto);
      NOD() const T& operator[] (CT::Index auto) const;
      
   protected:
      NOD() const TAny<T>& GetValues() const noexcept;
      NOD()       TAny<T>& GetValues() noexcept;

      NOD() constexpr       T&  GetRaw(Offset) noexcept;
      NOD() constexpr const T&  GetRaw(Offset) const noexcept;
      NOD() constexpr Handle<T> GetHandle(Offset) noexcept;

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;
      NOD() const T& Last() const;
      NOD() T& Last();

      template<class F>
      Count ForEachElement(F&&) const;
      template<class F>
      Count ForEachElement(F&&);

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      NOD() constexpr bool Is(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      NOD() constexpr bool IsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      NOD() constexpr bool IsExact(DMeta) const noexcept;

   protected:
      template<CT::NotSemantic>
      constexpr void Mutate() noexcept;
      void Mutate(DMeta);

   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (CT::Set auto const&) const;

      template<CT::NotSemantic T1>
      requires ::std::equality_comparable_with<T, T1>
      NOD() bool Contains(T1 const&) const;

      template<CT::NotSemantic T1>
      requires ::std::equality_comparable_with<T, T1>
      NOD() Index Find(T1 const&) const;

      template<CT::NotSemantic T1>
      requires ::std::equality_comparable_with<T, T1>
      NOD() Iterator FindIt(T1 const&);

      template<CT::NotSemantic T1>
      requires ::std::equality_comparable_with<T, T1>
      NOD() ConstIterator FindIt(T1 const&) const;

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class T1, class... TAIL>
      requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...>
      Count Insert(T1&&, TAIL&&...);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TSet& operator << (T1&&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count Remove(const T&);
      Iterator RemoveIt(const Iterator&);

      void Clear();
      void Reset();
      void Compact();
   };


   ///                                                                        
   ///   Set iterator                                                         
   ///                                                                        
   template<CT::Data T, bool ORDERED> template<bool MUTABLE>
   struct TSet<T, ORDERED>::TIterator {
   protected:
      friend TOrderedSet<T>;
      friend TUnorderedSet<T>;

      using typename TSet<T, ORDERED>::InfoType;
      const InfoType* mInfo {};
      const InfoType* mSentinel {};
      const T* mValue {};

      TIterator(const InfoType*, const InfoType*, const T*) noexcept;

   public:
      NOD() bool operator == (const TIterator&) const noexcept;

      NOD()       T& operator * () const noexcept requires (MUTABLE);
      NOD() const T& operator * () const noexcept requires (not MUTABLE);

      NOD()       T& operator -> () const noexcept requires (MUTABLE);
      NOD() const T& operator -> () const noexcept requires (not MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Anyness
