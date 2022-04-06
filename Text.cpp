#include "../include/PCFW.Memory.hpp"
#include <utf8.h>

namespace Langulus::Anyness
{

	/// Default construction																	
	Text::Text()
		: Block {DState::Typed, PCMEMORY.GetFallbackMetaChar(), 0, static_cast<void*>(nullptr)} {}

	/// Do a shallow copy																		
	///	@param other - the text to shallow-copy										
	Text::Text(const Text& other)
		: Block {other} {
		MakeConstant();
		PCMEMORY.Reference(mType, mRaw, 1);
	}

	/// Construct manually from text memory and count									
	///	@param text - text memory to reference											
	///	@param count - number of characters inside text								
	Text::Text(const char* text, pcptr count)
		: Block {DState::Constant + DState::Typed, PCMEMORY.GetFallbackMetaChar(), count, text} {
		bool no_jury;
		PCMEMORY.Reference(mType, mRaw, 1, no_jury);
		if (no_jury) {
			// We should monopolize the memory to avoid segfaults, in the	
			// case of the text container being initialized with temporary	
			// data																			
			TakeJurisdiction();
		}
	}

	/// Construct manually from wide text memory and count							
	///	@param text - wide text memory to reference									
	///	@param count - number of characters inside wide text						
	Text::Text(const wchar_t* text, pcptr count)
		: Text{} {
		try {
			#if WCHAR_MAX > 0xffff
				Allocate(count * 4);
				mCount = pcP2N(utf8::utf32to8(text, text + count, begin())) - pcP2N(begin());
			#elif WCHAR_MAX > 0xff
				Allocate(count * 2);
				mCount = pcP2N(utf8::utf16to8(text, text + count, begin())) - pcP2N(begin());
			#else
				Allocate(count, false, true);
				pcCopyMemory(text, begin(), count);
				return result;
			#endif
		}
		catch (utf8::exception&) {
			throw Except::ConvertText("utfw -> utf8 conversion error");
		}
	}

	/// Construct from a byte, returning a hex string									
	///	@param value - the byte to stringify											
	Text::Text(pcbyte value)
		: Text {pcToHex(static_cast<unsigned char>(value))} {}

	/// Construct from a character															
	///	@param value - the character to stringify										
	Text::Text(const char8& value)
		: Text {&value.mValue, 1} {}

	/// Construct from a wide character														
	///	@param value - the character to stringify										
	Text::Text(const charw& value)
		: Text {&value.mValue, 1} {}

	/// Construct from a data ID																
	///	@param value - the id to stringify												
	Text::Text(const DataID& value)
		: Text {!value ? DataID::DefaultToken : value.GetMeta()->GetToken()} {}

	/// Construct from a trait ID																
	///	@param value - the id to stringify												
	Text::Text(const TraitID& value)
		: Text {!value ? TraitID::DefaultToken : value.GetMeta()->GetToken()} {}

	/// Construct from a const ID																
	///	@param value - the id to stringify												
	Text::Text(const ConstID& value)
		: Text {!value ? ConstID::DefaultToken : value.GetMeta()->GetToken()} {}

	/// Construct from a verb ID																
	///	@param value - the id to stringify												
	Text::Text(const VerbID& value)
		: Text {!value ? VerbID::DefaultToken : value.GetMeta()->GetToken()} {}

	/// Construct from an exception															
	///	@param from - the exception to stringify										
	Text::Text(const AException& from)
		: Text { } {
		(*this) += from.GetExceptionName();
		(*this) += "(";
		(*this) += from.what();
		(*this) += ")";
	}

	/// Construct from an index																
	///	@param from - the index to stringify											
	Text::Text(const Index& from)
		: Text { } {
		// Convert an index																
		if (!from.IsArithmetic()) {
			(*this) += Index::Names[from.mIndex - uiMinIndex];
			return;
		}

		(*this) += from.mIndex;
		(*this) += "i";
	}

	/// Stringify meta																			
	///	@param meta - the definition to stringify										
	Text::Text(const AMeta& meta)
		: Text {meta.GetToken()} {}

	/// Destructor																					
	Text::~Text() {
		PCMEMORY.Reference(mType, mRaw, -1);
	}

