///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Text.hpp"
#include <charconv>
#include <limits>
#include <cstring>

namespace Langulus::Anyness
{

   /// Semantic text constructor                                              
   ///   @tparam S - the semantic to use                                      
   ///   @param other - the text container to use semantically                
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Text::Text(S&& other) requires Relevant<S>
      : TAny {other.template Forward<TAny>()} { }

   /// Construct from token                                                   
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param text - the text to wrap                                       
   LANGULUS(ALWAYSINLINE)
   Text::Text(const Token& text)
      : Text {Langulus::Copy(text.data()), text.size()} {}

   /// Construct manually from count-terminated C string                      
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param text - text memory to reference                               
   ///   @param count - number of characters inside text                      
   LANGULUS(ALWAYSINLINE)
   Text::Text(const Letter* text, const Count& count)
      : Text {Langulus::Copy(text), count} { }

   LANGULUS(ALWAYSINLINE)
   Text::Text(Letter* text, const Count& count)
      : Text {Langulus::Copy(text), count} { }

   /// Construct manually from count-terminated C string                      
   /// Data will never be cloned or referenced                                
   ///   @param text - text memory to wrap                                    
   ///   @param count - number of characters inside text                      
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Text::Text(S&& text, const Count& count) requires RawTextPointer<S>
      : TAny {TAny::From(text.Forward(), count)} { }

   /// Construct manually from a c style array                                
   /// Data will be cloned if we don't have authority over the memory         
   ///   @tparam T - type of the character in the array                       
   ///   @tparam C - size of the array                                        
   ///   @param text - the array                                              
   template<Count C>
   LANGULUS(ALWAYSINLINE)
   Text::Text(const Letter(&text)[C])
      : Text {Langulus::Copy(text), C-1} { }

   /// Construct from a single character                                      
   /// Data will be cloned if we don't have authority over the memory         
   ///   @tparam T - type of the character in the array                       
   ///   @param anyCharacter - the character to stringify                     
   LANGULUS(ALWAYSINLINE)
   Text::Text(const Letter& anyCharacter)
      : Text {Langulus::Copy(&anyCharacter), 1} { }

   /// Convert a number type to text                                          
   ///   @tparam T - number type to stringify                                 
   ///   @param number - the number to stringify                              
   LANGULUS(ALWAYSINLINE)
   Text::Text(const CT::DenseNumber auto& number) {
      using T = Decay<decltype(number)>;
      if constexpr (CT::Real<T>) {
         // Stringify a real number                                     
         constexpr auto size = ::std::numeric_limits<T>::max_digits10 * 2;
         char temp[size];
         auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number, ::std::chars_format::general);
         LANGULUS_ASSERT(errorCode == ::std::errc(), Convert,
            "std::to_chars failure");

         while ((*lastChar == '0' || *lastChar == '.') && lastChar > temp) {
            if (*lastChar == '.')
               break;
            --lastChar;
         }

         (*this) = Text {temp, static_cast<Count>(lastChar - temp)};
      }
      else if constexpr (CT::Integer<T>) {
         // Stringify an integer                                        
         constexpr auto size = ::std::numeric_limits<T>::digits10 * 2;
         char temp[size];
         auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number);
         LANGULUS_ASSERT(errorCode == ::std::errc(), Convert,
            "std::to_chars failure");

