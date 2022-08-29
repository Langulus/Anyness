///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Byte container copy-construction													
	/// Notice how container is explicitly cast to base class when forwarded	
	/// If that is not done, TAny will use the CT::Deep constructor instead		
	///	@param other - container to reference											
	inline Bytes::Bytes(const Bytes& other)
		: TAny {static_cast<const TAny&>(other)} {}

	/// Byte container move-construction													
	///	@param other - container to move													
	inline Bytes::Bytes(Bytes&& other) noexcept
		: TAny {Forward<TAny>(other)} {}

	/// Byte container copy-construction from TAny<Byte> base						
	///	@param other - container to reference											
	inline Bytes::Bytes(const TAny& other)
		: TAny {other} {}

	/// Byte container mvoe-construction from TAny<Byte> base						
	///	@param other - container to move													
	inline Bytes::Bytes(TAny&& other) noexcept
		: TAny {Forward<TAny>(other)} {}

	/// Construct via disowned copy															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Disowned<Bytes>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }
	
	/// Construct via abandoned move															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Abandoned<Bytes>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }

	/// Construct via disowned copy of TAny<Byte>										
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Disowned<TAny>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }
	
	/// Construct via abandoned move of TAny<Byte>										
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Abandoned<TAny>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }

	/// Construct manually via raw constant memory pointer and size				
	///	@param raw - raw memory to reference											
	///	@param size - number of bytes inside 'raw'									
	inline Bytes::Bytes(const void* raw, const Size& size)
		: TAny {static_cast<const Byte*>(raw), size} { }

	/// Construct manually via raw mjutable memory pointer and size				
	///	@param raw - raw memory to reference											
	///	@param size - number of bytes inside 'raw'									
	inline Bytes::Bytes(void* raw, const Size& size)
		: TAny {static_cast<Byte*>(raw), size} { }

	/// Construct by interpreting anything POD as bytes								
	///	@param value - the data to interpret											
	template<CT::POD T>
	Bytes::Bytes(const T& value) requires CT::Dense<T>
		: Bytes {&value, sizeof(T)} { }

	/// Construct by interpreting a string literal										
	///	@param value - the string to interpret											
	inline Bytes::Bytes(const Token& value)
		: Bytes {value.data(), value.size() * sizeof(Letter)} { }

	/// Construct by interpreting a meta definition										
	///	@param value - the meta to interpret											
	inline Bytes::Bytes(const RTTI::Meta* value)
		: Bytes {} {
		if (value) {
			*this += Count {value->mToken.size()};
			*this += value->mToken;
		}
		else *this += Count {0};
	}

	/// Shallow copy assignment from immutable byte container						
	///	@param rhs - the byte container to shallow-copy								
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (const Bytes& rhs) {
		TAny::operator = (static_cast<const TAny&>(rhs));
		return *this;
	}

	/// Move byte container																		
	///	@param rhs - the container to move												
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Bytes&& rhs) noexcept {
		TAny::operator = (Forward<TAny>(rhs));
		return *this;
	}

	/// Shallow copy disowned bytes															
	///	@param rhs - the byte container to shallow-copy								
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Disowned<Bytes>&& rhs) {
		TAny::operator = (rhs.Forward<TAny>());
		return *this;
	}

	/// Move an abandoned byte container													
	///	@param rhs - the container to move												
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Abandoned<Bytes>&& rhs) noexcept {
		TAny::operator = (rhs.Forward<TAny>());
		return *this;
	}
	
	/// Convert RHS to bytes and append destructively									
	///	@tparam RHS - type of RHS (deducible)											
	///	@param rhs - the stuff to convert and append									
	///	@return a reference to the byte container										
	template<class RHS>
	Bytes& Bytes::operator += (const RHS& rhs) {
		TAny::template operator += <Bytes, RHS> (rhs);
		return *this;
	}

	/// Convert RHS to bytes and append 													
	///	@tparam RHS - type of RHS (deducible)											
	///	@param rhs - the stuff to convert and append									
	///	@return a new byte container with RHS appended								
	template<class RHS>
	Bytes Bytes::operator + (const RHS& rhs) const {
		return TAny::template operator + <Bytes, RHS> (rhs);
	}

} // namespace Langulus::Anyness
