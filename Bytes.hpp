#pragma once
#include "Block.hpp"

namespace Langulus::Anyness
{

	///																								
	///	BYTE CONTAINER																			
	///																								
	/// Convenient wrapper for raw byte sequences										
	///																								
	class Bytes : public Block {
	public:
		Bytes();
		Bytes(const Bytes&);
		Bytes(Bytes&&) noexcept = default;
		Bytes(const Byte*, const Count&);
		
		explicit Bytes(const Disowned<Bytes>&) noexcept;
		explicit Bytes(Abandoned<Bytes>&&) noexcept;
		
		~Bytes();

		Bytes& operator = (const Bytes&);
		Bytes& operator = (Bytes&&) noexcept;
		Bytes& operator = (const Disowned<Bytes>&);
		Bytes& operator = (Abandoned<Bytes>&&) noexcept;
		
	public:
		NOD() Bytes Clone() const;
		NOD() Bytes Crop(const Offset&, const Count&) const;
		Bytes& Remove(const Offset&, const Count&);
		void Null(const Count&);
		void Clear() noexcept;
		void Reset();
		Bytes Extend(const Count&);
		Hash GetHash() const;

		bool operator == (const Bytes&) const noexcept;
		bool operator != (const Bytes&) const noexcept;

		NOD() const Byte& operator[] (const Offset&) const;
		NOD() Byte& operator[] (const Offset&);
		NOD() const Byte& operator[] (const Index&) const;
		NOD() Byte& operator[] (const Index&);

		Count Matches(const Bytes&) const noexcept;

		RANGED_FOR_INTEGRATION(Bytes, Byte);

		template<class T>
		Bytes& operator += (const T&);
		template<class T>
		Bytes operator + (const T&) const;
		template<class T>
		friend NOD() Bytes operator + (const T&, const Bytes&) requires NotSame<T, Bytes>;
	};

} // namespace Langulus::Anyness

#include "Bytes.inl"
