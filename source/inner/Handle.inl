///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Handle.hpp"

#define TEMPLATE() template<class T, bool EMBED>
#define HAND() Handle<T, EMBED>

namespace Langulus::Anyness
{

   /// Semantically construct a handle from pointer/handle                    
   ///   @attention handles have no ownership, so no referencing happens      
   ///   @tparam S - the semantic and type to use for the handle              
   ///   @param other - the value to use for construction                     
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   constexpr HAND()::Handle(S&& other) noexcept requires (!EMBED) {
      using ST = TypeOf<S>;

      if constexpr (CT::Handle<ST>) {
         using HT = TypeOf<ST>;
         static_assert(CT::Exact<T, HT>, "Type mismatch");

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon a handle                        
            SemanticAssign<T>(mValue, S::Nest(other.mValue.Get()));

            if constexpr (S::Keep || S::Move)
               mEntry = other.mValue.GetEntry();
            else
               mEntry = nullptr;

            if constexpr (S::Move) {
               // Reset remote entry, when moving                       
               other.mValue.GetEntry() = nullptr;

               // Optionally reset remote value, if not abandoned       
               if constexpr (S::Keep && CT::Sparse<T, HT>)
                  other.mValue.Get() = nullptr;
            }
         }
         else {
            // Clone a handle                                           
            TODO();
         }
      }
      else {
         // Assigning mutable pointer to a const handle is allowed      
         static_assert(
               (CT::Mutable<Deptr<T>> && CT::Exact<T, ST>)
            || (CT::Constant<Deptr<T>> && CT::Exact<T, const Deptr<ST>*>),
            "Type mismatch"
         );

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon a pointer                       
            // Since pointers don't have ownership, it's just a copy    
            // with an optional entry search, if not disowned, and if   
            // managed memory is enabled                                
            SemanticAssign<T>(mValue, S::Nest(other.mValue));

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               if constexpr (CT::Sparse<T> && CT::Allocatable<Deptr<T>> && (S::Keep || S::Move))
                  mEntry = Inner::Allocator::Find(MetaData::Of<Deptr<T>>(), mValue);
               else
                  mEntry = nullptr;
            #else
               mEntry = nullptr;
            #endif
         }
         else {
            // Clone a pointer                                          
            TODO();
         }
      }
   }

   /// Create an embedded handle                                              
   ///   @param v - a reference to the element                                
   ///   @param e - a reference to the element's entry                        
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr HAND()::Handle(T& v, Inner::Allocation*& e) SAFETY_NOEXCEPT() requires (EMBED && CT::Sparse<T>)
      : mValue {&v}
      , mEntry {&e} {}
      
   /// Create an embedded handle                                              
   ///   @param v - a reference to the element                                
   ///   @param e - the entry (optional)                                      
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr HAND()::Handle(T& v, Inner::Allocation* e) SAFETY_NOEXCEPT() requires (EMBED && CT::Dense<T>)
      : mValue {&v}
      , mEntry {e} {}
      
   /// Create a standalone handle                                             
   ///   @param v - the element                                               
   ///   @param e - the entry (optional)                                      
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr HAND()::Handle(T&& v, Inner::Allocation* e) SAFETY_NOEXCEPT() requires (!EMBED)
      : mValue {Forward<T>(v)}
      , mEntry {e} {}

   TEMPLATE()
   LANGULUS(INLINED)
   constexpr bool HAND()::operator == (const T* rhs) const noexcept requires (EMBED) {
      return mValue == rhs;
   }
      
   TEMPLATE()
   LANGULUS(INLINED)
   constexpr bool HAND()::operator == (const HAND()& rhs) const noexcept requires (EMBED) {
      return mValue == rhs.mValue;
   }
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(INLINED)
   HAND()& HAND()::operator ++ () noexcept requires (EMBED) {
      ++mValue;
      if constexpr (CT::Sparse<T>)
         ++mEntry;
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(INLINED)
   HAND()& HAND()::operator -- () noexcept requires (EMBED) {
      --mValue;
      if constexpr (CT::Sparse<T>)
         --mEntry;
      return *this;
   }
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(INLINED)
   HAND()& HAND()::operator += (Offset offset) noexcept requires (EMBED) {
      mValue += offset;
      if constexpr (CT::Sparse<T>)
         mEntry += offset;
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(INLINED)
   HAND()& HAND()::operator -= (Offset offset) noexcept requires (EMBED) {
      mValue -= offset;
      if constexpr (CT::Sparse<T>)
         mEntry -= offset;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @return the previous value of the handle                             
   TEMPLATE()
   LANGULUS(INLINED)
   HAND() HAND()::operator ++ (int) noexcept requires (EMBED) {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Suffix decrement operator                                              
   ///   @return the previous value of the handle                             
   TEMPLATE()
   LANGULUS(INLINED)
   HAND() HAND()::operator -- (int) noexcept requires (EMBED) {
      const auto backup = *this;
      operator -- ();
      return backup;
   }
      
   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   TEMPLATE()
   LANGULUS(INLINED)
   HAND() HAND()::operator + (Offset offset) noexcept requires (EMBED) {
      auto backup = *this;
      return backup += offset;
   }

   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   TEMPLATE()
   LANGULUS(INLINED)
   HAND() HAND()::operator - (Offset offset) noexcept requires (EMBED) {
      auto backup = *this;
      return backup -= offset;
   }

   /// Get a reference to the contents                                        
   TEMPLATE()
   LANGULUS(INLINED)
   T& HAND()::Get() const noexcept {
      if constexpr (Embedded)
         return const_cast<T&>(*mValue);
      else
         return const_cast<T&>(mValue);
   }
   
   /// Get the entry                                                          
   TEMPLATE()
   LANGULUS(INLINED)
   Inner::Allocation*& HAND()::GetEntry() const noexcept {
      if constexpr (Embedded && CT::Sparse<T>)
         return const_cast<Inner::Allocation*&>(*mEntry);
      else
         return const_cast<Inner::Allocation*&>(mEntry);
   }

   /// Assign a new pointer and entry at the handle                           
   TEMPLATE()
   LANGULUS(INLINED)
   void HAND()::New(T pointer, Inner::Allocation* entry) noexcept requires CT::Sparse<T> {
      Get() = pointer;
      GetEntry() = entry;
   }
   
   /// Assign a new pointer and entry at the handle                           
   TEMPLATE()
   LANGULUS(INLINED)
   void HAND()::New(T&& pointer, Inner::Allocation* entry) noexcept requires CT::Dense<T> {
      Get() = Forward<T>(pointer);
      GetEntry() = entry;
   }

   /// Semantically assign anything at the handle, ignoring the old handle    
   ///   @tparam S - the semantic to use                                      
   ///   @param rhs - what are we assigning                                   
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   void HAND()::New(S&& rhs) {
      using ST = TypeOf<S>;

      if constexpr (S::Shallow && CT::Sparse<T>) {
         // Do a copy/disown/abandon/move sparse LHS                    
         if constexpr (CT::Handle<ST>) {
            // RHS is a handle                                          
            using HT = TypeOf<ST>;
            static_assert(CT::Same<T, HT>, "Type mismatch");

            if constexpr (CT::Dense<HT>) {
               if constexpr (CT::Dense<T>)
                  // Dense = Dense                                      
                  SemanticNew<T>(&Get(), S::Nest(rhs.mValue.Get()));
               else
                  // Sparse = Dense                                     
                  Get() = &rhs.mValue.Get();
            }
            else {
               if constexpr (CT::Dense<T>)
                  // Dense = Sparse                                     
                  SemanticNew<T>(&Get(), S::Nest(*rhs.mValue.Get()));
               else
                  // Sparse = Sparse                                    
                  Get() = rhs.mValue.Get();
            }

            if constexpr (S::Keep || S::Move)
               GetEntry() = rhs.mValue.GetEntry();
            else
               GetEntry() = nullptr;

            if constexpr (S::Move) {
               // We're moving RHS, so we need to clear it up           
               if constexpr (S::Keep && CT::Sparse<T, HT>) {
                  // Clear the value only if we're not abandoning RHS   
                  // (and if the value is a pointer)                    
                  rhs.mValue.Get() = nullptr;
               }

               // Clearing entry is mandatory, because we're            
               // transferring the ownership                            
               rhs.mValue.GetEntry() = nullptr;
            }
            else if constexpr (S::Keep) {
               // Copying RHS, but keep it only if not disowning it     
               if (GetEntry())
                  GetEntry()->Keep();
            }
         }
         else if constexpr (CT::Sparse<T> && CT::Exact<ST, ::std::nullptr_t>) {
            // RHS is a simple nullptr                                  
            Get() = nullptr;
            GetEntry() = nullptr;
         }
         else {
            // RHS is not a handle, but we'll wrap it in a handle, in   
            // order to find its entry (if managed memory is enabled)   
            // Assigning mutable pointer to a const handle is allowed   
            static_assert(
                  (CT::Mutable<Deptr<T>> && CT::Exact<T, ST>)
               || (CT::Constant<Deptr<T>> && CT::Exact<T, const Deptr<ST>*>),
               "Type mismatch"
            );

            HandleLocal<T> rhsh {rhs.Forward()};
            Get() = rhsh.Get();
            GetEntry() = rhsh.GetEntry();

            if constexpr (S::Keep) {
               if (GetEntry())
                  GetEntry()->Keep();
            }
         }
      }
      else if constexpr (CT::Dense<T>) {
         // Do a copy/disown/abandon/move/clone inside a dense handle   
         if constexpr (CT::Handle<ST>) {
            static_assert(CT::Exact<T, TypeOf<ST>>, "Type mismatch");
            SemanticNew<T>(&Get(), S::Nest(rhs.mValue.Get()));
         }
         else {
            static_assert(CT::Exact<T, ST>, "Type mismatch");
            SemanticNew<T>(&Get(), rhs.Forward());
         }
      }
      else if constexpr (CT::Dense<Deptr<T>>) {
         // Do a clone                                                  
         using DT = Decay<T>;
         auto meta = MetaData::Of<DT>();
         auto entry = Inner::Allocator::Allocate(meta->RequestSize(1).mByteSize);
         auto pointer = entry->template As<DT>();

         if constexpr (CT::Handle<ST>) {
            static_assert(CT::Exact<T, TypeOf<ST>>, "Type mismatch");
            SemanticNew<DT>(pointer, S::Nest(*rhs.mValue.Get()));
         }
         else {
            static_assert(CT::Exact<T, ST>, "Type mismatch");
            SemanticNew<DT>(pointer, S::Nest(*rhs.mValue));
         }

         Get() = pointer;
         GetEntry() = entry;
      }
      else {
         //clone an indirection layer by nesting semanticnewhandle      
         TODO();
      }
   }
   
   /// Semantically assign anything at the handle                             
   ///   @tparam S - semantic to use for assignment (deducible)               
   ///   @param meta - the reflected type to use for assignment               
   ///   @param rhs - the data to assign                                      
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   void HAND()::NewUnknown(DMeta meta, S&& rhs) {
      if constexpr (S::Shallow) {
         // Do a copy/disown/abandon/move                               
         New(rhs.Forward());
      }
      else if (!meta->mDeptr->mIsSparse) {
         // Do a clone                                                  
         const auto bytesize = meta->mDeptr->RequestSize(1).mByteSize;
         auto entry = Inner::Allocator::Allocate(bytesize);
         auto pointer = entry->GetBlockStart();

         if constexpr (CT::Handle<TypeOf<S>>)
            SemanticNewUnknown(meta->mDeptr, pointer, S::Nest(rhs.mValue.mValue));
         else
            SemanticNewUnknown(meta->mDeptr, pointer, rhs.Forward());

         Get() = pointer;
         GetEntry() = entry;
      }
      else {
         //clone an indirection layer by nesting semanticnewhandle      
         TODO();
      }
   }

   /// Dereference/destroy the current handle contents, and set new ones      
   ///   @tparam S - the semantic to use for the assignment                   
   ///   @param rhs - new contents to assign                                  
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   void HAND()::Assign(S&& rhs) {
      Destroy();
      New(rhs.Forward());
   }
   
   /// Swap two handles                                                       
   ///   @tparam RHS_EMBED - right handle embedness (deducible)               
   ///   @param rhs - right hand side                                         
   TEMPLATE()
   template<bool RHS_EMBED>
   LANGULUS(INLINED)
   void HAND()::Swap(Handle<T, RHS_EMBED>& rhs) {
      HandleLocal<T> tmp {Abandon(*this)};
      New(Abandon(rhs));
      rhs.New(Abandon(tmp));
   }

   /// Compare the contents of the handle with content                        
   ///   @param rhs - data to compare against                                 
   ///   @return true if contents are equal                                   
   TEMPLATE()
   LANGULUS(INLINED)
   bool HAND()::Compare(const T& rhs) const {
      return Get() == rhs;
   }

   /// Compare the contents of the handle with another handle                 
   ///   @param rhs - handle to compare against                               
   ///   @return true if contents are equal                                   
   TEMPLATE()
   template<bool RHS_EMBED>
   LANGULUS(INLINED)
   bool HAND()::Compare(const Handle<T, RHS_EMBED>& rhs) const {
      return Get() == rhs.Get();
   }

   /// Reset the handle, by dereferencing entry, and destroying value, if     
   /// entry has been fully dereferenced                                      
   /// Does absolutely nothing for dense handles, they are destroyed when     
   /// handle is destroyed                                                    
   ///   @tparam RESET - whether or not to reset pointers to null             
   TEMPLATE()
   template<bool RESET>
   void HAND()::Destroy() const {
      if constexpr (CT::Sparse<T>) {
         // Handle is sparse, we should handle each indirection layer   
         if (GetEntry()) {
            if (1 == GetEntry()->GetUses()) {
               LANGULUS_ASSUME(DevAssumes, Get(), "Null pointer");

               if constexpr (CT::Sparse<Deptr<T>>) {
                  // Pointer to pointer                                 
                  // Release all nested indirection layers              
                  HandleLocal<Deptr<T>> {
                     Langulus::Copy(*Get())
                  }.Destroy();
               }
               else if constexpr (CT::Complete<Decay<T>>) {
                  if constexpr (!CT::POD<T> && CT::Destroyable<T>) {
                     // Pointer to a complete, destroyable dense        
                     // Call the destructor                             
                     using DT = Decay<T>;
                     Get()->~DT();
                  }
               }

               Inner::Allocator::Deallocate(GetEntry());
            }
            else GetEntry()->Free();
         }

         if constexpr (RESET)
            New(nullptr, nullptr);
      }
      else if constexpr (EMBED) {
         // Handle is dense and embedded, we should call the remote     
         // destructor, but don't touch the entry, its irrelevant       
         if constexpr (!CT::POD<T> && CT::Destroyable<T>)
            Get().~T();
      }
   }
   
   /// Reset the handle, by dereferencing entry, and destroying value, if     
   /// entry has been fully dereferenced                                      
   /// Does absolutely nothing for dense handles, they are destroyed when     
   /// handle is destroyed                                                    
   ///   @tparam RESET - whether or not to reset pointers to null             
   ///   @param meta - the true type behind the Byte pointer in handle        
   TEMPLATE()
   template<bool RESET>
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
                     Langulus::Copy(*reinterpret_cast<Byte**>(Get()))
                  }.DestroyUnknown(meta->mDeptr);
               }
               else if (!meta->mIsPOD && meta->mDeptr->mDestructor) {
                  // Call the destructor                                
                  meta->mDeptr->mDestructor(Get());
               }

               Inner::Allocator::Deallocate(GetEntry());
            }
            else GetEntry()->Free();
         }

         if constexpr (RESET)
            New(nullptr, nullptr);
      }
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef HAND
