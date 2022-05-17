///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Bytes.hpp"
#include "inner/Hashing.hpp"

namespace Langulus::Anyness
{

	/// Hash the byte sequence																	
	///	@return a hash of the contained byte sequence								
	Hash Bytes::GetHash() const {
		return HashBytes(GetRaw(), GetCount());
	}

	/// Allocate a number of bytes and zero them											
	///	@param count - the number of bytes to allocate								
	void Bytes::Null(const Count& count) {
		Allocate<false>(count);
		mCount = count;
		FillMemory(mRaw, {}, mCount);
	}

	/// Compare with another byte container												
	///	@param other - the byte container to compare with							
	///	@return true if both containers are identical								
	bool Bytes::operator == (const Bytes& other) const noexcept {
		return Compare(other);
	}

	/// Compare with another byte container												
	///	@param other - the byte container to compare with							
	///	@return true if both containers are not identical							
	bool Bytes::operator != (const Bytes& other) const noexcept {
		return !(*this == other);
	}

	/// Clone the byte container																
	///	@return the cloned byte container												
	Bytes Bytes::Clone() const {
		Bytes result {Disown(*this)};
		if (mCount) {
			const auto byteSize = RequestByteSize(mCount);
			result.mEntry = Allocator::Allocate(byteSize);
			result.mRaw = result.mEntry->GetBlockStart();
			result.mReserved = byteSize;
			CopyMemory(mRaw, result.mRaw, mCount);
		}
		else {
			result.mEntry = nullptr;
			result.mRaw = nullptr;
			result.mReserved = 0;
		}
		
		return Abandon(result);
	}

	/// Pick a constant part of the byte array											
	///	@param start - the starting byte offset										
	///	@param count - the number of bytes after 'start' to remain				
	///	@return a new container that references the original memory				
	Bytes Bytes::Crop(const Offset& start, const Count& count) const {
		return TAny::Crop<Bytes>(start, count);
	}

	/// Pick a part of the byte array														
	///	@param start - the starting byte offset										
	///	@param count - the number of bytes after 'start' to remain				
	///	@return a new container that references the original memory				
	Bytes Bytes::Crop(const Offset& start, const Count& count) {
		return TAny::Crop<Bytes>(start, count);
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
			MoveMemory(mRaw + end, mRaw + start, mCount - removed);
		}

		mCount -= removed;
		return *this;
	}

	/// Extend the byte sequence, change count, and return the new range			
	/// Static byte containers can't be extended											
	///	@param count - the number of bytes to append									
	///	@return the extended part - you will not be allowed to resize it		
	Bytes Bytes::Extend(const Count& count) {
		return TAny::Extend<Bytes>(count);
	}

} // namespace Langulus::Anyness
