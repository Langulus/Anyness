#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Hash the byte sequence																	
	///	@return a hash of the contained byte sequence								
	Hash Bytes::GetHash() const {
		const auto asString = reinterpret_cast<const char*>(GetRaw());
		return ::std::hash<::std::string_view>()({asString, GetCount()});
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

	/// Compare with another byte array														
	///	@param other - bytes to compare with											
	///	@return true if both containers match completely							
	bool Bytes::Compare(const Bytes& other) const noexcept {
		if (mRaw == other.mRaw)
			return mCount == other.mCount;
		else if (mCount != other.mCount)
			return false;

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (*t1 == *t2) {
			++t1;
			++t2;
		}

		return (t1 - GetRaw()) == mCount;
	}


	/// Compare byte sequences and return matching bytes								
	///	@param other - bytes to compare with											
	///	@return the number of matching bytes											
	Count Bytes::Matches(const Bytes& other) const noexcept {
		if (mRaw == other.mRaw)
			return ::std::min(mCount, other.mCount);

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (*t1 == *t2) {
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

	/// Clone the byte container																
	///	@return the cloned byte container												
	Bytes Bytes::Clone() const {
		Bytes result {Disown(*this)};
		if (mCount) {
			result.mEntry = Allocator::Allocate(GetSize());
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
