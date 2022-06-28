///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Bytes.hpp"

namespace Langulus::Anyness
{

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
		Text(Text&&) noexcept = default;

		Text(Disowned<Text>&&) noexcept;
		Text(Abandoned<Text>&&) noexcept;

		Text(const Token&);
		Text(const Byte&);
		Text(const Exception&);
		Text(const Index&);
		Text(const RTTI::Meta&);

		Text(const Letter*, const Count&);
		Text(Disowned<const Letter*>&&, const Count&);

		template<Count C>
		Text(const Letter(&)[C]);

		explicit Text(const Letter*);
		explicit Text(Disowned<const Letter*>&&);

		Text(const Letter&);

		template<CT::Dense T>
		Text(const T&) requires CT::Number<T>;

		Text& operator = (const Text&);
		Text& operator = (Text&&) noexcept;

		Text& operator = (Disowned<Text>&&);
		Text& operator = (Abandoned<Text>&&) noexcept;

	public:
		NOD() Hash GetHash() const;
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
		bool operator == (const Letter*) const noexcept;
		bool operator == (::std::nullptr_t) const noexcept;

		NOD() bool FindOffset(const Text&, Offset&) const;
		NOD() bool FindOffsetReverse(const Text&, Offset&) const;
		NOD() bool Find(const Text&) const;
		NOD() bool FindWild(const Text&) const;

		template<class RHS>
		Text& operator += (const RHS&);
		template<class RHS>
		NOD() Text operator + (const RHS&) const;
	};


	/// Text container specialized for logging											
	/// Serializing to text might produce a lot of unncessesary text, so this	
	/// differentiation is quite handy														
	class Debug : public Text {
		using Text::Text;
	};

} // namespace Langulus::Anyness

#include "Text.inl"
