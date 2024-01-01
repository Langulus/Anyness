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


namespace Langulus::CT
{

   /// Concept for recognizing arguments, with which a statically typed       
   /// container can be constructed                                           
   template<class T, class...A>
   concept DeepMakable = Inner::UnfoldMakableFrom<T, A...>
        or (sizeof...(A) == 1 and Block<Desem<FirstOf<A...>>>
           and SemanticOf<FirstOf<A...>>::Shallow or (
              Inner::SemanticMakableAlt<typename SemanticOf<FirstOf<A...>>::template As<T>>
        ));

   /// Concept for recognizing argument, with which a statically typed        
   /// container can be assigned                                              
   template<class T, class A>
   concept DeepAssignable = Inner::UnfoldMakableFrom<T, A> or (Block<Desem<A>>
       and SemanticOf<A>::Shallow or (
          Inner::SemanticAssignableAlt<typename SemanticOf<A>::template As<T>>));

} // namespace Langulus::CT

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
   /// type-erased container to a statically-optimized equivalent. Anyness    
   /// provides a strong guarantee that this operation is completely safe.    
   ///                                                                        
   template<CT::Data T>
   class TAny : public Any {
      LANGULUS(DEEP) true;
      LANGULUS(POD) false;
      LANGULUS(TYPED) T;
      LANGULUS_BASES(Any);

   public:
      static_assert(CT::Inner::Insertable<T>,
         "Contained type is not insertable");
      static_assert(CT::Inner::Allocatable<T>,
         "Contained type is not allocatable");

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

      template<class T1, class...TAIL>
      requires CT::DeepMakable<T, T1, TAIL...>
      TAny(T1&&, TAIL&&...);

      ~TAny();

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      TAny& operator = (const TAny&);
      TAny& operator = (TAny&&);

      template<class T1> requires CT::DeepAssignable<T, T1>
      TAny& operator = (T1&&);

   public:
      NOD() static TAny From(auto&&, Count = 1);

      template<CT::Data... LIST_T>
      NOD() static TAny Wrap(LIST_T&&...);
   
      void Null(Count);

      NOD() DMeta GetType() const noexcept;
      NOD() const T* GetRaw() const noexcept;
      NOD()       T* GetRaw()       noexcept;
      NOD() const T* GetRawEnd() const noexcept;
      NOD() decltype(auto) GetHandle(Offset)       IF_UNSAFE(noexcept);
      NOD() decltype(auto) GetHandle(Offset) const IF_UNSAFE(noexcept);


   private: IF_LANGULUS_TESTING(public:)
      using Any::GetRawSparse;

   public:
      NOD() const T& Last() const;
      NOD()       T& Last();

      template<CT::Data = T>
      NOD() decltype(auto) Get(Offset) const noexcept;
      template<CT::Data = T>
      NOD() decltype(auto) Get(Offset) noexcept;

      NOD() const T& operator [] (CT::Index auto) const;
      NOD()       T& operator [] (CT::Index auto);

      NOD() constexpr bool IsTyped() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr bool IsPOD() const noexcept;
      NOD() constexpr bool IsResolvable() const noexcept;

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
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<bool MOVE_ASIDE = true, class T1, class...TAIL>
      requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...>
      Count Insert(CT::Index auto, T1&&, TAIL&&...);

      template<bool MOVE_ASIDE = true, class...A>
      requires ::std::constructible_from<T, A...>
      Conditional<CT::Sparse<T>, T, T&> Emplace(CT::Index auto, A&&...);

      Count New(Count = 1);

      template<class...A>
      requires CT::Inner::MakableFrom<T, A...>
      Count New(Count, A&&...);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator << (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator >> (T1&&);

      template<bool MOVE_ASIDE = true, class T1, class...TAIL>
      requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...>
      Count Merge(CT::Index auto, T1&&, TAIL&&...);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator <<= (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator >>= (T1&&);

   private:
      // Disable these inherited functions                              
      using Any::SmartPush;

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false>
      Count Remove(const CT::Data auto&);
      Count RemoveIndex(CT::Index auto, Count = 1);
      Iterator RemoveIt(const Iterator&, Count = 1);

      void Trim(Count);
      NOD() TAny Crop(Offset, Count) const;
      NOD() TAny Crop(Offset, Count);

      void Clear();
      void Reset();

      ///                                                                     
      ///   Search                                                            
      ///                                                                     
      template<bool REVERSE = false>
      NOD() Index Find(const CT::Data auto&, Offset = 0) const noexcept;

      template<CT::Data ALT_T = T>
      requires (CT::Inner::Comparable<T, ALT_T>)
      bool operator == (const TAny<ALT_T>&) const noexcept;

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
      NOD() WRAPPER Extend(Count);

      void Swap(CT::Index auto, CT::Index auto);

      template<bool REVERSE = false>
      Count GatherFrom(const Block&);
      template<bool REVERSE = false>
      Count GatherFrom(const Block&, DataState);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T1> requires CT::DeepMakable<T, T1>
      NOD() TAny operator + (T1&&) const;

      template<class T1> requires CT::DeepMakable<T, T1>
      TAny& operator += (T1&&);

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

      operator       T& () const noexcept requires (MUTABLE);
      operator const T& () const noexcept requires (not MUTABLE);

      NOD()       T& operator *  () const noexcept requires (MUTABLE);
      NOD() const T& operator *  () const noexcept requires (not MUTABLE);

      NOD()       T& operator -> () const noexcept requires (MUTABLE);
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
