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
   LANGULUS(ALWAYSINLINE)
   constexpr HAND()::Handle(S&& other) noexcept requires (!EMBED) {
      using ST = TypeOf<S>;

      if constexpr (CT::Handle<ST>) {
         static_assert(CT::Exact<T, TypeOf<ST>>, "Type mismatch");

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon a handle                        
            SemanticAssign<T>(mValue, S::Nest(other.mValue.Get()));

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               if constexpr (S::Keep || S::Move)
                  mEntry = other.mValue.GetEntry();
               else
                  mEntry = nullptr;
            #endif

            if constexpr (S::Move) {
               // Reset remote entry, when moving                       
               IF_LANGULUS_MANAGED_MEMORY(other.mValue.GetEntry() = nullptr);

               // Optionally reset remote value, if not abandoned       
               if constexpr (S::Keep)
                  other.mValue.Get() = nullptr;
            }
         }
         else {
            // Clone a handle                                           
            TODO();
         }
      }
      else {
         static_assert(CT::Exact<T, ST>, "Type mismatch");

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon a pointer                       
            // Since pointers don't have ownership, it's just a copy    
            // with an optional entry search, if not disowned           
            SemanticAssign<T>(mValue, S::Nest(other.mValue));

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               if constexpr (CT::Sparse<T> && CT::Allocatable<Deptr<T>> && (S::Keep || S::Move))
                  mEntry = Inner::Allocator::Find(MetaData::Of<Deptr<T>>(), mValue);
               else
                  mEntry = nullptr;
            #endif
         }
         else {
            // Clone a pointer                                          
            TODO();
         }
      }
   }

#if LANGULUS_FEATURE(MANAGED_MEMORY)
   /// Create an embedded handle                                              
   ///   @param v - a reference to the element                                
   ///   @param e - a reference to the element's entry                        
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   constexpr HAND()::Handle(T& v, Inner::Allocation*& e) SAFETY_NOEXCEPT() requires (EMBED && CT::Sparse<T>)
      : mValue {&v}
      , mEntry {&e} {}
      
   /// Create an embedded handle                                              
   ///   @param v - a reference to the element                                
   ///   @param e - the entry (optional)                                      
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   constexpr HAND()::Handle(T& v, Inner::Allocation* e) SAFETY_NOEXCEPT() requires (EMBED && CT::Dense<T>)
      : mValue {&v}
      , mEntry {e} {}
      
   /// Create a standalone handle                                             
   ///   @param v - the element                                               
   ///   @param e - the entry (optional)                                      
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   constexpr HAND()::Handle(T&& v, Inner::Allocation* e) SAFETY_NOEXCEPT() requires (!EMBED)
      : mValue {Forward<T>(v)}
      , mEntry {e} {}
#else
   /// Semantically construct a handle from content reference                 
   ///   @attention handles have no ownership, so no referencing happens      
   ///   @param other - the value to use for construction                     
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   constexpr HAND()::Handle(T& other) noexcept requires EMBED
      : mValue {&other} {}
