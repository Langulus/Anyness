#pragma once
#include "Block.hpp"

namespace Langulus::Anyness
{

	///																								
	///	Any																						
	///																								
	///	More of an equivalent to std::vector, instead of std::any, since		
	/// it can contain any number of similarly-typed type-erased elements.		
	/// It gracefully wraps sparse and dense arrays, keeping track of static	
	/// and constant data blocks.																
	///	For a faster statically-optimized equivalent of this, use TAny			
	///																								
	class Any : public Block {
	public:
		template<ReflectedData T>
		friend class TAny;

		template<class T>
		static constexpr bool NotCustom = Sparse<T> || (!Same<T,Any> && !Same<T,Block>);

		constexpr Any() noexcept {}

		Any(const Any&);
		Any(Any&);
		Any(Any&&) noexcept;
		
		Any(const Block&);
		Any(Block&);
		Any(Block&&);

		template<ReflectedData T>
		Any(T&&) requires (Any::NotCustom<T>);
		template<ReflectedData T>
		Any(const T&) requires (Any::NotCustom<T>);
		template<ReflectedData T>
		Any(T&) requires (Any::NotCustom<T>);

		~Any();

		Any& operator = (const Any&);
		Any& operator = (Any&);
		Any& operator = (Any&&);
		
		Any& operator = (const Block&);
		Any& operator = (Block&);
		Any& operator = (Block&&);

		template<ReflectedData T>
		Any& operator = (const T&) requires (Any::NotCustom<T>);
		template<ReflectedData T>
		Any& operator = (T&) requires (Any::NotCustom<T>);
		template<ReflectedData T>
		Any& operator = (T&&) requires (Any::NotCustom<T>);

	public:
		NOD() static Any From(DMeta, const DataState& = {}) noexcept;
		NOD() static Any FromBlock(const Block&, const DataState& = {}) noexcept;
		template<ReflectedData T>
		NOD() static Any From(const DataState& = {}) noexcept;

		template<ReflectedData... Args>
		NOD() static Any Wrap(Args&&...);
		template<ReflectedData... Args>
		NOD() static Any WrapOne(Args&&...);
		template<ReflectedData... Args>
		NOD() static Any WrapOr(Args&&...);
		template<ReflectedData... Args>
		NOD() static Any WrapOneOr(Args&&...);

		void Clear();
		void Reset();
		NOD() Any Clone() const;

		using Block::Swap;
		void Swap(Any&) noexcept;

		NOD() Any Crop(const Offset&, const Count&) const;
		NOD() Any Crop(const Offset&, const Count&);

	protected:
		void ResetState();
	};

} // namespace Langulus::Anyness

#include "Any.inl"
