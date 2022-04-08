#pragma once
#include "Text.hpp"
#include <charconv>
#include <limits>
#include <cstring>

namespace Langulus::Anyness
{

	/// Construct from token																	
	///	@param text - the text to wrap													
	inline Text::Text(const Token& text)
		: Text {&text.front(), text.size()} {}

	/// Construct manually from count-terminated text									
	///	@param text - text memory to reference											
	///	@param count - number of characters inside text								
	template<Dense T>
	inline Text::Text(const T* text, Count count) requires Character<T>
		: Block {DataState::Constrained, PCMEMORY.GetFallbackMetaChar(), count, text} {
		bool no_jury;
		PCMEMORY.Reference(mType, mRaw, 1, no_jury);
		if (no_jury) {
			// We should monopolize the memory to avoid segfaults, in the	
			// case of the text container being initialized with temporary	
			// or static data																
			TakeAuthority();
		}
	}

	/// Construct from a character															
	///	@param value - the character to stringify										
	template<Dense T>
	Text::Text(const T& anyCharacter) requires Character<T>
		: Text {&anyCharacter, 1} {}

	/// Convert a number type to text														
	///	@param from - the number to stringify											
	template<Dense T>
	Text::Text(const T& number) requires Number<T>
		: Text {} {
		if constexpr (Real<T>) {
			// Stringify a real number													
			constexpr auto size = ::std::numeric_limits<T>::max_digits10 * 2;
			char temp[size];
			auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number, std::chars_format::general);
			if (errorCode != ::std::errc())
				throw Except::Convert("std::to_chars failure");

			while ((*lastChar == '0' || *lastChar == '.') && lastChar > temp) {
				if (*lastChar == '.')
					break;
				--lastChar;
			}

			(*this) = Text {temp, lastChar - temp};
		}
		else if constexpr (Integer<T>) {
			// Stringify an integer														
			constexpr auto size = ::std::numeric_limits<T>::digits10 * 2;
			char temp[size];
			auto [lastChar, errorCode] = ::std::to_chars(temp, temp + size, number);
			if (errorCode != ::std::errc())
				throw Except::Convert("std::to_chars failure");

			(*this) += Text {temp, lastChar - temp};
		}
		else LANGULUS_ASSERT("Unsupported number type");
	}

	/// Construct from null-terminated string												
	///	@param text - text memory to reference											
	template<Dense T>
	inline Text::Text(const T* nullterminatedText) requires Character<T>
		: Text {nullterminatedText, ::std::strlen(nullterminatedText)} {}

	/// Convert any pointer to text															
	///	@param from - the pointer to dereference and stringify					
	template<Dense T>
	Text::Text(const T* pointer) requires StaticallyConvertible<T, Text>
		: Text {} {
		if (!pointer)
			(*this) += "null";
		else
			(*this) += *pointer;
	}

	/// Raw character access (unsafe)														
	///	@attention the string is guaranteed to be null-terminated only after 
	///				  Terminate()																
	///	@return the pointer to the first character inside container				
	inline char8_t* Text::GetRaw() noexcept {
		return reinterpret_cast<char8_t*>(mRaw);
	}

	/// Raw constant character access (unsafe)											
	///	@attention the string is guaranteed to be null-terminated only after 
	///				  Terminate()																
	///	@return the pointer to the first character inside container				
	inline const char8_t* Text::GetRaw() const noexcept {
		return reinterpret_cast<const char8_t*>(mRaw);
	}

	/// Interpret text container as a literal												
	///	@attention the string is null-terminated only after Terminate()		
	constexpr Text::operator Token() const noexcept {
		return {GetRaw(), mCount};
	}

	/// Concatenate text with text															
	inline Text operator + (const Text& lhs, const Text& rhs) {
		Text result;
		result.Allocate(lhs.GetCount() + rhs.GetCount(), false, true);
		pcCopyMemory(lhs.GetRaw(), result.GetRaw(), lhs.GetCount());
		pcCopyMemory(rhs.GetRaw(), result.GetRaw() + lhs.GetCount(), rhs.GetCount());
		return result;
	}

	/// Concatenate anything but text with text											
	template<class T>
	Text operator + (const T& lhs, const Text& rhs) requires (!IsText<T>) {
		Text converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}

	/// Concatenate text with anything but text											
	template<class T>
	Text operator + (const Text& lhs, const T& rhs) requires (!IsText<T>) {
		Text converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}

	/// String concatenation in place														
	template<class T>
	Text& Text::operator += (const T& rhs) {
		if constexpr (IsText<T>) {
			auto mpoint = Extend(pcVal(rhs).GetCount());
			pcCopyMemory(pcVal(rhs).GetRaw(), mpoint.GetRaw(), pcVal(rhs).GetCount());
		}
		else {
			// Finally, attempt converting											
			Text converted;
			TConverter<T, Text>::Convert(rhs, converted);
			operator += (converted);
		}
		return *this;
	}

} // namespace Langulus::Anyness
