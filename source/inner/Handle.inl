///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Handle.hpp"

#define TEMPLATE() template<CT::Sparse T, bool EMBED>
#define HAND() Handle<T, EMBED>

namespace Langulus::Anyness
{

   /// Semantically construct a handle from pointer/handle                    
   ///   @param other - the pointer/handle to use for construction            
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr HAND()::Handle(S&& other) noexcept {
      static_assert(!Embedded, "Handle mustn't be embedded for this constructor");

      if constexpr (CT::Handle<TypeOf<S>>) {
         static_assert(CT::Exact<T, TypeOf<TypeOf<S>>>, "Type mismatch");

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon a handle                        
            mValue = other.mValue.Get();
            if constexpr (S::Keep || S::Move)
               mEntry = other.mValue.GetEntry();
            else
               mEntry = nullptr;

            if constexpr (S::Move) {
               // Reset remote entry, when moving                       
               other.mValue.SetEntry(nullptr);

               // Optionally reset remote value, if not abandoned       
               if constexpr (S::Keep)
                  other.mValue.Set(nullptr);
            }
         }
         else {
            // Clone a handle                                           
            TODO();
         }
      }
      else {
         static_assert(CT::Exact<T, TypeOf<S>>, "Type mismatch");

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon a pointer                       
            // Since pointers don't have ownership, it's just a copy    
            // with an optional entry search, if not disowned           
            mValue = other.mValue;
            if constexpr (CT::Allocatable<Deptr<T>> && (S::Keep || S::Move))
               mEntry = Inner::Allocator::Find(MetaData::Of<Deptr<T>>(), mValue);
            else
               mEntry = nullptr;
         }
         else {
            // Clone a pointer                                          
            TODO();
         }
      }
   }

   /// Create a handle                                                        
   ///   @param v - the pointer to element                                    
   ///   @param e - the pointer to element's entry                            
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   constexpr HAND()::Handle(decltype(mValue) v, decltype(mEntry) e) SAFETY_NOEXCEPT()
      : mValue {v}
      , mEntry {e} {
      LANGULUS_ASSUME(DevAssumes, v != nullptr, "Bad value pointer");
      LANGULUS_ASSUME(DevAssumes, e != nullptr, "Bad entry pointer");
   }
      
   /// Prefix dereference operator does nothing. Handles are interchangable   
   /// with pointers and often used in the same place as them, but handles    
   /// should remain dense and be forwarded, instead of dereferenced.         
   ///   @return this handle                                                  
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator * () noexcept {
      return *this;
   }
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator ++ () noexcept requires EMBED {
      ++mValue;
      ++mEntry;
      return *this;
   }

   /// Prefix decrement operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator -- () noexcept requires EMBED {
      --mValue;
      --mEntry;
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
   HAND() HAND()::operator + (int offset) noexcept requires EMBED {
      auto backup = *this;
      backup.mValue += offset;
      backup.mEntry += offset;
      return backup;
   }

   /// Offset the handle                                                      
   ///   @param offset - the offset to apply                                  
   ///   @return the offsetted handle                                         
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND() HAND()::operator - (int offset) noexcept requires EMBED {
      auto backup = *this;
      backup.mValue -= offset;
      backup.mEntry -= offset;
      return backup;
   }

   /// Get the pointer                                                        
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   T HAND()::Get() const noexcept {
      if constexpr (Embedded)
         return *mValue;
      else
         return mValue;
   }

   /// Get the entry                                                          
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   Inner::Allocation* HAND()::GetEntry() const noexcept {
      if constexpr (Embedded)
         return *mEntry;
      else
         return mEntry;
   }
   
   /// Set the pointer                                                        
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   void HAND()::Set(T value) noexcept {
      if constexpr (Embedded)
         *mValue = value;
      else
         mValue = value;
   }

   /// Set the entry                                                          
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   void HAND()::SetEntry(Inner::Allocation* entry) noexcept {
      if constexpr (Embedded)
         *mEntry = entry;
      else
         mEntry = entry;
   }

   /// Reset the handle, by dereferencing entry, and destroying value, if     
   /// entry has been fully dereferenced                                      
   ///   @tparam RESET - whether or not to reset pointers to null             
   TEMPLATE()
   template<bool RESET>
   LANGULUS(ALWAYSINLINE)
   void HAND()::Destroy() const {
      if (GetEntry()) {
         if (1 == GetEntry()->GetUses()) {
            LANGULUS_ASSUME(DevAssumes, Get(), "Null pointer");

            if constexpr (CT::Sparse<Deptr<T>>) {
               // Release all nested indirection layers                 
               Handle<Deptr<T>, false> {
                  Langulus::Copy(*Get())
               }.template Destroy<false>();
            }
            else if constexpr (CT::Destroyable<T>) {
               // Call the destructor                                   
               Get()->~Decay<T>();
            }

            Inner::Allocator::Deallocate(GetEntry());
         }
      }
      else GetEntry()->Free();

      if constexpr (RESET) {
         if constexpr (Embedded) {
            *mValue = nullptr;
            *mEntry = nullptr;
         }
         else {
            mValue = nullptr;
            mEntry = nullptr;
         }
      }
   }

   /// Create an unembedded handle on the stack, using the provided semantic  
   ///   @tparam T - the type to instantiate                                  
   ///   @tparam S - the semantic to use (deducible)                          
   ///   @param value - the constructor arguments and the semantic            
   ///   @return the handle                                                   
   template<class T, CT::Semantic S>
   NOD() LANGULUS(ALWAYSINLINE)
   auto SemanticMakeHandle(S&& value) {
      if constexpr (CT::Sparse<T>) {
         // Making a pointer, so a handle is required                   
         return Handle<T, false> {value.Forward()};
      }
      else {
         // Not making pointer, so use conventional semantic make       
         return SemanticMake<T>(value.Forward());
      }
   }

   /// Invoke a placement new inside a handle                                 
   /// Pretty much overwrites handles disregarding their old values           
   ///   @tparam T - the type to instantiate                                  
   ///   @tparam H - the handle and type to place in (deducible)              
   ///   @tparam S - the semantic to use (deducible)                          
   ///   @param handle - the handle to place in                               
   ///   @param value - the constructor arguments and the semantic            
   template<class T, class H, CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   void SemanticNewHandle(H&& handle, S&& value) {
      if constexpr (CT::Handle<H>) {
         // Destination is an embedded handle                           
         if constexpr (CT::Handle<TypeOf<S>>) {
            // Source is a handle, too                                  
            if constexpr (S::Shallow) {
               // Do a copy/disown/abandon/move                         
               handle.Set(value.mValue.Get());

               if constexpr (S::Keep || S::Move)
                  handle.SetEntry(value.mValue.GetEntry());
               else
                  handle.SetEntry(nullptr);
            }
            else {
               // Do a clone                                            
               TODO();
            }
         }
         else if constexpr (CT::Sparse<TypeOf<S>>) {
            // Source should be a pointer                               
            if constexpr (S::Shallow) {
               // Do a copy/disown/abandon/move                         
               handle.Set(value.mValue);

               using DT = Deptr<TypeOf<S>>;
               if constexpr (CT::Allocatable<DT> && (S::Keep || S::Move))
                  handle.SetEntry(Inner::Allocator::Find(MetaData::Of<DT>(), value.mValue));
               else
                  handle.SetEntry(nullptr);
            }
            else {
               // Do a clone                                            
               TODO();
            }
         }
         else LANGULUS_ERROR("Bad argument for in-handle semantic placement");
      }
      else if constexpr (CT::Sparse<H>) {
         // Destination is pointer, so just conventional SemanticNew    
         SemanticNew<T>(handle, value.Forward());
      }
      else LANGULUS_ERROR("Bad placement argument");
   }
   
   /// Assign new value to a handle, using the provided semantic              
   /// Overwrites embedded handles, by freeing their old values               
   ///   @tparam H - the handle/type to assign to (deducible)                 
   ///   @tparam S - the semantic to assign (deducible)                       
   ///   @param lhs - left hand side (what are we assigning to)               
   ///   @param rhs - right hand side (what are we assigning)                 
   template<class H, CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   void SemanticAssignHandle(H&& lhs, S&& rhs) {
      if constexpr (CT::Handle<H>) {
         static_assert(H::Embedded, "Handle must be embedded");

         // Destroy old stuff                                           
         lhs.template Destroy<false>();

         // Overwrite with new stuff                                    
         SemanticNewHandle<TypeOf<H>>(Forward<H>(lhs), rhs.Forward());
      }
      else if constexpr (CT::Sparse<H>) {
         // LHS is not a handle, so just conventional SemanticAssign    
         SemanticAssign(*lhs, rhs.Forward());
      }
      else LANGULUS_ERROR("Bad LHS type");
   }
   
   /// Swap two handles                                                       
   ///   @tparam LHS - left handle (deducible)                                
   ///   @tparam RHS - right handle (deducible)                               
   ///   @param lhs - left hand side                                          
   ///   @param rhs - right hand side                                         
   template<CT::NotSemantic LHS, CT::NotSemantic RHS>
   LANGULUS(ALWAYSINLINE)
   void SwapHandles(LHS&& lhs, RHS&& rhs) {
      if constexpr (CT::Handle<LHS, RHS>) {
         // Different kinds of handles are allowed, as long as their    
         // types are an exact match                                    
         static_assert(CT::Exact<TypeOf<LHS>, TypeOf<RHS>>,
            "Handle type mismatch");

         // Swap handles                                                
         // First make a temporary swapper on the stack                 
         using T = TypeOf<LHS>;
         auto tmp = SemanticMakeHandle<T>(Abandon(lhs));
         SemanticNewHandle<T>(Forward<LHS>(lhs), Abandon(rhs));
         SemanticNewHandle<T>(Forward<RHS>(rhs), Abandon(tmp));
      }
      else {
         // Value - Value swap                                          
         static_assert(CT::Same<LHS, RHS>, "Type mismatch");
         ::std::swap(DenseCast(lhs), DenseCast(rhs));
      }
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef HAND
