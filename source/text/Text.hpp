///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../many/TAny.hpp"
#include "../many/Bytes.hpp"
#include <Logger/Logger.hpp> // Logger has some core fmt::formatters defined  


namespace Langulus::Anyness::Serial
{
   enum RuleEnum {
      Skip = 0,
      Wrap = 1
   };

   enum MatchEnum {
      Exact = 0,
      BasedOn = 2,
   };

   enum Operator {
      OpenScope = 0,
      CloseScope,
      OpenCode,
      CloseCode,
      OpenComment,
      CloseComment,
      OpenString,
      CloseString,
      OpenStringAlt,
      CloseStringAlt,
      OpenCharacter,
      CloseCharacter,
      OpenByte,
      CloseByte,

      Future,
      Past,

      Constant,
      Long,
      Mono,
      Null,

      Mass,
      Rate,
      Time,
      Priority,

      OpCounter,
      NoOperator = OpCounter,
      ReflectedOperator,
      ReflectedVerb
   };

   struct OperatorProperties {
      Token mToken;
      bool mCharge = false;
   };

   /// Built-in operator properties                                           
   static constexpr std::array<OperatorProperties, Operator::OpCounter> Operators {
      OperatorProperties { "(" },         // OpenScope                  
      OperatorProperties { ")" },         // CloseScope                 
      OperatorProperties { "[" },         // OpenCode                   
      OperatorProperties { "]" },         // CloseCode                  
      OperatorProperties { "/*" },        // OpenComment                
      OperatorProperties { "*/" },        // CloseComment               
      OperatorProperties { "\"" },        // OpenString                 
      OperatorProperties { "\"" },        // CloseString                
      OperatorProperties { "`" },         // OpenStringAlt              
      OperatorProperties { "`" },         // CloseStringAlt             
      OperatorProperties { "'" },         // OpenCharacter              
      OperatorProperties { "'" },         // CloseCharacter             
      OperatorProperties { "#" },         // OpenByte                   
      OperatorProperties { "" },          // CloseByte                  

      OperatorProperties { "??" },        // Future                     
      OperatorProperties { "?" },         // Past                       

      OperatorProperties { "const" },     // Constant                   
      OperatorProperties { "long" },      // Long                       
      OperatorProperties { "mono" },      // Mono                       
      OperatorProperties { "null" },      // Null                       

      OperatorProperties { "*", true },   // Mass                       
      OperatorProperties { "^", true },   // Rate                       
      OperatorProperties { "@", true },   // Time                       
      OperatorProperties { "!", true }    // Priority                   
   };

   template<RuleEnum RULE, MatchEnum MATCH, class T, Operator START, Operator END>
   struct Rule {
      using Type = T;
      static constexpr RuleEnum  sRule = RULE;
      static constexpr MatchEnum sMatch = MATCH;
      static constexpr Operator  sStart = START;
      static constexpr Operator  sEnd = END;
   };

} // namespace Langulus::Anyness::Serial

namespace Langulus::CT
{
   
   /// Concept for any possible standard library representation of a string   
   /// This includes not only std::string, but also any contiguous range      
   /// that's filled with dense characters                                    
   template<class...T>
   concept StdString = StdContiguousContainer<T...>
       and DenseCharacter<TypeOf<T>...>;

   /// Concept for differentiating managed text types, based on Anyness::Text 
   /// Text containers are always binary compatible to Block                  
   template<class...T>
   concept TextBased = ((Decay<T>::CTTI_TextTrait and CT::Block<T>) and ...);

   /// Concept for differentiating built-in text types, that are provided by  
   /// C++ itself (including standard library ones)                           
   template<class...T>
   concept BuiltinText = ((String<T>
        or DenseCharacter<T>
        or StdString<T>
      ) and ...);

   /// Concept for differentiating any form of text                           
   /// This includes: Text containers; any contiguous standard containers,    
   /// that are statically typed and filled with characters; dense character  
   /// types; null-terminated (and unsafe) c-strings; string literals (aka    
   /// bounded character arrays)                                              
   template<class...T>
   concept Text = ((TextBased<T> or BuiltinText<T>) and ...);

