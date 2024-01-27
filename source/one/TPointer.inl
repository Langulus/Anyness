///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TPointer.hpp"
#include "TOwned.inl"

#define TEMPLATE() template<class T, bool DR>
#define TME() TPointer<T, DR>


namespace Langulus::Anyness
{
   
   /// Get handle representation of the contained pointer                     
   TEMPLATE() LANGULUS(INLINED)
   auto TME()::GetHandle() const {
      const auto mthis = const_cast<TME()*>(this);
      return Handle<Type> {mthis->mValue, mthis->mEntry};
   }

   /// Default costructor                                                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TPointer() noexcept
      : Base {nullptr}
      , mEntry {nullptr} {}

   /// Copy constructor                                                       
   ///   @param other - pointer to reference                                  
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TPointer(const TPointer& other)
      : TPointer {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - pointer to move                                       
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TPointer(TPointer&& other)
      : TPointer {Move(other)} {}
   
   /// Semantic construction                                                  
   ///   @param other - the value to initialize with                          
   TEMPLATE() template<template<class> class S> 
   requires CT::Inner::SemanticMakable<S, T*> LANGULUS(INLINED)
   constexpr TME()::TPointer(S<TPointer>&& other) {
      using SS = S<TPointer>;
      GetHandle().CreateSemantic(SS::Nest(other->GetHandle()));

      if constexpr (SS::Move) {
         // Remote value is removed, if moved and double-referenced     
         // Notice it is always nullified, even when abandoned          
         if constexpr (DR and CT::Referencable<T>)
            other->mValue = {};
      }
      else if constexpr (SS::Shallow and SS::Keep) {
         // Reference value, if double-referenced and copied            
         if constexpr (DR and CT::Referencable<T>) {
            if (mValue)
               mValue->Keep();
         }
      }
   }

   /// Construct from any compatible pointer                                  
   ///   @attention this will search for the allocation source of the pointer 
   ///      which will incur some runtime overhead, unless you use Disown     
   ///   @param other - the pointer                                           
   TEMPLATE() template<class A>
   requires CT::MakableFrom<T*, A> LANGULUS(INLINED)
   constexpr TME()::TPointer(A&& other) {
      using S = SemanticOf<A>;
      using ST = TypeOf<S>;

      if constexpr (CT::Nullptr<ST>) {
         // Assign a nullptr                                            
         return;
      }
      else {
         // Always copy, and thus reference raw pointers                
         auto converted = static_cast<Type>(DesemCast(other));
         GetHandle().CreateSemantic(S::Nest(converted));

         if constexpr (S::Shallow and S::Keep) {
            // Reference value, if double-referenced and copied         
            if constexpr (DR and CT::Referencable<T>) {
               if (mValue)
                  mValue->Keep();
            }
         }
      }
   }

   /// Shared pointer destruction                                             
   TEMPLATE() LANGULUS(INLINED)
   TME()::~TPointer() {
      ResetInner();
   }

   /// Create a new instance of T by providing constructor arguments          
   ///   @param arguments - the arguments                                     
   ///   @return the new instance                                             
   TEMPLATE() template<class...A>
   requires ::std::constructible_from<T, A...> LANGULUS(INLINED)
   void TME()::New(A&&...arguments) {
      TPointer pointer;
      pointer.mEntry = Allocator::Allocate(MetaDataOf<T>(), sizeof(T));
      LANGULUS_ASSERT(pointer.mEntry, Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<T*>(pointer.mEntry->GetBlockStart());
      new (pointer.mValue) T {Forward<A>(arguments)...};
      *this = Move(pointer);
   }

   /// Reset the pointer                                                      
   TEMPLATE() LANGULUS(INLINED)
   void TME()::ResetInner() {
      if constexpr (DR and CT::Referencable<T>) {
         // Do double referencing                                       
         if (mValue and mValue->GetReferences() > 1)
            mValue->Free();
      }

      GetHandle().template Destroy<false>();
   }

   /// Reset the pointer                                                      
   TEMPLATE() LANGULUS(INLINED)
   void TME()::Reset() {
      ResetInner();
      mValue = {};
   }

   /// Copy-assignment                                                        
   ///   @param rhs - pointer to reference                                    
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (const TPointer& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move-assignment                                                        
   ///   @param rhs - pointer to move                                         
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (TPointer&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantically assign from any pointer/shared pointer/nullptr/related    
   ///   @param rhs - the value and semantic to assign                        
   ///   @return a reference to this shared pointer                           
   TEMPLATE() template<template<class> class S>
   requires CT::Inner::SemanticAssignable<S, T*> LANGULUS(INLINED)
   TME()& TME()::operator = (S<TPointer>&& rhs) {
      using SS = S<TPointer>;
      if constexpr (DR and CT::Referencable<T>) {
         // Decrement deeper references                                 
         if (mValue and mValue->GetReferences() > 1)
            mValue->Free();
      }

      GetHandle().AssignSemantic(SS::Nest(rhs->GetHandle()));

      if constexpr (SS::Shallow and not SS::Move and SS::Keep) {
         // Reference value, if double-referenced and copied            
         if constexpr (DR and CT::Referencable<T>)
            mValue->Keep();
      }

      return *this;
   }

   /// Semantically assign from any pointer/shared pointer/nullptr/related    
   ///   @param rhs - the value and semantic to assign                        
   ///   @return a reference to this shared pointer                           
   TEMPLATE() template<CT::NotOwned A>
   requires CT::AssignableFrom<T*, A> LANGULUS(INLINED)
   TME()& TME()::operator = (A&& rhs) {
      using S = SemanticOf<A>;
      using ST = TypeOf<S>;

      if constexpr (CT::Nullptr<ST>) {
         // Assign a nullptr, essentially resetting the shared pointer  
         Reset();
      }
      else {
         // Assign a new pointer                                        
         if constexpr (DR and CT::Referencable<T>) {
            // Decrement deeper references                              
            if (mValue and mValue->GetReferences() > 1)
               mValue->Free();
         }

         // Raw pointers are always copied, and thus referenced         
         auto converted = static_cast<Type>(DesemCast(rhs));
         GetHandle().AssignSemantic(S::Nest(converted));

         if constexpr (S::Shallow and S::Keep) {
            // Reference value, if double-referenced and copied         
            if constexpr (DR and CT::Referencable<T>) {
               if (mValue)
                  mValue->Keep();
            }
         }
      }

      return *this;
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE() LANGULUS(INLINED)
   TME()::operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T> {
      return {mValue};
   }

   TEMPLATE() LANGULUS(INLINED)
   TME()::operator const T& () const noexcept {
      return mValue;
   }

   /// Check if we have authority over the memory                             
   ///   @return true if we own the memory behind the pointer                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::HasAuthority() const noexcept {
      return mEntry;
   }
      
   /// Get the references for the entry, where this pointer resides in        
   ///   @attention returns zero if pointer is not managed                    
   ///   @return number of uses for the pointer's memory                      
   TEMPLATE() LANGULUS(INLINED)
   constexpr Count TME()::GetUses() const noexcept {
      return mEntry ? mEntry->GetUses() : 0;
   }
               
   /// Get the block of the contained pointer                                 
   /// Can be invoked by the reflected resolver                               
   ///   @return the pointer, interfaced via a memory block                   
   TEMPLATE() LANGULUS(INLINED)
   Block TME()::GetBlock() const {
      return {
         DataState::Constrained,
         Base::GetType(), 1, &(mValue),
         // Notice entry is here, no search will occur                  
         mEntry
      };
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TME