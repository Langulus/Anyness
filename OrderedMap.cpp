///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "OrderedMap.hpp"

namespace Langulus::Anyness
{

	/// Copy-construct a map from a disowned map											
	/// The disowned map's contents will not be referenced							
	///	@param other - the map to disown													
	OrderedMap::OrderedMap(Disowned<OrderedMap>&& other) noexcept
		: UnorderedMap {other.Forward<UnorderedMap>()} { }

	/// Move-construct a map from an abandoned map										
	/// The abandoned map will be minimally reset, saving on some instructions	
	///	@param other - the map to abandon												
	OrderedMap::OrderedMap(Abandoned<OrderedMap>&& other) noexcept
		: UnorderedMap {other.Forward<UnorderedMap>()} { }

	/// Clone the map																				
	///	@return the cloned map																
	OrderedMap OrderedMap::Clone() const {
		OrderedMap cloned;
		static_cast<UnorderedMap&>(cloned) = UnorderedMap::Clone();
		return Abandon(cloned);
	}

} // namespace Langulus::Anyness
