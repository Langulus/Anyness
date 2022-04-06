#pragma once
#include "Block.hpp"
#include "../Manager/Manager.hpp"
#include "../Containers/Any.hpp"
#include "../Containers/Text.hpp"

namespace Langulus::Anyness
{

	PC_LEAKSAFETY Block::Block(Block&& other) noexcept
		: mRaw {other.mRaw}
		, mType {other.mType}
		, mCount {other.mCount}
		, mReserved {other.mReserved}
		, mState {other.mState} {
		other.ResetInner();
		PC_EXTENSIVE_LEAK_TEST(*this);
	}

	/// Manual construction via state and type											
	/// No allocation will happen																
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	constexpr Block::Block(const DState& state, DMeta meta) noexcept
		: mState {state}
		, mType {meta} {}

	/// Manual construction from constant data											
	/// No referencing shall occur and changes data state to dsConstant			
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the constant memory - safety is on you			
	PC_LEAKSAFETY Block::Block(const DState& state, DMeta meta, pcptr count, const void* raw) noexcept
		: mRaw {const_cast<void*>(raw)}
		, mType {meta}
		, mCount {count}
		, mReserved {count}
		, mState {state + DState::Constant} {
		PC_EXTENSIVE_LEAK_TEST(*this);
	}

	/// Manual construction from mutable data												
	/// No referencing shall occur and changes data state to dsConstant			
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the mutable memory - safety is on you			
	PC_LEAKSAFETY Block::Block(const DState& state, DMeta meta, pcptr count, void* raw) noexcept
		: mRaw {raw}
		, mType {meta}
		, mCount {count}
		, mReserved {count}
		, mState {state} {
		PC_EXTENSIVE_LEAK_TEST(*this);
	}

	/// Create a memory block from a typed pointer										
	/// No referencing shall occur, this simply initializes the block				
	///	@return the block																		
	template<RTTI::ReflectedData T>
	Block Block::From(T value) requires Sparse<T> {
		static_assert(!Same<T, Block>, 
			"Block of Blocks is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - use Any instead");

		const Block block {
			DState::Static, 
			DataID::Reflect<pcDecay<T>>(), 
			1, value
		};

		PC_EXTENSIVE_LEAK_TEST(block);
		return block;
	}

	///	@return the block																		
	template<RTTI::ReflectedData T>
	Block Block::From(T value, pcptr count) requires Sparse<T> {
		static_assert(!Same<T, Block>, 
			"Block of Blocks is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - use Any instead");

		const Block block {
			DState::Static, 
			DataID::Reflect<pcDecay<T>>(), 
			count, value
		};

		PC_EXTENSIVE_LEAK_TEST(block);
		return block;
	}

	/// Create a memory block from a value reference									
	/// No referencing shall occur, this simply initializes the block				
	///	@return the block																		
	template<RTTI::ReflectedData T>
	Block Block::From(T& value) requires Dense<T> {
		if constexpr (!IsText<T> && pcIsDeep<T> && sizeof(T) == sizeof(Block)) {
			PC_EXTENSIVE_LEAK_TEST(value);
			return static_cast<const Block&>(value);
		}
		else if constexpr (Resolvable<T>) {
			const auto block = value.GetBlock();
			PC_EXTENSIVE_LEAK_TEST(block);
			return block;
		}
		else {
			const Block block {
				DState::Static,
				DataID::Reflect<pcDecay<T>>(),
				1, &value
			};

			PC_EXTENSIVE_LEAK_TEST(block);
			return block;
		}
	}

	/// Create an empty memory block from a static type								
	///	@return the block																		
	template<RTTI::ReflectedData T>
	Block Block::From() {
		static_assert(!Same<T, Block>, 
			"Block of Blocks is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - use Any instead");

		return Block {
			DState::Default, 
			DataID::Reflect<T>(), 
			0, static_cast<void*>(nullptr)
		};
	}

	/// Shallow copy memory block																
	/// No referencing will occur - this simply copies the other block			
	/// This is not an alternative to Block::Copy! This operator simply			
	/// overwrites the instance, it does not check for type-constraints			
	/// and never references - it's an unsafe low level function					
	///	@param other - the memory block to copy										
	///	@return a reference to this block												
	PC_LEAKSAFETY Block& Block::operator = (const Block& other) noexcept {
		mRaw = other.mRaw;
		mType = other.mType;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		PC_EXTENSIVE_LEAK_TEST(*this);
		return *this;
	}

	/// Move memory block																		
	/// No referencing will occur - this simply copies and resets the other		
	/// Other will retain its type-constraints after the reset						
	///	@param other - the memory block to move										
	///	@return a reference to this block												
	PC_LEAKSAFETY Block& Block::operator = (Block&& other) noexcept {
		mRaw = other.mRaw;
		mType = other.mType;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		other.ResetInner();
		PC_EXTENSIVE_LEAK_TEST(*this);
		return *this;
	}

	/// Clear the block, only zeroing its size											
	constexpr void Block::ClearInner() noexcept {
		mCount = 0;
	}

	/// Reset the block, by zeroing everything, except type-constraints			
	constexpr void Block::ResetInner() noexcept {
		mRaw = nullptr;
		mCount = mReserved = 0;

		if (IsTypeConstrained())
			mState = DState::Typed;
		else {
			mType = nullptr;
			mState = DState::Default;
		}
	}

	/// Set the contained type, using a static type										
	///	@tparam T - the type to reflect and set										
	///	@param constrain - whether or not to enable type-constraints			
	template<RTTI::ReflectedData T>
	void Block::SetDataID(bool constrain) {
		static_assert(!Same<T, Block>, 
			"Block of type Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - use type Any instead");

		SetDataID(DataID::Reflect<T>(), constrain);
	}

	/// Get the contained type meta definition											
	///	@return the meta data																
	constexpr DMeta Block::GetMeta() const noexcept {
		return mType;
	}

	/// Get the number of reserved (maybe unconstructed) elements					
	///	@return the number of reserved (probably not constructed) elements	
	constexpr pcptr Block::GetReserved() const noexcept {
		return mReserved;
	}

