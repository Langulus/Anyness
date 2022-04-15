#include "Text.hpp"
#include "TAny.hpp"
#include <cctype>
#include <locale>

namespace Langulus::Anyness
{

	/// Default construction																	
	Text::Text()
		: Block {DataState::Typed, MetaData::Of<Letter>()} { }

	/// Shallow-copy construction																
	///	@param other - the text to shallow-copy										
	Text::Text(const Text& other)
		: Block {other} {
		Block::MakeConstant();
		Block::Keep();
	}

	/// Construct from an exception															
	///	@param from - the exception to stringify										
	Text::Text(const Exception& from)
		: Text { } {
		(*this) += from.GetName();
		(*this) += "(";
		(*this) += from.what();
		(*this) += ")";
	}

	/// Construct from an index																
	///	@param from - the index to stringify											
	Text::Text(const Index& from)
		: Text { } {
		// Convert an index to text													
		if (!from.IsArithmetic()) {
			(*this) += Index::Names[from.mIndex - Index::MinIndex];
			return;
		}

		(*this) += from.mIndex;
		(*this) += "i";
	}

	/// Stringify meta																			
	///	@param meta - the definition to stringify										
	Text::Text(const Meta& meta)
		: Text {meta.mToken} {}

	/// Count the number of newline characters											
	///	@return the number of newline characters + 1, or zero if empty			
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

	/// Shallow copy																				
	///	@param rhs - the text container to copy										
	///	@return a reference to this container											
	Text& Text::operator = (const Text& rhs) {
		rhs.Keep();
		Block::Dereference<false>(1);
		Block::operator = (rhs);
		return *this;
	}

	/// Move text container																		
	///	@param rhs - the text container to move										
	///	@return a reference to this container											
	Text& Text::operator = (Text&& rhs) noexcept {
		Block::Dereference<false>(1);
		Block::operator = (rhs);
		rhs.ResetState<true>();
		return *this;
	}

	/// Compare with another text container												
	bool Text::operator == (const Text& other) const noexcept {
		return other.mCount == mCount && (
			mRaw == other.mRaw || 
			pcStrMatches(GetRaw(), mCount, other.GetRaw(), other.mCount) == other.mCount
		);
	}

	/// Compare with another text container												
	bool Text::operator != (const Text& other) const noexcept {
		return !(*this == other);
	}

	/// Compare loosely with another, ignoring upper-case								
	///	@param other - text to compare with												
	///	@return true if both containers match loosely								
	bool Text::CompareLoose(const Text& other) const noexcept {
		return other.mCount == mCount && (
			mRaw == other.mRaw || 
			pcStrMatchesLoose(GetRaw(), mCount, other.GetRaw(), other.mCount) == other.mCount
		);
	}

	/// Compare with another																	
	///	@param other - text to compare with												
	///	@return the number of matching symbols											
	Count Text::Matches(const Text& other) const noexcept {
		return pcStrMatches(GetRaw(), mCount, other.GetRaw(), other.mCount);
	}

	/// Compare loosely with another, ignoring upper-case								
	///	@param other - text to compare with												
	///	@return the number of matching symbols											
	Count Text::MatchesLoose(const Text& other) const noexcept {
		return pcStrMatchesLoose(GetRaw(), mCount, other.GetRaw(), other.mCount);
	}

	/// Access specific character (unsafe)													
	///	@param i - index of character														
	///	@return constant reference to the character									
	const char8_t& Text::operator[] (const Count i) const {
		SAFETY(if (i >= mCount)
			throw Except::Access("Text access index is out of range"));
		return GetRaw()[i];
	}

	/// Access specific character (unsafe)													
	///	@param i - index of character														
	///	@return constant reference to the character									
	char8_t& Text::operator[] (const Count i) {
		SAFETY(if (i >= mCount)
			throw Except::Access("Text access index is out of range"));
		return GetRaw()[i];
	}

	/// Widen the text container to the utf16												
	///	@return the widened text container												
	TAny<char16_t> Text::Widen16() const {
		if (IsEmpty())
			return {};

		TAny<char16_t> to;
		to.Allocate(mCount);
		Count newCount = 0;
		try {
			newCount = utf8::utf8to16(begin(), end(), to.begin()) - to.begin();
		}
		catch (utf8::exception&) {
			throw Except::Convert("utf8 -> utf16 conversion error");
		}

		return to.Trim(newCount);
	}

	/// Widen the text container to the utf32												
	///	@return the widened text container												
	TAny<char32_t> Text::Widen32() const {
		if (IsEmpty())
			return {};

		TAny<char32_t> to;
		to.Allocate(mCount);
		Count newCount = 0;
		try {
			newCount = utf8::utf8to32(begin(), end(), to.begin()) - to.begin();
		}
		catch (utf8::exception&) {
			throw Except::Convert("utf8 -> utf16 conversion error");
		}

		return to.Trim(newCount);
	}

	/// Clone the text container																
	///	@return a new container that owns its memory									
	Text Text::Clone() const {
		Text result {Disown(*this)};
		if (mCount) {
			result.mEntry = Allocator::Allocate(mType, mCount);
			result.mRaw = result.mEntry->GetBlockStart();
		}
		else {
			result.mEntry = nullptr;
			result.mRaw = nullptr;
		}

		result.mCount = result.mReserved = mCount;
		CopyMemory(mRaw, result.mRaw, mCount);
		return Abandon(result);
	}

	/// Terminate text so that it ends with a zero character at the end			
	///	@return a new container that ownes its memory								
	Text Text::Terminate() const {
		if (mReserved > mCount && GetRaw()[mCount] == '\0')
			return *this;

		Text result {Disown(*this)};
		++result.mReserved;
		result.mEntry = Allocator::Allocate(mType, result.mReserved);
		result.mRaw = result.mEntry->GetBlockStart();
		CopyMemory(mRaw, result.mRaw, mCount);
		result.GetRaw()[mCount] = '\0';
		return Abandon(result);
	}

