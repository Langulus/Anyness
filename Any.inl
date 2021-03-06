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

	/// Same as copy-construction, but doesn't reference anything					
	///	@param other - the block to copy													
	inline Any::Any(Disowned<Any>&& other) noexcept
		: Block {other.Forward<Block>()} {
		mEntry = nullptr;
	}
	
	/// Same as move-construction but doesn't fully reset other, saving some	
	/// instructions																				
	///	@param other - the block to move													
	inline Any::Any(Abandoned<Any>&& other) noexcept
		: Block {other.Forward<Block>()} {
		other.mValue.mEntry = nullptr;
	}

	/// Construct by copying/referencing value of non-block type					
	///	@tparam T - the data type to push (deducible)								
	///	@param other - the dense value to shallow-copy								
	template <CT::CustomData T>
	Any::Any(const T& other) {
		if constexpr (CT::Sparse<T>)
			MakeSparse();
		SetType<T, false>();
		Insert<IndexBack, Any, true, false, T>(&other, &other + 1);
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
		if constexpr (CT::Sparse<T>)
			MakeSparse();
		SetType<T, false>();
		Insert<IndexBack, Any, true, false, T>(Move(other));
	}

	/// Construct by inserting a disowned value of non-block type					
	///	@tparam T - the data type to push (deducible)								
	///	@param other - the disowned value												
	template <CT::CustomData T>
	Any::Any(Disowned<T>&& other) {
		if constexpr (CT::Sparse<T>)
			MakeSparse();
		SetType<T, false>();
		Insert<IndexBack, Any, false, false, T>(&other.mValue, &other.mValue + 1);
	}

	/// Construct by inserting an abandoned value of non-block type				
	///	@tparam T - the data type to push (deducible)								
	///	@param other - the abandoned value												
	template <CT::CustomData T>
	Any::Any(Abandoned<T>&& other) {
		if constexpr (CT::Sparse<T>)
			MakeSparse();
		SetType<T, false>();
		Insert<IndexBack, Any, false, false, T>(Move(other.mValue));
	}

	/// Destruction																				
	inline Any::~Any() {
		Free();
	}

	/// Create an empty Any from a dynamic type and state								
	///	@param type - type of the container												
	///	@param state - optional state of the container								
	///	@return the new container instance												
	inline Any Any::FromMeta(DMeta type, const DataState& state) noexcept {
		return Any {Block {state, type}};
	}

	/// Create an empty Any by copying type and state of a block					
	///	@param block - the source of type and state									
	///	@param state - additional state of the container							
	///	@return the new container instance												
	inline Any Any::FromBlock(const Block& block, const DataState& state) noexcept {
		return Any::FromMeta(block.GetType(), block.GetUnconstrainedState() + state);
	}

	/// Create an empty Any by copying only state of a block							
	///	@param block - the source of the state											
	///	@param state - additional state of the container							
	///	@return the new container instance												
	inline Any Any::FromState(const Block& block, const DataState& state) noexcept {
		return Any::FromMeta(nullptr, block.GetUnconstrainedState() + state);
	}

	/// Create an empty Any from a static type and state								
	///	@tparam T - the contained type													
	///	@param state - optional state of the container								
	///	@return the new container instance												
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
			Any wrapped[] {Forward<LIST>(elements)...};
			result.SetType<Any, false>();
			result.Allocate(sizeof...(LIST));
			for (auto& it : wrapped)
				result.template Insert<IndexBack, Any, false, false>(Move(it));
			return result;
		}
	}

	/// Pack any number of same-type elements sequentially							
	///	@param head - first element														
	///	@param tail... - the rest of the elements										
	///	@returns the new container containing the data								
	template<CT::Data HEAD, CT::Data... TAIL>
	Any Any::WrapCommon(HEAD&& head, TAIL&&... tail) {
		if constexpr (sizeof...(TAIL) == 0)
			return {};
		else {
			Deref<HEAD> wrapped[] {Forward<HEAD>(head), Forward<TAIL>(tail)...};
			auto result = Any::From<HEAD>();
			for (auto& it : wrapped)
				result.template Insert<IndexBack, Any, false, false>(Move(it));
			return result;
		}
	}
	
	/// Shallow-copy a container																
	///	@param other - the container to copy											
	///	@return a reference to this container											
	inline Any& Any::operator = (const Any& other) {
		if (this == &other)
			return *this;

		// Since Any is type-erased, we have to make a runtime type check	
		if (IsTypeConstrained() && !CastsToMeta(other.mType)) {
			Throw<Except::Copy>(
				"Unable to copy-assign type-constrained container - types are incompatible");
		}

		// First we reference, so that we don't lose the memory, in			
		// the rare case where memory is same in both containers				
		other.Keep();
		Free();
		Block::operator = (other);
		return *this;
	}
	
	/// Move a container																			
	///	@param other - the container to move and reset								
	///	@return a reference to this container											
	inline Any& Any::operator = (Any&& other) {
		if (this == &other)
			return *this;

		// Since Any is type-erased, we have to make a runtime type check	
		if (IsTypeConstrained() && !CastsToMeta(other.mType)) {
			Throw<Except::Copy>(
				"Unable to move-assign type-constrained container - types are incompatible");
		}

		Free();
		Block::operator = (other);
		other.ResetMemory();
		other.ResetState();
		return *this;
	}

	/// Shallow-copy a disowned container (doesn't reference anything)			
	///	@param other - the container to copy											
	///	@return a reference to this container											
	inline Any& Any::operator = (Disowned<Any>&& other) {
		if (this == &other.mValue)
			return *this;

		// Since Any is type-erased, we have to make a runtime type check	
		if (IsTypeConstrained() && !CastsToMeta(other.mValue.mType)) {
			Throw<Except::Copy>(
				"Unable to disown-assign type-constrained container - types are incompatible");
		}

		Free();
		mRaw = other.mValue.mRaw;
		mCount = other.mValue.mCount;
		mReserved = other.mValue.mReserved;
		mState = other.mValue.mState;
		mType = other.mValue.mType;
		// No need to copy entry - it's been reset by Free(), and this is	
		// a disowned copy																
		return *this;
	}
	
	/// Move an abandoned container, minimally resetting the source				
	///	@param other - the container to move and reset								
	///	@return a reference to this container											
	inline Any& Any::operator = (Abandoned<Any>&& other) {
		if (this == &other.mValue)
			return *this;

		// Since Any is type-erased, we have to make a runtime type check	
		if (IsTypeConstrained() && !CastsToMeta(other.mValue.mType)) {
			Throw<Except::Copy>(
				"Unable to abandon-assign type-constrained container - types are incompatible");
		}

		Free();
		Block::operator = (Forward<Block>(other.mValue));
		other.mValue.mEntry = nullptr;
		return *this;
	}

	/// Helper function for preparing reassignment										
	template<class T>
	void Any::PrepareForReassignment() {
		const auto meta = MetaData::Of<Decay<T>>();

		// Since Any is type-erased, we have to make a runtime type check	
		if (IsTypeConstrained() && !CastsToMeta(meta)) {
			Throw<Except::Copy>(
				"Unable to value-assign type-constrained container - types are incompatible");
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
			else
				MakeDense();
			AllocateInner<false>(1);
		}
	}

	/// Assign by shallow-copying anything 												
	///	@param other - the item to copy													
	///	@return a reference to this container											
	template<CT::CustomData T>
	Any& Any::operator = (const T& other) {
		PrepareForReassignment<T>();
		InsertInner<true>(&other, &other + 1, 0);
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
		PrepareForReassignment<T>();
		InsertInner<true>(Forward<T>(other), 0);
		return *this;
	}

	/// Assign by disowning anything non-block											
	///	@param other - the disowned element to push									
	///	@return a reference to this container											
	template<CT::CustomData T>
	Any& Any::operator = (Disowned<T>&& other) requires CT::Dense<T> {
		const auto meta = MetaData::Of<Decay<T>>();

		// Since Any is type-erased, we have to make a runtime type check	
		if (IsTypeConstrained() && !CastsToMeta(meta)) {
			Throw<Except::Copy>(
				"Unable to disowned value-assign type-constrained container - types are incompatible");
		}

		if (GetUses() != 1 || IsSparse() != CT::Sparse<T> || !meta->Is(mType)) {
			// Reset and allocate new memory											
			// Disowned-construction will be used if possible					
			Reset();
			operator << (other.Forward());
		}
		else {
			// Just destroy and reuse memory											
			// Even better - types match, so we know this container			
			// is filled with T too, therefore we can use statically			
			// optimized routines for destruction and creation					
			if constexpr (CT::Sparse<T>) {
				CallKnownDestructors<T>();
				mCount = 1;
				mRawSparse->mPointer = reinterpret_cast<Byte*>(other.mValue);
				mRawSparse->mEntry = nullptr;
			}
			else {
				CallKnownDestructors<T>();
				mCount = 1;
				if constexpr (CT::DisownMakable<T>)
					new (mRaw) T {other.Forward()};
				else
					new (mRaw) T {other.mValue};
			}
		}

		return *this;
	}

	/// Assign by abandoning anything non-block											
	///	@param other - the abandoned element to push									
	///	@return a reference to this container											
	template<CT::CustomData T>
	Any& Any::operator = (Abandoned<T>&& other) requires CT::Dense<T> {
		const auto meta = MetaData::Of<Decay<T>>();

		// Since Any is type-erased, we have to make a runtime type check	
		if (IsTypeConstrained() && !CastsToMeta(meta)) {
			Throw<Except::Copy>(
				"Unable to abandoned value-assign type-constrained container - types are incompatible");
		}

		if (GetUses() != 1 || IsSparse() != CT::Sparse<T> || !meta->Is(mType)) {
			// Reset and allocate new memory											
			// Abandoned-construction will be used if possible					
			Reset();
			operator << (other.Forward());
		}
		else {
			// Just destroy and reuse memory											
			// Even better - types match, so we know this container			
			// is filled with T too, therefore we can use statically			
			// optimized routines for destruction and creation					
			if constexpr (CT::Sparse<T>) {
				CallKnownDestructors<T>();
				mCount = 1;
				mRawSparse->mPointer = reinterpret_cast<Byte*>(other.mValue);
				mRawSparse->mEntry = nullptr;
			}
			else {
				CallKnownDestructors<T>();
				mCount = 1;
				if constexpr (CT::AbandonMakable<T>)
					new (mRaw) T {other.Forward()};
				else
					new (mRaw) T {Forward<T>(other.mValue)};
			}
		}

		return *this;
	}

	/// Copy-insert an element (including arrays) at the back						
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator << (const T& other) {
		if constexpr (CT::Array<T>)
			Insert<IndexBack, Any, true, true>(SparseCast(other), SparseCast(other) + ExtentOf<T>);
		else
			Insert<IndexBack, Any, true, true>(&other, &other + 1);
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
			Insert<IndexBack, Any, false, true>(Move(other.mValue));
		else if constexpr (CT::Disowned<T>)
			Insert<IndexBack, Any, false, true>(&other.mValue, &other.mValue + 1);
		else
			Insert<IndexBack, Any, true, true>(Move(other));
		return *this;
	}

	/// Copy-insert an element (including arrays) at the front						
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator >> (const T& other) {
		if constexpr (CT::Array<T>)
			Insert<IndexFront, Any, true, true>(other, ExtentOf<T>);
		else
			Insert<IndexFront, Any, true, true>(&other, 1);
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
			Insert<IndexFront, Any, false, true>(Move(other.mValue));
		else if constexpr (CT::Disowned<T>)
			Insert<IndexFront, Any, false, true>(&other.mValue, 1);
		else
			Insert<IndexFront, Any, true, true>(Forward<T>(other));
		return *this;
	}

	/// Merge data (including arrays) at the back										
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator <<= (const T& other) {
		if constexpr (CT::Array<T>)
			Merge<IndexBack, Decay<T>, true, Any>(other, ExtentOf<T>);
		else
			Merge<IndexBack, T, true, Any>(&other, 1);
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
		Merge<IndexBack, T, true, Any>(Forward<T>(other));
		return *this;
	}

	/// Merge data at the front																
	///	@param other - the data to insert												
	///	@return a reference to this container for chaining							
	template<CT::Data T>
	Any& Any::operator >>= (const T& other) {
		if constexpr (CT::Array<T>)
			Merge<IndexFront, Decay<T>, true, Any>(other, ExtentOf<T>);
		else
			Merge<IndexFront, T, true, Any>(&other, 1);
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
		Merge<IndexFront, T, true, Any>(Forward<T>(other));
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
