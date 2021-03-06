///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Pair.hpp"

namespace Langulus::Anyness
{

	///																								
	///	A helper structure for pairing keys and values of any type				
	///																								
	template<CT::Data K, CT::Data V>
	struct TPair : public APair {
		using Key = K;
		using Value = V;

		Key mKey;
		Value mValue;

		TPair() = default;
		TPair(const TPair&) = default;
		TPair(TPair&&) noexcept = default;

		constexpr TPair(K&&, V&&) noexcept requires CT::MoveMakableNoexcept<K, V>;
		constexpr TPair(K&&, V&&) requires CT::MoveMakable<K, V>;
		constexpr TPair(const K&, const V&) noexcept requires CT::CopyMakableNoexcept<K, V>;
		constexpr TPair(const K&, const V&) requires CT::CopyMakable<K, V>;

		constexpr void Swap(TPair&) noexcept requires CT::SwappableNoexcept<K, V>;
		constexpr void Swap(TPair&) requires CT::Swappable<K, V>;

		TPair Clone() const;

		bool operator == (const TPair&) const noexcept = default;
		auto operator <=> (const TPair&) const noexcept;
		TPair& operator = (const TPair&) = default;
		TPair& operator = (TPair&&) noexcept = default;
	};

} // namespace Langulus::Anyness

#include "TPair.inl"