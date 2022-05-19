///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include <Langulus.Core.hpp>

namespace Langulus::Anyness
{

	///																								
	template <class T>
	T unaligned_load(void const* ptr) noexcept {
		// Using memcpy so we don't get into unaligned load problems		
		// Compiler should optimize this very well anyways						
		T t;
		::std::memcpy(&t, ptr, sizeof(T));
		return t;
	}


	/// Hash a sequence of bytes																
	///	@param ptr - memory start															
	///	@param len - number of bytes to hash											
	///	@return the hash																		
	inline Hash HashBytes(void const* ptr, size_t len) noexcept {
		static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
		static constexpr uint64_t seed = UINT64_C(0xe17a1465);
		static constexpr unsigned int r = 47;

		auto const* const data64 = static_cast<uint64_t const*>(ptr);
		uint64_t h = seed ^ (len * m);

		size_t const n_blocks = len / 8;
		for (size_t i = 0; i < n_blocks; ++i) {
			auto k = unaligned_load<uint64_t>(data64 + i);

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		auto const* const data8 = reinterpret_cast<uint8_t const*>(data64 + n_blocks);
		switch (len & 7U) {
		case 7:
			h ^= static_cast<uint64_t>(data8[6]) << 48U;
			[[fallthrough]];
		case 6:
			h ^= static_cast<uint64_t>(data8[5]) << 40U;
			[[fallthrough]];
		case 5:
			h ^= static_cast<uint64_t>(data8[4]) << 32U;
			[[fallthrough]];
		case 4:
			h ^= static_cast<uint64_t>(data8[3]) << 24U;
			[[fallthrough]];
		case 3:
			h ^= static_cast<uint64_t>(data8[2]) << 16U;
			[[fallthrough]];
		case 2:
			h ^= static_cast<uint64_t>(data8[1]) << 8U;
			[[fallthrough]];
		case 1:
			h ^= static_cast<uint64_t>(data8[0]);
			h *= m;
			[[fallthrough]];
		default:
			break;
		}

		h ^= h >> r;

		// Not doing the final step here, because this will be done by keyToIdx	
		// anyways																					
		//TODO move it here to be more general?
		// h *= m;
		// h ^= h >> r;
		return static_cast<Hash>(h);
	}

	template<CT::Number N>
	constexpr Hash HashNumber(const N& n) noexcept {
		// tried lots of different hashes, let's stick with murmurhash3. It's simple, fast, well tested,
		// and doesn't need any special 128bit operations.
		auto x = static_cast<uint64_t>(n);
		x ^= x >> 33U;
		x *= UINT64_C(0xff51afd7ed558ccd);
		x ^= x >> 33U;

		// not doing the final step here, because this will be done by keyToIdx anyways
		// x *= UINT64_C(0xc4ceb9fe1a85ec53);
		// x ^= x >> 33U;
		return static_cast<Hash>(x);
	}

	/// Hash any hashable data, including fundamental types							
	///	@tparam T - type to hash (deducible)											
	///	@param data - the data to hash													
	///	@return the hash																		
	template<class T>
	constexpr Hash HashData(const T& data) noexcept {
		if constexpr (CT::Hashable<T>) {
			// Hashable via a member GetHash() function							
			return data.GetHash();
		}
		else if constexpr (CT::Number<T>) {
			// A fundamental number is built-in hashable							
			return HashNumber(data);
		}
		else if constexpr (requires { Uneval<::std::hash<T>&>().operator ()(data); }) {
			// Hashable via std::hash													
			::std::hash<T> hasher;
			return hasher(data);
		}
		else if constexpr (CT::POD<T>) {
			// Explicitly marked POD type is always hashable, but be			
			// careful for POD types with padding - the junk inbetween		
			// members can interfere with the hash, giving unique hashes	
			// where the same hashes should be produced							
			return HashBytes(&data, sizeof(T));
		}
		else LANGULUS_ASSERT("Can't hash data");
	}

} // namespace Langulus::Anyness
