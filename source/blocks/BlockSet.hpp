///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../TAny.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased set block, base for all set types                        
   ///                                                                        
   ///   This is an inner structure, that doesn't reference any memory,       
   /// only provides the functionality to do so. You can use BlockSet as a    
   /// lightweight intermediate structure for iteration, etc.                 
   ///                                                                        
   class BlockSet {
   protected:
      using InfoType = ::std::uint8_t;

      // A precomputed pointer for the info bytes                       
      // Points to an offset inside mKeys allocation                    
      // Each byte represents an entry, and can be three things:        
      //    0 - the index is not used, data is not initialized          
      //    1 - the index is used, and key is exactly where it should be
      //   2+ - the index is used, but bucket is info-1 buckets to      
      //         the right of this index                                
      InfoType* mInfo {};

      // The block that contains the keys and info bytes                
      Block mKeys;

   public:
      static constexpr bool Ordered = false;
      static constexpr bool Ownership = false;
      static constexpr bool Sequential = false;
      static constexpr Offset InvalidOffset = -1;
      static constexpr Count MinimalAllocation = 8;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr BlockSet() noexcept = default;
      constexpr BlockSet(const BlockSet&) noexcept = default;
      constexpr BlockSet(BlockSet&&) noexcept = default;
      constexpr BlockSet(CT::Semantic auto&&) noexcept;

      constexpr BlockSet& operator = (const BlockSet&) noexcept = default;
      constexpr BlockSet& operator = (BlockSet&&) noexcept = default;
      constexpr BlockSet& operator = (CT::Semantic auto&&) noexcept;

   protected:
      template<class T>
      void BlockTransfer(CT::Semantic auto&&);
      template<class T>
      void BlockClone(const BlockSet&);

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetType() const noexcept;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsConstructible() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr Size GetBytesize() const noexcept;
      NOD() constexpr Count GetCount() const noexcept;
      NOD() Count GetCountDeep() const noexcept;
      NOD() Count GetCountElementsDeep() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;
      NOD() bool IsMissing() const noexcept;
      NOD() bool IsMissingDeep() const;

      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      DEBUGGERY(void Dump() const);

   protected:
      template<CT::Data T>
      NOD() const TAny<T>& GetValues() const noexcept;
      template<CT::Data T>
      NOD() TAny<T>& GetValues() noexcept;

      NOD() const InfoType* GetInfo() const noexcept;
      NOD() InfoType* GetInfo() noexcept;
      NOD() const InfoType* GetInfoEnd() const noexcept;

      NOD() Count GetCountDeep(const Block&) const noexcept;
      NOD() Count GetCountElementsDeep(const Block&) const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Block Get(const CT::Index auto&);
      NOD() Block Get(const CT::Index auto&) const;

      NOD() Block operator[] (const CT::Index auto&);
      NOD() Block operator[] (const CT::Index auto&) const;

   protected:
      NOD() Block GetInner(const Offset&) IF_UNSAFE(noexcept);
      NOD() Block GetInner(const Offset&) const IF_UNSAFE(noexcept);

      NOD() static Offset GetBucket(Offset, const CT::NotSemantic auto&) noexcept;
      NOD() static Offset GetBucketUnknown(Offset, const Block&) noexcept;

      template<CT::Data T>
      NOD() constexpr T& GetRaw(Offset) IF_UNSAFE(noexcept);
      template<CT::Data T>
      NOD() constexpr const T& GetRaw(Offset) const IF_UNSAFE(noexcept);

      template<CT::Data T>
      NOD() constexpr Handle<T> GetHandle(Offset) const IF_UNSAFE(noexcept);

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

      template<bool REVERSE = false, bool MUTABLE = true>
      Count ForEachElement(auto&&);
      template<bool REVERSE = false>
      Count ForEachElement(auto&&) const;

      template<bool REVERSE = false, bool MUTABLE = true, class... F>
      Count ForEach(F&&...);
      template<bool REVERSE = false, class... F>
      Count ForEach(F&&...) const;
   
      template<bool REVERSE = false, bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachDeep(F&&...);
      template<bool REVERSE = false, bool SKIP = true, class... F>
      Count ForEachDeep(F&&...) const;

      template<bool MUTABLE, class... F>
      Count ForEachElementRev(F&&...);
      template<class... F>
      Count ForEachElementRev(F&&...) const;

      template<bool MUTABLE, class... F>
      Count ForEachRev(F&&...);
      template<class... F>
      Count ForEachRev(F&&...) const;

      template<bool SKIP, bool MUTABLE, class... F>
      Count ForEachDeepRev(F&&...);
      template<bool SKIP, class... F>
      Count ForEachDeepRev(F&&...) const;

   protected:
      template<class F>
      static constexpr bool NoexceptIterator = not LANGULUS_SAFE()
         and noexcept(Fake<F&&>().operator() (Fake<ArgumentOf<F>>()));

      template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
      Count ForEachInner(auto&& f) noexcept(NoexceptIterator<decltype(f)>);
      template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
      Count ForEachDeepInner(auto&&);

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::NotSemantic>
      void Mutate();
      void Mutate(DMeta);
      template<CT::Data, CT::Data...>
      NOD() bool Is() const noexcept;
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool IsSimilar() const noexcept;
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() bool IsExact() const noexcept;
      NOD() bool IsExact(DMeta) const noexcept;

      NOD() bool IsTypeCompatibleWith(const BlockSet&) const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const BlockSet&) const;

      NOD() Hash GetHash() const;

      template<class SET = BlockSet>
      NOD() bool Contains(const CT::NotSemantic auto&) const;

      template<class SET = BlockSet>
      NOD() Index Find(const CT::NotSemantic auto&) const;
      template<class SET = BlockSet>
      NOD() Iterator FindIt(const CT::NotSemantic auto&);
      template<class SET = BlockSet>
      NOD() ConstIterator FindIt(const CT::NotSemantic auto&) const;

   protected:
      template<class SET>
      NOD() Offset FindInner(const CT::NotSemantic auto&) const;
      NOD() Offset FindInnerUnknown(const Block&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      template<class SET = BlockSet>
      void Reserve(const Count&);

   protected:
      /// @cond show_protected                                                
      void AllocateFresh(const Count&);
      template<bool REUSE, class SET>
      void AllocateData(const Count&);
      template<class SET>
      void AllocateInner(const Count&);

      void Reference(const Count&) const noexcept;
      void Keep() const noexcept;
      template<bool DESTROY, class SET>
      void Dereference(const Count&);
      template<class SET>
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<bool ORDERED = false>
      Count Insert(const CT::NotSemantic auto&);
      template<bool ORDERED = false>
      Count Insert(CT::NotSemantic auto&);
      template<bool ORDERED = false>
      Count Insert(CT::NotSemantic auto&&);
      template<bool ORDERED = false>
      Count Insert(CT::Semantic auto&&);

      template<bool ORDERED = false>
      Count InsertBlock(CT::Semantic auto&&);

      template<bool ORDERED = false>
      Count Merge(const BlockSet&);
      template<bool ORDERED = false>
      Count Merge(BlockSet&&);
      template<bool ORDERED = false>
      Count Merge(CT::Semantic auto&&);

      BlockSet& operator << (const CT::NotSemantic auto&);
      BlockSet& operator << (CT::NotSemantic auto&);
      BlockSet& operator << (CT::NotSemantic auto&&);
      BlockSet& operator << (CT::Semantic auto&&);

   protected:
      NOD() Size RequestKeyAndInfoSize(Count, Offset&) const IF_UNSAFE(noexcept);

      template<class SET>
      void Rehash(const Count&);
      template<class K>
      void ShiftPairs();

      template<bool CHECK_FOR_MATCH, bool ORDERED>
      Offset InsertInner(const Offset&, CT::Semantic auto&&);
      template<bool CHECK_FOR_MATCH, bool ORDERED>
      Offset InsertInnerUnknown(const Offset&, CT::Semantic auto&&);

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<class SET = BlockSet>
      Count Remove(const CT::NotSemantic auto&);

      template<class SET = BlockSet>
      void Clear();
      template<class SET = BlockSet>
      void Reset();
      void Compact();

   protected:
      template<class SET>
      void ClearInner();

      template<class K>
      void RemoveInner(const Offset&) IF_UNSAFE(noexcept);
      template<class SET>
      Count RemoveKeyInner(const CT::NotSemantic auto&);

   #if LANGULUS(TESTING)
      public: NOD() constexpr const void* GetRawMemory() const noexcept;
      public: NOD() const Allocation* GetEntry() const noexcept;
   #endif
   };


   ///                                                                        
   ///   Map iterator                                                         
   ///                                                                        
   template<bool MUTABLE>
   struct BlockSet::TIterator {
   protected:
      friend class BlockSet;

      const InfoType* mInfo {};
      const InfoType* mSentinel {};
      Block mKey;

      TIterator(const InfoType*, const InfoType*, const Block&) noexcept;

   public:
      TIterator() noexcept = default;
      TIterator(const TIterator&) noexcept = default;
      TIterator(TIterator&&) noexcept = default;

      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() Any operator * () const noexcept;

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
   };

} // namespace Langulus::Anyness


namespace Langulus::CT
{

   /// A reflected set type is any type that inherits BlockSet, and is        
   /// binary compatible to a BlockSet                                        
   /// Keep in mind, that sparse types are never considered CT::Set!          
   template<class... T>
   concept Set = ((DerivedFrom<T, Anyness::BlockSet>
       and sizeof(T) == sizeof(Anyness::BlockSet)) and ...);

   /// Check if a type is a statically typed set                              
   template<class... T>
   concept TypedSet = Set<T...> and Typed<T...>;

} // namespace Langulus::CT
