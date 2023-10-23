///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "inner/Config.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   A tiny class used as base to referenced types                        
   ///                                                                        
   /// Provides the interface to be considered CT::Referencable               
   /// The destructor of this type guarantees, that after destruction, the    
   /// member mReferences is zeroed.                                          
   ///                                                                        
   class Referenced {
      mutable Count mReferences = 1;

   public:
      LANGULUS(INLINED)
      constexpr ~Referenced() noexcept {
         mReferences = 0;
      }

      LANGULUS(INLINED)
      void Keep() const IF_UNSAFE(noexcept) {
         LANGULUS_ASSUME(DevAssumes, mReferences > 0,
            "Reference count resurrection");
         ++mReferences;
      }

      LANGULUS(INLINED)
      Count Free() const IF_UNSAFE(noexcept) {
         LANGULUS_ASSUME(DevAssumes, mReferences > 1,
            "Last dereference is reserved for destructor only");
         return --mReferences;
      }

      LANGULUS(INLINED)
      constexpr Count GetReferences() const noexcept {
         return mReferences;
      }
   };

} // namespace Langulus::Anyness
