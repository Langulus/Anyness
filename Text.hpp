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
		LANGULUS(DEEP) false;		
	public:
		using TAny::TAny;

		Text() = default;

		Text(const Text&);
		Text(Text&);
		Text(Text&&) noexcept = default;

		Text(const Disowned<Text>&) noexcept;
		Text(Abandoned<Text>&&) noexcept;

		Text(const Token&);
		Text(const Byte&);
		Text(const Exception&);
		Text(const Index&);
		Text(const Meta&);

		Text(const char*, const Count&);
		template<Count C>
		Text(const char(&)[C]);
		explicit Text(const char*);

		template<Langulus::IsDense T>
		Text(const T*, const Count&) requires IsCharacter<T>;
		template<Langulus::IsDense T, Count C>
		Text(const T(&)[C]) requires IsCharacter<T>;

		template<Langulus::IsDense T>
		Text(const T&) requires IsCharacter<T>;
		template<Langulus::IsDense T>
		explicit Text(const T*) requires IsCharacter<T>;
		template<Langulus::IsDense T>
		Text(const T&) requires IsNumber<T>;

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

		NOD() operator Token () const noexcept;

		#if LANGULUS_FEATURE(UTFCPP)
			NOD() TAny<char16_t> Widen16() const;
			NOD() TAny<char32_t> Widen32() const;
		#endif

		NOD() Count GetLineCount() const noexcept;

		bool operator == (const Text&) const noexcept;
		bool operator != (const Text&) const noexcept;

		NOD() bool FindOffset(const Text&, Offset&) const;
		NOD() bool FindOffsetReverse(const Text&, Offset&) const;
		NOD() bool Find(const Text&) const;
		NOD() bool FindWild(const Text&) const;

		template<class RHS>
		Text& operator += (const RHS&);
		template<class RHS>
		NOD() Text operator + (const RHS&) const;
	};

} // namespace Langulus::Anyness

#include "Text.inl"
