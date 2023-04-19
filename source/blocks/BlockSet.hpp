///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../TAny.inl"

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased set block, base for all set types                        
   ///                                                                        
   class BlockSet {
   protected:
      using Allocator = Inner::Allocator;
      static constexpr Count MinimalAllocation = 8;
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
      static constexpr bool Ownership = true;
      static constexpr bool Sequential = false;

      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr BlockSet() = default;

      BlockSet(const BlockSet&);
      BlockSet(BlockSet&&) noexcept;
      template<CT::Semantic S>
      constexpr BlockSet(S&&) noexcept;

      template<CT::NotSemantic T>
      BlockSet(::std::initializer_list<T>);

      ~BlockSet();

      BlockSet& operator = (const BlockSet&);
      BlockSet& operator = (BlockSet&&) noexcept;

      BlockSet& operator = (const CT::Data auto&);
      BlockSet& operator = (CT::Data auto&&) noexcept;

   protected:
      template<class T, CT::Semantic S>
      void BlockTransfer(S&&);
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
      NOD() constexpr Size GetByteSize() const noexcept;
      NOD() constexpr Count GetCount() const noexcept;
      NOD() constexpr Count GetReserved() const noexcept;
      NOD() constexpr bool IsEmpty() const noexcept;
      NOD() constexpr bool IsAllocated() const noexcept;
      NOD() constexpr bool HasAuthority() const noexcept;
      NOD() constexpr Count GetUses() const noexcept;

   protected:
      template<CT::Data T>
      NOD() const TAny<T>& GetValues() const noexcept;
      template<CT::Data T>
      NOD() TAny<T>& GetValues() noexcept;

      NOD() const InfoType* GetInfo() const noexcept;
      NOD() InfoType* GetInfo() noexcept;
      NOD() const InfoType* GetInfoEnd() const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Block Get(const CT::Index auto&);
      NOD() Block Get(const CT::Index auto&) const;

      NOD() Block operator[] (const CT::Index auto&);
      NOD() Block operator[] (const CT::Index auto&) const;

   protected:
      NOD() Block GetValue(const Offset&) SAFETY_NOEXCEPT();
      NOD() Block GetValue(const Offset&) const SAFETY_NOEXCEPT();

      NOD() Offset GetBucket(const CT::Data auto&) const noexcept;

      template<CT::Data T>
      NOD() constexpr T& GetRaw(Offset) SAFETY_NOEXCEPT();
      template<CT::Data T>
      NOD() constexpr const T& GetRaw(Offset) const SAFETY_NOEXCEPT();

      template<CT::Data T>
      NOD() constexpr Handle<T> GetHandle(Offset) const SAFETY_NOEXCEPT();

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

      template<bool REVERSE = false, bool MUTABLE = true, class F>
      Count ForEachElement(F&&);
      template<bool REVERSE = false, class F>
      Count ForEachElement(F&&) const;

      template<bool REVERSE = false, bool MUTABLE = true, class... F>
      Count ForEach(F&&...);
      template<bool REVERSE = false, class... F>
      Count ForEach(F&&...) const;
   
      template<bool REVERSE = false, bool SKIP = true, bool MUTABLE = true, class... F>
      Count ForEachDeep(F&&...);
      template<bool REVERSE = false, bool SKIP = true, class... F>
      Count ForEachDeep(F&&...) const;

   protected:
      template<bool MUTABLE, bool REVERSE, class F>
      Count ForEachSplitter(Block&, F&&);
      template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
      Count ForEachDeepSplitter(Block&, F&&);
      template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
      Count ForEachInner(Block&, TFunctor<R(A)>&&);
      template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
      Count ForEachDeepInner(Block&, TFunctor<R(A)>&&);
      template<bool REVERSE, bool MUTABLE, class F>
      Count ForEachElement(Block&, F&&);

   public:
      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data T>
      void Mutate();
      void Mutate(DMeta);

      template<class ALT_T>
      NOD() constexpr bool Is() const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const BlockSet&) const;

      NOD() Hash GetHash() const;

      NOD() bool Contains(const CT::NotSemantic auto&) const;
      NOD() Index Find(const CT::NotSemantic auto&) const;

   protected:
      template<CT::Data T>
      NOD() Offset FindIndex(const T&) const;
      NOD() Offset FindIndexUnknown(const Block&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(const Count& count);
      
   protected:
      /// @cond show_protected                                                
      template<bool REUSE>
      void Allocate(const Count&);
      void AllocateFresh(const Count&);
      void AllocateInner(const Count&);

      void Reference(const Count&) const noexcept;
      void Keep() const noexcept;
      template<bool DESTROY>
      void Dereference(const Count&);
      void Free();
      /// @endcond                                                            

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const CT::NotSemantic auto&);
      Count Insert(CT::NotSemantic auto&&);
      Count Insert(CT::Semantic auto&&);

      Count InsertUnknown(const Block&);
      Count InsertUnknown(Block&&);
      template<CT::Semantic S>
      Count InsertUnknown(S&&) requires (CT::Block<TypeOf<S>>);

      Count Merge(const BlockSet&);
      Count Merge(BlockSet&&);
      template<CT::Semantic S>
      Count Merge(S&&);

      BlockSet& operator << (const CT::NotSemantic auto&);
      BlockSet& operator << (CT::NotSemantic auto&&);
      BlockSet& operator << (CT::Semantic auto&&);

      BlockSet& operator << (const Block&);
      BlockSet& operator << (Block&&);
      template<CT::Semantic S>
      BlockSet& operator << (S&&) requires (CT::Block<TypeOf<S>>);

   protected:
      NOD() Size RequestKeyAndInfoSize(Count, Offset&) noexcept;

      void Rehash(const Count&, const Count&);

      template<bool CHECK_FOR_MATCH, CT::Semantic S>
      Offset InsertInnerUnknown(const Offset&, S&&);
      template<bool CHECK_FOR_MATCH, CT::Semantic S>
      Offset InsertInner(const Offset&, S&&);

      void CloneInner(const Block&, Block&) const;

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      template<CT::NotSemantic T>
      Count Remove(const T&);
      Count RemoveIndex(const Index&);

      void Clear();
      void Reset();
      void Compact();

   protected:
      void ClearInner();
      void RemoveIndex(const Offset&) SAFETY_NOEXCEPT();
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
   };

} // namespace Langulus::Anyness


namespace Langulus::CT
{

   /// A reflected set type is any type that inherits BlockSet, and is        
   /// binary compatible to a BlockSet                                        
   /// Keep in mind, that sparse types are never considered CT::Set!          
   template<class... T>
   concept Set = ((DerivedFrom<T, Anyness::BlockSet>
      && sizeof(T) == sizeof(Anyness::BlockSet)) && ...);

   /// Check if a type is a statically typed set                              
   template<class... T>
   concept TypedSet = ((Set<T> && Typed<T>) && ...);

} // namespace Langulus::CT

#include "BlockSet-Construct.inl"
#include "BlockSet-Capsulation.inl"
#include "BlockSet-Indexing.inl"
#include "BlockSet-RTTI.inl"
#include "BlockSet-Compare.inl"
#include "BlockSet-Memory.inl"
#include "BlockSet-Insert.inl"
#include "BlockSet-Remove.inl"