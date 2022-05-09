#pragma once
#include "inner/Integration.hpp"

namespace Langulus::Anyness
{

	struct fast_forward_tag {};


	///																								
	/// Generic iterator for both const_iterator and iterator						
	///																								
	template<bool IsConst, class Self>
	class Iterator {
	private:
		using Node = typename Self::Node;
		using NodePtr = Conditional<IsConst, Node const*, Node*>;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename Self::value_type;
		using reference = Conditional<IsConst, value_type const&, value_type&>;
		using pointer = Conditional<IsConst, value_type const*, value_type*>;
		using iterator_category = std::forward_iterator_tag;

		/// default constructed iterator can be compared to itself, but WON'T	
		/// return true when compared to end()												
		Iterator() = default;

		// Rule of zero: nothing specified. The conversion constructor is only enabled for
		// iterator to const_iterator, so it doesn't accidentally work as a copy ctor.

		/// Conversion constructor from iterator to const_iterator					
		template<bool OtherIsConst>
		Iterator(Iterator<OtherIsConst, Self> const& other) noexcept requires (IsConst && !OtherIsConst)
			: mKeyVals(other.mKeyVals)
			, mInfo(other.mInfo) {}

		Iterator(NodePtr valPtr, uint8_t const* infoPtr) noexcept
			: mKeyVals(valPtr)
			, mInfo(infoPtr) {}

		Iterator(NodePtr valPtr, uint8_t const* infoPtr, fast_forward_tag) noexcept
			: mKeyVals(valPtr)
			, mInfo(infoPtr) {
			fastForward();
		}

		template<bool OtherIsConst>
		Iterator& operator = (Iterator<OtherIsConst, Self> const& other) noexcept requires (IsConst && !OtherIsConst) {
			mKeyVals = other.mKeyVals;
			mInfo = other.mInfo;
			return *this;
		}

		/// Prefix increment - undefined behavior if we are at end()				
		Iterator& operator++() noexcept {
			mInfo++;
			mKeyVals++;
			fastForward();
			return *this;
		}

		Iterator operator++(int) noexcept {
			auto tmp = *this;
			++(*this);
			return tmp;
		}

		reference operator*() const {
			return **mKeyVals;
		}

		pointer operator->() const {
			return &**mKeyVals;
		}

		template<bool O>
		bool operator==(Iterator<O, Self> const& o) const noexcept {
			return mKeyVals == o.mKeyVals;
		}

		template<bool O>
		bool operator!=(Iterator<O, Self> const& o) const noexcept {
			return mKeyVals != o.mKeyVals;
		}

	private:
		/// Fast forward to the next non-free info byte									
		/// I've tried a few variants that don't depend on intrinsics, but		
		/// unfortunately they are quite a bit slower than this one.				
		/// So I've reverted that change again. See map_benchmark.					
		void fastForward() noexcept {
			size_t n = 0;
			while (0U == (n = unaligned_load<size_t>(mInfo))) {
				mInfo += sizeof(size_t);
				mKeyVals += sizeof(size_t);
			}

			size_t inc;
			if constexpr (LittleEndianMachine)
				inc = CountTrailingZeroes(n) / 8;
			else
				inc = CountLeadingZeroes(n) / 8;

			mInfo += inc;
			mKeyVals += inc;
		}

		//friend class Table<IsFlat, MaxLoadFactor100, key_type, mapped_type>;
		NodePtr mKeyVals{};
		uint8_t const* mInfo{};
	};

} // namespace Langulus::Anyness