#pragma once
#include "Text.hpp"

namespace Langulus::Anyness
{

	/// Construct from literal																	
	///	@param text - the text to contain												
	inline Text::Text(const Token& text)
		: Text {text.begin(), text.size()} {}

	/// Construct from standard array														
	///	@param text - the text to contain												
	template<pcptr S>
	Text::Text(const std::array<char, S>& text)
		: Text {text.data(), text.size() - (text[S - 1] ? 0 : 1)} {
		static_assert(S != 0, "Can't initialize with an empty array");
	}

	/// Convert a named type to text															
	///	@param from - the named type and value											
	template<LiterallyNamed T>
	Text::Text(const T& from) requires Dense<T> : Text {} {
		// Convert a named type															
		T temp {from};
		Text result;
		do {
			if (!result.IsEmpty())
				result += "+";
			result += temp.GetLiteralName();
		}
		while (static_cast<bool>(temp));

		(*this) = result;
	}

	/// Convert a number type to text														
	///	@param from - the number to stringify											
	template<Number T>
	Text::Text(const T& from) requires Dense<T> : Text{} {
		// Stringify a number															
		// The code resides here, because we can't simply override			
		// built-in cast operators														
		if constexpr (RealNumber<T>) {
			// Stringify a real number													
			constexpr auto size = std::numeric_limits<T>::max_digits10 * 2;
			char temp[size];
			auto [lastChar, errorCode] = std::to_chars(
				temp, temp + size, from, std::chars_format::general);
			if (errorCode != std::errc())
				throw Except::Convert("std::to_chars failure");

			while ((*lastChar == '0' || *lastChar == '.') && lastChar > temp) {
				if (*lastChar == '.')
					break;
				--lastChar;
			}

			const auto copied = pcP2N(lastChar) - pcP2N(temp);
			(*this) = Text(temp, copied);
		}
		else if constexpr (IntegerNumber<T>) {
			// Stringify an integer														
			constexpr auto size = std::numeric_limits<T>::digits10 * 2;
			char temp[size];
			auto [lastChar, errorCode] = std::to_chars(
				temp, temp + size, from);
			if (errorCode != std::errc())
				throw Except::Convert("std::to_chars failure");

			const auto copied = pcP2N(lastChar) - pcP2N(temp);
			(*this) += Text(temp, copied);
		}
		else if constexpr (CustomNumber<T>) {
			// Stringify a custom number												
			(*this) += Text(from.GetBuiltinNumber());
		}
		else LANGULUS_ASSERT("Unsupported number type");
	}

	/// Construct from null-terminated string												
	///	@param text - text memory to reference											
	template<>
	inline Text::Text(const char* text)
		: Text {text, pcStrSize(text)} {}

	template<>
	inline Text::Text(const char8* text)
		: Text {reinterpret_cast<const char*>(text), pcStrSize(text)} {}

	/// Construct from null-terminated wide string										
	///	@param text - text memory to convert											
	template<>
	inline Text::Text(const wchar_t* text)
		: Text {text, pcStrSize(text)} {}

	template<>
	inline Text::Text(const charw* text)
		: Text {reinterpret_cast<const wchar_t*>(text), pcStrSize(text)} {}

	/// Convert any pointer to text															
	///	@param from - the pointer to dereference and stringify					
	template<class T>
	Text::Text(const T* from) requires Dense<T> : Text {} {
		if (!from)
			(*this) += "null";
		else
			(*this) += *from;
	}

	/// Raw character access (unsafe)														
	///	@attention the string is null-terminated only after Terminate()		
	///	@return the pointer to the first character inside text container		
	constexpr char* Text::GetRaw() noexcept {
		return static_cast<char*>(mRaw);
	}

	/// Raw constant character access (unsafe)											
	///	@attention the string is null-terminated only after Terminate()		
	///	@return the pointer to the first character inside text container		
	constexpr const char* Text::GetRaw() const noexcept {
		return static_cast<const char*>(mRaw);
	}

