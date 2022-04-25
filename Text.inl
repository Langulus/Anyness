#pragma once
#include "Text.hpp"
#include <charconv>
#include <limits>
#include <cstring>

namespace Langulus::Anyness
{

	/// Construct from token																	
	/// Data will be cloned if we don't have authority over the memory			
	///	@param text - the text to wrap													
	inline Text::Text(const Token& text)
		: Text {&text.front(), text.size()} {}

	/// Construct manually from count-terminated C string								
	/// Data will be cloned if we don't have authority over the memory			
	///	@param text - text memory to reference											
	///	@param count - number of characters inside text								
	inline Text::Text(const char* text, const Count& count)
		: TAny {reinterpret_cast<const char8_t*>(text), count} { }

	template<Count C>
	inline Text::Text(const char(&text)[C])
		: TAny {reinterpret_cast<const char8_t*>(text), C - 1} { }

	/// Construct manually from count-terminated UTF text								
	/// Data will be cloned if we don't have authority over the memory			
	///	@param text - text memory to reference											
	///	@param count - number of characters inside text								
	template<IsDense T>
	inline Text::Text(const T* text, const Count& count) requires IsCharacter<T>
		: TAny {text, count} { }

	template<IsDense T, Count C>
	inline Text::Text(const T(&text)[C]) requires IsCharacter<T>
		: TAny {text, C - 1} { }

	/// Construct from a single character													
	/// Data will be cloned if we don't have authority over the memory			
	///	@param anyCharacter - the character to stringify							
	template<IsDense T>
	Text::Text(const T& anyCharacter) requires IsCharacter<T>
		: Text {&anyCharacter, 1} {}

	/// Convert a number type to text														
	///	@param from - the number to stringify											
	template<IsDense T>
	Text::Text(const T& number) requires IsNumber<T> {
		if constexpr (IsReal<T>) {
			// Stringify a real number													
			constexpr auto size = ::std::numeric_limits<T>::max_digits10 * 2;
			char temp[size];
			auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number, ::std::chars_format::general);
			if (errorCode != ::std::errc())
				throw Except::Convert("std::to_chars failure");

			while ((*lastChar == '0' || *lastChar == '.') && lastChar > temp) {
				if (*lastChar == '.')
					break;
				--lastChar;
			}

			(*this) = Text {temp, lastChar - temp};
		}
		else if constexpr (IsInteger<T>) {
			// Stringify an integer														
			constexpr auto size = ::std::numeric_limits<T>::digits10 * 2;
			char temp[size];
			auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number);
			if (errorCode != ::std::errc())
				throw Except::Convert("std::to_chars failure");

			(*this) += Text {temp, static_cast<Count>(lastChar - temp)};
		}
		else LANGULUS_ASSERT("Unsupported number type");
	}

	/// Construct from null-terminated C string											
	/// Data will be cloned if we don't have authority over the memory			
	///	@param nullterminatedText - text memory to reference						
	inline Text::Text(const char* nullterminatedText)
		: Text {nullterminatedText, ::std::strlen(nullterminatedText)} {}

	/// Construct from null-terminated UTF text											
	/// Data will be cloned if we don't have authority over the memory			
	///	@param nullterminatedText - text memory to reference						
	template<IsDense T>
	inline Text::Text(const T* nullterminatedText) requires IsCharacter<T>
		: Text {nullterminatedText, ::std::strlen(nullterminatedText)} {}

	/// Interpret text container as a literal												
	///	@attention the string is null-terminated only after Terminate()		
	constexpr Text::operator Token() const noexcept {
		return {GetRaw(), mCount};
	}

	template<class RHS>
	Text& Text::operator += (const RHS& rhs) {
		TAny<Letter>::operator+=<Text, RHS>(rhs);
		return *this;
	}

	template<class RHS>
	NOD() Text Text::operator + (const RHS& rhs) const {
		return TAny<Letter>::operator+<Text, RHS>(rhs);
	}

} // namespace Langulus::Anyness
