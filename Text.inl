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

	/// Construct manually from count-terminated text									
	/// Data will be cloned if we don't have authority over the memory			
	///	@param text - text memory to reference											
	///	@param count - number of characters inside text								
	template<Dense T>
	inline Text::Text(const T* text, const Count& count) requires Character<T>
		: TAny {text, count} { }

	/// Construct from a single character													
	/// Data will be cloned if we don't have authority over the memory			
	///	@param anyCharacter - the character to stringify							
	template<Dense T>
	Text::Text(const T& anyCharacter) requires Character<T>
		: Text {&anyCharacter, 1} {}

	/// Convert a number type to text														
	///	@param from - the number to stringify											
	template<Dense T>
	Text::Text(const T& number) requires Number<T> {
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
	/// Data will be cloned if we don't have authority over the memory			
	///	@param text - text memory to reference											
	template<Dense T>
	inline Text::Text(const T* nullterminatedText) requires Character<T>
		: Text {nullterminatedText, ::std::strlen(nullterminatedText)} {}

	/// Convert any pointer to text															
	///	@param from - the pointer to dereference and stringify					
	template<Dense T>
	Text::Text(const T* pointer) requires StaticallyConvertible<T, Text> {
		if (!pointer)
			(*this) += "null";
		else
			(*this) += *pointer;
	}

	/// Interpret text container as a literal												
	///	@attention the string is null-terminated only after Terminate()		
	constexpr Text::operator Token() const noexcept {
		return {GetRaw(), mCount};
	}

	/// Destructive text concatenation														
	template<class T>
	Text& Text::operator += (const T& rhs) {
		if constexpr (Sparse<T>)
			return operator += (*rhs);
		else if constexpr (Same<T, Text>) {
			// Concatenate bytes															
			const auto count = rhs.GetCount();
			Block::Allocate(mCount + count, false, false);
			Block::CopyMemory(rhs.mRaw, mRaw, count);
			Block::mCount += count;
			return *this;
		}
		else if constexpr (Convertible<T, Text>) {
			// Finally, attempt converting											
			return operator += (static_cast<Text>(rhs));
		}
		else LANGULUS_ASSERT("Can't concatenate Text - RHS is not convertible to Text");
	}

	/// Concatenate text containers															
	template<class T>
	Text Text::operator + (const T& rhs) const {
		if constexpr (Sparse<T>)
			return operator + (*rhs);
		else if constexpr (Same<T, Text>) {
			// Concatenate bytes															
			Text result = Disown(*this);
			result.mCount += rhs.mCount;
			result.mReserved = result.mCount;
			if (result.mCount) {
				result.mEntry = Allocator::Allocate(result.mType, result.mCount);
				result.mRaw = result.mEntry->GetBlockStart();
			}
			else {
				result.mEntry = nullptr;
				result.mRaw = nullptr;
			}

			CopyMemory(mRaw, result.mRaw, mCount);
			CopyMemory(rhs.mRaw, result.mRaw + mCount, rhs.mCount);
			return Abandon(result);
		}
		else if constexpr (Convertible<T, Text>) {
			// Attempt converting														
			return operator + (static_cast<Text>(rhs));
		}
		else LANGULUS_ASSERT("Can't concatenate Text - RHS is not convertible to Text");
	}

	/// Concatenate anything with text														
	template<class T>
	NOD() Text operator + (const T& lhs, const Text& rhs) requires NotSame<T, Text> {
		if constexpr (Sparse<T>)
			return operator + (*lhs, rhs);
		else if constexpr (Convertible<T, Text>) {
			auto result = static_cast<Text>(lhs);
			result += rhs;
			return result;
		}
		else LANGULUS_ASSERT("Can't concatenate Text - LHS is not convertible to Text");
	}

} // namespace Langulus::Anyness
