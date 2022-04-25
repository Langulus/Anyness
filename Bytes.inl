#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

	/// Construct via constant shallow copy												
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(const Bytes& other)
		: TAny {other} { }

	/// Construct via mutable shallow copy													
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(Bytes& other)
		: TAny {other} { }

	/// Construct via constant shallow copy of TAny										
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(const TAny& other)
		: TAny {other} { }

	/// Construct via shallow copy of TAny													
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(TAny& other)
		: TAny {other} { }

	/// Construct via move of TAny															
	///	@param other - the bytes to shallow-copy										
	inline Bytes::Bytes(TAny&& other) noexcept
		: TAny {Forward<TAny>(other)} { }

	/// Construct via disowned copy															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(const Disowned<Bytes>& other) noexcept
		: TAny {other.mValue} { }
	
	/// Construct via abandoned move															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Abandoned<Bytes>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }

	/// Construct via disowned copy															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(const Disowned<TAny>& other) noexcept
		: TAny {other.mValue} { }
	
	/// Construct via abandoned move															
	///	@param other - the bytes to move													
	inline Bytes::Bytes(Abandoned<TAny>&& other) noexcept
		: TAny {other.Forward<TAny>()} { }

	/// Construct manually via raw constant memory pointer and size				
	///	@param raw - raw memory to reference											
	///	@param size - number of bytes inside 'raw'									
	inline Bytes::Bytes(const void* raw, const Size& size)
		: TAny {reinterpret_cast<const Byte*>(raw), size} { }

	/// Construct manually via raw mjutable memory pointer and size				
	///	@param raw - raw memory to reference											
	///	@param size - number of bytes inside 'raw'									
	inline Bytes::Bytes(void* raw, const Size& size)
		: TAny {reinterpret_cast<Byte*>(raw), size} { }

	/// Destructor																					
	inline Bytes::~Bytes() {
		Free();
	}

	/// Shallow copy assignment																
	///	@param rhs - the byte container to shallow-copy								
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (const Bytes& rhs) {
		TAny::operator = (rhs);
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
	inline Bytes& Bytes::operator = (const Disowned<Bytes>& rhs) {
		TAny::operator = (rhs);
		return *this;
	}

	/// Move an abandoned byte container													
	///	@param rhs - the container to move												
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Abandoned<Bytes>&& rhs) noexcept {
		TAny::operator = (rhs.Forward<TAny>());
		return *this;
	}

	/// Shallow copy disowned bytes															
	///	@param rhs - the byte container to shallow-copy								
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (const Disowned<TAny>& rhs) {
		TAny::operator = (rhs);
		return *this;
	}

	/// Move an abandoned byte container													
	///	@param rhs - the container to move												
	///	@return a reference to this container											
	inline Bytes& Bytes::operator = (Abandoned<TAny>&& rhs) noexcept {
		TAny::operator = (rhs.Forward<TAny>());
		return *this;
	}
	
} // namespace Langulus::Anyness