	/// Get a constant byte array																
	///	@return a constant pointer to the beginning of the raw contained data
	PC_LEAKSAFETY const pcbyte* Block::GetBytes() const noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return static_cast<const pcbyte*>(mRaw);
	}

	/// Get a byte array																			
	///	@return a pointer to the beginning of the raw contained data			
	PC_LEAKSAFETY pcbyte* Block::GetBytes() noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return static_cast<pcbyte*>(mRaw);
	}

	/// Get a constant pointer array - useful for sparse containers				
	///	@return the raw data as an array of constant pointers						
	inline const void* const* Block::GetPointers() const noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return reinterpret_cast<const void* const*>(mRaw);
	}

	/// Get a pointer array - useful for sparse containers							
	///	@return the raw data as an array of pointers									
	inline void** Block::GetPointers() noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return reinterpret_cast<void**>(mRaw);
	}

	/// Check if memory is allocated															
	///	@return true if the block contains any reserved memory					
	constexpr bool Block::IsAllocated() const noexcept {
		return mRaw != nullptr;
	}

	/// Check if block is left-polarized													
	///	@returns true if this container is left-polarized							
	constexpr bool Block::IsLeft() const noexcept {
		return GetPolarity() == Past;
	}

	/// Check if block is right-polarized													
	///	@returns true if this container is right-polarized							
	constexpr bool Block::IsRight() const noexcept {
		return GetPolarity() == Future;
	}

	/// Check if block is not polarized														
	///	@returns true if this container is not polarized							
	constexpr bool Block::IsNeutral() const noexcept {
		return GetPolarity() == Now;
	}

	/// Check if block is marked as missing												
	///	@returns true if this container is marked as vacuum						
	constexpr bool Block::IsMissing() const noexcept {
		return mState & DState::Missing;
	}

	/// Check if block has a data type														
	///	@returns true if data contained in this pack is unspecified				
	constexpr bool Block::IsUntyped() const noexcept {
		return nullptr == mType;
	}

	/// Check if block has a data type, and is type-constrained						
	///	@return true if type-constrained													
	constexpr bool Block::IsTypeConstrained() const noexcept {
		return mType && (mState & DState::Typed);
	}

	/// Check if block is polarized															
	///	@returns true if this pack is either left-, or right-polar				
	constexpr bool Block::IsPolar() const noexcept {
		return mState & DState::Polar;
	}

	/// Check if block is encrypted															
	///	@returns true if the contents of this pack are encrypted					
	constexpr bool Block::IsEncrypted() const noexcept {
		return mState & DState::Encrypted;
	}

	/// Check if block is compressed															
	///	@returns true if the contents of this pack are compressed				
	constexpr bool Block::IsCompressed() const noexcept {
		return mState & DState::Compressed;
	}

	/// Check if block is constant															
	///	@returns true if the contents of this pack are constant					
	constexpr bool Block::IsConstant() const noexcept {
		return mState & DState::Constant;
	}

	/// Check if block is static																
	///	@returns true if the contents of this pack are constant					
	constexpr bool Block::IsStatic() const noexcept {
		return mState & DState::Static;
	}

	/// Check if block is inhibitory (or) container										
	///	@returns true if this is an inhibitory container							
	constexpr bool Block::IsOr() const noexcept {
		return mState & DState::Or;
	}

	/// Check if block contains no created elements (it may still have state)	
	///	@returns true if this is an empty container									
	constexpr bool Block::IsEmpty() const noexcept {
		return mCount == 0;
	}

	/// Check if block contains either created elements, or relevant state		
	///	@returns true if this is not an empty stateless container				
	constexpr bool Block::IsValid() const noexcept {
		return mCount > 0 || GetUnconstrainedState() || mType;
	}

	/// Check if block contains no elements and no relevant state					
	///	@returns true if this is an empty stateless container						
	constexpr bool Block::IsInvalid() const noexcept {
		return !IsValid();
	}

	/// Check if block contains dense data													
	///	@returns true if this container refers to dense memory					
	inline bool Block::IsDense() const {
		return !IsSparse();
	}

	/// Get polarity																				
	///	@return the polarity of the contained data									
	constexpr Block::Polarity Block::GetPolarity() const noexcept {
		if (!IsPolar())
			return Now;
		return mState & DState::Positive ? Future : Past;
	}

	/// Set polarity																				
	///	@param p - polarity to enable														
	inline void Block::SetPolarity(const Polarity p) noexcept {
		switch (p) {
		case Past:
			mState += DState::Polar;
			mState -= DState::Positive;
			return;
		case Now:
			mState -= DState::Polar;
			mState -= DState::Positive;
			return;
		case Future:
			mState += DState::Polar;
			mState += DState::Positive;
			return;
		}
	}

	/// Check polarity compatibility															
	///	@param other - the polarity to check											
	///	@return true if polarity is compatible											
	inline bool Block::CanFitPolarity(const Polarity& other) const noexcept {
		// As long as polarities are not opposite, they are compatible		
		const auto local_polarity = GetPolarity();
		return local_polarity != -other || (other == Now && local_polarity == other);
	}

	/// Check state compatibility																
	///	@param other - the state to check												
	///	@return true if state is compatible												
	inline bool Block::CanFitState(const Block& other) const noexcept {
		const bool orCompat = IsOr() == other.IsOr() || other.GetCount() <= 1 || IsEmpty();
		const bool typeCompat = !IsTypeConstrained() || (IsTypeConstrained() && other.InterpretsAs(mType));
		return typeCompat && (mState == other.mState || (orCompat && CanFitPolarity(other.GetPolarity())));
	}

	/// Get the size of the contained data, in bytes									
	///	@return the byte size																
	inline pcptr Block::GetSize() const noexcept {
		return GetCount() * GetStride();
	}

	/// Check if a type can be inserted														
	template<RTTI::ReflectedData T>
	bool Block::IsInsertable() const noexcept {
		if constexpr (Same<T, Block>)
			return false;
		else
			return IsInsertable(DataID::Reflect<T>());
	}

	/// Get the raw data inside the container												
	///	@attention as unsafe as it gets, but as fast as it gets					
	inline void* Block::GetRaw() noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return mRaw;
	}

	/// Get the raw data inside the container (const)									
	///	@attention as unsafe as it gets, but as fast as it gets					
	inline const void* Block::GetRaw() const noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return mRaw;
	}

	/// Get the end raw data pointer inside the container								
	///	@attention as unsafe as it gets, but as fast as it gets					
	inline const void* Block::GetRawEnd() const noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return GetBytes() + GetSize();
	}

	/// Get the raw data inside the container, reinterpreted as some type		
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<RTTI::ReflectedData T>
	inline T* Block::GetRawAs() noexcept {
		return reinterpret_cast<T*>(GetRaw());
	}

	/// Get the raw data inside the container, reinterpreted (const)				
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<RTTI::ReflectedData T>
	inline const T* Block::GetRawAs() const noexcept {
		return reinterpret_cast<const T*>(GetRaw());
	}

	/// Get the end raw data pointer inside the container								
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<RTTI::ReflectedData T>
	inline const T* Block::GetRawEndAs() const noexcept {
		return reinterpret_cast<const T*>(GetRawEnd());
	}

	/// Get the data state of the container												
	constexpr DState Block::GetState() const noexcept {
		return mState;
	}

	/// Set the current data state, overwriting the current							
	/// You can not remove type-constraints												
	inline void Block::SetState(DState state) noexcept {
		const bool typeConstrained = mState & DState::Typed;
		mState = state;
		if (typeConstrained)
			mState += DState::Typed;
	}

	/// Get the relevant state when relaying one block	to another					
	/// Relevant states exclude memory and type constraints							
	constexpr DState Block::GetUnconstrainedState() const noexcept {
		return mState - (DState::Static + DState::Constant + DState::Typed);
	}

	/// Compare memory blocks. This is a slow RTTI check								
	inline bool Block::operator == (const Block& other) const noexcept {
		return Compare(other);
	}

	inline bool Block::operator != (const Block& other) const noexcept {
		return !(*this == other);
	}

	/// Get the internal byte array with a given offset								
	/// This is lowest level access and checks nothing									
	///	@param byte_offset - number of bytes to add									
	///	@return the selected byte															
	inline pcbyte* Block::At(pcptr byte_offset) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		#if LANGULUS_SAFE()
			if (!mRaw) 
				throw Except::BadAccess(pcLogFuncError
					<< "Byte offset in invalid memory of type " << GetToken());
		#endif
		return GetBytes() + byte_offset;
	}

	inline const pcbyte* Block::At(pcptr byte_offset) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return const_cast<Block&>(*this).At(byte_offset);
	}

	/// Get templated element																	
	/// Checks only density																		
	template<RTTI::ReflectedData T>
	decltype(auto) Block::Get(pcptr idx, pcptr baseOffset) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return const_cast<Block&>(*this).Get<T>(idx, baseOffset);
	}

	/// Get templated element																	
	/// Checks density and type																
	template<RTTI::ReflectedData T>
	decltype(auto) Block::As(pcptr idx) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return const_cast<Block&>(*this).As<T>(idx);
	}

	/// Get templated element																	
	/// Checks range, density, type and special indices								
	template<RTTI::ReflectedData T>
	decltype(auto) Block::As(Index index) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		index = ConstrainMore<T>(index);
		if (index.IsSpecial()) {
			throw Except::BadAccess(pcLogFuncError 
				<< "Can't reference special index");
		}

		return As<T>(pcptr(index.mIndex));
	}

	template<RTTI::ReflectedData T>
	decltype(auto) Block::As(Index index) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return const_cast<Block&>(*this).As<T>(index);
	}

	/// Check if a pointer is anywhere inside the block's memory					
	///	@param ptr - the pointer to check												
	///	@return true if inside the memory block										
	inline bool Block::Owns(const void* ptr) const noexcept {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return mRaw	&& pcP2N(ptr) >= pcP2N(mRaw) 
			&& pcP2N(ptr) < pcP2N(mRaw) + GetSize();
	}

	/// Decay the block to some base type, if sizes are compatible					
	///	@tparam T - the base type to decay to											
	///	@return the decayed block															
	template<RTTI::ReflectedData T>
	Block Block::Decay() const {
		static_assert(!Same<T, Block>, 
			"Decaying Block to Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - Decay as Any instead");
		return Decay(MetaData::Of<T>());
	}

	/// Mutate the block to a different type, if possible								
	///	@tparam T - the type to change to												
	///	@return true if block was deepened to incorporate the new type			
	template<RTTI::ReflectedData T>
	bool Block::Mutate() {
		static_assert(!Same<T, Block>, 
			"Mutating Block to Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - mutate as Any instead");
		return Mutate(DataID::Reflect<T>());
	}

	/// Reserve a number of elements															
	///	@tparam T - the type of elements to allocate									
	///	@param count - the number of elements to reserve							
	///	@param construct - true to call default constructors of each element	
	///	@param setcount - true to set the count, too									
	///	@attention beware of blocks with unconstructed elements, but with		
	///				  set count - could cause undefined behavior						
	template<RTTI::ReflectedData T>
	void Block::Allocate(pcptr count, bool construct, bool setcount) {
		static_assert(!Same<T, Block>, 
			"Allocating Blocks inside Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - allocate Any instead");

		SetDataID<T>(false);
		Allocate(count, construct, setcount);
		PC_EXTENSIVE_LEAK_TEST(*this);
	}

	/// Constrain an index to the limits of the current block						
	///	@param idx - the index to constrain												
	///	@return the constrained index or a special one of constrain fails		
	constexpr Index Block::Constrain(const Index& idx) const noexcept {
		switch (idx.mIndex) {
		case uiAuto.mIndex: case uiFirst.mIndex: case uiFront.mIndex:
			return 0;
		case uiAll.mIndex: case uiBack.mIndex:
			return Index(mCount);
		case uiLast.mIndex:
			return mCount != 0 ? Index(mCount - 1) : uiNone;
		case uiMiddle.mIndex:
			return mCount / 2;
		case uiNone.mIndex:
			return uiNone;
		}
		return idx.Constrained(mCount);
	}

	/// Constrain an index to the limits of the current block						
	/// Supports additional type-dependent constraints									
	///	@param idx - the index to constrain												
	///	@return the constrained index or a special one of constrain fails		
	template<RTTI::ReflectedData T>
	Index Block::ConstrainMore(const Index& idx) const noexcept {
		const auto result = Constrain(idx);
		if (result.IsSpecial()) {
			switch (result.mIndex) {
			case uiBiggest.mIndex:
				if constexpr (Sortable<T>)
					return GetIndexMax<T>();
				else return uiNone;
				break;
			case uiSmallest.mIndex:
				if constexpr (Sortable<T>)
					return GetIndexMin<T>();
				else return uiNone;
				break;
			case uiMode.mIndex:
				if constexpr (Sortable<T>) {
					pcptr unused;
					return GetIndexMode<T>(unused);
				}
				else return uiNone;
				break;
			}
		}
		return result;
	}

	template<RTTI::ReflectedData T>
	bool Block::CanFit() const {
		return CanFit(MetaData::Of<T>());
	}

	template<RTTI::ReflectedData T>
	bool Block::InterpretsAs() const {
		return InterpretsAs(MetaData::Of<T>());
	}

	template<RTTI::ReflectedData T>
	bool Block::InterpretsAs(pcptr count) const {
		return InterpretsAs(MetaData::Of<T>(), count);
	}

	template<RTTI::ReflectedData T>
	bool Block::Is() const {
		return Is(DataID::Of<pcDecay<T>>);
	}

	/// Swap two elements (with raw indices)												
	///	@param from - first element index												
	///	@param to - second element index													
	template<RTTI::ReflectedData T>
	void Block::Swap(pcptr from, pcptr to) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if (from >= mCount || to >= mCount || from == to)
			return;

		auto data = &Get<T>();
		T temp(pcMove(data[to]));
		data[to] = pcMove(data[from]);
		data[from] = pcMove(temp);
	}

	/// Swap two elements (with special indices)											
	///	@param from - first element index												
	///	@param to - second element index													
	template<RTTI::ReflectedData T>
	void Block::Swap(Index from, Index to) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if (from == to)
			return;

		from = ConstrainMore<T>(from);
		to = ConstrainMore<T>(to);
		if (from.IsSpecial() || to.IsSpecial())
			return;

		Swap<T>(static_cast<pcptr>(from), static_cast<pcptr>(to));
	}

	/// Emplace anything compatible to container											
	///	@attention when emplacing pointers, their memory is referenced,		
	///				  and the pointer is not cleared, so you can free it later	
	///	@param item - item to move															
	///	@param index - use uiFront or uiBack for pushing to ends					
	///	@return number of inserted elements												
	template<RTTI::ReflectedData T, bool MUTABLE>
	pcptr Block::Emplace(T&& item, const Index& index) {
		PC_EXTENSIVE_LEAK_TEST(*this);

		static_assert(!Same<T, Block>, 
			"Emplacing Block inside Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - wrap your block in an Any first");

		// Check errors before actually doing anything							
		if constexpr (Sparse<T>) {
			if (!item) {
				throw Except::BadReference(pcLogFuncError
					<< "Move-insertion of a null pointer of type "
					<< GetToken() << " is not allowed");
			}
		}

		// Constrain index																
		auto tidx = ConstrainMore<T>(index);
		const auto starter = static_cast<pcptr>(tidx);

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T>())
				return Emplace<Any>(Any{ pcForward<T>(item) }, index);
		}

		// Allocate																			
		Allocate(mCount + 1);

		// Move memory if required														
		if (starter < mCount) {
			SAFETY(if (GetBlockReferences() > 1)
				throw Except::BadReference(pcLogFuncError
					<< "Moving elements that are used from multiple places"));

			CropInner(starter + 1, mCount - starter)
				.CallMoveConstructors(
					CropInner(starter, mCount - starter));
		}

		// Insert new data																
		auto data = GetBytes();
		data += starter * sizeof(T);
		if constexpr (Sparse<T>) {
			// Sparse data insertion (moving a pointer)							
			*reinterpret_cast<pcptr*>(data) = pcP2N(item);

			// Reference the pointer's memory										
			PCMEMORY.Reference(mType, item, 1);
		}
		else {
			static_assert(!pcIsAbstract<T>, "Can't emplace abstract item");

			// Dense data insertion (placement move-construction)				
			if constexpr (::std::is_move_constructible<T>())
				new (data) T { pcForward<T>(item) };
			else
				LANGULUS_ASSERT("Can't emplace non-move-constructible item");
		}

		++mCount;
		return 1;
	}
	
	/// Insert anything compatible to container											
	///	@param items - items to push														
	///	@param count - number of items inside											
	///	@param index - use uiFront or uiBack for pushing to ends					
	///	@return number of inserted elements												
	template<RTTI::ReflectedData T, bool MUTABLE>
	pcptr Block::Insert(const T* items, const pcptr count, const Index& index) {
		PC_EXTENSIVE_LEAK_TEST(*this);

		static_assert(!Same<T, Block>, 
			"Inserting Block inside Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - wrap your block in an Any first");

		// Constrain index																
		auto tidx = ConstrainMore<T>(index);
		const auto starter = static_cast<pcptr>(tidx);

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T>()) {
				Any wrapper;
				wrapper.Insert<T>(items, count);
				return Emplace<Any>(pcMove(wrapper), index);
			}
		}

		// Allocate																			
		Allocate(mCount + count);

		// Move memory if required														
		if (starter < mCount) {
			SAFETY(if (GetBlockReferences() > 1)
				throw Except::BadReference(pcLogFuncError
					<< "Moving elements that are used from multiple places"));

			CropInner(starter + count, mCount - starter)
				.CallMoveConstructors(
					CropInner(starter, mCount - starter));
		}

		// Insert new data																
		auto data = GetBytes();
		data += starter * sizeof(T);
		if constexpr (Sparse<T>) {
			// Sparse data insertion (copying pointers and referencing)		
			pcCopyMemory(items, data, sizeof(T) * count);
			pcptr c = 0;
			while (c < count) {
				if (!items[c]) {
					throw Except::BadReference(pcLogFuncError
						<< "Copy-insertion of a null pointer of type "
						<< GetToken() << " is not allowed");
				}

				// Reference each pointer												
				PCMEMORY.Reference(mType->GetDenseMeta(), items[c], 1);
				++c;
			}
		}
		else {
			static_assert(!pcIsAbstract<T>, "Can't insert abstract item");

			if constexpr (sizeof(T) == 1 || Same<T, wchar_t>) {
				// Optimized byte/char/wchar_t insertion							
				pcCopyMemory(items, data, count * sizeof(T));
			}
			else if constexpr (std::is_copy_constructible<T>()) {
				// Dense data insertion (placement copy-construction)			
				pcptr c = 0;
				while (c < count) {
					// Reset all items													
					new (data + c * sizeof(T)) T { items[c] };
					if constexpr (Same<T, Block>) {
						// Blocks don't have referencing copy-constructors,	
						// so we have to compensate for that here					
						items[c].Keep();
					}

					++c;
				}
			}
			else LANGULUS_ASSERT("Can't insert non-copy-constructible item(s)");
		}

		mCount += count;
		return count;
	}

	/// Remove non-sequential element(s)													
	///	@param items - the items to search for and remove							
	///	@param count - number of items inside array									
	///	@param index - the index to start searching from							
	///	@return the number of removed items												
	template<RTTI::ReflectedData T>
	pcptr Block::Remove(const T* items, const pcptr count, const Index& index) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		pcptr removed = 0;
		for (pcptr i = 0; i < count; ++i) {
			const auto idx = Find<T>(items[i], index);
			if (idx != uiNone)
				removed += RemoveIndex(static_cast<pcptr>(idx), 1);
		}

		return removed;
	}

	/// Find first matching element position inside container						
	///	@param item - the item to search for											
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	template<RTTI::ReflectedData T>
	Index Block::Find(const T& item, const Index& idx) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if (!mCount || !mType)
			return uiNone;

		if (!mType->IsSparse()) {
			if (!InterpretsAs<T>()) {
				// If dense and not forward compatible - fail					
				return uiNone;
			}
		}
		else if (!DataID::Reflect<T>()->InterpretsAs(mType)) {
			// If sparse and not backwards compatible - fail					
			return uiNone;
		}

		// Setup the iterator															
		const auto pcount = pcidx(mCount);
		pcidx starti, istep;
		switch (idx.mIndex) {
		case uiFront.mIndex:
			starti = 0;
			istep = 1;
			break;
		case uiBack.mIndex:
			starti = pcount - 1;
			istep = -1;
			break;
		default:
			starti = Constrain(idx).mIndex;
			istep = 1;
			if (starti + 1 >= pcidx(mCount))
				return uiNone;
		}

		// Search																			
		auto item_ptr = pcPtr(item);
		for (auto i = starti; i < pcount && i >= 0; i += istep) {
			auto left = pcPtr(Get<T>(i));
			if (left == item_ptr) {
				// Early success if pointers match									
				return i;
			}

			if constexpr (Sparse<T>) {
				// If searching for pointers - cease after pointer check		
				continue;
			}
			else {
				if constexpr (Resolvable<T>) {
					// Pointers didn't match, but we have ClassBlock 			
					// so we attempt to call reflected comparison operator	
					// for the concrete types											
					auto lhs = left->GetBlock();
					auto rhs = item_ptr->GetBlock();
					if (lhs.GetMeta() != rhs.GetMeta())
						continue;

					if (lhs.GetDescriptor()->mComparer) {
						if (lhs.GetDescriptor()->mComparer(lhs.GetRaw(), rhs.GetRaw()))
							return i;
					}
					else {
						pcLogFuncWarning << "Dynamically resolved type " << lhs.GetToken() 
							<< " missing compare operator implementation and/or reflection";
						pcLogFuncWarning << "This means that any Find, Merge, "
							"Select, etc. will fail for that type";
						pcLogFuncWarning << "Implement operator == and != either in type "
							"or in any relevant bases to fix this problem";
					}
				}
				else if constexpr (Comparable<T, T>) {
					// Pointers didn't match, but we have dense & comparable	
					// type, so attempt to compare using == operator			
					if (*left == *item_ptr)
						return i;
				}
				else {
					// A dense type that has no == operator results in			
					// failure, since it is uncomparable							
					LANGULUS_ASSERT("Type is not comparable in order to search for it");
				}
			}
		}

		// If this is reached, then no match was found							
		return uiNone;
	}
	
	/// Find first matching element position inside container, deeply				
	///	@param item - the item to search for											
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	template<RTTI::ReflectedData T>
	Index Block::FindDeep(const T& item, const Index& idx) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		auto found = uiNone;
		ForEachDeep([&](const Block& group) {
			found = group.Find<T>(item, idx);
			return found == uiNone;
		});

		return found;
	}

	/// Merge array elements inside container.											
	/// Pushes only if element(s) was not found											
	///	@param items - the items to push													
	///	@param count - number of items to push											
	///	@param idx - use uiFront or uiBack for pushing to ends					
	///	@return the number of inserted elements										
	template<RTTI::ReflectedData T, bool MUTABLE>
	pcptr Block::Merge(const T* items, const pcptr count, const Index& idx) {
		PC_EXTENSIVE_LEAK_TEST(*this);

		static_assert(!Same<T, Block>, 
			"Merge Block inside Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - wrap your block in an Any first");

		pcptr added = 0;
		for (pcptr i = 0; i < count; ++i) {
			if (uiNone == Find<T>(items[i]))
				added += Insert<T, MUTABLE>(items + i, 1, idx);
		}

		return added;
	}

	/// Get the index of the biggest dense element										
	template<RTTI::ReflectedData T>
	Index Block::GetIndexMax() const noexcept requires Sortable<T> {
		if (IsEmpty())
			return uiNone;

		auto data = Get<pcDecay<T>*>();
		auto max = data;
		for (pcptr i = 1; i < mCount; ++i) {
			if (*pcPtr(data[i]) > *pcPtr(*max))
				max = data + i;
		}

		return (pcP2N(max) - pcP2N(data)) / sizeof(T);
	}

	/// Get the index of the smallest dense element										
	template<RTTI::ReflectedData T>
	Index Block::GetIndexMin() const noexcept requires Sortable<T> {
		if (IsEmpty())
			return uiNone;

		auto data = Get<pcDecay<T>*>();
		auto min = data;
		for (pcptr i = 1; i < mCount; ++i) {
			if (*pcPtr(data[i]) < *pcPtr(*min))
				min = data + i;
		}

		return (pcP2N(min) - pcP2N(data)) / sizeof(T);
	}

	/// Get the index of dense element that repeats the most times					
	///	@param count - [out] count the number of repeats for the mode			
	///	@return the index of the first found mode										
	template<RTTI::ReflectedData T>
	Index Block::GetIndexMode(pcptr& count) const noexcept {
		if (IsEmpty()) {
			count = 0;
			return uiNone;
		}

		auto data = Get<pcDecay<T>*>();
		decltype(data) best = nullptr;
		pcptr best_count = 0;
		for (pcptr i = 0; i < mCount; ++i) {
			pcptr counter = 0;
			for (pcptr j = i; j < mCount; ++j) {
				if constexpr (Comparable<T, T>) {
					// First we compare by memory pointer, then by ==			
					if (pcPtr(data[i]) == pcPtr(data[j]) ||
						*pcPtr(data[i]) == *pcPtr(data[j]))
						++counter;
				}
				else {
					// No == operator, so just compare by memory	pointer		
					if (pcPtr(data[i]) == pcPtr(data[j]))
						++counter;
				}

				if (counter + (mCount - j) <= best_count)
					break;
			}

			if (counter > best_count || !best) {
				best_count = counter;
				best = data + i;
			}
		}

		count = best_count;
		return (pcP2N(best) - pcP2N(data)) / sizeof(T);
	}

	/// Sort the contents of this container using a static type						
	///	@param first - what will the first element be after sorting?			
	///					 - use uiSmallest for 123, or anything else for 321		
	template<RTTI::ReflectedData T>
	void Block::Sort(const Index& first) noexcept {
		auto data = Get<pcDecay<T>*>();
		if (nullptr == data)
			return;

		pcptr j = 0, i = 0;
		if (first == uiSmallest) {
			for (; i < mCount; ++i) {
				for (; j < i; ++j) {
					if (*pcPtr(data[i]) > *pcPtr(data[j]))
						Swap<T>(i, j);
				}
				for (j = i + 1; j < mCount; ++j) {
					if (*pcPtr(data[i]) > * pcPtr(data[j]))
						Swap<T>(i, j);
				}
			}
		}
		else {
			for (; i < mCount; ++i) {
				for (; j < i; ++j) {
					if (*pcPtr(data[i]) < * pcPtr(data[j]))
						Swap<T>(i, j);
				}
				for (j = i + 1; j < mCount; ++j) {
					if (*pcPtr(data[i]) < *pcPtr(data[j]))
						Swap<T>(i, j);
				}
			}
		}
	}

	/// A smart push uses the best approach to push anything inside container	
	/// in order to keep hierarchy and states, but also reuse memory				
	///	@param pack - the container to smart-push										
	///	@param finalState - a state to apply after pushing is done				
	///	@param attemptConcat - whether or not concatenation is allowed			
	///	@param attemptDeepen - whether or not deepening is allowed				
	///	@param index - the index at which to insert (if needed)					
	///	@return the number of pushed items (zero if unsuccessful)				
	template<RTTI::ReflectedData T>
	pcptr Block::SmartPush(const T& pack, DState finalState, 
		bool attemptConcat, bool attemptDeepen, Index index) {
		PC_EXTENSIVE_LEAK_TEST(*this);

		static_assert(!Same<T, Block>, 
			"SmartPush Block inside Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - wrap your block in an Any first");

		if constexpr (pcHasBase<T, Block>) {
			// Early exit if nothing to push											
			if (!pack.IsValid())
				return 0;
		}

		// Check if unmovable															
		if (IsStatic()) {
			pcLogFuncError << "Can't smart-push in static data region";
			return 0;
		}

		DMeta meta;
		if constexpr (pcHasBase<T, Block>)
			meta = pack.GetMeta();
		else
			meta = MetaData::Of<T>();

		// If this container is empty and has no conflicting state			
		// do a shallow copy and directly reference data						
		const bool isTypeCompliant = (!IsTypeConstrained() && IsEmpty()) || CanFit(meta);
		const bool isStateCompliant = CanFitState(pack);
		if (IsSparse() == pack.IsSparse() && IsEmpty() && isTypeCompliant && isStateCompliant) {
			const auto type_retainment = (!mType ? meta : mType);
			const auto state_retainment = mState;
			*this = pack;
			Keep();
			ToggleState(DState::Typed, false);
			ToggleState(state_retainment + finalState);

			// Retain type only if original package was sparse, to keep		
			// the initial pointer type												
			if (type_retainment && type_retainment->IsSparse())
				SetDataID(type_retainment, false);
			return 1;
		}

		// If this container is compatible and concatenation is enabled	
		// try concatenating the two containers. Concatenation will not	
		// be allowed if final state is OR, and there are multiple items	
		// in this container.															
		pcptr catenated = 0;
		const bool isOrCompliant = !(mCount > 1 && !IsOr() && finalState & DState::Or);
		if (attemptConcat && isTypeCompliant && isStateCompliant && isOrCompliant 
			&& 0 < (catenated = InsertBlock(pack, index))) {
			ToggleState(finalState);
			return catenated;
		}

		// If this container is deep, directly push the pack inside			
		// This will be disallowed if final state is OR, and there are		
		// multiple items	in this container.										
		if (isOrCompliant && IsDeep() && CanFit<T>()) {
			ToggleState(finalState);
			return Insert<T>(&pack, 1, index);
		}

		// Finally, if allowed, force make the container deep in order to	
		// push the pack inside															
		if constexpr (pcHasBase<T, Block>) {
			if (attemptDeepen && !IsTypeConstrained()) {
				Deepen<T>();
				ToggleState(finalState);
				return SmartPush<T>(pack, {}, attemptConcat, false, index);
			}
		}

		return 0;
	}

	/// Wrap all contained elements inside a sub-block, making this one deep	
	///	@param moveState - whether or not to move the current state over		
	template<RTTI::ReflectedData T>
	T& Block::Deepen(bool moveState) {
		PC_EXTENSIVE_LEAK_TEST(*this);

		static_assert(!Same<T, Block>, 
			"Deepen Block as Block is disallowed - Block is only for internal use, "
			"because there's danger of memory leaks - wrap your block in an Any first");

		static_assert(pcHasBase<T, Block>, 
			"The deepening type must inherit Block");

		if (IsTypeConstrained() && !Is<T>()) {
			throw Except::BadMutation(pcLogFuncError 
				<< "Attempting to deepen incompatible typed container");
		}

		#if LANGULUS_SAFE()
			if (GetBlockReferences() > 1)
				pcLogFuncWarning << "Container used from multiple places";
		#endif

		const auto movedStates = GetUnconstrainedState();
		if (!moveState)
			ToggleState(movedStates, false);

		Block wrapper = Block::From<T>();
		wrapper.Allocate(1, true);
		wrapper.Get<Block>() = pcMove(*this);
		*this = wrapper;
		if (!moveState)
			ToggleState(movedStates);
		PC_EXTENSIVE_LEAK_TEST(*this);
		return Get<T>();
	}

	/// Get an element pointer or reference with a given index						
	/// This is a lower-level routine that does no type checking					
	/// No conversion or copying shall occur in this routine, only pointer		
	/// arithmetic based on CTTI or RTTI													
	///	@param idx - simple index for accessing										
	///	@param baseOffset - byte offset from the element to apply				
	///	@return either pointer or reference to the element (depends on T)		
	template<RTTI::ReflectedData T>
	decltype(auto) Block::Get(const pcptr idx, const pcptr baseOffset) {
		PC_EXTENSIVE_LEAK_TEST(*this);

		// Get pointer via some arithmetic											
		auto pointer = pcP2N(At(mType->GetStride() * idx));
		if (mType->IsSparse()) {
			// Dereference and offset inner pointer								
			pointer = *reinterpret_cast<pcptr*>(pointer) + baseOffset;
		}
		else {
			// Offset pointer																
			pointer += baseOffset;
		}

		if constexpr (Dense<T>)
			return *reinterpret_cast<pcDeref<T>*>(pointer);
		else
			return reinterpret_cast<pcDeref<T>>(pointer);
	}

	/// Get an element with a given index, trying to interpret it as T			
	/// No conversion or copying shall occur in this routine, only pointer		
	/// arithmetic based on CTTI or RTTI													
	///	@param idx - simple index for accessing										
	///	@return either pointer or reference to the element (depends on T)		
	template<RTTI::ReflectedData T>
	decltype(auto) Block::As(const pcptr idx) {
		PC_EXTENSIVE_LEAK_TEST(*this);

		// First quick type stage for fast access									
		if (mType->Is<T>())
			return Get<T>(idx);

		// Second fallback stage for compatible bases and mappings			
		LinkedBase base;
		if (!mType->GetBase<T>(0, base)) {
			// There's still a chance if this container is resolvable		
			// This is the third and final stage									
			auto resolved = GetElementResolved(idx);
			if (resolved.mType->Is<T>()) {
				// Element resolved to a compatible type, so get it			
				return resolved.Get<T>();
			}
			else if (resolved.mType->GetBase<T>(0, base)) {
				// Get base memory of the resolved element and access			
				return resolved.GetBaseMemory(base).Get<T>(idx % base.mStaticBase.mCount);
			}

			// All stages of interpretation failed									
			// Don't log this, because it will spam the crap out of us		
			// That throw is used by ForEach to handle irrelevant types		
			throw Except::BadAccess("Type mismatch on Block::As");
		}

		// Get base memory of the required element and access					
		return 
			GetElementDense(idx / base.mStaticBase.mCount)
				.GetBaseMemory(base)
					.Get<T>(idx % base.mStaticBase.mCount);
	}

	/// Insert any data (including arrays) at the back									
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator << (const T& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if constexpr (pcIsArray<T>)
			Insert<pcDecay<T>>(other, pcExtentOf<T>, uiBack);
		else
			Insert<T>(&other, 1, uiBack);
		return *this;
	}

	/// Insert any data at the back															
	/// Override is required, so that we don't end up moving by accident			
	///	@tparam T - the type of the instance to push									
	///	@param other - the instance to push												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator << (T& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return operator << (const_cast<const T&>(other));
	}

	/// Emplace any data at the back															
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator << (T&& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		Emplace<T>(pcForward<T>(other), uiBack);
		return *this;
	}

	/// Insert any data (including arrays) at the front								
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator >> (const T& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if constexpr (pcIsArray<T>)
			Insert<pcDecay<T>>(other, pcExtentOf<T>, uiFront);
		else
			Insert<T>(&other, 1, uiFront);
		return *this;
	}

	/// Insert any data at the front															
	/// Override is required, so that we don't end up moving by accident			
	///	@tparam T - the type of the instance to push									
	///	@param other - the instance to push												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator >> (T& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return operator >> (const_cast<const T&>(other));
	}

	/// Emplace any data at the front														
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator >> (T&& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		Emplace<T>(pcForward<T>(other), uiFront);
		return *this;
	}

	/// Merge data (including arrays) at the back										
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator <<= (const T& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if constexpr (pcIsArray<T>)
			Merge<pcDecay<T>>(other, pcExtentOf<T>, uiBack);
		else
			Merge<T>(&other, 1, uiBack);
		return *this;
	}

	/// Merge data at the front																
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<RTTI::ReflectedData T>
	Block& Block::operator >>= (const T& other) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if constexpr (pcIsArray<T>)
			Merge<pcDecay<T>>(other, pcExtentOf<T>, uiFront);
		else
			Merge<T>(&other, 1, uiFront);
		return *this;
	}

	template<bool MUTABLE, class FUNCTION>
	pcptr Block::ForEach(FUNCTION&& call) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		using ReturnType = decltype(call(std::declval<ArgumentType>()));
		return ForEachInner<ReturnType, ArgumentType, false, MUTABLE
			>(pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	pcptr Block::ForEach(FUNCTION&& call) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEach<false
			>(pcForward<FUNCTION>(call));
	}

	template<bool MUTABLE, class FUNCTION>
	pcptr Block::ForEachRev(FUNCTION&& call) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		using ReturnType = decltype(call(std::declval<ArgumentType>()));
		return ForEachInner<ReturnType, ArgumentType, true, MUTABLE
			>(pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	pcptr Block::ForEachRev(FUNCTION&& call) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachRev<false
			>(pcForward<FUNCTION>(call));
	}

	template<bool SKIP_DEEP_OR_EMPTY, bool MUTABLE, class FUNCTION>
	pcptr Block::ForEachDeep(FUNCTION&& call) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		using ReturnType = decltype(call(std::declval<ArgumentType>()));
		if constexpr (pcIsDeep<ArgumentType>) {
			// If argument type is deep												
			return ForEachDeepInner<ReturnType, ArgumentType, false, SKIP_DEEP_OR_EMPTY, MUTABLE
				>(pcForward<FUNCTION>(call));
		}
		else {
			// Any other type is wrapped inside another ForEachDeep call	
			return ForEachDeep<SKIP_DEEP_OR_EMPTY, MUTABLE>([&call](Block& block) {
				block.ForEach<MUTABLE, FUNCTION>(pcForward<FUNCTION>(call));
			});
		}
	}

	template<bool SKIP_DEEP_OR_EMPTY, class FUNCTION>
	pcptr Block::ForEachDeep(FUNCTION&& call) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachDeep<SKIP_DEEP_OR_EMPTY, false
			>(pcForward<FUNCTION>(call));
	}

	template<bool SKIP_DEEP_OR_EMPTY, bool MUTABLE, class FUNCTION>
	pcptr Block::ForEachDeepRev(FUNCTION&& call) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		using ReturnType = decltype(call(std::declval<ArgumentType>()));
		if constexpr (pcIsDeep<ArgumentType>) {
			// If argument type is Block												
			return ForEachDeepInner<ReturnType, ArgumentType, true, SKIP_DEEP_OR_EMPTY, MUTABLE
				>(pcForward<FUNCTION>(call));
		}
		else {
			// Any other type is wrapped inside another ForEachDeep call	
			return ForEachDeepRev<SKIP_DEEP_OR_EMPTY, MUTABLE>([&call](Block& block) {
				block.ForEachRev<FUNCTION>(pcForward<FUNCTION>(call));
			});
		}
	}

	template<bool SKIP_DEEP_OR_EMPTY, class FUNCTION>
	pcptr Block::ForEachDeepRev(FUNCTION&& call) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachDeepRev<SKIP_DEEP_OR_EMPTY, false
			>(pcForward<FUNCTION>(call));
	}

	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE, bool MUTABLE>
	pcptr Block::ForEachInner(TFunctor<RETURN(ARGUMENT)>&& call) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		if (IsEmpty())
			return 0;
		 
		UNUSED() auto initialCount = mCount;
		constexpr bool HasBreaker = Same<bool, RETURN>;
		pcptr index = 0;
		if (mType->Is<ARGUMENT>()) {
			// Fast specialized routine that gives direct access				
			// Uses Get<>() instead of As<>()										
			while (index < mCount) {
				// Iterator is a reference												
				if constexpr (Dense<ARGUMENT>) {
					if constexpr (REVERSE) {
						if constexpr (HasBreaker) {
							if (!call(Get<ARGUMENT>(mCount - index - 1)))
								return index + 1;
						}
						else call(Get<ARGUMENT>(mCount - index - 1));
					}
					else {
						if constexpr (HasBreaker) {
							if (!call(Get<ARGUMENT>(index)))
								return index + 1;
						}
						else call(Get<ARGUMENT>(index));
					}
				}
				else {
					if constexpr (REVERSE) {
						auto pointer = Get<ARGUMENT>(mCount - index - 1);
						if constexpr (HasBreaker) {
							if (!call(pointer))
								return index + 1;
						}
						else call(pointer);
					}
					else {
						auto pointer = Get<ARGUMENT>(index);
						if constexpr (HasBreaker) {
							if (!call(pointer))
								return index + 1;
						}
						else call(pointer);
					}
				}

				if constexpr (MUTABLE) {
					// This block might change while iterating - make sure	
					// index compensates for changes									
					if (mCount < initialCount) {
						initialCount = mCount;
						continue;
					}
					else ++index;
				}
				else ++index;
			}

			return index;
		}
		else if (mType->InterpretsAs<ARGUMENT>()) {
			// Slow generalized routine that resolved each element			
			// Uses As<>() instead of Get<>()										
			pcptr successes = 0;
			while (index < mCount) {
				try {
					// Iterator is a reference											
					if constexpr (Dense<ARGUMENT>) {
						if constexpr (REVERSE) {
							if constexpr (HasBreaker) {
								if (!call(As<ARGUMENT>(mCount - index - 1)))
									return successes + 1;
							}
							else call(As<ARGUMENT>(mCount - index - 1));
						}
						else {
							if constexpr (HasBreaker) {
								if (!call(As<ARGUMENT>(index)))
									return successes + 1;
							}
							else call(As<ARGUMENT>(index));
						}
					}
					else {
						if constexpr (REVERSE) {
							auto pointer = As<ARGUMENT>(mCount - index - 1);
							if constexpr (HasBreaker) {
								if (!call(pointer))
									return successes + 1;
							}
							else call(pointer);
						}
						else {
							auto pointer = As<ARGUMENT>(index);
							if constexpr (HasBreaker) {
								if (!call(pointer))
									return successes + 1;
							}
							else call(pointer);
						}
					}
				}
				catch (const Except::BadAccess&) {
					// Don't count bad access exceptions as a success			
					++index;
					continue;
				}

				if constexpr (MUTABLE) {
					// This block might change while iterating - make sure	
					// index compensates for changes									
					if (mCount < initialCount) {
						initialCount = mCount;
						continue;
					}
					else {
						++index;
						++successes;
					}
				}
				else {
					++index;
					++successes;
				}
			}

			return successes;
		}

		return 0;
	}

	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE>
	pcptr Block::ForEachInner(TFunctor<RETURN(ARGUMENT)>&& call) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return const_cast<Block*>(this)->ForEachInner<RETURN, ARGUMENT, REVERSE, false
			>(pcForward<decltype(call)>(call));
	}
	
	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY, bool MUTABLE>
	pcptr Block::ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&& call) {
		PC_EXTENSIVE_LEAK_TEST(*this);
		constexpr bool HasBreaker = Same<bool, RETURN>;
		UNUSED() bool atLeastOneChange = false;
		auto count = GetCountDeep();
		pcptr index = 0;
		while (index < count) {
			auto block = pcReinterpret<ARGUMENT>(GetBlockDeep(index));
			if constexpr (MUTABLE) {
				if (!block)
					break;
			}

			if constexpr (SKIP_DEEP_OR_EMPTY) {
				if (block->IsDeep() || block->IsEmpty()) {
					++index;
					continue;
				}
			}

			UNUSED() const auto initialBlockCount = block->GetCount();
			if constexpr (HasBreaker) {
				if (!call(*block))
					return index + 1;
			}
			else call(*block);

			if constexpr (MUTABLE) {
				// Iterator might be invalid at this point!						
				if (block->GetCount() != initialBlockCount) {
					// Something changes, so do a recalculation					
					if (block->GetCount() < initialBlockCount) {
						// Something was removed, so propagate removal upwards
						// until all empty stateless blocks are removed			
						while (block && block->IsEmpty() && !block->GetUnconstrainedState()) {
							index -= RemoveIndexDeep(index);
							block = pcReinterpret<ARGUMENT>(GetBlockDeep(index - 1));
						}
					}

					count = GetCountDeep();
					atLeastOneChange = true;
				}
			}

			++index;
		}

		if constexpr (MUTABLE) {
			if (atLeastOneChange)
				Optimize();
		}

		return index;
	}

	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY>
	pcptr Block::ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&& call) const {
		PC_EXTENSIVE_LEAK_TEST(*this);
		return const_cast<Block*>(this)->ForEachDeepInner<RETURN, ARGUMENT, REVERSE, false
			>(pcForward<decltype(call)>(call));
	}

} // namespace Langulus::Anyness

/// Start an OR sequence of operations, that relies on short-circuiting to		
/// abort on a successful operation															
#define EitherDoThis [[maybe_unused]] volatile pcptr _____________________sequence = 0 <

/// Add another operation to an OR sequence of operations, relying on			
/// short-circuiting to abort on a successful operation								
#define OrThis || 0 <

/// Return if any of the OR sequence operations succeeded							
#define AndReturnIfDone ; if (_____________________sequence) return

/// Enter scope if any of the OR sequence operations succeeded						
#define AndIfDone ; if (_____________________sequence)
