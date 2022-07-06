///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

	/// Copy construction via Any - does a shallow copy and references 			
	///	@param other - the container to shallow-copy									
	inline Any::Any(const Any& other) 
		: Block {other} {
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

	/// Move construction via Block (well, not actually)								
	/// Since we are not aware if that block is referenced or not we reference	
	/// it just in case, and we also do not reset 'other' to avoid leaks			
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

	/// Construct by copying/referencing value of non-block type					
	///	@tparam T - the data type to push (deducible)								
	///	@param other - the dense value to shallow-copy								
	template <CT::CustomData T>
	Any::Any(const T& other) {
		SetType<T, false>();
		Insert<Any, true, false>(&other, 1);
	}

	/// This override is required to disambiguate automatically deduced T		
	/// Dimo, I know you want to remove this, but don't, said Dimo to himself	
	/// after actually deleting this function numerous times							
	template <CT::CustomData T>
	Any::Any(T& other)
		: Any {const_cast<const T&>(other)} {}

	/// Construct by moving a dense value of non-block type							
	///	@tparam T - the data type to push (deducible)								
	///	@param other - the dense value to forward and emplace						
	template <CT::CustomData T>
	Any::Any(T&& other) {
		SetType<T, false>();
		Insert<Any, true, false>(Forward<T>(other));
	}

	/// Construct by directly interfacing the memory of a dense non-block type	
	///	@tparam T - the data type to wrap (deducible)								
	///	@param other - the dense value to wrap											
	template <CT::CustomData T>
	Any::Any(Disowned<T>&& other) noexcept requires CT::Dense<T>
		: Block {
			DataState::Constrained, MetaData::Of<Decay<T>>(), 1,
			const_cast<const T*>(&other.mValue), nullptr
		} {}

	/// Construct by directly interfacing the memory of a dense non-block type	
	///	@tparam T - the data type to wrap (deducible)								
	///	@param other - the dense value to wrap											
	template <CT::CustomData T>
	Any::Any(Abandoned<T>&& other) noexcept requires CT::Dense<T>
		: Block {
			DataState::Member, MetaData::Of<Decay<T>>(), 1,
			&other.mValue, nullptr
		} {}

	/// Construct by inserting a sparse value of non-block type						
	///	@tparam T - the data type to push (deducible)								
	///	@param other - the disownes sparse value										
	template <CT::CustomData T>
	Any::Any(Disowned<T>&& other) requires CT::Sparse<T> {
		SetType<T, false>();
		Insert<Any, false, false>(&other.mValue, 1);
	}

	/// Construct by inserting a sparse value of non-block type						
	///	@tparam T - the data type to push (deducible)								
	///	@param other - the disownes sparse value										
	template <CT::CustomData T>
	Any::Any(Abandoned<T>&& other) requires CT::Sparse<T> {
		SetType<T, false>();
		Insert<Any, false, false>(Move(other.mValue));
	}

	/// Destruction																				
	inline Any::~Any() {
		Free();
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
	template<CT::Data T>
	Any Any::From(const DataState& state) noexcept {
		return Any {Block{state, MetaData::Of<Decay<T>>()}};
	}

	/// Pack any number of elements sequentially											
	///	@param elements - sequential elements											
	///	@returns the pack containing the data											
	template<CT::Data... LIST>
	Any Any::Wrap(LIST&&... elements) {
		if constexpr (sizeof...(LIST) == 0)
			return {};
		else {
			Any result;
			Any wrapped[] {Any{Forward<LIST>(elements)}...};
			result.Allocate(sizeof...(LIST));
			result.SetType<Any, false>();
			for (auto& it : wrapped)
				result.Insert<Any, true, false>(Move(it));
			return result;
		}
	}

	/// Pack any number of same-type elements sequentially							
	///	@param elements - sequential elements											
	///	@returns the pack containing the data											
	template<CT::Data HEAD, CT::Data... TAIL>
	Any Any::WrapCommon(HEAD&& head, TAIL&&... tail) {
		if constexpr (sizeof...(TAIL) == 0)
			return {};
		else {
			Deref<HEAD> wrapped[] {Forward<HEAD>(head), Forward<TAIL>(tail)...};
			Any result {Any::From<HEAD>()};
			for (auto& it : wrapped)
				result.Insert<Any, true, false>(Move(it));
			return result;
		}
	}
	
	/// Shallow-copy a mutable container													
	inline Any& Any::operator = (const Any& other) {
		// Just reference the memory of the other Any							
		if (IsTypeConstrained() && !CastsToMeta(other.mType)) {
			Throw<Except::Copy>(Logger::Error()
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
			Throw<Except::Copy>(Logger::Error()
				<< "Bad shallow-copy-assignment for Any: from "
				<< GetToken() << " to " << other.GetToken());
		}

		Free();
		Block::operator = (other);
		other.ResetMemory();
		other.ResetState();
		return *this;
	}

	/// Assign by shallow-copying anything 												
	///	@param other - the item to copy													
	///	@return a reference to this container											
	template<CT::CustomData T>
	Any& Any::operator = (const T& other) {
		static_assert(CT::NotAbandonedOrDisowned<T>, 
			"Copying an abandoned/disowned T is disallowed");

		if constexpr (CT::Same<T, Block>) {
			// Always reference a Block, by wrapping it in an Any				
			operator = (Any {other});			
		}
		else {
			const auto meta = MetaData::Of<Decay<T>>();
			if (IsTypeConstrained() && !CastsToMeta(meta)) {
				// Can't assign different type to a type-constrained Any		
				Throw<Except::Copy>(Logger::Error()
					<< "Bad shallow-copy-assignment for type-constrained Any: from "
					<< GetToken() << " to " << meta->mToken);
			}

			if (GetUses() == 1 && meta->Is(mType)) {
				// Just destroy and reuse memory										
				// Even better - types match, so we know this container		
				// is filled with T too, therefore we can use statically		
				// optimized routines for destruction								
				CallKnownDestructors<T>();
				mCount = 0;
			}
			else {
				// Reset and allocate new memory										
				Reset();
				mType = meta;
				if constexpr (CT::Sparse<T>)
					MakeSparse();
				AllocateInner<false>(1);
			}

			InsertInner<true>(&other, 1, 0);
		}

		return *this;
	}

	/// This override is required to disambiguate automatically deduced T		
	/// Dimo, I know you want to remove this, but don't, said Dimo to himself	
	template<CT::CustomData T>
	Any& Any::operator = (T& other) {
		return operator = (const_cast<const T&>(other));
	}

	/// Assign by moving anything																
	///	@param other - the item to move													
	///	@return a reference to this container											
	template<CT::CustomData T>
	Any& Any::operator = (T&& other) {
		if constexpr (CT::Same<T, Disowned<Any>> || CT::Same<T, Abandoned<Any>>) {
			// Move the other onto this if type is compatible					
			if (IsTypeConstrained() && !CastsToMeta(other.mValue.mType)) {
				Throw<Except::Copy>(Logger::Error()
					<< "Bad shallow-copy-assignment for Any: from "
					<< GetToken() << " to " << other.mValue.GetToken());
			}

			Free();
			Block::operator = (other.mValue);
			if constexpr (CT::Same<T, Abandoned<Any>>)
				other.mValue.mEntry = nullptr;
		}
		else if constexpr (CT::AbandonedOrDisowned<T>) {
			using InnerT = typename T::Type;

			if constexpr (CT::Same<InnerT, Block>) {
				// Never reference disowned/abandoned Block						
				operator = (Any {Forward<T>(other)});
			}
			else {
				const auto meta = MetaData::Of<Decay<InnerT>>();
				if (IsTypeConstrained() && !CastsToMeta(meta)) {
					// Can't assign different type to a type-constrained Any	
					Throw<Except::Copy>(Logger::Error()
						<< "Bad shallow-copy-assignment for type-constrained Any: from "
						<< GetToken() << " to " << meta->mToken);
				}

				if (GetUses() == 1 && meta->Is(mType)) {
					// Types match, so we know this container is filled with	
					// InnerT too, therefore we can use statically optimized 
					// routines	for destruction										
					CallKnownDestructors<InnerT>();
					mCount = 0;
				}
				else {
					// Reset and allocate new memory									
					Reset();
					mType = meta;
					if constexpr (CT::Sparse<InnerT>)
						MakeSparse();
					AllocateInner<false>(1);
				}

				if constexpr (CT::Abandoned<T>)
					InsertInner<false>(Move(other.mValue), 0);
				else
					InsertInner<false>(&other.mValue, 1, 0);
			}
		}
		else if constexpr (CT::Same<T, Block>) {
			// Always reference a Block, by wrapping it in an Any				
			operator = (Any {Forward<T>(other)});
		}
		else {
			const auto meta = MetaData::Of<Decay<T>>();
			if (IsTypeConstrained() && !CastsToMeta(meta)) {
				// Can't assign different type to a type-constrained Any		
				Throw<Except::Copy>(Logger::Error()
					<< "Bad shallow-copy-assignment for type-constrained Any: from "
					<< GetToken() << " to " << meta->mToken);
			}

			if (GetUses() == 1 && meta->Is(mType)) {
				// Types match, so we know this container is filled with T	
				// too, therefore we can use statically optimized routines	
				// for destruction														
				CallKnownDestructors<T>();
				mCount = 0;
			}
			else {
				// Reset and allocate new memory										
				Reset();
				mType = meta;
				if constexpr (CT::Sparse<T>)
					MakeSparse();
				AllocateInner<false>(1);
			}

			InsertInner<true>(Forward<T>(other), 0);
		}
		
		return *this;
	}
	
	/// Copy-insert an element (including arrays) at the back						
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator << (const T& other) {
		if constexpr (CT::Array<T>)
			Insert<Any, true, true>(SparseCast(other), ExtentOf<T>, Index::Back);
		else
			Insert<Any, true, true>(&other, 1, Index::Back);
		return *this;
	}

	/// Used to disambiguate from the && variant											
	/// Dimo, I know you want to remove this, but don't, said Dimo to himself	
	/// after actually deleting this function numerous times							
	template<CT::Data T>
	Any& Any::operator << (T& other) {
		return operator << (const_cast<const T&>(other));
	}

	/// Move-insert an element at the back													
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator << (T&& other) {
		if constexpr (CT::Abandoned<T>)
			Insert<Any, false, true>(Forward<T>(other.mValue), Index::Back);
		else if constexpr (CT::Disowned<T>)
			Insert<Any, false, true>(&other.mValue, 1, Index::Back);
		else
			Insert<Any, true, true>(Forward<T>(other), Index::Back);
		return *this;
	}

	/// Copy-insert an element (including arrays) at the front						
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator >> (const T& other) {
		if constexpr (CT::Array<T>)
			Insert<Any, true, true>(other, ExtentOf<T>, Index::Front);
		else
			Insert<Any, true, true>(&other, 1, Index::Front);
		return *this;
	}

	/// Used to disambiguate from the && variant											
	/// Dimo, I know you want to remove this, but don't, said Dimo to himself	
	/// after actually deleting this function numerous times							
	template<CT::Data T>
	Any& Any::operator >> (T& other) {
		return operator >> (const_cast<const T&>(other));
	}

	/// Move-insert element at the front													
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator >> (T&& other) {
		if constexpr (CT::Abandoned<T>)
			Insert<Any, false, true>(Forward<T>(other.mValue), Index::Front);
		else if constexpr (CT::Disowned<T>)
			Insert<Any, false, true>(&other.mValue, 1, Index::Front);
		else
			Insert<Any, true, true>(Forward<T>(other), Index::Front);
		return *this;
	}

	/// Merge data (including arrays) at the back										
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator <<= (const T& other) {
		if constexpr (CT::Array<T>)
			Merge<Decay<T>, true, Any>(other, ExtentOf<T>, Index::Back);
		else
			Merge<T, true, Any>(&other, 1, Index::Back);
		return *this;
	}

	/// Used to disambiguate from the && variant											
	/// Dimo, I know you want to remove this, but don't, said Dimo to himself	
	/// after actually deleting this function numerous times							
	template<CT::Data T>
	Any& Any::operator <<= (T& other) {
		return operator <<= (const_cast<const T&>(other));
	}

	/// Merge data at the back by move-insertion											
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator <<= (T&& other) {
		Merge<T, true, Any>(Forward<T>(other), Index::Back);
		return *this;
	}

	/// Merge data at the front																
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator >>= (const T& other) {
		if constexpr (CT::Array<T>)
			Merge<Decay<T>, true, Any>(other, ExtentOf<T>, Index::Front);
		else
			Merge<T, true, Any>(&other, 1, Index::Front);
		return *this;
	}

	/// Used to disambiguate from the && variant											
	/// Dimo, I know you want to remove this, but don't, said Dimo to himself	
	/// after actually deleting this function numerous times							
	template<CT::Data T>
	Any& Any::operator >>= (T& other) {
		return operator >>= (const_cast<const T&>(other));
	}

	/// Merge data at the front by move-insertion										
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator >>= (T&& other) {
		Merge<T, true, Any>(Forward<T>(other), Index::Front);
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
	constexpr void Any::ResetState() noexcept {
		mState = mState.mState & (DataState::Typed | DataState::Sparse);
		mType = mState.mState & DataState::Typed ? mType : nullptr;
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
