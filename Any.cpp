///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Any.hpp"

namespace Langulus::Anyness
{

	/// Clone container																			
	///	@return the cloned container														
	Any Any::Clone() const {
		Any clone;
		Block::Clone(clone);
		return Abandon(clone);
	}

	/// Destroy all elements, but retain allocated memory if possible				
	void Any::Clear() {
		if (IsEmpty())
			return;

		if (GetUses() == 1) {
			// Only one use - just destroy elements and reset count,			
			// reusing the allocation for later										
			CallUnknownDestructors();
			ClearInner();
		}
		else {
			// We're forced to reset the memory, because it's in use			
			// Keep the type and state, though										
			const auto state = GetUnconstrainedState();
			const auto meta = mType;
			Reset();
			mType = meta;
			mState += state;
		}
	}

} // namespace Langulus::Anyness
