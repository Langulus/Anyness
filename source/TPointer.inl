///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TPointer.hpp"

#define TEMPLATE_OWNED() template<CT::Data T>
#define TEMPLATE_SHARED() template<CT::Data T, bool DR>

namespace Langulus::Anyness
{

   /// Initialize with a value                                                
   ///   @param value - value to copy (no referencing shall occur if sparse)  
   TEMPLATE_OWNED()
   constexpr TOwned<T>::TOwned(const T& value) noexcept
      : mValue {value} { }

   /// Move ownership, resetting source value to default                      
   ///   @param value - value to move                                         
   TEMPLATE_OWNED()
   constexpr TOwned<T>::TOwned(TOwned&& value) noexcept
      : mValue {value.mValue} {
      value.mValue = {};
   }

   /// Copy a shared pointer                                                  
   ///   @param other - pointer to reference                                  
   TEMPLATE_SHARED()
   TPointer<T, DR>::TPointer(const TPointer& other)
      : Base {other}
      , mEntry {other.mEntry} {
      if (mValue) {
         if (mEntry)
            mEntry->Keep();
         if constexpr (DR && CT::Referencable<T>)
            mValue->Keep();
      }
   }

   /// Move a shared pointer                                                  
   ///   @param other - pointer to move                                       
   TEMPLATE_SHARED()
   TPointer<T, DR>::TPointer(TPointer&& other) noexcept
      : Base {Forward<Base>(other)}
      , mEntry {other.mEntry} {}

   /// Reference a pointer                                                    
   ///   @param ptr - pointer to reference                                    
   TEMPLATE_SHARED()
   TPointer<T, DR>::TPointer(MemberType ptr)
      : Base {ptr}
      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         , mEntry {Inner::Allocator::Find(MetaData::Of<T>(), ptr)} {
         if (mValue) {
            if (mEntry)
               mEntry->Keep();
            if constexpr (DR && CT::Referencable<T>)
               mValue->Keep();
         }
      }
      #else
         , mEntry {nullptr} {
         if (mValue) {
            if constexpr (DR && CT::Referencable<T>)
               mValue->Keep();
         }
      }
      #endif

   /// Shared pointer destruction                                             
   TEMPLATE_SHARED()
   TPointer<T, DR>::~TPointer() {
      Reset();
   }

   /// Create a new instance by moving an existing one                        
   ///   @param initializer - instance to move                                
   ///   @return the pointer                                                  
   /*TEMPLATE_SHARED()
   TPointer<T, DR> TPointer<T, DR>::Create(Decay<T>&& initializer) requires CT::MoveMakable<Decay<T>> {
      TPointer pointer;
      pointer.mEntry = Inner::Allocator::Allocate(
         RTTI::GetAllocationPageOf<Decay<T>>());
      LANGULUS_ASSERT(pointer.mEntry, Except::Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<MemberType>(
         pointer.mEntry->GetBlockStart());
      new (pointer.mValue) Decay<T> {Forward<Decay<T>>(initializer)};
      return pointer;
   }

   /// Create a new instance by copying an existing one                       
   /// Resulting pointer created that way has exactly one reference           
   ///   @param initializer - instance to copy                                
   ///   @return the pointer                                                  
   TEMPLATE_SHARED()
   TPointer<T, DR> TPointer<T, DR>::Create(const Decay<T>& initializer) requires CT::CopyMakable<Decay<T>> {
      TPointer pointer;
      pointer.mEntry = Inner::Allocator::Allocate(
         RTTI::GetAllocationPageOf<Decay<T>>());
      LANGULUS_ASSERT(pointer.mEntry, Except::Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<MemberType>(
         pointer.mEntry->GetBlockStart());
      new (pointer.mValue) Decay<T> {initializer};
      return pointer;
   }

   /// Create a default new instance                                          
   /// Resulting pointer created that way has exactly one reference           
   ///   @return the pointer                                                  
   TEMPLATE_SHARED()
   TPointer<T, DR> TPointer<T, DR>::Create() requires CT::Defaultable<Decay<T>> {
      TPointer pointer;
      pointer.mEntry = Inner::Allocator::Allocate(
         RTTI::GetAllocationPageOf<Decay<T>>());
      LANGULUS_ASSERT(pointer.mEntry, Except::Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<decltype(pointer.mValue)>(
         pointer.mEntry->GetBlockStart());
      new (pointer.mValue) Decay<T> {};
      return pointer;
   }*/

