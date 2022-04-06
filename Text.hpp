#pragma once
#include "../TConverter.hpp"

namespace Langulus::Anyness
{

	class AMeta;


	///																								
	///	COUNT-TERMINATED TEXT WRAPPER														
	///																								
	/// Convenient wrapper for strings. Specialization for a memory block		
	/// which references the memory. Internally, it always contains UTF8 text	
	///																								
	class PC_API_MMS Text : public Block, NOT_DEEP {
		REFLECT(Text);
	public:
		Text();
		Text(const Text&);
		Text(Text&&) noexcept = default;

		Text(const char*, pcptr);
		Text(const wchar_t*, pcptr);

		Text(const LiteralText&);
		explicit Text(pcbyte);
		explicit Text(const char8&);
		explicit Text(const charw&);
		explicit Text(const DataID&);
		explicit Text(const TraitID&);
		explicit Text(const ConstID&);
		explicit Text(const VerbID&);
		explicit Text(const AException&);
		explicit Text(const Index&);
		explicit Text(const AMeta&);

		template<LiterallyNamed T>
		explicit Text(const T&) requires Dense<T>;
		template<Number T>
		explicit Text(const T&) requires Dense<T>;
		template<class T>
		Text(const T*) requires Dense<T>;
		template<pcptr S>
		Text(const std::array<char, S>&);

		~Text();

	public:
		NOD() Text Clone() const;
		NOD() Text Terminate() const;
		NOD() Text Lowercase() const;
		NOD() Text Uppercase() const;
		NOD() Text Crop(pcptr, pcptr) const;
		NOD() Text Strip(char) const;
		Text& Remove(pcptr, pcptr);
		void Clear() noexcept;
		void Reset();

		TArray<char> Extend(pcptr);

		NOD() constexpr operator LiteralText () const noexcept;

		NOD() static Text FromCodepoint(pcu32);
		NOD() TAny<pcu16> Widen16() const;
		NOD() TAny<pcu32> Widen32() const;

		#if WCHAR_MAX > 0xffff
			NOD() TAny<pcu32> Widen() const;
		#elif WCHAR_MAX > 0xff
			NOD() TAny<pcu16> Widen() const;
		#endif

		NOD() constexpr char* GetRaw() noexcept;
		NOD() constexpr const char* GetRaw() const noexcept;

		Hash GetHash() const;

		NOD() pcptr GetLineCount() const noexcept;

		Text& operator = (const Text&);
		Text& operator = (Text&&) SAFE_NOEXCEPT();

		bool operator == (const Text&) const noexcept;
		bool operator != (const Text&) const noexcept;

		NOD() const char& operator[] (const pcptr) const;
		NOD() char& operator[] (const pcptr);

		bool CompareLoose(const Text&) const noexcept;
		pcptr Matches(const Text&) const noexcept;
		NOD() pcptr MatchesLoose(const Text&) const noexcept;

		PC_RANGED_FOR_INTEGRATION(char, GetRaw(), mCount)

		NOD() bool FindOffset(const Text&, pcptr&) const;
		NOD() bool FindOffsetReverse(const Text&, pcptr&) const;
		NOD() bool Find(const Text&) const;
		NOD() bool FindWild(const Text&) const;

		template<class ANYTHING>
		Text& operator += (const ANYTHING&);

		struct Selection;
		NOD() Selection Select(const Text&);
		NOD() Selection Select(pcptr, pcptr);
		NOD() Selection Select(pcptr);
	};

	/// Compile time check for text items													
	template<class T>
	concept IsText = pcHasBase<T, Text>;


	/// This is a text modifier, that can be changed and syncs						
	/// changes along the chain. Just make sure you modify along					
	/// the chain, towards the main Text instance.										
	struct Text::Selection {
	public:
		Selection() noexcept = default;
		Selection(Text*, pcptr, pcptr) noexcept;

		NOD() const Text& GetText() const noexcept;
		NOD() pcptr GetStart() const noexcept;
		NOD() pcptr GetEnd() const noexcept;
		NOD() pcptr GetLength() const noexcept;

		NOD() const char& operator[] (const pcptr) const noexcept;
		NOD() char& operator[] (const pcptr) noexcept;

		Selection& operator << (const Text&);
		template<class K>
		Selection& operator << (const K&) requires (!IsText<K>);
		Selection& operator >> (const Text&);
		template<class K>
		Selection& operator >> (const K&) requires (!IsText<K>);
		Selection& Replace(const Text&);
		template<class K>
		Selection& Replace(const K&) requires (!IsText<K>);
		Selection& Delete();
		Selection& Backspace();

	private:
		Text* mText = nullptr;
		pcptr mStart = 0;
		pcptr mEnd = 0;
	};

	NOD() Text operator + (const Text&, const Text&);

	template<class T>
	NOD() Text operator + (const T&, const Text&) requires (!IsText<T>);

	template<class T>
	NOD() Text operator + (const Text&, const T&) requires (!IsText<T>);

} // namespace Langulus::Anyness

#include "Text.inl"