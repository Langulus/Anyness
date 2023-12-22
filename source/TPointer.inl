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

   /// Copy constructor                                                       
   ///   @param other - pointer to reference                                  
   TEMPLATE() LANGULUS(INLINED)
   TME()::TPointer(const TPointer& other)
      : TPointer {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - pointer to move                                       
   TEMPLATE() LANGULUS(INLINED)
   TME()::TPointer(TPointer&& other)
      : TPointer {Move(other)} {}
   
   /// Semantic construction                                                  
   ///   @param other - the value to initialize with                          
   TEMPLATE()
   template<template<class> class S>
   LANGULUS(INLINED)
   TME()::TPointer(S<TPointer>&& other) requires CT::Inner::SemanticMakable<S, Type> {
      using SS = S<TPointer>;
      GetHandle().New(SS::Nest(other->GetHandle()));

      if constexpr (SS::Move) {
         // Remote value is removed, if moved and double-referenced     
         if constexpr (DR and CT::Referencable<T>)
            other->mValue = {};
      }
      else if constexpr (SS::Shallow and SS::Keep) {
         // Reference value, if double-referenced and copied            
         if constexpr (DR and CT::Referencable<T>)
            mValue->Keep();
      }
   }

   /// Forward any compatible arguments towards contained value constructor   
   ///   @param arguments... - the arguments to forward                       
   TEMPLATE()
   template<class A>
   LANGULUS(INLINED)
   TME()::TPointer(A&& other) requires CT::Inner::MakableFrom<Type, A&&> {
      if constexpr (CT::Nullptr<A>) {
         // Assign a nullptr                                            
         return;
      }
      else {
         // Always copy, and thus reference raw pointers                
         auto converted = static_cast<Type>(other);
         GetHandle().New(Copy(converted));

         // Always reference value, if double-referenced                
         if constexpr (DR and CT::Referencable<T>)
            mValue->Keep();
      }
   }

   /// Shared pointer destruction                                             
   TEMPLATE() LANGULUS(INLINED)
   TME()::~TPointer() {
      if (mValue)
         ResetInner();
   }

   /// Create a new instance of T by providing constructor arguments          
   ///   @tparam ...ARGS - the deduced arguments                              
   ///   @param arguments - the arguments                                     
   ///   @return the new instance                                             
   TEMPLATE()
   template<class... ARGS>
   LANGULUS(INLINED)
   void TME()::New(ARGS&&... arguments) {
      TPointer pointer;
      pointer.mEntry = Allocator::Allocate(
         MetaDataOf<Decay<T>>(), 
         sizeof(Decay<T>)
      );
      LANGULUS_ASSERT(pointer.mEntry, Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<decltype(pointer.mValue)>(
         const_cast<Byte*>(pointer.mEntry->GetBlockStart()));
      new (pointer.mValue) Decay<T> {Forward<ARGS>(arguments)...};
      *this = Move(pointer);
   }

   /// Reset the pointer                                                      
   ///   @attention assumes mValue is a valid pointer                         
   TEMPLATE() LANGULUS(INLINED)
   void TME()::ResetInner() {
      // Do referencing in the element itself, if available             
      if constexpr (DR and CT::Referencable<T>) {
         if (mValue->GetReferences() > 1)
            mValue->Free();
      }

      GetHandle().template Destroy<false>();
   }

   /// Reset the pointer                                                      
   TEMPLATE() LANGULUS(INLINED)
   void TME()::Reset() {
      if (mValue) {
         ResetInner();
         mValue = {};
      }
   }

   /// Copy-assignment                                                        
   ///   @param rhs - pointer to reference                                    
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (const TPointer& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move-assignment                                                        
   ///   @param rhs - pointer to move                                         
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (TPointer&& rhs) {
      return operator = (Move(rhs));
   }

   /// Copy-assign from any pointer/shared pointer/nullptr/related pointer    
   ///   @param rhs - the value to assign                                     
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (const CT::PointerRelated auto& rhs) {
      return operator = (Copy(rhs));
   }

   /// Copy-assign from any pointer/shared pointer/nullptr/related pointer    
   ///   @param rhs - the value to assign                                     
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (CT::PointerRelated auto& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move-assign from any pointer/shared pointer/nullptr/related pointer    
   ///   @param rhs - the value to assign                                     
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (CT::PointerRelated auto&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantically assign from any pointer/shared pointer/nullptr/related    
   ///   @param rhs - the value and semantic to assign                        
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::AssignFrom(CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Nullptr<ST>) {
         // Assign a nullptr, essentially resetting the shared pointer  
         Reset();
         return *this;
      }
      else {
         // Move/Abandon/Disown/Copy/Clone another TPointer or raw,     
         // pointer, as long as it is related                           
         if constexpr (S::Shallow and not S::Move and S::Keep) {
            if constexpr (DR and CT::Referencable<T>) {
               if (mValue and mEntry->GetUses() > 1)
                  mValue->Free();
            }
         }

         if constexpr (CT::Pointer<ST>) {
            static_assert(
               CT::Exact<Type, TypeOf<ST>> or CT::DerivedFrom<TypeOf<ST>, T>,
               "Unrelated type inside shared pointer"
            );

            GetHandle().Assign(S::Nest(rhs->GetHandle()));

            if constexpr (S::Shallow and not S::Move and S::Keep) {
               if constexpr (DR and CT::Referencable<T>) {
                  if (mValue)
                     mValue->Keep();
               }
            }
         }
         else {
            static_assert(
               CT::Exact<Type, ST> or CT::DerivedFrom<ST, T>,
               "Unrelated raw pointer"
            );

            // Raw pointers are always copied, and thus referenced      
            GetHandle().Assign(Copy(*rhs));

            if constexpr (DR and CT::Referencable<T>) {
               if (mValue)
                  mValue->Keep();
            }
         }

         return *this;
      }
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

   /*TEMPLATE() LANGULUS(INLINED)
   TME()::operator T* () const noexcept {
      return mValue;
   }*/

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