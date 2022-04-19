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

		/*Path(const Disowned<TAny<Letter>>&) noexcept;
		Path(Abandoned<TAny<Letter>>&&) noexcept;*/

	public:
		NOD() Path Clone() const;
		NOD() Text GetExtension() const;
		NOD() Path GetDirectory() const;
		NOD() Path GetFilename() const;
		NOD() Path operator / (const Text&) const;
		Path& operator /= (const Text&);

		/*template<class RHS>
		Path& operator += (const RHS& rhs) {
			TAny<Letter>::operator+=<Path>(rhs);
			return *this;
		}

		template<class RHS>
		NOD() Path operator + (const RHS& rhs) const {
			return TAny<Letter>::operator+<Path>(rhs);
		}*/
	};

} // namespace Langulus::Anyness
