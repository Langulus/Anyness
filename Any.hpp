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
	/// the cost of one runtime type check, because all Any variants are			
	/// binary-compatible.																		
	///																								
	class Any : public Block {
		LANGULUS(DEEP) true;
		LANGULUS_BASES(Block);
	public:
		template<CT::Data T>
		friend class TAny;
		friend class Block;

		///																							
		///	Construction																		
		///																							
		constexpr Any() noexcept = default;
		Any(const Any&);
		Any(Any&&) noexcept;

		template<CT::Deep T>
		Any(const T&) requires (CT::Dense<T> && !CT::Same<T, Any>);
		template<CT::Deep T>
		Any(T&) requires (CT::Dense<T> && !CT::Same<T, Any>);
		template<CT::Deep T>
		Any(T&&) requires (CT::Dense<T> && !CT::Same<T, Any>);

		template<CT::Deep T>
		constexpr Any(Disowned<T>&&) noexcept requires CT::Dense<T>;
		template<CT::Deep T>
		constexpr Any(Abandoned<T>&&) noexcept requires CT::Dense<T>;

		template<CT::Data T>
		Any(const T*, const T*);

		template<CT::CustomData T>
		Any(const T&);
		template<CT::CustomData T>
		Any(T&);
		template<CT::CustomData T>
		Any(T&&);

		template<CT::CustomData T>
		Any(Disowned<T>&&);
		template<CT::CustomData T>
		Any(Abandoned<T>&&);

		~Any();
	
		///																							
		///	Assignment																			
		///																							
		Any& operator = (const Any&);
		Any& operator = (Any&&);
	
		Any& operator = (Disowned<Any>&&);
		Any& operator = (Abandoned<Any>&&);
	
		template<CT::CustomData T>
		Any& operator = (const T&);
		template<CT::CustomData T>
		Any& operator = (T&);
		template<CT::CustomData T>
		Any& operator = (T&&);

		template<CT::CustomData T>
		Any& operator = (Disowned<T>&&) requires CT::Dense<T>;
		template<CT::CustomData T>
		Any& operator = (Abandoned<T>&&) requires CT::Dense<T>;

		///																							
		///	Comparison																			
		///																							
		using Block::operator ==;

	public:
		NOD() static Any FromMeta(DMeta, const DataState& = {}) noexcept;
		NOD() static Any FromBlock(const Block&, const DataState& = {}) noexcept;
		NOD() static Any FromState(const Block&, const DataState& = {}) noexcept;
		template<CT::Data T>
		NOD() static Any From(const DataState& = {}) noexcept;

		template<CT::Data... LIST>
		NOD() static Any Wrap(LIST&&...);
		template<CT::Data HEAD, CT::Data... TAIL>
		NOD() static Any WrapCommon(HEAD&&, TAIL&&...);

		void Clear();
		void Reset();
		constexpr void ResetState() noexcept;
		NOD() Any Clone() const;

		using Block::Swap;
		void Swap(Any&) noexcept;

		NOD() Any Crop(const Offset&, const Count&) const;
		NOD() Any Crop(const Offset&, const Count&);

		///																							
		///	Insertion																			
		///																							
		template<CT::Data T>
		Any& operator << (const T&);
		template<CT::Data T>
		Any& operator << (T&);
		template<CT::Data T>
		Any& operator << (T&&);
	
		template<CT::Data T>
		Any& operator >> (const T&);
		template<CT::Data T>
		Any& operator >> (T&);
		template<CT::Data T>
		Any& operator >> (T&&);

		template<CT::Data T>
		Any& operator <<= (const T&);
		template<CT::Data T>
		Any& operator <<= (T&);
		template<CT::Data T>
		Any& operator <<= (T&&);

		template<CT::Data T>
		Any& operator >>= (const T&);
		template<CT::Data T>
		Any& operator >>= (T&);
		template<CT::Data T>
		Any& operator >>= (T&&);
		
		///																							
		///	Concatenation																		
		///																							
		template<CT::Deep T>
		Any operator + (const T&) const requires CT::Dense<T>;
		template<CT::Deep T>
		Any operator + (T&) const requires CT::Dense<T>;
		template<CT::Deep T>
		Any operator + (T&&) const requires CT::Dense<T>;
		template<CT::Deep T>
		Any operator + (Disowned<T>&&) const requires CT::Dense<T>;
		template<CT::Deep T>
		Any operator + (Abandoned<T>&&) const requires CT::Dense<T>;

		template<CT::Deep T>
		Any& operator += (const T&) requires CT::Dense<T>;
		template<CT::Deep T>
		Any& operator += (T&) requires CT::Dense<T>;
		template<CT::Deep T>
		Any& operator += (T&&) requires CT::Dense<T>;
		template<CT::Deep T>
		Any& operator += (Disowned<T>&&) requires CT::Dense<T>;
		template<CT::Deep T>
		Any& operator += (Abandoned<T>&&) requires CT::Dense<T>;

	protected:
		template<class T>
		void PrepareForReassignment();
	};

} // namespace Langulus::Anyness

#include "Any.inl"
