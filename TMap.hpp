///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "TUnorderedMap.hpp"

namespace Langulus::Anyness
{

	///																								
	/// A highly optimized ordered hashmap implementation, using the Robin		
	/// Hood algorithm																			
	///																								
	template<CT::Data K, CT::Data V>
	class TOrderedMap : public TUnorderedMap<K, V> {
	public:
		using TUnorderedMap<K, V>::TUnorderedMap;

		NOD() TOrderedMap Clone() const;
	};


	/// The default map is always ordered													
	template<CT::Data K, CT::Data V>
	using TMap = TOrderedMap<K, V>;

} // namespace Langulus::Anyness

#include "TMap.inl"
