///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOwned.hpp"

#define TEMPLATE_OWNED() template<CT::Data T>

namespace Langulus::Anyness
{

   /// Shallow-copy constructor                                               
   ///   @param value - owned value to reference                              
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(const TOwned& value)
      : TOwned {Copy(value)} {}

   /// Move constructor                                                       
   ///   @param value - owned value to move                                   
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(TOwned&& value)
      : TOwned {Move(value)} {}
   
   /// Copy a value                                                           
   ///   @param value - value to copy                                         
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(const CT::NotSemantic auto& value)
      : TOwned {Copy(value)} {}

   /// Copy a value                                                           
   ///   @param value - value to copy                                         
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(CT::NotSemantic auto& value)
      : TOwned {Copy(value)} {}

   /// Move in a value                                                        
   ///   @param value - value to move                                         
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(CT::NotSemantic auto&& value)
      : TOwned {Move(value)} {}

   /// Semantic constructor                                                   
   ///   @param value - the value and semantic to use for initialization      
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>::TOwned(CT::Semantic auto&& value) {
      operator = (value.Forward());
   }

   /// Reset the value                                                        
   TEMPLATE_OWNED() LANGULUS(INLINED)
   void TOwned<T>::Reset() {
      mValue = {};
   }

   /// Shallow-copy-assignment                                                
   ///   @param value - the value to reference                                
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (const TOwned& value) {
      return operator = (Copy(value));
   }

   /// Move-assignment                                                        
   ///   @param value - the value to move                                     
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (TOwned&& value) {
      return operator = (Move(value));
   }

   /// Shallow-copy-assignment of a value                                     
   ///   @param value - the value to assign                                   
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (const CT::NotSemantic auto& value) {
      return operator = (Copy(value));
   }

   /// Shallow-copy-assignment of a value                                     
   ///   @param value - the value to assign                                   
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (CT::NotSemantic auto& value) {
      return operator = (Copy(value));
   }

   /// Move-assignment of a value                                             
   ///   @param value - the value to assign                                   
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (CT::NotSemantic auto&& value) {
      return operator = (Move(value));
   }

   /// Semantic assignment                                                    
   ///   @param rhs - the value and semantic to use for assignment            
   TEMPLATE_OWNED() LANGULUS(INLINED)
   constexpr TOwned<T>& TOwned<T>::operator = (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Exact<ST, TOwned>) {
         // Assign another TOwned                                       
         SemanticAssign(mValue, S::Nest(rhs->mValue));
         if constexpr (S::Move and S::Keep)
            rhs->Reset();
      }
      else if constexpr (CT::Sparse<T> and CT::Nullptr<ST>) {
         // Assign a nullptr (simply reset this)                        
         Reset();
      }
      else if constexpr (CT::Neat<ST>) {
         // Use a Neat to initialize the owned value                    
         TODO();
      }
      else {
         // Assign a raw value                                          
         SemanticAssign(mValue, rhs.Forward());
      }

      return *this;
   }

   /// Get a reference to the contained value (const)                         
   ///   @return the contained value reference                                
   TEMPLATE_OWNED() LANGULUS(INLINED)
   const T& TOwned<T>::Get() const noexcept {
      return mValue;
   }
   
   /// Get a reference to the contained value                                 
   ///   @return the contained value reference                                
   TEMPLATE_OWNED() LANGULUS(INLINED)
   T& TOwned<T>::Get() noexcept {
      return mValue;
   }

   /// Get the hash of the contained dense data, if hashable                  
   /// If data is incomplete or not hashable, hash the pointer instead        
   ///   @return the hash of the contained element                            
   TEMPLATE_OWNED() LANGULUS(INLINED)
   Hash TOwned<T>::GetHash() const {
      return HashOf(mValue);
   }

   /// Perform a dynamic cast on the pointer                                  
   ///   @tparam D - the desired type to cast to                              
   ///   @return the result of a dynamic_cast to the specified type           
   TEMPLATE_OWNED()
   template<class D>
   auto TOwned<T>::As() const noexcept requires CT::Sparse<T> {
      using RESOLVED = Conditional<CT::Constant<T>, const Decay<D>*, Decay<D>*>;
      return dynamic_cast<RESOLVED>(mValue);
   }

   /// Access constant pointer                                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant raw pointer                           
   TEMPLATE_OWNED() LANGULUS(INLINED)
   T TOwned<T>::operator -> () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access mutable pointer                                                 
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained raw pointer                                    
   TEMPLATE_OWNED() LANGULUS(INLINED)
   T TOwned<T>::operator -> () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access the dereferenced pointer (const)                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant reference                             
   TEMPLATE_OWNED() LANGULUS(INLINED)
   decltype(auto) TOwned<T>::operator * () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Access the dereferenced pointer                                        
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained mutable reference                              
   TEMPLATE_OWNED() LANGULUS(INLINED)
   decltype(auto) TOwned<T>::operator * () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Explicit boolean cast                                                  
   ///   @return true if value differs from default value                     
   TEMPLATE_OWNED() LANGULUS(INLINED)
   TOwned<T>::operator bool() const noexcept {
      return mValue != T {};
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE_OWNED() LANGULUS(INLINED)
   TOwned<T>::operator const T&() const noexcept {
      return mValue;
   }

   /// Cast to a mutable pointer, if mutable                                  
   ///   @return the mutable equivalent to this pointer                       
   TEMPLATE_OWNED() LANGULUS(INLINED)
   TOwned<T>::operator T&() noexcept {
      return mValue;
   }

   /// Compare raw values for equality                                        
   ///   @param rhs - the right value                                         
   ///   @return true if values match                                         
   TEMPLATE_OWNED() LANGULUS(INLINED)
   bool TOwned<T>::operator == (const T& rhs) const noexcept requires CT::Inner::Comparable<T> {
      return mValue == rhs;
   }

   /// Compare pointer for nullptr                                            
   ///   @return true contained pointer is nullptr                            
   TEMPLATE_OWNED() LANGULUS(INLINED)
   bool TOwned<T>::operator == (std::nullptr_t) const noexcept requires CT::Sparse<T> {
      return mValue == nullptr;
   }

   /// Get the type of the contained value                                    
   /// Can be invoked by the reflected resolver                               
   ///   @return the type of the contained value                              
   TEMPLATE_OWNED() LANGULUS(INLINED)
   DMeta TOwned<T>::GetType() const {
      return MetaData::Of<Decay<T>>();
   }

   /// Get a block representation of the contained value                      
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced by a static memory block               
   TEMPLATE_OWNED() LANGULUS(INLINED)
   Block TOwned<T>::GetBlock() const {
      return {
         DataState::Constrained, GetType(), 1, &mValue 
         // Notice entry is missing, which means it will be searched    
      };
   }

} // namespace Langulus::Anyness

#undef TEMPLATE_OWNED
