///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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
   /// member mReferences is zeroed                                           
   ///                                                                        
   class Referenced {
      Count mReferences {1};

   public:
      void Keep() const noexcept {
         LANGULUS_ASSUME(DevAssumes, mReferences > 0,
            "Reference count resurrection");
         ++const_cast<Count&>(mReferences);
      }

      Count Free() const SAFETY_NOEXCEPT() {
         LANGULUS_ASSUME(DevAssumes, mReferences > 1,
            "Last dereference is reserved for destructor only");
         return --const_cast<Count&>(mReferences);
      }

      Count GetReferences() const noexcept {
         return mReferences;
      }

      ~Referenced() {
         /*LANGULUS_ASSUME(DevAssumes, mReferences == 1,
            "Destroying a referenced element, that is still in use");*/
         mReferences = 0;
      }
   };

} // namespace Langulus::Anyness
