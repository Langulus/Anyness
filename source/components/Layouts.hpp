///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Components.hpp"
#include <Core/Types.hpp>


///                                                                           
/// Layouts are component compositions, designed to describe an archetypal    
/// container. Each call to the interface of a container goes through all     
/// the components, in the order they appear.                                 
///                                                                           
namespace Langulus::Anyness::Layout
{

   using Handle = Types<
      Component::Data,
      Component::Source
   >;

   using Block = Types<
      Component::Data,
      Component::Source,
      Component::Range,
      Component::Meta,
      Component::Missing,
      Component::Or,
      Component::Compress,
      Component::Encrypt,
      Component::Constant,
      Component::LockType
   >;

} // namespace Langulus::Anyness::Layout