///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../blocks/Block.hpp"

namespace Langulus::A
{
   struct Handle {};
}

namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   A pointer & allocation pair                                          
   ///                                                                        
   /// Used as intermediate type when managed memory is enabled, to keep      
   /// track of pointers inserted to containers.                              
   ///                                                                        
   template<CT::Data T>
   struct Handle : A::Handle {
      LANGULUS(TYPED) T*;
      LANGULUS_BASES(A::Handle);

   protected: TESTING(public:)
      friend class Block;

      /// @cond show_protected                                                
      T*& mPointer;
      Inner::Allocation*& mEntry;
      /// @endcond show_protected                                             

   public:
      Handle() = delete;
      Handle(const Handle&) = delete;
      Handle(Handle&&) = delete;

      constexpr Handle(T*&, Inner::Allocation*&) noexcept;
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{
   template<class... T>
   concept Handle = (DerivedFrom<T, A::Handle> && ...);
}
