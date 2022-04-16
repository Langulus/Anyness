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
	public:
		Bytes() = default;
		Bytes(const Bytes&);
		Bytes(Bytes&&) noexcept = default;
		Bytes(const Byte*, const Count&);
		
		Bytes(const Disowned<Bytes>&) noexcept;
		Bytes(Abandoned<Bytes>&&) noexcept;
		
		~Bytes();

		Bytes& operator = (const Bytes&);
		Bytes& operator = (Bytes&&) noexcept;

		Bytes& operator = (const Disowned<Bytes>&);
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

		NOD() bool Compare(const Bytes&) const noexcept;
		NOD() Count Matches(const Bytes&) const noexcept;

		template<class T>
		Bytes& operator += (const T&);
		template<class T>
		NOD() Bytes operator + (const T&) const;
		template<class T>
		friend Bytes operator + (const T&, const Bytes&) requires NotSame<T, Bytes>;
	};

} // namespace Langulus::Anyness

#include "Bytes.inl"
