#include "Text.hpp"
#include <cctype>
#include <locale>

namespace Langulus::Anyness
{

	/// Shallow-copy construction (const)													
	///	@param other - the text to shallow-copy										
	Text::Text(const Text& other)
		: TAny {other} { }

	/// Shallow-copy construction																
	///	@param other - the text to shallow-copy										
	Text::Text(Text& other)
		: TAny {other} { }

	/// Copy other but do not reference it, because it is disowned					
	///	@param other - the block to copy													
	Text::Text(const Disowned<Text>& other) noexcept
		: TAny {other.Forward<TAny>()} { }	
	
	/// Move other, but do not bother cleaning it up, because it is disowned	
	///	@param other - the block to move													
	Text::Text(Abandoned<Text>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }	
	
	/// Construct from an exception															
	///	@param from - the exception to stringify										
	Text::Text(const Exception& from) {
		(*this) += from.GetName();
		(*this) += "(";
		(*this) += from.what();
		(*this) += ")";
	}

	/// Construct from an index																
	///	@param from - the index to stringify											
	Text::Text(const Index& from) {
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

	/// Shallow copy assignment																
	///	@param rhs - the text container to copy										
	///	@return a reference to this container											
	Text& Text::operator = (const Text& rhs) {
		TAny::operator = (rhs);
		return *this;
	}

	/// Move text container																		
	///	@param rhs - the text container to move										
	///	@return a reference to this container											
	Text& Text::operator = (Text&& rhs) noexcept {
		TAny::operator = (Forward<TAny>(rhs));
		return *this;
	}

	/// Compare with another text container												
	///	@param other - the text container to compare with							
	///	@return true if both containers are identical								
	bool Text::operator == (const Text& other) const noexcept {
		return Compare(other);
	}

	/// Compare with another text container												
	///	@param other - the text container to compare with							
	///	@return true if both containers are not identical							
	bool Text::operator != (const Text& other) const noexcept {
		return !(*this == other);
	}

	/// Compare with another string															
	///	@param other - text to compare with												
	///	@return true if both containers match completely							
	bool Text::Compare(const Text& other) const noexcept {
		if (mRaw == other.mRaw)
			return mCount == other.mCount;
		else if (mCount != other.mCount)
			return false;

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (t1 < GetRawEnd() && *t1 == *t2) {
			++t1;
			++t2;
		}

		return (t1 - GetRaw()) == mCount;
	}

	/// Compare loosely with another, ignoring upper-case								
	///	@param other - text to compare with												
	///	@return true if both containers match loosely								
	bool Text::CompareLoose(const Text& other) const noexcept {
		if (mRaw == other.mRaw)
			return mCount == other.mCount;
		else if (mCount != other.mCount)
			return false;

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (t1 < GetRawEnd() && (*t1 == *t2 || (::std::isalpha(*t1) && ::std::isalpha(*t2) && (*t1 + 32 == *t2 || *t1 == *t2 + 32)))) {
			++t1;
			++t2;
		}

		return (t1 - GetRaw()) == mCount;
	}

	/// Count how many consecutive letters match in two strings						
	///	@param other - text to compare with												
	///	@return the number of matching symbols											
	Count Text::Matches(const Text& other) const noexcept {
		if (mRaw == other.mRaw)
			return ::std::min(mCount, other.mCount);

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (t1 < GetRawEnd() && *t1 == *t2) {
			++t1;
			++t2;
		}

		/*
		__m128i first = _mm_loadu_si128( reinterpret_cast<__m128i*>( &arr1 ) );
		__m128i second = _mm_loadu_si128( reinterpret_cast<__m128i*>( &arr2 ) );
		return std::popcount(_mm_movemask_epi8( _mm_cmpeq_epi8( first, second ) ));
		*/

		return t1 - GetRaw();
	}

	/// Compare loosely with another, ignoring upper-case								
	/// Count how many consecutive letters match in two strings						
	///	@param other - text to compare with												
	///	@return the number of matching symbols											
	Count Text::MatchesLoose(const Text& other) const noexcept {
		if (mRaw == other.mRaw)
			return ::std::min(mCount, other.mCount);

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (t1 < GetRawEnd() && (*t1 == *t2 || (::std::isalpha(*t1) && ::std::isalpha(*t2) && (*t1 + 32 == *t2 || *t1 == *t2 + 32)))) {
			++t1;
			++t2;
		}

		return t1 - GetRaw();
	}

	#if LANGULUS_FEATURE(UTFCPP)
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
	#endif

	/// Clone the text container																
	///	@return a new container that owns its memory									
	Text Text::Clone() const {
		Text result {Disown(*this)};
		if (mCount) {
			result.mReserved = mCount;
			result.mEntry = Allocator::Allocate(mCount);
			result.mRaw = result.mEntry->GetBlockStart();
			CopyMemory(mRaw, result.mRaw, mCount);
		}
		else {
			result.mEntry = nullptr;
			result.mRaw = nullptr;
			result.mReserved = 0;
		}

		return Abandon(result);
	}

	/// Terminate text so that it ends with a zero character at the end			
	///	@return a new container that ownes its memory								
	Text Text::Terminate() const {
		if (mReserved > mCount && GetRaw()[mCount] == '\0')
			return *this;

		Text result {Disown(*this)};
		++result.mReserved;
		result.mEntry = Allocator::Allocate(result.mReserved);
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
			i = static_cast<Letter>(std::tolower(i));
		return result;
	}

	/// Make all letters uppercase															
	///	@return a new text container with all letter made uppercase				
	Text Text::Uppercase() const {
		Text result = Clone();
		for (auto& i : result)
			i = static_cast<Letter>(std::toupper(i));
		return result;
	}

	/// Find a substring and set 'offset' to its location								
	///	@param pattern - the pattern to search for									
	///	@param offset - [in/out] offset to set if found								
	///	@return true if pattern was found												
	bool Text::FindOffset(const Text& pattern, Offset& offset) const {
		if (pattern.IsEmpty() || offset >= mCount || pattern.mCount > mCount - offset)
			return false;

		const auto end = mCount - pattern.mCount;
		for (Offset i = offset; i < end; ++i) {
			auto remaining = Block::CropInner(i, pattern.mCount, pattern.mCount);
			auto& asText = ReinterpretCast<Text&>(remaining);
			if (asText.Compare(pattern)) {
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
	bool Text::FindOffsetReverse(const Text& pattern, Offset& offset) const {
		if (pattern.IsEmpty() || offset >= mCount || pattern.mCount > mCount - offset)
			return false;

		const auto start = mCount - pattern.mCount - offset;
		for (auto i = ::std::ptrdiff_t(start); i >= 0; --i) {
			auto remaining = Block::CropInner(i, pattern.mCount, pattern.mCount);
			auto& asText = ReinterpretCast<Text&>(remaining);
			if (asText.Compare(pattern)) {
				offset = Offset(i);
				return true;
			}
		}

		return false;
	}

	/// Find a pattern																			
	///	@param pattern - the pattern to search for									
	///	@return true on first match														
	bool Text::Find(const Text& pattern) const {
		[[maybe_unused]] Offset unused;
		return FindOffset(pattern, unused);
	}

	/// Find a match using wildcards in a pattern										
	///	@param pattern - the pattern with the wildcards								
	///	@return true on first match														
	bool Text::FindWild(const Text& pattern) const {
		if (pattern.IsEmpty() || pattern.mCount > mCount)
			return false;

		Offset offset {};
		for (Offset i = 0; i < pattern.mCount; ++i) {
			if (pattern[i] == '*')
				continue;

			// Get every substring between *s										
			Offset accum {};
			while (i + accum < pattern.mCount && pattern[i + accum] != '*')
				++accum;

			if (accum > 0 && !FindOffset(pattern.Crop(i, accum), offset))
				// Mismatch																	
				return false;

			offset += accum;
			i += accum;
		}

		// Success																			
		return true;
	}

	/// Pick a part of the text (const)														
	///	@param start - offset of the starting character								
	///	@param count - the number of characters after 'start'						
	///	@return new text that references the original memory						
	Text Text::Crop(const Count& start, const Count& count) const {
		return TAny::Crop<Text>(start, count);
	}

	/// Pick a part of the text																
	///	@param start - offset of the starting character								
	///	@param count - the number of characters after 'start'						
	///	@return new text that references the original memory						
	Text Text::Crop(const Count& start, const Count& count) {
		return TAny::Crop<Text>(start, count);
	}

	/// Remove all instances of a symbol from the text container					
	///	@param symbol - the character to remove										
	///	@return a new container with the text stripped								
	Text Text::Strip(Letter symbol) const {
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

	/// Remove a part of the text																
	///	@attention assumes end >= start													
	///	@param start - the starting character											
	///	@param end - the ending character												
	///	@return a reference to this text													
	Text& Text::Remove(const Count& start, const Count& end) {
		const auto removed = ::std::min(end, mCount) - ::std::min(start, mCount);
		if (0 == mCount || 0 == removed)
			return *this;

		if (IsConstant())
			throw Except::Destruct("Can't remove from constant text container");

		if (end < mCount)
			Block::MoveMemory(mRaw + end, mRaw + start, mCount - end);

		mCount -= removed;
		return *this;
	}

	/// Extend the text container and return a referenced part of it				
	///	@return an array that represents the extended part							
	Text Text::Extend(const Count& count) {
		return TAny::Extend<Text>(count);
	}

} // namespace Langulus::Anyness
