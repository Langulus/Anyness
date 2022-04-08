#pragma once
#include "inner/Block.hpp"

namespace Langulus::Anyness
{
	
	///																								
	///	COUNT-TERMINATED TEXT WRAPPER														
	///																								
	///	Convenient wrapper for UTF strings												
	///																								
	class Text : public Inner::Block {
	public:
		Text();
		Text(const Text&);
		Text(Text&&) noexcept = default;

		Text(const char8_t*, Count);
		Text(const char16_t*, Count);
		Text(const char32_t*, Count);

		explicit Text(const char8_t&);
		explicit Text(const char16_t&);
		explicit Text(const char32_t&);

		explicit Text(const Token&);
		explicit Text(const Byte&);
		explicit Text(const Exception&);
		explicit Text(const Index&);
		explicit Text(const Meta&);

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
		NOD() Text Crop(Offset, Count) const;
		NOD() Text Strip(char) const;
		Text& Remove(Offset, Count);
		void Clear() noexcept;
		void Reset();

		Text Extend(Count);

		NOD() constexpr operator Token () const noexcept;

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
		Text& operator = (Text&&) noexcept;

		bool operator == (const Text&) const noexcept;
		bool operator != (const Text&) const noexcept;

		NOD() const char& operator[] (Offset) const;
		NOD() char& operator[] (Offset);

		bool CompareLoose(const Text&) const noexcept;
		Count Matches(const Text&) const noexcept;
		NOD() Count MatchesLoose(const Text&) const noexcept;

		RANGED_FOR_INTEGRATION(Text, char8_t)

		NOD() bool FindOffset(const Text&, Offset&) const;
		NOD() bool FindOffsetReverse(const Text&, Offset&) const;
		NOD() bool Find(const Text&) const;
		NOD() bool FindWild(const Text&) const;

		template<class ANYTHING>
		Text& operator += (const ANYTHING&);
	};

	/// Compile time check for text items													
	template<class T>
	concept IsText = pcHasBase<T, Text>;

} // namespace Langulus::Anyness

#include "Text.inl"
