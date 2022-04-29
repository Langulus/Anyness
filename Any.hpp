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
		LANGULUS(DEEP) true;		
	public:
		template<ReflectedData T>
		friend class TAny;

		constexpr Any() noexcept = default;
		Any(const Any&);
		Any(Any&);
		Any(Any&&) noexcept;
		
		Any(const Block&);
		Any(Block&);
		Any(Block&&);
		
		Any(Disowned<Any>&&) noexcept;
		Any(Abandoned<Any>&&) noexcept;		

		template<IsCustom T>
		Any(T&&);
		template<IsCustom T>
		Any(const T&);
		template<IsCustom T>
		Any(T&);

		~Any();

		/*Any& operator = (const Any&);
		Any& operator = (Any&);
		Any& operator = (Any&&);
		
		Any& operator = (const Block&);
		Any& operator = (Block&);
		Any& operator = (Block&&);

		Any& operator = (Disowned<Any>&&);
		Any& operator = (Abandoned<Any>&&) noexcept;*/
		
		template<class T>
		Any& operator = (const T&);
		template<class T>
		Any& operator = (T&);
		template<class T>
		Any& operator = (T&&);

	public:
		NOD() static Any FromMeta(DMeta, const DataState& = {}) noexcept;
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
