///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Config.hpp"

namespace Langulus
{

   namespace A
   {
      /// An abstract iterator base                                           
      struct Iterator {};
   }

   namespace CT
   {
      /// Check if a type is an iterator                                      
      template<class... T>
      concept Iterator = (DerivedFrom<T, A::Iterator> and ...);
   }

} // namespace Langulus
