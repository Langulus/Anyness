///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Path.hpp"

namespace Langulus::Anyness
{

	/// Copy other but do not reference it, because it is disowned					
	///	@param other - the block to copy													
	Path::Path(const Disowned<Path>& other) noexcept
		: Text {other.Forward<Text>()} { }	
	
	/// Move other, but do not bother cleaning it up, because it is disowned	
	///	@param other - the block to move													
	Path::Path(Abandoned<Path>&& other) noexcept
		: Text {other.Forward<Text>()} { }	
	
	/// Clone the path, preserving type														
	///	@return the cloned path																
	Path Path::Clone() const {
		return Path {Abandon(Text::Clone())};
	}

	/// Return the lowercase file extension (the part after the last '.')		
	///	@return a cloned text container with the extension							
	Text Path::GetExtension() const {
		Offset offset {};
		if (Text::FindOffsetReverse('.', offset))
			return Text::Crop(offset + 1, mCount - offset - 1).Lowercase();
		return {};
	}

	/// Return the directory part of the path												
	///	@return the directory part, including the last '/'							
	Path Path::GetDirectory() const {
		Offset offset {};
		if (Text::FindOffsetReverse('/', offset))
			return Text::Crop(0, offset + 1);
		return {};
	}

	/// Return the filename part of the path												
	///	@return the filename part (after the last '/')								
	Path Path::GetFilename() const {
		Offset offset {};
		if (Text::FindOffsetReverse('/', offset))
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
			else {
				auto temp = *this + '/';
				temp += rhs;
				return Path {Abandon(temp)};
			}
		}
	}

	/// Append a subdirectory or filename													
	///	@param rhs - the text to append													
	///	@return the combined directory name												
	Path& Path::operator /= (const Text& rhs) {
		if (last() == '/') {
			if (rhs.last() == '/')
				*this += rhs.Crop(1, rhs.GetCount() - 1);
			else
				*this += rhs;
		}
		else {
			if (rhs.last() == '/')
				*this += rhs;
			else {
				*this += '/';
				*this += rhs;
			}
		}
		return *this;
	}

} // namespace Langulus::Anyness
