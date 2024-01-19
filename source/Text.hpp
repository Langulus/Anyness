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
#include <Logger/Logger.hpp> // Logger has some core fmt::formatters defined  
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
   /// This includes: Text containers; any contiguous standard containers,    
   /// that are statically typed and filled with characters; dense character  
   /// types; null-terminated (and unsafe) c-strings; string literals (aka    
   /// bounded character arrays)                                              
   template<class...T>
   concept Text = ((TextBased<T>
        or CT::String<T>
        or CT::DenseCharacter<T>
        or CT::StdString<T>
      ) and ...);

   namespace Inner
   {

      /// Low level concept for checking if a type is stringifiable by {fmt}  
      /// Notice that any CT::Text are omitted here, to avoid ambiguities     
      /*template<class...T>
      concept ExplicitlyFormattable = not Text<T...> and not Semantic<T...>
          and (::fmt::is_formattable<Deref<T>>::value and ...);*/
   
      /// Same as above, but also includes CT::Text                           
      template<class...T>
      concept Formattable = ((Text<T>
           or ExplicitlyFormattable<T>) and ...);
   
      /// This is the higher level concept, for checking if something can be  
      /// converted to Text container                                         
      template<class T>
      concept Stringifiable = 
            requires (T& a) { a.operator ::Langulus::Anyness::Text(); }
         or requires (T& a) { ::Langulus::Anyness::Text {a}; };

   } // namespace Langulus::CT::Inner

   /// A stringifiable type is one that has either an implicit or explicit    
   /// cast operator to Text type, or can be used to explicitly initialize a  
   /// Text container                                                         
   template<class...T>
   concept Stringifiable = (Inner::Stringifiable<T> and ...);

} // namespace Langulus::CT


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Count-terminated UTF text container                                  
   ///                                                                        
   ///   This is a general purpose text container. It can contain serialized  
   /// data, but converting to it is a one way process. While serialization   
   /// aims at being isomorphic, converting to Text aims at readability only. 
   /// Consider it a general day to day speech container, that may or may not 
   /// be formal. The container leverages {fmt} library, and can be extended  
   /// using fmt::formatter specializations                                   
   ///   If you want to serialize your data in a readable format, convert to  
   /// Flow::Code, or other isomorphic representations                        
   ///                                                                        
   struct Text : TAny<Letter> {
      using Base = TAny<Letter>;
      static constexpr bool CTTI_TextTrait = true;

      LANGULUS(DEEP) false;
      LANGULUS(POD) false;
      LANGULUS_BASES(A::Text, Base);
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

      //explicit Text(const CT::Inner::ExplicitlyFormattable auto&);

      template<class T1, class T2, class...TN>
      requires CT::Stringifiable<T1, T2, TN...>
      Text(T1&&, T2&&, TN&&...);

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

   protected:
      void UnfoldInsert(auto&&);

   public:
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

      NOD() static Text Hex(const auto&);
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

} // namespace Langulus::Anyness

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

      template<class CONTEXT> LANGULUS(INLINED)
      auto format(const T& element, CONTEXT& ctx) {
         using namespace Langulus;
         static_assert(CT::Complete<T>, "T isn't complete");
         auto& me = const_cast<T&>(element);

         auto token = me.operator Token();
         return fmt::format_to(ctx.out(), "{}", token);
      }
   };

   ///                                                                        
   /// Extend FMT to be capable of logging anything that is convertible to    
   /// Anyness::Text. Constness of T doesn't matter.                          
   ///                                                                        
   /*template<Langulus::CT::Stringifiable T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT> LANGULUS(INLINED)
      auto format(const T& element, CONTEXT& ctx) {
         using namespace Langulus;
         static_assert(CT::Complete<T>, "T isn't complete");
         auto& me = const_cast<T&>(element);

         Anyness::Text asText;
         if constexpr (requires { me.operator Anyness::Text(); })
            asText = Anyness::Text {me};
         else if constexpr (requires { Anyness::Text {me}; })
            asText = Anyness::Text {me};
         else
            LANGULUS_ERROR("Unhandled conversion route");

         auto token = asText.operator Token();
         return fmt::format_to(ctx.out(), "{}", token);
      }
   };*/

} // namespace fmt