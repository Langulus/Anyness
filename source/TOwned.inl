///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOwned.hpp"

#define TEMPLATE_OWNED() template<CT::Data T>

namespace Langulus::Anyness
{

   /// Move ownership, resetting source value to default if sparse            
   ///   @param value - owned value to move                                   
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>::TOwned(TOwned&& value)
      : TOwned {Langulus::Move(value)} {}

   /// Semantic construction of dense semantic-constructible value            
   ///   @tparam S - the semantic used for constructing the owned value       
   ///   @param value - owned value to use                                    
   TEMPLATE_OWNED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>::TOwned(S&& value) requires (CT::Dense<T> && S::template Exact<TOwned<T>> && CT::SemanticMakable<S, T>)
      : mValue {S::Nest(value.mValue.mValue)} {
      if constexpr (S::Move && S::Keep)
         value.mValue.mValue = {};
   }

   /// Semantic construction of sparse/fundamental value                      
   ///   @tparam S - the semantic used for constructing the owned value       
   ///   @param value - owned value to use                                    
   TEMPLATE_OWNED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>::TOwned(S&& value) requires ((CT::Fundamental<T> || CT::Sparse<T>) && S::template Exact<TOwned<T>>)
      : mValue {value.mValue.mValue} {
      if constexpr (S::Move && S::Keep)
         value.mValue.mValue = {};
   }

   /// Initialize with a value                                                
   ///   @param value - value to copy (no referencing shall occur if sparse)  
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>::TOwned(const T& value)
      : mValue {value} { }

   /// Reset the value                                                        
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   void TOwned<T>::Reset() {
      mValue = {};
   }

   /// Copy-assign an owned value                                             
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>& TOwned<T>::operator = (const TOwned& value) noexcept {
      return operator = (Langulus::Copy(value));
   }

   /// Move-assign an owned value                                             
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>& TOwned<T>::operator = (TOwned&& value) noexcept {
      return operator = (Langulus::Move(value));
   }

   /// Semantic-assign an owned value                                         
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>& TOwned<T>::operator = (S&& value) noexcept requires (S::template Exact<TOwned<T>>) {
      SemanticAssign(mValue, S::Nest(value.mValue.mValue));
      if constexpr (S::Move && S::Keep)
         value.mValue.mValue = {};
      return *this;
   }

   /// Copy-assign raw value                                                  
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>& TOwned<T>::operator = (const T& value) noexcept {
      return operator = (Langulus::Copy(value));
   }

   /// Move-assign raw value                                                  
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>& TOwned<T>::operator = (T&& value) noexcept {
      return operator = (Langulus::Move(value));
   }

   /// Semantic-assign a raw value                                            
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr TOwned<T>& TOwned<T>::operator = (S&& value) noexcept requires (S::template Exact<T>) {
      SemanticAssign(mValue, value.Forward());
      if constexpr (S::Move && S::Keep)
         value.mValue = {};
      return *this;
   }

   /// Get a reference to the contained value (const)                         
   ///   @return the contained pointer                                        
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   decltype(auto) TOwned<T>::Get() const noexcept {
      return mValue;
   }
   
   /// Get a reference to the contained value                                 
   ///   @return the contained pointer                                        
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   decltype(auto) TOwned<T>::Get() noexcept {
      return mValue;
   }

   /// Get the hash of the contained type                                     
   ///   @return the hash of the container type                               
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   Hash TOwned<T>::GetHash() const requires CT::Hashable<T> {
      if (!mValue)
         return {};
      return mValue->GetHash();
   }

   /// Perform a dynamic cast on the pointer                                  
   ///   @tparam D - the desired type to cast to                              
   ///   @return the result of a dynamic_cast to the specified type           
   TEMPLATE_OWNED() template<class D>
   auto TOwned<T>::As() const noexcept requires CT::Sparse<T> {
      using RESOLVED = Conditional<CT::Constant<T>, const Decay<D>*, Decay<D>*>;
      return dynamic_cast<RESOLVED>(mValue);
   }

   /// Access constant pointer                                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant raw pointer                           
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   auto TOwned<T>::operator -> () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access mutable pointer                                                 
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained raw pointer                                    
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   auto TOwned<T>::operator -> () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access the dereferenced pointer (const)                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant reference                             
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   decltype(auto) TOwned<T>::operator * () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Access the dereferenced pointer                                        
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained mutable reference                              
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   decltype(auto) TOwned<T>::operator * () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Explicit boolean cast                                                  
   ///   @return true if value differs from default value                     
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   TOwned<T>::operator bool() const noexcept {
      return mValue != T {};
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   TOwned<T>::operator const T&() const noexcept {
      return mValue;
   }

   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   TOwned<T>::operator T&() noexcept {
      return mValue;
   }

   /// Compare pointers for equality                                          
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   bool TOwned<T>::operator == (const TOwned<T>& rhs) const noexcept {
      return mValue == rhs.mValue;
   }

   /// Compare pointers for equality                                          
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   bool TOwned<T>::operator == (const T& rhs) const noexcept {
      return mValue == rhs;
   }

   /// Compare pointer for nullptr                                            
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   bool TOwned<T>::operator == (std::nullptr_t) const noexcept requires CT::Sparse<T> {
      return mValue == nullptr;
   }

   /// Get the block of the contained value                                   
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced via a memory block                     
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   DMeta TOwned<T>::GetType() const {
      return MetaData::Of<Decay<T>>();
   }

   /// Get the block of the contained value                                   
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced via a memory block                     
   TEMPLATE_OWNED()
   LANGULUS(ALWAYSINLINE)
   Block TOwned<T>::GetBlock() const {
      return {
         CT::Sparse<T> 
            ? DataState::Constrained | DataState::Sparse
            : DataState::Constrained,
         GetType(), 1, &mValue 
         // Notice entry is missing, which means it will be searched    
      };
   }

} // namespace Langulus::Anyness

#undef TEMPLATE_OWNED
