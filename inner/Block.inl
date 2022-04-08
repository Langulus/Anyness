#pragma once
#include "Block.hpp"

namespace Langulus::Anyness::Inner
{

	/// Manual construction via state and type											
	/// No allocation will happen																
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	constexpr Block::Block(const DataState& state, DMeta meta) noexcept
		: mState {state}
		, mType {meta} { }

	/// Manual construction from constant data											
	/// No referencing shall occur and changes data state to dsConstant			
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the constant memory - safety is on you			
	constexpr Block::Block(const DataState& state, DMeta meta, Count count, const void* raw) noexcept
		: mRaw {const_cast<void*>(raw)}
		, mType {meta}
		, mCount {count}
		, mReserved {count}
		, mState {state.mState | DataState::Constant} { }

	/// Manual construction from mutable data												
	/// No referencing shall occur and changes data state to dsConstant			
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the mutable memory - safety is on you			
	constexpr Block::Block(const DataState& state, DMeta meta, Count count, void* raw) noexcept
		: mRaw {raw}
		, mType {meta}
		, mCount {count}
		, mReserved {count}
		, mState {state} { }

	/// Create a memory block from a typed pointer										
	/// No referencing shall occur, this simply initializes the block				
	///	@return the block																		
	template<ReflectedData T>
	Block Block::From(T value) requires Sparse<T> {
		const Block block {
			DataState::Static,
			MetaData::From<T>(), 
			1, value
		};

		return block;
	}

	///	@return the block																		
	template<ReflectedData T>
	Block Block::From(T value, Count count) requires Sparse<T> {
		const Block block {
			DataState::Static, 
			MetaData::From<T>(), 
			count, value
		};

		return block;
	}

	/// Create a memory block from a value reference									
	/// No referencing shall occur, this simply initializes the block				
	///	@return the block																		
	template<ReflectedData T>
	Block Block::From(T& value) requires Dense<T> {
		if constexpr (!IsText<T> && pcIsDeep<T> && sizeof(T) == sizeof(Block)) {
			return static_cast<const Block&>(value);
		}
		else if constexpr (Resolvable<T>) {
			return value.GetBlock();
		}
		else {
			return Block {
				DataState::Static,
				MetaData::From<T>(), 
				1, &value
			};
		}
	}

