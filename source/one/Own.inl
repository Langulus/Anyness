///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Own.hpp"

#define TEMPLATE()   template<CT::Data T>
#define TME()        Own<T>


namespace Langulus::Anyness
{   

   /// Get handle representation of the contained data                        
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::GetHandle() const {
      const auto mthis = const_cast<TME()*>(this);
      // Notice entry is missing, which means it will be sought         
      return Handle {mthis->mValue};
   }

   /// Default constructor                                                    
   ///   @param value - owned value to reference                              
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::Own() requires CT::Defaultable<T>
      : mValue {} {}

   /// Refer constructor                                                      
   ///   @param value - owned value to reference                              
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::Own(const Own& value)
   requires (CT::Sparse<T> or CT::ReferMakable<T>)
      : Own {Refer(value)} {}

   /// Move constructor                                                       
   ///   @param value - owned value to move                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::Own(Own&& value)
   requires (CT::Sparse<T> or CT::MoveMakable<T>)
      : Own {Move(value)} {}
   
   /// Generic constructor                                                    
   TEMPLATE() template<template<class> class S>
   requires CT::IntentMakable<S, T> LANGULUS(INLINED)
   constexpr TME()::Own(S<Own>&& other)
      : mValue {S<Own>::Nest(other->mValue)} {}

   /// Argument forwarding constructor                                        
   TEMPLATE() template<CT::NotOwned...A>
   requires ::std::constructible_from<T, A...> LANGULUS(INLINED)
   constexpr TME()::Own(A&&...args)
      : mValue {Forward<A>(args)...} {}

   /// Refer assignment                                                       
   ///   @param value - the value to reference                                
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (const Own& value)
   requires (CT::Sparse<T> or CT::ReferAssignable<T>) {
      return operator = (Refer(value));
   }

   /// Move assignment                                                        
   ///   @param value - the value to move                                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (Own&& value)
   requires (CT::Sparse<T> or CT::MoveAssignable<T>) {
      return operator = (Move(value));
   }
   
   /// Generic assignment                                                     
   TEMPLATE() template<template<class> class S>
   requires CT::IntentAssignable<S, T> LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (S<Own>&& rhs) {
      mValue = S<Own>::Nest(rhs->mValue);
      return *this;
   }

   /// Argument forwarding assignment                                         
   TEMPLATE() template<CT::NotOwned A>
   requires CT::AssignableFrom<T, A> LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (A&& rhs) {
      mValue = Forward<A>(rhs);
      return *this;
   }

   /// Get the type of the contained value                                    
   /// Can be invoked by the reflected resolver                               
   ///   @return the type of the contained value                              
   TEMPLATE() LANGULUS(INLINED)
   DMeta TME()::GetType() const {
      return MetaDataOf<T>();
   }

   /// Get a block representation of the contained value                      
   /// Can be invoked by the reflected resolver                               
   ///   @attention TOwned doesn't keep track of memory entries, so getting   
   ///      the block will have some memory search overhead                   
   ///   @return the value, interfaced by a static memory block               
   TEMPLATE() LANGULUS(INLINED)
   constexpr Block<T> TME()::GetBlock() const {
      return {
         DataState::Typed, GetType(), 1, &mValue
         // Notice entry is missing, which means it will be sought if   
         // transferred to a block with ownership                       
      };
   }

   /// Get a reference to the contained value                                 
   ///   @return the contained value reference                                
   TEMPLATE() LANGULUS(INLINED)
   constexpr const T& TME()::Get() const noexcept {
      return mValue;
   }
   
   TEMPLATE() LANGULUS(INLINED)
   constexpr T& TME()::Get() noexcept {
      return mValue;
   }

   /// Get the hash of the contained data, if hashable                        
   ///   @return the hash of the contained element                            
   TEMPLATE() LANGULUS(INLINED)
   Hash TME()::GetHash() const requires CT::Hashable<T> {
      return HashOf(mValue);
   }

   /// Perform a dynamic cast on the pointer                                  
   ///   @tparam D - the desired type to cast to (a pointer will be added)    
   ///   @return the result of a dynamic_cast to the specified type           
   TEMPLATE() template<class D> LANGULUS(INLINED)
   auto TME()::As() const noexcept requires CT::Sparse<T> {
      if constexpr (CT::Constant<T>)
         return dynamic_cast<const D*>(mValue);
      else
         return dynamic_cast<D*>(mValue);
   }

   /// Access constant pointer                                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant raw pointer                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr auto TME()::operator -> () const {
      if constexpr (CT::Sparse<T>)
         return mValue;
      else
         return &mValue;
   }

   /// Access mutable pointer                                                 
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained raw pointer                                    
   TEMPLATE() LANGULUS(INLINED)
   constexpr auto TME()::operator -> () {
      if constexpr (CT::Sparse<T>)
         return mValue;
      else
         return &mValue;
   }

   /// Access the dereferenced pointer (const)                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant reference                             
   TEMPLATE() LANGULUS(INLINED)
   constexpr auto& TME()::operator * () const IF_UNSAFE(noexcept)
   requires (CT::Sparse<T> and not CT::Void<Decay<T>>) {
      LANGULUS_ASSUME(UserAssumes, mValue, "Dereferening null pointer");
      return *mValue;
   }

   /// Access the dereferenced pointer                                        
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained mutable reference                              
   TEMPLATE() LANGULUS(INLINED)
   constexpr auto& TME()::operator * () IF_UNSAFE(noexcept)
   requires (CT::Sparse<T> and not CT::Void<Decay<T>>) {
      LANGULUS_ASSUME(UserAssumes, mValue, "Dereferening null pointer");
      return *mValue;
   }

   /// Explicit boolean cast                                                  
   ///   @return true if value differs from default value                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::operator bool() const noexcept {
      return mValue != T {};
   }

   /// Cast to a pointer, if mutable                                          
   ///   @return the pointer                                                  
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::operator T&() const noexcept {
      return const_cast<T&>(mValue);
   }

   /// Reset the value                                                        
   TEMPLATE() LANGULUS(INLINED)
      void TME()::Reset() {
      mValue = {};
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TME