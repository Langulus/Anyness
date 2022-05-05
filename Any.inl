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
		: Block {static_cast<Block&>(other)} {
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
		: Block {static_cast<Block&>(other.mValue)} {
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
		Emplace<T, false, Any>(Forward<T>(other));
	}

	/// Construct by copying/referencing value of non-block type					
	///	@param other - the dense value to shallow-copy								
	template <IsCustom T>
	Any::Any(T& other) {
		SetType<T, false>();
		Insert<T, false, Any>(&other, 1);
	}

	/// Create an empty Any from a dynamic type and state								
	///	@param type - type of the container												
	///	@param state - optional state of the container								
	///	@return the new any																	
	inline Any Any::FromMeta(DMeta type, const DataState& state) noexcept {
		return Any {Block {state, type}};
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
	template<ReflectedData... LIST>
	Any Any::Wrap(LIST&&... elements) {
		if constexpr (sizeof...(LIST) == 0)
			return {};
		else {
			Any result;
			Any wrapped[] {Any{Forward<LIST>(elements)}...};
			result.Allocate(sizeof...(LIST));
			result.SetType<Any>();
			for (auto& it : wrapped)
				result.Emplace<Any, false>(Move(it));
			return result;
		}
	}

	/// Pack any number of same-type elements sequentially							
	///	@param elements - sequential elements											
	///	@returns the pack containing the data											
	template<ReflectedData HEAD, ReflectedData... TAIL>
	Any Any::WrapCommon(HEAD&& head, TAIL&&... tail) {
		if constexpr (sizeof...(TAIL) == 0)
			return {};
		else {
			HEAD wrapped[] {Forward<HEAD>(head), Forward<TAIL>(tail)...};
			Any result {Any::From<HEAD>()};
			for (auto& it : wrapped)
				result.Emplace<HEAD>(Move(it));
			return result;
		}
	}

	/// Shallow-copy a constant container													
	inline Any& Any::operator = (const Any& other) {
		operator = (const_cast<Any&>(other));
		MakeConstant();
		return *this;
	}
	
	/// Shallow-copy a mutable container													
	inline Any& Any::operator = (Any& other) {
		// Just reference the memory of the other Any							
		if (IsTypeConstrained() && !CastsToMeta(other.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad shallow-copy-assignment for type-constrained Any: from "
				<< GetToken() << " to " << other.GetToken());
		}

		// First we reference, so that we don't lose the memory, in			
		// the rare case where memory is same in both containers				
		other.Keep();
		Free();
		Block::operator = (other);
		return *this;
	}
	
	/// Move a container																			
	inline Any& Any::operator = (Any&& other) {
		// Free this container and move the other onto it						
		if (IsTypeConstrained() && !CastsToMeta(other.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad shallow-copy-assignment for Any: from "
				<< GetToken() << " to " << other.GetToken());
		}

		Free();
		Block::operator = (other);
		other.ResetMemory();
		other.Reset();
		return *this;
	}

	/// Assign by shallow-copying something constant									
	///	@param value - the value to copy													
	///	@return a reference to this container											
	/*template<ReflectedData T>
	Any& Any::operator = (const T& other) {
		operator = (const_cast<T&>(other));
		MakeConstant();
		return *this;
	}*/

	/// Assign by shallow-copying something 												
	///	@param other - the value to copy													
	///	@return a reference to this container											
	template<ReflectedData T>
	Any& Any::operator = (T& other) {
		if constexpr (IsSame<T, Block>) {
			// Always reference a Block, by wrapping it in an Any				
			operator = (Any {other});			
		}
		else {
			const auto meta = MetaData::Of<Decay<T>>();
			if (IsTypeConstrained() && !CastsToMeta(meta)) {
				// Can't assign different type to a type-constrained Any		
				throw Except::Copy(Logger::Error()
					<< "Bad shallow-copy-assignment for type-constrained Any: from "
					<< GetToken() << " to " << meta->mToken);
			}
			else if (mEntry && mEntry->mReferences == 1 && meta->Is(mType)) {
				// Just destroy and reuse memory										
				// Even better - types match, so we know this container		
				// is filled with T too, therefore we can use statically		
				// optimized routines for destruction								
				CallKnownDestructors<T>();
				mCount = 0;
				InsertInner<T>(&other, 1, 0);
			}
			else {
				// Reset and allocate new memory										
				Reset();
				mType = meta;
				if constexpr (Langulus::IsSparse<T>)
					MakeSparse();
				AllocateInner<false>(1);
				InsertInner<T>(&other, 1, 0);
			}
		}

		return *this;
	}

	/// Assign by moving something															
	///	@param other - the value to move													
	///	@return a reference to this container											
	template<ReflectedData T>
	Any& Any::operator = (T&& other) {
		if constexpr (IsSame<T, Block>) {
			// Always reference a Block, by wrapping it in an Any				
			operator = (Any {Forward<T>(other)});
		}
		else {
			const auto meta = MetaData::Of<Decay<T>>();
			if (IsTypeConstrained() && !CastsToMeta(meta)) {
				// Can't assign different type to a type-constrained Any		
				throw Except::Copy(Logger::Error()
					<< "Bad shallow-copy-assignment for type-constrained Any: from "
					<< GetToken() << " to " << meta->mToken);
			}
			else if (mEntry && mEntry->mReferences == 1 && meta->Is(mType)) {
				// Types match, so we know this container is filled with T	
				// too, therefore we can use statically optimized routines	
				// for destruction														
				CallKnownDestructors<T>();
				mCount = 0;
				EmplaceInner<T>(Forward<T>(other), 0);
			}
			else {
				// Reset and allocate new memory										
				Reset();
				mType = meta;
				if constexpr (Langulus::IsSparse<T>)
					MakeSparse();
				AllocateInner<false>(1);
				EmplaceInner<T>(Forward<T>(other), 0);
			}
		}
		
		return *this;
	}
	
	/// Insert any data (including arrays) at the back									
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Any& Any::operator << (const T& other) {
		if constexpr (IsArray<T>)
			Insert<Decay<T>, true, Any>(other, ExtentOf<T>, Index::Back);
		else
			Insert<T, true, Any>(&other, 1, Index::Back);
		return *this;
	}

	/// Emplace any data at the back															
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<IsDecayed T>
	Any& Any::operator << (T&& other) {
		Emplace<T, true, Any>(Forward<T>(other), Index::Back);
		return *this;
	}

	/// Insert any data (including arrays) at the front								
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Any& Any::operator >> (const T& other) {
		if constexpr (IsArray<T>)
			Insert<Decay<T>, true, Any>(other, ExtentOf<T>, Index::Front);
		else
			Insert<T, true, Any>(&other, 1, Index::Front);
		return *this;
	}

	/// Emplace any data at the front														
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<IsDecayed T>
	Any& Any::operator >> (T&& other) {
		Emplace<T, true, Any>(Forward<T>(other), Index::Front);
		return *this;
	}

	/// Merge data (including arrays) at the back										
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Any& Any::operator <<= (const T& other) {
		if constexpr (IsArray<T>)
			Merge<Decay<T>, true, Any>(other, ExtentOf<T>, Index::Back);
		else
			Merge<T, true, Any>(&other, 1, Index::Back);
		return *this;
	}

	/// Merge data at the front																
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Any& Any::operator >>= (const T& other) {
		if constexpr (IsArray<T>)
			Merge<Decay<T>, true, Any>(other, ExtentOf<T>, Index::Front);
		else
			Merge<T, true, Any>(&other, 1, Index::Front);
		return *this;
	}

	/// Reset the container																		
	inline void Any::Reset() {
		Free();
		mRaw = nullptr;
		mCount = mReserved = 0;
		ResetState();
	}

	/// Reset container state																	
	inline void Any::ResetState() {
		if (IsTypeConstrained())
			Block::ResetState<true>();
		else
			Block::ResetState<false>();
	}

	/// Swap two container's contents														
	///	@param other - [in/out] the container to swap contents with				
	inline void Any::Swap(Any& other) noexcept {
		other = ::std::exchange(*this, Move(other));
	}

	/// Pick a constant region and reference it from another container			
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the container																
	inline Any Any::Crop(const Offset& start, const Count& count) const {
		return Any {Block::Crop(start, count)};
	}

	/// Pick a region and reference it from another container						
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the container																
	inline Any Any::Crop(const Offset& start, const Count& count) {
		return Any {Block::Crop(start, count)};
	}

} // namespace Langulus::Anyness
