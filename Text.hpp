///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Bytes.hpp"
#include <string>

namespace Langulus::Anyness
{

	///																								
	///	COUNT-TERMINATED TEXT WRAPPER														
	///																								
	/// Convenient wrapper for UTF8 strings												
	///																								
	class Text : public TAny<Letter> {
		LANGULUS(DEEP) false;
		LANGULUS_BASES(A::Text, TAny<Letter>);

	public:
		Text() = default;
		Text(const Text&);
		Text(Text&&) noexcept;

		Text(const TAny&);
		Text(TAny&&) noexcept;

		/*template<CT::Deep T>
		Text(const T&) = delete;*/

		Text(Disowned<Text>&&) noexcept;
		Text(Abandoned<Text>&&) noexcept;
		Text(Disowned<TAny>&&) noexcept;
		Text(Abandoned<TAny>&&) noexcept;

		Text(const Token&);
		Text(const Exception&);
		Text(const RTTI::Meta&);

		Text(const Letter*, const Count&) noexcept;
		Text(Disowned<const Letter*>&&, const Count&) noexcept;

		template<Count C>
		Text(const Letter(&)[C]);

		explicit Text(const Letter*) noexcept;
		explicit Text(Disowned<const Letter*>&&) noexcept;

		Text(const Letter&);

		template<CT::Dense T>
		explicit Text(const T&) requires CT::Number<T>;

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
		NOD() Text Crop(Offset, Count) const;
		NOD() Text Crop(Offset, Count);
		NOD() Text Strip(Letter) const;
		Text& Remove(Offset, Count);
		Text Extend(Count);

		NOD() operator Token () const noexcept;

		#if LANGULUS_FEATURE(UTFCPP)
			NOD() TAny<char16_t> Widen16() const;
			NOD() TAny<char32_t> Widen32() const;
		#endif

		NOD() Count GetLineCount() const noexcept;

		using CompatibleStdString = ::std::basic_string<Letter, ::std::char_traits<Letter>, ::std::allocator<Letter>>;
		bool operator == (const CompatibleStdString&) const noexcept;
		using CompatibleStdStringView = ::std::basic_string_view<Letter>;
		bool operator == (const CompatibleStdStringView&) const noexcept;
		bool operator == (const Text&) const noexcept;
		bool operator == (const Letter*) const noexcept;

		NOD() bool FindOffset(const Text&, Offset&) const;
		NOD() bool FindOffsetReverse(const Text&, Offset&) const;
		NOD() bool Find(const Text&) const;
		NOD() bool FindWild(const Text&) const;

		///																							
		///	Concatenation																		
		///																							
		NOD() Text operator + (const Text&) const;
		NOD() Text operator + (Text&&) const;
		NOD() Text operator + (Disowned<Text>&&) const;
		NOD() Text operator + (Abandoned<Text>&&) const;

		Text& operator += (const Text&);
		Text& operator += (Text&&);
		Text& operator += (Disowned<Text>&&);
		Text& operator += (Abandoned<Text>&&);
	};


	/// Text container specialized for logging											
	/// Serializing to text might produce a lot of unncessesary text, so this	
	/// differentiation is quite handy														
	class Debug : public Text {
		LANGULUS_BASES(Text);
	public:
		Debug() = default;
		Debug(const Text&);
		Debug(Text&&) noexcept;
		Debug(Disowned<Debug>&& o) noexcept;
		Debug(Abandoned<Debug>&& o) noexcept;
	};

} // namespace Langulus::Anyness


namespace Langulus::CT
{

	/// Concept for differentiating managed Anyness Text types						
	template<class T>
	concept Text = DerivedFrom<T, ::Langulus::Anyness::Text>;

	/// Concept for differentiating managed Anyness Text types						
	template<class T>
	concept NotText = !Text<T>;

} // namespace Langulus::CT


namespace Langulus
{

	Anyness::Text operator "" _text(const char*, ::std::size_t);

} // namespace Langulus

#include "Text.inl"
