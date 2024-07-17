///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../many/TMany.hpp"


namespace Langulus
{
   namespace A
   {

      ///                                                                     
      /// An abstract Set structure                                           
      /// It defines the size for CT::Set concept                             
      ///                                                                     
      struct BlockSet {
         LANGULUS(ABSTRACT) true;
         LANGULUS(POD) true;

         using InfoType = ::std::uint8_t;
         using OrderType = Offset;

         static constexpr bool CTTI_Container = true;
         static constexpr bool Sequential = false;
         static constexpr Offset InvalidOffset = -1;
         static constexpr Count MinimalAllocation = 8;

      protected:
         // A precomputed pointer for the info (and ordering) bytes     
         // Points to an offset inside mKeys allocation                 
         // Each byte represents an entry, and can be three things:     
         //    0 - the index is not used, data is not initialized       
         //    1 - the index is used, key is exactly where it should be 
         //   2+ - the index is used, but bucket is info-1 buckets to   
         //         the right of this index                             
         InfoType* mInfo {};

         // The block that contains the keys and info bytes             
         Anyness::Block<> mKeys;

      public:
         constexpr BlockSet() noexcept = default;
         constexpr BlockSet(const BlockSet&) noexcept = default;
         constexpr BlockSet(BlockSet&&) noexcept = default;

         constexpr BlockSet& operator = (const BlockSet&) noexcept = default;
         constexpr BlockSet& operator = (BlockSet&&) noexcept = default;
      };

   } // namespace Langulus::A

   namespace CT
   {

      /// A reflected set type is any type that publicly inherits A::BlockSet 
      /// and is binary compatible to it                                      
      /// Keep in mind, that sparse types are never considered CT::Set!       
      template<class...T>
      concept Set = ((DerivedFrom<T, A::BlockSet>
          and sizeof(T) == sizeof(A::BlockSet)) and ...);

      /// Check if a type is a statically typed set                           
      template<class...T>
      concept TypedSet = Set<T...> and Typed<T...>;

   } // namespace Langulus::CT

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased set block, base for all set types                        
   ///                                                                        
   ///   This is an inner structure, that doesn't reference any memory,       
   /// only provides the functionality to do so. You can use BlockSet as a    
   /// lightweight intermediate structure for iteration, etc.                 
   ///                                                                        
   struct BlockSet : A::BlockSet {
      LANGULUS(ABSTRACT) false;

      static constexpr bool Ownership = false;
      static constexpr bool Ordered = false;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      using A::BlockSet::BlockSet;
      using A::BlockSet::operator =;

   protected:
      template<CT::Set TO, template<class> class S, CT::Set FROM>
      requires CT::Intent<S<FROM>>
      void BlockTransfer(S<FROM>&&);

      template<CT::Set>
      void BranchOut();

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      template<CT::Set = UnorderedSet>
      NOD() DMeta GetType() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsUntyped() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsDeep() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsSparse() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsDense() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr DataState GetState() const noexcept;
      NOD() constexpr Count GetCount() const noexcept;
      NOD() Count GetCountDeep() const noexcept;
      NOD() Count GetCountElementsDeep() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;

      NOD() bool IsMissing() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() bool IsMissingDeep() const;

      template<CT::Set = UnorderedSet>
      NOD() bool IsExecutable() const noexcept;
      template<CT::Set = UnorderedSet>
      NOD() bool IsExecutableDeep() const;

      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsInsertable(DMeta) const noexcept;
      template<CT::Data, CT::Set = UnorderedSet>
      NOD() constexpr bool IsInsertable() const noexcept;

      template<CT::Set>
      NOD() bool IsOrdered() const noexcept;

      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      #if LANGULUS(DEBUG)
         template<CT::Set> void Dump() const;
      #endif

   protected:
      template<CT::Set>
      NOD() auto& GetValues() const noexcept;
      template<CT::Set>
      NOD() auto& GetValues() noexcept;

