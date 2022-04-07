namespace PCFW::Memory
{

	/// By default, an index is always uiNone												
	constexpr Index::Index() noexcept
		: mIndex(uiNoneInner) { }

	/// Cast constructor from arbitrary arithmetic types								
	constexpr Index::Index(const ReservedIndices& value) noexcept
		: mIndex(value) { }

	/// Cast constructor from arbitrary arithmetic types								
	template<Number T>
	constexpr Index::Index(const T& value) noexcept
		: mIndex(static_cast<pcidx>(value)) { }

	/// Constrain the index to some count (immutable)									
	///   If index is out of scope, return uiNone										
	///   If index is special - return it as it is										
	constexpr Index Index::Constrained(const pcptr count) const noexcept {
		if (IsSpecial()) {
			if (IsArithmetic()) {
				// Index is negative, wrap it around (if in range)				
				return pcidx(count) + mIndex >= 0 ? pcidx(count) + mIndex : uiNoneInner;
			}

			// Index is a special value, so don't change anything				
			return mIndex;
		}

		return mIndex >= pcidx(count) ? uiNoneInner : mIndex;
	}

	/// Constrain the index to some count (destructive)								
	///	@param count - the count to constrain to										
	constexpr void Index::Constrain(const pcptr count) noexcept {
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
		return mIndex != uiNoneInner; 
	}

	/// Check invalidity																			
	constexpr bool Index::IsInvalid() const noexcept {
		return mIndex == uiNoneInner; 
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
		return mIndex >= uiMinInner;
	}

	/// Return true if index is valid														
	constexpr Index::operator bool () const noexcept {
		return IsValid();
	}

	/// Convert to any kind of number														
	template<Number T>
	constexpr Index::operator T () const noexcept {
		if constexpr (Signed<T>)
			return IsSpecial() ? T(0) : static_cast<T>(mIndex);
		else
			return static_cast<T>(mIndex);
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
		if (!IsArithmetic() || !v.IsArithmetic() || mIndex - v.mIndex < uiMinInner)
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
		case uiAllInner:
		case uiManyInner:
		case uiSingleInner:
			// uiSingleInner < uiManyInner < uiAllInner							
			switch (v.mIndex) {
			case uiAllInner:
			case uiManyInner:
			case uiSingleInner:
				return mIndex < v.mIndex;
			default:
				return false;
			};

		case uiBackInner:
		case uiMiddleInner:
		case uiFrontInner:
		case uiNoneInner:
			// uiNoneInner < uiFrontInner												
			// < uiMiddleInner < uiBackInner											
			switch (v.mIndex) {
			case uiBackInner:
			case uiMiddleInner:
			case uiFrontInner:
			case uiNoneInner:
				return mIndex < v.mIndex;
			default:
				return false;
			};

		case uiModeInner:
		case uiBiggestInner:
		case uiSmallestInner:
		case uiAutoInner:
		case uiRandomInner:
			// Uncomparable																
			return false;
		default:
			// Index is not special														
			if (mIndex < 0) {
				// Comparison is inverted												
				if (v.mIndex < 0 && v.mIndex >= uiMinInner)
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

} // namespace PCFW::Memory