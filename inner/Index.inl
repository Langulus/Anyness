///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include <limits>

namespace Langulus::Anyness
{

	/// Constructor from special index														
	///	@param value - the index to copy													
	constexpr Index::Index(const SpecialIndices& value) noexcept
		: mIndex {value} { }

	/// Constructor from signed integer														
	///	@param value - integer to set														
	template<CT::SignedInteger T>
	constexpr Index::Index(const T& value) noexcept
		: mIndex {value} { }
	
	/// Constructor from unsigned integer													
	///	@param value - integer to set														
	template<CT::UnsignedInteger T>
	constexpr Index::Index(const T& value)
		: mIndex {static_cast<Type>(value)} {
		LANGULUS_ASSERT(value <= static_cast<T>(MaxIndex),
			Except::Access, "Index overflow");
	}

	/// Constrain the index to some count (immutable)									
	///   If index is out of scope, return None											
	///   If index is special - return it as it is										
	constexpr Index Index::Constrained(const Count count) const noexcept {
		switch (mIndex) {
		case Auto: case First: case Front:
			return {0};
		case All: case Back:
			return {count};
		case Last:
			return count 
				? Index {count - 1} 
				: Index {None};
		case Middle:
			return count / 2;
		case None:
			return None;
		default: {
			const Type c = static_cast<Type>(count);
			if (IsSpecial()) {
				if (IsArithmetic()) {
					// Index is negative, wrap it around (if in range)			
					return c + mIndex >= 0 ? Index {c + mIndex} : Index {None};
				}

				// Index is a special value, so don't change anything			
				return mIndex;
			}

			return mIndex >= c ? Index {None} : Index {mIndex};
		}}
	}

	/// Get an unsigned offset from the index, if possible							
	/// Throws Except::Access if not possible to extract index						
	///	@return a valid offset																
	inline Offset Index::GetOffset() const {
		LANGULUS_ASSERT(!IsSpecial(), Except::Access,
			"Can't convert index to offset");
		return static_cast<Offset>(mIndex);
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

		auto digits = DigitsOf(other.mIndex);
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
		return IsSpecial() && IsArithmetic();
	}

	/// Check if index is special																
	constexpr bool Index::IsArithmetic() const noexcept {
		return mIndex >= Counter;
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
		if (!IsArithmetic() || !v.IsArithmetic()
			|| mIndex - v.mIndex < Counter)
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
				if (v.mIndex < 0 && v.mIndex >= Counter)
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
