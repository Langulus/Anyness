#include "Text.hpp"
#include "TAny.hpp"
#include <cctype>
#include <locale>

namespace Langulus::Anyness
{

	/// Default construction																	
	Text::Text()
		: Block {DataState::Typed, PCMEMORY.GetFallbackMetaChar(), 0, static_cast<void*>(nullptr)} {}

	/// Do a shallow copy																		
	///	@param other - the text to shallow-copy										
	Text::Text(const Text& other)
		: Block {other} {
		MakeConstant();
		PCMEMORY.Reference(mType, mRaw, 1);
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

	/// Destructor																					
	Text::~Text() {
		PCMEMORY.Reference(mType, mRaw, -1);
	}

	/// Clear the contents, but do not deallocate memory if possible				
	void Text::Clear() noexcept {
		if (GetBlockReferences() == 1)
			mCount = 0;
		else Reset();
	}

	/// Reset the contents, deallocating any memory										
	/// Text containers are always type-constrained, and retain that				
	void Text::Reset() {
		PCMEMORY.Reference(mType, mRaw, -1);
		mRaw = nullptr;
		mCount = mReserved = 0;
		mState = DataState::Typed;
	}

	/// Hash the text																				
	///	@return a hash of the contained text											
	Hash Text::GetHash() const {
		return ::std::hash<::std::u8string_view>()({GetRaw(), GetCount()});
	}

	/// Count the number of newline characters											
	///	@return the number of newline characters + 1, or zero if empty			
	NOD() Count Text::GetLineCount() const noexcept {
		if (IsEmpty())
			return 0;

		Count lines {1};
		for (Count i = 0; i < mCount; ++i) {
			if ((*this)[i] == '\n')
				++lines;
		}

		return lines;
	}

	/// Do a shallow copy																		
	///	@param text - the text container to reference								
	///	@return a reference to this container											
	Text& Text::operator = (const Text& other) {
		PCMEMORY.Reference(mType, mRaw, -1);
		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		PCMEMORY.Reference(mType, mRaw, 1);
		return *this;
	}

	/// Move text container																		
	///	@param text - the text container to move										
	///	@return a reference to this container											
	Text& Text::operator = (Text&& other) noexcept {
		PCMEMORY.Reference(mType, mRaw, -1);
		SAFETY(if (other.CheckJurisdiction() && !other.CheckUsage())
			throw Except::Move(Logger::Error()
				<< "You've hit a really nasty corner case, where trying to move a container destroys it, "
				<< "due to a circular referencing. Try to move a shallow-copy, instead of a reference to "
				<< "the original. Data may be incorrect at this point, but the moved container was: " << Token {other}));

		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		other.mRaw = nullptr;
		other.mCount = other.mReserved = 0;
		other.mState = DataState::Typed;
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
		Text result;
		result.mReserved = mReserved;
		result.mCount = mCount;
		if (mReserved > 0) {
			// Accounted for the terminating character if any					
			result.mRaw = PCMEMORY.Allocate(mType, mReserved);
			pcCopyMemory(mRaw, result.mRaw, mReserved);
		}
		return result;
	}

	/// Terminate text so that it ends with a zero character							
	///	@return a new container that ownes its memory								
	Text Text::Terminate() const {
		if (mReserved > mCount && GetRaw()[mCount] == '\0')
			return *this;

		Text result;
		result.mCount = mCount; // count should never change!!!
		result.mReserved = mCount + 1;
		result.mRaw = PCMEMORY.Allocate(mType, result.mReserved);
		pcCopyMemory(mRaw, result.mRaw, mCount);
		result.GetRaw()[mCount] = '\0';
		return result;
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
	Text Text::Crop(Count start, Count count) const {
		Text result;
		static_cast<Block&>(result) = Block::Crop(start, count);
		result.ReferenceBlock(1);
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
					pcCopyMemory(GetRaw() + start, segment.GetRaw(), size);
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
	Text& Text::Remove(Count start, Count end) {
		const auto removed = end - start;
		if (0 == mCount || 0 == removed)
			return *this;

		if (end < mCount) {
			TakeAuthority();
			pcMoveMemory(GetRaw() + end, GetRaw() + start, mCount - removed);
		}

		mCount -= removed;
		return *this;
	}

	/// Extend the string, change count, and if data is out of jurisdiction -	
	/// move it to a new place where we own it											
	///	@return an array that represents the extended part							
	Text Text::Extend(Count count) {
		const auto lastCount = mCount;
		if (mCount + count <= mReserved) {
			mCount += count;
			return {GetRaw() + lastCount, count};
		}

		if (!mRaw) {
			// If text container is empty - allocate								
			mRaw = PCMEMORY.Allocate(mType, mCount + count);
		}
		else if (!IsStatic()) {
			// If text is not unmovable and already allocated - resize		
			mRaw = PCMEMORY.Reallocate(mType, mRaw, mCount + count, mCount);
		}
		else {
			// In case memory is unmovable - clone and reallocate				
			*this = Clone();
			mRaw = PCMEMORY.Reallocate(mType, mRaw, mCount + count, mCount);
		}

		mCount += count;
		mReserved = mCount;
		return {GetRaw() + lastCount, count};
	}

} // namespace Langulus::Anyness
