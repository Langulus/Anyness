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

#define TEMPLATE_SHARED() template<class T, bool DR>
#define SHARED_POINTER() TPointer<T, DR>


namespace Langulus::Anyness
{
   
   /// Get handle representation of the contained pointer                     
   TEMPLATE_SHARED() LANGULUS(INLINED)
   auto SHARED_POINTER()::GetHandle() const {
      const auto mthis = const_cast<SHARED_POINTER()*>(this);
      return Handle<Type> {mthis->mValue, mthis->mEntry};
   }

   /// Copy constructor                                                       
   ///   @param other - pointer to reference                                  
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::TPointer(const TPointer& other)
      : TPointer {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - pointer to move                                       
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::TPointer(TPointer&& other)
      : TPointer {Move(other)} {}

   /// Copy construct from any pointer/shared pointer/nullptr/related pointer 
   ///   @param value - the value to use for initialization                   
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::TPointer(const CT::PointerRelated auto& value)
      : TPointer {Copy(value)} {}

   /// Copy construct from any pointer/shared pointer/nullptr/related pointer 
   ///   @param value - the value to use for initialization                   
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::TPointer(CT::PointerRelated auto& value)
      : TPointer {Copy(value)} {}

   /// Move construct from any pointer/shared pointer/nullptr/related pointer 
   ///   @param value - the value to use for initialization                   
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::TPointer(CT::PointerRelated auto&& value)
      : TPointer {Move(value)} {}

   /// Semantic construction from any pointer/shared pointer/nullptr/related  
   ///   @param other - the value & semantic to use for initialization        
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::TPointer(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Nullptr<ST>) {
         // Assign a nullptr                                            
         return;
      }
      else if constexpr (CT::Pointer<ST>) {
         // Move/Abandon/Disown/Copy/Clone another TPointer, as long as 
         // it is related                                               
         static_assert(
            CT::Exact<Type, TypeOf<ST>> or CT::DerivedFrom<TypeOf<ST>, T>,
            "Unrelated type inside shared pointer"
         );

         GetHandle().New(S::Nest(other->GetHandle()));
         
         if constexpr (S::Move) {
            // Remote value is removed, if moved and double-referenced  
            if constexpr (DR and CT::Referencable<T>)
               other->mValue = {};
         }
         else if constexpr (S::Shallow and S::Keep) {
            // Reference value, if double-referenced and copied         
            if constexpr (DR and CT::Referencable<T>)
               mValue->Keep();
         }
      }
      else if constexpr (CT::DerivedFrom<ST, T>) {
         // Move/Abandon/Disown/Copy/Clone raw pointer                  
         Type converted = static_cast<Type>(*other);
         GetHandle().New(S::Nest(converted));

         // Always reference value, if double-referenced and not cloned 
         if constexpr (S::Shallow and DR and CT::Referencable<T>)
            mValue->Keep();
      }
      else LANGULUS_ERROR("Bad semantic construction");
   }

   /// Shared pointer destruction                                             
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::~TPointer() {
      if (mValue)
         ResetInner();
   }

