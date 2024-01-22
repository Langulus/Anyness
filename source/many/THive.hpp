///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TAny.hpp"


namespace Langulus
{
   namespace A
   {

      /// An abstract hive                                                    
      struct Hive {
         LANGULUS(ABSTRACT) true;
      };

   } // namespace Langulus::A

   namespace CT
   {

      /// Hive is anything derived from A::Hive                               
      template<class...T>
      concept Hive = (DerivedFrom<T, A::Hive> and ...);

   } // namespace Langulus::CT

} // namespace Langulus

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Hive                                                                 
   ///                                                                        
   ///   Templated container used to contain, produce, but most importantly   
   /// reuse memory for instances of data. Elements are guaranteed to NEVER   
   /// move, and are reused in-place. Extensively used by Flow::TFactory.     
   /// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0447r21.html 
   ///                                                                        
   template<CT::Data T>
   class THive : public A::Hive {
   public:
      LANGULUS(TYPED) T;
      LANGULUS(ABSTRACT) false;

      static constexpr Count DefaultFrameSize = 8;

   protected: IF_LANGULUS_TESTING(public:)
      class Cell;
      using Frame = TAny<Cell>;

      // Elements are allocated here in frames                          
      // If resizing one frame of cells requires memory to move, then   
      // another frame will be added to the sequence, guaranteeing      
      // that memory underneath any cells never moves                   
      TAny<Frame> mFrames;
      // The start of the reusable chain, in the first frame that has   
      // a free cell                                                    
      Cell* mReusable {};
      // Number of initialized elements across all frames               
      Count mCount = 0;

   public:
      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr THive() noexcept = default;
      THive(const THive&);
      THive(THive&&);

      template<template<class> class S> requires CT::Semantic<S<THive>>
      THive(S<THive>&&);

      ~THive();

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      THive& operator = (const THive&);
      THive& operator = (THive&&);

      template<template<class> class S> requires CT::Semantic<S<THive>>
      THive& operator = (S<THive>&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() const Frame* Owns(const void*) const noexcept;
      NOD() DMeta GetType() const noexcept;
      NOD() Count GetCount() const noexcept;
      NOD() bool IsEmpty() const noexcept;
      NOD() constexpr explicit operator bool() const noexcept;

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<class>
      struct Iterator;

      NOD() constexpr Iterator<THive>       begin() noexcept;
      NOD() constexpr Iterator<THive const> begin() const noexcept;

      NOD() constexpr Iterator<THive>       last() noexcept;
      NOD() constexpr Iterator<THive const> last() const noexcept;

      constexpr A::IteratorEnd end() const noexcept { return {}; }

      template<bool REVERSE = false>
      Count ForEach(auto&&...) const;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<class...A> requires ::std::constructible_from<T, A...>
      T* New(A&&...);

   protected:
      template<class...A> requires ::std::constructible_from<T, A...>
      Cell* NewInner(A&&...);

   public:
      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      void Destroy(Cell*);

      void Reset();
   };


   ///                                                                        
   ///   Hive cell (for internal usage)                                       
   ///                                                                        
   /// Intended to be allocated only dynamically, and never instantiated on   
   /// the stack, because it may contain an instance of an uninitialized T.   
   ///                                                                        
   template<CT::Data T>
   class THive<T>::Cell {
   IF_LANGULUS_TESTING(public:)
      friend class THive<T>;

      // If zero, then this cell is in use, and mData is valid          
      // If not zero, then it points to the next free cell              
      // @attention this may point beyond frame's reserved memory, if   
      // cell is the last one                                           
      Cell* mNextFreeCell {};

      // Data reserved for T's instance                                 
      T mData;

      /// Only THive is capable of creating and destroying these cells        
      Cell() = delete;

      template<class...A> requires ::std::constructible_from<T, A...>
      Cell(A&&...);

      /// @attention after a cell is destroyed, its mNextFreeCell must be     
      /// set to the next free cell, as this informs iterators, that the cell 
      /// isn't initialized                                                   
      ~Cell() = default;
   };


   ///                                                                        
   ///   Hive iterator                                                        
   ///                                                                        
   template<CT::Data T> template<class HIVE>
   struct THive<T>::Iterator {
      static_assert(CT::Hive<HIVE>, "HIVE must be a CT::Hive type");
      static constexpr bool Mutable = CT::Mutable<HIVE>;

      LANGULUS(ABSTRACT) false;
      LANGULUS(TYPED) T;

   protected:
      friend class THive<T>;

      // Current iterator position pointer inside current frame         
      Cell* mCell;
      // Iterator position which is considered the 'end' frame iterator 
      Cell const* mCellEnd;

      // Current frame                                                  
      Frame* mFrame;
      // The frame that is considered end of frames                     
      Frame const* mFrameEnd;

      constexpr Iterator(Cell*, Cell const*, Frame*, Frame const*) noexcept;

   public:
      Iterator() noexcept = delete;
      constexpr Iterator(const Iterator&) noexcept = default;
      constexpr Iterator(Iterator&&) noexcept = default;
      constexpr Iterator(const A::IteratorEnd&) noexcept;

      constexpr Iterator& operator = (const Iterator&) noexcept = default;
      constexpr Iterator& operator = (Iterator&&) noexcept = default;

      NOD() constexpr bool operator == (const Iterator&) const noexcept;
      NOD() constexpr bool operator == (const A::IteratorEnd&) const noexcept;

      NOD() constexpr decltype(auto) operator *  () const noexcept;
      NOD() constexpr decltype(auto) operator -> () const noexcept;

      // Prefix operator                                                
      constexpr Iterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() constexpr Iterator operator ++ (int) noexcept;

      constexpr explicit operator bool() const noexcept;
      constexpr operator Iterator<const HIVE>() const noexcept requires Mutable;
   };

} // namespace Langulus::Flow
