#include "Map.hpp"
#include "Text.hpp"

namespace Langulus::Anyness
{

	/// Copy construction																		
	Map::Map(const Map& copy)
		: Any {static_cast<const Any&>(copy)}
		, mKeys {copy.mKeys} { }

	/// Move construction																		
	Map::Map(Map&& copy) noexcept
		: Any {Forward<Any>(copy)}
		, mKeys {Move(copy.mKeys)} { }

	/// Manual construction of a constant container										
	Map::Map(const Block& keys, const Block& values)
		: Any {values}
		, mKeys {keys} { }

	/// Create a typed map container															
	Map Map::From(DMeta keyType, DMeta valueType, const DataState& state) noexcept {
		return Map {
			Block {state, keyType},
			Block {state, valueType}
		};
	}

	/// Get the map token for serialization and logging								
	Text Map::GetMapToken(DMeta keytype, DMeta valuetype) {
		Text name;
		name += keytype ? keytype->mToken : MetaData::DefaultToken;
		name += "Mapped";
		name += valuetype ? valuetype->mToken : MetaData::DefaultToken;
		return name;
	}

	/// Clone the map																				
	Map Map::Clone() const {
		Map clone;
		static_cast<Any&>(clone) = Any::Clone();
		clone.mKeys = mKeys.Clone();
		return clone;
	}

	/// Deconstructs all elements but keeps reserved memory if possible			
	void Map::Clear() {
		Any::Clear();
		mKeys.Clear();
	}

	/// Deconstructs and releases reserved memory										
	void Map::Reset() {
		Any::Reset();
		mKeys.Reset();
	}

	/// Copy operator. Doesn't clone data, only references it						
	/// Never copies if types are not compatible, only clears.						
	Map& Map::operator = (const Map& anypack) {
		Reset();
		new (this) Map {anypack};
		return *this;
	}

	/// Copy operator. Doesn't clone data, only references it						
	/// Never copies if types are not compatible, only clears.						
	Map& Map::operator = (Map&& anypack) noexcept {
		Reset();
		new (this) Map {Forward<Map>(anypack)};
		return *this;
	}

} // namespace Langulus::Anyness