      NOD() InfoType const* GetInfo() const noexcept;
      NOD() InfoType*       GetInfo() noexcept;
      NOD() InfoType const* GetInfoEnd() const noexcept;

      NOD() Count GetCountDeep(const Block<>&) const noexcept;
      NOD() Count GetCountElementsDeep(const Block<>&) const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) Get(CT::Index auto) const;

      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) operator[] (CT::Index auto) const;

   protected:
      template<CT::Set, CT::Index INDEX>
      NOD() Offset SimplifyIndex(INDEX) const
      noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>);

      NOD() static Offset GetBucket(Offset, const CT::NoIntent auto&) noexcept;
      NOD() static Offset GetBucketUnknown(Offset, const Block<>&) noexcept;

      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRaw(Offset)       IF_UNSAFE(noexcept);
      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRaw(Offset) const IF_UNSAFE(noexcept);

      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRef(Offset)       IF_UNSAFE(noexcept);
      template<CT::Set = UnorderedSet>
      NOD() decltype(auto) GetRef(Offset) const IF_UNSAFE(noexcept);

      template<CT::Set = UnorderedSet>
      NOD() auto GetHandle(Offset) IF_UNSAFE(noexcept);

   public:
      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<class Set>
      struct Iterator;

      template<CT::Set SET>
      NOD() Iterator<SET> begin() noexcept;
      template<CT::Set SET>
      NOD() Iterator<const SET> begin() const noexcept;

      template<CT::Set SET>
      NOD() Iterator<SET> last() noexcept;
      template<CT::Set SET>
      NOD() Iterator<const SET> last() const noexcept;

      constexpr A::IteratorEnd end() const noexcept { return {}; }

      template<bool REVERSE = false, CT::Set>
      Count ForEach(auto&&...) const;

      template<bool REVERSE = false, CT::Set>
      Count ForEachElement(auto&&) const;

      template<bool REVERSE = false, bool SKIP = true, CT::Set>
      Count ForEachDeep(auto&&...) const;

   protected:
      template<class F>
      static constexpr bool NoexceptIterator = not LANGULUS_SAFE()
         and noexcept(Fake<F&&>().operator() (Fake<ArgumentOf<F>>()));

      template<CT::Set, bool REVERSE>
      LoopControl ForEachInner(auto&& f, Count&) const noexcept(NoexceptIterator<decltype(f)>);

      template<CT::Set, bool REVERSE, bool SKIP>
      LoopControl ForEachDeepInner(auto&&, Count&) const;

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Set, CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      template<CT::Set>
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Set, CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      template<CT::Set>
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Set, CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      template<CT::Set>
      NOD() bool IsExact(DMeta) const noexcept;

      template<bool CONSTRAIN = false, CT::Set = UnorderedSet>
      void SetType(DMeta);
      template<CT::Data, bool CONSTRAIN = false, CT::Set = UnorderedSet>
      void SetType();

   protected:
      template<CT::Set, CT::Data, class FORCE = Many>
      bool Mutate();
      template<CT::Set, class FORCE = Many>
      bool Mutate(DMeta);

      template<CT::Set = UnorderedSet>
      NOD() constexpr bool IsTypeCompatibleWith(CT::Set auto const&) const noexcept;
      
   public:
      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::Set = UnorderedSet>
      bool operator == (const CT::NoIntent auto&) const;

      template<CT::Set = UnorderedSet>
      NOD() Hash GetHash() const;

      template<CT::Set = UnorderedSet>
      NOD() bool Contains(const CT::NoIntent auto&) const;

      template<CT::Set = UnorderedSet>
      NOD() Index Find(const CT::NoIntent auto&) const;
      template<CT::Set THIS = UnorderedSet>
      NOD() Iterator<THIS> FindIt(const CT::NoIntent auto&);
      template<CT::Set THIS = UnorderedSet>
      NOD() Iterator<const THIS> FindIt(const CT::NoIntent auto&) const;

   protected:
      template<CT::Set>
      NOD() Offset FindInner(const CT::NoIntent auto&) const;
      template<CT::Set>
      NOD() Offset FindBlockInner(const Block<>&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<CT::Set = UnorderedSet>
      void Reserve(Count);

   protected:
      /// @cond show_protected                                                
      template<CT::Set>
      void AllocateFresh(Count);
      template<CT::Set, bool REUSE>
      void AllocateData(Count);
      template<CT::Set>
      void AllocateInner(Count);

      void Keep() const noexcept;
      template<CT::Set>
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::Set = UnorderedSet, class T1, class...TAIL>
      Count Insert(T1&&, TAIL&&...);

      template<CT::Set = UnorderedSet, class T> requires CT::Set<Deint<T>>
      Count InsertBlock(T&&);
      
      template<CT::Set = UnorderedSet, class T> requires CT::Block<Deint<T>>
      Count InsertBlock(T&&);

   protected:
      template<CT::Set>
      auto CreateValHandle(auto&&);

      template<CT::Set>
      NOD() Size RequestKeyAndInfoSize(Count, Offset&) const IF_UNSAFE(noexcept);

      template<CT::Set>
      void Rehash(Count);
      template<CT::Set>
      void ShiftPairs();

      template<CT::Set, bool CHECK_FOR_MATCH>
      Offset InsertInner(Offset, auto&&);

      template<CT::Set, bool CHECK_FOR_MATCH, template<class> class S, CT::Block B>
      requires CT::Intent<S<B>>
      Offset InsertBlockInner(Offset, S<B>&&);

      template<CT::Set>
      Count UnfoldInsert(auto&&);

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::Set = UnorderedSet>
      Count Remove(const CT::NoIntent auto&);

      template<CT::Set = UnorderedSet>
      void Clear();
      template<CT::Set = UnorderedSet>
      void Reset();
      template<CT::Set = UnorderedSet>
      void Compact();

   protected:
      template<CT::Set, bool FORCE = true>
      void ClearInner();

      template<CT::Set>
      void RemoveInner(Offset) IF_UNSAFE(noexcept);
      template<CT::Set>
      Count RemoveKeyInner(const CT::NoIntent auto&);

   #if LANGULUS(TESTING)
      public: NOD() constexpr const void* GetRawMemory() const noexcept;
      public: NOD() const Allocation* GetEntry() const noexcept;
   #endif
   };


   ///                                                                        
   ///   Set iterator                                                         
   ///                                                                        
   template<class SET>
   struct BlockSet::Iterator : A::Iterator {
      static_assert(CT::Set<SET>, "SET must be a CT::Set type");
      static constexpr bool Mutable = CT::Mutable<SET>;
      static constexpr bool TypeErased = not CT::Typed<SET>;

      using T = Conditional<TypeErased, void,
         Conditional<Mutable, TypeOf<SET>, const TypeOf<SET>>>;

      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED)    T;

   protected:
      friend struct BlockSet;
      using InnerT = Conditional<TypeErased, Block<>, T*>;

      // Pointer to the currently selected info                         
      const InfoType* mInfo;
      // Pointer to the end of the set                                  
      const InfoType* mSentinel;
      // Currently selected element                                     
      InnerT mKey;

      constexpr Iterator(const InfoType*, const InfoType*, const InnerT&) noexcept;

   public:
      Iterator() noexcept = delete;
      constexpr Iterator(const Iterator&) noexcept = default;
      constexpr Iterator(Iterator&&) noexcept = default;
      constexpr Iterator(const A::IteratorEnd&) noexcept;

      constexpr Iterator& operator = (const Iterator&) noexcept = default;
      constexpr Iterator& operator = (Iterator&&) noexcept = default;

      NOD() constexpr bool operator == (const Iterator&) const noexcept;
      NOD() constexpr bool operator == (const A::IteratorEnd&) const noexcept;

      NOD() constexpr decltype(auto) operator * () const;

      // Prefix operator                                                
      constexpr Iterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() constexpr Iterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
      constexpr operator Iterator<const SET>() const noexcept requires Mutable;
   };

} // namespace Langulus::Anyness
