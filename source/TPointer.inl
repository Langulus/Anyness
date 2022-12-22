///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TPointer.hpp"

#define TEMPLATE_SHARED() template<class T, bool DR>
#define SHARED_POINTER() TPointer<T, DR>

namespace Langulus::Anyness
{

   /// Copy a shared pointer                                                  
   ///   @param other - pointer to reference                                  
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(const TPointer& other)
      : TPointer {Langulus::Copy(other)} {}

   /// Move a shared pointer                                                  
   ///   @param other - pointer to move                                       
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(TPointer&& other) noexcept
      : TPointer {Langulus::Move(other)} {}

   /// Semantic self-constructor                                              
   ///   @tparam S - type of semantic                                         
   ///   @param other - semantic value to construct with                      
   /*TEMPLATE_SHARED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(S&& other) noexcept requires (CT::Exact<TypeOf<S>, SHARED_POINTER()>)
      : Base {other.template Forward<Base>()} {
      if constexpr (S::Move) {
         // Move in the contents of the other shared pointer            
         mEntry = other.mValue.mEntry;
         other.mValue.mEntry = nullptr;
      }
      else if constexpr (S::Keep) {
         // Copy the entry of the other shared pointer                  
         mEntry = other.mValue.mEntry;

         if (mValue) {
            // And reference the memory if pointer is valid             
            if (mEntry)
               mEntry->Keep();
            if constexpr (DR && CT::Referencable<T>)
               mValue->Keep();
         }
      }
      else mEntry = nullptr;
   }*/

   /// Reference a raw pointer                                                
   ///   @param ptr - pointer to reference                                    
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(MemberType ptr)
      : TPointer {Langulus::Copy(ptr)} {}

   /// Semantic construction by raw pointer                                   
   ///   @tparam S - type of semantic                                         
   ///   @param other - semantic raw pointer to construct with                
   /*TEMPLATE_SHARED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(S&& ptr) noexcept requires (CT::Exact<TypeOf<S>, TypeOf<SHARED_POINTER()>>)
      : Base {ptr.template Forward<Base>()} {
      if constexpr (S::Move) {
         // Move in the contents of the other shared pointer            
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            mEntry = Inner::Allocator::Find(MetaData::Of<T>(), ptr.mValue);
         #endif
      }
      else if constexpr (S::Keep) {
         // Copy the entry of the other shared pointer                  
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            mEntry = Inner::Allocator::Find(MetaData::Of<T>(), ptr.mValue);
         #endif

         if (mValue) {
            // And reference the memory if pointer is valid             
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               if (mEntry)
                  mEntry->Keep();
            #endif

            if constexpr (DR && CT::Referencable<T>)
               mValue->Keep();
         }
      }
      else mEntry = nullptr;
   }*/

   /// Shared pointer destruction                                             
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::~TPointer() {
      Reset();
   }

