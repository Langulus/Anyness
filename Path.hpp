#pragma once
#include "Text.hpp"

namespace Langulus::Anyness
{

	///																								
	///	FILENAME CONTAINER																	
	///																								
	class LANGULUS_MODULE(Anyness) Path : public Text {
		REFLECT(Path);
	public:
		using Text::Text;

		Path(const Text&);
		Path(Text&&) noexcept;

		NOD() Path Clone() const;
		NOD() Text GetExtension() const;
		NOD() Path GetDirectory() const;
		NOD() Path GetFilename() const;
		NOD() Path operator / (const Text&) const;
	};

} // namespace Langulus::Anyness
