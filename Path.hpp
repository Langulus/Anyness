#pragma once
#include "Text.hpp"

namespace Langulus::Anyness
{

	///																								
	///	File path container																	
	///																								
	class Path : public Text {
	public:
		using Text::Text;

		Path(const Disowned<Path>&) noexcept;
		Path(Abandoned<Path>&&) noexcept;

	public:
		NOD() Path Clone() const;
		NOD() Text GetExtension() const;
		NOD() Path GetDirectory() const;
		NOD() Path GetFilename() const;
		NOD() Path operator / (const Text&) const;
		Path& operator /= (const Text&);
	};

} // namespace Langulus::Anyness
