///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
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
	///	You can always ReinterpretAs a statically optimized equivalent for	
	/// the cost of one runtime type check.												
	/// (all Any variants are binary-compatible)											
	///																								
	class Any : public Block {
		LANGULUS(DEEP) true;
		LANGULUS_BASES(Block);
	public:
		template<CT::Data T>
		friend class TAny;

		constexpr Any() noexcept = default;
		Any(const Any&);
		Any(Any&&) noexcept;
		
		Any(const Block&);
		Any(Block&&);
		
		Any(Disowned<Any>&&) noexcept;
		Any(Abandoned<Any>&&) noexcept;		

		Any(Disowned<Block>&&) noexcept;
		Any(Abandoned<Block>&&) noexcept;

		template<CT::CustomData T>
		Any(const T&);
		template<CT::CustomData T>
		Any(T&);
		template<CT::CustomData T>
		Any(T&&);

		~Any();
	
		Any& operator = (const Any&);
		Any& operator = (Any&&);
	
		template<CT::Data T>
		Any& operator = (const T&);
		template<CT::Data T>
		Any& operator = (T&);
		template<CT::Data T>
		Any& operator = (T&&);

	public:
		NOD() static Any FromMeta(DMeta, const DataState& = {}) noexcept;
		NOD() static Any FromBlock(const Block&, const DataState& = {}) noexcept;
		template<CT::Data T>
		NOD() static Any From(const DataState& = {}) noexcept;

		template<CT::Data... LIST>
		NOD() static Any Wrap(LIST&&...);
		template<CT::Data HEAD, CT::Data... TAIL>
		NOD() static Any WrapCommon(HEAD&&, TAIL&&...);

		void Clear();
		void Reset();
		NOD() Any Clone() const;

		using Block::Swap;
		void Swap(Any&) noexcept;

		NOD() Any Crop(const Offset&, const Count&) const;
		NOD() Any Crop(const Offset&, const Count&);

		template<CT::Data T>
		Any& operator << (const T&);
		template<CT::Decayed T>
		Any& operator << (T&&);
	
		template<CT::Data T>
		Any& operator >> (const T&);
		template<CT::Decayed T>
		Any& operator >> (T&&);

		template<CT::Data T>
		Any& operator <<= (const T&);
		template<CT::Decayed T>
		Any& operator <<= (T&&);

		template<CT::Data T>
		Any& operator >>= (const T&);
		template<CT::Decayed T>
		Any& operator >>= (T&&);

	protected:
		constexpr void ResetState() noexcept;
	};

} // namespace Langulus::Anyness

#include "Any.inl"