	/// Create an empty memory block from a static type								
	///	@return the block																		
	template<ReflectedData T>
	Block Block::From() {
		return Block {
			DataState::Default, 
			MetaData::From<T>(), 
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
	constexpr Block& Block::operator = (const Block& other) noexcept {
		mRaw = other.mRaw;
		mType = other.mType;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		return *this;
	}

	/// Move memory block																		
	/// No referencing will occur - this simply copies and resets the other		
	/// Other will retain its type-constraints after the reset						
	///	@param other - the memory block to move										
	///	@return a reference to this block												
	Block& Block::operator = (Block&& other) noexcept {
		mRaw = other.mRaw;
		mType = other.mType;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState;
		other.ResetInner();
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

		if (IsTypeConstrained()) {
			mState.mState = DataState::Typed;
		}
		else {
			mType = nullptr;
			mState.mState = DataState::Default;
		}
	}

	/// Get the contained type meta definition											
	///	@return the meta data																
	constexpr const DMeta& Block::GetType() const noexcept {
		return mType;
	}

	constexpr const Count& Block::GetCount() const noexcept {
		return mCount;
	}

	/// Get the number of reserved (maybe unconstructed) elements					
	///	@return the number of reserved (probably not constructed) elements	
	constexpr const Count& Block::GetReserved() const noexcept {
		return mReserved;
	}

	/// Check if memory is allocated															
	///	@return true if the block contains any reserved memory					
	constexpr bool Block::IsAllocated() const noexcept {
		return mRaw != nullptr;
	}

	/// Check if block is left-polarized													
	///	@returns true if this container is left-polarized							
	constexpr bool Block::IsPast() const noexcept {
		return GetPhase() == Phase::Past;
	}

	/// Check if block is right-polarized													
	///	@returns true if this container is right-polarized							
	constexpr bool Block::IsFuture() const noexcept {
		return GetPhase() == Phase::Future;
	}

	/// Check if block is not polarized														
	///	@returns true if this container is not polarized							
	constexpr bool Block::IsNow() const noexcept {
		return GetPhase() == Phase::Now;
	}

	/// Check if block is marked as missing												
	///	@returns true if this container is marked as vacuum						
	constexpr bool Block::IsMissing() const noexcept {
		return mState.mState & DataState::Missing;
	}

	/// Check if block has a data type														
	///	@returns true if data contained in this pack is unspecified				
	constexpr bool Block::IsUntyped() const noexcept {
		return !mType;
	}

	/// Check if block has a data type, and is type-constrained						
	///	@return true if type-constrained													
	constexpr bool Block::IsTypeConstrained() const noexcept {
		return mType && (mState.mState & DataState::Typed);
	}

	/// Check if block is polarized															
	///	@returns true if this pack is either left-, or right-polar				
	constexpr bool Block::IsPhased() const noexcept {
		return mState.mState & DataState::Phased;
	}

	/// Check if block is encrypted															
	///	@returns true if the contents of this pack are encrypted					
	constexpr bool Block::IsEncrypted() const noexcept {
		return mState.mState & DataState::Encrypted;
	}

	/// Check if block is compressed															
	///	@returns true if the contents of this pack are compressed				
	constexpr bool Block::IsCompressed() const noexcept {
		return mState.mState & DataState::Compressed;
	}

	/// Check if block is constant															
	///	@returns true if the contents of this pack are constant					
	constexpr bool Block::IsConstant() const noexcept {
		return mState.mState & DataState::Constant;
	}

	/// Check if block is static																
	///	@returns true if the contents of this pack are constant					
	constexpr bool Block::IsStatic() const noexcept {
		return mState.mState & DataState::Static;
	}

	/// Check if block is inhibitory (or) container										
	///	@returns true if this is an inhibitory container							
	constexpr bool Block::IsOr() const noexcept {
		return mState.mState & DataState::Or;
	}

	/// Check if block contains no created elements (it may still have state)	
	///	@returns true if this is an empty container									
	constexpr bool Block::IsEmpty() const noexcept {
		return mCount == 0;
	}

	/// Check if block contains either created elements, or relevant state		
	///	@returns true if this is not an empty stateless container				
	constexpr bool Block::IsValid() const noexcept {
		return mCount || GetUnconstrainedState() || mType;
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
	constexpr Phase Block::GetPhase() const noexcept {
		if (!IsPhased())
			return Phase::Now;
		return mState.mState & DataState::Future ? Phase::Future : Phase::Past;
	}

	/// Set polarity																				
	///	@param p - polarity to enable														
	inline void Block::SetPhase(const Phase p) noexcept {
		switch (p) {
		case Phase::Past:
			mState.mState &= ~DataState::Future;
			mState.mState |= DataState::Phased;
			return;
		case Phase::Now:
			mState.mState &= ~(DataState::Phased | DataState::Future);
			return;
		case Phase::Future:
			mState.mState |= DataState::Future | DataState::Phased;
			return;
		}
	}

	/// Check polarity compatibility															
	///	@param other - the polarity to check											
	///	@return true if polarity is compatible											
	inline bool Block::CanFitPhase(const Phase& other) const noexcept {
		// As long as polarities are not opposite, they are compatible		
		const auto p = GetPhase();
		return int(p) != -int(other) || (other == Phase::Now && p == other);
	}

	/// Check state compatibility																
	///	@param other - the state to check												
	///	@return true if state is compatible												
	inline bool Block::CanFitState(const Block& other) const noexcept {
		const bool sparseness = IsSparse() == other.IsSparse();
		const bool orCompat = IsOr() == other.IsOr() || other.GetCount() <= 1 || IsEmpty();
		const bool typeCompat = !IsTypeConstrained() || (IsTypeConstrained() && other.InterpretsAs(mType));
		return sparseness && typeCompat && (mState == other.mState || (orCompat && CanFitPhase(other.GetPhase())));
	}

	/// Get the size of the contained data, in bytes									
	///	@return the byte size																
	inline Stride Block::GetSize() const noexcept {
		return GetCount() * GetStride();
	}

	/// Check if a type can be inserted														
	template<ReflectedData T>
	bool Block::IsInsertable() const noexcept {
		return IsInsertable(MetaData::Of<T>());
	}

	/// Get the raw data inside the container												
	///	@attention as unsafe as it gets, but as fast as it gets					
	inline Byte* Block::GetRaw() noexcept {
		return mRaw;
	}

	/// Get the raw data inside the container (const)									
	///	@attention as unsafe as it gets, but as fast as it gets					
	inline const Byte* Block::GetRaw() const noexcept {
		return mRaw;
	}

	/// Get the end raw data pointer inside the container								
	///	@attention as unsafe as it gets, but as fast as it gets					
	inline const Byte* Block::GetRawEnd() const noexcept {
		return GetRaw() + GetSize();
	}

	/// Get a constant pointer array - useful for sparse containers				
	///	@return the raw data as an array of constant pointers						
	inline const Byte* const* Block::GetRawSparse() const noexcept {
		return mRawSparse;
	}

	/// Get a pointer array - useful for sparse containers							
	///	@return the raw data as an array of pointers									
	inline Byte** Block::GetRawSparse() noexcept {
		return mRawSparse;
	}

	/// Get the raw data inside the container, reinterpreted as some type		
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<ReflectedData T>
	inline T* Block::GetRawAs() noexcept {
		return reinterpret_cast<T*>(GetRaw());
	}

	/// Get the raw data inside the container, reinterpreted (const)				
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<ReflectedData T>
	inline const T* Block::GetRawAs() const noexcept {
		return reinterpret_cast<const T*>(GetRaw());
	}

	/// Get the end raw data pointer inside the container								
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<ReflectedData T>
	inline const T* Block::GetRawEndAs() const noexcept {
		return reinterpret_cast<const T*>(GetRawEnd());
	}

	/// Get the data state of the container												
	///	@return the current block state													
	constexpr const DataState& Block::GetState() const noexcept {
		return mState;
	}

	/// Set the current data state, overwriting the current							
	/// You can not remove constraints														
	inline void Block::SetState(DataState state) noexcept {
		mState.mState = state.mState | (mState.mState & DataState::Constrained);
	}

	/// Get the relevant state when relaying one block	to another					
	/// Relevant states exclude memory and type constraints							
	constexpr DataState Block::GetUnconstrainedState() const noexcept {
		return mState.mState & ~DataState::Constrained;
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
	inline Byte* Block::At(Offset byte_offset) {
		#if LANGULUS_SAFE()
			if (!mRaw) {
				throw Except::Access(Logger::Error()
					<< "Byte offset in invalid memory of type " << GetToken());
			}
		#endif
		return GetRaw() + byte_offset;
	}

	inline const Byte* Block::At(Offset byte_offset) const {
		return const_cast<Block&>(*this).At(byte_offset);
	}

	/// Get templated element																	
	/// Checks only density																		
	template<ReflectedData T>
	decltype(auto) Block::Get(Offset idx, Offset baseOffset) const {
		return const_cast<Block&>(*this).Get<T>(idx, baseOffset);
	}

	/// Get templated element																	
	/// Checks density and type																
	template<ReflectedData T>
	decltype(auto) Block::As(Offset idx) const {
		return const_cast<Block&>(*this).As<T>(idx);
	}

	/// Get templated element																	
	/// Checks range, density, type and special indices								
	template<ReflectedData T>
	decltype(auto) Block::As(Index index) {
		index = ConstrainMore<T>(index);
		if (index.IsSpecial()) {
			throw Except::Access(Logger::Error()
				<< "Can't reference special index");
		}

		return As<T>(pcptr(index.mIndex));
	}

	template<ReflectedData T>
	decltype(auto) Block::As(Index index) const {
		return const_cast<Block&>(*this).As<T>(index);
	}

	/// Check if a pointer is anywhere inside the block's memory					
	///	@param ptr - the pointer to check												
	///	@return true if inside the memory block										
	inline bool Block::Owns(const void* ptr) const noexcept {
		return ptr >= GetRaw() && ptr < GetRawEnd();
	}

	/// Decay the block to some base type, if sizes are compatible					
	///	@tparam T - the base type to decay to											
	///	@return the decayed block															
	template<ReflectedData T>
	Block Block::Decay() const {
		return Decay(MetaData::Of<T>());
	}

	/// Mutate the block to a different type, if possible								
	///	@tparam T - the type to change to												
	///	@return true if block was deepened to incorporate the new type			
	template<ReflectedData T>
	bool Block::Mutate() {
		return Mutate(MetaData::Of<T>());
	}

	/// Reserve a number of elements															
	///	@tparam T - the type of elements to allocate									
	///	@param count - the number of elements to reserve							
	///	@param construct - true to call default constructors of each element	
	///	@param setcount - true to set the count, too									
	///	@attention beware of blocks with unconstructed elements, but with		
	///				  set count - could cause undefined behavior						
	template<ReflectedData T>
	void Block::Allocate(Count count, bool construct, bool setcount) {
		SetType<T>(false);
		Allocate(count, construct, setcount);
	}

	/// Constrain an index to the limits of the current block						
	///	@param idx - the index to constrain												
	///	@return the constrained index or a special one of constrain fails		
	constexpr Index Block::Constrain(const Index& idx) const noexcept {
		switch (idx.mIndex) {
		case Index::Auto: case Index::First: case Index::Front:
			return Index {0};
		case Index::All: case Index::Back:
			return Index {mCount};
		case Index::Last:
			return mCount ? Index {mCount - 1} : Index::None;
		case Index::Middle:
			return mCount / 2;
		case Index::None:
			return Index::None;
		}
		
		return idx.Constrained(mCount);
	}

	/// Constrain an index to the limits of the current block						
	/// Supports additional type-dependent constraints									
	///	@param idx - the index to constrain												
	///	@return the constrained index or a special one of constrain fails		
	template<ReflectedData T>
	Index Block::ConstrainMore(const Index& idx) const noexcept {
		const auto result = Constrain(idx);
		if (result.IsSpecial()) {
			switch (result.mIndex) {
			case Index::Biggest:
				if constexpr (Sortable<T>)
					return GetIndexMax<T>();
				else return Index::None;
				break;
			case Index::Smallest:
				if constexpr (Sortable<T>)
					return GetIndexMin<T>();
				else return Index::None;
				break;
			case Index::Mode:
				if constexpr (Sortable<T>) {
					pcptr unused;
					return GetIndexMode<T>(unused);
				}
				else return Index::None;
				break;
			}
		}
		return result;
	}

	template<ReflectedData T>
	bool Block::CanFit() const {
		return CanFit(MetaData::Of<T>());
	}

	template<ReflectedData T>
	bool Block::InterpretsAs() const {
		return InterpretsAs(MetaData::Of<T>());
	}

	template<ReflectedData T>
	bool Block::InterpretsAs(Count count) const {
		return InterpretsAs(MetaData::Of<T>(), count);
	}

	template<ReflectedData T>
	bool Block::Is() const {
		return Is(MetaData::Of<T>);
	}

	/// Swap two elements (with raw indices)												
	///	@param from - first element index												
	///	@param to - second element index													
	template<ReflectedData T>
	void Block::Swap(Offset from, Offset to) {
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
	template<ReflectedData T>
	void Block::Swap(Index from, Index to) {
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
	template<ReflectedData T, bool MUTABLE>
	Count Block::Emplace(T&& item, const Index& index) {
		// Check errors before actually doing anything							
		if constexpr (Sparse<T>) {
			if (!item) {
				throw Except::Reference(Logger::Error()
					<< "Move-insertion of a null pointer of type "
					<< GetToken() << " is not allowed");
			}
		}

		// Constrain index																
		const auto starter = ConstrainMore<T>(index).GetOffset();

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
				throw Except::Reference(Logger::Error()
					<< "Moving elements that are used from multiple places"));

			CropInner(starter + 1, mCount - starter)
				.CallMoveConstructors(
					CropInner(starter, mCount - starter));
		}

		// Insert new data																
		if constexpr (Sparse<T>) {
			// Sparse data insertion (moving a pointer)							
			auto data = GetRawSparse() + starter;
			data = reinterpret_cast<Byte*>(item);

			// Reference the pointer's memory										
			PCMEMORY.Reference(mType, item, 1);
		}
		else {
			static_assert(!pcIsAbstract<T>, "Can't emplace abstract item");

			// Dense data insertion (placement move-construction)				
			auto data = GetRaw() + starter * sizeof(T);
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
	template<ReflectedData T, bool MUTABLE>
	Count Block::Insert(const T* items, const Count count, const Index& index) {
		// Constrain index																
		const auto starter = ConstrainMore<T>(index).GetOffset();

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
				throw Except::Reference(Logger::Error()
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
			Count c {};
			while (c < count) {
				if (!items[c]) {
					throw Except::Reference(Logger::Error()
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
	template<ReflectedData T>
	Count Block::Remove(const T* items, const Count count, const Index& index) {
		Count removed {};
		for (Count i = 0; i < count; ++i) {
			const auto idx = Find<T>(items[i], index);
			if (idx)
				removed += RemoveIndex(static_cast<pcptr>(idx), 1);
		}

		return removed;
	}

	/// Find first matching element position inside container						
	///	@param item - the item to search for											
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	template<ReflectedData T>
	Index Block::Find(const T& item, const Index& idx) const {
		if (!mCount || !mType)
			return Index::None;

		if (IsDense()) {
			if (!InterpretsAs<T>()) {
				// If dense and not forward compatible - fail					
				return Index::None;
			}
		}
		else if (!MetaData::Of<T>()->InterpretsAs(mType)) {
			// If sparse and not backwards compatible - fail					
			return Index::None;
		}

		// Setup the iterator															
		Index starti, istep;
		switch (idx.mIndex) {
		case Index::Front:
			starti = 0;
			istep = 1;
			break;
		case Index::Back:
			starti = mCount - 1;
			istep = -1;
			break;
		default:
			starti = Constrain(idx);
			istep = 1;
			if (starti + 1 >= mCount)
				return Index::None;
		}

		// Search																			
		auto item_ptr = pcPtr(item);
		for (auto i = starti; i < mCount && i >= 0; i += istep) {
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
						Logger::Warning() << "Dynamically resolved type " << lhs.GetToken()
							<< " missing compare operator implementation and/or reflection";
						Logger::Warning() << "This means that any Find, Merge, "
							"Select, etc. will fail for that type";
						Logger::Warning() << "Implement operator == and != either in type "
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
		return Index::None;
	}
	
	/// Find first matching element position inside container, deeply				
	///	@param item - the item to search for											
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	template<ReflectedData T>
	Index Block::FindDeep(const T& item, const Index& idx) const {
		Index found;
		ForEachDeep([&](const Block& group) {
			found = group.Find<T>(item, idx);
			return !found;
		});

		return found;
	}

	/// Merge array elements inside container.											
	/// Pushes only if element(s) was not found											
	///	@param items - the items to push													
	///	@param count - number of items to push											
	///	@param idx - use uiFront or uiBack for pushing to ends					
	///	@return the number of inserted elements										
	template<ReflectedData T, bool MUTABLE>
	Count Block::Merge(const T* items, const Count count, const Index& idx) {
		Count added {};
		for (Count i = 0; i < count; ++i) {
			if (!Find<T>(items[i]))
				added += Insert<T, MUTABLE>(items + i, 1, idx);
		}

		return added;
	}

	/// Get the index of the biggest dense element										
	template<ReflectedData T>
	Index Block::GetIndexMax() const noexcept requires Sortable<T> {
		if (IsEmpty())
			return Index::None;

		auto data = Get<Decay<T>*>();
		auto max = data;
		for (Count i = 1; i < mCount; ++i) {
			if (*pcPtr(data[i]) > *pcPtr(*max))
				max = data + i;
		}

		return max - data;
	}

	/// Get the index of the smallest dense element										
	template<ReflectedData T>
	Index Block::GetIndexMin() const noexcept requires Sortable<T> {
		if (IsEmpty())
			return Index::None;

		auto data = Get<Decay<T>*>();
		auto min = data;
		for (Count i = 1; i < mCount; ++i) {
			if (*pcPtr(data[i]) < *pcPtr(*min))
				min = data + i;
		}

		return min - data;
	}

	/// Get the index of dense element that repeats the most times					
	///	@param count - [out] count the number of repeats for the mode			
	///	@return the index of the first found mode										
	template<ReflectedData T>
	Index Block::GetIndexMode(Count& count) const noexcept {
		if (IsEmpty()) {
			count = 0;
			return Index::None;
		}

		auto data = Get<Decay<T>*>();
		decltype(data) best = nullptr;
		Count best_count {};
		for (Count i = 0; i < mCount; ++i) {
			Count counter {};
			for (Count j = i; j < mCount; ++j) {
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
		return best - data;
	}

	/// Sort the contents of this container using a static type						
	///	@param first - what will the first element be after sorting?			
	///					 - use uiSmallest for 123, or anything else for 321		
	template<ReflectedData T>
	void Block::Sort(const Index& first) noexcept {
		auto data = Get<Decay<T>*>();
		if (!data)
			return;

		Count j {}, i {};
		if (first == Index::Smallest) {
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
	template<ReflectedData T>
	Count Block::SmartPush(const T& pack, DataState finalState, bool attemptConcat, bool attemptDeepen, Index index) {
		if constexpr (Deep<T>) {
			// Early exit if nothing to push											
			if (!pack.IsValid())
				return 0;
		}

		// Check if unmovable															
		if (IsStatic()) {
			Logger::Error() << "Can't smart-push in static data region";
			return 0;
		}

		DMeta meta;
		if constexpr (Deep<T>)
			meta = pack.GetType();
		else
			meta = MetaData::Of<T>();

		// If this container is empty and has no conflicting state			
		// do a shallow copy and directly reference data						
		const bool isTypeCompliant = (!IsTypeConstrained() && IsEmpty()) || CanFit(meta);
		const bool isStateCompliant = CanFitState(pack);
		if (IsEmpty() && isTypeCompliant && isStateCompliant) {
			const auto type_retainment = !mType ? meta : mType;
			const auto state_retainment = mState;
			*this = pack;
			Keep();
			ToggleState(DataState::Typed, false);
			ToggleState(state_retainment + finalState);

			// Retain type only if original package was sparse, to keep		
			// the initial pointer type												
			if (IsSparse())
				SetType(type_retainment, false);
			return 1;
		}

		// If this container is compatible and concatenation is enabled	
		// try concatenating the two containers. Concatenation will not	
		// be allowed if final state is OR, and there are multiple items	
		// in this container.															
		Count catenated {};
		const bool isOrCompliant = !(mCount > 1 && !IsOr() && finalState.mState & DataState::Or);
		if (attemptConcat && isTypeCompliant && isStateCompliant && isOrCompliant && 0 < (catenated = InsertBlock(pack, index))) {
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
	template<Deep T>
	T& Block::Deepen(bool moveState) {
		if (IsTypeConstrained() && !Is<T>()) {
			throw Except::Mutate(Logger::Error()
				<< "Attempting to deepen incompatible typed container");
		}

		#if LANGULUS_SAFE()
			if (GetBlockReferences() > 1)
				Logger::Warning() << "Container used from multiple places";
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

		return Get<T>();
	}

	/// Get an element pointer or reference with a given index						
	/// This is a lower-level routine that does no type checking					
	/// No conversion or copying shall occur in this routine, only pointer		
	/// arithmetic based on CTTI or RTTI													
	///	@param idx - simple index for accessing										
	///	@param baseOffset - byte offset from the element to apply				
	///	@return either pointer or reference to the element (depends on T)		
	template<ReflectedData T>
	decltype(auto) Block::Get(const Offset idx, const Offset baseOffset) {
		Byte* pointer;
		if (IsSparse())
			pointer = GetRawSparse() + baseOffset;
		else
			pointer = At(mType->GetStride() * idx) + baseOffset;

		if constexpr (Dense<T>)
			return *reinterpret_cast<Deref<T>*>(pointer);
		else
			return reinterpret_cast<Deref<T>>(pointer);
	}

	/// Get an element with a given index, trying to interpret it as T			
	/// No conversion or copying shall occur in this routine, only pointer		
	/// arithmetic based on CTTI or RTTI													
	///	@param idx - simple index for accessing										
	///	@return either pointer or reference to the element (depends on T)		
	template<ReflectedData T>
	decltype(auto) Block::As(const Offset idx) {
		// First quick type stage for fast access									
		if (mType->Is<T>())
			return Get<T>(idx);

		// Second fallback stage for compatible bases and mappings			
		Base base;
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
			throw Except::Access("Type mismatch on Block::As");
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
	template<ReflectedData T>
	Block& Block::operator << (const T& other) {
		if constexpr (Array<T>)
			Insert<Decay<T>>(other, pcExtentOf<T>, Index::Back);
		else
			Insert<T>(&other, 1, Index::Back);
		return *this;
	}

	/// Insert any data at the back															
	/// Override is required, so that we don't end up moving by accident			
	///	@tparam T - the type of the instance to push									
	///	@param other - the instance to push												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Block& Block::operator << (T& other) {
		return operator << (const_cast<const T&>(other));
	}

	/// Emplace any data at the back															
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Block& Block::operator << (T&& other) {
		Emplace<T>(pcForward<T>(other), Index::Back);
		return *this;
	}

	/// Insert any data (including arrays) at the front								
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Block& Block::operator >> (const T& other) {
		if constexpr (Array<T>)
			Insert<Decay<T>>(other, pcExtentOf<T>, Index::Front);
		else
			Insert<T>(&other, 1, Index::Front);
		return *this;
	}

	/// Insert any data at the front															
	/// Override is required, so that we don't end up moving by accident			
	///	@tparam T - the type of the instance to push									
	///	@param other - the instance to push												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Block& Block::operator >> (T& other) {
		return operator >> (const_cast<const T&>(other));
	}

	/// Emplace any data at the front														
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Block& Block::operator >> (T&& other) {
		Emplace<T>(pcForward<T>(other), Index::Front);
		return *this;
	}

	/// Merge data (including arrays) at the back										
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Block& Block::operator <<= (const T& other) {
		if constexpr (Array<T>)
			Merge<Decay<T>>(other, pcExtentOf<T>, Index::Back);
		else
			Merge<T>(&other, 1, Index::Back);
		return *this;
	}

	/// Merge data at the front																
	///	@param other - the data to insert												
	///	@return a reference to this memory block for chaining						
	template<ReflectedData T>
	Block& Block::operator >>= (const T& other) {
		if constexpr (Array<T>)
			Merge<Decay<T>>(other, pcExtentOf<T>, Index::Front);
		else
			Merge<T>(&other, 1, Index::Front);
		return *this;
	}

	template<bool MUTABLE, class FUNCTION>
	Count Block::ForEach(FUNCTION&& call) {
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		using ReturnType = decltype(call(std::declval<ArgumentType>()));
		return ForEachInner<ReturnType, ArgumentType, false, MUTABLE
			>(pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	Count Block::ForEach(FUNCTION&& call) const {
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEach<false
			>(pcForward<FUNCTION>(call));
	}

	template<bool MUTABLE, class FUNCTION>
	Count Block::ForEachRev(FUNCTION&& call) {
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		using ReturnType = decltype(call(std::declval<ArgumentType>()));
		return ForEachInner<ReturnType, ArgumentType, true, MUTABLE
			>(pcForward<FUNCTION>(call));
	}

	template<class FUNCTION>
	Count Block::ForEachRev(FUNCTION&& call) const {
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachRev<false
			>(pcForward<FUNCTION>(call));
	}

	template<bool SKIP_DEEP_OR_EMPTY, bool MUTABLE, class FUNCTION>
	Count Block::ForEachDeep(FUNCTION&& call) {
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
	Count Block::ForEachDeep(FUNCTION&& call) const {
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachDeep<SKIP_DEEP_OR_EMPTY, false
			>(pcForward<FUNCTION>(call));
	}

	template<bool SKIP_DEEP_OR_EMPTY, bool MUTABLE, class FUNCTION>
	Count Block::ForEachDeepRev(FUNCTION&& call) {
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
	Count Block::ForEachDeepRev(FUNCTION&& call) const {
		using ArgumentType = decltype(GetLambdaArgument(&FUNCTION::operator()));
		static_assert(Constant<ArgumentType>, 
			"Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachDeepRev<SKIP_DEEP_OR_EMPTY, false
			>(pcForward<FUNCTION>(call));
	}

	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class RETURN, ReflectedData ARGUMENT, bool REVERSE, bool MUTABLE>
	Count Block::ForEachInner(TFunctor<RETURN(ARGUMENT)>&& call) {
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
	template<class RETURN, ReflectedData ARGUMENT, bool REVERSE>
	Count Block::ForEachInner(TFunctor<RETURN(ARGUMENT)>&& call) const {
		return const_cast<Block*>(this)->ForEachInner<RETURN, ARGUMENT, REVERSE, false
			>(pcForward<decltype(call)>(call));
	}
	
	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class RETURN, ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY, bool MUTABLE>
	Count Block::ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&& call) {
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
	template<class RETURN, ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY>
	Count Block::ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&& call) const {
		return const_cast<Block*>(this)->ForEachDeepInner<RETURN, ARGUMENT, REVERSE, false
			>(pcForward<decltype(call)>(call));
	}

} // namespace Langulus::Anyness

