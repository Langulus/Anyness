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
      
   /// Prefix increment operator                                              
   ///   @return the next handle                                              
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()& HAND()::operator ++ () noexcept requires EMBED {
      ++mValue;
      ++mEntry;
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

   /// Implicitly cast to the item                                            
   TEMPLATE()
   LANGULUS(ALWAYSINLINE)
   HAND()::operator T () const noexcept {
      return *mValue;
   }

   /// Reset the handle, by dereferencing entry, and destroying value, if     
   /// entry has been fully dereferenced                                      
   ///   @tparam RESET - whether or not to reset pointers to null             
   TEMPLATE()
   template<bool RESET>
   LANGULUS(ALWAYSINLINE)
   void HAND()::Destroy() const {
      if (*mEntry) {
         if (1 == (*mEntry)->GetUses()) {
            LANGULUS_ASSUME(DevAssumes, *mValue != nullptr, "Null pointer");

            if constexpr (CT::Sparse<Deptr<T>>) {
               // Release all nested indirection layers                 
               auto subptr = **mValue;
               auto subent = Inner::Allocator::Find(MetaData::Of<Deptr<T>>(), subptr);
               Handle<Deptr<T>> {&subptr, &subent}.Destroy<false>();
            }
            else if (CT::Destroyable<T>) {
               // Call the destructor                                   
               (**mValue).~Decay<T>();
            }

            Inner::Allocator::Deallocate(*mEntry);
         }
      }
      else (*mEntry)->Free();

      if constexpr (RESET) {
         *mValue = nullptr;
         *mEntry = nullptr;
      }
   }

   /// Create a handle on the stack, using the provided semantic              
   ///   @tparam T - the type to instantiate                                  
   ///   @tparam S - the semantic to use (deducible)                          
   ///   @param value - the constructor arguments and the semantic            
   ///   @return the handle                                                   
   template<class T, CT::Semantic S>
   NOD() LANGULUS(ALWAYSINLINE)
   auto SemanticMakeHandle(S&& value) {
      if constexpr (CT::Sparse<T>) {
         if constexpr (!S::Shallow) {
            // Clone                                                    
            TODO();
         }
         else {
            using A = TypeOf<S>;
            if constexpr (CT::Handle<A>) {
               // Copy/Move handle                                      
            }
            else if constexpr (CT::Sparse<A>) {
               // Copy/Move unknown pointer                             
               return Handle<T> {
                  value.mValue,
                  S::Keep || S::Move
                     ? Inner::Allocator::Find(MetaData::Of<Deptr<A>>(), value.mValue)
                     : nullptr
               };
            }
         }
      }
      else return SemanticMake<T>(value.Forward());
   }

   /// Invoke a placement new inside a handle                                 
   ///   @tparam T - the type to instantiate                                  
   ///   @tparam H - the handle and type to place in (deducible)              
   ///   @tparam S - the semantic to use (deducible)                          
   ///   @param handle - the handle to place in                               
   ///   @param value - the constructor arguments and the semantic            
   ///   @return the handle                                                   
   template<class T, class H, CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   void SemanticNewHandle(H&& handle, S&& value) {
      if constexpr (CT::Handle<H>)
         SemanticNew<T>(handle.mValue, value.Forward());
      else if constexpr (CT::Sparse<H>)
         SemanticNew<H>(handle, value.Forward());
      else
         SemanticNew<H>(&handle, value.Forward());
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef HAND
