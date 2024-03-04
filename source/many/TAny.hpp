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
   namespace Inner
   {

      /// Test whether a TAny is constructible with the given arguments       
      ///   @tparam T - the contained type in TAny<T>                         
      ///   @tparam ...A - the arguments to test                              
      ///   @return true if TAny<T> is constructible using {A...}             
      template<class T, class...A>
      consteval bool DeepMakable() noexcept {
         if constexpr (UnfoldMakableFrom<T, A...>) {
            // If we can directly forward A..., then always prefer that 
            return true;
         }
         else if constexpr (sizeof...(A) == 1) {
            // If only one A provided, it HAS to be a CT::Block type    
            using FA = FirstOf<A...>;
            using SA = SemanticOf<FA>;

            if constexpr (CT::Block<Desem<FA>>) {
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
                  // Cloning always calls element constructors, and we  
                  // have to check whether contained elements can do it 
                  return SemanticMakableAlt<typename SA::template As<T>>;
               }
            }
            else return false;
         }
         else return false;
      };
      
      /// Test whether a TAny is assignable with the given argument           
      ///   @tparam T - the contained type in TAny<T>                         
      ///   @tparam A - the argument to test                                  
      ///   @return true if TAny<T> is assignable using = A                   
      template<class T, class A>
      consteval bool DeepAssignable() noexcept {
         if constexpr (UnfoldMakableFrom<T, A>) {
            // If we can directly forward A..., then always prefer that 
            // Othewise, it has to be a CT::Block type                  
            return true;
         }
         else if constexpr (CT::Block<Desem<A>>) {
            using SA = SemanticOf<A>;

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
               return SemanticAssignableAlt<typename SA::template As<T>>;
            }
         }
         else return false;
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
      static_assert(CT::Complete<T>,
         "Contained type must be complete");
      static_assert(CT::Insertable<T>,
         "Contained type must be insertable");
      static_assert(CT::Allocatable<T>,
         "Contained type must be allocatable");
      static_assert(not CT::Reference<T>,
         "Contained type can't be a reference");

      LANGULUS(DEEP) true;
      LANGULUS(POD) false;
      LANGULUS(TYPED) T;
      LANGULUS_BASES(Any);

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr TAny();
      TAny(const TAny&);
      TAny(TAny&&) noexcept;

      template<class T1, class...TN>
      requires CT::DeepMakable<T, T1, TN...>
      TAny(T1&&, TN&&...);

      ~TAny();

      NOD() static TAny From(auto&&, Count = 1);

      template<CT::Data...LIST_T>
      NOD() static TAny Wrap(LIST_T&&...);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      TAny& operator = (const TAny&);
      TAny& operator = (TAny&&);

      template<class T1> requires CT::DeepAssignable<T, T1>
      TAny& operator = (T1&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetType() const noexcept;
      NOD() constexpr Token GetToken() const noexcept;
      constexpr void ResetState() noexcept;

      NOD() constexpr bool IsTyped() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr bool IsPOD() const noexcept;
      NOD() constexpr bool IsResolvable() const noexcept;

      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr Size GetBytesize() const noexcept;
      NOD() constexpr Count GetCountDeep() const noexcept;
      NOD() constexpr Count GetCountElementsDeep() const noexcept;

      NOD() constexpr bool IsMissingDeep() const;
      NOD() constexpr bool IsConcatable(const CT::Block auto&) const noexcept;
      NOD() constexpr bool IsInsertable(DMeta) const noexcept;
      template<CT::Data>
      NOD() constexpr bool IsInsertable() const noexcept;

   protected: IF_LANGULUS_TESTING(public:)
      template<class THIS = TAny<T>>
      NOD() constexpr auto GetRaw() noexcept;
      template<class THIS = TAny<T>>
      NOD() constexpr auto GetRaw() const noexcept;
      template<class THIS = TAny<T>>
      NOD() constexpr auto GetRawEnd() const noexcept;

      template<class = TAny<T>> NOD() IF_UNSAFE(constexpr)
      auto GetRawSparse()       IF_UNSAFE(noexcept);
      template<class = TAny<T>> NOD() IF_UNSAFE(constexpr)
      auto GetRawSparse() const IF_UNSAFE(noexcept);

      template<CT::Data T1, class = TAny<T>>
      NOD() T1*       GetRawAs() noexcept;
      template<CT::Data T1, class = TAny<T>>
      NOD() T1 const* GetRawAs() const noexcept;
      template<CT::Data T1, class = TAny<T>>
      NOD() T1 const* GetRawEndAs() const noexcept;

      template<CT::Data T1, class = TAny<T>>
      NOD() T1**             GetRawSparseAs()       IF_UNSAFE(noexcept);
      template<CT::Data T1, class = TAny<T>>
      NOD() T1 const* const* GetRawSparseAs() const IF_UNSAFE(noexcept);

      template<class = TAny<T>>
      NOD() const Allocation* const* GetEntries() const IF_UNSAFE(noexcept);
      template<class = TAny<T>>
      NOD() const Allocation**       GetEntries()       IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() T const& Last() const;
      NOD() T&       Last();

      template<CT::Data = T>
      NOD() decltype(auto) Get(Offset) const noexcept;
      template<CT::Data = T>
      NOD() decltype(auto) Get(Offset) noexcept;

      NOD() const T& operator [] (CT::Index auto) const;
      NOD()       T& operator [] (CT::Index auto);

      //NOD() decltype(auto) GetHandle(Offset)       IF_UNSAFE(noexcept);
      //NOD() decltype(auto) GetHandle(Offset) const IF_UNSAFE(noexcept);

      NOD() TAny Crop(Offset, Count) const;
      NOD() TAny Crop(Offset, Count);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator      = Block::Iterator<TAny>;
      using ConstIterator = Block::Iterator<const TAny>;

      NOD() Iterator begin() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator last() const noexcept;

      template<bool REVERSE = false>
      Count ForEachElement(auto&&);
      template<bool REVERSE = false>
      Count ForEachElement(auto&&) const;

      template<bool REVERSE = false>
      Count ForEach(auto&&...);
      template<bool REVERSE = false>
      Count ForEach(auto&&...) const;

      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...);
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...) const;

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

      template<bool BINARY_COMPATIBLE = false>
      NOD() bool CastsToMeta(DMeta) const;
      template<bool BINARY_COMPATIBLE = false>
      NOD() bool CastsToMeta(DMeta, Count) const;

      template<CT::Data, bool BINARY_COMPATIBLE = false>
      NOD() bool CastsTo() const;
      template<CT::Data, bool BINARY_COMPATIBLE = false>
      NOD() bool CastsTo(Count) const;

      template<CT::Block B>
      NOD() B ReinterpretAs(const B&) const;
      template<CT::Data T1>
      NOD() TAny<T1> ReinterpretAs() const;

      NOD() Block GetMember(const RTTI::Member&, CT::Index auto);
      NOD() Block GetMember(const RTTI::Member&, CT::Index auto) const;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::NotSemantic T1>
      requires (CT::UntypedBlock<T1>
            or (CT::TypedBlock<T1> and CT::Inner::Comparable<T, TypeOf<T1>>)
            or CT::Inner::Comparable<T, T1>)
      bool operator == (const T1&) const;

      template<bool RESOLVE = true>
      NOD() bool Compare(const CT::Block auto&) const;
      NOD() Hash GetHash() const requires CT::Hashable<T>;

      template<bool REVERSE = false, CT::NotSemantic T1>
      requires CT::Inner::Comparable<T, T1>
      NOD() Index Find(const T1&, Offset = 0) const noexcept;

      template<CT::NotSemantic T1>
      requires CT::Inner::Comparable<T, T1>
      NOD() Iterator FindIt(const T1&);

      template<CT::NotSemantic T1>
      requires CT::Inner::Comparable<T, T1>
      NOD() ConstIterator FindIt(const T1&) const;

      template<bool REVERSE = false>
      NOD() Index FindBlock(const CT::Block auto&, CT::Index auto) const noexcept;

      NOD() bool CompareLoose(const CT::Block auto&) const noexcept;
      NOD() Count Matches(const CT::Block auto&) const noexcept;
      NOD() Count MatchesLoose(const CT::Block auto&) const noexcept;

      template<bool ASCEND = false>
      requires CT::Inner::Sortable<T>
      void Sort();

      void Swap(CT::Index auto, CT::Index auto);

      template<bool REVERSE = false>
      Count GatherFrom(const CT::Block auto&);
      template<bool REVERSE = false>
      Count GatherFrom(const CT::Block auto&, DataState);

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<bool SETSIZE = false>
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<bool MOVE_ASIDE = true, class T1, class...TN>
      requires CT::Inner::UnfoldMakableFrom<T, T1, TN...>
      Count Insert(CT::Index auto, T1&&, TN&&...);

      template<class FORCE = Any, bool MOVE_ASIDE = true, class T1>
      requires CT::Block<Desem<T1>>
      Count InsertBlock(CT::Index auto, T1&&);

      template<bool MOVE_ASIDE = true, class T1, class...TN>
      requires CT::Inner::UnfoldMakableFrom<T, T1, TN...>
      Count Merge(CT::Index auto, T1&&, TN&&...);

      template<class FORCE = Any, bool MOVE_ASIDE = true, class T1>
      requires CT::Block<Desem<T1>>
      Count MergeBlock(CT::Index auto, T1&&);
   
      template<bool MOVE_ASIDE = true, class...A>
      requires ::std::constructible_from<T, A...>
      Conditional<CT::Sparse<T>, T, T&> Emplace(CT::Index auto, A&&...);

      template<class...A>
      requires ::std::constructible_from<T, A...>
      Count New(Count, A&&...);

      Count New(Count = 1) requires CT::Inner::Defaultable<T>;

      template<CT::Deep T1, bool TRANSFER_OR = true>
      requires CT::CanBeDeepened<T1, TAny>
      T1& Deepen();

      void Null(Count);

      NOD() TAny<T> Extend(Count);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator << (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator >> (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator <<= (T1&&);

      template<class T1>
      requires CT::Inner::UnfoldMakableFrom<T, T1>
      TAny& operator >>= (T1&&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<bool REVERSE = false>
      Count Remove(const CT::Data auto&);
      Count RemoveIndex(CT::Index auto, Count = 1);
      Count RemoveIndexDeep(CT::Index auto);
      Iterator RemoveIt(const Iterator&, Count = 1);

      void Trim(Count);
      void Optimize();
      void Clear();
      void Reset();

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T1> requires CT::DeepMakable<T, T1>
      NOD() TAny operator + (T1&&) const;

      template<class T1> requires CT::DeepMakable<T, T1>
      TAny& operator += (T1&&);

      ///                                                                     
      ///   Flow                                                              
      ///                                                                     
      // Intentionally undefined, because it requires Langulus::Flow    
      Flow::Verb& Run(Flow::Verb&) const;
      // Intentionally undefined, because it requires Langulus::Flow    
      Flow::Verb& Run(Flow::Verb&);

   private:
      /// Services graveyard - disallowed interface for typed containers      
      using Any::FromMeta;
      using Any::FromBlock;
      using Any::FromState;
      using Any::From;
      using Any::Wrap;
      using Any::SetType;
      using Any::MakeTypeConstrained;
      using Any::GetResolved;
      using Any::GetDense;
      using Any::GetBlockDeep;
   };

} // namespace Langulus::Anyness