         (*this) += Text {temp, static_cast<Count>(lastChar - temp)};
      }
      else LANGULUS_ERROR("Unsupported number type");
   }

   /// Construct from null-terminated UTF text                                
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param nullterminatedText - text memory to reference                 
   LANGULUS(ALWAYSINLINE)
   Text::Text(const Letter* nullterminatedText)
      : Text {Langulus::Copy(nullterminatedText)} {}

   LANGULUS(ALWAYSINLINE)
   Text::Text(Letter* nullterminatedText)
      : Text {Langulus::Copy(nullterminatedText)} {}

   /// Construct from null-terminated UTF text                                
   /// Data will never be cloned or referenced                                
   ///   @param nullterminatedText - text to wrap                             
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Text::Text(S&& nullterminatedText) requires RawTextPointer<S>
      : Text {
         nullterminatedText.Forward(), 
         nullterminatedText.mValue 
            ? ::std::strlen(nullterminatedText.mValue) 
            : 0
      } {}

   
   /// Text container copy-construction                                       
   /// Notice how container is explicitly cast to base class when forwarded   
   /// If that is not done, TAny will use the CT::Deep constructor instead    
   ///   @param other - container to reference                                
   LANGULUS(ALWAYSINLINE)
   Text::Text(const Text& other)
      : TAny {static_cast<const TAny&>(other)} {}

   /// Text container move-construction                                       
   ///   @param other - container to move                                     
   LANGULUS(ALWAYSINLINE)
   Text::Text(Text&& other) noexcept
      : TAny {Forward<TAny>(other)} {}

   /// Text container copy-construction from TAny<Byte> base                  
   ///   @param other - container to reference                                
   LANGULUS(ALWAYSINLINE)
   Text::Text(const TAny& other)
      : TAny {other} {}

   /// Text container mvoe-construction from TAny<Byte> base                  
   ///   @param other - container to move                                     
   LANGULUS(ALWAYSINLINE)
   Text::Text(TAny&& other) noexcept
      : TAny {Forward<TAny>(other)} {}

   /// Construct from a Langulus exception                                    
   ///   @param from - the exception to stringify                             
   LANGULUS(ALWAYSINLINE)
   Text::Text(const Exception& from) {
      (*this) += from.GetName();
      (*this) += "(";
      (*this) += from.GetMessage();
      (*this) += " at ";
      (*this) += from.GetLocation();
      (*this) += ")";
   }

   /// Stringify meta                                                         
   ///   @param meta - the definition to stringify                            
   LANGULUS(ALWAYSINLINE)
   Text::Text(const RTTI::Meta& meta)
      : Text {meta.mToken} {}

   /// Count the number of newline characters                                 
   ///   @return the number of newline characters + 1, or zero if empty       
   LANGULUS(ALWAYSINLINE)
   Count Text::GetLineCount() const noexcept {
      if (IsEmpty())
         return 0;

      Count lines {1};
      for (Count i = 0; i < mCount; ++i) {
         if ((*this)[i] == '\n')
            ++lines;
      }

      return lines;
   }

   /// Shallow copy assignment an immutable text container                    
   ///   @param rhs - the text container to copy                              
   ///   @return a reference to this container                                
   LANGULUS(ALWAYSINLINE)
   Text& Text::operator = (const Text& rhs) {
      TAny::operator = (static_cast<const TAny&>(rhs));
      return *this;
   }

   /// Move text container                                                    
   ///   @param rhs - the text container to move                              
   ///   @return a reference to this container                                
   LANGULUS(ALWAYSINLINE)
   Text& Text::operator = (Text&& rhs) noexcept {
      TAny::operator = (Forward<TAny>(rhs));
      return *this;
   }

   #if LANGULUS_FEATURE(UTFCPP)
      /// Widen the text container to the utf16                               
      ///   @return the widened text container                                
      TAny<char16_t> Text::Widen16() const {
         if (IsEmpty())
            return {};

         TAny<char16_t> to;
         to.AllocateFresh(to.RequestSize(mCount));
         Count newCount = 0;
         try {
            newCount = utf8::utf8to16(begin(), end(), to.begin()) - to.begin();
         }
         catch (utf8::exception&) {
            LANGULUS_THROW(Convert, "utf8 -> utf16 conversion error");
         }

         return to.Trim(newCount);
      }

      /// Widen the text container to the utf32                               
      ///   @return the widened text container                                
      TAny<char32_t> Text::Widen32() const {
         if (IsEmpty())
            return {};

         TAny<char32_t> to;
         to.AllocateFresh(to.RequestSize(mCount));
         Count newCount = 0;
         try {
            newCount = utf8::utf8to32(begin(), end(), to.begin()) - to.begin();
         }
         catch (utf8::exception&) {
            LANGULUS_THROW(Convert, "utf8 -> utf16 conversion error");
         }

         return to.Trim(newCount);
      }
   #endif

   /// Clone the text container                                               
   ///   @return a new container that owns its memory                         
   LANGULUS(ALWAYSINLINE)
   Text Text::Clone() const {
      Text result {Disown(*this)};
      if (mCount) {
         const auto request = RequestSize(mCount);
         result.AllocateFresh(request);
         result.mReserved = request.mElementCount;
         CopyMemory(mRaw, result.mRaw, mCount);
      }
      else {
         result.mEntry = nullptr;
         result.mRaw = nullptr;
         result.mReserved = 0;
      }

      return Abandon(result);
   }

   /// Terminate text so that it ends with a zero character at the end        
   ///   @return a new container that ownes its memory                        
   LANGULUS(ALWAYSINLINE)
   Text Text::Terminate() const {
      if (mReserved > mCount && GetRaw()[mCount] == '\0')
         return *this;

      //TODO: always cloning? why tho? what if this text has one use only?
      Text result {Disown(*this)};
      const auto request = RequestSize(result.mReserved + 1);
      result.AllocateFresh(request);
      result.mReserved = request.mElementCount;
      CopyMemory(mRaw, result.mRaw, mCount);
      result.GetRaw()[mCount] = '\0';
      return Abandon(result);
   }

   /// Make all letters lowercase                                             
   ///   @return a new text container with all letter made lowercase          
   LANGULUS(ALWAYSINLINE)
   Text Text::Lowercase() const {
      Text result = Clone();
      for (auto i : result)
         i = static_cast<Letter>(::std::tolower(i));
      return result;
   }

   /// Make all letters uppercase                                             
   ///   @return a new text container with all letter made uppercase          
   LANGULUS(ALWAYSINLINE)
   Text Text::Uppercase() const {
      Text result = Clone();
      for (auto i : result)
         i = static_cast<Letter>(::std::toupper(i));
      return result;
   }

   /// Find a substring and set 'offset' to its location                      
   ///   @param pattern - the pattern to search for                           
   ///   @param offset - [in/out] offset to set if found                      
   ///   @return true if pattern was found                                    
   LANGULUS(ALWAYSINLINE)
   bool Text::FindOffset(const Text& pattern, Offset& offset) const {
      if (pattern.IsEmpty() || offset >= mCount || pattern.mCount > mCount - offset)
         return false;

      const auto end = mCount - pattern.mCount;
      for (Offset i = offset; i < end; ++i) {
         auto remaining = Block::CropInner(i, pattern.mCount, pattern.mCount);
         auto& asText = ReinterpretCast<Text&>(remaining);
         if (asText.Compare(pattern)) {
            offset = i;
            return true;
         }
      }

      return false;
   }

   /// Find a substring in reverse, and set offset to its location            
   ///   @param pattern - the pattern to search for                           
   ///   @param offset - [out] offset to set if found                         
   ///   @return true if pattern was found                                    
   LANGULUS(ALWAYSINLINE)
   bool Text::FindOffsetReverse(const Text& pattern, Offset& offset) const {
      if (pattern.IsEmpty() || offset >= mCount || pattern.mCount > mCount - offset)
         return false;

      const auto start = mCount - pattern.mCount - offset;
      for (auto i = ::std::ptrdiff_t(start); i >= 0; --i) {
         auto remaining = Block::CropInner(i, pattern.mCount, pattern.mCount);
         auto& asText = ReinterpretCast<Text&>(remaining);
         if (asText.Compare(pattern)) {
            offset = Offset(i);
            return true;
         }
      }

      return false;
   }

   /// Find a pattern                                                         
   ///   @param pattern - the pattern to search for                           
   ///   @return true on first match                                          
   LANGULUS(ALWAYSINLINE)
   bool Text::Find(const Text& pattern) const {
      UNUSED() Offset unused;
      return FindOffset(pattern, unused);
   }

   /// Find a match using wildcards in a pattern                              
   ///   @param pattern - the pattern with the wildcards                      
   ///   @return true on first match                                          
   LANGULUS(ALWAYSINLINE)
   bool Text::FindWild(const Text& pattern) const {
      if (pattern.IsEmpty() || pattern.mCount > mCount)
         return false;

      Offset offset {};
      for (Offset i = 0; i < pattern.mCount; ++i) {
         if (pattern[i] == '*')
            continue;

         // Get every substring between *s                              
         Offset accum {};
         while (i + accum < pattern.mCount && pattern[i + accum] != '*')
            ++accum;

         if (accum > 0 && !FindOffset(pattern.Crop(i, accum), offset))
            // Mismatch                                                 
            return false;

         offset += accum;
         i += accum;
      }

      // Success                                                        
      return true;
   }

   /// Pick a part of the text (const)                                        
   ///   @param start - offset of the starting character                      
   ///   @param count - the number of characters after 'start'                
   ///   @return new text that references the original memory                 
   LANGULUS(ALWAYSINLINE)
   Text Text::Crop(Count start, Count count) const {
      return TAny::Crop<Text>(start, count);
   }

   /// Pick a part of the text                                                
   ///   @param start - offset of the starting character                      
   ///   @param count - the number of characters after 'start'                
   ///   @return new text that references the original memory                 
   LANGULUS(ALWAYSINLINE)
   Text Text::Crop(Count start, Count count) {
      return TAny::Crop<Text>(start, count);
   }

   /// Remove all instances of a symbol from the text container               
   ///   @param symbol - the character to remove                              
   ///   @return a new container with the text stripped                       
   LANGULUS(ALWAYSINLINE)
   Text Text::Strip(Letter symbol) const {
      Text result;
      Count start {}, end {};
      for (Count i = 0; i <= mCount; ++i) {
         if (i == mCount || (*this)[i] == symbol) {
            const auto size = end - start;
            if (size) {
               auto segment = result.Extend(size);
               CopyMemory(GetRaw() + start, segment.GetRaw(), size);
            }

            start = end = i + 1;
         }
         else ++end;
      }

      return result;
   }

   /// Remove a part of the text                                              
   ///   @attention assumes end >= start                                      
   ///   @param start - the starting character                                
   ///   @param end - the ending character                                    
   ///   @return a reference to this text                                     
   LANGULUS(ALWAYSINLINE)
   Text& Text::Remove(Count start, Count end) {
      LANGULUS_ASSUME(UserAssumes, end >= start, "end < start");
      const auto removed = ::std::min(end, mCount) - ::std::min(start, mCount);
      if (0 == mCount || 0 == removed)
         return *this;

      LANGULUS_ASSERT(IsMutable(), Destruct,
         "Can't remove from constant container");

      if (end < mCount)
         Block::MoveMemory(mRaw + end, mRaw + start, mCount - end);

      mCount -= removed;
      return *this;
   }

   /// Extend the text container and return a referenced part of it           
   ///   @return an array that represents the extended part                   
   LANGULUS(ALWAYSINLINE)
   Text Text::Extend(Count count) {
      return TAny::Extend<Text>(count);
   }

   /// Hash the text                                                          
   ///   @return a hash of the contained byte sequence                        
   LANGULUS(ALWAYSINLINE)
   Hash Text::GetHash() const {
      return HashBytes(GetRaw(), static_cast<int>(GetCount()));
   }

   /// Interpret text container as a literal                                  
   ///   @attention the string is null-terminated only after Terminate()      
   LANGULUS(ALWAYSINLINE)
   Text::operator Token() const noexcept {
      return {GetRaw(), mCount};
   }

   /// Compare with a std::string                                             
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(ALWAYSINLINE)
   bool Text::operator == (const CompatibleStdString& rhs) const noexcept {
      return operator == (Text {Disown(rhs.data()), rhs.size()});
   }

   /// Compare with a std::string_view                                        
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(ALWAYSINLINE)
   bool Text::operator == (const CompatibleStdStringView& rhs) const noexcept {
      return operator == (Text {Disown(rhs.data()), rhs.size()});
   }

   /// Compare two text containers                                            
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(ALWAYSINLINE)
   bool Text::operator == (const Text& rhs) const noexcept {
      return TAny::operator == (static_cast<const TAny&>(rhs));
   }

   /// Compare with a string                                                  
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(ALWAYSINLINE)
   bool Text::operator == (const Letter* rhs) const noexcept {
      return operator == (Text {Disown(rhs)});
   }

   
   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// Copy-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(ALWAYSINLINE)
   Text Text::operator + (const Text& rhs) const {
      return Concatenate<Text>(Langulus::Copy(rhs));
   }

   /// Copy-concatenate with a letter                                         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(ALWAYSINLINE)
   Text Text::operator + (const Letter& rhs) const {
      return Concatenate<Text>(Langulus::Abandon(Text {rhs}));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(ALWAYSINLINE)
   Text Text::operator + (Text&& rhs) const {
      return Concatenate<Text>(Langulus::Move(rhs));
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Text Text::operator + (S&& rhs) const requires Relevant<S> {
      return Concatenate<Text>(rhs.Forward());
   }

   /// Destructive copy-concatenate with another TAny                         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(ALWAYSINLINE)
   Text& Text::operator += (const Text& rhs) {
      InsertBlock(Langulus::Copy(rhs));
      return *this;
   }

   /// Destructive copy-concatenate with a character                          
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(ALWAYSINLINE)
   Text& Text::operator += (const Letter& rhs) {
      Emplace(rhs);
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(ALWAYSINLINE)
   Text& Text::operator += (Text&& rhs) {
      InsertBlock(Langulus::Move(rhs));
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Text& Text::operator += (S&& rhs) requires Relevant<S> {
      InsertBlock(rhs.Forward());
      return *this;
   }

} // namespace Langulus::Anyness

namespace Langulus
{

   /// Make a text literal                                                    
   LANGULUS(ALWAYSINLINE)
   Anyness::Text operator "" _text(const char* text, ::std::size_t size) {
      return Anyness::Text {text, size};
   }

} // namespace Langulus
