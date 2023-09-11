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

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Count-terminated UTF8 text container                                 
   ///                                                                        
   class Text : public TAny<Letter> {
   public:
      using Base = TAny<Letter>;
      LANGULUS(DEEP) false;
      LANGULUS(POD) false;
      LANGULUS_BASES(A::Text, Base);
      LANGULUS(FILES) "txt";

      using CompatibleStdString = ::std::basic_string<Letter>;
      using CompatibleStdStringView = ::std::basic_string_view<Letter>;

   protected:
      template<CT::Semantic S>
      static constexpr bool Relevant = CT::DerivedFrom<TypeOf<S>, Base>;
      template<CT::Semantic S>
      static constexpr bool RawTextPointer = CT::BuiltinCharacter<TypeOf<S>> and CT::Sparse<TypeOf<S>>;

   private:
      explicit Text(const Base&);
      explicit Text(Base&&) noexcept;

   public:
      constexpr Text() = default;

      Text(const Text&);
      Text(Text&&) noexcept;

      template<CT::Semantic S>
      Text(S&&) requires Relevant<S>;

      Text(const CompatibleStdString&);
      Text(const CompatibleStdStringView&);
      Text(const Exception&);
      Text(const RTTI::Meta&);
      Text(const RTTI::Meta*);
      Text(const Letter&);
      Text(const CT::DenseBuiltinNumber auto&);

      // Bounded array constructor                                      
      template<Count C>
      Text(const Letter(&)[C]);

      // Count-terminated constructors                                  
      Text(const Letter*, const Count&);
      Text(Letter*, const Count&);
      template<CT::Semantic S>
      Text(S&&, const Count&) requires RawTextPointer<S>;

      // Zero-terminated constructors                                   
      Text(const Letter*);
      Text(Letter*);
      template<CT::Semantic S>
      Text(S&&) requires RawTextPointer<S>;

      Text& operator = (const Text&);
      Text& operator = (Text&&) noexcept;
      Text& operator = (const Letter&) noexcept;
      template<CT::Semantic S>
      Text& operator = (S&&);

   public:
      NOD() Hash GetHash() const;
      NOD() Text Terminate() const;
      NOD() Text Lowercase() const;
      NOD() Text Uppercase() const;
      NOD() Text Crop(Offset, Count) const;
      NOD() Text Crop(Offset, Count);
      NOD() Text Strip(Letter) const;
      Text& Remove(Offset, Count);
      Text Extend(Count);

      NOD() operator Token () const noexcept;

      #if LANGULUS_FEATURE(UNICODE)
         NOD() TAny<char16_t> Widen16() const;
         NOD() TAny<char32_t> Widen32() const;
      #endif

      NOD() Count GetLineCount() const noexcept;

      bool operator == (const Text&) const noexcept;
      bool operator == (const CompatibleStdString&) const noexcept;
      bool operator == (const CompatibleStdStringView&) const noexcept;
      bool operator == (const Letter*) const noexcept;
      bool operator == (const Letter&) const noexcept;

      NOD() bool FindOffset(const Text&, Offset&) const;
      NOD() bool FindOffsetReverse(const Text&, Offset&) const;
      NOD() bool Find(const Text&) const;

      template<class... ARGS>
      NOD() static Text Template(const Token&, ARGS&&...);
      template<class... ARGS>
      NOD() static Text TemplateRt(const Token&, ARGS&&...);
      template<class... ARGS>
      NOD() static constexpr auto TemplateCheck(const Token&, ARGS&&...);

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      NOD() Text operator + (const Text&) const;
      friend Text operator + (const char*, const Text&);

      Text& operator += (const Text&);

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

      Debug(const Text&);
      Debug(Text&&);
   };

} // namespace Langulus::Anyness

namespace Langulus::CT
{

   /// Concept for differentiating managed Anyness Text types                 
   template<class T>
   concept Text = DerivedFrom<T, ::Langulus::Anyness::Text>;

   /// Concept for differentiating managed Anyness Text types                 
   template<class T>
   concept NotText = !Text<T>;

   namespace Inner
   {
      template<class T>
      concept Stringifiable = not Text<T> and not Same<T, ::Langulus::Token()> and (
         requires (T& a) { a.operator ::Langulus::Anyness::Text(); }
      );

      template<class T>
      concept Debuggable = not Text<T> and not Same<T, ::Langulus::Token()> and (
         requires (T& a) { a.operator ::Langulus::Anyness::Debug(); }
      );
   }

   /// A stringifiable type is one that has either an implicit or explicit    
   /// cast operator to Text type. Reverse conversion through                 
   /// constructors is avoided to mitigate ambiguity problems.                
   template<class... T>
   concept Stringifiable = (Inner::Stringifiable<T> and ...);

   /// A debuggable type is one that has either an implicit or explicit       
   /// cast operator to Debug type. Reverse conversion through                
   /// constructors is avoided to mitigate ambiguity problems.                
   template<class... T>
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