///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TMany.hpp"
#include "Many.inl"
#include <cctype>

#define TEMPLATE() template<CT::Data T>


namespace Langulus::Anyness
{

   /// Default construction                                                   
   /// TMany is always type-constrained, but its type is set on demand to     
   /// avoid requesting meta definitions before meta database initialization, 
   /// and to significanty improve TMany initialization time (also to allow   
   /// for constexpr default construction)                                    
   TEMPLATE() LANGULUS(ALWAYS_INLINED)
   constexpr TMany<T>::TMany() {
      if constexpr (CT::Constant<T>)
         mState = DataState::Typed | DataState::Constant;
      else
         mState = DataState::Typed;
   }

   /// Refer constructor                                                      
   ///   @param other - the TMany to reference                                
   TEMPLATE() LANGULUS(ALWAYS_INLINED)
   TMany<T>::TMany(const TMany& other)
      : TMany {Refer(other)} {}
    
   /// Move constructor                                                       
   ///   @param other - the TMany to move                                     
   TEMPLATE() LANGULUS(ALWAYS_INLINED)
   TMany<T>::TMany(TMany&& other) noexcept
      : TMany {Move(other)} {}
   
   /// Create from a list of elements, each of them can be semantic or not,   
   /// an array, as well as any other kinds of anies                          
   ///   @param t1 - first element                                            
   ///   @param tn - tail of elements (optional)                              
   TEMPLATE() template<class T1, class...TN>
   requires CT::DeepMakable<T, T1, TN...> LANGULUS(INLINED)
   TMany<T>::TMany(T1&& t1, TN&&...tn) {
      Base::BlockCreate(Forward<T1>(t1), Forward<TN>(tn)...);
   }

   /// Destructor                                                             
   TEMPLATE() LANGULUS(ALWAYS_INLINED)
   TMany<T>::~TMany() {
      Base::Free();
   }

   /// Refer assignment                                                       
   ///   @param rhs - the container to refer to                               
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator = (const TMany& rhs) {
      static_assert(CT::DeepAssignable<T, Referred<TMany<T>>>);
      return operator = (Refer(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the container to move                                   
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator = (TMany&& rhs) {
      static_assert(CT::DeepAssignable<T, Moved<TMany<T>>>);
      return operator = (Move(rhs));
   }

   /// Generic assignment                                                     
   ///   @param rhs - the element/array/container to assign                   
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepAssignable<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator = (T1&& rhs) {
      return Base::template BlockAssign<TMany>(Forward<T1>(rhs));
   }

   /// Insert an element at the back of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator << (T1&& rhs) {
      Base::Insert(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Insert an element at the front of the container                        
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator >> (T1&& rhs) {
      Base::Insert(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the back of the container                          
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator <<= (T1&& rhs) {
      Base::Merge(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the front of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator >>= (T1&& rhs) {
      Base::Merge(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Crop(Offset start, Count count) const IF_UNSAFE(noexcept) {
      return Base::template Crop<TMany>(start, count);
   }
   
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Crop(Offset start, Count count) IF_UNSAFE(noexcept) {
      return Base::template Crop<TMany>(start, count);
   }
   
   /// Extend the container via default construction, and return the new part 
   ///   @param count - the number of elements to extend by                   
   ///   @return a container that represents only the extended part           
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Extend(const Count count) {
      return Base::template Extend<TMany>(count);
   }
  
   /// Concatenate anything, semantically or not                              
   ///   @param rhs - the element/block/array to copy-concatenate             
   ///   @return a new container, containing both blocks                      
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TMany<T> TMany<T>::operator + (T1&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      return Base::template ConcatBlock<TMany>(S::Nest(rhs));
   }

   /// Concatenate destructively, semantically or not                         
   ///   @param rhs - the element/block/array to semantically concatenate     
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator += (T1&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      Base::template InsertBlock<void>(IndexBack, S::Nest(rhs));
      return *this;
   }

   /// Statically typed container can always be represented by a type-erased  
   TEMPLATE()
   TMany<T>::operator Many& () noexcept {
      // Just make sure that type member has been populated             
      (void) Base::GetType();
      return reinterpret_cast<Many&>(*this);
   }

   /// Statically typed container can always be represented by a type-erased  
   TEMPLATE()
   TMany<T>::operator const Many& () const noexcept {
      // Just make sure that type member has been populated             
      (void) Base::GetType();
      return reinterpret_cast<const Many&>(*this);
   }

} // namespace Langulus::Anyness

#undef TEMPLATE