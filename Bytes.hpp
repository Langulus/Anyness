#pragma once
#include "TAny.hpp"

namespace Langulus::Anyness
{

	///																								
	///	BYTE CONTAINER																			
	///																								
	/// Convenient wrapper for raw byte sequences										
	///																								
	class Bytes : public TAny<Byte> {
		LANGULUS(DEEP) false;		
	public:
		Bytes() = default;
		Bytes(const Bytes&);
		Bytes(Bytes&);
		Bytes(Bytes&&) noexcept = default;
		Bytes(const TAny&);
		Bytes(TAny&);
		Bytes(TAny&&) noexcept;

		Bytes(const void*, const Size&);
		Bytes(void*, const Size&);
		
		Bytes(const Disowned<Bytes>&) noexcept;
		Bytes(Abandoned<Bytes>&&) noexcept;
		Bytes(const Disowned<TAny>&) noexcept;
		Bytes(Abandoned<TAny>&&) noexcept;
		
		~Bytes();

		Bytes& operator = (const Bytes&);
		Bytes& operator = (Bytes&&) noexcept;
		//Bytes& operator = (const TAny&);
		//Bytes& operator = (TAny&&) noexcept;

		Bytes& operator = (Disowned<Bytes>&&);
		Bytes& operator = (Abandoned<Bytes>&&) noexcept;
		//Bytes& operator = (Disowned<TAny>&&);
		//Bytes& operator = (Abandoned<TAny>&&) noexcept;
		
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

		NOD() bool Compare(const Bytes&) const noexcept;
		NOD() Count Matches(const Bytes&) const noexcept;
	};

} // namespace Langulus::Anyness

#include "Bytes.inl"
