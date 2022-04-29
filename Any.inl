#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	/// Copy construction via Any - does a shallow copy and references 			
	///	@param other - the container to shallow-copy									
	inline Any::Any(const Any& other) 
		: Block {static_cast<const Block&>(other)} {
		Keep();
	}

	/// Copy construction via Any - does a shallow copy and references 			
	///	@param other - the container to shallow-copy									
	inline Any::Any(Any& other) 
		: Block {static_cast<Block&>(other)} {
		Keep();
	}

	/// Construct by moving another container												
	///	@param other - the container to move											
	inline Any::Any(Any&& other) noexcept
		: Block {Forward<Block>(other)} {
		other.ResetMemory();
		other.ResetState();
	}

	/// Copy construct via Block - does a shallow copy, and references (const)	
	///	@param other - the container to shallow-copy									
	inline Any::Any(const Block& other) 
		: Block {other} {
		Keep();
	}
	
	/// Copy construct via Block - does a shallow copy, and references			
	///	@param other - the container to shallow-copy									
	inline Any::Any(Block& other) 
		: Block {other} {
		Keep();
	}

	/// Move construction via Block (well, not actually)								
	///	@attention since we are not aware if that block is referenced or not	
	///				  we reference it just in case, and we also do not reset		
	///              'other' to avoid memory leaks										
	///	@param other - the block to shallow-copy										
	inline Any::Any(Block&& other) 
		: Block {static_cast<Block&>(other)} {
		Keep();
	}
	
	/// Same as shallow-copy but doesn't reference anything							
	///	@param other - the block to shallow-copy										
	inline Any::Any(Disowned<Any>&& other) noexcept
		: Block {static_cast<const Block&>(other.mValue)} {}	
	
	/// Same as shallow-move but doesn't fully reset other, saving some			
	/// instructions																				
	///	@param other - the block to shallow-copy										
	inline Any::Any(Abandoned<Any>&& other) noexcept
		: Block {Forward<Block>(other.mValue)} {
		other.mValue.mEntry = nullptr;
	}

	/// Destruction																				
	inline Any::~Any() {
		Free();
	}

	/// Construct by moving a dense value of non-block type							
	///	@param other - the dense value to forward and emplace						
	template <IsCustom T>
	Any::Any(T&& other) {
		SetType<T, false>();
		Emplace<T, false>(Forward<T>(other));
	}

	/// Construct by copying/referencing value of non-block type					
	///	@param other - the dense value to shallow-copy								
	template <IsCustom T>
	Any::Any(T& other) {
		SetType<T, false>();
		Insert<T, false>(&other, 1);
	}

	/// Create an empty Any from a dynamic type and state								
	///	@param type - type of the container												
	///	@param state - optional state of the container								
	///	@return the new any																	
	inline Any Any::FromMeta(DMeta type, const DataState& state) noexcept {
		return Any {Block{state, type}};
	}

	/// Create an empty Any by copying type and state of a block					
	///	@param type - type of the container												
	///	@param state - additional state of the container							
	///	@return the new any																	
	inline Any Any::FromBlock(const Block& type, const DataState& state) noexcept {
		return Any::FromMeta(type.GetType(), type.GetUnconstrainedState() + state);
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

	/// Assign by shallow-copying something constant									
	///	@param value - the value to copy													
	template<class T>
	Any& Any::operator = (const T& other) {
		operator = (const_cast<T&>(other));
		MakeConstant();
		return *this;
	}

	/// Assign by shallow-copying something mutable										
	///	@param value - the value to copy													
	template<class T>
	Any& Any::operator = (T& other) {
		if constexpr (IsSame<T, Any>) {
			if (IsTypeConstrained() && !InterpretsAs(other.mType)) {
				throw Except::Copy(Logger::Error()
					<< "Bad shallow-copy-assignment for Any: from "
					<< GetToken() << " to " << other.GetToken());
			}
	
			other.Keep();
			Free();
			Block::operator = (other);
			return *this;
		}
		else if constexpr (IsSame<T, Block>) {
			return operator = (Any {other});			
		}
		else {
			const auto meta = MetaData::Of<Decay<T>>();
			if (IsTypeConstrained() && !InterpretsAs(meta)) {
				throw Except::Copy(Logger::Error()
					<< "Bad shallow-copy-assignment for Any: from "
					<< GetToken() << " to " << meta->mToken);
			}
	
			Reset();
			Block::operator << <T>(other);
			return *this;
		}
	}

	/// Assign by moving something															
	///	@param value - the value to move													
	template<class T>
	Any& Any::operator = (T&& other) {
		if constexpr (IsSame<T, Any>) {
			if (IsTypeConstrained() && !InterpretsAs(other.mType)) {
				throw Except::Copy(Logger::Error()
					<< "Bad shallow-copy-assignment for Any: from "
					<< GetToken() << " to " << other.GetToken());
			}
	
			Free();
			Block::operator = (Forward<T>(other));
			return *this;
		}
		else if constexpr (IsSame<T, Block>) {
			return operator = (Any {Forward<T>(other)});			
		}
		else {
			const auto meta = MetaData::Of<Decay<T>>();
			if (IsTypeConstrained() && !InterpretsAs(meta)) {
				throw Except::Copy(Logger::Error()
					<< "Bad shallow-copy-assignment for Any: from "
					<< GetToken() << " to " << meta->mToken);
			}
	
			Reset();
			Block::operator << <T>(Forward<T>(other));
			return *this;
		}
	}

} // namespace Langulus::Anyness
