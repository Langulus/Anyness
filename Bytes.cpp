#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Default construction																	
	Bytes::Bytes()
		: Block {DataState::Typed, PCMEMORY.GetFallbackMetaByte()} { }

	/// Do a shallow copy																		
	///	@param other - the text to shallow-copy										
	Bytes::Bytes(const Bytes& other)
		: Block {other} {
		MakeConstant();
		PCMEMORY.Reference(mType, mRaw, 1);
	}

	/// Construct manually from byte memory and count									
	///	@param raw - raw memory to reference											
	///	@param count - number of bytes inside 'raw'									
	Bytes::Bytes(const Byte* raw, const Count& count)
		: Block {DataState::Constant + DataState::Typed, PCMEMORY.GetFallbackMetaByte(), count, raw} {
		bool no_jury;
		PCMEMORY.Reference(mType, mRaw, 1, no_jury);
		if (no_jury) {
			// We should monopolize the memory to avoid segfaults, in the	
			// case of the byte container being initialized with temporary	
			// data																			
			Block::TakeAuthority();
		}
	}

	/// Destructor																					
	Bytes::~Bytes() {
		PCMEMORY.Reference(mType, mRaw, -1);
	}

	/// Clear the contents, but do not deallocate memory if possible				
	void Bytes::Clear() noexcept {
		if (GetBlockReferences() == 1)
			mCount = 0;
		else Reset();
	}

	/// Reset the contents, deallocating any memory										
	/// Byte containers are always type-constrained, and retain that				
	void Bytes::Reset() {
		PCMEMORY.Reference(mType, mRaw, -1);
		mRaw = nullptr;
		mCount = mReserved = 0;
		mState = DataState::Typed;
	}

	/// Hash the byte sequence																	
	///	@return a hash of the contained byte sequence								
	Hash Bytes::GetHash() const {
		return ::std::hash<::std::span<Byte>>()({GetRaw(), GetCount()});
	}

	/// Allocate 'count' elements and fill the container with zeroes				
	void Bytes::Null(const Count& count) {
		Allocate(count, false, true);
		pcFillMemory(mRaw, {}, mCount);
	}

	/// Do a shallow copy																		
	///	@param other - the byte container to reference								
	///	@return a reference to this container											
	Bytes& Bytes::operator = (const Bytes& other) {
		PCMEMORY.Reference(mType, mRaw, -1);
		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		PCMEMORY.Reference(mType, mRaw, 1);
		return *this;
	}

	/// Move byte container																		
	///	@param other - the container to move											
	///	@return a reference to this container											
	Bytes& Bytes::operator = (Bytes&& other) {
		PCMEMORY.Reference(mType, mRaw, -1);
		if (other.CheckJurisdiction() && !other.CheckUsage()) {
			//TODO is this relevant? test it
			throw Except::Move(Logger::Error()
				<< "You've hit a really nasty corner case, where trying to move a container destroys it, "
				<< "due to a circular referencing. Try to move a shallow-copy, instead of a reference to "
				<< "the original. Data may be incorrect at this point. ");
		}

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
	bool Bytes::operator == (const Bytes& other) const noexcept {
		return other.mCount == mCount && (
			mRaw == other.mRaw || 
			pcMatchBytes(GetRaw(), mCount, other.GetRaw(), other.mCount) == other.mCount
		);
	}

	/// Compare with another text container												
	bool Bytes::operator != (const Bytes& other) const noexcept {
		return !(*this == other);
	}

	/// Compare with another																	
	///	@param other - text to compare with												
	///	@return the number of matching symbols											
	Count Bytes::Matches(const Bytes& other) const noexcept {
		return pcMatchBytes(GetRaw(), mCount, other.GetRaw(), other.mCount);
	}

	/// Access specific character (const, unsafe)										
	///	@param i - index of character														
	///	@return constant reference to the character									
	const Byte& Bytes::operator[] (const Offset& i) const {
		SAFETY(if (i >= mCount)
			throw Except::Access("Byte access index is out of range"));
		return GetRaw()[i];
	}

	/// Access specific character (unsafe)													
	///	@param i - index of character														
	///	@return constant reference to the character									
	Byte& Bytes::operator[] (const Offset& i) {
		SAFETY(if (i >= mCount)
			throw Except::Access("Byte access index is out of range"));
		return GetRaw()[i];
	}

	/// Clone the byte container																
	///	@return a new container that owns its memory									
	Bytes Bytes::Clone() const {
		Bytes result;
		result.mReserved = mReserved;
		result.mCount = mCount;
		if (mReserved > 0) {
			result.mRaw = PCMEMORY.Allocate(mType, mReserved);
			pcCopyMemory(mRaw, result.mRaw, mReserved);
		}
		return result;
	}

	/// Pick a part of the byte array - doesn't copy or monopolize data			
	///	@param start - the starting byte offset										
	///	@param count - the number of bytes after 'start' to remain				
	///	@return a new container that references the original memory				
	Bytes Bytes::Crop(const Offset& start, const Count& count) const {
		Bytes result;
		static_cast<Block&>(result) = Block::Crop(start, count);
		result.ReferenceBlock(1);
		return result;
	}

	/// Remove a part of the text. If memory is out of jurisdiction, we're		
	/// monopolizing it in a new allocation												
	///	@param start - the starting character											
	///	@param end - the ending character												
	///	@return a reference to this text													
	Bytes& Bytes::Remove(const Offset& start, const Offset& end) {
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

	/// Extend the byte sequence, change count, and if data is out of				
	/// jurisdiction - move it to a new place where we own it						
	///	@return an array that represents the extended part							
	Bytes Bytes::Extend(const Count& count) {
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
