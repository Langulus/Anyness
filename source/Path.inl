///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Path.hpp"

namespace Langulus::Anyness
{

   /// Construct by copying a text container                                  
   ///   @param other - the container to copy                                 
   LANGULUS(ALWAYSINLINE)
   Path::Path(const Text& other)
      : Text {other} {}

   /// Construct by moving a text container                                   
   ///   @param other - the container to copy                                 
   LANGULUS(ALWAYSINLINE)
   Path::Path(Text&& other)
      : Text {Forward<Text>(other)} {}

   /// Return the lowercase file extension (the part after the last '.')      
   ///   @return a cloned text container with the extension                   
   LANGULUS(ALWAYSINLINE)
   Text Path::GetExtension() const {
      Offset offset {};
      if (Text::FindOffsetReverse(Text {'.'}, offset))
         return Text::Crop(offset + 1, mCount - offset - 1).Lowercase();
      return {};
   }

   /// Return the directory part of the path                                  
   ///   @return the directory part, including the last '/'                   
   LANGULUS(ALWAYSINLINE)
   Path Path::GetDirectory() const {
      Offset offset {};
      if (Text::FindOffsetReverse(Text {'/'}, offset))
         return Text::Crop(0, offset + 1);
      return {};
   }

   /// Return the filename part of the path                                   
   ///   @return the filename part (after the last '/')                       
   LANGULUS(ALWAYSINLINE)
   Path Path::GetFilename() const {
      Offset offset {};
      if (Text::FindOffsetReverse(Text {'/'}, offset))
         return Text::Crop(offset + 1, mCount - offset - 1);
      return *this;
   }

   /// Append a subdirectory or filename                                      
   ///   @param rhs - the text to append                                      
   ///   @return the combined directory name                                  
   inline Path Path::operator / (const Text& rhs) const {
      if (last() == '/') {
         if (rhs.last() == '/')
            return *this + rhs.Crop(1, rhs.GetCount() - 1);
         else
            return *this + rhs;
      }
      else {
         if (rhs.last() == '/')
            return *this + rhs;
         else {
            auto temp = *this + Text {'/'};
            temp += rhs;
            return Path {Abandon(temp)};
         }
      }
   }

   /// Append a subdirectory or filename                                      
   ///   @param rhs - the text to append                                      
   ///   @return the combined directory name                                  
   inline Path& Path::operator /= (const Text& rhs) {
      if (last() == '/') {
         if (rhs.last() == '/')
            *this += rhs.Crop(1, rhs.GetCount() - 1);
         else
            *this += rhs;
      }
      else {
         if (rhs.last() == '/')
            *this += rhs;
         else {
            *this += Text {'/'};
            *this += rhs;
         }
      }
      return *this;
   }

} // namespace Langulus::Anyness
