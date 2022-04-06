#pragma once
#include "inner/Block.hpp"

namespace Langulus::Anyness
{

	///																								
	/// A piception spin-off of std::any													
	/// Can contain any number of copy constructible or movable type				
	/// Additionally extended to safely contain sparse data							
	///																								
	class LANGULUS_MODULE(Anyness) Any : public Block {
		REFLECT(Any);
	public:
		template<class T>
		static constexpr bool NotCustom = 
			Sparse<T> || (!Same<T, Any> && !Same<T, Block>);

		constexpr Any() noexcept {}

		Any(const Any&);
		PC_LEAKSAFETY Any(Any&&) noexcept;
		Any(const Block&);
		Any(Block&&);

		template<RTTI::ReflectedData T>
		Any(T&&) requires (Any::NotCustom<T>);
		template<RTTI::ReflectedData T>
		Any(const T&) requires (Any::NotCustom<T>);
		template<RTTI::ReflectedData T>
		Any(T&) requires (Any::NotCustom<T>);

		~Any();

	public:
		NOD() static Any FromStateOf(const Block&) noexcept;
		NOD() static Any From(DataID, const DState& = {}) noexcept;
		NOD() static Any From(DMeta, const DState& = {}) noexcept;
		NOD() static Any From(const Block&, const DState& = {}) noexcept;
		template<RTTI::ReflectedData T = void>
		NOD() static Any From(const DState& = {}) noexcept;

		template<RTTI::ReflectedData... Args>
		NOD() static Any Wrap(Args...);
		template<RTTI::ReflectedData T, RTTI::ReflectedData... Args>
		NOD() static Any WrapOne(const T&, Args...);
		template<RTTI::ReflectedData... Args>
		NOD() static Any WrapOr(Args...);
		template<RTTI::ReflectedData T, RTTI::ReflectedData... Args>
		NOD() static Any WrapOneOr(const T&, Args...);

		Any& operator = (const Any&);
		Any& operator = (Any&&);
		Any& operator = (const Block&);
		Any& operator = (Block&&);

		template<RTTI::ReflectedData T>
		Any& operator = (const T&) requires (Any::NotCustom<T>);
		template<RTTI::ReflectedData T>
		Any& operator = (T&) requires (Any::NotCustom<T>);
		template<RTTI::ReflectedData T>
		Any& operator = (T&&) requires (Any::NotCustom<T>);

		Any& MakeMissing();
		Any& MakeStatic();
		Any& MakeConstant();
		Any& MakeTypeConstrained();
		Any& MakeOr();
		Any& MakeAnd();
		Any& MakeLeft();
		Any& MakeRight();

		void Clear();
		void Reset();
		NOD() Any Clone() const;

		using Block::Swap;
		void Swap(Any&) noexcept;

		NOD() Any Crop(pcptr, pcptr) const;
	};

} // namespace Langulus::Anyness

#include "Any.inl"
