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
   
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   auto SHARED_POINTER()::GetHandle() const {
      const auto mthis = const_cast<SHARED_POINTER()*>(this);
      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         return Handle<Type> {mthis->mValue, mthis->mEntry};
      #else
         return Handle<Type> {mthis->mValue};
      #endif
   }

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
   SHARED_POINTER()::TPointer(TPointer&& other)
      : TPointer {Langulus::Move(other)} {}

   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(const CT::NotSemantic auto& value)
      : TPointer {Copy(value)} {}

   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(CT::NotSemantic auto& value)
      : TPointer {Copy(value)} {}

   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(CT::NotSemantic auto&& value)
      : TPointer {Move(value)} {}

   TEMPLATE_SHARED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()::TPointer(S&& other) {
      if constexpr (CT::Exact<TypeOf<S>, TPointer>)
         GetHandle().New(S::Nest(other.mValue.GetHandle()));
      else
         GetHandle().New(other.Forward());

      if constexpr (S::Shallow && !S::Move && S::Keep && DR && CT::Referencable<T>)
         mValue->Keep();
   }

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

      GetHandle().template Destroy<false>();
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
      if constexpr (CT::CloneMakable<T>) {
         if (mValue)
            return Create(Langulus::Clone(*mValue));
         return {};
      }
      else return *this;
   }

   /// Copy a shared pointer                                                  
   ///   @param other - pointer to reference                                  
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (const TPointer& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move a shared pointer                                                  
   ///   @param other - pointer to move                                       
   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (TPointer&& rhs) {
      return operator = (Langulus::Move(rhs));
   }

   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (const CT::NotSemantic auto& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (CT::NotSemantic auto& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   TEMPLATE_SHARED()
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (CT::NotSemantic auto&& rhs) {
      return operator = (Langulus::Move(rhs));
   }

   /// Constructor needs to be declared here to avoid MSVC parser bug      
   TEMPLATE_SHARED()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   SHARED_POINTER()& SHARED_POINTER()::operator = (S&& rhs) {
      if constexpr (DR && CT::Referencable<T>) {
         if (mValue)
            mValue->Free();
      }

      if constexpr (CT::Exact<TypeOf<S>, TPointer>)
         GetHandle().Assign(S::Nest(rhs.mValue.GetHandle()));
      else
         GetHandle().Assign(rhs.Forward());

      if constexpr (S::Shallow && !S::Move && S::Keep && DR && CT::Referencable<T>)
         mValue->Keep();

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
         DataState::Constrained,
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