	/// Make all letters lowercase															
	///	@return a new text container with all letter made lowercase				
	Text Text::Lowercase() const {
		Text result = Clone();
		for (auto& i : result)
			i = static_cast<char8_t>(std::tolower(i));
		return result;
	}

	/// Make all letters uppercase															
	///	@return a new text container with all letter made uppercase				
	Text Text::Uppercase() const {
		Text result = Clone();
		for (auto& i : result)
			i = static_cast<char8_t>(std::toupper(i));
		return result;
	}

	/// Find a substring and set offset to its location								
	///	@param pattern - the pattern to search for									
	///	@param offset - [out] offset to set if found									
	///	@return true if pattern was found												
	bool Text::FindOffset(const Text& pattern, Count& offset) const {
		for (Count i = offset; i < mCount; ++i) {
			if (pcStrMatches(GetRaw() + i, mCount - i, pattern.GetRaw(), pattern.mCount) == pattern.mCount) {
				offset = i;
				return true;
			}
		}
		return false;
	}

	/// Find a substring in reverse, and set offset to its location				
	///	@param pattern - the pattern to search for									
	///	@param offset - [out] offset to set if found									
	///	@return true if pattern was found												
	bool Text::FindOffsetReverse(const Text& pattern, Count& offset) const {
		if (pattern.mCount >= mCount)
			return pcStrMatches(GetRaw(), mCount, pattern.GetRaw(), pattern.mCount) == pattern.mCount;

		for (int i = int(mCount) - int(pattern.mCount) - int(offset); i >= 0 && i < int(mCount); --i) {
			if (pcStrMatches(GetRaw() + i, mCount - i, pattern.GetRaw(), pattern.mCount) == pattern.mCount) {
				offset = Count(i);
				return true;
			}
		}

		return false;
	}

	/// Find a pattern																			
	///	@param pattern - the pattern to search for									
	///	@return true on first match														
	bool Text::Find(const Text& pattern) const {
		for (Count i = 0; i < mCount; ++i) {
			if (pcStrMatches(GetRaw() + i, mCount - i, pattern.GetRaw(), pattern.mCount) == pattern.mCount)
				return true;
		}

		return false;
	}

	/// Find a match using wildcards in a pattern										
	///	@param pattern - the pattern with the wildcards								
	///	@return true on first match														
	bool Text::FindWild(const Text& pattern) const {
		Count scan_offset {};
		for (Count i = 0; i < pattern.mCount; ++i) {
			if (pattern[i] == '*')
				continue;

			// Get every substring between *s										
			Count accum {};
			while (i + accum < pattern.mCount && pattern[i + accum] != '*')
				++accum;

			if (accum > 0 && !FindOffset(pattern.Crop(i, accum), scan_offset))
				// Mismatch																	
				return false;

			scan_offset += accum;
			i += accum;
		}

		// Success																			
		return true;
	}

	/// Pick a part of the text - doesn't copy or monopolize data					
	///	@param start - offset of the starting character								
	///	@param count - the number of characters after 'start'						
	///	@return a new container that references the original memory				
	Text Text::Crop(const Count& start, const Count& count) const {
		Text result;
		static_cast<Block&>(result) = Block::Crop(start, count);
		result.Keep();
		return result;
	}

	/// Remove all instances of a symbol from the text container					
	///	@param symbol - the character to remove										
	///	@return a new container with the text stripped								
	Text Text::Strip(char8_t symbol) const {
		Text result;
		Count start {}, end {};
		for (Count i = 0; i <= mCount; ++i) {
			if (i == mCount || (*this)[i] == symbol) {
				const auto size = end - start;
				if (size) {
					auto segment = result.Extend(size);
					CopyMemory(GetRaw() + start, segment.GetRaw(), size);
				}

				start = end = i + 1;
			}
			else ++end;
		}

		return result;
	}

	/// Remove a part of the text. If memory is out of jurisdiction, we're		
	/// monopolizing it in a new allocation												
	///	@param start - the starting character											
	///	@param end - the ending character												
	///	@return a reference to this text													
	Text& Text::Remove(const Count& start, const Count& end) {
		const auto removed = end - start;
		if (0 == mCount || 0 == removed)
			return *this;

		if (end < mCount) {
			TakeAuthority();
			Block::MoveMemory(mRaw + end, mRaw + start, mCount - removed);
		}

		mCount -= removed;
		return *this;
	}

	/// Extend the string, change count, and if data is out of jurisdiction -	
	/// move it to a new place where we own it											
	///	@return an array that represents the extended part							
	Text Text::Extend(const Count& count) {
		if (IsStatic())
			// You can not extend static containers								
			return {};

		const auto newCount = mCount + count;
		const auto oldCount = mCount;
		if (newCount <= mReserved) {
			// There is enough available space										
			mCount += count;
			Text result {*this};
			result.MakeStatic();
			result.mRaw += oldCount;
			result.mCount = result.mReserved = count;
			return Abandon(result);
		}

		// Allocate more space															
		mEntry = Allocator::Reallocate(mType, newCount, mEntry);
		mRaw = mEntry->GetBlockStart();
		mCount = mReserved = newCount;

		Text result {*this};
		result.MakeStatic();
		result.mRaw += oldCount;
		result.mCount = result.mReserved = count;
		return Abandon(result);
	}

	/// Clone text array into a new owned memory block									
	/// If we have jurisdiction, the memory won't move									
	void Text::TakeAuthority() {
		if (mEntry)
			return;

		operator = (Clone());
	}


} // namespace Langulus::Anyness
