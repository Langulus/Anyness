#pragma once
#include "Block.hpp"
#include <cstring>

namespace Langulus::Anyness
{
	
	/// Copy-construction from constant block												
	///	@param other - the block instance to move										
	constexpr Block::Block(const Block& other) noexcept
		: Block {const_cast<Block&>(other)} {
		MakeConstant();
	}
	
	/// Manual construction via type															
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	constexpr Block::Block(DMeta meta) noexcept
		: mType {meta} { }

	/// Manual construction via state and type											
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	constexpr Block::Block(const DataState& state, DMeta meta) noexcept
		: mState {state}
		, mType {meta} { }
	
	/// Manual construction from mutable data												
	/// This constructor has a slight runtime overhead, due to unknown Entry	
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the mutable memory									
	inline Block::Block(const DataState& state, DMeta meta, Count count, void* raw) noexcept
		: mRaw {static_cast<Byte*>(raw)}
		, mType {meta}
		, mCount {count}
		, mReserved {count}
		, mState {state}
		, mEntry {Allocator::Find(meta, raw)} { }
	
	/// Manual construction from constant data											
	/// This constructor has a slight runtime overhead, due to unknown Entry	
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the constant memory									
	inline Block::Block(const DataState& state, DMeta meta, Count count, const void* raw) noexcept
		: Block {state, meta, count, const_cast<void*>(raw)} {
		MakeConstant();
	}

	/// Manual construction from mutable data and preallocated entry				
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the mutable memory									
	///	@param entry - the memory entry													
	inline Block::Block(const DataState& state, DMeta meta, Count count, void* raw, Entry* entry) noexcept
		: mRaw {static_cast<Byte*>(raw)}
		, mType {meta}
		, mCount {count}
		, mReserved {count}
		, mState {state}
		, mEntry {entry} { }
	
	/// Manual construction from constant data and preallocated entry				
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the constant memory									
	///	@param entry - the memory entry													
	inline Block::Block(const DataState& state, DMeta meta, Count count, const void* raw, Entry* entry) noexcept
		: Block {state, meta, count, const_cast<void*>(raw), entry} {
		MakeConstant();
	}

	/// Create a memory block from a single typed pointer								
	///	@tparam T - the type of the value to wrap (deducible)						
	///	@tparam CONSTRAIN - makes container type-constrained						
	///	@return the block																		
	template<ReflectedData T, bool CONSTRAIN>
	Block Block::From(T value) requires Langulus::IsSparse<T> {
		if constexpr (CONSTRAIN)
			return {DataState::Member, MetaData::Of<T>(), 1, value};
		else
			return {DataState::Static, MetaData::Of<T>(), 1, value};		
	}

	/// Create a memory block from a count-terminated array							
	///	@tparam T - the type of the value to wrap (deducible)						
	///	@tparam CONSTRAIN - makes container type-constrained						
	///	@return the block																		
	template<ReflectedData T, bool CONSTRAIN>
	Block Block::From(T value, Count count) requires Langulus::IsSparse<T> {
		if constexpr (CONSTRAIN)
			return {DataState::Member, MetaData::Of<T>(), count, value};
		else
			return {DataState::Static, MetaData::Of<T>(), count, value};
	}

	/// Create a memory block from a value reference									
	/// If value is resolvable, GetBlock() will produce the Block					
	/// If value is deep, T will be down-casted to Block								
	/// Anything else will be interfaced via a new Block (without referencing)	
	///	@tparam T - the type of the value to wrap (deducible)						
	///	@tparam CONSTRAIN - makes container type-constrained						
	///	@return a block that wraps a dense value										
	template<ReflectedData T, bool CONSTRAIN>
	Block Block::From(T& value) requires Langulus::IsDense<T> {
		Block result;
		if constexpr (Langulus::IsResolvable<T>)
			result = value.GetBlock();
		else if constexpr (Anyness::IsDeep<T>)
			result = static_cast<const Block&>(value);
		else
			result = {DataState::Static, MetaData::Of<Decay<T>>(), 1, &value};
		
		if constexpr (CONSTRAIN)
			result.MakeTypeConstrained();
		return result;
	}

	/// Create an empty memory block from a static type								
	///	@tparam T - the type of the value to wrap (deducible)						
	///	@tparam CONSTRAIN - makes container type-constrained						
	///	@return the block																		
	template<ReflectedData T, bool CONSTRAIN>
	Block Block::From() {
		if constexpr (CONSTRAIN)
			return {DataState::Typed, MetaData::Of<T>()};
		else
			return {MetaData::Of<T>()};
	}

	/// Reference memory block																	
	///	@param times - number of references to add									
	inline void Block::Reference(const Count& times) noexcept {
		if (!mEntry)
			// We don't own the data - don't touch it								
			return;

		mEntry->mReferences += times;
	}
	
	/// Reference memory block (const)														
	///	@param times - number of references to add									
	inline void Block::Reference(const Count& times) const noexcept {
		const_cast<Block&>(*this).Reference(times);
	}
	
	/// Reference memory block once															
	///	@return the remaining references for the block								
	inline void Block::Keep() noexcept {
		Reference(1);
	}
	
	/// Reference memory block once (const)												
	///	@return the remaining references for the block								
	inline void Block::Keep() const noexcept {
		const_cast<Block&>(*this).Keep();
	}
			
	/// Dereference memory block																
	/// Upon full dereference, element destructors are called if DESTROY			
	/// It is your responsibility to clear your Block after that					
	///	@param times - number of references to subtract								
	///	@return true if entry has been deallocated 									
	template<bool DESTROY>
	bool Block::Dereference(const Count& times) {
		if (!mEntry)
			// Data is either static or unallocated - don't touch it			
			return false;

		if (mEntry->mReferences <= times) {
			// Destroy all elements and deallocate the entry					
			if constexpr (DESTROY)
				CallDestructors();
			Allocator::Deallocate(mType, mEntry);
			return true;
		}

		mEntry->mReferences -= times;
		return false;
	}

