#pragma once
#include "Integration.hpp"

namespace Langulus::Anyness::Inner
{

   /// Shamelessly stolen from boost and extended to my liking						
   #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	   #define LANGULUS_FUNCTION() __PRETTY_FUNCTION__
   #elif defined(__clang__)
	   #define LANGULUS_FUNCTION() __PRETTY_FUNCTION__
   #elif defined(__DMC__) && (__DMC__ >= 0x810)
	   #define LANGULUS_FUNCTION() __PRETTY_FUNCTION__
   #elif defined(__FUNCSIG__) || defined(_MSC_VER)
	   #define LANGULUS_FUNCTION() __FUNCSIG__
   #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	   #define LANGULUS_FUNCTION() __FUNCTION__
   #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	   #define LANGULUS_FUNCTION() __FUNC__
   #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	   #define LANGULUS_FUNCTION() __func__
   #elif defined(__cplusplus) && (__cplusplus >= 201103)
	   #define LANGULUS_FUNCTION() __func__
   #else
	   #define LANGULUS_FUNCTION() "(unknown)"
   #endif

   /// Pretty function used to wrap a type as a template argument             
   /// Once we wrap and stringify it, we can isolate the typename itself      
   template<typename T>
   constexpr Token TypeAsTemplateArgument() {
      return reinterpret_cast<const char8_t*>(LANGULUS_FUNCTION());
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
      SkipPrefix(name, u8"class");
      SkipSpace(name);
   }

   /// Skip struct identifier                                                 
   constexpr void SkipStruct(Token& name) {
      SkipSpace(name);
      SkipPrefix(name, u8"struct");
      SkipSpace(name);
   }

   /// Skip enum identifier                                                   
   constexpr void SkipEnum(Token& name) {
      SkipSpace(name);
      SkipPrefix(name, u8"enum");
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
         constexpr Token Prefix = u8"Langulus::Token Langulus::Anyness::Inner::TypeAsTemplateArgument() [T = ";
         constexpr Token Suffix = u8"]";
      #elif defined(__GNUC__) && !defined(__clang__)
         constexpr Token Prefix = u8"constexpr std::basic_string_view<char8_t> Langulus::Anyness::Inner::TypeAsTemplateArgument() [with T = ";
         constexpr Token Suffix = u8"]";
      #elif defined(_MSC_VER)
         constexpr Token Prefix = u8"class std::basic_string_view<char8_t,struct std::char_traits<char8_t> > __cdecl Langulus::Anyness::Inner::TypeAsTemplateArgument<";
         constexpr Token Suffix = u8">(void)";
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
