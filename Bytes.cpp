#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Default construction																	
	Bytes::Bytes()
		: Block {DataState::Typed, MetaData::Of<Byte>()} { }

	/// Construct via shallow copy															
	///	@param other - the bytes to shallow-copy										
	Bytes::Bytes(const Bytes& other)
		: Block {other} {
		Block::Keep();
	}

	/// Construct via disowned copy															
	///	@param other - the bytes to move													
	Bytes::Bytes(const Disowned<Bytes>& other) noexcept
		: Block {other.Value} { }
	
	/// Construct via abandoned move															
	///	@param other - the bytes to move													
	Bytes::Bytes(Abandoned<Bytes>&& other) noexcept
		: Block {other.Forward<Block>()} { }

	/// Construct manually																		
	///	@param raw - raw memory to reference											
	///	@param count - number of bytes inside 'raw'									
	Bytes::Bytes(const Byte* raw, const Count& count)
		: Block {DataState::Constrained, MetaData::Of<Byte>(), count, raw} {
		// Data is not owned by us, it may be on the stack						
		// We should monopolize the memory to avoid segfaults, in the		
		// case of the byte container being initialized with					
		// temporary data on the stack												
		Block::TakeAuthority();
	}

	/// Destructor																					
	Bytes::~Bytes() {
		// A byte container needs no destructor calls, so just dereference
		Block::Dereference<false>(1);
	}

	/// Clear the contents, but do not deallocate memory if possible				
	void Bytes::Clear() noexcept {
		if (GetReferences() == 1)
			mCount = 0;
		else Reset();
	}

	/// Reset the contents, deallocating any memory										
	/// Byte containers are always type-constrained, and retain that				
	void Bytes::Reset() {
		Block::Dereference<false>(1);
		Block::ResetMemory();
		Block::ResetState<true>();
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
		// First we keep, in order to make sure data is not freed			
		// before being dereferenced													
		other.Keep();
		Block::Dereference<false>(1);
		Block::operator=(other);
		return *this;
	}

	/// Move byte container																		
	///	@param other - the container to move											
	///	@return a reference to this container											
	Bytes& Bytes::operator = (Bytes&& other) {
		Block::Dereference<false>(1);
		Block::operator=(other);
		other.ResetState<true>();
		return *this;
	}

	/// Compare with another byte container												
	bool Bytes::operator == (const Bytes& other) const noexcept {
		return other.mCount == mCount && (
			mRaw == other.mRaw || 
			pcMatchBytes(GetRaw(), mCount, other.GetRaw(), other.mCount) == other.mCount
		);
	}

	/// Compare with another byte container												
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
		return GetRaw()[i];
	}

	/// Access specific character (unsafe)													
	///	@param i - index of character														
	///	@return constant reference to the character									
	Byte& Bytes::operator[] (const Offset& i) {
		return GetRaw()[i];
	}

	/// Clone the byte container																
	///	@return the cloned byte container												
	Bytes Bytes::Clone() const {
		Bytes result {Disown(*this)};
		if (mCount) {
			result.mEntry = Allocator::Allocate(mType, mCount);
			result.mRaw = result.mEntry->GetBlockStart();
		}
		else {
			result.mEntry = nullptr;
			result.mRaw = nullptr;
		}
		
		result.mCount = result.mReserved = mCount;
		pcCopyMemory(mRaw, result.mRaw, mReserved);
		return Abandon(result);
	}

	/// Pick a part of the byte array														
	///	@param start - the starting byte offset										
	///	@param count - the number of bytes after 'start' to remain				
	///	@return a new container that references the original memory				
	Bytes Bytes::Crop(const Offset& start, const Count& count) const {
		Block::CheckRange(start, cound);
		Bytes result {*this};
		result.MakeStatic();
		result.mRaw += start;
		result.mCount = result.mReserved = count;
		return Abandon(result);
	}

	/// Remove a region of bytes																
	/// Can't remove bytes from static containers 										
	///	@param start - the starting offset												
	///	@param end - the ending offset													
	///	@return a reference to the byte container										
	Bytes& Bytes::Remove(const Offset& start, const Offset& end) {
		if (IsEmpty() || IsStatic() || start >= end)
			return *this;
		
		const auto removed = end - start;
		if (end < mCount) {
			// Removing in the middle, so memory has to move					
			pcMoveMemory(mRaw + end, mRaw + start, mCount - removed);
		}

		mCount -= removed;
		return *this;
	}

	/// Extend the byte sequence, change count, and return the new range			
	/// Static byte containers can't be extended											
	///	@param count - the number of bytes to append									
	///	@return the extended part - you will not be allowed to resize it		
	Bytes Bytes::Extend(const Count& count) {
		if (IsStatic())
			// You can not extend static containers								
			return {};
		
		const auto newCount = mCount + count;
		const auto oldCount = mCount;
		if (newCount <= mReserved) {
			// There is enough available space										
			mCount += count;
			
			Bytes result {*this};
			result.MakeStatic();
			result.mRaw += oldCount;
			result.mCount = result.mReserved = count;
			return Abandon(result);
		}

		// Allocate more space															
		mEntry = Allocator::Reallocate(mType, newCount, mEntry);
		mRaw = mEntry->GetBlockStart();
		mCount = mReserved = newCount;
		
		Bytes result {*this};
		result.MakeStatic();
		result.mRaw += oldCount;
		result.mCount = result.mReserved = count;
		return Abandon(result);
	}

} // namespace Langulus::Anyness
