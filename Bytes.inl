#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Destructive byte concatenation														
	template<class T>
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
	}
	
} // namespace Langulus::Anyness
