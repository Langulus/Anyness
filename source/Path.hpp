///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
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

      static constexpr char Separator = '/';

      using Text::Text;

      Path(const Text&);
      Path(Text&&);

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
