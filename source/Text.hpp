///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Bytes.hpp"
#include <string>


namespace Langulus::CT
{
   
   /// Concept for any possible standard library representation of a string   
   /// This includes not only std::string, but also any contiguous range      
   /// that's filled with dense characters                                    
   template<class...T>
   concept StdString = CT::StdContiguousContainer<T...>
       and CT::DenseCharacter<TypeOf<T>...>;

   /// Concept for differentiating managed text types                         
   /// Text containers are always binary compatible to Block                  
   template<class...T>
   concept TextBased = ((Decay<T>::CTTI_TextTrait and CT::Block<T>) and ...);

   /// Concept for differentiating any form of text                           
   template<class...T>
   concept Text = ((TextBased<T>
        or CT::String<T>
        or CT::DenseCharacter<T>
        or CT::StdString<T>
      ) and ...);

   /// Low level concept for checking if a type is stringifiable by libfmt    
   /// Notice that any CT::Text are omitted here, to avoid ambiguities        
   /// Any higher order conversion from containers to Text is achieved by     
   /// defining an explicit/implicit cast operator to Text, using the         
   /// CT::Stringifiable and CT::Debuggable concepts instead                  
   template<class...T>
   concept Formattable = not Text<T...> 
       and (::fmt::is_formattable<T>::value and ...);

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Count-terminated UTF8 text container                                 
   ///                                                                        
   ///   This is a general purpose text container. It can contain serialized  
   /// data, but converting to it doesn't necessarily mean data will be       
   /// serialized. Consider it a general day to day speech container, that    
   /// may or may not be formal.                                              
   ///                                                                        
   struct Text : TAny<Letter> {
      using Base = TAny<Letter>;
      static constexpr bool CTTI_TextTrait = true;

      LANGULUS(DEEP) false;
      LANGULUS(POD) false;
      LANGULUS_BASES(A::Text, TAny<Letter>);
      LANGULUS(FILES) "txt";

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Text() noexcept = default;
      constexpr Text(::std::nullptr_t) noexcept;
      Text(const Text&);
      Text(Text&&) noexcept;

      template<class T> requires CT::Block<Desem<T>>
      Text(T&&);

      template<class T> requires CT::String<Desem<T>>
      Text(T&&);

      template<class T> requires CT::DenseCharacter<Desem<T>>
      Text(T&&);

      template<class T> requires CT::StdString<Desem<T>>
      Text(T&&);

      template<class...T>
      requires(sizeof...(T) > 1 and ((CT::Text<T> or CT::Formattable<T>) and ...))
      Text(T&&...);

      /// By extending libfmt via formatters, we also extend Text capabilities
      /// Notice, that StringPointer and StringLiteral are handled separately 
      /// in order to make those implicit. This one is explicit, to avoid     
      /// ambiguities                                                         
      explicit Text(CT::Formattable auto&&);

      template<class T> requires CT::String<Desem<T>>
      static Text From(T&&, Count);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Text& operator = (const Text&);
      Text& operator = (Text&&) noexcept;

      template<class T> requires CT::Text<Desem<T>>
      Text& operator = (T&&);

      /// By extending libfmt via formatters, we also extend Text capabilities
      //Text& operator = (CT::Formattable auto&&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() Hash GetHash() const;
      NOD() Count GetLineCount() const noexcept;

      NOD() operator Token () const noexcept;

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Text Crop(CT::Index auto, Count) const;
      NOD() Text Crop(CT::Index auto, Count);
      NOD() Text Crop(CT::Index auto) const;
      NOD() Text Crop(CT::Index auto);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const CT::Block auto&) const noexcept;
      bool operator == (const CT::DenseCharacter auto&) const noexcept;
      bool operator == (const CT::String auto&) const noexcept;
      bool operator == (const CT::StdString auto&) const noexcept;
      bool operator == (::std::nullptr_t) const noexcept;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Text Extend(Count);
      NOD() Text Terminate() const;

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      NOD() Text Strip(Letter) const;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T> requires CT::Text<Desem<T>>
      NOD() Text operator + (T&&) const;
      //NOD() Text operator + (CT::Formattable auto&&) const;

