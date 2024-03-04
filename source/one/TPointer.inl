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

#define TEMPLATE() template<class T>
#define TME() TPointer<T>


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

   /// Refer constructor                                                      
   ///   @param other - pointer to reference                                  
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()::TPointer(const TPointer& other)
      : TPointer {Refer(other)} {}

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
   }

   /// Construct from any compatible pointer                                  
   ///   @attention this will search for the allocation source of the pointer 
   ///      which will incur some runtime overhead, unless you use Disown     
   ///   @param other - the pointer                                           
   TEMPLATE() template<class A>
   requires CT::MakableFrom<T*, A> LANGULUS(INLINED)
   constexpr TME()::TPointer(A&& other) {
      using S = SemanticOf<decltype(other)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Nullptr<ST>) {
         // Assign a nullptr                                            
         return;
      }
      else {
         // Always copy, and thus reference raw pointers                
         auto converted = static_cast<Type>(DesemCast(other));
         GetHandle().CreateSemantic(S::Nest(converted));
      }
   }

   /// Shared pointer destruction                                             
   TEMPLATE() LANGULUS(INLINED)
   TME()::~TPointer() {
      if (mEntry)
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
      *this = Abandon(pointer);
   }

   /// Reset the pointer                                                      
   ///   @attention assumes pointer is valid                                  
   TEMPLATE() LANGULUS(INLINED)
   void TME()::ResetInner() {
      LANGULUS_ASSUME(DevAssumes, mValue, "Null value");
      LANGULUS_ASSUME(DevAssumes, mEntry, "Null entry");
      GetHandle().template Destroy<false>();
   }

   /// Reset the pointer                                                      
   TEMPLATE() LANGULUS(INLINED)
   void TME()::Reset() {
      if (mEntry) {
         ResetInner();
         mEntry = {};
      }
      mValue = {};
   }

   /// Refer-assignment                                                       
   ///   @param rhs - pointer to reference                                    
   ///   @return a reference to this shared pointer                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr TME()& TME()::operator = (const TPointer& rhs) {
      return operator = (Refer(rhs));
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
      if (mEntry)
         ResetInner();
      new (this) TPointer {rhs.Forward()};
      return *this;
   }

   /// Semantically assign from any pointer/shared pointer/nullptr/related    
   ///   @param rhs - the value and semantic to assign                        
   ///   @return a reference to this shared pointer                           
   TEMPLATE() template<CT::NotOwned A>
   requires CT::AssignableFrom<T*, A> LANGULUS(INLINED)
   TME()& TME()::operator = (A&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Nullptr<ST>) {
         // Assign a nullptr, essentially resetting the shared pointer  
         Reset();
      }
      else {
         // Assign a new pointer                                        
         if (mEntry)
            ResetInner();
         new (this) TPointer {Forward<A>(rhs)};
      }

      return *this;
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE() LANGULUS(INLINED)
   TME()::operator TPointer<const T>() const noexcept requires CT::Mutable<T> {
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
         nullptr//mEntry
      };
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TME