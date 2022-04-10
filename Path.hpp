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

		NOD() Path Clone() const;
		NOD() Text GetExtension() const;
		NOD() Path GetDirectory() const;
		NOD() Path GetFilename() const;
		NOD() Path operator / (const Text&) const;
	};

} // namespace Langulus::Anyness
