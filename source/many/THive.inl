///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "THive.hpp"
#include "TMany.inl"

#define TEMPLATE()   template<CT::Data T>
#define TME()        THive<T>


namespace Langulus::Anyness
{

   /// Refer-constructor                                                      
   ///   @param other - the hive to refer to                                  
   TEMPLATE() LANGULUS(INLINED)
   TME()::THive(const THive& other)
      : THive {Refer(other)} {}

   /// Move-constructor                                                       
   ///   @param other - the hive to move                                      
   TEMPLATE() LANGULUS(INLINED)
   TME()::THive(THive&& other)
      : THive {Move(other)} {}

   /// Intent constructor                                                     
   ///   @param other - the hive and intent to use                            
   TEMPLATE() template<template<class> class S>
   requires CT::Intent<S<THive<T>>> LANGULUS(INLINED)
   THive<T>::THive(S<THive>&&) {
      TODO();
   }

   /// Hive destructor                                                        
   TEMPLATE() LANGULUS(INLINED)
   TME()::~THive() {
      static_assert(CT::Complete<T>,     "T must be a complete type");
      static_assert(CT::Dense<T>,        "T must be a dense type");
      static_assert(CT::Data<T>,         "T can't be void");
      static_assert(CT::Referencable<T>, "T must be referencable");
      static_assert(not CT::Abstract<T>, "T can't be abstract");
      Reset();
   }

   /// Refer assignment                                                       
   ///   @param other - the hive to refer to                                  
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::operator = (const THive& other) -> THive& {
      return operator = (Refer(other));
   }

   /// Move assignment                                                        
   ///   @param other - the hive to move                                      
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::operator = (THive&& other) -> THive& {
      return operator = (Move(other));
   }

   /// Intent assignment                                                      
   ///   @param other - the hive to assign                                    
   TEMPLATE() template<template<class> class S> 
   requires CT::Intent<S<THive<T>>> LANGULUS(INLINED)
   auto THive<T>::operator = (S<THive>&& other) -> THive& {
      mFrames = other.Nest(other->mFrames);
      mReusable = other->mReusable;
      mCount = other->mCount;

      if constexpr (ResetsOnMove<S>()) {
         other->mCount = 0;
         other->mReusable = nullptr;
      }
      return *this;
   }

   /// Reset the factory                                                      
   TEMPLATE() LANGULUS(INLINED)
   bool TME()::IsEmpty() const noexcept {
      return mCount == 0;
   }

   /// Explicit bool cast operator, for use in if statements                  
   ///   @return true if block contains at least one valid element            
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::operator bool() const noexcept {
      return not IsEmpty();
   }

   /// Returns a valid pointer to the frame that owns the memory pointer, if  
   /// that memory pointer is at all owned by this hive                       
   ///   @param ptr - the pointer to check                                    
   ///   @return nullptr if not found, or a pointer to the owning frame       
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::Owns(const void* ptr) const noexcept -> const Frame* {
      for (auto& frame : mFrames)
         if (frame.Owns(ptr))
            return &frame;
      return nullptr;
   }

   /// Get the type of the contained data                                     
   ///   @return the meta data                                                
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::GetType() const noexcept -> DMeta {
      return MetaDataOf<T>();
   }

