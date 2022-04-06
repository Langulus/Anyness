#include "Path.hpp"

namespace Langulus::Anyness
{

	/// Path from text construction															
	///	@param other - text container to reference									
	Path::Path(const Text& other)
		: Text{ other } {}

	/// Path from text movement																
	///	@param other - text container to move											
	Path::Path(Text&& other) noexcept
		: Text{ pcForward<Text>(other) } {}

	/// Clone the path, preserving type														
	///	@return the cloned path																
	Path Path::Clone() const {
		return Text::Clone();
	}

	/// Return the lowercase file extension (the part after the last '.')		
	///	@return a cloned text container with the extension							
	Text Path::GetExtension() const {
		Count offset = 0;
		if (Text::FindOffsetReverse(".", offset))
			return Text::Crop(offset + 1, mCount - offset - 1).Lowercase();
		return {};
	}

	/// Return the directory part of the path												
	///	@return the directory part, including the last '/'							
	Path Path::GetDirectory() const {
		Count offset = 0;
		if (Text::FindOffsetReverse("/", offset))
			return Text::Crop(0, offset + 1);
		return {};
	}

	/// Return the filename part of the path												
	///	@return the filename part (after the last '/')								
	Path Path::GetFilename() const {
		Count offset = 0;
		if (Text::FindOffsetReverse("/", offset))
			return Text::Crop(offset + 1, mCount - offset - 1);
		return *this;
	}

	/// Append a subdirectory or filename													
	///	@param rhs - the text to append													
	///	@return the combined directory name												
	Path Path::operator / (const Text& rhs) const {
		if (last() == '/') {
			if (rhs.last() == '/')
				return *this + rhs.Crop(1, rhs.GetCount() - 1);
			else
				return *this + rhs;
		}
		else {
			if (rhs.last() == '/')
				return *this + rhs;
			else
				return *this + "/" + rhs;
		}
	}

} // namespace Langulus::Anyness
