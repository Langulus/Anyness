///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "TAny.hpp"

namespace Langulus::Anyness
{

	///																								
	///	BYTE CONTAINER																			
	///																								
	/// Convenient wrapper for raw byte sequences										
	/// Can represent any POD type as a sequence of bytes								
	///																								
	class Bytes : public TAny<Byte> {
		LANGULUS(DEEP) false;		
		LANGULUS_BASES(TAny<Byte>);

	public:
		Bytes() = default;
		Bytes(const Bytes&);
		Bytes(Bytes&&) noexcept;

		Bytes(const TAny&);
		Bytes(TAny&&) noexcept;

		template<CT::Deep T>
		Bytes(const T&) = delete;

		Bytes(const void*, const Size&);
		Bytes(void*, const Size&);
		
		Bytes(Disowned<Bytes>&&) noexcept;
		Bytes(Abandoned<Bytes>&&) noexcept;
		Bytes(Disowned<TAny<Byte>>&&) noexcept;
		Bytes(Abandoned<TAny<Byte>>&&) noexcept;

		template<CT::POD T>
		explicit Bytes(const T&) requires CT::Dense<T>;

		explicit Bytes(const Token&);
		explicit Bytes(const RTTI::Meta*);

		Bytes& operator = (const Bytes&);
		Bytes& operator = (Bytes&&) noexcept;

		Bytes& operator = (Disowned<Bytes>&&);
		Bytes& operator = (Abandoned<Bytes>&&) noexcept;
		
	public:
		NOD() Bytes Clone() const;
		NOD() Bytes Crop(const Offset&, const Count&) const;
		NOD() Bytes Crop(const Offset&, const Count&);
		Bytes& Remove(const Offset&, const Count&);
		void Null(const Count&);
		Bytes Extend(const Count&);
		Hash GetHash() const;

		bool operator == (const Bytes&) const noexcept;
		bool operator != (const Bytes&) const noexcept;

		///																							
		///	Concatenation																		
		///																							
		NOD() Bytes operator + (const Bytes&) const;
		NOD() Bytes operator + (Bytes&&) const;
		NOD() Bytes operator + (Disowned<Bytes>&&) const;
		NOD() Bytes operator + (Abandoned<Bytes>&&) const;

		Bytes& operator += (const Bytes&);
		Bytes& operator += (Bytes&&);
		Bytes& operator += (Disowned<Bytes>&&);
		Bytes& operator += (Abandoned<Bytes>&&);
	};

} // namespace Langulus::Anyness

#include "Bytes.inl"
