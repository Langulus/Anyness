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

#define TEMPLATE() template<CT::Data T>
#define TME() TOwned<T>


namespace Langulus::Anyness
{

   /// Shallow-copy constructor                                               
   ///   @param value - owned value to reference                              
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TOwned(const TOwned& value)
      : TOwned {Copy(value)} {}

   /// Move constructor                                                       
   ///   @param value - owned value to move                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TOwned(TOwned&& value)
      : TOwned {Move(value)} {}
   
   /// Copy a value                                                           
   ///   @param value - value to copy                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TOwned(const CT::NotSemantic auto& value)
      : TOwned {Copy(value)} {}

   /// Copy a value                                                           
   ///   @param value - value to copy                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TOwned(CT::NotSemantic auto& value)
      : TOwned {Copy(value)} {}

   /// Move in a value                                                        
   ///   @param value - value to move                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TOwned(CT::NotSemantic auto&& value)
      : TOwned {Move(value)} {}

   /// General semantic construction                                          
   ///   @param other - the value & semantic to use for initialization        
   TEMPLATE() LANGULUS(INLINED)
   void TME()::ConstructFrom(CT::Semantic auto&& other) {
      static_assert(CT::NotOwned<T>, "Can't nest owned types");
      using S = Decay<decltype(other)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Sparse<T> and CT::Nullptr<ST>) {
         // Assign a nullptr (just rely on default constructor)         
         return;
      }
      else if constexpr (CT::Owned<ST>) {
         // Move/Abandon/Disown/Copy/Clone another TOwned               
         if constexpr (CT::Dense<T> and CT::Destroyable<T>)
            mValue.~T();
         SemanticNew(&mValue, S::Nest(other->mValue));
         
         if constexpr (S::Move and S::Keep)
            other->mValue = {};
      }
      else {
         // Move/Abandon/Disown/Copy/Clone value                        
         if constexpr (CT::Dense<T> and CT::Destroyable<T>)
            mValue.~T();
         SemanticNew(&mValue, other.Forward());
      }
   }

   /// Shallow semantic construction                                          
   ///   @param other - the value & semantic to use for initialization        
   TEMPLATE() LANGULUS(INLINED)
   TME()::TOwned(CT::ShallowSemantic auto&& other) {
      ConstructFrom(other.Forward());
   }

   /// Deep semantic construction                                             
   ///   @param other - the value & semantic to use for initialization        
   TEMPLATE() LANGULUS(INLINED)
   TME()::TOwned(CT::DeepSemantic auto&& other) requires CT::CloneMakable<T> {
      ConstructFrom(other.Forward());
   }

   /// Reset the value                                                        
   TEMPLATE() LANGULUS(INLINED)
   void TME()::Reset() {
      mValue = {};
   }

   /// Shallow-copy-assignment                                                
   ///   @param value - the value to reference                                
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (const TOwned& value) {
      return operator = (Copy(value));
   }

   /// Move-assignment                                                        
   ///   @param value - the value to move                                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (TOwned&& value) {
      return operator = (Move(value));
   }

   /// Shallow-copy-assignment of a value                                     
   ///   @param value - the value to assign                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (const CT::NotSemantic auto& value) {
      return operator = (Copy(value));
   }

   /// Shallow-copy-assignment of a value                                     
   ///   @param value - the value to assign                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (CT::NotSemantic auto& value) {
      return operator = (Copy(value));
   }

   /// Move-assignment of a value                                             
   ///   @param value - the value to assign                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (CT::NotSemantic auto&& value) {
      return operator = (Move(value));
   }

   /// Semantic assignment                                                    
   ///   @param rhs - the value and semantic to use for assignment            
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::AssignFrom(CT::Semantic auto&& rhs) {
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
      else {
         // Assign a raw value                                          
         SemanticAssign(mValue, rhs.Forward());
      }

      return *this;
   }

   /// Semantically assign from any pointer/shared pointer/nullptr/related    
   ///   @param rhs - the value and semantic to assign                        
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (CT::ShallowSemantic auto&& rhs) {
      return AssignFrom(rhs.Forward());
   }
   
   /// Semantically assign from any pointer/shared pointer/nullptr/related    
   ///   @param rhs - the value and semantic to assign                        
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (CT::DeepSemantic auto&& rhs) requires CT::CloneAssignable<T> {
      return AssignFrom(rhs.Forward());
   }

   /// Get a reference to the contained value (const)                         
   ///   @return the contained value reference                                
   TEMPLATE() LANGULUS(INLINED)
   const T& TME()::Get() const noexcept {
      return mValue;
   }
   
   /// Get a reference to the contained value                                 
   ///   @return the contained value reference                                
   TEMPLATE() LANGULUS(INLINED)
   T& TME()::Get() noexcept {
      return mValue;
   }

   /// Get the hash of the contained dense data, if hashable                  
   /// If data is incomplete or not hashable, hash the pointer instead        
   ///   @return the hash of the contained element                            
   TEMPLATE() LANGULUS(INLINED)
   Hash TME()::GetHash() const requires CT::Hashable<T> {
      return HashOf(mValue);
   }

   /// Perform a dynamic cast on the pointer                                  
   ///   @tparam D - the desired type to cast to                              
   ///   @return the result of a dynamic_cast to the specified type           
   TEMPLATE()
   template<class D>
   auto TME()::As() const noexcept requires CT::Sparse<T> {
      using RESOLVED = Conditional<CT::Constant<T>, const Decay<D>*, Decay<D>*>;
      return dynamic_cast<RESOLVED>(mValue);
   }

   /// Access constant pointer                                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant raw pointer                           
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::operator -> () const {
      if constexpr (CT::Sparse<T>)
         return mValue;
      else
         return &mValue;
   }

   /// Access mutable pointer                                                 
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained raw pointer                                    
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::operator -> () {
      if constexpr (CT::Sparse<T>)
         return mValue;
      else
         return &mValue;
   }

   /// Access the dereferenced pointer (const)                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant reference                             
   TEMPLATE() LANGULUS(INLINED)
   auto& TME()::operator * () const IF_UNSAFE(noexcept)
   requires (CT::Sparse<T> and not CT::Void<Decay<T>>) {
      LANGULUS_ASSUME(UserAssumes, mValue, "Dereferening null pointer");
      return *mValue;
   }

   /// Access the dereferenced pointer                                        
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained mutable reference                              
   TEMPLATE() LANGULUS(INLINED)
   auto& TME()::operator * () IF_UNSAFE(noexcept)
   requires (CT::Sparse<T> and not CT::Void<Decay<T>>) {
      LANGULUS_ASSUME(UserAssumes, mValue, "Dereferening null pointer");
      return *mValue;
   }

   /// Explicit boolean cast                                                  
   ///   @return true if value differs from default value                     
   TEMPLATE() LANGULUS(INLINED)
   TME()::operator bool() const noexcept {
      return mValue != T {};
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE() LANGULUS(INLINED)
   TME()::operator const T&() const noexcept {
      return mValue;
   }

   /// Cast to a mutable pointer, if mutable                                  
   ///   @return the mutable equivalent to this pointer                       
   TEMPLATE() LANGULUS(INLINED)
   TME()::operator T&() noexcept {
      return mValue;
   }

   /// Get the type of the contained value                                    
   /// Can be invoked by the reflected resolver                               
   ///   @return the type of the contained value                              
   TEMPLATE() LANGULUS(INLINED)
   DMeta TME()::GetType() const {
      return MetaData::Of<Decay<T>>();
   }

   /// Get a block representation of the contained value                      
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced by a static memory block               
   TEMPLATE() LANGULUS(INLINED)
   Block TME()::GetBlock() const {
      return {
         DataState::Constrained, GetType(), 1, &mValue 
         // Notice entry is missing, which means it will be searched    
      };
   }


} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TME