   /// Create a new instance of T by providing constructor arguments          
   ///   @tparam ...ARGS - the deduced arguments                              
   ///   @param arguments - the arguments                                     
   ///   @return the new instance                                             
   TEMPLATE_SHARED()
   template<class... ARGS>
   LANGULUS(ALWAYSINLINE)
   void SHARED_POINTER()::New(ARGS&&... arguments) {
      TPointer pointer;
      pointer.mEntry = Inner::Allocator::Allocate(sizeof(Decay<T>));
      LANGULUS_ASSERT(pointer.mEntry, Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<decltype(pointer.mValue)>(
         pointer.mEntry->GetBlockStart());
      new (pointer.mValue) Decay<T> {Forward<ARGS>(arguments)...};
      *this = pointer;
   }

   /// Reset the pointer                                                      
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   void SHARED_POINTER()::ResetInner() {
      // Do referencing in the element itself, if available             
      if constexpr (DR && CT::Referencable<T>)
         mValue->Free();

      if (mEntry) {
         // We own this data and are responsible for dereferencing it   
         if (mEntry->GetUses() == 1) {
            using Decayed = Decay<T>;
            if constexpr (CT::Destroyable<T>)
               mValue->~Decayed();
            Inner::Allocator::Deallocate(mEntry);
         }
         else mEntry->Free();
      }
   }

   /// Reset the pointer                                                      
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   void SHARED_POINTER()::Reset() {
      if (mValue) {
         ResetInner();
         mValue = {};
      }
   }

   /// Clone the pointer                                                      
   ///   @return the cloned pointer                                           
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER() SHARED_POINTER()::Clone() const {
      if constexpr (CT::Clonable<T>) {
         if (mValue)
            return Create(mValue->Clone());
         return {};
      }
      else return *this;
   }

   /// Copy a shared pointer                                                  
   ///   @param other - pointer to reference                                  
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (const SHARED_POINTER()& other) {
      if (other.mValue) {
         // Always first reference the other, before dereferencing, so  
         // we	don't prematurely lose the data in the rare case         
         // pointers are the same                                       
         if constexpr (DR && CT::Referencable<T>)
            other.mValue->Keep();
         if (other.mEntry)
            other.mEntry->Keep();
         if (mValue)
            ResetInner();

         mValue = other.mValue;
         mEntry = other.mEntry;
         return *this;
      }
      
      Reset();
      return *this;
   }

   /// Move a shared pointer                                                  
   ///   @param other - pointer to move                                       
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (SHARED_POINTER()&& other) {
      if (other.mValue) {
         if (mValue)
            ResetInner();

         mValue = other.mValue;
         mEntry = other.mEntry;
         other.mValue = {};
         return *this;
      }

      Reset();
      return *this;
   }

   /// Reference a raw pointer                                                
   ///   @param ptr - pointer to reference                                    
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (MemberType ptr) {
      if (mValue)
         ResetInner();

      new (this) TPointer<T, DR> {ptr};
      return *this;
   }

   /// Attempt to cast any pointer to the contained pointer                   
   ///   @param ptr - pointer to reference                                    
   TEMPLATE_SHARED() template<CT::Sparse ALT_T>
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (ALT_T rhs) {
      static_assert(CT::Constant<T> || !CT::Constant<ALT_T>,
         "Can't assign a constant pointer to a non-constant pointer wrapper");

      Reset();
      new (this) TPointer<T, DR> {
         dynamic_cast<Conditional<CT::Constant<ALT_T>, const T*, T*>>(rhs)
      };
      return *this;
   }

   /// Attempt to cast any pointer to the contained pointer                   
   ///   @param ptr - pointer to reference                                    
   TEMPLATE_SHARED() template<class ALT_T>
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (const TPointer<ALT_T, DR>& ptr) {
      static_assert(CT::Constant<T> || !CT::Constant<ALT_T>,
         "Can't assign a constant pointer to a non-constant pointer wrapper");

      Reset();
      new (this) TPointer<T, DR> {
         dynamic_cast<Conditional<CT::Constant<ALT_T>, const T*, T*>>(ptr.Get())
      };
      return *this;
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T> {
      return {mValue};
   }

   /// Check if we have authority over the memory                             
   ///   @return true if we own the memory                                    
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   constexpr bool SHARED_POINTER()::HasAuthority() const noexcept {
      return mValue && mEntry;
   }
      
   /// Get the references for the entry, where this pointer resides in        
   ///   @attention returns zero if pointer is not managed                    
   ///   @return number of uses for the pointer's memory                      
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   constexpr Count SHARED_POINTER()::GetUses() const noexcept {
      return (mValue && mEntry) ? mEntry->GetUses() : 0;
   }
               
   /// Get the block of the contained pointer                                 
   /// Can be invoked by the reflected resolver                               
   ///   @return the pointer, interfaced via a memory block                   
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   Block SHARED_POINTER()::GetBlock() const {
      return {
         DataState::Constrained | DataState::Sparse,
         Base::GetType(), 1, &(mValue),
         // Notice entry is here, no search will occur                  
         mEntry
      };
   }

   /// Compare pointers for equality                                          
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   bool SHARED_POINTER()::operator == (const SHARED_POINTER()& rhs) const noexcept {
      return Base::operator == (rhs);
   }

} // namespace Langulus::Anyness

#undef TEMPLATE_SHARED
#undef SHARED_POINTER