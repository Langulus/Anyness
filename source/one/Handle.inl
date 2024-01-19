///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Handle.hpp"
#include "TPointer.hpp"

#define TEMPLATE() template<class T, bool EMBED>
#define HAND() Handle<T, EMBED>


namespace Langulus::Anyness
{

   /// Create an embedded handle                                              
   ///   @param v - a reference to the element                                
   ///   @param e - a reference to the element's entry                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr HAND()::Handle(T& v, const Allocation*& e) IF_UNSAFE(noexcept)
   requires (Embedded and CT::Sparse<T>)
      : mValue {&v}
      , mEntry {&e} {
      static_assert(CT::NotHandle<T>, "Handles can't be nested");
   }
      
   /// Create an embedded handle                                              
   ///   @param v - a reference to the element                                
   ///   @param e - the entry (optional)                                      
   TEMPLATE() LANGULUS(INLINED)
   constexpr HAND()::Handle(T& v, const Allocation* e) IF_UNSAFE(noexcept)
   requires (Embedded and CT::Dense<T>)
      : mValue {&v}
      , mEntry {e} {
      static_assert(CT::NotHandle<T>, "Handles can't be nested");
   }
      
   /// Create a standalone handle                                             
   ///   @param v - the element                                               
   ///   @param e - the entry (optional)                                      
   TEMPLATE() template<CT::NotHandle T1> LANGULUS(INLINED)
   constexpr HAND()::Handle(T1&& v, const Allocation* e)
   requires (not Embedded and CT::Inner::MakableFrom<T, T1>)
      : mValue {Forward<T1>(v)}
      , mEntry {e} {
      if constexpr (CT::Semantic<T1> and CT::Sparse<T>) {
         if constexpr (T1::Shallow) {
            // Copy/Disown/Move/Abandon a pointer                       
            // Since pointers don't have ownership, it's just a copy    
            // with an optional entry search, if not disowned, and if   
            // managed memory is enabled                                
            using DT = Deptr<T>;
            if constexpr (CT::Allocatable<DT> and (T1::Keep or T1::Move))
               mEntry = Allocator::Find(MetaDataOf<DT>(), mValue);
         }
         else {
            // Clone a pointer                                          
            TODO();
         }
      }
   }

   /// Semantically construct using handle of any compatible type             
   ///   @param other - the handle and semantic to construct with             
   TEMPLATE() template<template<class> class S, CT::Handle H> LANGULUS(INLINED)
   constexpr HAND()::Handle(S<H>&& other)
   requires (CT::Inner::MakableFrom<T, S<TypeOf<H>>>)
      : mValue {S<TypeOf<H>>(other->Get())}
      , mEntry {other->GetEntry()} {
      using HT = TypeOf<H>;
      static_assert(CT::NotHandle<HT>, "Handles can't be nested");
      static_assert(CT::Exact<T, HT>, "Handle types must match exactly");

      if constexpr (not Embedded) {
         if constexpr (S<H>::Shallow) {
            // Copy/Disown/Move/Abandon a handle                        
            if constexpr (not S<H>::Keep)
               mEntry = nullptr;

            if constexpr (S<H>::Move) {
               // Always reset remote entry, when moving                
               other->GetEntry() = nullptr;

               // Also reset remote value, if not an abandoned pointer  
               if constexpr (S<H>::Keep and CT::Sparse<HT>)
                  other->Get() = nullptr;
            }
         }
         else {
            // Clone a handle                                           
            TODO();
         }
      }
   }

   /// Compare a handle with a comparable value                               
   TEMPLATE() template<class T1> requires CT::Inner::Comparable<T, T1>
   LANGULUS(INLINED)
   constexpr bool HAND()::operator == (const T1& rhs) const noexcept {
      if constexpr (Embedded)
         return *mValue == rhs;
      else
         return mValue == rhs;
   }
      
