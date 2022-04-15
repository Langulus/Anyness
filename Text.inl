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
	///	@param anyCharacter - the character to stringify							
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
		else LANGULUS_ASSERT("Can't concatenate to Text - RHS is not convertible");
	}

	/// Concatenate byte containers															
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
		else LANGULUS_ASSERT("Can't concatenate to Text - RHS is not convertible");
	}

	/// Concatenate anything with bytes														
	template<class T>
	Text operator + (const T& lhs, const Text& rhs) requires NotSame<T, Text> {
		if constexpr (Sparse<T>)
			return operator + (*lhs, rhs);
		else if constexpr (Convertible<T, Text>) {
			auto result = static_cast<Text>(lhs);
			result += rhs;
			return result;
		}
		else LANGULUS_ASSERT("Can't concatenate to Text - LHS is not convertible");
	}

} // namespace Langulus::Anyness
