#pragma once
#include "../Block/Block.hpp"

namespace Langulus::Anyness
{

	///																								
	///	BYTE CONTAINER																			
	///																								
	/// Convenient wrapper for raw byte sequences										
	///																								
	class PC_API_MMS Bytes : public Block {
		REFLECT(Bytes);
	public:
		Bytes();
		Bytes(const Bytes&);
		Bytes(Bytes&&) noexcept = default;

		Bytes(const void*, pcptr);

		template<Dense T>
		explicit Bytes(const T&) requires (pcIsPOD<T> || pcHasBase<T, InternalID>);

		~Bytes();

	public:
		NOD() Bytes Clone() const;
		NOD() Bytes Crop(pcptr, pcptr) const;
		Bytes& Remove(pcptr, pcptr);
		void Null(pcptr);
		void Clear() noexcept;
		void Reset();

		TArray<pcbyte> Extend(pcptr);

		NOD() constexpr pcbyte* GetRaw() noexcept;
		NOD() constexpr const pcbyte* GetRaw() const noexcept;

		Hash GetHash() const;

		Bytes& operator = (const Bytes&);
		Bytes& operator = (Bytes&&) SAFE_NOEXCEPT();

		bool operator == (const Bytes&) const noexcept;
		bool operator != (const Bytes&) const noexcept;

		NOD() const pcbyte& operator[] (const pcptr) const;
		NOD() pcbyte& operator[] (const pcptr);

		pcptr Matches(const Bytes&) const noexcept;

		PC_RANGED_FOR_INTEGRATION(pcbyte, GetRaw(), mCount)

		template<class ANYTHING>
		Bytes& operator += (const ANYTHING&);
	};

	/// Compile time check for text items													
	template<class T>
	concept IsBytes = pcHasBase<T, Bytes>;

	NOD() Bytes operator + (const Bytes&, const Bytes&);

	template<class T>
	NOD() Bytes operator + (const T&, const Bytes&) requires (!IsBytes<T>);

	template<class T>
	NOD() Bytes operator + (const Bytes&, const T&) requires (!IsBytes<T>);

} // namespace Langulus::Anyness

namespace PCFW
{

	/// Explicit specialization for making bytes not deep								
	template<> constexpr bool pcIsDeep<Memory::Bytes> = false;
	template<> constexpr bool pcIsDeep<Memory::Bytes*> = false;
	template<> constexpr bool pcIsDeep<const Memory::Bytes*> = false;

}

#include "Bytes.inl"