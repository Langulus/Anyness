///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Referenced.hpp"

using namespace Langulus;
using namespace Anyness;


Referenced::~Referenced() {
   mReferences = 0;
}

void Referenced::Keep() const IF_UNSAFE(noexcept) {
   LANGULUS_ASSUME(DevAssumes, mReferences > 0,
      "Reference count resurrection");
   ++mReferences;
}

Count Referenced::Free() const IF_UNSAFE(noexcept) {
   LANGULUS_ASSUME(DevAssumes, mReferences > 1,
      "Last dereference is reserved for destructor only");
   return --mReferences;
}

Count Referenced::GetReferences() const noexcept {
   return mReferences;
}