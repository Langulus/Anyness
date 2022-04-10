#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Concatenate bytes with bytes															
	inline Bytes operator + (const Bytes& lhs, const Bytes& rhs) {
		Bytes result;
		result.Allocate(lhs.GetCount() + rhs.GetCount(), false, true);
		pcCopyMemory(lhs.GetRaw(), result.GetRaw(), lhs.GetCount());
		pcCopyMemory(rhs.GetRaw(), result.GetRaw() + lhs.GetCount(), rhs.GetCount());
		return result;
	}

	/// Concatenate anything but bytes with bytes										
	template<class T>
	Bytes operator + (const T& lhs, const Bytes& rhs) requires (!IsBytes<T>) {
		Bytes converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}

	/// Concatenate bytes with anything but bytes										
	template<class T>
	Bytes operator + (const Bytes& lhs, const T& rhs) requires (!IsBytes<T>) {
		Bytes converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}

	/// Byte concatenation in place															
	template<class T>
	Bytes& Bytes::operator += (const T& rhs) {
		if constexpr (IsBytes<T>) {
			const auto count = pcVal(rhs).GetCount();
			auto mpoint = Extend(count);
			pcCopyMemory(pcVal(rhs).GetRaw(), mpoint.GetRaw(), count);
		}
		else {
			// Finally, attempt converting											
			Bytes converted;
			TConverter<T, Bytes>::Convert(rhs, converted);
			operator += (converted);
		}
		return *this;
	}

} // namespace Langulus::Anyness
