#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Construct via shallow copy															
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(const Bytes& other)
		: TAny {other} { }

	/// Construct via shallow copy of TAny													
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(const TAny& other)
		: TAny {other} { }

	/// Construct via move of TAny															
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(TAny&& other) noexcept
		: TAny {Forward<TAny>(other)} { }

	/// Construct via disowned copy															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(const Disowned<Bytes>& other) noexcept
		: TAny {other.mValue} { }
	
	/// Construct via abandoned move															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Abandoned<Bytes>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }

	/// Construct via disowned copy															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(const Disowned<TAny>& other) noexcept
		: TAny {other.mValue} { }
	
	/// Construct via abandoned move															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Abandoned<TAny>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }

	/// Construct manually																		
	///	@param raw - raw memory to reference											
	///	@param count - number of bytes inside 'raw'									
	inline Bytes::Bytes(const Byte* raw, const Count& count)
		: TAny {raw, count} { }

	/// Destructor																					
	inline Bytes::~Bytes() {
		Free();
	}

	/// Shallow copy assignment																
	///	@param rhs - the byte container to shallow-copy								
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (const Bytes& rhs) {
		TAny::operator = (rhs);
		return *this;
	}

	/// Move byte container																		
	///	@param rhs - the container to move												
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Bytes&& rhs) noexcept {
		TAny::operator = (Forward<TAny>(rhs));
		return *this;
	}

	/// Shallow copy disowned bytes															
	///	@param rhs - the byte container to shallow-copy								
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (const Disowned<Bytes>& rhs) {
		TAny::operator = (rhs);
		return *this;
	}

	/// Move an abandoned byte container													
	///	@param rhs - the container to move												
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Abandoned<Bytes>&& rhs) noexcept {
		TAny::operator = (rhs.Forward<TAny>());
		return *this;
	}

	/// Shallow copy disowned bytes															
	///	@param rhs - the byte container to shallow-copy								
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (const Disowned<TAny>& rhs) {
		TAny::operator = (rhs);
		return *this;
	}

	/// Move an abandoned byte container													
	///	@param rhs - the container to move												
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Abandoned<TAny>&& rhs) noexcept {
		TAny::operator = (rhs.Forward<TAny>());
		return *this;
	}

	/// Destructive byte concatenation														
	/*template<class T>
	Bytes& Bytes::operator += (const T& rhs) {
		if constexpr (Sparse<T>)
			return operator += (*rhs);
		else if constexpr (Same<T, Bytes>) {
			// Concatenate bytes															
			const auto count = rhs.GetCount();
			Block::Allocate(mCount + count, false, false);
			Block::CopyMemory(rhs.mRaw, mRaw, count);
			Block::mCount += count;
			return *this;
		}
		else if constexpr (Convertible<T, Bytes>) {
			// Finally, attempt converting											
			return operator += (static_cast<Bytes>(rhs));
		}
		else LANGULUS_ASSERT("Can't concatenate to Bytes - RHS is not convertible");
	}

	/// Concatenate byte containers															
	template<class T>
	Bytes Bytes::operator + (const T& rhs) const {
		if constexpr (Sparse<T>)
			return operator + (*rhs);
		else if constexpr (Same<T, Bytes>) {
			// Concatenate bytes															
			Bytes result = Disown(*this);
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
		else if constexpr (Convertible<T, Bytes>) {
			// Attempt converting														
			return operator + (static_cast<Bytes>(rhs));
		}
		else LANGULUS_ASSERT("Can't concatenate to Bytes - RHS is not convertible");
	}
	
	/// Concatenate anything with bytes														
	template<class T>
	NOD() Bytes operator + (const T& lhs, const Bytes& rhs) requires NotSame<T, Bytes> {
		if constexpr (Sparse<T>)
			return operator + (*lhs, rhs);
		else if constexpr (Convertible<T, Bytes>) {
			auto result = static_cast<Bytes>(lhs);
			result += rhs;
			return result;
		}
		else LANGULUS_ASSERT("Can't concatenate to Bytes - LHS is not convertible");
	}*/
	
} // namespace Langulus::Anyness
