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
   namespace Inner
   {
      template<class T>
      concept Text = T::CTTI_TextTrait and CT::Block<T>;
   }

   /// Concept for differentiating managed text types                         
   template<class...T>
   concept Text = (Inner::Text<Decay<T>> and ...);

   template<class...T>
   concept NotText = not Text<T...>;

} // namespace Langulus::CT

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Count-terminated UTF8 text container                                 
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

      template<class T> requires CT::DenseCharacter<Desem<T>>
      Text(T&&);

      template<class T> requires CT::StringPointer<Desem<T>>
      Text(T&&);

      template<class T> requires CT::StringLiteral<Desem<T>>
      Text(T&&);

      template<class T> requires (CT::StandardContiguousContainer<Desem<T>>
                             and  CT::DenseCharacter<TypeOf<Desem<T>>>)
      Text(T&&);

      Text(const Exception&);
      Text(const CT::Meta auto&);
      Text(const CT::DenseBuiltinNumber auto&);

      template<class T> requires CT::StringPointer<Desem<T>>
      Text(T&&, Count);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Text& operator = (const Text&);
      Text& operator = (Text&&) noexcept;

      template<class T> requires CT::Block<Desem<T>>
      Text& operator = (const T&);

      template<class T> requires CT::DenseCharacter<Desem<T>>
      Text& operator = (const T&);

      template<class T> requires CT::StringPointer<Desem<T>>
      Text& operator = (const T&);

      template<class T> requires CT::StringLiteral<Desem<T>>
      Text& operator = (const T&);

      template<class T> requires (CT::StandardContiguousContainer<Desem<T>>
                             and  CT::DenseCharacter<TypeOf<Desem<T>>>)
      Text& operator = (const T&);

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() Hash GetHash() const;
      NOD() Count GetLineCount() const noexcept;

      NOD() operator Token () const noexcept;

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() Text Crop(Offset, Count) const;
      NOD() Text Crop(Offset, Count);

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const CT::Block auto&) const noexcept;
      bool operator == (const CT::DenseCharacter auto&) const noexcept;
      bool operator == (const CT::StringPointer auto&) const noexcept;
      bool operator == (const CT::StringLiteral auto&) const noexcept;
      template<class T> requires (CT::StandardContiguousContainer<T>
                             and  CT::DenseCharacter<TypeOf<T>>)
      bool operator == (const T&) const noexcept;
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
      template<class T> requires CT::Block<Desem<T>>
      NOD() Text operator + (const T&) const;
      template<class T> requires CT::DenseCharacter<Desem<T>>
      NOD() Text operator + (const T&) const;
      template<class T> requires CT::StringPointer<Desem<T>>
      NOD() Text operator + (const T&) const;
      template<class T> requires CT::StringLiteral<Desem<T>>
      NOD() Text operator + (const T&) const;
      template<class T> requires (CT::StandardContiguousContainer<T>
                             and  CT::DenseCharacter<TypeOf<T>>)
      NOD() Text operator + (const T&) const;

      template<class T> requires CT::Block<Desem<T>>
      Text& operator += (const T&);
      template<class T> requires CT::DenseCharacter<Desem<T>>
      Text& operator += (const T&);
      template<class T> requires CT::StringPointer<Desem<T>>
      Text& operator += (const T&);
      template<class T> requires CT::StringLiteral<Desem<T>>
      Text& operator += (const T&);
      template<class T> requires (CT::StandardContiguousContainer<T>
                             and  CT::DenseCharacter<TypeOf<T>>)
      Text& operator += (const T&);

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


   ///                                                                        
   /// Free standing catenators                                               
   ///                                                                        

   template<class T> requires CT::DenseCharacter<Desem<T>>
   Text operator + (const T&, const Text&);

   template<class T> requires CT::StringPointer<Desem<T>>
   Text operator + (const T&, const Text&);

   template<class T> requires CT::StringLiteral<Desem<T>>
   Text operator + (const T&, const Text&);

   template<class T> requires (CT::StandardContiguousContainer<T>
                          and  CT::DenseCharacter<TypeOf<T>>)
   Text operator + (const T&, const Text&);


   /// Text container specialized for logging                                 
   /// Serializing to text might produce a lot of unncessesary text, so this  
   /// differentiation is quite handy                                         
   struct Debug : Text {
      LANGULUS_BASES(Text);
      LANGULUS(FILES) "";

      using Text::Text;

      Debug(const Text&);
      Debug(Text&&);
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{
   namespace Inner
   {

      template<class T>
      concept Stringifiable = not Text<T> and not StandardContiguousContainer<T> and (
         requires (T& a) { a.operator ::Langulus::Anyness::Text(); }
      );

      template<class T>
      concept Debuggable = not Text<T> and not StandardContiguousContainer<T> and (
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
   /// Anyness::Text                                                          
   ///                                                                        
   template<Langulus::CT::Text T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace Langulus;
         auto asText = element.operator Token();
         return fmt::format_to(ctx.out(), "{}", asText);
      }
   };


   ///                                                                        
   /// Extend FMT to be capable of logging anything that is statically        
   /// convertible to a Token/Debug/Text string by an explicit or implicit    
   /// conversion operator. It doesn't apply to these typed directly.         
   ///                                                                        
   template<Langulus::CT::Debuggable T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace Langulus;
         if constexpr (requires (T& a) { a.operator Anyness::Debug(); }) {
            auto asText = element.operator Anyness::Debug();
            return fmt::format_to(ctx.out(), "{}",
               static_cast<Logger::TextView>(asText));
         }
         else if constexpr (requires (T& a) { a.operator Anyness::Text(); }) {
            auto asText = element.operator Anyness::Text();
            return fmt::format_to(ctx.out(), "{}",
               static_cast<Logger::TextView>(asText));
         }
      }
   };

} // namespace fmt