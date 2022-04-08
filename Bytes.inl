#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Construct from anything dense														
	///	@param value - the id to serialize												
	template<Dense T>
	Bytes::Bytes(const T& value) requires (pcIsPOD<T> || pcHasBase<T, InternalID>)
		: Bytes {} {
		if constexpr (pcHasBase<T, InternalID>) {
			if (!value) {
				(*this) += pcptr(0);
				return;
			}

			// Serializing internals to binary needs special care				
			Bytes result;
			#if LANGULUS_RTTI_IS(NAMED)
				const auto meta = value.GetMeta();
				const auto count = meta->GetToken().GetCount();
				result.Allocate(sizeof(count) + count);
				result += count;
				result += Bytes{meta->GetToken().GetRaw(), count};
			#elif LANGULUS_RTTI_IS(HASHED)
				result += Bytes{&value, sizeof(InternalID)};
			#else
				#error RTTI tactic not implemented
			#endif

			(*this) += result;
		}
		else if constexpr (Boolean<T>) {
			// Make sure booleans are always a single byte, either 1 or 0	
			const uchar byte { value ? 1 : 0 };
			(*this) += Bytes{ &byte, sizeof(uchar) };
		}
		else {
			// POD types are easily represented as bytes							
			(*this) += Bytes{ &value, sizeof(T) };
		}
	}

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