   namespace Inner
   {
   
      /// Do types have an explicit or implicit cast operator to Text         
      template<class...T>
      concept StringifiableByOperator = requires (T&...a) {
         ((a.operator ::Langulus::Anyness::Text()), ...); };

      /// Does Text has an explicit/implicit constructor that accepts T       
      template<class...T>
      concept StringifiableByConstructor = requires (T&...a) {
         ((::Langulus::Anyness::Text {a}), ...); };

      /// Used internally in Text, to sum up all types a variadic Text        
      /// constructor can accept                                              
      template<class...T>
      concept Stringifiable = ((Text<T>
           or BuiltinNumber<T>
           or Exception<T>
           or Meta<T>
           or Bytes<T>
           or HasNamedValues<T>
           or Similar<T, Anyness::Serial::Operator>
           or Inner::StringifiableByOperator<T>
         ) and ...);

      /// Used internally for specializing fmt::formatter without causing     
      /// ambiguities - excludes CT::BuiltinText, as those are already        
      /// defined by {fmt}, as well as other types, like CT::Exception, which 
      /// are defined by Langulus::Logger, etc.                               
      template<class...T>
      concept FmtStringifiable = ((not BuiltinText<T>
          and not Nullptr<T>
          and not Meta<T>
          and not Exception<T>
          and not BuiltinNumber<T>
          and not HasNamedValues<T>
          and (StringifiableByOperator<T> or StringifiableByConstructor<T>)
         ) and ...);

   } // namespace Langulus::CT::Inner

   /// A stringifiable type is one that has either an implicit or explicit    
   /// cast operator to Text type, or can be used to explicitly initialize a  
   /// Text container                                                         
   template<class...T>
   concept Stringifiable = ((Inner::StringifiableByOperator<T>
        or Inner::StringifiableByConstructor<T>) and ...);

} // namespace Langulus::CT

namespace Langulus::A
{

   /// Check if a type is compatible with CT::Character concept at runtime    
   struct Text {
      LANGULUS(ABSTRACT) true;
      LANGULUS(CONCRETE) Anyness::Text;

      static constexpr bool CTTI_TextTrait = true;

      ~Text() = delete;
   };

   struct Code;
}

namespace Langulus::Anyness
{

   ///                                                                        
   ///   Count-terminated UTF text container                                  
   ///                                                                        
   ///   This is a general purpose text container. It can contain serialized  
   /// data, but converting to it is a one way process. While serialization   
   /// aims at being isomorphic, converting to Text aims at readability only. 
   /// Consider it a general day to day speech container, that may or may not 
   /// be formal. If you want to serialize your data in a readable format,    
   /// convert to Flow::Code, or other isomorphic representations.            
   ///                                                                        
   struct Text : TAny<Letter> {
      using Base = TAny<Letter>;
      static constexpr bool CTTI_TextTrait = true;

      LANGULUS(DEEP) false;
      LANGULUS(POD) false;
      LANGULUS(FILES) "txt";
      LANGULUS_BASES(A::Text, Base);
      LANGULUS_CONVERTS_FROM(
         Index, Byte, bool, float, double,
         uint8_t, uint16_t, uint32_t, uint64_t,
         int8_t, int16_t, int32_t, int64_t,
         DMeta, TMeta, VMeta, CMeta, AMeta
      );

      /// The presence of this structure makes Text a serializer              
      struct SerializationRules {
         // Text serializer can be lossy to omit unnecessary details,   
         // and you can configure how many elements to show             
         #ifdef LANGULUS_MAX_DEBUGGABLE_ELEMENTS
            static constexpr Count MaxIterations
               = LANGULUS_MAX_DEBUGGABLE_ELEMENTS;
         #elif LANGULUS(DEBUG) or LANGULUS(SAFE)
            static constexpr Count MaxIterations = 32;
         #else
            static constexpr Count MaxIterations = 8;
         #endif

