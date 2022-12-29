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
