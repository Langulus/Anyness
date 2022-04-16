#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{
	
	using Letter = char8_t;


	///																								
	///	COUNT-TERMINATED TEXT WRAPPER														
	///																								
	/// Convenient wrapper for UTF8 strings												
	///																								
	class Text : public TAny<Letter> {
	public:
		Text() = default;
		Text(const Text&);
		Text(Text&&) noexcept = default;

		Text(const Disowned<Text>&) noexcept;
		Text(Abandoned<Text>&&) noexcept;

		Text(const Token&);
		Text(const Byte&);
		Text(const Exception&);
		Text(const Index&);
		Text(const Meta&);

		template<Dense T>
		Text(const T&) requires Character<T>;
		template<Dense T>
		Text(const T*) requires Character<T>;
		template<Dense T>
		Text(const T*, const Count&) requires Character<T>;
		template<Dense T>
		Text(const T&) requires Number<T>;
		template<Dense T>
		Text(const T&) requires StaticallyConvertible<T, Text>;
		template<Dense T>
		Text(const T*) requires StaticallyConvertible<T, Text>;

		Text& operator = (const Text&);
		Text& operator = (Text&&) noexcept;

		Text& operator = (const Disowned<Text>&);
		Text& operator = (Abandoned<Text>&&) noexcept;

	public:
		NOD() Text Clone() const;
		NOD() Text Terminate() const;
		NOD() Text Lowercase() const;
		NOD() Text Uppercase() const;
		NOD() Text Crop(const Offset&, const Count&) const;
		NOD() Text Crop(const Offset&, const Count&);
		NOD() Text Strip(Letter) const;
		Text& Remove(const Offset&, const Count&);
		Text Extend(const Count&);

		NOD() constexpr operator Token () const noexcept;

		NOD() TAny<char16_t> Widen16() const;
		NOD() TAny<char32_t> Widen32() const;

		NOD() Count GetLineCount() const noexcept;

		bool operator == (const Text&) const noexcept;
		bool operator != (const Text&) const noexcept;

		NOD() bool Compare(const Text&) const noexcept;
		NOD() bool CompareLoose(const Text&) const noexcept;
		NOD() Count Matches(const Text&) const noexcept;
		NOD() Count MatchesLoose(const Text&) const noexcept;

		NOD() bool FindOffset(const Text&, Offset&) const;
		NOD() bool FindOffsetReverse(const Text&, Offset&) const;
		NOD() bool Find(const Text&) const;
		NOD() bool FindWild(const Text&) const;

		template<class T>
		Text& operator += (const T&);
		template<class T>
		NOD() Text operator + (const T&) const;
		template<class T>
		friend Text operator + (const T&, const Text&) requires NotSame<T, Text>;
	};

} // namespace Langulus::Anyness

#include "Text.inl"
