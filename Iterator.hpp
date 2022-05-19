///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "inner/Config.hpp"

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

		NodePtr mNode {};
		const uint8_t* mInfo {};

	public:
		using Difference = ::std::ptrdiff_t;
		using Type = typename CONTAINER::Type;
		using Reference = Conditional<CONSTANT, Type const&, Type&>;
		using Pointer = Conditional<CONSTANT, Type const*, Type*>;
		using Category = ::std::forward_iterator_tag;

		/// default constructed iterator can be compared to itself, but WON'T	
		/// return true when compared to end()												
		Iterator() = default;

		/// Rule of zero: nothing specified													
		/// The conversion constructor is only enabled for iterator to				
		/// const_iterator, so it doesn't accidentally work as a copy ctor		

		/// Conversion constructor from iterator to const_iterator					
		template<bool ALT_CONSTANT>
		Iterator(const Iterator<ALT_CONSTANT, CONTAINER>& other) noexcept requires (CONSTANT && !ALT_CONSTANT)
			: mNode {other.mNode}
			, mInfo {other.mInfo} {}

		/// Manual construction																	
		Iterator(NodePtr valPtr, const uint8_t* infoPtr) noexcept
			: mNode {valPtr}
			, mInfo {infoPtr} {}

		Iterator(NodePtr valPtr, const uint8_t* infoPtr, fast_forward_tag) noexcept
			: mNode {valPtr}
			, mInfo {infoPtr} {
			fastForward();
		}

		template<bool ALT_CONSTANT>
		Iterator& operator = (const Iterator<ALT_CONSTANT, CONTAINER>& other) noexcept requires (CONSTANT && !ALT_CONSTANT) {
			mNode = other.mNode;
			mInfo = other.mInfo;
			return *this;
		}

		/// Prefix increment - undefined behavior if we are at end()				
		Iterator& operator ++ () noexcept {
			mInfo++;
			mNode++;
			fastForward();
			return *this;
		}

		Iterator operator ++ (int) noexcept {
			auto tmp = *this;
			++(*this);
			return tmp;
		}

		Reference operator * () const {
			return **mNode;
		}

		Pointer operator -> () const {
			return &**mNode;
		}

		template<bool ALT_CONSTANT>
		bool operator == (const Iterator<ALT_CONSTANT, CONTAINER>& o) const noexcept {
			return mNode == o.mNode;
		}

		template<bool ALT_CONSTANT>
		bool operator != (const Iterator<ALT_CONSTANT, CONTAINER>& o) const noexcept {
			return mNode != o.mNode;
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
				mNode += sizeof(size_t);
			}

			size_t inc;
			if constexpr (LittleEndianMachine)
				inc = CountTrailingZeroes(n) / 8;
			else
				inc = CountLeadingZeroes(n) / 8;

			mInfo += inc;
			mNode += inc;
		}
	};

} // namespace Langulus::Anyness