	/// Clear the block, only zeroing its size											
	constexpr void Block::ClearInner() noexcept {
		mCount = 0;
	}

	/// Reset the memory inside the block													
	constexpr void Block::ResetMemory() noexcept {
		mRaw = nullptr;
		mEntry = nullptr;
		mCount = mReserved = 0;
	}
	
	/// Reset the block's state																
	template<bool TYPED>
	constexpr void Block::ResetState() noexcept {
		if constexpr (TYPED) {
			// DON'T clear type, and restore typed state							
			mState = DataState::Typed;
		}
		else {
			// Clear both type and state												
			mType = nullptr;
			mState.Reset();
		}
	}
	
	/// Allocate a number of elements, relying on the type of the container		
	///	@tparam CREATE - true to call constructors									
	///	@param elements - number of elements to allocate							
	template<bool CREATE>
	void Block::Allocate(Count elements) {
		if (!mType) {
			throw Except::Allocate(Logger::Error()
				<< "Attempting to allocate " << elements 
				<< " element(s) of an invalid type");
		}
		else if (mType->mIsAbstract) {
			throw Except::Allocate(Logger::Error()
				<< "Attempting to allocate " << elements 
				<< " element(s) of abstract type " << GetToken());
		}

		if (mCount > elements) {
			// Destroy back entries on smaller allocation						
			RemoveIndex(elements, mCount - elements);
			return;
		}

		if (mReserved >= elements) {
			// Required memory is already available								
			if constexpr (CREATE) {
				// But is not yet initialized, so initialize it					
				if (mCount < elements)
					CallDefaultConstructors(elements - mCount);
			}
			
			return;
		}
		
		// Retrieve the required byte size											
		const Size byteSize = GetStride() * elements;
		
		// Allocate/reallocate															
		if (mEntry) {
			// Reallocate																	
			Block previousBlock {*this};
			if (mEntry->mReferences == 1) {
				// Memory is used only once and it is safe to move it			
				// Make note, that Allocator::Reallocate doesn't copy			
				// anything, it doesn't use realloc for various reasons, so	
				// we still have to call move construction for all elements	
				// if entry moved (enabling MANAGED_MEMORY feature				
				// significantly reduces the possiblity for a move)			
				// Also, make sure to free the previous mEntry if moved		
				mEntry = Allocator::Reallocate(byteSize, mEntry);
				if (mEntry != previousBlock.mEntry) {
					// Memory moved, and we should call move-construction		
					mRaw = mEntry->GetBlockStart();
					mCount = 0;
					CallMoveConstructors(Move(previousBlock));
					previousBlock.Free();
				}
				
				if constexpr (CREATE) {
					// Default-construct the rest										
					CallDefaultConstructors(elements - mCount);
				}
			}
			else {
				// Memory is used from multiple locations, and we must		
				// copy the memory for this block - we can't move it!			
				mEntry = Allocator::Allocate(byteSize);
				mRaw = mEntry->GetBlockStart();
				mCount = 0;
				CallCopyConstructors(previousBlock);
				previousBlock.Free();
				
				if constexpr (CREATE) {
					// Default-construct the rest										
					CallDefaultConstructors(elements - mCount);
				}
			}
		}
		else {
			// Allocate a fresh set of elements										
			mEntry = Allocator::Allocate(byteSize);			
			mRaw = mEntry->GetBlockStart();
			if constexpr (CREATE) {
				// Default-construct everything										
				CallDefaultConstructors(elements);
			}
		}
		
		mReserved = elements;
		return;
	}
	
