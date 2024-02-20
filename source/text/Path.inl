///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Path.hpp"
#include "Text.inl"


namespace Langulus::Anyness
{

   /// Construct by copying a text container                                  
   ///   @param other - the container to copy                                 
   LANGULUS(INLINED)
   Path::Path(const Text& other)
      : Text {other} {}

   /// Construct by moving a text container                                   
   ///   @param other - the container to copy                                 
   LANGULUS(INLINED)
   Path::Path(Text&& other)
      : Text {Forward<Text>(other)} {}

   /// Descriptor-constructor                                                 
   ///   @param describe - the descriptor                                     
   LANGULUS(INLINED)
   Path::Path(Describe&& describe) {
      LANGULUS_ASSUME(UserAssumes, *describe,
         "Empty descriptor for ", NameOf<Path>());
      describe->ExtractData<Text>(*this);
   }

   /// Return the lowercase file extension (the part after the last '.')      
   ///   @return a cloned text container with the extension                   
   LANGULUS(INLINED)
   Text Path::GetExtension() const {
      const auto found = Find<true>('.');
      if (found)
         return Text::Crop(found);
      return {};
   }

   /// Return the directory part of the path                                  
   ///   @return the directory part, including the last '/'                   
   LANGULUS(INLINED)
   Path Path::GetDirectory() const {
      const auto found = Find<true>(Separator);
      if (found)
         return Text::Crop(0, found.GetOffsetUnsafe() + 1);
      return {};
   }

   /// Return the filename part of the path                                   
   ///   @return the filename part (after the last '/')                       
   LANGULUS(INLINED)
   Path Path::GetFilename() const {
      const auto found = Find<true>(Separator);
      if (found)
         return Text::Crop(found + 1);
      return *this;
   }

   /// Append a subdirectory or filename                                      
   ///   @param rhs - the text to append                                      
   ///   @return the combined directory name                                  
   inline Path Path::operator / (const Text& rhs) const {
      if (IsEmpty())
         return rhs;

      if (*last() == Separator) {
         if (*rhs.last() == Separator)
            return *this + rhs.Crop(1);
         else
            return *this + rhs;
      }
      else {
         if (*rhs.last() == Separator)
            return *this + rhs;
         else {
            auto temp = *this + Text {Separator};
            temp += rhs;
            return Path {Abandon(temp)};
         }
      }
   }

   /// Append a subdirectory or filename                                      
   ///   @param rhs - the text to append                                      
   ///   @return the combined directory name                                  
   inline Path& Path::operator /= (const Text& rhs) {
      if (IsEmpty())
         return operator = (rhs);

      if (*last() == Separator) {
         if (*rhs.last() == Separator)
            *this += rhs.Crop(1);
         else
            *this += rhs;
      }
      else {
         if (*rhs.last() == Separator)
            *this += rhs;
         else {
            *this += Text {Separator};
            *this += rhs;
         }
      }
      return *this;
   }

} // namespace Langulus::Anyness