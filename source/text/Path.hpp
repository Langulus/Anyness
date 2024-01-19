///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Text.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   File path container                                                  
   ///                                                                        
   struct Path : Text {
      LANGULUS_BASES(Text);
      LANGULUS(FILES) "";

      static constexpr char Separator = '/';

      using Text::Text;
      using Text::operator +=;

      Path(const Text&);
      Path(Text&&);
      Path(Describe&&);

      NOD() Text GetExtension() const;
      NOD() Path GetDirectory() const;
      NOD() Path GetFilename() const;

      NOD() Path operator / (const Text&) const;
      Path& operator /= (const Text&);
   };

} // namespace Langulus::Anyness

namespace Langulus
{

   Anyness::Path operator "" _path(const char*, ::std::size_t);

} // namespace Langulus
