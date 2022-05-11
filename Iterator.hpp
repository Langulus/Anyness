#pragma once
#include "inner/Integration.hpp"

namespace Langulus::Anyness
{

	struct fast_forward_tag {};


	///																								
	///	Generic iterator																		
	///																								
	template<bool CONSTANT, class CONTAINER>
	class Iterator {
	private:
		friend CONTAINER;
		using Node = typename CONTAINER::Node;
		using NodePtr = Conditional<CONSTANT, Node const*, Node*>;

		NodePtr mKeyVals{};
		uint8_t const* mInfo{};

	public:
		using Difference = std::ptrdiff_t;
		using Type = typename CONTAINER::Pair;
		using Reference = Conditional<CONSTANT, Type const&, Type&>;
		using Pointer = Conditional<CONSTANT, Type const*, Type*>;
		using Category = std::forward_iterator_tag;

		/// default constructed iterator can be compared to itself, but WON'T	
		/// return true when compared to end()												
		Iterator() = default;

		/// Rule of zero: nothing specified													
		/// The conversion constructor is only enabled for iterator to				
		/// const_iterator, so it doesn't accidentally work as a copy ctor		

		/// Conversion constructor from iterator to const_iterator					
		template<bool ALT_CONSTANT>
		Iterator(Iterator<ALT_CONSTANT, CONTAINER> const& other) noexcept requires (CONSTANT && !ALT_CONSTANT)
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

		template<bool ALT_CONSTANT>
		Iterator& operator = (Iterator<ALT_CONSTANT, CONTAINER> const& other) noexcept requires (CONSTANT && !ALT_CONSTANT) {
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

		Reference operator*() const {
			return **mKeyVals;
		}

		Pointer operator->() const {
			return &**mKeyVals;
		}

		template<bool ALT_CONSTANT>
		bool operator == (Iterator<ALT_CONSTANT, CONTAINER> const& o) const noexcept {
			return mKeyVals == o.mKeyVals;
		}

		template<bool ALT_CONSTANT>
		bool operator != (Iterator<ALT_CONSTANT, CONTAINER> const& o) const noexcept {
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
	};

} // namespace Langulus::Anyness