      template<class T> requires CT::Text<Desem<T>>
      Text& operator += (T&&);
      //Text& operator += (CT::Formattable auto&&);

      ///                                                                     
      ///   Services                                                          
      ///                                                                     
      NOD() Text Lowercase() const;
      NOD() Text Uppercase() const;

      #if LANGULUS_FEATURE(UNICODE)
         NOD() TAny<char16_t> Widen16() const;
         NOD() TAny<char32_t> Widen32() const;
      #endif

      template<class...ARGS>
      NOD() static Text Template(const Token&, ARGS&&...);
      template<class...ARGS>
      NOD() static Text TemplateRt(const Token&, ARGS&&...);
      template<class...ARGS>
      NOD() static constexpr auto TemplateCheck(const Token&, ARGS&&...);

   protected:
      template<::std::size_t...N>
      static constexpr auto CheckPattern(const Token&, ::std::index_sequence<N...>);
   };


   /// Text container specialized for logging                                 
   /// Serializing to text might produce a lot of unncessesary text, so this  
   /// differentiation is quite handy                                         
   struct Debug : Text {
      LANGULUS_BASES(Text);
      LANGULUS(FILES) "";

      using Text::Text;
      using Text::operator ==;

      Debug(const Text&);
      Debug(Text&&);
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{
   namespace Inner
   {

      template<class T>
      concept Stringifiable = /*not Text<T> and not StandardContiguousContainer<T> and*/ (
         requires (T& a) { a.operator ::Langulus::Anyness::Text(); }
      );

      template<class T>
      concept Debuggable = /*not Text<T> and not StandardContiguousContainer<T> and*/ (
         requires (T& a) { a.operator ::Langulus::Anyness::Debug(); }
      );

   } // namespace Langulus::CT::Inner

   /// A stringifiable type is one that has either an implicit or explicit    
   /// cast operator to Text type. Reverse conversion through                 
   /// constructors is avoided to mitigate ambiguity problems.                
   template<class...T>
   concept Stringifiable = (Inner::Stringifiable<T> and ...);

   /// A debuggable type is one that has either an implicit or explicit       
   /// cast operator to Debug type. Reverse conversion through                
   /// constructors is avoided to mitigate ambiguity problems.                
   template<class...T>
   concept Debuggable = ((Inner::Stringifiable<T> or Inner::Debuggable<T>) and ...);

} // namespace Langulus::CT

namespace Langulus
{

   Anyness::Text operator "" _text(const char*, ::std::size_t);

} // namespace Langulus

namespace fmt
{
   
   ///                                                                        
   /// Extend FMT to be capable of logging anything that is derived from      
   /// Anyness::Text. Constness of T doesn't matter.                          
   ///                                                                        
   template<Langulus::CT::TextBased T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(const T& element, CONTEXT& ctx) {
         using namespace Langulus;
         static_assert(CT::Complete<T>, "T isn't complete");

         auto asText = const_cast<T&>(element).operator Token();
         return fmt::format_to(ctx.out(), "{}", asText);
      }
   };


   ///                                                                        
   /// Extend FMT to be capable of logging anything that is statically        
   /// convertible to a Token/Debug/Text string by an explicit or implicit    
   /// conversion operator. It doesn't apply to these typed directly.         
   /// Constness of T doesn't matter.                                         
   ///                                                                        
   template<Langulus::CT::Debuggable T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(const T& element, CONTEXT& ctx) {
         using namespace Langulus;
         static_assert(CT::Complete<T>, "T isn't complete");

         if constexpr (requires (T& a) {
            a.operator Anyness::Debug();
         }) {
            // Convert to a debug string if possible                    
            auto asText = const_cast<T&>(element)
               .operator Anyness::Debug();
            return fmt::format_to(ctx.out(), "{}",
               static_cast<Logger::TextView>(asText));
         }
         else if constexpr (requires (T& a) {
            a.operator Anyness::Text();
         }) {
            // Convert to text as a fallback                            
            auto asText = const_cast<T&>(element)
               .operator Anyness::Text();
            return fmt::format_to(ctx.out(), "{}",
               static_cast<Logger::TextView>(asText));
         }
      }
   };

} // namespace fmt