   /// Get the number of initialized cells                                    
   ///   @return the number of valid entries                                  
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::GetCount() const noexcept -> Count {
      return mCount;
   }

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   constexpr auto THive<T>::begin() noexcept -> Iterator<THive> {
      if (IsEmpty())
         return end();

      auto& firstFrame = mFrames[0];
      Cell* cell = firstFrame.GetRaw();
      Cell const* const cellEnd = cell + firstFrame.GetReserved();
      while (cell->mNextFreeCell and cell < cellEnd)
         ++cell;

      return {
         cell, cellEnd,
         &firstFrame, &firstFrame + mFrames.GetCount() - 1
      };
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr auto THive<T>::begin() const noexcept -> Iterator<const THive> {
      return const_cast<THive*>(this)->begin();
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   constexpr typename THive<T>::template Iterator<THive<T>> THive<T>::last() noexcept {
      if (IsEmpty())
         return end();

      auto& lastFrame = mFrames.Last();
      auto cell = lastFrame.GetRaw() + (lastFrame.GetReserved() - 1);
      const auto cellEnd = lastFrame.GetRaw();
      while (cell->mNextFreeCell and cell >= cellEnd)
         --cell;

      return {
         cell, cell + lastFrame.GetReserved(),
         &lastFrame, &lastFrame
      };
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr auto THive<T>::last() const noexcept -> Iterator<const THive> {
      return const_cast<THive*>(this)->last();
   }

   /// Emplace a new instance inside the hive                                 
   ///   @param args... - arguments to forward to T's constructor             
   ///   @return a pointer to the newly constructed instance of T             
   TEMPLATE()
   template<class...A> requires ::std::constructible_from<T, A...>
   auto THive<T>::New(A&&...args) -> T* {
      return &(NewInner(Forward<A>(args)...)->mData);
   }

   /// Emplace a new instance inside the hive                                 
   ///   @param args... - arguments to forward to T's constructor             
   ///   @return a pointer to the newly constructed instance of T             
   TEMPLATE()
   template<class...A> requires ::std::constructible_from<T, A...>
   auto THive<T>::NewInner(A&&...args) -> Cell* {
      Cell* result;

      if (mReusable) {
         // Reuse a slot                                                
         const auto nextReusable = mReusable->mNextFreeCell;
         try { result = new (mReusable) Cell {Forward<A>(args)...}; }
         catch (...) { return nullptr; }

         mReusable = nextReusable;

         // Make sure that the mReusable is inside limits, as it may    
         // go out of bounds in edge cases                              
         auto frame = Owns(mReusable);
         if (not frame)
            mReusable = nullptr;
         else
            ++const_cast<Frame*>(frame)->mCount;
      }
      else {
         // Add new frame                                               
         const auto nextReserved = not mFrames.IsEmpty()
            ? mFrames.Last().GetReserved() * 2
            : DefaultFrameSize;

         // Use first cell to initialize our object                     
         Frame* frame = nullptr;
         try {
            mFrames.New(1);
            frame = &mFrames.Last();
            frame->Reserve(nextReserved);

            result = new (frame->GetRaw()) Cell {Forward<A>(args)...};
         }
         catch (...) {
            // Pass through all new unused cells, and set their markers 
            // We allocated a new frame, let's not let it go to waste   
            mReusable = frame->GetRaw();
            const auto cellEnd = frame->GetRaw() + frame->GetReserved();
            for (auto cell = mReusable; cell < cellEnd; ++cell)
               cell->mNextFreeCell = cell + 1;
            return nullptr;
         }

         ++frame->mCount;

         // Pass through all new unused cells, and set their markers    
         mReusable = frame->GetRaw() + 1;
         const auto cellEnd = frame->GetRaw() + frame->GetReserved();
         for (auto cell = mReusable; cell < cellEnd; ++cell)
            cell->mNextFreeCell = cell + 1;
      }

      ++mCount;
      return result;
   }

   /// Destroys a valid cell from the hive                                    
   ///   @attention item pointer is no longer valid after this call           
   ///   @attention assumes that Cell is initialized                          
   ///   @attention assumes that Cell is owned by the hive                    
   ///   @param cell - cell to destroy                                        
   TEMPLATE()
   void THive<T>::Destroy(Cell* cell) {
      LANGULUS_ASSUME(DevAssumes, cell,
         "Pointer is not valid");
      LANGULUS_ASSUME(DevAssumes, Owns(cell),
         "Pointer is not valid");
      LANGULUS_ASSUME(DevAssumes, not cell->mNextFreeCell,
         "Cell is not initialized");

      // Destroy the cell, if owned by this hive                        
      cell->~Cell();
      cell->mNextFreeCell = mReusable;
      mReusable = cell;
      --mCount;
   }
   
   /// Reset the factory                                                      
   TEMPLATE() LANGULUS(INLINED)
   void TME()::Reset() {
      for (auto& frame : mFrames) {
         if (frame.IsEmpty())
            continue;

         auto raw = frame.GetRaw();
         const auto rawEnd = frame.GetRaw() + frame.GetReserved();
         while (raw != rawEnd and frame.mCount) {
            if (not raw->mNextFreeCell) {
               if (raw->mData.Reference(-1) == 0) {
                  // Safe to destroy the instance from here             
                  // Otherwise hive cell is in use somewhere else. It   
                  // will continue to live as a Ref<T> somewhere, until 
                  // the handle's destructor is called, and the last    
                  // reference is released                              
                  LANGULUS_ASSUME(DevAssumes, frame.GetUses() >= 1,
                     "A populated cell must have references");
                  raw->~Cell();
               }

               --frame.mCount;
            }

            ++raw;
         }

         LANGULUS_ASSUME(DevAssumes, frame.mCount == 0,
            "Frame should be empty at this point");
      }

      mFrames.Reset();
      mReusable = nullptr;
      mCount = 0;
   }

   /// Reference the factory and detach all elements if fully dereferenced    
   /// Used as a first-stage destruction to solve circular references         
   TEMPLATE() LANGULUS(INLINED)
   Count TME()::Reference(int x) {
      LANGULUS_ASSUME(DevAssumes, mReferences or x == 0,
         "Dead instance resurrection/overkill");
      LANGULUS_ASSUME(DevAssumes, x >= 0 or mReferences >= static_cast<Count>(-x),
         "Live instance overkill");
      mReferences += x;
      if (mReferences)
         return mReferences;

      // If reached, then it's time to detach everything                
      for (auto& frame : mFrames) {
         if (frame.IsEmpty())
            continue;

         auto counter = frame.mCount;
         auto raw = frame.GetRaw();
         const auto rawEnd = frame.GetRaw() + frame.GetReserved();
         while (raw != rawEnd and counter) {
            if (not raw->mNextFreeCell) {
               // Even if the cell is still referenced, it no longer    
               // is part of the factory and should be detached.        
               // Detachment is like pre-destruction, where all ties    
               // in a hierarchical element are severed, so that any    
               // circular dependencies are accounted for.              
               raw->mData.Reference(x + 1);
               --counter;
            }

            ++raw;
         }

         LANGULUS_ASSUME(DevAssumes, counter == 0,
            "Frame should be empty at this point");
      }

      return 0;
   }

   /// Construct an iterator                                                  
   ///   @param start - the current iterator position                         
   ///   @param end - the ending marker                                       
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr THive<T>::Iterator<HIVE>::Iterator(
      Cell* start, Cell const* end, Frame* startf, Frame const* lastf
   ) noexcept
      : mCell      {start}
      , mCellEnd   {end}
      , mFrame     {startf}
      , mFrameLast {lastf} {}

   /// Construct an end iterator                                              
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr THive<T>::Iterator<HIVE>::Iterator(const A::IteratorEnd&) noexcept
      : mCell      {nullptr}
      , mCellEnd   {nullptr}
      , mFrame     {nullptr}
      , mFrameLast {nullptr} {}

   /// Compare two iterators                                                  
   ///   @param rhs - the other iterator                                      
   ///   @return true if iterators point to the same element                  
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr bool THive<T>::Iterator<HIVE>::operator == (const Iterator& rhs) const noexcept {
      return mCell == rhs.mCell;
   }

   /// Compare iterator with an end marker                                    
   ///   @param rhs - the end iterator                                        
   ///   @return true element is at or beyond the end marker                  
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr bool THive<T>::Iterator<HIVE>::operator == (const A::IteratorEnd&) const noexcept {
      return mCell ? mCell >= mFrameLast->GetRawEnd() : true;
   }
   
   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr decltype(auto) THive<T>::Iterator<HIVE>::operator * () const noexcept {
      return (mCell->mData);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr decltype(auto) THive<T>::Iterator<HIVE>::operator -> () const noexcept {
      return &(mCell->mData);
   }

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr auto THive<T>::Iterator<HIVE>::operator ++ () noexcept -> Iterator& {
      ++mCell;

      // Skip uninitialized cells                                       
      while (mCell->mNextFreeCell and mCell < mCellEnd)
         ++mCell;

      if (mCell >= mCellEnd) {
         // If end of frame was reached, move to the next frame         
         ++mFrame;

         if (mFrame <= mFrameLast) {
            mCell = mFrame->GetRaw();
            mCellEnd = mCell + mFrame->GetReserved();
         }
         else mCell = nullptr;
      }

      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr auto THive<T>::Iterator<HIVE>::operator ++ (int) noexcept -> Iterator {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Check if iterator is valid                                             
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr THive<T>::Iterator<HIVE>::operator bool() const noexcept {
      return *this != A::IteratorEnd {};
   }

   /// Implicitly convert to a constant iterator                              
   TEMPLATE() template<class HIVE> LANGULUS(INLINED)
   constexpr THive<T>::Iterator<HIVE>::operator Iterator<const HIVE>() const noexcept requires Mutable {
      return {mCell, mCellEnd, mFrame, mFrameLast};
   }

} // namespace Langulus::Flow

#undef TEMPLATE
#undef TME
