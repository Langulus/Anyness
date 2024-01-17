///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Text.hpp"
#include "TAny.inl"
#include <charconv>
#include <limits>
#include <cstring>


namespace Langulus::Anyness
{

   /// Default construction with nullptr_t                                    
   LANGULUS(INLINED)
   constexpr Text::Text(::std::nullptr_t) noexcept {}

   /// Shallow-copy constructor                                               
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Text::Text(const Text& other)
      : Text {Copy(other)} { }

   /// Move constructor                                                       
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Text::Text(Text&& other) noexcept
      : Text {Move(other)} { }

   /// Semantic text constructor                                              
   ///   @param other - the text container to use semantically                
   template<class T> requires CT::Block<Desem<T>> LANGULUS(INLINED)
   Text::Text(T&& other)
      : Base {Forward<T>(other)} {
      if constexpr (not CT::Text<Desem<T>>) {
         // Only Text container is guaranteed to be properly terminated 
         mCount = strnlen(GetRaw(), mCount);
      }
   }

   /// Construct from single character                                        
   ///   @param other - the character to copy                                 
   /*template<class T> requires CT::DenseCharacter<Desem<T>> LANGULUS(INLINED)
   Text::Text(T&& other) {
      AllocateFresh<Text>(RequestSize<Text>(1));
      (*this)[0] = DesemCast(other);
   }

   /// Construct from a char*                                                 
   ///   @attention assumes that the character sequence is null-terminated    
   ///   @param other - the string and semantic                               
   template<class T> requires CT::StringPointer<Desem<T>> LANGULUS(INLINED)
   Text::Text(T&& other) {
      using S = SemanticOf<T>;
      const Count count = DesemCast(other)
         ? ::std::strlen(DesemCast(other)) : 0;

      if (not count)
         return;

      SetMemory(DataState::Constrained, mType, count, DesemCast(other));
      if constexpr (S::Move or S::Keep)
         TakeAuthority<Text>();
   }

   /// Construct from a bounded character array                               
   ///   @attention assumes that the character sequence is null-terminated    
   ///      - this is usually guaranteed for string literals, but its left    
   ///        to an assumption, as the user can reinterpret_cast as array     
   ///   @param other - the string and semantic                               
   template<class T> requires CT::StringLiteral<Desem<T>> LANGULUS(INLINED)
   Text::Text(T&& other) {
      using S = SemanticOf<T>;
      const Count count = DesemCast(other)
         ? strnlen(DesemCast(other), ExtentOf<Desem<T>>) : 0;

      if (not count)
         return;

      SetMemory(DataState::Constrained, mType, count, DesemCast(other));
      if constexpr (S::Move or S::Keep)
         TakeAuthority<Text>();
   }

   /// Construct from any standard contiguous range statically typed with     
   /// characters. This includes std::string, string_view, span, vector,      
   /// array, etc. Containers that aren't strings will be strnlen'ed          
   ///   @param other - the string and semantic                               
   template<class T> requires (CT::StandardContiguousContainer<Desem<T>>
                          and  CT::DenseCharacter<TypeOf<Desem<T>>>)
   LANGULUS(INLINED) Text::Text(T&& other) {
      using S = SemanticOf<T>;
      if (DesemCast(other).empty())
         return;

      Count count;
      if constexpr (not ::std::is_convertible_v<Desem<T>, std::string_view>)
         count = strnlen(DesemCast(other).data(), DesemCast(other).size());
      else
         count = DesemCast(other).size();

      SetMemory(
         DataState::Constrained, mType,
         DesemCast(other).size(), DesemCast(other).data()
      );
      if constexpr (S::Move or S::Keep)
         TakeAuthority<Text>();
   }

   /// Stringify a Langulus::Exception                                        
   ///   @param from - the exception to stringify                             
   LANGULUS(INLINED)
   Text::Text(const Exception& from) {
      #if LANGULUS(DEBUG)
         (*this) = TemplateRt("{}({} at {})",
            from.GetName(),
            from.GetMessage(),
            from.GetLocation()
         );
      #else
         (*this) = Disown(from.GetName());
      #endif
   }
   
   /// Stringify meta                                                         
   ///   @param meta - the definition to stringify                            
   LANGULUS(INLINED)
   Text::Text(const CT::Meta auto& meta)
      : Text {meta.GetToken()} {}

   /// Convert a number type to text                                          
   ///   @param number - the number to stringify                              
   LANGULUS(INLINED)
   Text::Text(const CT::DenseBuiltinNumber auto& number) {
      using T = Decay<decltype(number)>;

      if constexpr (CT::Real<T>) {
         // Stringify a real number                                     
         constexpr auto size = ::std::numeric_limits<T>::max_digits10 * 2;
         char temp[size];
         auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number, ::std::chars_format::general);
         LANGULUS_ASSERT(errorCode == ::std::errc(), Convert,
            "std::to_chars failure");

         while ((*lastChar == '0' || *lastChar == '.') and lastChar > temp) {
            if (*lastChar == '.')
               break;
            --lastChar;
         }

         (*this) = Text::From(temp, static_cast<Count>(lastChar - temp));
      }
      else if constexpr (CT::Integer<T>) {
         // Stringify an integer                                        
         constexpr auto size = ::std::numeric_limits<T>::digits10 * 2;
         char temp[size];
         auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number);
         LANGULUS_ASSERT(errorCode == ::std::errc(), Convert,
            "std::to_chars failure");

         (*this) += Text::From(temp, static_cast<Count>(lastChar - temp));
      }
      else LANGULUS_ERROR("Unsupported number type");
   }*/

   /// Semantic construction from count-terminated array                      
   ///   @param text - text memory to wrap                                    
   ///   @param count - number of characters inside text                      
   template<class T> requires CT::String<Desem<T>>
   Text Text::From(T&& text, Count count) {
      return Base::From(Forward<T>(text), count);
   }

   /// Shallow copy assignment                                                
   ///   @param rhs - the text to copy                                        
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Text& Text::operator = (const Text& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the text to move                                        
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Text& Text::operator = (Text&& rhs) noexcept {
      return operator = (Move(rhs));
   }
   
   /// Assign a block of any kind                                             
   ///   @param rhs - the block to assign                                     
   template<class T> requires CT::Text<Desem<T>> LANGULUS(INLINED)
   Text& Text::operator = (T&& rhs) {
      Base::operator = (Forward<T>(rhs));

      // Base constructor should handle initialization from anything    
      // TAny<Letter> based, but it will not make any null-             
      // termination corrections, so we have to do them here.           
      /*if constexpr (not CT::Text<Desem<T>>)
         mCount = strnlen(GetRaw(), mCount);*/
      return *this;
   }

   /// Assign a single character                                              
   ///   @param rhs - the character                                           
   ///   @return a reference to this container                                
   /*template<class T> requires CT::DenseCharacter<Desem<T>> LANGULUS(INLINED)
   Text& Text::operator = (T&& rhs) {
      Base::operator = (Forward<T>(rhs));
      return *this;
   }

   /// Assign a null-terminated string pointer                                
   ///   @param rhs - the pointer                                             
   ///   @return a reference to this container                                
   template<class T> requires CT::StringPointer<Desem<T>> LANGULUS(INLINED)
   Text& Text::operator = (T&& rhs) {
      Base::operator = (Forward<T>(rhs));

      // Base constructor should handle initialization from anything    
      // TAny<Letter> based, but it will not make any null-             
      // termination corrections, so we have to do them here.           
      if constexpr (not CT::Text<Desem<T>>)
         mCount = strlen(GetRaw());

      using S = SemanticOf<T>;
      if constexpr (S::Move or S::Keep)
         TakeAuthority<Text>();
      return *this;
   }
   
   /// Assign a bounded array string                                          
   ///   @param rhs - the pointer                                             
   ///   @return a reference to this container                                
   template<class T> requires CT::StringLiteral<Desem<T>> LANGULUS(INLINED)
   Text& Text::operator = (T&& rhs) {
      Base::operator = (Forward<T>(rhs));

      // Base constructor should handle initialization from anything    
      // TAny<Letter> based, but it will not make any null-             
      // termination corrections, so we have to do them here.           
      if constexpr (not CT::Text<Desem<T>>)
         mCount = strnlen(GetRaw(), ExtentOf<Desem<T>>);

      using S = SemanticOf<T>;
      if constexpr (S::Move or S::Keep)
         TakeAuthority<Text>();
      return *this;
   }

   /// Assign any standard contiguous range statically typed with             
   /// characters. This includes std::string, string_view, span, vector,      
   /// array, etc. Containers that aren't strings will be strnlen'ed          
   ///   @param rhs - the string and semantic                                 
   ///   @return a reference to this container                                
   template<class T> requires (CT::StandardContiguousContainer<Desem<T>>
                          and  CT::DenseCharacter<TypeOf<Desem<T>>>)
   LANGULUS(INLINED) Text& Text::operator = (T&& rhs) {
      Base::operator = (Forward<T>(rhs));

      Count count;
      if constexpr (not ::std::is_convertible_v<Desem<T>, std::string_view>)
         count = strnlen(DesemCast(rhs).data(), DesemCast(rhs).size());
      else
         count = DesemCast(rhs).size();

      using S = SemanticOf<T>;
      if constexpr (S::Move or S::Keep)
         TakeAuthority<Text>();
      return *this;
   }*/

   #if LANGULUS_FEATURE(UNICODE)
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

         to.Trim(newCount);
         return to;
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

         to.Trim(newCount);
         return to;
      }
   #endif
      
   /// Count the number of newline characters                                 
   ///   @return the number of newline characters + 1, or zero if empty       
   LANGULUS(INLINED)
   Count Text::GetLineCount() const noexcept {
      if (IsEmpty())
         return 0;

      Count lines = 1;
      for (Count i = 0; i < mCount; ++i) {
         if ((*this)[i] == '\n')
            ++lines;
      }

      return lines;
   }

   /// Terminate text so that it ends with a zero character at the end        
   /// This works even if the text container was empty                        
   ///   @return null-terminated equivalent to this Text                      
   LANGULUS(INLINED)
   Text Text::Terminate() const {
      // This greatly improves performance, but gives inconsistent      
      // results if MEMORY_STATISTICS feature is enabled (gives false   
      // positives for leaks if by random chance, the end of the string 
      // turns out to be \0 due to memory junk).                        
      // So, when MEMORY_STATISTICS is enabled we sacrifice performance 
      // for memory consistency. Generally, MEMORY_STATISTICS           
      // sacrifices performance anyways.                                
      #if not LANGULUS_FEATURE(MEMORY_STATISTICS)
         if (mReserved > mCount and GetRaw()[mCount] == '\0')
            return *this;
      #endif

      if (mEntry and mEntry->GetUses() == 1) {
         auto mutableThis = const_cast<Text*>(this);

         // If we have ownership and this is the only place where data  
         // is referenced, then we can simply resize this container,    
         // and attach a zero at the end of it                          
         if (mReserved > mCount) {
            // Required memory is already available                     
            mutableThis->GetRaw<Text>()[mCount] = '\0';
            return *this;
         }

         Block previousBlock {*this};
         const auto request = RequestSize<Text>(mCount + 1);
         mutableThis->mEntry = Allocator::Reallocate(
            request.mByteSize, const_cast<Allocation*>(mEntry));
         LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
         mutableThis->mReserved = request.mElementCount;

         if (mEntry != previousBlock.mEntry) {
            // Memory moved, and we should move all elements in it      
            mutableThis->mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
            CopyMemory(mutableThis->mRaw, previousBlock.mRaw, mCount);
            previousBlock.Free<Text>();
         }

         // Add the null-termination                                    
         mutableThis->GetRaw<Text>()[mCount] = '\0';
         return *this;
      }
      else {
         // We have to branch-off and make another allocation           
         Text result;
         const auto request = RequestSize<Text>(mCount + 1);
         result.mType = mType;
         result.AllocateFresh<Text>(request);
         result.mCount = mCount;
         //result.mReserved = request.mElementCount;
         CopyMemory(result.mRaw, mRaw, mCount);
         result.GetRaw<Text>()[mCount] = '\0';
         return Abandon(result);
      }
   }

   /// Make all letters lowercase                                             
   ///   @return a new text container with all letter made lowercase          
   LANGULUS(INLINED)
   Text Text::Lowercase() const {
      Text result {Clone(*this)};
      for (auto& i : result)
         i = static_cast<Letter>(::std::tolower(i));
      return result;
   }

   /// Make all letters uppercase                                             
   ///   @return a new text container with all letter made uppercase          
   LANGULUS(INLINED)
   Text Text::Uppercase() const {
      Text result {Clone(*this)};
      for (auto& i : result)
         i = static_cast<Letter>(::std::toupper(i));
      return result;
   }

   /// Select a substring                                                     
   ///   @param start - offset of the starting character                      
   ///   @param count - the number of characters after 'start'                
   ///   @return new text that references the original memory                 
   LANGULUS(INLINED)
   Text Text::Crop(CT::Index auto start, Count count) const {
      return Block::Crop<Text>(start, count);
   }

   LANGULUS(INLINED)
   Text Text::Crop(CT::Index auto start, Count count) {
      return Block::Crop<Text>(start, count);
   }

   /// Select a substring on the right of the index                           
   ///   @param start - offset of the starting character                      
   ///   @return new text that references the original memory                 
   LANGULUS(INLINED)
   Text Text::Crop(CT::Index auto start) const {
      const auto index = SimplifyIndex<Text>(start);
      return Crop(index, mCount - index);
   }

   LANGULUS(INLINED)
   Text Text::Crop(CT::Index auto start) {
      const auto index = SimplifyIndex<Text>(start);
      return Crop(index, mCount - index);
   }

   /// Remove all instances of a symbol from the text container               
   ///   @param symbol - the character to remove                              
   ///   @return a new container with the text stripped                       
   LANGULUS(INLINED)
   Text Text::Strip(const Letter symbol) const {
      Text result;
      Count start = 0, end = 0;
      for (Count i = 0; i <= mCount; ++i) {
         if (i == mCount or (*this)[i] == symbol) {
            const auto size = end - start;
            if (size) {
               auto segment = result.Extend(size);
               CopyMemory(segment.GetRaw<Text>(), GetRaw<Text>() + start, size);
            }

            start = end = i + 1;
         }
         else ++end;
      }

      return result;
   }

   /// Extend the text container and return a referenced part of it           
   ///   @return a container that represents the extended part                
   LANGULUS(INLINED)
   Text Text::Extend(const Count count) {
      return Block::Extend<Text>(count);
   }

   /// Hash the text                                                          
   ///   @return a hash of the contained byte sequence                        
   LANGULUS(INLINED)
   Hash Text::GetHash() const {
      return HashBytes(GetRaw<Text>(), static_cast<int>(GetCount()));
   }

   /// Interpret text container as a string_view                              
   ///   @attention the string is null-terminated only after Terminate()      
   LANGULUS(INLINED)
   Text::operator Token() const noexcept {
      return {GetRaw<Text>(), mCount};
   }

   /// Compare with another block                                             
   ///   @param rhs - the block to compare with                               
   ///   @return true if blocks are the same                                  
   LANGULUS(INLINED)
   bool Text::operator == (const CT::Block auto& rhs) const noexcept {
      using B = Deref<decltype(rhs)>;
      if constexpr (CT::Typed<B>) {
         if constexpr (CT::Similar<Letter, TypeOf<B>>) {
            // Comparing with another Text or TAny<Letter> - we can     
            // compare directly                                         
            return Base::operator == (rhs);
         }
         else if constexpr (CT::DenseCharacter<TypeOf<B>>) {
            // We're comparing with a different type of characters -    
            // do UTF conversions here                                  
            TODO();
         }
         else return false;
      }
      else {
         // Type-erased compare                                         
         return Base::operator == (rhs);
      }
   }

   /// Compare with a single character                                        
   ///   @param rhs - the character to compare with                           
   ///   @return true if this container contains this exact character         
   LANGULUS(INLINED)
   bool Text::operator == (const CT::DenseCharacter auto& rhs) const noexcept {
      return operator == (Text::From(Disown(&rhs), 1));
   }

   /// Compare with a null-terminated or bounded literal string               
   ///   @param rhs - the string to compare with                              
   ///   @return true if this container contains this exact string            
   LANGULUS(INLINED)
   bool Text::operator == (const CT::String auto& rhs) const noexcept {
      return operator == (Text {Disown(rhs)});
   }

   /// Compare with standard contiguous range statically typed with           
   /// characters. This includes std::string, string_view, span, vector,      
   /// array, etc. Containers that aren't playing by string rules will have   
   /// their data strnlen'd                                                   
   ///   @param rhs - the string to compare with                              
   ///   @return true if this container contains this exact string            
   LANGULUS(INLINED) bool Text::operator == (const CT::StdString auto& rhs) const noexcept {
      return operator == (Text {Disown(rhs)});
   }

   /// Comparing against nullptr_t checks if container is empty               
   ///   @return true if container is empty                                   
   LANGULUS(INLINED)
   bool Text::operator == (::std::nullptr_t) const noexcept {
      return IsEmpty();
   }

   /// Concatenate two text containers                                        
   ///   @param rhs - right hand side                                         
   ///   @return the concatenated text container                              
   /*template<class T> requires CT::Text<Desem<T>> LANGULUS(INLINED)
   Text Text::operator + (T&& rhs) const {
      using S = SemanticOf<T>;
      using B = TypeOf<S>;
      if constexpr (CT::Typed<B>) {
         if constexpr (CT::Similar<Letter, TypeOf<B>>) {
            // We can concat directly                                   
            return Block::ConcatBlock<Text>(S::Nest(rhs));
         }
         else if constexpr (CT::DenseCharacter<TypeOf<B>>) {
            // We're concatenating with different type of characters -  
            // do UTF conversions here                                  
            TODO();
         }
         else LANGULUS_ERROR("Can't concatenate with this container");
      }
      else {
         // Type-erased concat                                          
         return Block::ConcatBlock<Text>(S::Nest(rhs));
      }
   }

   /// Concatenate with stuff to the left                                     
   ///   @param rhs - right hand side                                         
   ///   @return the concatenated text container                              
   template<class T> requires CT::DenseCharacter<Desem<T>>
   LANGULUS(INLINED) Text operator + (T&& lhs, const Text& rhs) {
      if constexpr (CT::Semantic<T>)
         return Text::From(Disown(&*lhs), 1) + rhs;
      else
         return Text::From(Disown(&lhs), 1) + rhs;
   }

   template<class T> requires CT::StringPointer<Desem<T>>
   LANGULUS(INLINED) Text operator + (T&& lhs, const Text& rhs) {
      return Text {Disown(lhs)} + rhs;
   }

   template<class T> requires CT::StringLiteral<Desem<T>>
   LANGULUS(INLINED) Text operator + (T&& lhs, const Text& rhs) {
      return Text {Disown(lhs)} + rhs;
   }

   template<class T> requires (CT::StandardContiguousContainer<T>
                          and  CT::DenseCharacter<TypeOf<T>>)
   LANGULUS(INLINED) Text operator + (T&& lhs, const Text& rhs) {
      return Text {Disown(lhs)} + rhs;
   }*/

   /// Concatenate (destructively) text containers                            
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this container                                
   template<class T> requires CT::Text<Desem<T>>
   Text& Text::operator += (T&& rhs) {
      using S = SemanticOf<T>;
      using B = TypeOf<S>;

      if constexpr (CT::Block<T>) {
         if constexpr (CT::Typed<B>) {
            if constexpr (CT::Similar<Letter, TypeOf<B>>) {
               // We can concat directly                                
               Block::InsertBlock<Text, void>(IndexBack, S::Nest(rhs));
            }
            else if constexpr (CT::DenseCharacter<TypeOf<B>>) {
               // We're concatenating with different type of characters 
               // - do UTF conversions here                             
               TODO();
            }
            else LANGULUS_ERROR("Can't concatenate with this container");
         }
         else {
            // Type-erased concat                                       
            Block::InsertBlock<Text, void>(IndexBack, S::Nest(rhs));
         }
      }
      else TODO();

      return *this;
   }

   /// Fill template arguments using libfmt                                   
   /// If you get a constexpr error in this function, use TemplateRt instead  
   ///   @param format - the template string                                  
   ///   @param args... - the arguments                                       
   ///   @return the instantiated template                                    
   template<class... ARGS>
   Text Text::Template(const Token& format, ARGS&&...args) {
      const auto size = fmt::formatted_size(format, Forward<ARGS>(args)...);
      Text result;
      result.Reserve(size);
      fmt::format_to_n(
         result.GetRaw(), size, format,
         Forward<ARGS>(args)...
      );
      return Abandon(result);
   }
   
   /// Fill template arguments using libfmt (at runtime)                      
   ///   @tparam ...ARGS - arguments for the template                         
   ///   @param format - the template string                                  
   ///   @param args... - the arguments                                       
   ///   @return the instantiated template                                    
   template<class... ARGS>
   Text Text::TemplateRt(const Token& format, ARGS&&...args) {
      const auto size = fmt::formatted_size(
         fmt::runtime(format),
         Forward<ARGS>(args)...
      );

      Text result;
      result.Reserve(size);
      fmt::format_to_n(
         result.GetRaw(), size,
         fmt::runtime(format),
         Forward<ARGS>(args)...
      );
      return Abandon(result);
   }

   /// Should complain at compile-time if format string has mismatching       
   /// number of arguments                                                    
   template<::std::size_t...N>
   LANGULUS(INLINED)
   constexpr auto Text::CheckPattern(const Token& f, ::std::index_sequence<N...>) {
      return fmt::format_string<decltype(N)...> {fmt::runtime(f)};
   }

   /// Attempt filling the template, statically checking if argument count is 
   /// satisfied, and other erroneous conditions. Since arguments contain     
   /// unformattable data, that has more to do with seeking actual data from  
   /// the node hierarchy later, arguments are substituted with an index      
   /// sequence.                                                              
   ///   @tparam ...ARGS - arguments for the template                         
   ///   @param format - the template string                                  
   ///   @param args... - the arguments                                       
   template<class... ARGS>
   LANGULUS(INLINED)
   constexpr auto Text::TemplateCheck(const Token& f, ARGS&&...) {
      return CheckPattern(f, ::std::make_index_sequence<sizeof...(ARGS)> {});
   }



   /// Construct by copying a text container                                  
   ///   @param other - the container to copy                                 
   LANGULUS(INLINED)
   Debug::Debug(const Text& other)
      : Text {other} {}

   /// Construct by moving a text container                                   
   ///   @param other - the container to copy                                 
   LANGULUS(INLINED)
   Debug::Debug(Text&& other)
      : Text {Forward<Text>(other)} {}

} // namespace Langulus::Anyness

namespace Langulus
{

   /// Make a text literal                                                    
   LANGULUS(INLINED)
   Anyness::Text operator "" _text(const char* text, ::std::size_t size) {
      return Anyness::Text::From(Disown(text), size);
   }

} // namespace Langulus