   /// Create a new instance of T by providing constructor arguments          
   ///   @tparam ...ARGS - the deduced arguments                              
   ///   @param arguments - the arguments                                     
   ///   @return the new instance                                             
   TEMPLATE_SHARED()
   template<class... ARGS>
   LANGULUS(INLINED)
   void SHARED_POINTER()::New(ARGS&&... arguments) {
      TPointer pointer;
      pointer.mEntry = Allocator::Allocate(
         RTTI::MetaData::Of<Decay<T>>(), 
         sizeof(Decay<T>)
      );
      LANGULUS_ASSERT(pointer.mEntry, Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<decltype(pointer.mValue)>(
         pointer.mEntry->GetBlockStart());
      new (pointer.mValue) Decay<T> {Forward<ARGS>(arguments)...};
      *this = Move(pointer);
   }

   /// Reset the pointer                                                      
   ///   @attention assumes mValue is a valid pointer                         
   TEMPLATE_SHARED() LANGULUS(INLINED)
   void SHARED_POINTER()::ResetInner() {
      // Do referencing in the element itself, if available             
      if constexpr (DR and CT::Referencable<T>) {
         if (mValue->GetReferences() > 1)
            mValue->Free();
      }

      GetHandle().template Destroy<false>();
   }

   /// Reset the pointer                                                      
   TEMPLATE_SHARED() LANGULUS(INLINED)
   void SHARED_POINTER()::Reset() {
      if (mValue) {
         ResetInner();
         mValue = {};
      }
   }

   /// Copy-assignment                                                        
   ///   @param rhs - pointer to reference                                    
   ///   @return a reference to this shared pointer                           
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()& SHARED_POINTER()::operator = (const TPointer& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move-assignment                                                        
   ///   @param rhs - pointer to move                                         
   ///   @return a reference to this shared pointer                           
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()& SHARED_POINTER()::operator = (TPointer&& rhs) {
      return operator = (Move(rhs));
   }

   /// Copy-assign from any pointer/shared pointer/nullptr/related pointer    
   ///   @param rhs - the value to assign                                     
   ///   @return a reference to this shared pointer                           
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()& SHARED_POINTER()::operator = (const CT::PointerRelated auto& rhs) {
      return operator = (Copy(rhs));
   }

   /// Copy-assign from any pointer/shared pointer/nullptr/related pointer    
   ///   @param rhs - the value to assign                                     
   ///   @return a reference to this shared pointer                           
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()& SHARED_POINTER()::operator = (CT::PointerRelated auto& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move-assign from any pointer/shared pointer/nullptr/related pointer    
   ///   @param rhs - the value to assign                                     
   ///   @return a reference to this shared pointer                           
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()& SHARED_POINTER()::operator = (CT::PointerRelated auto&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantically assign from any pointer/shared pointer/nullptr/related    
   ///   @param rhs - the value and semantic to assign                        
   ///   @return a reference to this shared pointer                           
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()& SHARED_POINTER()::operator = (CT::Semantic auto&& rhs) {
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
         }
         else {
            static_assert(
               CT::Exact<Type, ST> or CT::DerivedFrom<ST, T>,
               "Unrelated raw pointer"
            );

            GetHandle().Assign(rhs.Forward());
         }

         if constexpr (S::Shallow and not S::Move and S::Keep) {
            if constexpr (DR and CT::Referencable<T>) {
               if (mValue)
                  mValue->Keep();
            }
         }

         return *this;
      }
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE_SHARED() LANGULUS(INLINED)
   SHARED_POINTER()::operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T> {
      return {mValue};
   }

   /// Check if we have authority over the memory                             
   ///   @return true if we own the memory behind the pointer                 
   TEMPLATE_SHARED() LANGULUS(INLINED)
   constexpr bool SHARED_POINTER()::HasAuthority() const noexcept {
      return mEntry;
   }
      
   /// Get the references for the entry, where this pointer resides in        
   ///   @attention returns zero if pointer is not managed                    
   ///   @return number of uses for the pointer's memory                      
   TEMPLATE_SHARED() LANGULUS(INLINED)
   constexpr Count SHARED_POINTER()::GetUses() const noexcept {
      return mEntry ? mEntry->GetUses() : 0;
   }
               
   /// Get the block of the contained pointer                                 
   /// Can be invoked by the reflected resolver                               
   ///   @return the pointer, interfaced via a memory block                   
   TEMPLATE_SHARED() LANGULUS(INLINED)
   Block SHARED_POINTER()::GetBlock() const {
      return {
         DataState::Constrained,
         Base::GetType(), 1, &(mValue),
         // Notice entry is here, no search will occur                  
         mEntry
      };
   }

   /// Compare any similar shared pointer                                     
   ///   @tparam ALT_T - type of rhs pointer                                  
   ///   @tparam ALT_DR - referencing type of rhs pointer                     
   ///   @param rhs - the pointer to compare against                          
   ///   @return true if pointers match                                       
   TEMPLATE_SHARED()
   template<class ALT_T, bool ALT_DR>
   LANGULUS(INLINED)
   bool SHARED_POINTER()::operator == (const TPointer<ALT_T, ALT_DR>& rhs) const noexcept requires (CT::Inner::Comparable<T*, ALT_T*>) {
      return mValue == rhs.mValue;
   }

   /// Check if shared pointer is valid, by comparing it with nullptr         
   ///   @return true if pointer is nullptr                                   
   TEMPLATE_SHARED() LANGULUS(INLINED)
   bool SHARED_POINTER()::operator == (::std::nullptr_t) const noexcept {
      return mValue == nullptr;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE_SHARED
#undef SHARED_POINTER