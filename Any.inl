#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	/// Copy construction via Any - does a shallow copy and references 			
	///	@param other - the container to shallow-copy									
	Any::Any(const Any& other) 
		: Block {other} {
		Keep();
	}

	/// Construct by moving another container												
	///	@param other - the container to move											
	Any::Any(Any&& other) noexcept
		: Block {Forward<Block>(other)} {
		other.ResetMemory();
		other.ResetState();
	}

	/// Copy construction via Block - does a shallow copy, and references		
	///	@param other - the container to shallow-copy									
	Any::Any(const Block& other) 
		: Block {other} {
		Keep();
	}

	/// Move construction via Block (well, not actually)								
	///	@attention since we are not aware if that block is referenced or not	
	///				  we reference it just in case, and we also do not reset		
	///              'other' to avoid memory leaks										
	///	@param other - the block to shallow-copy										
	Any::Any(Block&& other) 
		: Block {static_cast<const Block&>(other)} {
		Keep();
	}

	/// Destruction																				
	Any::~Any() {
		Free();
	}

	/// Construct by moving a dense value of non-block type							
	///	@param other - the dense value to forward and emplace						
	template <ReflectedData T>
	Any::Any(T&& other) requires (Any::NotCustom<T>) {
		SetType<T, false>();
		Emplace<T, false>(Forward<T>(other));
	}

	/// Construct by copying/referencing value of non-block type					
	///	@param other - the dense value to shallow-copy								
	template <ReflectedData T>
	Any::Any(const T& other) requires (Any::NotCustom<T>) {
		SetType<T, false>();
		Insert<T, false>(&other, 1);
	}

	/// Construct by copying/referencing value of non-block type					
	/// Required to not move other by mistake												
	///	@param other - the dense value to shallow-copy								
	template <ReflectedData T>
	Any::Any(T& other) requires (Any::NotCustom<T>)
		: Any {const_cast<const T&>(other)} {}

	/// Create an empty Any from a dynamic type and state								
	///	@param type - type of the container												
	///	@param state - optional state of the container								
	///	@return the new any																	
	inline Any Any::From(DMeta type, const DataState& state) noexcept {
		return Any {Block{state, type}};
	}

	/// Create an empty Any by copying type and state of a block					
	///	@param type - type of the container												
	///	@param state - additional state of the container							
	///	@return the new any																	
	inline Any Any::From(const Block& type, const DataState& state) noexcept {
		return Any::From(type.GetType(), type.GetUnconstrainedState() + state);
	}

	/// Create an empty Any from a static type and state								
	///	@param state - optional state of the container								
	///	@return the new any																	
	template<ReflectedData T>
	Any Any::From(const DataState& state) noexcept {
		return Any {Block{state, MetaData::Of<T>()}};
	}

	/// Pack any number of elements sequentially											
	///	@param elements - sequential elements											
	///	@returns the pack containing the data											
	template<ReflectedData... Args>
	Any Any::Wrap(Args&&... elements) {
		if constexpr (sizeof...(Args) == 0)
			return {};
		else {
			Any result;
			Any wrapped[] {Any{Forward<Args>(elements)}...};
			result.Allocate<Any>(sizeof...(Args));
			for (auto& it : wrapped)
				result.Emplace<Any>(Move(it));
			return result;
		}
	}

	/// Pack any number of elements sequentially											
	///	@param elements - sequential elements											
	///	@returns the pack containing the data											
	template<ReflectedData... Args>
	Any Any::WrapOne(Args&&... elements) {
		if constexpr (sizeof...(Args) == 0)
			return {};
		else {
			auto wrapped[] {Forward<Args>(elements)...};
			Any result {Any::From<Decay<decltype(wrapped)>>()};
			for (auto& it : wrapped)
				result.Emplace<Decay<decltype(wrapped)>>(Move(it));
			return result;
		}
	}

	/// Pack any number of elements sequentially	in an OR container				
	///	@param elements - sequential elements											
	///	@returns the pack containing the data											
	template<ReflectedData... Args>
	Any Any::WrapOr(Args&&... elements) {
		Any result {Wrap(Forward<Args>(elements)...)};
		result.MakeOr();
		return result;
	}

	/// Pack any number of elements sequentially in an OR container				
	///	@param elements - sequential elements											
	///	@returns the pack containing the data											
	template<ReflectedData... Args>
	Any Any::WrapOneOr(Args&&... elements) {
		Any result {WrapOne(Forward<Args>(elements)...)};
		result.MakeOr();
		return result;
	}

	/// Assign by shallow-copying some value different from Any						
	///	@param value - the value to copy													
	template<ReflectedData T>
	Any& Any::operator = (const T& value) requires (Any::NotCustom<T>) {
		return operator = (Any {value});
	}

	template<ReflectedData T>
	Any& Any::operator = (T& value) requires (Any::NotCustom<T>) {
		return operator = (const_cast<const T&>(value));
	}

	/// Assign by moving some value different from Any									
	///	@param value - the value to move													
	template<ReflectedData T>
	Any& Any::operator = (T&& value) requires (Any::NotCustom<T>) {
		return operator = (Any {Forward<T>(value)});
	}

} // namespace Langulus::Anyness
