///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TEdit.hpp"

#define TEMPLATE() template<DenseTypedBlock T>

namespace Langulus::Anyness
{

   /// Create an editor interface                                             
   ///   @param container - what container are we editing?                    
   ///   @param start - the starting marker of the selection (optional)       
   ///   @param end - the ending marker of the selection (optional)           
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   TEdit<T>::TEdit(T& container, Offset start, Offset end) noexcept
      : mSource(container)
      , mStart(start)
      , mEnd(end) {}

   /// Get the container we're editing                                        
   ///   @return a constant reference to the source container                 
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   const T& TEdit<T>::GetSource() const noexcept {
      return *mText;
   }

   /// Get the start of the selection                                         
   ///   @return the start of the selection                                   
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   Offset TEdit<T>::GetStart() const noexcept {
      return mStart;
   }

   /// Get the end of the selection                                           
   ///   @return the end of the selection                                     
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   Offset TEdit<T>::GetEnd() const noexcept {
      return mEnd;
   }

   /// Get the size of the selection                                          
   ///   @return the size of the selection                                    
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   Count TEdit<T>::GetLength() const noexcept {
      return mEnd - mStart;
   }

   /// Access an element at a given index, relative to the selection (const)  
   ///   @param index - the index to get                                      
   ///   @return a reference to the element at that index                     
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   auto& TEdit<T>::operator[] (Offset index) const noexcept {
      return mSource[mStart + index];
   }

   /// Access an element at a given index, relative to the selection          
   ///   @param index - the index to get                                      
   ///   @return a reference to the element at that index                     
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   auto& TEdit<T>::operator[] (Offset index) noexcept {
      return mSource[mStart + index];
   }

   /// Concatenate at the end of the selection                                
   ///   @param other - the container to concatenate                          
   ///   @return a reference to the editor for chaining                       
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::operator << (const T& other) {
      mSource.InsertBlockAt(other, mEnd);
      return *this;
   }

   /// Concatenate at the start of the selection                              
   ///   @param other - the container to concatenate                          
   ///   @return a reference to the editor for chaining                       
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::operator >> (const T& other) {
      const auto concatenated = mSource.InsertBlockAt(other, mStart);
      mStart += concatenated;
      mEnd += concatenated;
      return *this;
   }

   /// Replace the selection                                                  
   /// After replacement, the selection will collapse to the end of the       
   /// replacement                                                            
   ///   @param other - the container to use for replacement                  
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::Replace(const T& other) {
      if constexpr (CT::POD<MemberType> || CT::Sparse<MemberType>) {
         const auto offset = mStart * mSource.GetStride();

         // Optimization for POD/sparse containers that spares the      
         // execution of move constructors of all elements              
         if (other.GetCount() > GetLength()) {
            // Replacement is bigger, we need a bit more space          
            const auto surplus = (other.GetCount() + mStart) - mEnd;
            mSource.AllocateMore<false, true>(mSource.GetCount() + surplus);

            const auto surplusBytes = surplus * mSource.GetStride();
            ::std::memmove(
               mSource.GetRaw() + offset + surplusBytes,
               mSource.GetRaw() + offset,
               mSource.GetByteSize() - offset - surplusBytes
            );
         }
         else if (other.GetCount() < GetLength()) {
            // Replacement is smaller, move data backwards              
            const auto excess = GetLength() - other.GetCount();
            mSource.RemoveIndexAt(mStart + other.GetCount(), excess);
         }

         // Copy new data over the old one                              
         ::std::memcpy(
            mSource.GetRaw() + offset,
            other.GetRaw(), 
            other.GetByteSize()
         );
      }
      else {
         // Generalized, but safe replacement                           
         if (GetLength()) {
            //TODO can be optimized further, by avoiding the redundant move
            mSource.RemoveIndex(mStart, GetLength());
            mSource.InsertBlockAt(other, mStart);
         }
      }

      // Move marker and collapse selection                             
      mStart += other.GetCount();
      mEnd = mStart;
      return *this;
   }

   TEMPLATE()
   template<class K>
   LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::operator << (const K& other) requires (!IsText<K>) {
      if (!mText)
         return *this;

      if constexpr (Character<K> && Dense<K>)
         return operator << (Text(&other, 1));
      else if constexpr (ConstructibleWith<Text, K>)
         return operator << (Text(other));
      else {
         // Finally, attempt converting                                 
         Text converted;
         if (TConverter<K, Text>::Convert(other, converted) > 0)
            return operator << (converted);
         return *this;
      }
   }


   /// String concatenation                                                   
   /// Appends to the front of the selection                                  
   TEMPLATE()
   template<class K>
   LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::operator >> (const K& other) requires (!IsText<K>) {
      if (!mText)
         return *this;

      if constexpr (Character<K> && Dense<K>)
         return operator >> (Text(&other, 1));
      else if constexpr (ConstructibleWith<Text, K>)
         return operator >> (Text(other));
      else {
         // Finally, attempt converting                                 
         Text converted;
         if (TConverter<K, Text>::Convert(other, converted) > 0)
            return operator >> (converted);
         return *this;
      }
   }


   /// Replace selection (selection will collapse)                            
   /// Collapsed selection means mStart == mEnd                               
   TEMPLATE()
   template<class K>
   LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::Replace(const K& other) requires (!IsText<K>) {
      if (!mText)
         return *this;

      if constexpr (Character<K> && Dense<K>)
         return Replace(Text(&other, 1));
      else if constexpr (ConstructibleWith<Text, K>)
         return Replace(Text(other));
      else {
         // Finally, attempt converting                                 
         Text converted;
         if (TConverter<K, Text>::Convert(other, converted) > 0)
            return Replace(converted);
         return *this;
      }
   }

   /// Delete selection (selection will collapse),                            
   /// or delete symbol after collapsed selection marker                      
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::Delete() {
      if (mStart != mEnd) {
         mText->Remove(mStart, mEnd);
         mEnd = mStart;
      }
      else if (mStart < mText->mCount) {
         mText->Remove(mStart, mStart + 1);
      }
      return *this;
   }

   /// Delete selection (with collapse), or delete symbol                     
   /// before a collapsed selection marker                                    
   TEMPLATE() LANGULUS(ALWAYSINLINE)
   TEdit<T>& TEdit<T>::Backspace() {
      if (mStart != mEnd) {
         mText->Remove(mStart, mEnd);
         mEnd = mStart;
      }
      else if (mStart > 0 && mText->mCount > 0) {
         mText->Remove(mStart, mStart - 1);
         --mStart;
         mEnd = mStart;
      }
      return *this;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE