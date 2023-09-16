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
#include "TAny-Iteration.inl"
#include <charconv>
#include <limits>
#include <cstring>


namespace Langulus::Anyness
{

   using RTTI::MetaData;

   /// Text container copy-construction from base                             
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Text::Text(const Base& other)
      : Base {other} {
      // TAny's have no null-termination considerations, add them here  
      mCount = strnlen(GetRaw(), mCount);
   }

   /// Text container move-construction from base                             
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Text::Text(Base&& other) noexcept
      : Base {Forward<Base>(other)} {
      // TAny's have no null-termination considerations, add them here  
      mCount = strnlen(GetRaw(), mCount);
   }

   /// Text container copy-construction                                       
   /// Notice how container is explicitly cast to base class when forwarded   
   /// If that is not done, TAny will use the CT::Deep constructor instead    
   ///   @param other - container to reference                                
   LANGULUS(INLINED)
   Text::Text(const Text& other)
      : Base {static_cast<const Base&>(other)} {}

   /// Text container move-construction                                       
   ///   @param other - container to move                                     
   LANGULUS(INLINED)
   Text::Text(Text&& other) noexcept
      : Base {Forward<Base>(other)} {}

   /// Semantic text constructor                                              
   ///   @param other - the text container to use semantically                
   LANGULUS(INLINED)
   Text::Text(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;
      mType = MetaData::Of<TypeOf<Base>>();

      if constexpr (Relevant<S>) {
         BlockTransfer<Text>(other.template Forward<Base>());

         // Base constructor should handle initialization from anything 
         // TAny<Letter> based, but it will not make any null-          
         // termination corrections, so we have to do them here         
         if constexpr (not CT::Text<T>)
            mCount = strnlen(GetRaw(), mCount);
      }
      else if constexpr (CT::Exact<T, Letter>) {
         AllocateFresh(RequestSize(1));
         mRaw[0] = reinterpret_cast<Byte&>(*other);
      }
      else if constexpr (RawTextPointer<S>) {
         const Count count = *other ? ::std::strlen(*other) : 0;
         if constexpr (CT::Sparse<T>)
            SetMemory(DataState::Constrained, mType, count, *other, nullptr);
         else
            SetMemory(DataState::Constrained, mType, 1, &(*other), nullptr);

         if constexpr (not S::Move and S::Keep)
            TakeAuthority();
      }
      else LANGULUS_ERROR("Bad semantic construction");
   }

   /// Construct from compatible std::string                                  
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param text - the text to wrap                                       
   LANGULUS(INLINED)
   Text::Text(const CompatibleStdString& text)
      : Text {Copy(text.data()), text.size()} {}

   /// Construct from compatible std::string_view                             
   /// Data will be cloned if we don't have authority over the memory         
   ///   @param text - the text to wrap                                       
   LANGULUS(INLINED)
   Text::Text(const CompatibleStdStringView& text)
      : Text {Copy(text.data()), text.size()} {}

   /// Stringify a Langulus::Exception                                        
   ///   @param from - the exception to stringify                             
   LANGULUS(INLINED)
   Text::Text(const Exception& from) {
      (*this) += from.GetName();
      #if LANGULUS(DEBUG)
         (*this) += '(';
         (*this) += from.GetMessage();
         (*this) += " at ";
         (*this) += from.GetLocation();
         (*this) += ')';
      #endif
   }
   
   /// Stringify meta                                                         
   ///   @param meta - the definition to stringify                            
   LANGULUS(INLINED)
   Text::Text(const RTTI::Meta& meta)
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         : Text {meta.GetShortestUnambiguousToken()} {}
      #else
         : Text {meta.mToken} {}
      #endif

   /// Stringify meta                                                         
   ///   @param meta - the definition to stringify                            
   LANGULUS(INLINED)
   Text::Text(const RTTI::Meta* meta) {
      if (meta)
         *this = Text {*meta};
   }
   
   /// Construct from a single character                                      
   ///   @param anyCharacter - the character                                  
   LANGULUS(INLINED)
   Text::Text(const Letter& anyCharacter)
      : Text {Copy(&anyCharacter), 1} { }

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
   
   /// Construct from bounded array                                           
   ///   @tparam C - size of the array                                        
   ///   @param text - the bounded array                                      
   template<Count C>
   LANGULUS(INLINED)
   Text::Text(const Letter(&text)[C])
      : Text {Copy(text), strnlen(text, C)} { }

   /// Construct manually from count-terminated array                         
   ///   @param text - text memory to reference                               
   ///   @param count - number of characters inside text, not including any   
   ///                  termination character, if any                         
   LANGULUS(INLINED)
   Text::Text(const Letter* text, const Count& count)
      : Text {Copy(text), count} { }

   LANGULUS(INLINED)
   Text::Text(Letter* text, const Count& count)
      : Text {Copy(text), count} { }

   /// Semantic construction from count-terminated array                      
   ///   @param text - text memory to wrap                                    
   ///   @param count - number of characters inside text                      
   LANGULUS(INLINED)
   Text::Text(CT::Semantic auto&& text, const Count& count)
      : Base {Base::From(text.Forward(), count)} { }

   /// Construct from null-terminated array                                   
   ///   @param nullterminatedText - text memory to reference                 
   LANGULUS(INLINED)
   Text::Text(const Letter* nullterminatedText)
      : Text {Copy(nullterminatedText)} {}

   LANGULUS(INLINED)
   Text::Text(Letter* nullterminatedText)
      : Text {Copy(nullterminatedText)} {}

   /// Count the number of newline characters                                 
   ///   @return the number of newline characters + 1, or zero if empty       
   LANGULUS(INLINED)
   Count Text::GetLineCount() const noexcept {
      if (IsEmpty())
         return 0;

      Count lines {1};
      for (Count i = 0; i < mCount; ++i) {
         if ((*this)[i] == '\n')
            ++lines;
      }

      return lines;
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
   
   /// Assign a single character                                              
   ///   @param rhs - the character                                           
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Text& Text::operator = (const Letter& rhs) noexcept {
      return operator = (Copy(rhs));
   }
   
   /// Semantic assignment                                                    
   ///   @tparam S - the semantic and type of assignment (deducible)          
   ///   @param rhs - the right hand side                                     
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Text& Text::operator = (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (Relevant<S>) {
         Base::operator = (rhs.template Forward<Base>());

         // Base constructor should handle initialization from anything 
         // TAny<Letter> based, but it will not make any null-          
         // termination corrections, so we have to do them here.        
         if constexpr (not CT::Text<T>)
            mCount = strnlen(GetRaw(), mCount);
      }
      else if constexpr (CT::Same<T, Letter>)
         Base::operator = (rhs.Forward());
      else
         LANGULUS_ERROR("Bad semantic assignment");
      return *this;
   }

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
            mutableThis->GetRaw()[mCount] = '\0';
            return *this;
         }

         Block previousBlock {*this};
         const auto request = RequestSize(mCount + 1);
         mutableThis->mEntry = Allocator::Reallocate(request.mByteSize, mEntry);
         LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
         mutableThis->mReserved = request.mElementCount;

         if (mEntry != previousBlock.mEntry) {
            // Memory moved, and we should move all elements in it      
            mutableThis->mRaw = mEntry->GetBlockStart();
            CopyMemory(mutableThis->mRaw, previousBlock.mRaw, mCount);
            previousBlock.Free();
         }

         // Add the null-termination                                    
         mutableThis->GetRaw()[mCount] = '\0';
         return *this;
      }
      else {
         // We have to branch-off and make another allocation           
         Text result;
         const auto request = RequestSize(mCount + 1);
         result.mType = mType;
         result.AllocateFresh(request);
         result.mCount = mCount;
         result.mReserved = request.mElementCount;
         CopyMemory(result.mRaw, mRaw, mCount);
         result.GetRaw()[mCount] = '\0';
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

   /// Find a substring and set 'offset' to the start of the first match      
   /// The search begins at 'offset'                                          
   ///   @param pattern - the pattern to search for                           
   ///   @param offset - [in/out] offset to set if found                      
   ///   @attention offset might change, even if nothing was found            
   ///   @return true if pattern was found                                    
   LANGULUS(INLINED)
   bool Text::FindOffset(const Text& pattern, Offset& offset) const {
      if (pattern.IsEmpty() or offset >= mCount or pattern.mCount > mCount - offset)
         return false;

      auto lhs = GetRaw() + offset;
      auto rhs = pattern.GetRaw();
      const auto lhsEnd = GetRawEnd() - pattern.GetCount() + 1;
      const auto rhsEnd = pattern.GetRawEnd();
      while (lhs != lhsEnd) {
         if (*lhs == *rhs) {
            offset = lhs - GetRaw();
            ++lhs;
            ++rhs;

            while (rhs != rhsEnd and *lhs == *rhs) {
               ++lhs;
               ++rhs;
            }

            if (rhs == rhsEnd) {
               // Match found                                           
               return true;
            }

            lhs = GetRaw() + offset;
            rhs = pattern.GetRaw();
         }

         ++lhs;
      }

      return false;
   }

   /// Find a substring in reverse, and set offset to its location            
   ///   @param pattern - the pattern to search for                           
   ///   @param offset - [out] offset to set if found                         
   ///   @return true if pattern was found                                    
   LANGULUS(INLINED)
   bool Text::FindOffsetReverse(const Text& pattern, Offset& offset) const {
      if (pattern.IsEmpty() or offset >= mCount or pattern.mCount > mCount - offset)
         return false;

      auto lhs = GetRawEnd() - offset - pattern.GetCount();
      auto rhs = pattern.GetRaw();
      const auto lhsEnd = GetRaw() - 1;
      const auto rhsEnd = pattern.GetRawEnd();
      while (lhs != lhsEnd) {
         if (*lhs == *rhs) {
            offset = GetRawEnd() - lhs - 1;
            ++lhs;
            ++rhs;

            while (rhs != rhsEnd and *lhs == *rhs) {
               ++lhs;
               ++rhs;
            }

            if (rhs == rhsEnd) {
               // Match found                                           
               return true;
            }

            lhs = GetRawEnd() - offset - 1;
            rhs = pattern.GetRaw();
         }

         --lhs;
      }

      return false;
   }

   /// Find a pattern                                                         
   ///   @param pattern - the pattern to search for                           
   ///   @return true on first match                                          
   LANGULUS(INLINED)
   bool Text::Find(const Text& pattern) const {
      UNUSED() Offset unused {};
      return FindOffset(pattern, unused);
   }

   /// Pick a part of the text (const)                                        
   ///   @param start - offset of the starting character                      
   ///   @param count - the number of characters after 'start'                
   ///   @return new text that references the original memory                 
   LANGULUS(INLINED)
   Text Text::Crop(Count start, Count count) const {
      return Text {Base::Crop(start, count)};
   }

   /// Pick a part of the text                                                
   ///   @param start - offset of the starting character                      
   ///   @param count - the number of characters after 'start'                
   ///   @return new text that references the original memory                 
   LANGULUS(INLINED)
   Text Text::Crop(Count start, Count count) {
      return Text {Base::Crop(start, count)};
   }

   /// Remove all instances of a symbol from the text container               
   ///   @param symbol - the character to remove                              
   ///   @return a new container with the text stripped                       
   LANGULUS(INLINED)
   Text Text::Strip(Letter symbol) const {
      Text result;
      Count start {}, end {};
      for (Count i = 0; i <= mCount; ++i) {
         if (i == mCount or (*this)[i] == symbol) {
            const auto size = end - start;
            if (size) {
               auto segment = result.Extend(size);
               CopyMemory(segment.GetRaw(), GetRaw() + start, size);
            }

            start = end = i + 1;
         }
         else ++end;
      }

      return result;
   }

   /// Remove a part of the text                                              
   ///   @attention assumes end >= start                                      
   ///   @param start - the starting character                                
   ///   @param end - the ending character                                    
   ///   @return a reference to this text                                     
   LANGULUS(INLINED)
   Text& Text::Remove(Count start, Count end) {
      LANGULUS_ASSUME(UserAssumes, end >= start, "end < start");
      const auto removed = ::std::min(end, mCount) - ::std::min(start, mCount);
      if (0 == mCount or 0 == removed)
         return *this;

      LANGULUS_ASSERT(IsMutable(), Destruct,
         "Can't remove from constant container");

      if (end < mCount)
         MoveMemory(mRaw + start, mRaw + end, mCount - end);

      mCount -= removed;
      return *this;
   }

   /// Extend the text container and return a referenced part of it           
   ///   @return an array that represents the extended part                   
   LANGULUS(INLINED)
   Text Text::Extend(Count count) {
      return Base::Extend<Text>(count);
   }

   /// Hash the text                                                          
   ///   @return a hash of the contained byte sequence                        
   LANGULUS(INLINED)
   Hash Text::GetHash() const {
      return HashBytes(GetRaw(), static_cast<int>(GetCount()));
   }

   /// Interpret text container as a literal                                  
   ///   @attention the string is null-terminated only after Terminate()      
   LANGULUS(INLINED)
   Text::operator Token() const noexcept {
      return {GetRaw(), mCount};
   }

   /// Compare with a std::string                                             
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(INLINED)
   bool Text::operator == (const CompatibleStdString& rhs) const noexcept {
      return operator == (Text {Disown(rhs.data()), rhs.size()});
   }

   /// Compare with a std::string_view                                        
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(INLINED)
   bool Text::operator == (const CompatibleStdStringView& rhs) const noexcept {
      return operator == (Text {Disown(rhs.data()), rhs.size()});
   }

   /// Compare two text containers                                            
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(INLINED)
   bool Text::operator == (const Text& rhs) const noexcept {
      return Base::operator == (static_cast<const Base&>(rhs));
   }

   /// Compare with a null-terminated string                                  
   ///   @param rhs - the text to compare against                             
   ///   @return true if both strings are the same                            
   LANGULUS(INLINED)
   bool Text::operator == (const Letter* rhs) const noexcept {
      return operator == (Text {Disown(rhs)});
   }

   /// Concatenate two text containers                                        
   ///   @param rhs - right hand side                                         
   ///   @return the concatenated text container                              
   LANGULUS(INLINED)
   Text Text::operator + (const Text& rhs) const {
      if (rhs.IsEmpty())
         return *this;

      Text combined;
      combined.mType = MetaData::Of<Letter>();
      combined.AllocateFresh(RequestSize(mCount + rhs.mCount));
      combined.InsertInner<Copied<Letter>>(GetRaw(), GetRawEnd(), 0);
      combined.InsertInner<Copied<Letter>>(rhs.GetRaw(), rhs.GetRawEnd(), mCount);
      return Abandon(combined);
   }
   
   /// Concatenate two text containers                                        
   ///   @param rhs - right hand side                                         
   ///   @return the concatenated text container                              
   LANGULUS(INLINED)
   Text operator + (const char* lhs, const Text& rhs) {
      if (rhs.IsEmpty())
         return Text {lhs};

      Text leftside {lhs};
      Text combined;
      combined.mType = MetaData::Of<Letter>();
      combined.AllocateFresh(leftside.RequestSize(leftside.mCount + rhs.mCount));
      combined.InsertInner<Copied<Letter>>(leftside.GetRaw(), leftside.GetRawEnd(), 0);
      combined.InsertInner<Copied<Letter>>(rhs.GetRaw(), rhs.GetRawEnd(), leftside.mCount);
      return Abandon(combined);
   }

   /// Concatenate (destructively) text container                             
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Text& Text::operator += (const Text& rhs) {
      if (rhs.IsEmpty())
         return *this;

      mType = MetaData::Of<Letter>();
      AllocateMore(mCount + rhs.mCount);
      InsertInner<Copied<Letter>>(rhs.GetRaw(), rhs.GetRawEnd(), mCount);
      return *this;
   }

   /// Fill template arguments using libfmt                                   
   ///   @tparam ...ARGS - arguments for the template                         
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
      return Anyness::Text {text, size};
   }

} // namespace Langulus
