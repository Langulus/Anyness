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
      static_assert(CT::Inner::Comparable<T>,
         "Set's type must be equality-comparable to itself");

      using Value = T;
      using Self = TUnorderedSet<T>;

      LANGULUS(TYPED) T;

      static constexpr bool Ordered = false;

   public:
      ///                                                                     
      ///   Construction & Assignment                                         
      ///                                                                     
      constexpr TUnorderedSet();

      TUnorderedSet(const TUnorderedSet&);
      TUnorderedSet(TUnorderedSet&&);

      TUnorderedSet(const CT::NotSemantic auto&);
      TUnorderedSet(CT::NotSemantic auto&);
      TUnorderedSet(CT::NotSemantic auto&&);
      TUnorderedSet(CT::Semantic auto&&);

      template<CT::Data HEAD, CT::Data... TAIL>
      TUnorderedSet(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      ~TUnorderedSet();

      TUnorderedSet& operator = (const TUnorderedSet&);
      TUnorderedSet& operator = (TUnorderedSet&&);

      TUnorderedSet& operator = (const CT::NotSemantic auto&);
      TUnorderedSet& operator = (CT::NotSemantic auto&);
      TUnorderedSet& operator = (CT::NotSemantic auto&&);
      TUnorderedSet& operator = (CT::Semantic auto&&);

   public:
      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() DMeta GetType() const;
      NOD() constexpr bool IsUntyped() const noexcept;
      NOD() constexpr bool IsTypeConstrained() const noexcept;
      NOD() constexpr bool IsAbstract() const noexcept;
      NOD() constexpr bool IsConstructible() const noexcept;
      NOD() constexpr bool IsDeep() const noexcept;
      NOD() constexpr bool IsSparse() const noexcept;
      NOD() constexpr bool IsDense() const noexcept;
      NOD() constexpr Size GetStride() const noexcept;
      NOD() constexpr Size GetBytesize() const noexcept;

   public:
      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD()       T& Get(const CT::Index auto&);
      NOD() const T& Get(const CT::Index auto&) const;

      NOD()       T& operator[] (const CT::Index auto&);
      NOD() const T& operator[] (const CT::Index auto&) const;
      
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
      template<class>
      NOD() constexpr bool Is() const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const TUnorderedSet&) const;

      NOD() bool Contains(const T&) const;
      NOD() Index Find(const T&) const;

   protected:
      NOD() Offset FindIndex(const T&) const;

   public:
      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(const Count&);

   protected:
      void AllocateFresh(const Count&);
      template<bool REUSE>
      void AllocateData(const Count&);
      void AllocateInner(const Count&);

   public:
      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Count Insert(const T&);
      Count Insert(T&&);
      Count Insert(CT::Semantic auto&&);

      TUnorderedSet& operator << (const T&);
      TUnorderedSet& operator << (T&&);
      TUnorderedSet& operator << (CT::Semantic auto&&);

   protected:
      NOD() static Size RequestKeyAndInfoSize(Count, Offset&) noexcept;

      void Rehash(const Count&);

      template<bool CHECK_FOR_MATCH, CT::Semantic S>
      Offset InsertInner(const Offset&, S&&);

      template<class ALT_T>
      void CloneInner(const ALT_T&, ALT_T&) const;

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      Count Remove(const T&);
      Count RemoveIndex(const Index&);
      Iterator RemoveIndex(const Iterator&);

      void Clear();
      void Reset();
      void Compact();

   protected:
      void ClearInner();
      template<class ALT_T>
      static void RemoveInner(ALT_T*) noexcept;
      template<class ALT_T>
      static void Overwrite(ALT_T&&, ALT_T&) noexcept;
      void RemoveIndex(const Offset&) SAFETY_NOEXCEPT();
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
