///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Config.hpp"
#include <string>

namespace Langulus::Anyness::Inner
{

   /// Pretty function used to wrap a type as a template argument             
   /// Once we wrap and stringify it, we can isolate the typename itself      
   template<typename T>
   constexpr Token TypeAsTemplateArgument() {
      return LANGULUS_FUNCTION();
   }

   /// Filter a literal matching at the front                                 
   constexpr void SkipPrefix(Token& str, const Token& prefix) {
      if (str.starts_with(prefix))
         str.remove_prefix(prefix.size());
   }

   /// Filter a literal matching at the back                                  
   constexpr void SkipSuffix(Token& str, const Token& suffix) {
      if (str.ends_with(suffix))
         str.remove_suffix(suffix.size());
   }

   /// Skip empty space                                                       
   constexpr void SkipSpace(Token& str) {
      while (str.size() > 0 && (str[0] == ' ' || str[0] == '\t'))
         str.remove_prefix(1);
   }

   /// Skip class identifier                                                  
   constexpr void SkipClass(Token& name) {
      SkipSpace(name);
      SkipPrefix(name, "class");
      SkipSpace(name);
   }

   /// Skip struct identifier                                                 
   constexpr void SkipStruct(Token& name) {
      SkipSpace(name);
      SkipPrefix(name, "struct");
      SkipSpace(name);
   }

   /// Skip enum identifier                                                   
   constexpr void SkipEnum(Token& name) {
      SkipSpace(name);
      SkipPrefix(name, "enum");
      SkipSpace(name);
   }

   /// Skip all decorations in front of a type name                           
   constexpr void SkipDecorations(Token& name) {
      SkipStruct(name);
      SkipClass(name);
      SkipEnum(name);
   }

   /// The naming is based on CTTI                                            
   /// It is essentially an attempt at constexpr reflection                   
   /// See here for more details: https://github.com/Manu343726/ctti          
   template<typename T>
   constexpr Token NameOf() noexcept {
      /// These definitions might change in future compiler versions and will 
      /// probably need constant maintenance. Luckily, their failure is not   
      /// necessarily critical, since all we need is actually a hash of an    
      /// unique name regardless of its actual name contents                  
      #if defined(__clang__)
         constexpr Token Prefix = "Langulus::Token Langulus::Anyness::Inner::TypeAsTemplateArgument() [T = ";
         constexpr Token Suffix = "]";
      #elif defined(__GNUC__) && !defined(__clang__)
         constexpr Token Prefix = "constexpr Langulus::Token Langulus::Anyness::Inner::TypeAsTemplateArgument() [with T = ";
         constexpr Token Suffix = "; Langulus::Token = std::basic_string_view<char>]";
      #elif defined(_MSC_VER)
         constexpr Token Prefix = "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl Langulus::Anyness::Inner::TypeAsTemplateArgument<";
         constexpr Token Suffix = ">(void)";
      #else
         #error "No support for this compiler"
      #endif

      auto original = TypeAsTemplateArgument<Decay<T>>();
      //printf("Original: %s\n", std::u8string(original).c_str());
      SkipPrefix(original, Prefix);
      //printf("No prefix: %s\n", std::u8string(original).c_str());
      SkipSuffix(original, Suffix);
      //printf("No suffix: %s\n", std::u8string(original).c_str());
      SkipDecorations(original);
      //printf("No decorations: %s\n", std::u8string(original).c_str());
      return original;
   }

} // namespace Langulus::Anyness::Inner