	/// Interpret text container as a literal												
	///	@attention the string is null-terminated only after Terminate()		
	constexpr Text::operator Token() const noexcept {
		return { GetRaw(), mCount };
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


	///																								
	///	TEXT SELECTIONS																		
	///																								
	/// Selection constructor																	
	inline Text::Selection::Selection(Text* text, pcptr start, pcptr end) noexcept
		: mText(text), mStart(start), mEnd(end) {}

	inline const Text& Text::Selection::GetText() const noexcept {
		return *mText;
	}

	inline pcptr Text::Selection::GetStart() const noexcept {
		return mStart;
	}

	inline pcptr Text::Selection::GetEnd() const noexcept {
		return mEnd;
	}

	inline pcptr Text::Selection::GetLength() const noexcept {
		return mEnd - mStart;
	}

	inline const char& Text::Selection::operator[] (const pcptr i) const noexcept {
		return (*mText)[mStart + i];
	}

	inline char& Text::Selection::operator[] (const pcptr i) noexcept {
		return (*mText)[mStart + i];
	}

	/// String concatenation																	
	/// Appends to the back of the selection												
	inline Text::Selection& Text::Selection::operator << (const Text& other) {
		if (!mText)
			return *this;

		mText->Extend(other.mCount);
		if (mEnd < mText->mCount)
			pcMoveMemory(mText->GetRaw() + mEnd, mText->GetRaw() + mEnd + other.mCount, mText->mCount - mEnd - other.mCount);
		pcCopyMemory(other.GetRaw(), mText->GetRaw() + mEnd, other.mCount);
		return *this;
	}

	/// String concatenation with something that is not Text							
	/// Appends to the back of the selection												
	template<class K>
	Text::Selection& Text::Selection::operator << (const K& other) requires (!IsText<K>) {
		if (!mText)
			return *this;

		if constexpr (Character<K> && Dense<K>)
			return operator << (Text(&other, 1));
		else if constexpr (ConstructibleWith<Text, K>)
			return operator << (Text(other));
		else {
			// Finally, attempt converting											
			Text converted;
			if (TConverter<K, Text>::Convert(other, converted) > 0)
				return operator << (converted);
			return *this;
		}
	}

	/// String concatenation																	
	/// Appends to the front of the selection												
	inline Text::Selection& Text::Selection::operator >> (const Text& other) {
		if (!mText)
			return *this;

		mText->Extend(other.mCount);
		pcMoveMemory(mText->GetRaw() + mStart, mText->GetRaw() + mStart + other.mCount, mText->mCount - mStart - other.mCount);
		pcCopyMemory(other.GetRaw(), mText->GetRaw() + mStart, other.mCount);

		// Update selection																
		mStart += other.mCount;
		mEnd += other.mCount;
		return *this;
	}

	/// String concatenation																	
	/// Appends to the front of the selection												
	template<class K>
	Text::Selection& Text::Selection::operator >> (const K& other) requires (!IsText<K>) {
		if (!mText)
			return *this;

		if constexpr (Character<K> && Dense<K>)
			return operator >> (Text(&other, 1));
		else if constexpr (ConstructibleWith<Text, K>)
			return operator >> (Text(other));
		else {
			// Finally, attempt converting											
			Text converted;
			if (TConverter<K, Text>::Convert(other, converted) > 0)
				return operator >> (converted);
			return *this;
		}
	}

	/// Replace selection (selection will collapse)										
	/// Collapsed selection means mStart == mEnd											
	inline Text::Selection& Text::Selection::Replace(const Text& other) {
		if (!mText)
			return *this;

		if (other.mCount > mEnd - mStart) {
			// Replacement is bigger, we need more space							
			const auto surplus = (other.mCount + mStart) - mEnd;
			mText->Extend(surplus);
			// Move memory from the required space									
			pcMoveMemory(mText->GetRaw() + mStart, mText->GetRaw() + mStart + surplus, mText->mCount - mStart - surplus);
		}
		else if (other.mCount < mEnd - mStart) {
			// Replacement is smaller, move data backwards						
			pcMoveMemory(mText->GetRaw() + mEnd, mText->GetRaw() + mStart + other.mCount, mText->mCount - mEnd);
			mText->mCount -= (mEnd - mStart) - other.mCount;
			mText->Allocate(mText->mCount);
		}

		// Copy new data																	
		pcCopyMemory(other.GetRaw(), mText->GetRaw() + mStart, other.mCount);

		// Move marker and collapse selection										
		mStart += other.mCount;
		mEnd = mStart;
		return *this;
	}

	/// Replace selection (selection will collapse)										
	/// Collapsed selection means mStart == mEnd											
	template<class K>
	Text::Selection& Text::Selection::Replace(const K& other) requires (!IsText<K>) {
		if (!mText)
			return *this;

		if constexpr (Character<K> && Dense<K>)
			return Replace(Text(&other, 1));
		else if constexpr (ConstructibleWith<Text, K>)
			return Replace(Text(other));
		else {
			// Finally, attempt converting											
			Text converted;
			if (TConverter<K, Text>::Convert(other, converted) > 0)
				return Replace(converted);
			return *this;
		}
	}

	/// Delete selection (selection will collapse),										
	/// or delete symbol after collapsed selection marker								
	inline Text::Selection& Text::Selection::Delete() {
		if (!mText)
			return *this;

		if (mStart != mEnd) {
			mText->Remove(mStart, mEnd);
			mEnd = mStart;
		}
		else if (mStart < mText->mCount) {
			mText->Remove(mStart, mStart + 1);
		}
		return *this;
	}

	/// Delete selection (with collapse), or delete symbol							
	/// before a collapsed selection marker												
	inline Text::Selection& Text::Selection::Backspace() {
		if (!mText)
			return *this;

		if (mStart != mEnd) {
			mText->Remove(mStart, mEnd);
			mEnd = mStart;
		}
		else if (mStart > 0 && mText->mCount > 0) {
			mText->Remove(mStart, mStart - 1);
			--mStart;
			mEnd = mStart;
		}
		return *this;
	}

} // namespace Langulus::Anyness