#endif

   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   constexpr bool HAND()::operator == (const T* rhs) const noexcept requires EMBED {
      return mValue == rhs;
   }
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator ++ () noexcept requires EMBED {
      ++mValue;
      IF_LANGULUS_MANAGED_MEMORY(if constexpr (CT::Sparse<T>) ++mEntry);
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator -- () noexcept requires EMBED {
      --mValue;
      IF_LANGULUS_MANAGED_MEMORY(if constexpr (CT::Sparse<T>) --mEntry);
      return *this;
   }
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator += (Offset offset) noexcept requires EMBED {
      mValue += offset;
      IF_LANGULUS_MANAGED_MEMORY(if constexpr (CT::Sparse<T>) mEntry += offset);
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator -= (Offset offset) noexcept requires EMBED {
      mValue -= offset;
      IF_LANGULUS_MANAGED_MEMORY(if constexpr (CT::Sparse<T>) mEntry -= offset);
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @return the previous value of the handle                             
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND() HAND()::operator ++ (int) noexcept requires EMBED {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Suffix decrement operator                                              
   ///   @return the previous value of the handle                             
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND() HAND()::operator -- (int) noexcept requires EMBED {
      const auto backup = *this;
      operator -- ();
      return backup;
   }
      
   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND() HAND()::operator + (Offset offset) noexcept requires EMBED {
      auto backup = *this;
      return backup += offset;
   }

   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND() HAND()::operator - (Offset offset) noexcept requires EMBED {
      auto backup = *this;
      return backup -= offset;
   }

   /// Get a reference to the contents                                        
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   T& HAND()::Get() const noexcept {
      if constexpr (Embedded)
         return const_cast<T&>(*mValue);
      else
         return const_cast<T&>(mValue);
   }
   
#if LANGULUS_FEATURE(MANAGED_MEMORY)
   /// Get the entry                                                          
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Inner::Allocation*& HAND()::GetEntry() const noexcept {
      if constexpr (Embedded && CT::Sparse<T>)
         return const_cast<Inner::Allocation*&>(*mEntry);
      else
         return const_cast<Inner::Allocation*&>(mEntry);
   }
#endif

   /// Assign a new pointer and entry at the handle                           
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   void HAND()::New(T pointer, Inner::Allocation* IF_LANGULUS_MANAGED_MEMORY(entry)) noexcept requires CT::Sparse<T> {
      Get() = pointer;
      IF_LANGULUS_MANAGED_MEMORY(GetEntry() = entry);
   }
   
   /// Assign a new pointer and entry at the handle                           
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   void HAND()::New(T&& pointer, Inner::Allocation* IF_LANGULUS_MANAGED_MEMORY(entry)) noexcept requires CT::Dense<T> {
      Get() = Forward<T>(pointer);
      IF_LANGULUS_MANAGED_MEMORY(GetEntry() = entry);
   }

   /// Semantically assign anything at the handle                             
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   void HAND()::New(S&& rhs) {
      using ST = TypeOf<S>;

      if constexpr (S::Shallow) {
         // Do a copy/disown/abandon/move                               
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

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               if constexpr (S::Keep || S::Move)
                  GetEntry() = rhs.mValue.GetEntry();
               else
                  GetEntry() = nullptr;
            #endif
         }
         else {
            static_assert(CT::Exact<T, ST>, "Type mismatch");
            HandleLocal<T> rhsh {rhs.Forward()};
            Get() = rhsh.Get();
            IF_LANGULUS_MANAGED_MEMORY(GetEntry() = rhsh.GetEntry());
         }
      }
      else if constexpr (CT::Dense<T>) {
         // Do a clone inside a dense handle                            
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
         IF_LANGULUS_MANAGED_MEMORY(GetEntry() = entry);
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
   LANGULUS(ALWAYSINLINE)
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
         IF_LANGULUS_MANAGED_MEMORY(GetEntry() = entry);
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
   LANGULUS(ALWAYSINLINE)
   void HAND()::Assign(S&& rhs) {
      Destroy();
      New(rhs.Forward());
   }
   
   /// Swap two handles                                                       
   ///   @tparam RHS_EMBED - right handle embedness (deducible)               
   ///   @param rhs - right hand side                                         
   TEMPLATE()
   template<bool RHS_EMBED>
   LANGULUS(ALWAYSINLINE)
   void HAND()::Swap(Handle<T, RHS_EMBED>& rhs) {
      HandleLocal<T> tmp {Abandon(*this)};
      New(Abandon(rhs));
      rhs.New(Abandon(tmp));
   }

   /// Compare the contents of the handle                                     
   ///   @param rhs - data to compare against                                 
   ///   @return true if contents are equal                                   
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   bool HAND()::Compare(const T& rhs) const {
      return Get() == rhs;
   }

   /// Reset the handle, by dereferencing entry, and destroying value, if     
   /// entry has been fully dereferenced                                      
   /// Does absolutely nothing for dense handles, they are destroyed when     
   /// handle is destroyed                                                    
   ///   @tparam RESET - whether or not to reset pointers to null             
   TEMPLATE()
   template<bool RESET>
   LANGULUS(ALWAYSINLINE)
   void HAND()::Destroy() const {
      #if LANGULUS_FEATURE(MANAGED_MEMORY)
      if constexpr (CT::Sparse<T>) {
         if (GetEntry()) {
            if (1 == GetEntry()->GetUses()) {
               LANGULUS_ASSUME(DevAssumes, Get(), "Null pointer");

               if constexpr (CT::Sparse<Deptr<T>>) {
                  // Release all nested indirection layers              
                  HandleLocal<Deptr<T>> {
                     Langulus::Copy(*Get())
                  }.Destroy();
               }
               else if constexpr (CT::Destroyable<T>) {
                  // Call the destructor                                
                  Get()->~Decay<T>();
               }

               Inner::Allocator::Deallocate(GetEntry());
            }
            else GetEntry()->Free();
         }
      }
      #endif

      if constexpr (RESET && CT::Sparse<T>)
         New(nullptr, nullptr);
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef HAND