   /// Create a new instance of T by providing constructor arguments          
   ///   @tparam ...ARGS - the deduced arguments                              
   ///   @param arguments - the arguments                                     
   ///   @return the new instance                                             
   TEMPLATE_SHARED() template<class... ARGS>
   /*TPointer<T, DR>*/void TPointer<T, DR>::New(ARGS&&... arguments) {
      TPointer pointer;
      pointer.mEntry = Inner::Allocator::Allocate(sizeof(Decay<T>));
         /*RTTI::GetAllocationPageOf<Decay<T>>());*/
      LANGULUS_ASSERT(pointer.mEntry, Except::Allocate, "Out of memory");
      pointer.mValue = reinterpret_cast<decltype(pointer.mValue)>(
         pointer.mEntry->GetBlockStart());
      new (pointer.mValue) Decay<T> {Forward<ARGS>(arguments)...};
      *this = pointer;
      //return pointer;
   }

   /// Reset the value                                                        
   TEMPLATE_OWNED()
   void TOwned<T>::Reset() noexcept {
      mValue = {};
   }

   /// Reset the pointer                                                      
   TEMPLATE_SHARED()
   void TPointer<T, DR>::ResetInner() {
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
   void TPointer<T, DR>::Reset() {
      if (mValue) {
         ResetInner();
         mValue = {};
      }
   }

   /// Clone the pointer                                                      
   ///   @return the cloned pointer                                           
   TEMPLATE_SHARED()
   TPointer<T, DR> TPointer<T, DR>::Clone() const {
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
   TPointer<T, DR>& TPointer<T, DR>::operator = (const TPointer<T, DR>& other) {
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
   TPointer<T, DR>& TPointer<T, DR>::operator = (TPointer<T, DR>&& other) {
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
   TPointer<T, DR>& TPointer<T, DR>::operator = (MemberType ptr) {
      if (mValue)
         ResetInner();

      new (this) TPointer<T, DR> {ptr};
      return *this;
   }

   /// Move-assign a value                                                    
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   constexpr TOwned<T>& TOwned<T>::operator = (TOwned&& value) noexcept {
      mValue = value.mValue;
      value.mValue = {};
      return *this;
   }

   /// Overwrite the value                                                    
   ///   @param value - the new value                                         
   TEMPLATE_OWNED()
   constexpr TOwned<T>& TOwned<T>::operator = (const T& value) noexcept {
      mValue = value;
      return *this;
   }

   /// Attempt to cast any pointer to the contained pointer                   
   ///   @param ptr - pointer to reference                                    
   TEMPLATE_SHARED() template<CT::Sparse ANY_POINTER>
   TPointer<T, DR>& TPointer<T, DR>::operator = (ANY_POINTER rhs) {
      static_assert(CT::Constant<T> || !CT::Constant<ANY_POINTER>,
         "Can't assign a constant pointer to a non-constant pointer wrapper");

      Reset();
      new (this) TPointer<T, DR> {
         dynamic_cast<Conditional<CT::Constant<ANY_POINTER>, const T*, T*>>(rhs)
      };
      return *this;
   }

   /// Attempt to cast any pointer to the contained pointer                   
   ///   @param ptr - pointer to reference                                    
   TEMPLATE_SHARED() template<CT::Data ANY_POINTER>
   TPointer<T, DR>& TPointer<T, DR>::operator = (const TPointer<ANY_POINTER, DR>& ptr) {
      static_assert(CT::Constant<T> || !CT::Constant<ANY_POINTER>,
         "Can't assign a constant pointer to a non-constant pointer wrapper");

      Reset();
      new (this) TPointer<T, DR> {
         dynamic_cast<Conditional<CT::Constant<ANY_POINTER>, const T*, T*>>(ptr.Get())
      };
      return *this;
   }

   /// Get the pointer                                                        
   ///   @return the contained pointer                                        
   TEMPLATE_OWNED()
   decltype(auto) TOwned<T>::Get() const noexcept {
      return mValue;
   }
   
   TEMPLATE_OWNED()
   decltype(auto) TOwned<T>::Get() noexcept {
      return mValue;
   }

   /// Get the hash of the contained type                                     
   ///   @return the hash of the container type                               
   TEMPLATE_OWNED()
   Hash TOwned<T>::GetHash() const requires CT::Hashable<T> {
      if (!mValue)
         return {};
      return mValue->GetHash();
   }

   /// Perform a dynamic cast on the pointer                                  
   ///   @tparam D - the desired type to cast to                              
   ///   @return the result of a dynamic_cast to the specified type           
   TEMPLATE_OWNED() template<CT::Data D>
   auto TOwned<T>::As() const noexcept requires CT::Sparse<T> {
      using RESOLVED = Conditional<CT::Constant<T>, const Decay<D>*, Decay<D>*>;
      return dynamic_cast<RESOLVED>(mValue);
   }

   /// Access constant pointer                                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant raw pointer                           
   TEMPLATE_OWNED()
   auto TOwned<T>::operator -> () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access mutable pointer                                                 
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained raw pointer                                    
   TEMPLATE_OWNED()
   auto TOwned<T>::operator -> () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return mValue;
   }

   /// Access the dereferenced pointer (const)                                
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained constant reference                             
   TEMPLATE_OWNED()
   decltype(auto) TOwned<T>::operator * () const requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Access the dereferenced pointer                                        
   ///   @attention assumes contained pointer is valid                        
   ///   @return the contained mutable reference                              
   TEMPLATE_OWNED()
   decltype(auto) TOwned<T>::operator * () requires CT::Sparse<T> {
      LANGULUS_ASSUME(UserAssumes, mValue, "Invalid pointer");
      return *mValue;
   }

   /// Explicit boolean cast                                                  
   ///   @return true if value differs from default value                     
   TEMPLATE_OWNED()
   TOwned<T>::operator bool() const noexcept {
      return mValue != T {};
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE_OWNED()
   TOwned<T>::operator const T&() const noexcept {
      return mValue;
   }

   TEMPLATE_OWNED()
   TOwned<T>::operator T&() noexcept {
      return mValue;
   }

   /// Cast to a constant pointer, if mutable                                 
   ///   @return the constant equivalent to this pointer                      
   TEMPLATE_SHARED()
   TPointer<T, DR>::operator TPointer<const T, DR>() const noexcept requires CT::Mutable<T> {
      return {mValue};
   }

   /// Compare pointers for equality                                          
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   bool TOwned<T>::operator == (const TOwned<T>& rhs) const noexcept {
      return mValue == rhs.mValue;
   }

   /// Compare pointers for equality                                          
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   bool TOwned<T>::operator == (const T& rhs) const noexcept {
      return mValue == rhs;
   }

   /// Compare pointer for nullptr                                            
   ///   @param rhs - the right pointer                                       
   ///   @return true if pointers match                                       
   TEMPLATE_OWNED()
   bool TOwned<T>::operator == (std::nullptr_t) const noexcept requires CT::Sparse<T> {
      return mValue == nullptr;
   }

   /// Get the block of the contained value                                   
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced via a memory block                     
   TEMPLATE_OWNED()
   DMeta TOwned<T>::GetType() const {
      return MetaData::Of<Decay<T>>();
   }

   /// Get the block of the contained value                                   
   /// Can be invoked by the reflected resolver                               
   ///   @return the value, interfaced via a memory block                     
   TEMPLATE_OWNED()
   Block TOwned<T>::GetBlock() const {
      return {
         CT::Sparse<T> 
            ? DataState::Constrained | DataState::Sparse
            : DataState::Constrained,
         GetType(), 1, &mValue 
         // Notice entry is missing, which means it will be searched    
      };
   }

   /// Check if we have authority over the memory                             
   ///   @return true if we own the memory                                    
   TEMPLATE_SHARED()
   constexpr bool TPointer<T, DR>::HasAuthority() const noexcept {
      return mValue && mEntry;
   }
      
   /// Get the references for the entry, where this pointer resides in        
   ///   @attention returns zero if pointer is not managed                    
   ///   @return number of uses for the pointer's memory                      
   TEMPLATE_SHARED()
   constexpr Count TPointer<T, DR>::GetUses() const noexcept {
      return (mValue && mEntry) ? mEntry->GetUses() : 0;
   }
               
   /// Get the block of the contained pointer                                 
   /// Can be invoked by the reflected resolver                               
   ///   @return the pointer, interfaced via a memory block                   
   TEMPLATE_SHARED()
   Block TPointer<T, DR>::GetBlock() const {
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
   bool TPointer<T, DR>::operator == (const TPointer<T, DR>& rhs) const noexcept {
      return Base::operator == (rhs);
   }

} // namespace Langulus::Anyness

#undef TEMPLATE_OWNED
#undef TEMPLATE_SHARED
