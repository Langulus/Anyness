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
   class Path : public Text {
   public:
      using Text::Text;
      //Path() = default;
      //Path(const Text& other);

      //Path(const Path&);

      /*template<Count C>
      Path(const Letter(&other)[C]) : Text {other} {}*/

      Path(Disowned<Path>&&) noexcept;
      Path(Abandoned<Path>&&) noexcept;

      //Path& operator = (const Text&);
      //Path& operator = (Text&&);

      //Path& operator = (const Path&);
      //Path& operator = (Path&&);

      using Text::operator =;
      Path& operator = (Disowned<Path>&&);
      Path& operator = (Abandoned<Path>&&);

   public:
      NOD() Path Clone() const;
      NOD() Text GetExtension() const;
      NOD() Path GetDirectory() const;
      NOD() Path GetFilename() const;
      NOD() Path operator / (const Text&) const;
      Path& operator /= (const Text&);
   };

} // namespace Langulus::Anyness
