#pragma once
#include "Integration.hpp"
#include <type_traits>
#include <limits>

namespace Langulus
{

	/// Forward lvalue as either lvalue or rvalue										
	template<class T>
	NOD() constexpr T&& Forward(Deref<T>& _Arg) noexcept {
		return static_cast<T&&>(_Arg);
	}

	/// Forward rvalue as rvalue																
	template<class T>
	NOD() constexpr T&& Forward(Deref<T>&& _Arg) noexcept {
		static_assert(!::std::is_lvalue_reference_v<T>, "Bad forward call");
		return static_cast<T&&>(_Arg);
	}

	/// Forward as movable																		
	template<class T>
	NOD() constexpr Deref<T>&& Move(T&& _Arg) noexcept {
		return static_cast<Deref<T>&&>(_Arg);
	}

	/// Abandon a value																			
	/// Same as Move, but resets only mandatory data inside source after move	
	/// essentially saving up on a couple of instructions								
	template<class T>
	struct Abandoned {
		T&& Value;
		
		/// Forward as abandoned																
		template<class ALT_T>
		NOD() constexpr Abandoned<ALT_T> Forward() const noexcept {
			return Abandoned<ALT_T>{
				Langulus::Forward<ALT_T>(Value)
			};
		}
	};
	
	template<class T>
	NOD() constexpr Abandoned<T> Abandon(T&& _Arg) noexcept {
		return Abandoned<T>{Forward<T>(_Arg)};
	}

	/// Disown a value																			
	/// Same as a shallow-copy, but never references									
	template<class T>
	struct Disowned {
		const T& Value;
	};
	
	template<class T>
	NOD() constexpr Disowned<T> Disown(const T& item) noexcept {
		return Disowned<T>{item};
	}
	
	/// Get number of digits inside an integer											
	/// The routine is as statically optimized as possible							
	///	@param n - value																		
	///	@return the number of digits inside the value								
	template<class T, T OFFSET = 10, Count RESULT = 1>
	NOD() constexpr Count DigitsOf(const T& n) noexcept {
		if constexpr (::std::numeric_limits<T>::digits10 + 1 > RESULT) {
			if constexpr (Signed<T>) {
				// Get rid of negatives													
				using UnsignedT = ::std::make_unsigned_t<T>;
				return DigitsOf<UnsignedT, OFFSET, RESULT>(
					static_cast<UnsignedT>(::std::abs(n))
				);
			}
			else {
				if (n < OFFSET) {
					// Early exit															
					return RESULT;
				}

				// Next digit																
				return DigitsOf<T, OFFSET * 10, RESULT + 1>(n);
			}
		}
		else {
			// Reached the numeric limit												
			return RESULT;
		}
	}

} // namespace Langulus
