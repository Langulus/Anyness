///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "THashMap.hpp"

namespace Langulus::Anyness
{

	/// Set																							
	template <CT::Data T, Count MaxLoadFactor100 = 80>
	using THashDenseSet = Inner::Table<true, MaxLoadFactor100, T, void>;

	template <CT::Data T, Count MaxLoadFactor100 = 80>
	using THashSparseSet = Inner::Table<false, MaxLoadFactor100, T, void>;

	template <CT::Data T, Count MaxLoadFactor100 = 80>
	using THashSet = Inner::Table<CT::OnStackCriteria<T>, MaxLoadFactor100, T, void>;

} // namespace Langulus::Anyness
