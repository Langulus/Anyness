///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
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
		: Text {text.data(), text.size()} {}

	/// Construct manually from count-terminated C string								
	/// Data will be cloned if we don't have authority over the memory			
	///	@param text - text memory to reference											
	///	@param count - number of characters inside text								
	inline Text::Text(const Letter* text, const Count& count)
		: TAny {text, count} { }

	/// Construct manually from count-terminated C string								
	/// Data will never be cloned or referenced											
	///	@param text - text memory to wrap												
	///	@param count - number of characters inside text								
	inline Text::Text(Disowned<const Letter*>&& text, const Count& count)
		: TAny {Disown(text.mValue), count} { }

	/// Construct manually from a c style array											
	/// Data will be cloned if we don't have authority over the memory			
	///	@tparam T - type of the character in the array								
	///	@tparam C - size of the array														
	///	@param text - the array																
	template<Count C>
	inline Text::Text(const Letter(&text)[C])
		: TAny {text, C - 1} { }

	/// Construct from a single character													
	/// Data will be cloned if we don't have authority over the memory			
	///	@tparam T - type of the character in the array								
	///	@param anyCharacter - the character to stringify							
	inline Text::Text(const Letter& anyCharacter)
		: Text {&anyCharacter, 1} {}

	/// Convert a number type to text														
	///	@tparam T - number type to stringify											
	///	@param number - the number to stringify										
	template<CT::Dense T>
	Text::Text(const T& number) requires CT::Number<T> {
		if constexpr (CT::Real<T>) {
			// Stringify a real number													
			constexpr auto size = ::std::numeric_limits<T>::max_digits10 * 2;
			char temp[size];
			auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number, ::std::chars_format::general);
			if (errorCode != ::std::errc())
				Throw<Except::Convert>("std::to_chars failure");

			while ((*lastChar == '0' || *lastChar == '.') && lastChar > temp) {
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
			if (errorCode != ::std::errc())
				Throw<Except::Convert>("std::to_chars failure");

			(*this) += Text {temp, static_cast<Count>(lastChar - temp)};
		}
		else LANGULUS_ASSERT("Unsupported number type");
	}

	/// Construct from null-terminated UTF text											
	/// Data will be cloned if we don't have authority over the memory			
	///	@param nullterminatedText - text memory to reference						
	inline Text::Text(const Letter* nullterminatedText)
		: Text {nullterminatedText, ::std::strlen(reinterpret_cast<const char*>(nullterminatedText))} {}

	/// Construct from null-terminated UTF text											
	/// Data will never be cloned or referenced											
	///	@param nullterminatedText - text to wrap										
	inline Text::Text(Disowned<const Letter*>&& nullterminatedText)
		: Text {Move(nullterminatedText), ::std::strlen(reinterpret_cast<const char*>(nullterminatedText.mValue))} {}

	/// Interpret text container as a literal												
	///	@attention the string is null-terminated only after Terminate()		
	inline Text::operator Token() const noexcept {
		return {GetRaw(), mCount};
	}

	/// Destructive concatenatenation of anything convertible to Text				
	///	@tparam RHS - type to stringify (deducible)									
	///	@param rhs - the data to stringify												
	///	@return a reference to this Text													
	template<class RHS>
	Text& Text::operator += (const RHS& rhs) {
		TAny::operator+=<Text, RHS>(rhs);
		return *this;
	}

	/// Concatenatenation of anything convertible to Text								
	///	@tparam RHS - type to stringify (deducible)									
	///	@param rhs - the data to stringify												
	///	@return a new Text container with both sides concatenated				
	template<class RHS>
	NOD() Text Text::operator + (const RHS& rhs) const {
		return TAny::operator+<Text, RHS>(rhs);
	}

	/// Compare two text containers															
	///	@param rhs - the text to compare against										
	///	@return true if both strings are the same										
	inline bool Text::operator == (const Text& rhs) const noexcept {
		return TAny::operator == (static_cast<const TAny&>(rhs));
	}

	/// Compare with a string																	
	///	@param rhs - the text to compare against										
	///	@return true if both strings are the same										
	inline bool Text::operator == (const Letter* rhs) const noexcept {
		return operator == (Text {Disown(rhs)});
	}

	/// Check if text container has been allocated										
	///	@return true if container is not allocated									
	inline bool Text::operator == (::std::nullptr_t) const noexcept {
		return Block::operator == (nullptr);
	}

} // namespace Langulus::Anyness

namespace Langulus
{

	/// Make a text literal																		
	inline Anyness::Text operator "" _text(const char* text, ::std::size_t size) {
		return Anyness::Text {text, size};
	}

}