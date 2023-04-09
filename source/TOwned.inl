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
   LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(const TOwned& value)
      : TOwned {Langulus::Copy(value)} {}

   /// Move ownership, resetting source value to default if sparse            
   ///   @param value - owned value to move                                   
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(TOwned&& value)
      : TOwned {Langulus::Move(value)} {}
   
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(const CT::NotSemantic auto& value)
      : TOwned {Copy(value)} {}

   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(CT::NotSemantic auto& value)
      : TOwned {Copy(value)} {}

   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(CT::NotSemantic auto&& value)
      : TOwned {Move(value)} {}

   TEMPLATE_OWNED()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(S&& value) {
      if constexpr (CT::Exact<TypeOf<S>, TOwned>) {
         SemanticNew<T>(&mValue, S::Nest(value.mValue.mValue));
         if constexpr (S::Move && S::Keep)
            value.mValue.mValue = {};
      }
      else SemanticNew<T>(&mValue, value.Forward());
   }

   /// Reset the value                                                        
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   void TOwned<T>::Reset() {
      mValue = {};
   }

   /// Copy-assign an owned value                                             
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (const TOwned& value) {
      return operator = (Copy(value));
   }

   /// Move-assign an owned value                                             
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (TOwned&& value) {
      return operator = (Move(value));
   }

   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (const CT::NotSemantic auto& value) {
      return operator = (Copy(value));
   }

   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (CT::NotSemantic auto& value) {
      return operator = (Copy(value));
   }

   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (CT::NotSemantic auto&& value) {
      return operator = (Move(value));
   }

   TEMPLATE_OWNED()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (S&& rhs) {
      if constexpr (CT::Exact<TypeOf<S>, TOwned>) {
         SemanticAssign(mValue, S::Nest(rhs.mValue.mValue));
         if constexpr (S::Move && S::Keep)
            rhs.mValue.mValue = {};
      }
      else SemanticAssign(mValue, rhs.Forward());

      return *this;
   }

   /// Get a reference to the contained value (const)                         
   ///   @return the contained pointer                                        
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   decltype(auto) TOwned<T>::Get() const noexcept {
      return (mValue);
   }
   
   /// Get a reference to the contained value                                 
   ///   @return the contained pointer                                        
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   decltype(auto) TOwned<T>::Get() noexcept {
      return (mValue);
   }

   /// Get the hash of the contained dense data, if hashable                  
   /// If data is incomplete or not hashable, hash the pointer instead        
   ///   @return the hash of the contained element                            
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   Hash TOwned<T>::GetHash() const {
      if constexpr (CT::Sparse<T>) {
         if (!mValue)
            return {};
      }

      if constexpr (CT::Hashable<T>)
         return DenseCast(mValue).GetHash();
      else if constexpr (CT::Sparse<T>)
         return HashData(mValue);
      else
         LANGULUS_ERROR("Contained value is not hashable");
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
   LANGULUS(INLINED)
   auto TOwned<T>::operator -> () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access mutable pointer                                                 
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained raw pointer                                    
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   auto TOwned<T>::operator -> () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access the dereferenced pointer (const)                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant reference                             
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   decltype(auto) TOwned<T>::operator * () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Access the dereferenced pointer                                        
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained mutable reference                              
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   decltype(auto) TOwned<T>::operator * () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Explicit boolean cast                                                  
   ///   @return true if value differs from default value                     
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   TOwned<T>::operator bool() const noexcept {
      return mValue != T {};
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   TOwned<T>::operator const T&() const noexcept {
      return mValue;
   }

   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   TOwned<T>::operator T&() noexcept {
      return mValue;
   }

   /// Compare pointers for equality                                          
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   bool TOwned<T>::operator == (const TOwned<T>& rhs) const noexcept {
      return mValue == rhs.mValue;
   }

   /// Compare pointers for equality                                          
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   bool TOwned<T>::operator == (const T& rhs) const noexcept {
      return mValue == rhs;
   }

   /// Compare pointer for nullptr                                            
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   bool TOwned<T>::operator == (std::nullptr_t) const noexcept requires CT::Sparse<T> {
      return mValue == nullptr;
   }

   /// Get the block of the contained value                                   
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced via a memory block                     
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   DMeta TOwned<T>::GetType() const {
      return MetaData::Of<Decay<T>>();
   }

   /// Get the block of the contained value                                   
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced via a memory block                     
   TEMPLATE_OWNED()
   LANGULUS(INLINED)
   Block TOwned<T>::GetBlock() const {
      return {
         DataState::Constrained, GetType(), 1, &mValue 
         // Notice entry is missing, which means it will be searched    
      };
   }

} // namespace Langulus::Anyness

#undef TEMPLATE_OWNED
