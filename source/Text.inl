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

   /// Construct from token                                                   
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param text - the text to wrap                                       
   inline Text::Text(const Token& text)
      : Text {text.data(), text.size()} {}

   /// Construct manually from count-terminated C string                      
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param text - text memory to reference                               
   ///   @param count - number of characters inside text                      
   inline Text::Text(const Letter* text, const Count& count) SAFETY_NOEXCEPT()
      : TAny {text, count} { }

   /// Construct manually from count-terminated C string                      
   /// Data will never be cloned or referenced                                
   ///   @param text - text memory to wrap                                    
   ///   @param count - number of characters inside text                      
   inline Text::Text(Disowned<const Letter*>&& text, const Count& count) noexcept
      : TAny {Disown(text.mValue), count} { }

   /// Construct manually from a c style array                                
   /// Data will be cloned if we don't have authority over the memory         
   ///   @tparam T - type of the character in the array                       
   ///   @tparam C - size of the array                                        
   ///   @param text - the array                                              
   template<Count C>
   inline Text::Text(const Letter(&text)[C])
      : TAny {text, C - 1} { }

   /// Construct from a single character                                      
   /// Data will be cloned if we don't have authority over the memory         
   ///   @tparam T - type of the character in the array                       
   ///   @param anyCharacter - the character to stringify                     
   inline Text::Text(const Letter& anyCharacter)
      : Text {&anyCharacter, 1} {}

   /// Convert a number type to text                                          
   ///   @tparam T - number type to stringify                                 
   ///   @param number - the number to stringify                              
   template<CT::Dense T>
   Text::Text(const T& number) requires CT::Number<T> {
      if constexpr (CT::Real<T>) {
         // Stringify a real number                                     
         constexpr auto size = ::std::numeric_limits<T>::max_digits10 * 2;
         char temp[size];
         auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number, ::std::chars_format::general);
         LANGULUS_ASSERT(errorCode == ::std::errc(), Except::Convert,
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
         LANGULUS_ASSERT(errorCode == ::std::errc(), Except::Convert,
            "std::to_chars failure");

         (*this) += Text {temp, static_cast<Count>(lastChar - temp)};
      }
      else LANGULUS_ERROR("Unsupported number type");
   }

   /// Construct from null-terminated UTF text                                
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param nullterminatedText - text memory to reference                 
   inline Text::Text(const Letter* nullterminatedText) SAFETY_NOEXCEPT()
      : Text {nullterminatedText, 
         nullterminatedText ? ::std::strlen(nullterminatedText) : 0
      } {}

   /// Construct from null-terminated UTF text                                
   /// Data will never be cloned or referenced                                
   ///   @param nullterminatedText - text to wrap                             
   inline Text::Text(Disowned<const Letter*>&& nullterminatedText) noexcept
      : Text {nullterminatedText.Forward(), 
         nullterminatedText.mValue ? ::std::strlen(nullterminatedText.mValue) : 0
      } {}

   /// Interpret text container as a literal                                  
   ///   @attention the string is null-terminated only after Terminate()      
   inline Text::operator Token() const noexcept {
      return {GetRaw(), mCount};
   }

   /// Compare with a std::string                                             
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   inline bool Text::operator == (const CompatibleStdString& rhs) const noexcept {
      return operator == (Text {Disown(rhs.data()), rhs.size()});
   }

   /// Compare with a std::string_view                                        
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   inline bool Text::operator == (const CompatibleStdStringView& rhs) const noexcept {
      return operator == (Text {Disown(rhs.data()), rhs.size()});
   }

   /// Compare two text containers                                            
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   inline bool Text::operator == (const Text& rhs) const noexcept {
      return TAny::operator == (static_cast<const TAny&>(rhs));
   }

   /// Compare with a string                                                  
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   inline bool Text::operator == (const Letter* rhs) const noexcept {
      return operator == (Text {Disown(rhs)});
   }

   
   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// Copy-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   inline Text Text::operator + (const Text& rhs) const {
      return Concatenate<Text, true>(rhs);
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   inline Text Text::operator + (Text&& rhs) const {
      return Concatenate<Text, true>(Forward<Text>(rhs));
   }

   /// Disown-concatenate with another TAny                                   
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   inline Text Text::operator + (Disowned<Text>&& rhs) const {
      return Concatenate<Text, false>(rhs.mValue);
   }

   /// Abandon-concatenate with another TAny                                  
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   inline Text Text::operator + (Abandoned<Text>&& rhs) const {
      return Concatenate<Text, false>(Forward<Text>(rhs.mValue));
   }

   /// Destructive copy-concatenate with another TAny                         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   inline Text& Text::operator += (const Text& rhs) {
      InsertBlock(rhs);
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   inline Text& Text::operator += (Text&& rhs) {
      InsertBlock(Forward<Text>(rhs));
      return *this;
   }

   /// Destructive disown-concatenate with any deep type                      
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   inline Text& Text::operator += (Disowned<Text>&& rhs) {
      InsertBlock(rhs.Forward());
      return *this;
   }

   /// Destructive abandon-concatenate with any deep type                     
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   inline Text& Text::operator += (Abandoned<Text>&& rhs) {
      InsertBlock(rhs.Forward());
      return *this;
   }

} // namespace Langulus::Anyness

namespace Langulus
{

   /// Make a text literal                                                    
   inline Anyness::Text operator "" _text(const char* text, ::std::size_t size) {
      return Anyness::Text {text, size};
   }

} // namespace Langulus