         using Operator = Serial::Operator;

         static constexpr auto Operators = Serial::Operators;
         static constexpr bool CriticalFailure = false;
         static constexpr bool SkipElements = true;

         static bool BeginScope(const Block&, Text&);
         static bool EndScope(const Block&, Text&);
         static bool Separate(const Block&, Text&);
         
         using Rules = Types<
            Serial::Rule<Serial::Wrap, Serial::BasedOn, A::Code,Operator::OpenCode,      Operator::CloseCode>,
            Serial::Rule<Serial::Wrap, Serial::BasedOn, Text,   Operator::OpenString,    Operator::CloseString>,
            Serial::Rule<Serial::Wrap, Serial::BasedOn, Bytes,  Operator::OpenByte,      Operator::CloseByte>,
            Serial::Rule<Serial::Wrap, Serial::Exact,   Letter, Operator::OpenCharacter, Operator::CloseCharacter>
         >;
      };

      using Operator = SerializationRules::Operator;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Text() noexcept = default;
      constexpr Text(::std::nullptr_t) noexcept;
      Text(const Text&);
      Text(Text&&) noexcept;

      template<class T> requires CT::TextBased<Desem<T>>
      Text(T&&);

      template<class T> requires CT::String<Desem<T>>
      Text(T&&);

      template<class T> requires CT::DenseCharacter<Desem<T>>
      Text(T&&);

      template<class T> requires CT::StdString<Desem<T>>
      Text(T&&);

      Text(const CT::Meta auto&);
      Text(const CT::Exception auto&);
      Text(const CT::Bytes auto&);
      Text(Byte);
      
      explicit Text(Operator);

      explicit Text(const CT::HasNamedValues auto&);
      explicit Text(const CT::BuiltinNumber auto&);

      template<class T1, class T2, class...TN>
      requires CT::Inner::Stringifiable<T1, T2, TN...>
      Text(T1&&, T2&&, TN&&...);

      template<class T> requires CT::String<Desem<T>>
      static Text From(T&&, Count);

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Text& operator = (const Text&);
      Text& operator = (Text&&) noexcept;

      template<class T> requires CT::TextBased<Desem<T>>
      Text& operator = (T&&);

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
      NOD() Text Strip(const CT::Text auto&) const;
      NOD() Text Replace(const CT::Text auto& what, const CT::Text auto& with) const;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T> requires CT::Stringifiable<Desem<T>>
      NOD() Text operator + (T&&) const;

      template<class T> requires CT::Stringifiable<Desem<T>>
      Text& operator += (T&&);

   protected:
      template<CT::TextBased THIS, class T>
      THIS ConcatInner(T&&) const;
      template<CT::TextBased THIS, class T>
      THIS& ConcatRelativeInner(T&&);

      void UnfoldInsert(auto&&);

   public:
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

   namespace A
   {

      ///                                                                     
      ///   Abstract code container                                           
      ///                                                                     
      struct Code : Anyness::Text {
         LANGULUS_BASES(Anyness::Text);
         LANGULUS(FILES) "";

         using Anyness::Text::Text;

         Code(const Anyness::Text& a)
            : Anyness::Text {a} {}

         Code(Anyness::Text&& a)
            : Anyness::Text {Forward<Anyness::Text>(a)} {}
      };

   } // namespace Langulus::A

} // namespace Langulus

namespace fmt
{

   ///                                                                        
   /// Extend FMT to be capable of logging anything that is convertible to    
   /// Anyness::Text. Constness of T doesn't matter.                          
   ///                                                                        
   template<Langulus::CT::Inner::FmtStringifiable T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT> LANGULUS(INLINED)
      auto format(const T& element, CONTEXT& ctx) const {
         using namespace Langulus;
         static_assert(CT::Complete<T>, "T isn't complete");
         auto& me = const_cast<T&>(element);

         Anyness::Text asText {me};
         const auto token = asText.operator Token();
         return fmt::format_to(ctx.out(), "{}", token);
      }
   };

} // namespace fmt