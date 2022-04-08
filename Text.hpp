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

		explicit Text(const Token&);
		explicit Text(const Byte&);
		explicit Text(const Exception&);
		explicit Text(const Index&);
		explicit Text(const Meta&);

		template<Dense T>
		explicit Text(const T&) requires Character<T>;
		template<Dense T>
		explicit Text(const T*) requires Character<T>;
		template<Dense T>
		explicit Text(const T*, Count) requires Character<T>;
		template<Dense T>
		explicit Text(const T&) requires Number<T>;
		template<Dense T>
		explicit Text(const T&) requires StaticallyConvertible<T, Text>;
		template<Dense T>
		explicit Text(const T*) requires StaticallyConvertible<T, Text>;

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

		NOD() Text Widen16() const;
		NOD() Text Widen32() const;
		NOD() Text Widen() const;

		NOD() char8_t* GetRaw() noexcept;
		NOD() const char8_t* GetRaw() const noexcept;
		NOD() char8_t* GetRawEnd() noexcept;
		NOD() const char8_t* GetRawEnd() const noexcept;

		Hash GetHash() const;

		NOD() Count GetLineCount() const noexcept;

		Text& operator = (const Text&);
		Text& operator = (Text&&) noexcept;

		bool operator == (const Text&) const noexcept;
		bool operator != (const Text&) const noexcept;

		NOD() const char& operator[] (Offset) const;
		NOD() char& operator[] (Offset);

		bool CompareLoose(const Text&) const noexcept;
		Count Matches(const Text&) const noexcept;
		NOD() Count MatchesLoose(const Text&) const noexcept;

		RANGED_FOR_INTEGRATION(Text, char8_t);

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
