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
   class LANGULUS_API(ANYNESS) Referenced {
      mutable Count mReferences = 1;

   public:
      ~Referenced();

      void  Keep() const IF_UNSAFE(noexcept);
      Count Free() const IF_UNSAFE(noexcept);
      Count GetReferences() const noexcept;
   };

} // namespace Langulus::Anyness
