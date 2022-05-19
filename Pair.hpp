///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	/// An abstract pair																			
	struct APair {
		LANGULUS_ABSTRACT() true;
	};


	///																								
	/// A helper structure for pairing keys and values of any type					
	///																								
	struct Pair : public APair {
		LANGULUS_ABSTRACT() false;
		Any mKey;
		Any mValue;
	};

} // namespace Langulus::Anyness

namespace Langulus::CT
{

	/// Check if T is a pair																	
	template<class T>
	concept Pair = DerivedFrom<T, ::Langulus::Anyness::APair>;

}