   /// Compare handles                                                        
   TEMPLATE() template<class T1, bool EMBED1> requires CT::Inner::Comparable<T, T1>
   LANGULUS(INLINED)
   constexpr bool HAND()::operator == (const Handle<T1, EMBED1>& rhs) const noexcept {
      if constexpr (Embedded) {
         if constexpr (EMBED1)
            return *mValue == *rhs.mValue;
         else
            return *mValue == rhs.mValue;
      }
      else {
         if constexpr (EMBED1)
            return mValue == *rhs.mValue;
         else
            return mValue == rhs.mValue;
      }
   }
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE() LANGULUS(INLINED)
   HAND()& HAND()::operator ++ () noexcept requires Embedded {
      ++mValue;
      if constexpr (CT::Sparse<T>)
         ++mEntry;
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   TEMPLATE() LANGULUS(INLINED)
   HAND()& HAND()::operator -- () noexcept requires Embedded {
      --mValue;
      if constexpr (CT::Sparse<T>)
         --mEntry;
      return *this;
   }
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE() LANGULUS(INLINED)
   HAND()& HAND()::operator += (Offset offset) noexcept requires Embedded {
      mValue += offset;
      if constexpr (CT::Sparse<T>)
         mEntry += offset;
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   TEMPLATE() LANGULUS(INLINED)
   HAND()& HAND()::operator -= (Offset offset) noexcept requires Embedded {
      mValue -= offset;
      if constexpr (CT::Sparse<T>)
         mEntry -= offset;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @return the previous value of the handle                             
   TEMPLATE() LANGULUS(INLINED)
   HAND() HAND()::operator ++ (int) noexcept requires Embedded {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Suffix decrement operator                                              
   ///   @return the previous value of the handle                             
   TEMPLATE() LANGULUS(INLINED)
   HAND() HAND()::operator -- (int) noexcept requires Embedded {
      const auto backup = *this;
      operator -- ();
      return backup;
   }
      
   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   TEMPLATE() LANGULUS(INLINED)
   HAND() HAND()::operator + (Offset offset) noexcept requires Embedded {
      auto backup = *this;
      return backup += offset;
   }

   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   TEMPLATE() LANGULUS(INLINED)
   HAND() HAND()::operator - (Offset offset) noexcept requires Embedded {
      auto backup = *this;
      return backup -= offset;
   }

   /// Get a reference to the contents                                        
   TEMPLATE() LANGULUS(INLINED)
   T& HAND()::Get() const noexcept {
      if constexpr (Embedded)
         return const_cast<T&>(*mValue);
      else
         return const_cast<T&>(mValue);
   }
   
   /// Get the entry                                                          
   TEMPLATE() LANGULUS(INLINED)
   const Allocation*& HAND()::GetEntry() const noexcept {
      if constexpr (Embedded and CT::Sparse<T>)
         return const_cast<const Allocation*&>(*mEntry);
      else
         return const_cast<const Allocation*&>(mEntry);
   }

   /// Assign a new pointer and entry at the handle                           
   ///   @attention this overwrites previous handle without dereferencing it, 
   ///      and without destroying anything                                   
   ///   @param pointer - the new pointer to assign                           
   ///   @param entry - the allocation that the pointer is part of            
   TEMPLATE() LANGULUS(INLINED)
   void HAND()::Create(T pointer, const Allocation* entry) noexcept requires CT::Sparse<T> {
      Get() = pointer;
      GetEntry() = entry;
   }
   
   /// Move-assign a new value and entry at the handle                        
   ///   @attention this overwrites previous handle without dereferencing it, 
   ///      and without destroying anything                                   
   ///   @param value - the new value to assign                               
   ///   @param entry - the allocation that the value is part of              
   TEMPLATE() LANGULUS(INLINED)
   void HAND()::Create(T&& value, const Allocation* entry) noexcept requires CT::Dense<T> {
      SemanticNew(&Get(), Move(value));
      GetEntry() = entry;
   }

   /// Copy-assign a new value and entry at the handle                        
   ///   @attention this overwrites previous handle without dereferencing it, 
   ///      and without destroying anything                                   
   ///   @param value - the new value to assign                               
   ///   @param entry - the allocation that the value is part of              
   TEMPLATE() LANGULUS(INLINED)
   void HAND()::Create(const T& value, const Allocation* entry) noexcept requires CT::Dense<T> {
      SemanticNew(&Get(), Copy(value));
      GetEntry() = entry;
   }

   /// Semantically assign anything at the handle                             
   ///   @attention this overwrites previous handle without dereferencing it, 
   ///      and without destroying anything                                   
   ///   @param rhs - what are we assigning                                   
   TEMPLATE() template<template<class> class S, class ST>
   requires CT::Semantic<S<ST>> LANGULUS(INLINED)
   void HAND()::CreateSemantic(S<ST>&& rhs) {
      if constexpr (S<ST>::Shallow and CT::Sparse<T>) {
         // Do a copy/disown/abandon/move sparse LHS                    
         if constexpr (CT::Handle<ST>) {
            // RHS is a handle                                          
            using HT = TypeOf<ST>;
            static_assert(CT::Similar<T, HT>, "Handle type mismatch");
            Get() = rhs->Get();

            if constexpr (S<ST>::Keep or S<ST>::Move)
               GetEntry() = rhs->GetEntry();
            else
               GetEntry() = nullptr;

            if constexpr (S<ST>::Move) {
               // We're moving RHS, so we need to clear it up           
               if constexpr (S<ST>::Keep)
                  rhs->Get() = nullptr;

               // Clearing entry is mandatory, because we're            
               // transferring the ownership                            
               rhs->GetEntry() = nullptr;
            }
            else if constexpr (S<ST>::Keep) {
               // Copying RHS, but keep it only if not disowning it     
               if (GetEntry())
                  const_cast<Allocation*>(GetEntry())->Keep();
            }
         }
         else if constexpr (CT::Nullptr<ST>) {
            // RHS is a simple nullptr                                  
            Get() = nullptr;
            GetEntry() = nullptr;
         }
         else {
            // RHS is not a handle, but we'll wrap it in a handle, in   
            // order to find its entry (if managed memory is enabled)   
            static_assert(CT::Similar<T, ST>, "Type mismatch");
            HandleLocal<T> rhsh {rhs.Forward()};
            Get() = rhsh.Get();
            GetEntry() = rhsh.GetEntry();

            if constexpr (S<ST>::Keep) {
               if (GetEntry())
                  const_cast<Allocation*>(GetEntry())->Keep();
            }
         }
      }
      else if constexpr (CT::Dense<T>) {
         // Do a copy/disown/abandon/move/clone inside a dense handle   
         if constexpr (CT::Handle<ST> and CT::Similar<T, TypeOf<ST>>)
            SemanticNew(&Get(), S<ST>::Nest(rhs->Get()));
         else if constexpr (CT::Similar<T, ST>)
            SemanticNew(&Get(), rhs.Forward());
         else if constexpr (CT::Pointer<T> and CT::Similar<TypeOf<T>, ST>)
            SemanticNew(&Get(), rhs.Forward());
         else
            LANGULUS_ERROR("Can't initialize dense T");
      }
      else if constexpr (CT::Dense<Deptr<T>>) {
         // Do a clone, unless T is meta                                
         if constexpr (CT::Meta<T>) {
            // If T is meta, just copy pointer                          
            Get() = rhs->Get();
            GetEntry() = nullptr;
         }
         else if constexpr (CT::Resolvable<T>) {
            // If T is resolvable, we need to always clone the resolved 
            // (a.k.a the most concrete) type                           
            TODO();
         }
         else {
            // Otherwise attempt cloning DT conventionally              
            using DT = Decay<T>;
            auto meta = MetaDataOf<DT>();
            auto entry = Allocator::Allocate(meta, meta->RequestSize(1).mByteSize);
            auto pointer = entry->template As<DT>();

            if constexpr (CT::Handle<ST>) {
               static_assert(CT::Exact<T, TypeOf<ST>>, "Type mismatch");
               SemanticNew(pointer, S<ST>::Nest(*rhs->Get()));
            }
            else {
               static_assert(CT::Exact<T, ST>, "Type mismatch");
               SemanticNew(pointer, S<ST>::Nest(**rhs));
            }

            Get() = pointer;
            GetEntry() = entry;
         }
      }
      else {
         //clone an indirection layer by nesting semanticnewhandle      
         TODO();
      }
   }

   /// Dereference/destroy the current handle contents, and set new ones      
   ///   @param rhs - new contents to assign                                  
   TEMPLATE() template<template<class> class S, class ST>
   requires CT::Semantic<S<ST>> LANGULUS(INLINED)
   void HAND()::AssignSemantic(S<ST>&& rhs) {
      Destroy();
      CreateSemantic(rhs.Forward());
   }
   
   /// Swap two handles                                                       
   ///   @tparam RHS_EMBED - right handle embedness (deducible)               
   ///   @param rhs - right hand side                                         
   TEMPLATE() template<bool RHS_EMBED> LANGULUS(INLINED)
   void HAND()::Swap(Handle<T, RHS_EMBED>& rhs) {
      HandleLocal<T> tmp {Abandon(*this)};
      CreateSemantic(Abandon(rhs));
      rhs.CreateSemantic(Abandon(tmp));
   }

   /// Compare the contents of the handle with content                        
   ///   @param rhs - data to compare against                                 
   ///   @return true if contents are equal                                   
   TEMPLATE() template<class T1> requires CT::Inner::Comparable<T, T1>
   LANGULUS(INLINED) bool HAND()::Compare(const T1& rhs) const {
      return Get() == rhs;
   }

   /// Compare the contents of the handle with another handle                 
   ///   @param rhs - handle to compare against                               
   ///   @return true if contents are equal                                   
   TEMPLATE() template<class T1, bool RHS_EMBED>
   requires CT::Inner::Comparable<T, T1> LANGULUS(INLINED)
   bool HAND()::Compare(const Handle<T1, RHS_EMBED>& rhs) const {
      return Get() == rhs.Get();
   }

   /// Reset the handle, by dereferencing entry, and destroying value, if     
   /// entry has been fully dereferenced                                      
   /// Does absolutely nothing for dense handles, they are destroyed when     
   /// handle is destroyed                                                    
   ///   @tparam RESET - whether or not to reset pointers to null             
   TEMPLATE() template<bool RESET>
   void HAND()::Destroy() const {
      if constexpr (CT::Sparse<T>) {
         // Handle is sparse, we should handle each indirection layer   
         if (GetEntry()) {
            if (1 == GetEntry()->GetUses()) {
               LANGULUS_ASSUME(DevAssumes, Get(), "Null pointer");

               if constexpr (CT::Sparse<Deptr<T>>) {
                  // Pointer to pointer                                 
                  // Release all nested indirection layers              
                  HandleLocal<Deptr<T>> {Copy(*Get())}.Destroy();
               }
               else if constexpr (CT::Destroyable<T>) {
                  // Pointer to a complete, destroyable dense           
                  // Call the destructor                                
                  using DT = Decay<T>;
                  Get()->~DT();
               }

               Allocator::Deallocate(const_cast<Allocation*>(GetEntry()));
            }
            else const_cast<Allocation*>(GetEntry())->Free();
         }

         if constexpr (RESET)
            Create(nullptr, nullptr);
      }
      else if constexpr (EMBED) {
         // Handle is dense and embedded, we should call the remote     
         // destructor, but don't touch the entry, its irrelevant       
         if constexpr (CT::Destroyable<T>)
            Get().~T();
      }
   }
   
   /// Reset the handle, by dereferencing entry, and destroying value, if     
   /// entry has been fully dereferenced                                      
   /// Does absolutely nothing for dense handles, they are destroyed when     
   /// handle is destroyed                                                    
   ///   @tparam RESET - whether or not to reset pointers to null             
   ///   @param meta - the true type behind the pointer in this handle        
   TEMPLATE() template<bool RESET>
   void HAND()::DestroyUnknown(DMeta meta) const {
      if constexpr (CT::Sparse<T>) {
         LANGULUS_ASSUME(DevAssumes, meta->mIsSparse,
            "Provided meta must match T sparseness");

         if (GetEntry()) {
            if (1 == GetEntry()->GetUses()) {
               LANGULUS_ASSUME(DevAssumes, Get(), "Null pointer");

               if (meta->mDeptr->mIsSparse) {
                  // Release all nested indirection layers              
                  HandleLocal<Byte*> {
                     Copy(*reinterpret_cast<Byte**>(Get()))
                  }.DestroyUnknown(meta->mDeptr);
               }
               else if (meta->mDestructor) {
                  // Call the origin's destructor, if available         
                  meta->mDestructor(Get());
               }

               Allocator::Deallocate(const_cast<Allocation*>(GetEntry()));
            }
            else const_cast<Allocation*>(GetEntry())->Free();
         }

         if constexpr (RESET)
            Create(nullptr, nullptr);
      }
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef HAND