	/// Create UTF8 text from an UTF32 codepoint											
	///	@param cp - the codepoint															
	///	@return a text container containing the UTF8 equivalent					
	Text Text::FromCodepoint(pcu32 cp) {
		Text result;
		result.Allocate(sizeof(cp));
		try {
			const auto count = pcP2N(utf8::utf32to8(&cp, &cp + 1, result.begin())) - pcP2N(result.begin());
			result.Trim(count);
		}
		catch (utf8::exception&) {
			throw Except::ConvertText("utf32 -> utf8 conversion error");
		}
		return result;
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
		mState = DState::Typed;
	}

	/// Hash the text																				
	///	@return a hash of the contained text											
	Hash Text::GetHash() const {
		return pcHash(GetRaw(), mCount);
	}

	/// Count the number of newline characters											
	///	@return the number of newline characters + 1, or zero if empty			
	NOD() pcptr Text::GetLineCount() const noexcept {
		if (mCount == 0)
			return 0;

		pcptr lines = 1;
		for (pcptr i = 0; i < mCount; ++i) {
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
	Text& Text::operator = (Text&& other) SAFE_NOEXCEPT() {
		PCMEMORY.Reference(mType, mRaw, -1);
		SAFETY(if (other.CheckJurisdiction() && !other.CheckUsage())
			throw Except::BadMove(pcLogFuncError
				<< "You've hit a really nasty corner case, where trying to move a container destroys it, "
				<< "due to a circular referencing. Try to move a shallow-copy, instead of a reference to "
				<< "the original. Data may be incorrect at this point, but the moved container was: " << LiteralText{ other }));

		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		other.mRaw = nullptr;
		other.mCount = other.mReserved = 0;
		other.mState = DState::Typed;
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
	pcptr Text::Matches(const Text& other) const noexcept {
		return pcStrMatches(GetRaw(), mCount, other.GetRaw(), other.mCount);
	}

	/// Compare loosely with another, ignoring upper-case								
	///	@param other - text to compare with												
	///	@return the number of matching symbols											
	pcptr Text::MatchesLoose(const Text& other) const noexcept {
		return pcStrMatchesLoose(GetRaw(), mCount, other.GetRaw(), other.mCount);
	}

	/// Access specific character (unsafe)													
	///	@param i - index of character														
	///	@return constant reference to the character									
	const char& Text::operator[] (const pcptr i) const {
		SAFETY(if (i >= mCount)
			throw Except::BadAccess("Text access index is out of range"));
		return GetRaw()[i];
	}

	/// Access specific character (unsafe)													
	///	@param i - index of character														
	///	@return constant reference to the character									
	char& Text::operator[] (const pcptr i) {
		SAFETY(if (i >= mCount)
			throw Except::BadAccess("Text access index is out of range"));
		return GetRaw()[i];
	}

	/// Widen the text container to the utf16												
	///	@return the widened text container												
	TAny<pcu16> Text::Widen16() const {
		if (IsEmpty())
			return {};

		TAny<pcu16> to;
		to.Allocate(mCount);
		pcptr newCount = 0;
		try {
			newCount = (pcP2N(utf8::utf8to16(begin(), end(), to.begin())) - pcP2N(to.begin())) / 2;
		}
		catch (utf8::exception&) {
			throw Except::ConvertText("utf8 -> utf16 conversion error");
		}

		return to.Trim(newCount);
	}

	/// Widen the text container to the utf32												
	///	@return the widened text container												
	TAny<pcu32> Text::Widen32() const {
		if (IsEmpty())
			return {};

		TAny<pcu32> to;
		to.Allocate(mCount);
		pcptr newCount = 0;
		try {
			newCount = (pcP2N(utf8::utf8to32(begin(), end(), to.begin())) - pcP2N(to.begin())) / 4;
		}
		catch (utf8::exception&) {
			throw Except::ConvertText("utf8 -> utf16 conversion error");
		}

		return to.Trim(newCount);
	}

	/// Widen the text container to the default wchar_t								
	#if WCHAR_MAX > 0xffff
		TAny<pcu32> Text::Widen() const {
			return Widen32();
		}
	#elif WCHAR_MAX > 0xff
		TAny<pcu16> Text::Widen() const {
			return Widen16();
		}
	#else
		#error "Your compiler doesn't support text widening"
	#endif

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
			i = static_cast<char>(std::tolower(i));
		return result;
	}

	/// Make all letters uppercase															
	///	@return a new text container with all letter made uppercase				
	Text Text::Uppercase() const {
		Text result = Clone();
		for (auto& i : result)
			i = static_cast<char>(std::toupper(i));
		return result;
	}

	/// Find a substring and set offset to its location								
	///	@param pattern - the pattern to search for									
	///	@param offset - [out] offset to set if found									
	///	@return true if pattern was found												
	bool Text::FindOffset(const Text& pattern, pcptr& offset) const {
		for (pcptr i = offset; i < mCount; ++i) {
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
	bool Text::FindOffsetReverse(const Text& pattern, pcptr& offset) const {
		if (pattern.mCount >= mCount)
			return pcStrMatches(GetRaw(), mCount, pattern.GetRaw(), pattern.mCount) == pattern.mCount;

		for (int i = int(mCount) - int(pattern.mCount) - int(offset); i >= 0 && i < int(mCount); --i) {
			if (pcStrMatches(GetRaw() + i, mCount - i, pattern.GetRaw(), pattern.mCount) == pattern.mCount) {
				offset = pcptr(i);
				return true;
			}
		}

		return false;
	}

	/// Find a pattern																			
	///	@param pattern - the pattern to search for									
	///	@return true on first match														
	bool Text::Find(const Text& pattern) const {
		for (pcptr i = 0; i < mCount; ++i) {
			if (pcStrMatches(GetRaw() + i, mCount - i, pattern.GetRaw(), pattern.mCount) == pattern.mCount)
				return true;
		}

		return false;
	}

	/// Find a match using wildcards in a pattern										
	///	@param pattern - the pattern with the wildcards								
	///	@return true on first match														
	bool Text::FindWild(const Text& pattern) const {
		pcptr scan_offset = 0;
		for (pcptr i = 0; i < pattern.mCount; ++i) {
			if (pattern[i] == '*')
				continue;

			// Get every substring between *s										
			pcptr accum = 0;
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
	Text Text::Crop(pcptr start, pcptr count) const {
		Text result;
		static_cast<Block&>(result) = Block::Crop(start, count);
		result.ReferenceBlock(1);
		return result;
	}

	/// Remove all instances of a symbol from the text container					
	///	@param symbol - the character to remove										
	///	@return a new container with the text stripped								
	Text Text::Strip(char symbol) const {
		Text result;
		pcptr start = 0, end = 0;
		for (pcptr i = 0; i <= mCount; ++i) {
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
	Text& Text::Remove(pcptr start, pcptr end) {
		const auto removed = end - start;
		if (0 == mCount || 0 == removed)
			return *this;

		if (end < mCount) {
			TakeJurisdiction();
			pcMoveMemory(GetRaw() + end, GetRaw() + start, mCount - removed);
		}

		mCount -= removed;
		return *this;
	}

	/// Extend the string, change count, and if data is out of jurisdiction -	
	/// move it to a new place where we own it											
	///	@return an array that represents the extended part							
	TArray<char> Text::Extend(pcptr count) {
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

	/// Find a token in the text, and return a selection								
	///	@param pattern - the text to select												
	///	@return the selection																
	Text::Selection Text::Select(const ME& pattern) {
		pcptr offset = 0;
		if (!FindOffset(pattern, offset))
			return {};
		return {this, offset, offset + pattern.mCount};
	}

	/// Select a region of text [start; end)												
	///	@param start - the starting character											
	///	@param end - the ending character												
	///	@return the selection																
	Text::Selection Text::Select(pcptr start, pcptr end) {
		if (start > mCount || end > mCount || end < start)
			return {};
		return {this, start, end};
	}

	/// Move the text marker																	
	///	@param marker - cursor position													
	///	@return the selection																
	Text::Selection Text::Select(pcptr marker) {
		if (marker > mCount)
			return {};
		return {this, marker, marker};
	}

} // namespace Langulus::Anyness