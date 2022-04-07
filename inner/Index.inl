#pragma once
#include "Exceptions.hpp"
#include <limits>

namespace Langulus::Anyness
{

	/// Constructor from special index														
	constexpr Index::Index(const SpecialIndices& value) noexcept
		: mIndex {value} { }

	/// Constructor from raw type																
	constexpr Index::Index(const Type& value) noexcept
		: mIndex {value} { }
	
	/// Constructor from Count																	
	constexpr Index::Index(const Count& value)
		: mIndex {static_cast<Type>(value)} {
		SAFETY(if (value > static_cast<Count>(::std::numeric_limits<Type>::max()))
			throw Except::Overflow());
	}

	/// Constrain the index to some count (immutable)									
	///   If index is out of scope, return None											
	///   If index is special - return it as it is										
	constexpr Index Index::Constrained(const Count count) const noexcept {
		const Type c = static_cast<Type>(count);
		if (IsSpecial()) {
			if (IsArithmetic()) {
				// Index is negative, wrap it around (if in range)				
				return c + mIndex >= 0 ? c + mIndex : None;
			}

			// Index is a special value, so don't change anything				
			return mIndex;
		}

		return mIndex >= c ? None : mIndex;
	}

	/// Constrain the index to some count (destructive)								
	///	@param count - the count to constrain to										
	constexpr void Index::Constrain(const Count count) noexcept {
		*this = Constrained(count); 
	}

	/// Concatenate index (destructive)														
	///	@param count - the count to constrain to										
	constexpr void Index::Concat(const Index& other) noexcept {
		if (IsSpecial())
			return;

		auto digits = pcNumDigits(other.mIndex);
		while (digits) {
			mIndex *= 10;
			--digits;
		}

		mIndex += other.mIndex;
	}

	/// Check validity																			
	constexpr bool Index::IsValid() const noexcept {
		return mIndex != None; 
	}

	/// Check invalidity																			
	constexpr bool Index::IsInvalid() const noexcept {
		return mIndex == None; 
	}

	/// Check if index is special																
	constexpr bool Index::IsSpecial() const noexcept {
		return mIndex < 0;
	}

	/// Check if index is special																
	constexpr bool Index::IsReverse() const noexcept {
		return mIndex < 0 && IsArithmetic();
	}

	/// Check if index is special																
	constexpr bool Index::IsArithmetic() const noexcept {
		return mIndex >= SpecialIndexCounter;
	}

	/// Return true if index is valid														
	constexpr Index::operator bool () const noexcept {
		return IsValid();
	}

	/// Convert to any kind of number														
	constexpr Index::operator const Type& () const noexcept {
		return mIndex;
	}

	/// Destructive increment by 1															
	constexpr void Index::operator ++ () noexcept {
		if (IsArithmetic())
			++mIndex; 
	}

	/// Destructive decrement by 1															
	constexpr void Index::operator -- () noexcept {
		if (IsArithmetic())
			--mIndex;
	}

	/// Index - Index Arithmetics																
	constexpr void Index::operator += (const Index& v) noexcept {
		if (!IsArithmetic() || !v.IsArithmetic())
			return; 
		mIndex += v.mIndex; 
	}

	constexpr void Index::operator -= (const Index& v) noexcept {
		if (!IsArithmetic() || !v.IsArithmetic())
			return;
		mIndex -= v.mIndex; 
	}

	constexpr void Index::operator *= (const Index& v) noexcept {
		if (!IsArithmetic() || !v.IsArithmetic())
			return;
		mIndex *= v.mIndex; 
	}

	constexpr void Index::operator /= (const Index& v) noexcept {
		if (!IsArithmetic() || !v.IsArithmetic())
			return;
		mIndex /= v.mIndex; 
	}

	constexpr Index Index::operator + (const Index& v) const noexcept {
		if (!IsArithmetic() || !v.IsArithmetic())
			return *this;
		return mIndex + v.mIndex; 
	}

	constexpr Index Index::operator - (const Index& v) const noexcept {
		if (!IsArithmetic() || !v.IsArithmetic() || mIndex - v.mIndex < SpecialIndexCounter)
			return *this; 
		return mIndex - v.mIndex; 
	}

	constexpr Index Index::operator * (const Index& v) const noexcept {
		if (!IsArithmetic() || !v.IsArithmetic())
			return *this;
		return mIndex * v.mIndex; 
	}

	constexpr Index Index::operator / (const Index& v) const noexcept {
		if (!IsArithmetic() || !v.IsArithmetic())
			return *this;
		return mIndex / v.mIndex; 
	}

	/// Invert the index																			
	constexpr Index Index::operator - () const noexcept {
		if (IsArithmetic())
			return *this; 
		return -mIndex; 
	}

	/// Comparison																					
	constexpr bool Index::operator == (const Index& v) const noexcept {
		return mIndex == v.mIndex;
	}
	constexpr bool Index::operator != (const Index& v) const noexcept {
		return mIndex != v.mIndex;
	}

	constexpr bool Index::operator < (const Index& v) const noexcept {
		switch (mIndex) {
		case All: case Many: case Single:
			// Single < Many < All														
			switch (v.mIndex) {
			case All: case Many: case Single:
				return mIndex < v.mIndex;
			default:
				return false;
			};

		case Back: case Middle: case Front: case None:
			// None < Front < Middle < Back											
			switch (v.mIndex) {
			case Back: case Middle: case Front: case None:
				return mIndex < v.mIndex;
			default:
				return false;
			};

		case Mode: case Biggest: case Smallest: case Auto: case Random:
			// Uncomparable																
			return false;
			
		default:
			// Index is not special														
			if (mIndex < 0) {
				// Comparison is inverted												
				if (v.mIndex < 0 && v.mIndex >= SpecialIndexCounter)
					return mIndex > v.mIndex;
			}
			else {
				// Comparison is not inverted											
				if (v.mIndex > 0)
					return mIndex < v.mIndex;
			}
			return false;
		}
	}

	constexpr bool Index::operator > (const Index& v) const noexcept {
		return *this != v && !(*this < v);
	}

	constexpr bool Index::operator <= (const Index& v) const noexcept {
		return *this == v || *this < v;
	}

	constexpr bool Index::operator >= (const Index& v) const noexcept {
		return *this == v || !(*this < v);
	}

} // namespace Langulus::Anyness
