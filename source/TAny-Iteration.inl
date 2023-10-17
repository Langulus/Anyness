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

#define TEMPLATE() template<CT::Data T>
#define ITERATOR() typename TAny<T>::template TIterator<MUTABLE>
#define ITERATOR_END() typename TAny<T>::template TIteratorEnd<MUTABLE>


namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   TAny iterator                                                        
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param e - element                                                   
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr TAny<T>::TIterator<MUTABLE>::TIterator(const T* e) noexcept
      : mElement {e} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr ITERATOR()& TAny<T>::TIterator<MUTABLE>::operator ++ () noexcept {
      ++mElement;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr ITERATOR() TAny<T>::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare two iterators                                                  
   ///   @param rhs - the other iterator                                      
   ///   @return true if iterators point to the same element                  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr bool TAny<T>::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mElement == rhs.mElement;
   }

   /// Compare iterator with an end marker                                    
   ///   @param rhs - the end iterator                                        
   ///   @return true element is at or beyond the end marker                  
   TEMPLATE()
   template<bool MUTABLE>
   template<bool RHS_MUTABLE>
   LANGULUS(INLINED)
   constexpr bool TAny<T>::TIterator<MUTABLE>::operator == (const TIteratorEnd<RHS_MUTABLE>& rhs) const noexcept {
      return mElement >= rhs.mEndMarker;
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   TAny<T>::TIterator<MUTABLE>::operator T& () const noexcept requires (MUTABLE) {
      return const_cast<T&>(*mElement);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   TAny<T>::TIterator<MUTABLE>::operator const T& () const noexcept requires (!MUTABLE) {
      return *mElement;
   }
   
   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   T& TAny<T>::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return const_cast<T&>(*mElement);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   const T& TAny<T>::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return *mElement;
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   T& TAny<T>::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return const_cast<T&>(*mElement);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   const T& TAny<T>::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return *mElement;
   }
   

   ///                                                                        
   ///   TAny end iterator                                                    
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param end - pointer to the 'end' element                            
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr TAny<T>::TIteratorEnd<MUTABLE>::TIteratorEnd(const T* end) noexcept
      : mEndMarker {end} {}

   /// Prefix increment operator, does nothing                                
   /*TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr ITERATOR_END()& TAny<T>::TIteratorEnd<MUTABLE>::operator ++ () const noexcept {
      // Does nothing intentionally, it's an end iterator               
      return *this;
   }

   /// Suffix increment operator, does nothing                                
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr ITERATOR_END() TAny<T>::TIteratorEnd<MUTABLE>::operator ++ (int) const noexcept {
      // Does nothing intentionally, it's an end iterator               
      return *this;
   }*/

   /// Compare iterators                                                      
   ///   @param rhs - the other iterator                                      
   ///   @return true if rhs is at or beyond the end marker                   
   TEMPLATE()
   template<bool MUTABLE>
   template<bool RHS_MUTABLE>
   LANGULUS(INLINED)
   constexpr bool TAny<T>::TIteratorEnd<MUTABLE>::operator == (const TIterator<RHS_MUTABLE>& rhs) const noexcept {
      return rhs.mElement >= mEndMarker;
   }

   /// Compare end with an end marker, always returns true                    
   ///   @return always true                                                  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   constexpr bool TAny<T>::TIteratorEnd<MUTABLE>::operator == (const TIteratorEnd&) const noexcept {
      return true;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef ITERATOR
#undef ITERATOR_END