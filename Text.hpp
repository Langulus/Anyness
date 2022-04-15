#pragma once
#include "TAny.hpp"

namespace Langulus::Anyness
{
	
	///																								
	///	COUNT-TERMINATED TEXT WRAPPER														
	///																								
	///	Convenient wrapper for UTF strings												
	///																								
	class Text : public Block {
	public:
		using Letter = char8_t;
		
		Text();
		Text(const Text&);
		Text(Text&&) noexcept = default;

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
		Text(const T*, Count) requires Character<T>;
		template<Dense T>
		Text(const T&) requires Number<T>;
		template<Dense T>
		Text(const T&) requires StaticallyConvertible<T, Text>;
		template<Dense T>
		Text(const T*) requires StaticallyConvertible<T, Text>;

		~Text();

	public:
		NOD() Text Clone() const;
		NOD() Text Terminate() const;
		NOD() Text Lowercase() const;
		NOD() Text Uppercase() const;
		NOD() Text Crop(Offset, Count) const;
		NOD() Text Strip(Letter) const;
		Text& Remove(Offset, Count);
		void Clear() noexcept;
		void Reset();
		Text Extend(Count);

		NOD() constexpr operator Token () const noexcept;

		NOD() TAny<char16_t> Widen16() const;
		NOD() TAny<char32_t> Widen32() const;

		NOD() Letter* GetRaw() noexcept;
		NOD() const Letter* GetRaw() const noexcept;
		NOD() Letter* GetRawEnd() noexcept;
		NOD() const Letter* GetRawEnd() const noexcept;

		Hash GetHash() const;

		NOD() Count GetLineCount() const noexcept;

		Text& operator = (const Text&);
		Text& operator = (Text&&) noexcept;

		bool operator == (const Text&) const noexcept;
		bool operator != (const Text&) const noexcept;

		NOD() const Letter& operator[] (Offset) const;
		NOD() Letter& operator[] (Offset);

		bool CompareLoose(const Text&) const noexcept;
		Count Matches(const Text&) const noexcept;
		NOD() Count MatchesLoose(const Text&) const noexcept;

		RANGED_FOR_INTEGRATION(Text, Letter);

		NOD() bool FindOffset(const Text&, Offset&) const;
		NOD() bool FindOffsetReverse(const Text&, Offset&) const;
		NOD() bool Find(const Text&) const;
		NOD() bool FindWild(const Text&) const;

		template<class T>
		Text& operator += (const T&);

		template<class T>
		friend NOD() Text operator + (const T&, const Text&);

		template<class T>
		NOD() friend Text operator + (const Text&, const T&);

	};

	/// Compile time check for text items													
	template<class T>
	concept IsText = pcHasBase<T, Text>;

} // namespace Langulus::Anyness

#include "Text.inl"