	/// Check if a range is inside the block												
	/// This function throws on error only if LANGULUS(SAFE) is enabled			
	inline void Block::CheckRange(const Offset& start, const Count& count) const {
		#if LANGULUS_SAFE()
			if (start > mCount) {
				throw Except::Access(Logger::Error()
					<< "Crop left offset is out of limits");
			}
			
			if (start + count > mCount) {
				throw Except::Access(Logger::Error()
					<< "Crop count is out of limits");
			}
		#endif
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
	
	/// Check if we have jurisdiction over the contained memory						
	///	@return true if memory is under our authority								
	constexpr bool Block::HasAuthority() const noexcept {
		return mEntry != nullptr;
	}
	
	/// Get the number of references for the allocated memory block				
	///	@attention always returns 1 if memory is outside authority				
	///	@return the references for the memory block									
	constexpr Count Block::GetReferences() const noexcept {
		return mEntry ? mEntry->mReferences : 1;
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
		return mState & DataState::Missing;
	}

	/// Check if block has a data type														
	///	@returns true if data contained in this pack is unspecified				
	constexpr bool Block::IsUntyped() const noexcept {
		return !mType;
	}

	/// Check if block has a data type, and is type-constrained						
	///	@return true if type-constrained													
	constexpr bool Block::IsTypeConstrained() const noexcept {
		return mType && (mState & DataState::Typed);
	}

	/// Check if block is polarized															
	///	@returns true if this pack is either left-, or right-polar				
	constexpr bool Block::IsPhased() const noexcept {
		return mState & DataState::Phased;
	}

	/// Check if block is encrypted															
	///	@returns true if the contents of this pack are encrypted					
	constexpr bool Block::IsEncrypted() const noexcept {
		return mState & DataState::Encrypted;
	}

	/// Check if block is compressed															
	///	@returns true if the contents of this pack are compressed				
	constexpr bool Block::IsCompressed() const noexcept {
		return mState & DataState::Compressed;
	}

	/// Check if block is constant															
	///	@returns true if the contents are immutable									
	constexpr bool Block::IsConstant() const noexcept {
		return mState & DataState::Constant;
	}

	/// Check if block is static																
	///	@returns true if the contents are static (size-constrained)				
	constexpr bool Block::IsStatic() const noexcept {
		return mRaw && ((mState & DataState::Static) || !mEntry);
	}

	/// Check if block is inhibitory (or) container										
	///	@returns true if this is an inhibitory container							
	constexpr bool Block::IsOr() const noexcept {
		return mState & DataState::Or;
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
	constexpr bool Block::IsDense() const {
		return !IsSparse();
	}
	
	/// Make memory block vacuum (a.k.a. missing)										
	constexpr void Block::MakeMissing() noexcept {
		mState += DataState::Missing;
	}

	/// Make memory block static (unmovable and unresizable)							
	constexpr void Block::MakeStatic() noexcept {
		mState += DataState::Static;
	}

	/// Make memory block constant															
	constexpr void Block::MakeConstant() noexcept {
		mState += DataState::Constant;
	}

	/// Make memory block type-immutable													
	constexpr void Block::MakeTypeConstrained() noexcept {
		mState += DataState::Typed;
	}

	/// Make memory block exlusive (a.k.a. OR container)								
	constexpr void Block::MakeOr() noexcept {
		mState += DataState::Or;
	}

	/// Make memory block inclusive (a.k.a. AND container)							
	constexpr void Block::MakeAnd() noexcept {
		mState -= DataState::Or;
	}

	/// Set memory block phase to past														
	constexpr void Block::MakePast() noexcept {
		SetPhase(Phase::Past);
	}

	/// Set memory block phase to future													
	constexpr void Block::MakeFuture() noexcept {
		SetPhase(Phase::Future);
	}
	
	/// Set sparseness																			
	constexpr void Block::MakeSparse() noexcept {
		mState += DataState::Sparse;
	}
	
	/// Get polarity																				
	///	@return the polarity of the contained data									
	constexpr Phase Block::GetPhase() const noexcept {
		if (!IsPhased())
			return Phase::Now;
		
		return mState & DataState::Future 
			? Phase::Future 
			: Phase::Past;
	}

	/// Set polarity																				
	///	@param p - polarity to enable														
	constexpr void Block::SetPhase(const Phase p) noexcept {
		switch (p) {
		case Phase::Past:
			mState -= DataState::Future;
			mState += DataState::Past;
			return;
		case Phase::Now:
			mState -= DataState::Future;
			mState += DataState::Phased;
			return;
		case Phase::Future:
			mState += DataState::Future;
			return;
		}
	}

	/// Check polarity compatibility															
	///	@param other - the polarity to check											
	///	@return true if polarity is compatible											
	constexpr bool Block::CanFitPhase(const Phase& other) const noexcept {
		// As long as polarities are not opposite, they are compatible		
		const auto p = GetPhase();
		return int(p) != -int(other) || (other == Phase::Now && p == other);
	}

	/// Check state compatibility																
	///	@param other - the state to check												
	///	@return true if state is compatible												
	constexpr bool Block::CanFitState(const Block& other) const noexcept {
		const bool sparseness = IsSparse() == other.IsSparse();
		const bool orCompat = IsOr() == other.IsOr() || other.GetCount() <= 1 || IsEmpty();
		const bool typeCompat = !IsTypeConstrained() || (IsTypeConstrained() && other.CastsToMeta(mType));
		return sparseness && typeCompat && (mState == other.mState || (orCompat && CanFitPhase(other.GetPhase())));
	}

	/// Get the size of the contained data, in bytes									
	///	@return the byte size																
	constexpr Size Block::GetSize() const noexcept {
		return GetCount() * GetStride();
	}

	/// Check if a type can be inserted														
	template<ReflectedData T>
	bool Block::IsInsertable() const noexcept {
		return IsInsertable(MetaData::Of<Decay<T>>());
	}

	/// Get the raw data inside the container												
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr Byte* Block::GetRaw() noexcept {
		return mRaw;
	}

	/// Get the raw data inside the container (const)									
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr const Byte* Block::GetRaw() const noexcept {
		return mRaw;
	}

	/// Get the end raw data pointer inside the container								
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr Byte* Block::GetRawEnd() noexcept {
		return GetRaw() + GetSize();
	}

	/// Get the end raw data pointer inside the container (const)					
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr const Byte* Block::GetRawEnd() const noexcept {
		return GetRaw() + GetSize();
	}

	/// Get a constant pointer array - useful for sparse containers (const)		
	///	@return the raw data as an array of constant pointers						
	constexpr const Byte* const* Block::GetRawSparse() const noexcept {
		return mRawSparse;
	}

	/// Get a pointer array - useful for sparse containers							
	///	@return the raw data as an array of pointers									
	constexpr Byte** Block::GetRawSparse() noexcept {
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
	
	/// Check if contained type is abstract												
	///	@returns true if the type of this pack is abstract							
	constexpr bool Block::IsAbstract() const noexcept {
		return mType && mType->mIsAbstract;
	}

	/// Check if contained type is constructible											
	/// Some are only referencable, such as abstract types							
	///	@returns true if the contents of this pack are constructible			
	constexpr bool Block::IsConstructible() const noexcept {
		return mType && mType->mDefaultConstructor;
	}

	/// Check if block contains pointers													
	///	@return true if the block contains pointers									
	constexpr bool Block::IsSparse() const {
		return mState.IsSparse();
	}
	
	/// Check if the memory block contains memory blocks								
	///	@return true if the memory block contains memory blocks					
	constexpr bool Block::IsDeep() const noexcept {
		return mType && mType->mIsDeep;
	}

	/// IsDeep (slower) check if there's anything missing inside nested blocks	
	///	@return true if the deep or flat memory block contains missing stuff	
	constexpr bool Block::IsMissingDeep() const {
		if (IsMissing())
			return true;

		bool result = false;
		ForEachDeep([&result](const Block& group) {
			result = group.IsMissing();
			return !result;
		});

		return result;
	}
	
	/// Get the size of a single element (in bytes)										
	///	@attention this returns size of pointer if container is sparse			
	///	@attention this returns zero if block is untyped							
	///	@return the size is bytes															
	constexpr Size Block::GetStride() const noexcept {
		return mState.IsSparse() ? sizeof(void*) : (mType ? mType->mSize : 0);
	}
	
	/// Get the token of the contained type												
	///	@return the token																		
	constexpr Token Block::GetToken() const noexcept {
		return IsUntyped() ? MetaData::DefaultToken : mType->mToken;
	}
	
	/// Get the data state of the container												
	///	@return the current block state													
	constexpr const DataState& Block::GetState() const noexcept {
		return mState;
	}

	/// Overwrite the current data state													
	/// You can not remove constraints														
	constexpr void Block::SetState(DataState state) noexcept {
		mState = state - DataState::Constrained;
	}

	/// Get the relevant state when relaying one block	to another					
	/// Relevant states exclude memory and type constraints							
	constexpr DataState Block::GetUnconstrainedState() const noexcept {
		return mState - DataState::Constrained;
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
	///	@param byteOffset - number of bytes to add									
	///	@return the selected byte															
	inline Byte* Block::At(const Offset& byteOffset) {
		#if LANGULUS_SAFE()
			if (!mRaw) {
				throw Except::Access(Logger::Error()
					<< "Byte offset in invalid memory of type " << GetToken());
			}
		#endif
		return GetRaw() + byteOffset;
	}

	inline const Byte* Block::At(const Offset& byte_offset) const {
		return const_cast<Block&>(*this).At(byte_offset);
	}

	/// Get templated element																	
	/// Checks only density																		
	template<ReflectedData T>
	decltype(auto) Block::Get(const Offset& idx, const Offset& baseOffset) const {
		return const_cast<Block&>(*this).Get<T>(idx, baseOffset);
	}

	/// Get templated element																	
	/// Checks density and type																
	template<ReflectedData T>
	decltype(auto) Block::As(const Offset& idx) const {
		return const_cast<Block&>(*this).As<T>(idx);
	}

	/// Get templated element																	
	/// Checks range, density, type and special indices								
	template<ReflectedData T>
	decltype(auto) Block::As(const Index& index) {
		return As<T>(ConstrainMore<T>(index).GetOffset());
	}

	template<ReflectedData T>
	decltype(auto) Block::As(const Index& index) const {
		return const_cast<Block&>(*this).As<T>(index);
	}

	/// Check if a pointer is anywhere inside the block's memory					
	///	@param ptr - the pointer to check												
	///	@return true if inside the memory block										
	inline bool Block::Owns(const void* ptr) const noexcept {
		return ptr >= GetRaw() && ptr < GetRawEnd();
	}

	/// Mutate the block to a different type, if possible								
	///	@tparam T - the type to change to												
	///	@return true if block was deepened to incorporate the new type			
	template<ReflectedData T, Anyness::IsDeep WRAPPER>
	bool Block::Mutate() {
		const auto deepened = Mutate<WRAPPER>(MetaData::Of<Decay<T>>());
		if constexpr (Langulus::IsSparse<T>)
			MakeSparse();
		return deepened;
	}
	
	/// Mutate to another compatible type, deepening the container if allowed	
	///	@param meta - the type to mutate into											
	///	@return true if block was deepened												
	template<Anyness::IsDeep WRAPPER>
	bool Block::Mutate(DMeta meta) {
		if (IsUntyped()) {
			// Undefined containers can mutate freely								
			SetType<false>(meta);
		}
		else if (mType->Is(meta)) {
			// No need to mutate - types are the same								
			return false;
		}
		else if (IsAbstract() && IsEmpty() && meta->CastsTo(mType)) {
			// Abstract compatible containers can be concretized				
			SetType<false>(meta);
		}
		else if (!IsInsertable(meta)) {
			// Not insertable due to some reasons									
			if (!IsTypeConstrained()) {
				// Container is not type-constrained, so we can safely		
				// deepen it, to incorporate the new data							
				Deepen<WRAPPER>();
				return true;
			}
			else throw Except::Mutate(Logger::Error()
				<< "Attempting to deepen incompatible type-constrained container from "
				<< GetToken() << " to " << meta->mToken);
		}

		SAFETY(if (!CastsToMeta(meta)) {
			throw Except::Mutate(Logger::Error()
				<< "Mutation results in incompatible data " << meta->mToken
				<< " (container of type " << GetToken() << ")");
		})

		return false;
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
				if constexpr (IsSortable<T>)
					return GetIndexMax<T>();
				else return Index::None;
				break;
			case Index::Smallest:
				if constexpr (IsSortable<T>)
					return GetIndexMin<T>();
				else return Index::None;
				break;
			case Index::Mode:
				if constexpr (IsSortable<T>) {
					[[maybe_unused]] Count unused;
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
		return CanFit(MetaData::Of<Decay<T>>());
	}

	template<ReflectedData T>
	bool Block::CastsTo() const {
		return CastsToMeta(MetaData::Of<Decay<T>>());
	}

	template<ReflectedData T>
	bool Block::CastsTo(Count count) const {
		return CastsToMeta(MetaData::Of<Decay<T>>(), count);
	}

	template<ReflectedData T>
	bool Block::Is() const {
		return Is(MetaData::Of<Decay<T>>());
	}

	/// Set the data ID - use this only if you really know what you're doing	
	///	@tparam CONSTRAIN - whether or not to enable type-constraints			
	///	@param type - the type meta to set												
	template<bool CONSTRAIN>
	void Block::SetType(DMeta type) {
		if (mType == type) {
			if constexpr (CONSTRAIN)
				MakeTypeConstrained();
			return;
		}
		else if (!mType) {
			mType = type;
			if constexpr (CONSTRAIN)
				MakeTypeConstrained();
			return;
		}

		// At this point, the container has a set type							
		if (IsTypeConstrained()) {
			// You can't change type of a type-constrained block				
			throw Except::Mutate(Logger::Error()
				<< "Changing typed block is disallowed: from "
				<< GetToken() << " to " << type->mToken);
		}

		if (mType->CastsTo(type)) {
			// Type is compatible, but only sparse data can mutate freely	
			// Dense containers can't mutate because their destructors		
			// might be wrong later														
			if (IsSparse())
				mType = type;
			else throw Except::Mutate(Logger::Error()
				<< "Changing to compatible dense type is disallowed: from "
				<< GetToken() << " to " << type->mToken);
		}
		else {
			// Type is not compatible, but container is not typed, so if	
			// it has no constructed elements, we can still mutate it		
			if (IsEmpty())
				mType = type;
			else throw Except::Mutate(Logger::Error()
				<< "Changing to incompatible type while there's constructed "
				<< "data is disallowed: from " << GetToken()
				<< " to " << type->mToken);
		}

		if constexpr (CONSTRAIN)
			MakeTypeConstrained();
	}
	
	/// Set the contained data type															
	///	@tparam T - the contained type													
	///	@tparam CONSTRAIN - whether or not to enable type-constraints			
	template<ReflectedData T, bool CONSTRAIN>
	void Block::SetType() {
		SetType<CONSTRAIN>(MetaData::Of<Decay<T>>());
		if constexpr (Langulus::IsSparse<T>)
			MakeSparse();
	}

	/// Swap two elements (with raw indices)												
	///	@param from - first element index												
	///	@param to - second element index													
	template<ReflectedData T>
	void Block::Swap(Offset from, Offset to) {
		if (from >= mCount || to >= mCount || from == to)
			return;

		auto data = &Get<T>();
		T temp {Move(data[to])};
		data[to] = Move(data[from]);
		data[from] = Move(temp);
	}

	/// Swap two elements (with special indices)											
	///	@param from - first element index												
	///	@param to - second element index													
	template<ReflectedData T>
	void Block::Swap(Index from, Index to) {
		if (from == to)
			return;

		Swap<T>(
			ConstrainMore<T>(from).GetOffset(), 
			ConstrainMore<T>(to).GetOffset()
		);
	}

	/// Emplace anything compatible to container											
	///	@attention when emplacing pointers, their memory is referenced,		
	///				  and the pointer is not cleared, so you can free it later	
	///	@param item - item to move															
	///	@param index - use uiFront or uiBack for pushing to ends					
	///	@return number of inserted elements												
	template<ReflectedData T, bool MUTABLE, ReflectedData WRAPPER>
	Count Block::Emplace(T&& item, const Index& index) {
		const auto starter = ConstrainMore<T>(index).GetOffset();

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T, WRAPPER>())
				return Emplace<Any, false, WRAPPER>(Any {Forward<T>(item)}, index);
		}

		// Allocate																			
		Allocate<false>(mCount + 1);

		// Move memory if required														
		if (starter < mCount) {
			SAFETY(if (GetReferences() > 1)
				throw Except::Reference(Logger::Error()
					<< "Moving elements that are used from multiple places"));

			Block::CropInner(starter + 1, 0, mCount - starter)
				.CallMoveConstructors(
					Block::CropInner(starter, mCount - starter, mCount - starter));
		}

		// Insert new data																
		if constexpr (Langulus::IsSparse<T>) {
			// Sparse data insertion (moving a pointer)							
			auto data = GetRawSparse() + starter;
			*data = reinterpret_cast<Byte*>(item);

			// Reference the pointer's memory										
			Allocator::Keep(mType, item, 1);
		}
		else {
			static_assert(!Langulus::IsAbstract<T>, "Can't emplace abstract item");

			// Dense data insertion (placement move-construction)				
			auto data = GetRaw() + starter * sizeof(T);
			if constexpr (IsMoveConstructible<T>)
				new (data) T {Forward<T>(item)};
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
	template<ReflectedData T, bool MUTABLE, ReflectedData WRAPPER>
	Count Block::Insert(T* items, const Count count, const Index& index) {
		const auto starter = ConstrainMore<T>(index).GetOffset();

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T, WRAPPER>()) {
				WRAPPER wrapper;
				wrapper.template Insert<T, false, WRAPPER>(items, count);
				return Emplace<WRAPPER, false, WRAPPER>(Move(wrapper), index);
			}
		}

		// Allocate																			
		Allocate<false>(mCount + count);

		// Move memory if required														
		if (starter < mCount) {
			SAFETY(if (GetReferences() > 1)
				throw Except::Reference(Logger::Error()
					<< "Moving elements that are used from multiple places"));

			CropInner(starter + count, 0, mCount - starter)
				.CallMoveConstructors(
					CropInner(starter, mCount - starter, mCount - starter));
		}

		// Insert new data																
		auto data = GetRaw() + starter * sizeof(T);
		if constexpr (Langulus::IsSparse<T>) {
			// Sparse data insertion (copying pointers and referencing)		
			// Doesn't care about abstract items									
			CopyMemory(items, data, sizeof(T) * count);
			Count c {};
			while (c < count) {
				if (!items[c]) {
					throw Except::Reference(Logger::Error()
						<< "Copy-insertion of a null pointer of type "
						<< GetToken() << " is not allowed");
				}

				// Reference each pointer												
				Allocator::Keep(mType, items[c], 1);
				++c;
			}
		}
		else {
			static_assert(!Langulus::IsAbstract<T>, "Can't insert abstract item");

			if constexpr (IsPOD<T>) {
				// Optimized POD insertion												
				CopyMemory(items, data, count * sizeof(T));
			}
			else if constexpr (IsCopyConstructible<T>) {
				// Dense data insertion (placement copy-construction)			
				Count c {};
				while (c < count) {
					// Reset all items													
					new (data + c * sizeof(T)) T {items[c]};
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
		for (Offset i = 0; i < count; ++i) {
			const auto idx = Find<T>(items[i], index);
			if (idx)
				removed += RemoveIndex(idx.GetOffset(), 1);
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
			if (!CastsTo<T>()) {
				// If dense and not forward compatible - fail					
				return Index::None;
			}
		}
		else if (!MetaData::Of<T>()->CastsTo(mType)) {
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
		auto item_ptr = Langulus::MakeSparse(item);
		for (auto i = starti; i < mCount && i >= 0; i += istep) {
			auto left = Langulus::MakeSparse(Get<T>(i.GetOffset()));
			if (left == item_ptr) {
				// Early success if pointers match									
				return i;
			}

			if constexpr (Langulus::IsSparse<T>) {
				// If searching for pointers - cease after pointer check		
				continue;
			}
			else if constexpr (IsResolvable<T>) {
				// Pointers didn't match, but we have ClassBlock				
				// so we attempt to call reflected comparison operator		
				// for the concrete types												
				auto lhs = left->GetBlock();
				auto rhs = item_ptr->GetBlock();
				if (lhs.GetMeta() != rhs.GetMeta())
					continue;

				if (lhs.GetType()->mComparer) {
					if (lhs.GetType()->mComparer(lhs.GetRaw(), rhs.GetRaw()))
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
			else if constexpr (IsComparable<T>) {
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
	template<ReflectedData T, bool MUTABLE, ReflectedData WRAPPER>
	Count Block::Merge(const T* items, const Count count, const Index& idx) {
		Count added {};
		for (Offset i = 0; i < count; ++i) {
			if (!Find<T>(items[i]))
				added += Insert<T, MUTABLE, WRAPPER>(items + i, 1, idx);
		}

		return added;
	}

	/// Get the index of the biggest dense element										
	template<ReflectedData T>
	Index Block::GetIndexMax() const noexcept requires IsSortable<T> {
		if (IsEmpty())
			return Index::None;

		auto data = Get<Decay<T>*>();
		auto max = data;
		for (Offset i = 1; i < mCount; ++i) {
			if (MakeDense(data[i]) > MakeDense(*max))
				max = data + i;
		}

		return max - data;
	}

	/// Get the index of the smallest dense element										
	template<ReflectedData T>
	Index Block::GetIndexMin() const noexcept requires IsSortable<T> {
		if (IsEmpty())
			return Index::None;

		auto data = Get<Decay<T>*>();
		auto min = data;
		for (Offset i = 1; i < mCount; ++i) {
			if (MakeDense(data[i]) < MakeDense(*min))
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
		for (Offset i = 0; i < mCount; ++i) {
			Count counter {};
			for (Count j = i; j < mCount; ++j) {
				if constexpr (IsComparable<T, T>) {
					// First we compare by memory pointer, then by ==			
					if (Langulus::MakeSparse(data[i]) == Langulus::MakeSparse(data[j]) ||
						 Langulus::MakeDense(data[i])  == Langulus::MakeDense(data[j]))
						++counter;
				}
				else {
					// No == operator, so just compare by memory	pointer		
					if (Langulus::MakeSparse(data[i]) == Langulus::MakeSparse(data[j]))
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
	template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, ReflectedData T, Anyness::IsDeep WRAPPER>
	Count Block::SmartPush(const T& value, DataState state, Index index) {
		// Wrap the value, but don't reference anything yet					
		auto pack = Block::From(value);
		
		// Early exit if nothing to push												
		if (!pack.IsValid())
			return 0;

		// Check if unmovable															
		if (IsStatic()) {
			Logger::Error() << "Can't smart-push in static data region";
			return 0;
		}

		// If this container is empty and has no conflicting state			
		// do a shallow copy and directly reference data						
		const auto meta = pack.GetType();
		const bool typeCompliant = (!IsTypeConstrained() && IsEmpty()) || CanFit(meta);
		const bool stateCompliant = CanFitState(pack);
		if (IsEmpty() && typeCompliant && stateCompliant) {
			const auto previousType = !mType ? meta : mType;
			const auto previousState = mState - DataState::Sparse;
			*this = pack;
			Keep();
			SetState((mState + previousState + state) - DataState::Typed);

			if (previousState.IsTyped())
				// Retain type if original package was constrained				
				SetType<true>(previousType);
			else if (IsSparse())
				// Retain type if current package is sparse						
				SetType<false>(previousType);
			return 1;
		}

		[[maybe_unused]]
		const bool orCompliant = !(mCount > 1 && !IsOr() && state.IsOr());
		
		if constexpr (ALLOW_CONCAT) {
			// If this container is compatible and concatenation is enabled
			// try concatenating the two containers. Concatenation will not
			// be allowed if final state is OR, and there are multiple		
			// items	in this container.												
			Count catenated {};
			if (typeCompliant && stateCompliant && orCompliant && 0 < (catenated = InsertBlock(pack, index))) {
				SetState(mState + state);
				return catenated;
			}
		}

		// If this container is deep, directly push the pack inside			
		// This will be disallowed if final state is OR, and there are		
		// multiple items	in this container.										
		if (orCompliant && IsDeep()) {
			SetState(mState + state);
			return Emplace<WRAPPER, false, WRAPPER>(pack, index);
		}

		// Finally, if allowed, force make the container deep in order to	
		// push the pack inside															
		if constexpr (ALLOW_DEEPEN) {
			if (!IsTypeConstrained()) {
				Deepen<WRAPPER>();
				SetState(mState + state);
				return Emplace<WRAPPER, false, WRAPPER>(Move(pack), index);
			}
		}

		return 0;
	}

	/// Wrap all contained elements inside a sub-block, making this one deep	
	///	@tparam T - the type of deep container to use								
	///	@tparam MOVE_STATE - whether or not to send the current state over	
	///	@return a reference to this container											
	template<Anyness::IsDeep T, bool MOVE_STATE>
	T& Block::Deepen() {
		if (IsTypeConstrained() && !Is<T>()) {
			throw Except::Mutate(Logger::Error()
				<< "Attempting to deepen incompatible typed container");
		}

		#if LANGULUS_SAFE()
			if (GetReferences() > 1)
				Logger::Warning() << "Container used from multiple places";
		#endif

		// Back up the state so that we can restore it if not moved over	
		[[maybe_unused]] const auto state {GetUnconstrainedState()};
		if constexpr (!MOVE_STATE)
			mState -= state;

		// Allocate a new T and move this inside it								
		Block wrapper;
		wrapper.SetType<T>();
		wrapper.Allocate<true>(1);
		wrapper.Get<Block>() = Move(*this);
		*this = wrapper;
		
		// Restore the state of not moved over										
		if constexpr (!MOVE_STATE)
			mState += state;

		return Get<T>();
	}

	/// Get an element pointer or reference with a given index						
	/// This is a lower-level routine that does no type checking					
	/// No conversion or copying occurs, only pointer arithmetic					
	///	@param idx - simple index for accessing										
	///	@param baseOffset - byte offset from the element to apply				
	///	@return either pointer or reference to the element (depends on T)		
	template<ReflectedData T>
	decltype(auto) Block::Get(const Offset& idx, const Offset& baseOffset) {
		Byte* pointer;
		if (IsSparse())
			pointer = GetRawSparse()[idx] + baseOffset;
		else
			pointer = At(mType->mSize * idx) + baseOffset;

		if constexpr (Langulus::IsDense<T>)
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
	decltype(auto) Block::As(const Offset& idx) {
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
				return resolved.GetBaseMemory(base).Get<T>(idx % base.mCount);
			}

			// All stages of interpretation failed									
			// Don't log this, because it will spam the crap out of us		
			// That throw is used by ForEach to handle irrelevant types		
			throw Except::Access("Type mismatch on Block::As");
		}

		// Get base memory of the required element and access					
		return 
			GetElementDense(idx / base.mCount)
				.GetBaseMemory(base)
					.Get<T>(idx % base.mCount);
	}

	template<bool MUTABLE, class F>
	Count Block::ForEach(F&& call) {
		using A = decltype(GetLambdaArgument(&F::operator()));
		using R = decltype(call(std::declval<A>()));
		return ForEachInner<R, A, false, MUTABLE>(Forward<F>(call));
	}

	template<class F>
	Count Block::ForEach(F&& call) const {
		using A = decltype(GetLambdaArgument(&F::operator()));
		static_assert(Langulus::IsConstant<A>, "Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEach<false>(Forward<F>(call));
	}

	template<bool MUTABLE, class F>
	Count Block::ForEachRev(F&& call) {
		using A = decltype(GetLambdaArgument(&F::operator()));
		using R = decltype(call(std::declval<A>()));
		return ForEachInner<R, A, true, MUTABLE>(Forward<F>(call));
	}

	template<class F>
	Count Block::ForEachRev(F&& call) const {
		using A = decltype(GetLambdaArgument(&F::operator()));
		static_assert(Langulus::IsConstant<A>, "Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachRev<false>(Forward<F>(call));
	}

	template<bool SKIP, bool MUTABLE, class F>
	Count Block::ForEachDeep(F&& call) {
		using A = decltype(GetLambdaArgument(&F::operator()));
		using R = decltype(call(std::declval<A>()));
		if constexpr (Anyness::IsDeep<A>) {
			// If argument type is deep												
			return ForEachDeepInner<R, A, false, SKIP, MUTABLE>(Forward<F>(call));
		}
		else {
			// Any other type is wrapped inside another ForEachDeep call	
			return ForEachDeep<SKIP, MUTABLE>([&call](Block& block) {
				block.ForEach<MUTABLE, F>(Forward<F>(call));
			});
		}
	}

	template<bool SKIP, class F>
	Count Block::ForEachDeep(F&& call) const {
		using A = decltype(GetLambdaArgument(&F::operator()));
		static_assert(Langulus::IsConstant<A>, "Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachDeep<SKIP, false>(Forward<F>(call));
	}

	template<bool SKIP, bool MUTABLE, class F>
	Count Block::ForEachDeepRev(F&& call) {
		using A = decltype(GetLambdaArgument(&F::operator()));
		using R = decltype(call(std::declval<A>()));
		if constexpr (Anyness::IsDeep<A>) {
			// If argument type is deep												
			return ForEachDeepInner<R, A, true, SKIP, MUTABLE>(
				Forward<F>(call));
		}
		else {
			// Any other type is wrapped inside another ForEachDeep call	
			return ForEachDeepRev<SKIP, MUTABLE>([&call](Block& block) {
				block.ForEachRev<F>(Forward<F>(call));
			});
		}
	}

	template<bool SKIP, class F>
	Count Block::ForEachDeepRev(F&& call) const {
		using A = decltype(GetLambdaArgument(&F::operator()));
		static_assert(Langulus::IsConstant<A>, "Non constant iterator for constant memory block");
		return const_cast<Block*>(this)->ForEachDeepRev<SKIP, false>(Forward<F>(call));
	}

	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class R, ReflectedData A, bool REVERSE, bool MUTABLE>
	Count Block::ForEachInner(TFunctor<R(A)>&& call) {
		if (IsEmpty())
			return 0;
		 
		[[maybe_unused]] auto initialCount = mCount;
		constexpr bool HasBreaker = IsSame<bool, R>;
		Count index {};
		if (mType->Is<A>()) {
			// Fast specialized routine that gives direct access				
			// Uses Get<>() instead of As<>()										
			while (index < mCount) {
				// Iterator is a reference												
				if constexpr (Langulus::IsDense<A>) {
					if constexpr (REVERSE) {
						if constexpr (HasBreaker) {
							if (!call(Get<A>(mCount - index - 1)))
								return index + 1;
						}
						else call(Get<A>(mCount - index - 1));
					}
					else {
						if constexpr (HasBreaker) {
							if (!call(Get<A>(index)))
								return index + 1;
						}
						else call(Get<A>(index));
					}
				}
				else {
					if constexpr (REVERSE) {
						auto pointer = Get<A>(mCount - index - 1);
						if constexpr (HasBreaker) {
							if (!call(pointer))
								return index + 1;
						}
						else call(pointer);
					}
					else {
						auto pointer = Get<A>(index);
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
		else if (mType->CastsTo<A>()) {
			// Slow generalized routine that resolved each element			
			// Uses As<>() instead of Get<>()										
			Count successes {};
			while (index < mCount) {
				try {
					// Iterator is a reference											
					if constexpr (Langulus::IsDense<A>) {
						if constexpr (REVERSE) {
							if constexpr (HasBreaker) {
								if (!call(As<A>(mCount - index - 1)))
									return successes + 1;
							}
							else call(As<A>(mCount - index - 1));
						}
						else {
							if constexpr (HasBreaker) {
								if (!call(As<A>(index)))
									return successes + 1;
							}
							else call(As<A>(index));
						}
					}
					else {
						if constexpr (REVERSE) {
							auto pointer = As<A>(mCount - index - 1);
							if constexpr (HasBreaker) {
								if (!call(pointer))
									return successes + 1;
							}
							else call(pointer);
						}
						else {
							auto pointer = As<A>(index);
							if constexpr (HasBreaker) {
								if (!call(pointer))
									return successes + 1;
							}
							else call(pointer);
						}
					}
				}
				catch (const Except::Access&) {
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
	template<class R, ReflectedData A, bool REVERSE>
	Count Block::ForEachInner(TFunctor<R(A)>&& call) const {
		return const_cast<Block*>(this)->ForEachInner<R, A, REVERSE, false>(
			Forward<decltype(call)>(call));
	}
	
	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class RETURN, ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY, bool MUTABLE>
	Count Block::ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&& call) {
		constexpr bool HasBreaker = IsSame<bool, RETURN>;
		[[maybe_unused]] bool atLeastOneChange = false;
		auto count {GetCountDeep()};
		Count index {};
		while (index < count) {
			auto block = ReinterpretCast<ARGUMENT>(GetBlockDeep(index));
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

			[[maybe_unused]] const auto initialBlockCount = block->GetCount();
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
		return const_cast<Block*>(this)->ForEachDeepInner<RETURN, ARGUMENT, REVERSE, false>(
			Forward<decltype(call)>(call));
	}

	/// Wrapper for memcpy																		
	///	@param from - source of data to copy											
	///	@param to - [out] destination memory											
	///	@param size - number of bytes to copy											
	inline void Block::CopyMemory(const void* from, void* to, const Size& size) noexcept {
		::std::memcpy(to, from, size);
	}
	
	/// Wrapper for memmove																		
	///	@param from - source of data to move											
	///	@param to - [out] destination memory											
	///	@param size - number of bytes to move											
	inline void Block::MoveMemory(const void* from, void* to, const Size& size) noexcept {
		::std::memmove(to, from, size);
		#if LANGULUS(PARANOID)
			TODO() // zero old memory, but beware - `from` and `to` might overlap
		#endif
	}
	
	/// Wrapper for memset																		
	///	@param to - [out] destination memory											
	///	@param filler - the byte to fill with											
	///	@param size - number of bytes to move											
	inline void Block::FillMemory(void* to, Byte filler, const Size& size) noexcept {
		::std::memset(to, static_cast<int>(filler), size);
	}
	
	/// Wrapper for memcmp																		
	///	@param a1 - size of first array													
	///	@param a2 - size of second array													
	///	@param size - number of bytes to compare										
	inline int Block::CompareMemory(const void* a1, const void* a2, const Size& size) noexcept {
		return ::std::memcmp(a1, a2, size);
	}

	/// Dereference memory block once and destroy all elements if data was		
	/// fully dereferenced																		
	///	@return the remaining references for the block								
	inline bool Block::Free() {
		return Dereference<true>(1);
	}

	/// Select region from the memory block - unsafe and may return memory		
	/// that has not been initialized yet (for internal use only)					
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the block representing the region										
	inline Block Block::CropInner(const Offset& start, const Count& count, const Count& reserved) const noexcept {
		Block result {*this};
		result.mCount = ::std::min(start < mCount ? mCount - start : 0, count);
		result.mRaw += start * mType->mSize;
		result.mReserved = ::std::min(reserved, mReserved - start);
		return result;
	}

	/// Select an initialized region from the memory block					 		
	///	@param start - starting element index											
	///	@param count - number of elements to remain after 'start'				
	///	@return the block representing the region										
	inline Block Block::Crop(const Offset& start, const Count& count) {
		CheckRange(start, count);
		if (count == 0)
			return {mState, mType};

		Block result {*this};
		result.mCount = result.mReserved = count;
		result.mRaw += start * mType->mSize;
		result.mState += DataState::Member;
		return result;
	}

	/// Select a constant region from the memory block 								
	/// Never references																			
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the block representing the region										
	inline Block Block::Crop(const Offset& start, const Count& count) const {
		auto result = const_cast<Block*>(this)->Crop(start, count);
		result.MakeConstant();
		return result;
	}
	
} // namespace Langulus::Anyness

