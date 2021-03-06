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
	
	/// Block doesn't have ownership, so this constructor is here only to		
	/// avoid RTTI complaining when containing Block in Block						
	constexpr Block::Block(Disowned<Block>&& other) noexcept
		: Block {other.mValue} {}

	/// Block doesn't have ownership, so this constructor is here only to		
	/// avoid RTTI complaining when containing Block in Block						
	constexpr Block::Block(Abandoned<Block>&& other) noexcept
		: Block {static_cast<const Block&>(other.mValue)} {}

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
	/// This constructor has runtime overhead if managed memory is enabled		
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
		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			, mEntry {Inner::Allocator::Find(meta, raw)} { }
		#else
			, mEntry {nullptr} { }
		#endif
	
	/// Manual construction from constant data											
	/// This constructor has runtime overhead if managed memory is enabled		
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the constant memory									
	inline Block::Block(const DataState& state, DMeta meta, Count count, const void* raw) noexcept
		: Block {state, meta, count, const_cast<void*>(raw)} {
		MakeConst();
	}

	/// Manual construction from mutable data and preallocated entry				
	///	@param state - the initial state of the container							
	///	@param meta - the type of the memory block									
	///	@param count - initial element count and reserve							
	///	@param raw - pointer to the mutable memory									
	///	@param entry - the memory entry													
	inline Block::Block(const DataState& state, DMeta meta, Count count, void* raw, Inner::Allocation* entry) noexcept
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
	inline Block::Block(const DataState& state, DMeta meta, Count count, const void* raw, Inner::Allocation* entry) noexcept
		: Block {state, meta, count, const_cast<void*>(raw), entry} {
		MakeConst();
	}

	/// Create a memory block from a single typed pointer								
	///	@tparam T - the type of the value to wrap (deducible)						
	///	@tparam CONSTRAIN - makes container type-constrained						
	///	@return the block																		
	template<CT::Data T, bool CONSTRAIN>
	Block Block::From(T value) requires CT::Sparse<T> {
		if constexpr (CONSTRAIN)
			return {DataState::Member, MetaData::Of<T>(), 1, value};
		else
			return {DataState::Static, MetaData::Of<T>(), 1, value};		
	}

	/// Create a memory block from a count-terminated array							
	///	@tparam T - the type of the value to wrap (deducible)						
	///	@tparam CONSTRAIN - makes container type-constrained						
	///	@return the block																		
	template<CT::Data T, bool CONSTRAIN>
	Block Block::From(T value, Count count) requires CT::Sparse<T> {
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
	template<CT::Data T, bool CONSTRAIN>
	Block Block::From(T& value) requires CT::Dense<T> {
		Block result;
		if constexpr (CT::Resolvable<T>) {
			// Resolve a runtime-resolvable value									
			result = value.GetBlock();
		}
		else if constexpr (CT::Deep<T>) {
			// Static cast to Block if CT::Deep										
			result = static_cast<const Block&>(value);
		}
		else {
			// Any other value gets wrapped inside a temporary Block			
			result = {DataState::Static, MetaData::Of<Decay<T>>(), 1, &value};
		}
		
		if constexpr (CONSTRAIN)
			result.MakeTypeConstrained();
		return result;
	}

	/// Create an empty memory block from a static type								
	///	@tparam T - the type of the value to wrap (deducible)						
	///	@tparam CONSTRAIN - makes container type-constrained						
	///	@return the block																		
	template<CT::Data T, bool CONSTRAIN>
	Block Block::From() {
		if constexpr (CONSTRAIN)
			return {DataState::Typed, MetaData::Of<T>()};
		else
			return {MetaData::Of<T>()};
	}

	/// Reference memory block if we own it												
	///	@param times - number of references to add									
	inline void Block::Reference(const Count& times) noexcept {
		if (mEntry)
			mEntry->Keep(times);
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

		#if LANGULUS(SAFE)
			if (mEntry->GetUses() < times)
				Throw<Except::Reference>("Bad memory dereferencing");
		#endif

		if (mEntry->GetUses() == times) {
			// Destroy all elements and deallocate the entry					
			if constexpr (DESTROY)
				CallUnknownDestructors();
			Inner::Allocator::Deallocate(mEntry);
			mEntry = nullptr;
			return true;
		}

		mEntry->Free(times);
		mEntry = nullptr;
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
	template<bool TYPED, bool SPARSE>
	constexpr void Block::ResetState() noexcept {
		if constexpr (TYPED) {
			// DON'T clear type, and restore typed state							
			if constexpr (SPARSE)
				mState = DataState::Typed | DataState::Sparse;
			else
				mState = DataState::Typed;
		}
		else {
			// Clear both type and state												
			mType = nullptr;
			if constexpr (SPARSE)
				mState = DataState::Sparse;
			else
				mState.Reset();
		}
	}
	
	/// Get a size based on reflected allocation page and count (unsafe)			
	///	@param count - the number of elements to request							
	///	@returns both the provided byte size and reserved count					
	inline auto Block::RequestSize(const Count& count) const noexcept {
		if (IsSparse()) {
			AllocationRequest result;
			const auto requested = sizeof(KnownPointer) * count;
			result.mByteSize = requested > Alignment ? Roof2(requested) : Alignment;
			result.mElementCount = result.mByteSize / sizeof(KnownPointer);
			return result;
		}
		
		return mType->RequestSize(mType->mSize * count);
	}

	/// Allocate a number of elements, relying on the type of the container		
	///	@attention inner unsafe function													
	///	@tparam CREATE - true to call constructors									
	///	@param elements - number of elements to allocate							
	template<bool CREATE>
	void Block::AllocateInner(const Count& elements) {
		// Retrieve the required byte size											
		const auto request = RequestSize(elements);
		
		// Allocate/reallocate															
		if (mEntry) {
			// Reallocate																	
			Block previousBlock {*this};
			if (mEntry->GetUses() == 1) {
				// Memory is used only once and it is safe to move it			
				// Make note, that Allocator::Reallocate doesn't copy			
				// anything, it doesn't use realloc for various reasons, so	
				// we still have to call move construction for all elements	
				// if entry moved (enabling MANAGED_MEMORY feature				
				// significantly reduces the possiblity for a move)			
				// Also, make sure to free the previous mEntry if moved		
				mEntry = Inner::Allocator::Reallocate(request.mByteSize, mEntry);
				if (!mEntry)
					Throw<Except::Allocate>("Out of memory on block reallocation");

				if (mEntry != previousBlock.mEntry) {
					// Memory moved, and we should call move-construction		
					mRaw = mEntry->GetBlockStart();
					mCount = 0;
					CallUnknownMoveConstructors<false>(previousBlock.mCount, Move(previousBlock));
					//previousBlock.Free();
				}
				
				if constexpr (CREATE) {
					// Default-construct the rest										
					CallUnknownDefaultConstructors(elements - mCount);
				}
			}
			else {
				// Memory is used from multiple locations, and we must		
				// copy the memory for this block - we can't move it!			
				mEntry = Inner::Allocator::Allocate(request.mByteSize);
				if (!mEntry)
					Throw<Except::Allocate>("Out of memory on additional block allocation");

				mRaw = mEntry->GetBlockStart();
				mCount = 0;
				CallUnknownCopyConstructors<true>(previousBlock.mCount, previousBlock);
				previousBlock.Free();
				
				if constexpr (CREATE) {
					// Default-construct the rest										
					CallUnknownDefaultConstructors(elements - mCount);
				}
			}
		}
		else {
			// Allocate a fresh set of elements										
			mEntry = Inner::Allocator::Allocate(request.mByteSize);
			if (!mEntry)
				Throw<Except::Allocate>("Out of memory on a fresh block allocation");

			mRaw = mEntry->GetBlockStart();
			if constexpr (CREATE) {
				// Default-construct everything										
				CallUnknownDefaultConstructors(elements);
			}
		}

		mReserved = request.mElementCount;
	}

	/// Allocate a number of elements, relying on the type of the container		
	///	@tparam CREATE - true to call constructors									
	///	@tparam SETSIZE - true to set count, despite not calling any			
	///							constructors. Works only when CREATE is false		
	///	@param elements - number of elements to allocate							
	template<bool CREATE, bool SETSIZE>
	void Block::Allocate(const Count& elements) {
		if (!mType)
			Throw<Except::Allocate>(
				"Allocating element(s) of an invalid type is not allowed");
		else if (mType->mIsAbstract && IsDense())
			Throw<Except::Allocate>(
				"Allocating element(s) of abstract dense type is not allowed");

		if (mCount > elements) {
			// Destroy back entries on smaller allocation						
			// Allowed even when container is static and out of				
			// jurisdiction, as in that case this acts as a simple count	
			// decrease, and no destructors shall be called						
			RemoveIndex(elements, mCount - elements);
			return;
		}

		if (mReserved >= elements) {
			// Required memory is already available								
			if constexpr (CREATE) {
				// But is not yet initialized, so initialize it					
				if (mCount < elements)
					CallUnknownDefaultConstructors(elements - mCount);
			}
			
			return;
		}
		
		AllocateInner<CREATE>(elements);

		if constexpr (!CREATE && SETSIZE)
			mCount = elements;
	}
	
	/// Check if a range is inside the block												
	/// This function throws on error only if LANGULUS(SAFE) is enabled			
	inline void Block::CheckRange(const Offset& start, const Count& count) const {
		#if LANGULUS_SAFE()
			if (start > mCount)
				Throw<Except::Access>("Crop left offset is out of limits");
			if (start + count > mCount)
				Throw<Except::Access>("Crop count is out of limits");
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
	
	/// Get the number of reserved bytes													
	///	@return the number of reserved bytes											
	constexpr Size Block::GetReservedSize() const noexcept {
		if (mEntry)
			return mEntry->GetAllocatedSize();
		return mType ? mReserved * mType->mSize : 0;
	}
	
	/// Check if we have jurisdiction over the contained memory						
	///	@return true if memory is under our authority								
	constexpr bool Block::HasAuthority() const noexcept {
		return mEntry != nullptr;
	}
	
	/// Get the number of references for the allocated memory block				
	///	@attention returns 0 if memory is outside authority						
	///	@return the references for the memory block									
	constexpr Count Block::GetUses() const noexcept {
		return mEntry ? mEntry->GetUses() : 0;
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
		return mCount || (GetUnconstrainedState() - DataState::Sparse);
	}

	/// Check if block contains no elements and no relevant state					
	///	@returns true if this is an empty stateless container						
	constexpr bool Block::IsInvalid() const noexcept {
		return !IsValid();
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
	constexpr void Block::MakeConst() noexcept {
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
	
	/// Make the container type sparse														
	constexpr void Block::MakeSparse() noexcept {
		mState += DataState::Sparse;
	}
	
	/// Make the container type dense														
	constexpr void Block::MakeDense() noexcept {
		mState -= DataState::Sparse;
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
	constexpr Size Block::GetByteSize() const noexcept {
		return GetCount() * GetStride();
	}

	/// Check if a type can be inserted														
	template<CT::Data T>
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
		return GetRaw() + GetByteSize();
	}

	/// Get the end raw data pointer inside the container (const)					
	///	@attention as unsafe as it gets, but as fast as it gets					
	constexpr const Byte* Block::GetRawEnd() const noexcept {
		return GetRaw() + GetByteSize();
	}

	/// Get a constant pointer array - useful for sparse containers (const)		
	///	@return the raw data as an array of constant pointers						
	constexpr const Block::KnownPointer* Block::GetRawSparse() const noexcept {
		return mRawSparse;
	}

	/// Get a pointer array - useful for sparse containers							
	///	@return the raw data as an array of pointers									
	constexpr Block::KnownPointer* Block::GetRawSparse() noexcept {
		return mRawSparse;
	}

	/// Get the raw data inside the container, reinterpreted as some type		
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<CT::Data T>
	T* Block::GetRawAs() noexcept {
		return reinterpret_cast<T*>(GetRaw());
	}

	/// Get the raw data inside the container, reinterpreted (const)				
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<CT::Data T>
	const T* Block::GetRawAs() const noexcept {
		return reinterpret_cast<const T*>(GetRaw());
	}

	/// Get the end raw data pointer inside the container								
	///	@attention as unsafe as it gets, but as fast as it gets					
	template<CT::Data T>
	const T* Block::GetRawEndAs() const noexcept {
		return reinterpret_cast<const T*>(GetRawEnd());
	}
	
	/// Check if contained type is abstract												
	///	@returns true if the type of this pack is abstract							
	constexpr bool Block::IsAbstract() const noexcept {
		return mType && mType->mIsAbstract;
	}

	/// Check if contained type is default-constructible								
	/// Some are only referencable, such as abstract types							
	///	@returns true if the contents of this pack are constructible			
	constexpr bool Block::IsDefaultable() const noexcept {
		return mType && mType->mDefaultConstructor;
	}

	/// Check if block contains pointers													
	///	@return true if the block contains pointers									
	constexpr bool Block::IsSparse() const noexcept {
		return mState.IsSparse();
	}

	/// Check if block contains dense data													
	///	@returns true if this container refers to dense memory					
	constexpr bool Block::IsDense() const noexcept {
		return !IsSparse();
	}

	/// Check if block contains POD items - if so, it's safe to directly copy	
	/// raw memory from container. Note, that this doesn't only consider the	
	/// standard c++ type traits, like trivially_constructible. You also need	
	/// to explicitly reflect your type with LANGULUS(POD) true;					
	/// This gives a lot more control over your code									
	///	@return true if contained data is plain old data							
	constexpr bool Block::IsPOD() const noexcept {
		return mType && mType->mIsPOD;
	}

	/// Check if block contains resolvable items, that is, items that have a	
	/// GetBlock() function, that can be used to represent themselves as their	
	/// most concretely typed block															
	///	@return true if contained data can be resolved on element basis		
	constexpr bool Block::IsResolvable() const noexcept {
		return mType && mType->mResolver;
	}

	/// Check if block data can be safely set to zero bytes							
	/// This is tied to LANGULUS(NULLIFIABLE) reflection parameter					
	///	@return true if contained data can be memset(0) safely					
	constexpr bool Block::IsNullifiable() const noexcept {
		return mType && mType->mIsNullifiable;
	}

	/// Check if the memory block contains memory blocks								
	///	@return true if the memory block contains memory blocks					
	constexpr bool Block::IsDeep() const noexcept {
		// This should be the same as CT::Deep, but at runtime				
		return mType && mType->mIsDeep && mType->mSize == sizeof(Block) && mType->CastsTo<Block, false>();
	}

	/// Deep (slower) check if there's anything missing inside nested blocks	
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
		return mState.IsSparse() ? sizeof(KnownPointer) : (mType ? mType->mSize : 0);
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

	/// Add a state																				
	///	@attention you can't add constraint states, even if you want to		
	///	@param state - the state to add to the current								
	constexpr void Block::AddState(DataState state) noexcept {
		mState += state - DataState::Constrained;
	}

	/// Remove a state																			
	///	@attention you can't remove constraint states, even if you want to	
	///	@param state - the state to remove from the current						
	constexpr void Block::RemoveState(DataState state) noexcept {
		mState -= state - DataState::Constrained;
	}

	/// Get the relevant state when relaying one block	to another					
	/// Relevant states exclude memory and type constraints							
	constexpr DataState Block::GetUnconstrainedState() const noexcept {
		return mState - DataState::Constrained;
	}

	/// Compare memory blocks - this is a slow runtime compare						
	///	@param other - the block to compare against									
	///	@return true if block's elements all match and are in same order		
	inline bool Block::operator == (const Block& other) const noexcept {
		return Compare(other);
	}

	/// Check if memory block is not allocated											
	///	@return true if block is valid													
	inline bool Block::operator == (::std::nullptr_t) const noexcept {
		return !IsAllocated();
	}

	/// Get the internal byte array with a given offset								
	/// This is lowest level access and checks nothing									
	///	@param byteOffset - number of bytes to add									
	///	@return the selected byte															
	inline Byte* Block::At(const Offset& byteOffset) {
		#if LANGULUS_SAFE()
			if (!mRaw)
				Throw<Except::Access>("Byte offset in invalid memory of type");
		#endif
		return GetRaw() + byteOffset;
	}

	inline const Byte* Block::At(const Offset& byte_offset) const {
		return const_cast<Block*>(this)->At(byte_offset);
	}

	/// Get templated element																	
	/// Checks only density																		
	template<CT::Data T>
	decltype(auto) Block::Get(const Offset& idx, const Offset& baseOffset) const {
		return const_cast<Block*>(this)->Get<T>(idx, baseOffset);
	}

	/// Get an element pointer or reference with a given index						
	/// This is a lower-level routine that does no type checking					
	/// No conversion or copying occurs, only pointer arithmetic					
	///	@param idx - simple index for accessing										
	///	@param baseOffset - byte offset from the element to apply				
	///	@return either pointer or reference to the element (depends on T)		
	template<CT::Data T>
	decltype(auto) Block::Get(const Offset& idx, const Offset& baseOffset) {
		Byte* pointer;
		if (IsSparse())
			pointer = GetRawSparse()[idx].mPointer + baseOffset;
		else
			pointer = At(mType->mSize * idx) + baseOffset;

		if constexpr (CT::Dense<T>)
			return *reinterpret_cast<Deref<T>*>(pointer);
		else
			return reinterpret_cast<Deref<T>>(pointer);
	}

	/// Check if a pointer is anywhere inside the block's memory					
	///	@attention doesn't check deep data if container is sparse				
	///	@param ptr - the pointer to check												
	///	@return true if inside the memory block										
	inline bool Block::Owns(const void* ptr) const noexcept {
		return ptr >= GetRaw() && ptr < GetRawEnd();
	}

	/// Mutate the block to a different type, if possible								
	/// This can also change sparseness, if T is pointer								
	///	@tparam T - the type to change to												
	///	@tparam ALLOW_DEEPEN - are we allowed to mutate to WRAPPER?				
	///	@tparam WRAPPER - container to use for deepening							
	///	@return true if block was deepened to incorporate the new type			
	template<CT::Data T, bool ALLOW_DEEPEN, CT::Data WRAPPER>
	bool Block::Mutate() {
		static_assert(ALLOW_DEEPEN && CT::Deep<WRAPPER>, "WRAPPER must be deep");
		const auto deepened = Mutate<ALLOW_DEEPEN, WRAPPER>(MetaData::Of<Decay<T>>());
		if constexpr (ALLOW_DEEPEN && CT::Sparse<T>) {
			if (deepened)
				Get<WRAPPER>(mCount - 1).MakeSparse();
			else {
				if constexpr (CT::Sparse<T>)
					MakeSparse();
				else
					MakeDense();
			}
		}
		else if constexpr (CT::Sparse<T>)
			MakeSparse();
		else
			MakeDense();
		return deepened;
	}
	
	/// Mutate to another compatible type, deepening the container if allowed	
	///	@attention doesn't affect sparseness											
	///	@tparam ALLOW_DEEPEN - are we allowed to mutate to WRAPPER				
	///	@tparam WRAPPER - type to use to deepen										
	///	@param meta - the type to mutate into											
	///	@return true if block was deepened												
	template<bool ALLOW_DEEPEN, CT::Data WRAPPER>
	bool Block::Mutate(DMeta meta) {
		static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");

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
			if constexpr (ALLOW_DEEPEN) {
				if (!IsTypeConstrained()) {
					// Container is not type-constrained, so we can safely	
					// deepen it, to incorporate the new data						
					Deepen<WRAPPER>();
					return true;
				}
			}

			Throw<Except::Mutate>(
				"Attempting to deepen incompatible type-constrained container");
		}

		SAFETY(if (!CastsToMeta(meta))
			Throw<Except::Mutate>("Mutation results in incompatible data"));

		return false;
	}

	/// Constrain an index to the limits of the current block						
	///	@param idx - the index to constrain												
	///	@return the constrained index or a special one of constrain fails		
	constexpr Index Block::Constrain(const Index& idx) const noexcept {
		return idx.Constrained(mCount);
	}

	/// Constrain an index to the limits of the current block						
	/// Supports additional type-dependent constraints									
	///	@tparam T - the type to use for comparisons									
	///	@param idx - the index to constrain												
	///	@return the constrained index or a special one of constrain fails		
	template<CT::Data T>
	Index Block::ConstrainMore(const Index& idx) const noexcept {
		const auto result = Constrain(idx);
		if (result == IndexBiggest) {
			if constexpr (CT::Sortable<T, T>)
				return GetIndexMax<T>();
			else
				return IndexNone;
		}
		else if (result == IndexSmallest) {
			if constexpr (CT::Sortable<T, T>)
				return GetIndexMin<T>();
			else
				return IndexNone;
		}
		else if (result == IndexMode) {
			if constexpr (CT::Sortable<T, T>) {
				UNUSED() Count unused;
				return GetIndexMode<T>(unused);
			}
			else return IndexNone;
		}

		return result;
	}

	/// Check if type T can fit inside this container									
	///	@tparam T - the type to compare against										
	///	@return true if T can fit inside this container								
	template<CT::Data T>
	bool Block::CanFit() const {
		return CanFit(MetaData::Of<Decay<T>>());
	}

	/// Check if this container's data can be represented as type T				
	/// with nothing more than pointer arithmetic										
	///	@tparam T - the type to compare against										
	///	@return true if contained data is reinterpretable as T					
	template<CT::Data T>
	bool Block::CastsTo() const {
		return CastsToMeta(MetaData::Of<Decay<T>>());
	}

	/// Check if this container's data can be represented as a specific number	
	/// of elements of type T, with nothing more than pointer arithmetic			
	///	@tparam T - the type to compare against										
	///	@param count - the number of elements of T									
	///	@return true if contained data is reinterpretable as T					
	template<CT::Data T>
	bool Block::CastsTo(Count count) const {
		return CastsToMeta(MetaData::Of<Decay<T>>(), count);
	}

	/// Check if this container's data is exactly one of the listed types		
	///	@attention ignores sparsity														
	///	@tparam T - the types to compare against										
	///	@return true if data type matches at least one type						
	template<CT::Data... T>
	bool Block::Is() const {
		return (Is(MetaData::Of<Decay<T>>()) || ...);
	}

	/// Set the data ID - use this only if you really know what you're doing	
	///	@attention doesn't affect sparseness											
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
			Throw<Except::Mutate>(
				"Changing typed block is disallowed");
		}

		if (mType->CastsTo(type)) {
			// Type is compatible, but only sparse data can mutate freely	
			// Dense containers can't mutate because their destructors		
			// might be wrong later														
			if (IsSparse())
				mType = type;
			else Throw<Except::Mutate>(
				"Changing to compatible dense type is disallowed");
		}
		else {
			// Type is not compatible, but container is not typed, so if	
			// it has no constructed elements, we can still mutate it		
			if (IsEmpty())
				mType = type;
			else Throw<Except::Mutate>(
				"Changing to incompatible type while there's "
				"initialized data is disallowed");
		}

		if constexpr (CONSTRAIN)
			MakeTypeConstrained();
	}
	
	/// Set the contained data type															
	///	@attention doesn't affect sparseness											
	///	@tparam T - the contained type													
	///	@tparam CONSTRAIN - whether or not to enable type-constraints			
	template<CT::Data T, bool CONSTRAIN>
	void Block::SetType() {
		SetType<CONSTRAIN>(MetaData::Of<Decay<T>>());
	}

	/// Swap two elements (with raw indices)												
	///	@tparam T - the contained type													
	///	@param from - first element index												
	///	@param to - second element index													
	template<CT::Data T>
	void Block::Swap(Offset from, Offset to) {
		if (from >= mCount || to >= mCount || from == to)
			return;

		auto data = &Get<T>();
		T temp {Move(data[to])};
		data[to] = Move(data[from]);
		data[from] = Move(temp);
	}

	/// Swap two elements (with special indices)											
	///	@tparam T - the contained type													
	///	@param from - first element index												
	///	@param to - second element index													
	template<CT::Data T>
	void Block::Swap(Index from, Index to) {
		if (from == to)
			return;

		Swap<T>(
			ConstrainMore<T>(from).GetOffset(), 
			ConstrainMore<T>(to).GetOffset()
		);
	}

	/// Copy-insert anything compatible at a special index							
	/// Slight overhead due to runtime-resolving the index							
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen to incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param start - pointer to the first item										
	///	@param end - pointer to the end of items										
	///	@param index - the special index to insert at								
	///	@return number of inserted elements												
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::InsertAt(const T* start, const T* end, Index index) {
		const auto offset = ConstrainMore<T>(index).GetOffset(); //TODO constraining assumes this is filled with T? might cause problems :(
		return InsertAt<WRAPPER, KEEP, MUTABLE, T>(start, end, offset);
	}

	/// Copy-insert anything compatible at a simple offset							
	///	@attention assumes offset is in the block's limits							
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen or incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param start - pointer to the first item										
	///	@param end - pointer to the end of items										
	///	@param index - the simple index to insert at									
	///	@return number of inserted elements												
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::InsertAt(const T* start, const T* end, Offset index) {
		static_assert(CT::Deep<WRAPPER>,
			"WRAPPER must be deep");
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't copy-insert into container of constant elements");

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T, WRAPPER>()) {
				WRAPPER wrapper;
				wrapper.template Insert<IndexBack, WRAPPER, KEEP, false, T>(start, end);
				const auto pushed = InsertAt<WRAPPER, false, false, T>(Move(wrapper), index);
				wrapper.mEntry = nullptr;
				return pushed;
			}
		}

		// Allocate																			
		const auto count = end - start;
		Allocate<false>(mCount + count);

		// Move memory if required														
		if (index < mCount) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"
					" - you should first clone the container"));

			CropInner(index + count, 0, mCount - index)
				.template CallKnownMoveConstructors<T>(
					mCount - index,
					CropInner(index, mCount - index, mCount - index)
				);
		}

		InsertInner<KEEP>(start, end, index);
		return count;
	}

	/// Move-insert anything compatible at a special index							
	/// Slight overhead due to runtime-resolving the index							
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen to incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param item - the item to move in												
	///	@param index - the special index to insert at								
	///	@return number of inserted elements												
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::InsertAt(T&& item, Index index) {
		const auto offset = ConstrainMore<T>(index).GetOffset(); //TODO constraining assumes this is filled with T? might cause problems :(
		return InsertAt<WRAPPER, KEEP, MUTABLE, T>(Move(item), offset);
	}

	/// Move-insert anything compatible at a simple offset							
	///	@attention assumes offset is in the block's limits							
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen to incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param item - the item to move in												
	///	@param index - the simple offset to insert at								
	///	@return number of inserted elements												
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::InsertAt(T&& item, Offset index) {
		static_assert(CT::Deep<WRAPPER>,
			"WRAPPER must be deep");
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't move-insert into container of constant elements");

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T, WRAPPER>()) {
				WRAPPER wrapper;
				wrapper.template Insert<IndexBack, WRAPPER, KEEP, false>(Move(item));
				const auto pushed = InsertAt<WRAPPER, false, false, T>(Move(wrapper), index);
				wrapper.mEntry = nullptr;
				return pushed;
			}
		}

		// Allocate																			
		Allocate<false>(mCount + 1);

		// Move memory if required														
		if (index < mCount) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"
					" - you should first clone the container"));

			CropInner(index + 1, 0, mCount - index)
				.template CallKnownMoveConstructors<T>(
					mCount - index,
					CropInner(index, mCount - index, mCount - index)
				);
		}

		InsertInner<KEEP>(Move(item), index);
		return 1;
	}

	/// Copy-insert anything compatible either at the start or the end			
	///	@tparam INDEX - use IndexBack or IndexFront to append accordingly		
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen or incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param start - pointer to the first item										
	///	@param end - pointer to the end of items										
	///	@return number of inserted elements												
	template<Index INDEX, CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::Insert(const T* start, const T* end) {
		static_assert(CT::Deep<WRAPPER>,
			"WRAPPER must be deep");
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't copy-insert into container of constant elements");

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T, true, WRAPPER>()) {
				WRAPPER wrapper;
				wrapper.template Insert<IndexBack, WRAPPER, KEEP, false, T>(start, end);
				const auto pushed = Insert<INDEX, WRAPPER, false, false, WRAPPER>(Move(wrapper));
				wrapper.mEntry = nullptr;
				return pushed;
			}
		}

		// Allocate																			
		const auto count = end - start;
		Allocate<false>(mCount + count);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"
					" - you should first clone the container"));

			CropInner(count, 0, mCount)
				.template CallKnownMoveConstructors<T>(
					mCount, CropInner(0, mCount, mCount)
				);

			InsertInner<KEEP>(start, end, 0);
		}
		else if constexpr (INDEX == IndexBack)
			InsertInner<KEEP>(start, end, mCount);
		else
			LANGULUS_ASSERT("Invalid index provided; use either IndexBack "
				"or IndexFront, or Block::InsertAt to insert at an offset");

		return count;
	}

	/// Move-insert anything compatible either at the start or the end			
	///	@tparam INDEX - use IndexBack or IndexFront to append accordingly		
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen or incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param item - item to move int													
	///	@return number of inserted elements												
	template<Index INDEX, CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::Insert(T&& item) {
		static_assert(CT::Deep<WRAPPER>,
			"WRAPPER must be deep");
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't copy-insert into container of constant elements");

		if constexpr (MUTABLE) {
			// Type may mutate															
			if (Mutate<T, true, WRAPPER>()) {
				WRAPPER wrapper;
				wrapper.template Insert<IndexBack, WRAPPER, KEEP, false, T>(Move(item));
				Insert<INDEX, WRAPPER, false, false, WRAPPER>(Move(wrapper));
				wrapper.mEntry = nullptr;
				return 1;
			}
		}

		// Allocate																			
		Allocate<false>(mCount + 1);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"
					" - you should first clone the container"));

			CropInner(1, 0, mCount)
				.template CallKnownMoveConstructors<T>(
					mCount, CropInner(0, mCount, mCount)
				);

			InsertInner<KEEP>(Move(item), 0);
		}
		else if constexpr (INDEX == IndexBack)
			InsertInner<KEEP>(Move(item), mCount);
		else
			LANGULUS_ASSERT("Invalid index provided; use either IndexBack "
				"or IndexFront, or Block::InsertAt to insert at an offset");

		return 1;
	}

	/// Inner copy-insertion function														
	///	@attention this is an inner function and should be used with caution	
	///	@attention relies that the required free space has been prepared		
	///				  at the appropriate place												
	///	@tparam KEEP - whether or not to reference the new contents				
	///	@tparam T - the type to insert (deducible)									
	///	@param start - pointer to the first item										
	///	@param end - pointer to the end of items										
	///	@param starter - the offset at which to insert								
	template<bool KEEP, CT::NotAbandonedOrDisowned T>
	void Block::InsertInner(const T* start, const T* end, Offset at) {
		const auto count = end - start;
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't copy-insert into container of constant elements");

		// Insert new data																
		if constexpr (CT::Sparse<T>) {
			// Sparse data insertion (copying pointers and referencing)		
			// Doesn't care about abstract items									
			auto data = GetRawSparse() + at;
			while (start != end) {
				data->mPointer = const_cast<Byte*>(
					reinterpret_cast<const Byte*>(*start));

				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					if constexpr (KEEP) {
						// Reference each pointer										
						// Also keep the found entry, so we don't ever			
						// search again (it's costly)									
						// To avoid searching like that, insert as Disowned	
						data->mEntry = Inner::Allocator::Find(mType, *start);
						if (data->mEntry)
							data->mEntry->Keep();
					}
					else data->mEntry = nullptr;
				#else
					data->mEntry = nullptr;
				#endif

				++data; ++start;
			}
		}
		else {
			// Abstract stuff is allowed only if sparse							
			static_assert(!CT::Abstract<T>,
				"Can't insert abstract item in dense container");

			auto data = GetRawAs<T>() + at;
			if constexpr (CT::POD<T>) {
				// Optimized POD insertion												
				CopyMemory(start, data, sizeof(T) * count);
			}
			else {
				// Dense data insertion 												
				while (start != end) {
					if constexpr (KEEP) {
						static_assert(CT::CopyMakable<T>,
							"Can't insert non-copy-constructible item(s)");
						new (data) T {*start};
					}
					else {
						static_assert(::std::constructible_from<T, Disowned<T>&&>,
							"Can't insert non-disowned-constructible item(s)");
						new (data) T {Disown(*start)};
					}

					++data; ++start;
				}
			}
		}

		mCount += count;
	}

	/// Inner move-insertion function														
	///	@attention this is an inner function and should be used with caution	
	///	@attention relies that the required free space has been prepared		
	///				  at the appropriate place												
	///	@tparam KEEP - whether or not to reference the new contents				
	///	@tparam T - the type to insert (deducible)									
	///	@param item - item to move in														
	///	@param starter - the offset at which to insert								
	template<bool KEEP, CT::NotAbandonedOrDisowned T>
	void Block::InsertInner(T&& item, Offset at) {
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't move-insert into container of constant elements");

		// Insert new data																
		if constexpr (CT::Sparse<T>) {
			// Sparse data insertion (moving a pointer)							
			const auto data = GetRawSparse() + at;
			data->mPointer = const_cast<Byte*>(
				reinterpret_cast<const Byte*>(item));

			// Reference the pointer's memory										
			#if LANGULUS_FEATURE(MANAGED_MEMORY)
				if constexpr (KEEP) {
					data->mEntry = Inner::Allocator::Find(mType, item);
					if (data->mEntry)
						data->mEntry->Keep();
				}
				else data->mEntry = nullptr;
			#else
				data->mEntry = nullptr;
			#endif
		}
		else {
			static_assert(!CT::Abstract<T>,
				"Can't emplace abstract item in dense container");

			// Dense data insertion (placement move-construction)				
			const auto data = GetRawAs<T>() + at;
			if constexpr (KEEP) {
				if constexpr (CT::MoveMakable<T>)
					new (data) T {Forward<T>(item)};
				else if constexpr (CT::CopyMakable<T>)
					new (data) T {item};
				else
					LANGULUS_ASSERT("Can't emplace non-move/copy-constructible item");
			}
			else if constexpr (CT::AbandonMakable<T>)
				new (data) T {Abandon(item)};
			else if constexpr (CT::Fundamental<T>)
				new (data) T {item};
			else
				LANGULUS_ASSERT("Can't emplace non-abandon-constructible item");
		}

		++mCount;
	}

	/// Remove non-sequential element(s)													
	///	@tparam T - the type to insert (deducible)									
	///	@param items - the items to search for and remove							
	///	@param count - number of items inside array									
	///	@param index - the index to start searching from							
	///	@return the number of removed items												
	template<CT::Data T>
	Count Block::Remove(const T* items, const Count count, Index index) {
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't remove elements from container of constant elements");
		Count removed {};
		const auto itemsEnd = items + count;
		while (items != itemsEnd) {
			const auto idx = Find<T>(*items, index);
			if (idx)
				removed += RemoveIndex(idx.GetOffset(), 1);
			++items;
		}

		return removed;
	}

	/// Find first matching element position inside container						
	///	@param item - the item to search for											
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	template<CT::Data T>
	Index Block::Find(const T& item, const Index& idx) const {
		if (!mCount || !mType)
			return IndexNone;

		if (IsDense()) {
			if (!CastsTo<T>()) {
				// If dense and not forward compatible - fail					
				return IndexNone;
			}
		}
		else if (!MetaData::Of<T>()->CastsTo(mType)) {
			// If sparse and not backwards compatible - fail					
			return IndexNone;
		}

		// Setup the iterator															
		Index starti, istep;
		if (idx == IndexFront) {
			starti = 0;
			istep = 1;
		}
		else if (idx == IndexBack) {
			starti = mCount - 1;
			istep = -1;
		}
		else {
			starti = Constrain(idx);
			istep = 1;
			if (starti + 1 >= mCount)
				return IndexNone;
		}

		// Search																			
		auto item_ptr = SparseCast(item);
		for (auto i = starti; i < mCount && i >= 0; i += istep) {
			auto left = SparseCast(Get<T>(i.GetOffset()));
			if (left == item_ptr) {
				// Early success if pointers match									
				return i;
			}

			if constexpr (CT::Sparse<T>) {
				// If searching for pointers - cease after pointer check		
				continue;
			}
			else if constexpr (CT::Resolvable<T>) {
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
			else if constexpr (CT::Comparable<T>) {
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
		return IndexNone;
	}
	
	/// Find first matching element position inside container, deeply				
	///	@param item - the item to search for											
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	template<CT::Data T>
	Index Block::FindDeep(const T& item, const Index& idx) const {
		Index found;
		ForEachDeep([&](const Block& group) {
			found = group.Find<T>(item, idx);
			return !found;
		});

		return found;
	}

	/// Merge-copy-insert array elements at special index								
	/// Each element will be pushed only if not found in block						
	/// A bit of runtime overhead due to resolving index								
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen or incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param start - pointer to the first item										
	///	@param end - pointer to the end of items										
	///	@param index - the special index to insert at								
	///	@return the number of inserted elements										
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::MergeAt(const T* start, const T* end, Index index) {
		const auto offset = ConstrainMore<T>(index).GetOffset(); //TODO constraining assumes this is filled with T? might cause problems :(
		return MergeAt<WRAPPER, KEEP, MUTABLE, T>(start, end, offset);
	}

	/// Merge-copy-insert array elements at special index								
	/// Each element will be pushed only if not found in block						
	///	@attention assumes offset is in the block's limits							
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen or incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param start - pointer to the first item										
	///	@param end - pointer to the end of items										
	///	@param index - the special index to insert at								
	///	@return the number of inserted elements										
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::MergeAt(const T* start, const T* end, Offset index) {
		Count added {};
		while (start != end) {
			if (!Find<T>(*start)) {
				added += InsertAt<WRAPPER, KEEP, MUTABLE, T>(start, start + 1, index);
				++index;
			}

			++start;
		}

		return added;
	}

	/// Merge-move-insert array elements at special index								
	/// Element will be pushed only if not found in block								
	/// A bit of runtime overhead due to resolving index								
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen or incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param item - the item to move in												
	///	@param index - the special index to insert at								
	///	@return the number of inserted elements										
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::MergeAt(T&& item, Index index) {
		if (!Find<T>(item))
			return InsertAt<WRAPPER, KEEP, MUTABLE, T>(Move(item), index);
		return 0;
	}

	/// Merge-move-insert array elements at simple index								
	/// Element will be pushed only if not found in block								
	///	@attention assumes offset is in the block's limits							
	///	@tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled	
	///	@tparam KEEP - whether to reference data on copy							
	///	@tparam MUTABLE - is it allowed the block to deepen or incorporate	
	///							the new insertion, if not compatible					
	///	@tparam T - the type to insert (deducible)									
	///	@param item - the item to move in												
	///	@param index - the simple index to insert at									
	///	@return the number of inserted elements										
	template<CT::Data WRAPPER, bool KEEP, bool MUTABLE, CT::NotAbandonedOrDisowned T>
	Count Block::MergeAt(T&& item, Offset index) {
		if (!Find<T>(item))
			return InsertAt<WRAPPER, KEEP, MUTABLE, T>(Move(item), index);
		return 0;
	}

	/// Get the index of the biggest element												
	///	@attention assumes that T is binary-compatible with contained type	
	///	@tparam T - the type to use for comparison									
	///	@return the index of the biggest element T inside this block			
	template<CT::Data T>
	Index Block::GetIndexMax() const noexcept requires CT::Sortable<T, T> {
		if (IsEmpty())
			return IndexNone;

		auto data = Get<Decay<T>*>();
		auto max = data;
		for (Offset i = 1; i < mCount; ++i) {
			if (DenseCast(data[i]) > DenseCast(*max))
				max = data + i;
		}

		return max - data;
	}

	/// Get the index of the smallest element												
	///	@attention assumes that T is binary-compatible with contained type	
	///	@tparam T - the type to use for comparison									
	///	@return the index of the smallest element T inside this block			
	template<CT::Data T>
	Index Block::GetIndexMin() const noexcept requires CT::Sortable<T, T> {
		if (IsEmpty())
			return IndexNone;

		auto data = Get<Decay<T>*>();
		auto min = data;
		for (Offset i = 1; i < mCount; ++i) {
			if (DenseCast(data[i]) < DenseCast(*min))
				min = data + i;
		}

		return min - data;
	}

	/// Get the index of dense element that repeats the most times					
	///	@attention assumes that T is binary-compatible with contained type	
	///	@tparam T - the type to use for comparison									
	///	@param count - [out] count the number of repeats for the mode			
	///	@return the index of the first found mode										
	template<CT::Data T>
	Index Block::GetIndexMode(Count& count) const noexcept {
		if (IsEmpty()) {
			count = 0;
			return IndexNone;
		}

		auto data = Get<Decay<T>*>();
		decltype(data) best = nullptr;
		Count best_count {};
		for (Offset i = 0; i < mCount; ++i) {
			Count counter {};
			for (Count j = i; j < mCount; ++j) {
				if constexpr (CT::Comparable<T, T>) {
					// First we compare by memory pointer, then by ==			
					if (SparseCast(data[i]) == SparseCast(data[j]) ||
						 DenseCast(data[i])  == DenseCast(data[j]))
						++counter;
				}
				else {
					// No == operator, so just compare by memory	pointer		
					if (SparseCast(data[i]) == SparseCast(data[j]))
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
	///	@attention assumes that T is binary-compatible with contained type	
	///	@tparam T - the type to use for comparison									
	///	@param first - what will the first element be after sorting?			
	///					 - use uiSmallest for 123, or anything else for 321		
	template<CT::Data T>
	void Block::Sort(const Index& first) noexcept {
		auto data = Get<Decay<T>*>();
		if (!data)
			return;

		Count j {}, i {};
		if (first == IndexSmallest) {
			for (; i < mCount; ++i) {
				for (; j < i; ++j) {
					if (*SparseCast(data[i]) > *SparseCast(data[j]))
						Swap<T>(i, j);
				}
				for (j = i + 1; j < mCount; ++j) {
					if (*SparseCast(data[i]) > *SparseCast(data[j]))
						Swap<T>(i, j);
				}
			}
		}
		else {
			for (; i < mCount; ++i) {
				for (; j < i; ++j) {
					if (*SparseCast(data[i]) < *SparseCast(data[j]))
						Swap<T>(i, j);
				}
				for (j = i + 1; j < mCount; ++j) {
					if (*SparseCast(data[i]) < *SparseCast(data[j]))
						Swap<T>(i, j);
				}
			}
		}
	}

	/// A smart insert uses the best approach to push anything inside				
	/// container in order to keep hierarchy and states, but also reuse memory	
	///	@tparam ALLOW_CONCAT - whether or not concatenation is allowed			
	///	@tparam ALLOW_DEEPEN - whether or not deepening is allowed				
	///	@tparam T - type of data to push (deducible)									
	///	@tparam WRAPPER - type of container used for deepening if enabled		
	///	@param value - the value to smart-push											
	///	@param index - the index at which to insert (if needed)					
	///	@param state - a state to apply after pushing is done						
	///	@return the number of pushed items (zero if unsuccessful)				
	template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data T, CT::Data INDEX, CT::Data WRAPPER>
	Count Block::SmartPushAt(T value, INDEX index, DataState state) {
		static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");

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

		UNUSED()
		const bool orCompliant = !(mCount > 1 && !IsOr() && state.IsOr());
		
		if constexpr (ALLOW_CONCAT) {
			// If this container is compatible and concatenation is enabled
			// try concatenating the two containers. Concatenation will not
			// be allowed if final state is OR, and there are multiple		
			// items	in this container.												
			Count catenated {};
			if (typeCompliant && stateCompliant && orCompliant && 0 < (catenated = InsertBlockAt(pack, index))) {
				SetState(mState + state);
				return catenated;
			}
		}

		// If this container is deep, directly push the pack inside			
		// This will be disallowed if final state is OR, and there are		
		// multiple items	in this container.										
		if (orCompliant && IsDeep()) {
			SetState(mState + state);
			return InsertAt<WRAPPER, false, false>(WRAPPER {pack}, index);
		}

		// Finally, if allowed, force make the container deep in order to	
		// push the pack inside															
		if constexpr (ALLOW_DEEPEN) {
			if (!IsTypeConstrained()) {
				Deepen<WRAPPER>();
				SetState(mState + state);
				return InsertAt<WRAPPER, false, false>(WRAPPER {pack}, index);
			}
		}

		return 0;
	}

	/// A smart copy-insert uses the best approach to push anything inside		
	/// container in order to keep hierarchy and states, but also reuse memory	
	///	@tparam ALLOW_CONCAT - whether or not concatenation is allowed			
	///	@tparam ALLOW_DEEPEN - whether or not deepening is allowed				
	///	@tparam T - type of data to push (deducible)									
	///	@tparam WRAPPER - type of container used for deepening if enabled		
	///	@param value - the value to smart-push											
	///	@param index - the index at which to insert (if needed)					
	///	@param state - a state to apply after pushing is done						
	///	@return the number of pushed items (zero if unsuccessful)				
	template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data T, CT::Data WRAPPER>
	Count Block::SmartPush(T value, DataState state) {
		static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");

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

		UNUSED()
		const bool orCompliant = !(mCount > 1 && !IsOr() && state.IsOr());
		
		if constexpr (ALLOW_CONCAT) {
			// If this container is compatible and concatenation is enabled
			// try concatenating the two containers. Concatenation will not
			// be allowed if final state is OR, and there are multiple		
			// items	in this container.												
			Count catenated {};
			if (typeCompliant && stateCompliant && orCompliant && 0 < (catenated = InsertBlock<INDEX>(pack))) {
				SetState(mState + state);
				return catenated;
			}
		}

		// If this container is deep, directly push the pack inside			
		// This will be disallowed if final state is OR, and there are		
		// multiple items	in this container.										
		if (orCompliant && IsDeep()) {
			SetState(mState + state);
			return Insert<INDEX, WRAPPER, false, false>(WRAPPER {pack});
		}

		// Finally, if allowed, force make the container deep in order to	
		// push the pack inside															
		if constexpr (ALLOW_DEEPEN) {
			if (!IsTypeConstrained()) {
				Deepen<WRAPPER>();
				SetState(mState + state);
				return Insert<INDEX, WRAPPER, false, false>(WRAPPER {pack});
			}
		}

		return 0;
	}

	/// Wrap all contained elements inside a sub-block, making this one deep	
	///	@tparam T - the type of deep container to use								
	///	@tparam MOVE_STATE - whether or not to send the current state over	
	///	@return a reference to this container											
	template<CT::Data T, bool MOVE_STATE>
	T& Block::Deepen() {
		static_assert(CT::Deep<T>, "T must be deep");

		if (IsTypeConstrained() && !Is<T>()) {
			Throw<Except::Mutate>(
				"Attempting to deepen incompatible typed container");
		}

		if (GetUses() > 1) {
			Throw<Except::Mutate>(
				"Attempting to deepen container that is referenced from multiple locations");
		}

		// Back up the state so that we can restore it if not moved over	
		UNUSED() const auto state {GetUnconstrainedState()};
		if constexpr (!MOVE_STATE)
			mState -= state;

		// Allocate a new T and move this inside it								
		Block wrapper;
		wrapper.SetType<T, false>();
		wrapper.Allocate<true>(1);
		wrapper.Get<Block>() = Move(*this);
		*this = wrapper;
		
		// Restore the state of not moved over										
		if constexpr (!MOVE_STATE)
			mState += state;

		return Get<T>();
	}

	/// Get an element via simple index, trying to interpret it as T				
	/// No conversion or copying shall occur in this routine, only pointer		
	/// arithmetic based on CTTI or RTTI													
	///	@tparam T - the type to interpret to											
	///	@tparam IDX - the type used for indexing (deducible)						
	///	@param idx - simple index for accessing - use negative for reverse,	
	///		or special indices for smart indexing										
	///	@return either pointer or reference to the element (depends on T)		
	template<CT::Data T, CT::Index IDX>
	decltype(auto) Block::As(const IDX& index) {
		// Constrain the index if needed												
		Offset idx;
		if constexpr (CT::Same<IDX, Index>)
			idx = ConstrainMore<T>(index).GetOffset();
		else if constexpr (CT::Signed<IDX>) {
			if (index < 0)
				idx = mCount - static_cast<Offset>(-index);
			else
				idx = static_cast<Offset>(index);
		}
		else idx = index;

		// First quick type stage for fast access									
		if (mType->Is<T>())
			return Get<T>(idx);

		// Second fallback stage for compatible bases and mappings			
		RTTI::Base base;
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
			Throw<Except::Access>("Type mismatch on Block::As");
		}

		// Get base memory of the required element and access					
		return 
			GetElementDense(idx / base.mCount)
				.GetBaseMemory(base)
					.Get<T>(idx % base.mCount);
	}

	template<CT::Data T, CT::Index IDX>
	decltype(auto) Block::As(const IDX& index) const {
		return const_cast<Block&>(*this).template As<T, IDX>(index);
	}

	/// Execute functions for each element inside container							
	/// Function returns immediately after the first viable iterator is done	
	///	@tparam MUTABLE - whether or not a change to container is allowed		
	///							while iterating												
	///	@tparam F - the function types (deducible)									
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool MUTABLE, class... F>
	Count Block::ForEach(F&&... calls) {
		return (... || ForEachSplitter<MUTABLE, false>(Forward<F>(calls)));
	}

	/// Execute functions for each element inside container (immutable)			
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<class... F>
	Count Block::ForEach(F&&... calls) const {
		return const_cast<Block&>(*this).ForEach<false>(Forward<F>(calls)...);
	}

	/// Execute functions for each element inside container (reverse)				
	///	@tparam MUTABLE - whether or not a change to container is allowed		
	///							while iterating												
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool MUTABLE, class... F>
	Count Block::ForEachRev(F&&... calls) {
		return (... || ForEachSplitter<MUTABLE, true>(Forward<F>(calls)));
	}

	/// Execute F for each element inside container (immutable, reverse)			
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<class... F>
	Count Block::ForEachRev(F&&... calls) const {
		return const_cast<Block&>(*this).ForEachRev<false>(Forward<F>(calls)...);
	}

	/// Execute functions for each element inside container, nested for any		
	/// contained deep containers																
	///	@tparam SKIP - set to false, to execute F for containers, too			
	///						set to true, to execute only for non-deep elements		
	///	@tparam MUTABLE - whether or not a change to container is allowed		
	///							while iterating												
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool SKIP, bool MUTABLE, class... F>
	Count Block::ForEachDeep(F&&... calls) {
		return (... || ForEachDeepSplitter<SKIP, MUTABLE, false>(Forward<F>(calls)));
	}

	/// Execute function F for each element inside container, nested for any	
	/// contained deep containers (immutable)												
	///	@tparam SKIP - set to false, to execute F for containers, too			
	///						set to true, to execute only for non-deep elements		
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool SKIP, class... F>
	Count Block::ForEachDeep(F&&... calls) const {
		return const_cast<Block&>(*this).ForEachDeep<SKIP, false>(Forward<F>(calls)...);
	}

	/// Execute function F for each element inside container, nested for any	
	/// contained deep containers (reverse)												
	///	@tparam SKIP - set to false, to execute F for containers, too			
	///						set to true, to execute only for non-deep elements		
	///	@tparam MUTABLE - whether or not a change to container is allowed		
	///							while iterating												
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool SKIP, bool MUTABLE, class... F>
	Count Block::ForEachDeepRev(F&&... calls) {
		return (... || ForEachDeepSplitter<SKIP, MUTABLE, true>(Forward<F>(calls)));
	}

	/// Execute function F for each element inside container, nested for any	
	/// contained deep containers (immutable, reverse)									
	///	@tparam SKIP - set to false, to execute F for containers, too			
	///						set to true, to execute only for non-deep elements		
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool SKIP, class... F>
	Count Block::ForEachDeepRev(F&&... calls) const {
		return const_cast<Block*>(this)->ForEachDeepRev<SKIP, false>(Forward<F>(calls)...);
	}

	/// Execute functions for each element inside container							
	///	@tparam MUTABLE - whether or not a change to container is allowed		
	///							while iterating												
	///	@tparam F - the function types (deducible)									
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool MUTABLE, bool REVERSE, class F>
	Count Block::ForEachSplitter(F&& call) {
		using A = decltype(GetLambdaArgument(&F::operator()));
		using R = decltype(call(Uneval<A>()));

		static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
			"Non constant iterator for constant memory block");

		return ForEachInner<R, A, REVERSE, MUTABLE>(Forward<F>(call));
	}

	/// Execute functions for each element inside container, nested for any		
	/// contained deep containers																
	///	@tparam SKIP - set to false, to execute F for containers, too			
	///						set to true, to execute only for non-deep elements		
	///	@tparam MUTABLE - whether or not a change to container is allowed		
	///							while iterating												
	///	@tparam F - the function type (deducible)										
	///	@param call - the instance of the function F to call						
	///	@return the number of called functions											
	template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
	Count Block::ForEachDeepSplitter(F&& call) {
		using A = decltype(GetLambdaArgument(&F::operator()));
		using R = decltype(call(Uneval<A>()));

		static_assert(CT::Constant<A> || (CT::Mutable<A> && MUTABLE),
			"Non constant iterator for constant memory block");

		if constexpr (CT::Deep<A>) {
			// If argument type is deep												
			return ForEachDeepInner<R, A, REVERSE, SKIP, MUTABLE>(Forward<F>(call));
		}
		else if constexpr (CT::Constant<A>) {
			// Any other type is wrapped inside another ForEachDeep call	
			return ForEachDeep<SKIP, MUTABLE>([&call](const Block& block) {
				block.ForEach(Forward<F>(call));
			});
		}
		else {
			// Any other type is wrapped inside another ForEachDeep call	
			return ForEachDeep<SKIP, MUTABLE>([&call](Block& block) {
				block.ForEach(Forward<F>(call));
			});
		}
	}

	/// Iterate and execute call for each element										
	///	@param call - the function to execute for each element of type T		
	///	@return the number of executions that occured								
	template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
	Count Block::ForEachInner(TFunctor<R(A)>&& call) {
		if (IsEmpty())
			return 0;
		 
		UNUSED() auto initialCount = mCount;
		constexpr bool HasBreaker = CT::Same<bool, R>;
		Count index {};
		if (mType->Is<A>()) {
			// Fast specialized routine that gives direct access				
			// Uses Get<>() instead of As<>()										
			while (index < mCount) {
				// Iterator is a reference												
				if constexpr (CT::Dense<A>) {
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
			// Slow generalized routine that resolves each element			
			// Uses As<>() instead of Get<>()										
			Count successes {};
			while (index < mCount) {
				try {
					// Iterator is a reference											
					if constexpr (CT::Dense<A>) {
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
	template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
	Count Block::ForEachDeepInner(TFunctor<R(A)>&& call) {
		constexpr bool HasBreaker = CT::Same<bool, R>;
		UNUSED() bool atLeastOneChange = false;
		auto count {GetCountDeep()};
		Count index {};
		while (index < count) {
			auto block = ReinterpretCast<A>(GetBlockDeep(index));
			if constexpr (MUTABLE) {
				if (!block)
					break;
			}

			if constexpr (SKIP) {
				// Skip deep/empty sub blocks											
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
							block = ReinterpretCast<A>(GetBlockDeep(index - 1));
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
		result.mRaw += start * GetStride();
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
		result.mRaw += start * GetStride();
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
		result.MakeConst();
		return result;
	}
	
	/// A helper function, that allocates and moves inner memory					
	///	@param other - the memory we'll be inserting									
	///	@param index - the place we'll be inserting at								
	///	@param region - the newly allocated region (!mCount, only mReserved)	
	///	@return number if inserted items in case of mutation						
	inline void Block::AllocateRegion(const Block& other, Offset index, Block& region) {
		// Type may mutate, but never deepen										
		Mutate<false>(other.mType);

		// Allocate the required memory - this will not initialize it		
		Allocate<false>(mCount + other.mCount);

		// Move memory if required														
		if (index < mCount) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"));

			CropInner(index + other.mCount, 0, mCount - index)
				.template CallUnknownMoveConstructors<false>(
					mCount - index,
					CropInner(index, mCount - index, mCount - index)
				);
		}

		// Pick the region that should be overwritten with new stuff		
		region = CropInner(index, 0, other.mCount);
	}

	/// Call destructors in a region - after this call the memory is not			
	/// considered initialized, but mCount is still valid, so be careful			
	///	@attention this function is intended for internal use						
	///	@attention this operates on initialized memory only, and any			
	///				  misuse will result in undefined behavior						
	///	@attention assumes that mCount > 0												
	///	@tparam T - the type to destroy													
	template<CT::Data T>
	void Block::CallKnownDestructors() {
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't destroy constant elements");
		using DT = Decay<T>;

		if constexpr (CT::Sparse<T>) {
			auto data = GetRawSparse();
			const auto dataEnd = data + mCount;

			// We dereference each pointer - destructors will be called		
			// if data behind these pointers is fully dereferenced, too		
			while (data != dataEnd) {
				if (!data->mEntry) {
					++data;
					continue;
				}

				if (data->mEntry->GetUses() == 1) {
					if constexpr (CT::Destroyable<T>)
						reinterpret_cast<DT*>(data->mPointer)->~DT();
					Inner::Allocator::Deallocate(data->mEntry);
				}
				else data->mEntry->Free();

				++data;
			}

			return;
		}
		else if constexpr (!CT::POD<T> && CT::Destroyable<T>) {
			auto data = GetRawAs<T>();
			const auto dataEnd = data + mCount;

			// Destroy every dense element											
			while (data != dataEnd) {
				data->~DT();
				++data;
			}
		}

		// Always nullify upon destruction only if we're paranoid			
		PARANOIA(FillMemory(data, {}, GetByteSize()));
	}
	
	/// Call destructors in a region - after this call the memory is not			
	/// considered initialized, but mCount is still valid, so be careful			
	///	@attention this function is intended for internal use						
	///	@attention this operates on initialized memory only, and any			
	///				  misuse will result in undefined behavior						
	///	@attention mCount is not zeroed in this call									
	inline void Block::CallUnknownDestructors() {
		if (IsSparse()) {
			auto data = GetRawSparse();
			const auto dataEnd = data + mCount;

			// We dereference each pointer - destructors will be called		
			// if data behind these pointers is fully dereferenced, too		
			while (data != dataEnd) {
				if (!data->mEntry) {
					++data;
					continue;
				}

				if (data->mEntry->GetUses() == 1) {
					if (!mType->mIsPOD) {
						if (!mType->mDestructor) {
							Throw<Except::Destruct>(
								"Can't destroy elements - no destructor was reflected");
						}

						Inner::Allocator::Deallocate(data->mEntry);
					}
				}
				else data->mEntry->Free();

				++data;
			}

			return;
		}
		else if (!mType->mIsPOD && mType->mDestructor) {
			// Destroy every dense element, one by one, using the 			
			// reflected destructors (if any)										
			auto data = GetRaw();
			const auto dataStride = mType->mSize;
			const auto dataEnd = data + mCount * mType->mSize;

			// Destroy every dense element											
			while (data != dataEnd) {
				mType->mDestructor(data);
				data += dataStride;
			}
		}

		#if LANGULUS_PARANOID()
			// Nullify upon destruction only if we're paranoid					
			FillMemory(mRaw, {}, GetByteSize());
		#endif
	}

	/// Call move constructors in a region and initialize memory					
	///	@tparam KEEP - true to use move-construction, false to use abandon	
	///	@tparam T - the type to move-construct											
	///	@param count - number of elements to move										
	///	@param source - the block of elements to move								
	template<bool KEEP, CT::Data T>
	void Block::CallKnownMoveConstructors(const Count count, Block&& source) {
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't move-construct in container of constant elements");

		if constexpr (CT::Sparse<T>) {
			// Move known pointers														
			const auto to = GetRawSparse() + mCount;
			const auto size = sizeof(KnownPointer) * count;
			MoveMemory(source.mRaw, to, size);

			// It is safe to just erase source count at this point			
			// since pointers/PODs don't need to be destroyed					
			source.mCount = 0;
		}
		else if constexpr (CT::POD<T>) {
			// Copy POD																		
			const auto to = GetRawAs<T>() + mCount;
			const auto size = sizeof(T) * count;
			MoveMemory(source.mRaw, to, size);

			// It is safe to just erase source count at this point			
			// since pointers/PODs don't need to be destroyed					
			source.mCount = 0;
		}
		else {
			// Both RHS and LHS are dense and non POD								
			// Call the move-constructor for each element						
			static_assert(CT::MoveMakable<T>, 
				"Trying to move-construct but it's impossible for this type");

			auto to = GetRawAs<T>() + mCount;
			auto from = source.GetRawAs<T>();
			const auto fromEnd = from + count;
			while (from != fromEnd) {
				if constexpr (KEEP)
					new (to) T {Move(*from)};
				else
					new (to) T {Abandon(*from)};

				++to; ++from;
			}

			// Note that source.mCount remains, in order to call				
			// destructors at a later point (if T is destroyable at all)	
			// If we're abandoning it, then no need to destroy at all		
			if constexpr (!CT::Destroyable<T> || !KEEP)
				source.mCount = 0;
		}
		
		// Mark the initialized count													
		mCount += count;
	}
	
	/// Call move constructors in a region and initialize memory					
	///	@tparam KEEP - true to use move-construction, false to use abandon	
	///	@attention this operates on uninitialized memory only, and any			
	///		misuse will result in loss of data and undefined behavior			
	///	@attention source must have a binary-compatible type						
	///	@attention source must contain at least mReserved - mCount items		
	///	@attention after the move, source will have zero count,					
	///		signifying that items have been consumed, but is still allocated	
	///	@param source - the elements to move											
	template<bool KEEP>
	void Block::CallUnknownMoveConstructors(const Count count, Block&& source) {
		if (IsSparse() && source.IsSparse()) {
			// Copy pointers																
			const auto size = sizeof(KnownPointer) * count;
			MoveMemory(source.mRaw, GetRawEnd(), size);
		}
		else if (mType->mIsPOD && IsDense() == source.IsDense()) {
			// Copy POD																		
			const auto size = mType->mSize * count;
			MoveMemory(source.mRaw, GetRawEnd(), size);
		}
		else if (source.IsSparse()) {
			// RHS is pointer, LHS must be dense									
			// Copy each dense element from RHS										
			if constexpr (KEEP) {
				if (!mType->mMoveConstructor) {
					Throw<Except::Construct>(
						"Can't move-construct elements - no move constructor was reflected");
				}
			}
			else {
				if (!mType->mAbandonConstructor) {
					Throw<Except::Construct>(
						"Can't abandon-construct elements - no abandon constructor was reflected");
				}
			}

			auto to = GetRawEnd();
			const auto toStride = mType->mSize;
			auto pointer = source.GetRawSparse();
			const auto pointersEnd = pointer + count;
			while (pointer != pointersEnd) {
				if constexpr (KEEP)
					mType->mMoveConstructor(to, pointer->mPointer);
				else
					mType->mAbandonConstructor(to, pointer->mPointer);

				to += toStride;
				++pointer;
			}
		}
		else if (IsSparse()) {
			// LHS is pointer, RHS must be dense									
			// Move each pointer from RHS												
			auto to = GetRawSparse() + mCount;
			const auto toEnd = to + count;
			auto from = source.GetRaw();
			const auto fromStride = source.mType->mSize;
			while (to != toEnd) {
				to->mPointer = const_cast<Byte*>(from);
				to->mEntry = source.mEntry;
				++to;
				from += fromStride;
			}

			if constexpr (KEEP) {
				// We have to reference RHS by the number of pointers we		
				// made. Since we're converting dense to sparse, the			
				// referencing is	mandatory											
				source.mEntry->Keep(count);
			}
		}
		else {
			// Both RHS and LHS must be dense										
			if constexpr (KEEP) {
				if (!mType->mMoveConstructor) {
					Throw<Except::Construct>(
						"Can't move-construct elements - no move constructor was reflected");
				}
			}
			else {
				if (!mType->mAbandonConstructor) {
					Throw<Except::Construct>(
						"Can't abandon-construct elements - no abandon constructor was reflected");
				}
			}

			auto to = GetRawEnd();
			auto from = source.GetRaw();
			const auto stride = mType->mSize;
			const auto fromEnd = from + count * stride;
			while (from != fromEnd) {
				if constexpr (KEEP)
					mType->mMoveConstructor(to, from);
				else
					mType->mAbandonConstructor(to, from);

				to += stride;
				from += stride;
			}
		}

		mCount += count;
	}

	/// Call default constructors in a region and initialize memory				
	///	@param count - the number of elements to initialize						
	template<CT::Data T>
	void Block::CallKnownDefaultConstructors(Count count) {
		if constexpr (CT::Nullifiable<T>) {
			// Just zero the memory (optimization)									
			FillMemory(GetRawEnd(), {}, count * GetStride());
			mCount += count;
		}
		else if constexpr (CT::Defaultable<T>) {
			// Construct requested elements in place								
			new (GetRawEnd()) T [count];
			mCount += count;
		}
		else LANGULUS_ASSERT(
			"Trying to default-construct elements that are "
			"incapable of default-construction");
	}
	
	/// Call default constructors in a region and initialize memory				
	///	@attention this is a type-erased call and has quite the overhead		
	///	@attention assumes mType is set													
	///	@attention this operates on uninitialized memory only, and any			
	///		misuse will result in loss of data and undefined behavior			
	///	@param count - the number of elements to initialize						
	inline void Block::CallUnknownDefaultConstructors(Count count) {
		if (IsSparse()) {
			// Just zero the memory (optimization)									
			FillMemory(GetRawEnd(), {}, count * sizeof(KnownPointer));
		}
		else if (mType->mIsNullifiable) {
			// Just zero the memory (optimization)									
			FillMemory(GetRawEnd(), {}, count * mType->mSize);
		}
		else {
			if (!mType->mDefaultConstructor) {
				Throw<Except::Construct>(
					"Can't default-construct elements - no constructor was reflected");
			}
			
			// Construct requested elements one by one							
			auto to = GetRawEnd();
			const auto stride = mType->mSize;
			const auto toEnd = to + count * stride;
			while (to != toEnd) {
				mType->mDefaultConstructor(to);
				to += stride;
			}
		}
		
		mCount += count;
	}

	/// Call copy constructors in a region and initialize memory					
	///	@param source - the elements to copy											
	template<bool KEEP, CT::Data T>
	void Block::CallKnownCopyConstructors(Count count, const Block& source) {
		if constexpr (CT::Sparse<T> || CT::POD<T>) {
			// Just copy the POD/pointer memory (optimization)					
			CopyMemory(source.GetRaw(), GetRawEnd(), GetStride() * count);

			if constexpr (CT::Sparse<T> && KEEP) {
				// Since we're copying pointers, we have to reference the	
				// dense memory behind each one of them							
				auto p = GetRawSparse() + mCount;
				const auto pEnd = p + count;
				while (p != pEnd) {
					// Reference each pointer											
					if (p->mEntry)
						p->mEntry->Keep();
					++p;
				}
			}
		}
		else {
			// Both RHS and LHS are dense and non POD								
			// Call the reflected copy-constructor for each element			
			static_assert(CT::CopyMakable<T>, 
				"Trying to copy-construct but it's impossible for this type");

			auto to = GetRawAs<T>() + mCount;
			auto from = source.GetRawAs<T>();
			const auto fromEnd = from + count;
			while (from != fromEnd) {
				if constexpr (KEEP)
					new (to) T {*from};
				else
					new (to) T {Abandon(*from)};
				++to;
				++from;
			}
		}
	}
	
	/// Call copy constructors in a region and initialize memory					
	///	@attention assumes mType is set													
	///	@attention this operates on uninitialized memory only, and any			
	///		misuse will result in loss of data and undefined behavior			
	///	@attention source must have a binary-compatible type						
	///	@param source - the elements to copy											
	template<bool KEEP>
	void Block::CallUnknownCopyConstructors(Count count, const Block& source) {
		if (IsSparse() && source.IsSparse()) {
			// Copy the known pointers (optimization)								
			CopyMemory(source.mRaw, GetRawEnd(), count * sizeof(KnownPointer));

			if constexpr (KEEP) {
				// Since we're copying pointers, we have to reference the	
				// dense memory behind each one of them							
				auto pointer = GetRawSparse() + mCount;
				const auto pointersEnd = pointer + count;
				while (pointer != pointersEnd) {
					if (pointer->mEntry)
						pointer->mEntry->Keep();
					++pointer;
				}
			}

			return;
		}
		else if (mType->mIsPOD && IsDense() == source.IsDense()) {
			// Just copy the POD memory (optimization)							
			CopyMemory(source.mRaw, GetRawEnd(), count * mType->mSize);
			return;
		}

		// Construct element by element												
		if (IsSparse()) {
			// LHS is pointer, RHS must be dense									
			// Get each pointer from RHS, and reference it						
			auto to = GetRawSparse() + mCount;
			const auto toEnd = to + count;
			auto from = source.GetRaw();
			const auto fromStride = source.mType->mSize;
			while (to != toEnd) {
				to->mPointer = const_cast<Byte*>(from);
				to->mEntry = source.mEntry;
				++to;
				from += fromStride;
			}

			if constexpr (KEEP)
				source.mEntry->Keep(count);
		}
		else if (source.IsSparse()) {
			// RHS is pointer, LHS must be dense									
			// Shallow-copy each dense element from RHS							
			if constexpr (KEEP) {
				if (!mType->mCopyConstructor) {
					Throw<Except::Construct>(
						"Can't copy-construct elements - no copy-constructor was reflected");
				}
			}
			else {
				if (!mType->mDisownConstructor) {
					Throw<Except::Construct>(
						"Can't disown-construct - no disown constructor was reflected");
				}
			}

			auto to = GetRawEnd();
			const auto toStride = mType->mSize;
			auto pointer = source.GetRawSparse();
			const auto pointerEnd = pointer + count;
			while (pointer != pointerEnd) {
				if constexpr (KEEP)
					mType->mCopyConstructor(to, pointer->mPointer);
				else
					mType->mDisownConstructor(to, pointer->mPointer);

				++pointer;
				to += toStride;
			}
		}
		else {
			// Both RHS and LHS must be dense										
			// Call the reflected copy-constructor for each element			
			if constexpr (KEEP) {
				if (!mType->mCopyConstructor) {
					Throw<Except::Construct>(
						"Can't copy-construct elements - no copy constructor was reflected");
				}
			}
			else {
				if (!mType->mDisownConstructor) {
					Throw<Except::Construct>(
						"Can't disown-construct - no disown constructor was reflected");
				}
			}

			auto to = GetRawEnd();
			auto from = source.GetRaw();
			const auto stride = mType->mSize;
			const auto fromEnd = from + count * stride;
			while (from != fromEnd) {
				if constexpr (KEEP)
					mType->mCopyConstructor(to, from);
				else
					mType->mDisownConstructor(to, from);

				to += stride;
				from += stride;
			}
		}
		
		mCount += count;
	}
	
	/// Copy-insert all elements of a block at a special index						
	///	@param other - the block to insert												
	///	@param index - special index to insert them at								
	///	@return the number of inserted elements										
	template<CT::NotAbandonedOrDisowned T>
	Count Block::InsertBlockAt(const T& other, Index index) {
		static_assert(CT::Block<T>, "T must be a block type");
		const auto offset = Constrain(index).GetOffset();
		return InsertBlockAt(other, offset);
	}

	/// Copy-insert all elements of a block at a simple index						
	///	@attention assumes that index is inside block's limits					
	///	@param other - the block to insert												
	///	@param index - simple index to insert them at								
	///	@return the number of inserted elements										
	template<CT::NotAbandonedOrDisowned T>
	Count Block::InsertBlockAt(const T& other, Offset index) {
		static_assert(CT::Block<T>, "T must be a block type");
		Block region;
		AllocateRegion(other, index, region);

		if (region.IsAllocated()) {
			// Call copy-constructors in the new region							
			region.template CallUnknownCopyConstructors<true>(other.mCount, other);
			mCount += region.mReserved;
			return region.mReserved;
		}

		return 0;
	}

	/// Move-insert all elements of a block at a special index						
	///	@param other - the block to move in												
	///	@param index - special index to insert them at								
	///	@return the number of inserted elements										
	template<CT::NotAbandonedOrDisowned T>
	Count Block::InsertBlockAt(T&& other, Index index) {
		static_assert(CT::Block<T>, "T must be a block type");
		const auto offset = Constrain(index).GetOffset();
		return InsertBlockAt(Move(other), offset);
	}

	/// Move-insert all elements of a block at a simple index						
	///	@param other - the block to move in												
	///	@param index - simple index to insert them at								
	///	@return the number of inserted elements										
	template<CT::NotAbandonedOrDisowned T>
	Count Block::InsertBlockAt(T&& other, Offset index) {
		static_assert(CT::Block<T>, "T must be a block type");
		Block region;
		AllocateRegion(other, index, region);

		if (region.IsAllocated()) {
			// Call move-constructors in the new region							
			region.template CallUnknownMoveConstructors<true>(other.mCount, Move(other));
			return region.mReserved;
		}

		return 0;
	}

	/// Move-insert all elements of an abandoned block at a special index		
	///	@param other - the block to move in												
	///	@param index - special index to insert them at								
	///	@return the number of inserted elements										
	template<CT::Data T>
	Count Block::InsertBlockAt(Abandoned<T>&& other, Index index) {
		static_assert(CT::Block<T>, "T must be a block type");
		const auto offset = Constrain(index).GetOffset();
		return InsertBlockAt(Move(other), offset);
	}

	/// Move-insert all elements of an abandoned block at a simple index			
	///	@param other - the block to move in												
	///	@param index - simple index to insert them at								
	///	@return the number of inserted elements										
	template<CT::Data T>
	Count Block::InsertBlockAt(Abandoned<T>&& other, Offset index) {
		static_assert(CT::Block<T>, "T must be a block type");
		Block region;
		AllocateRegion(other.mValue, index, region);

		if (region.IsAllocated()) {
			// Call move-constructors in the new region							
			region.template CallUnknownMoveConstructors<false>(other.mValue.mCount, Move(other.mValue));
			return region.mReserved;
		}

		return 0;
	}

	/// Copy-insert all elements of a disowned block at a special index			
	///	@param other - the block to move in												
	///	@param index - special index to insert them at								
	///	@return the number of inserted elements										
	template<CT::Data T>
	Count Block::InsertBlockAt(Disowned<T>&& other, Index index) {
		static_assert(CT::Block<T>, "T must be a block type");
		const auto offset = Constrain(index).GetOffset();
		return InsertBlockAt(Move(other), offset);
	}

	/// Copy-insert all elements of a disowned block at a simple index			
	///	@param other - the block to move in												
	///	@param index - simple index to insert them at								
	///	@return the number of inserted elements										
	template<CT::Data T>
	Count Block::InsertBlockAt(Disowned<T>&& other, Offset index) {
		static_assert(CT::Block<T>, "T must be a block type");
		Block region;
		AllocateRegion(other.mValue, index, region);

		if (region.IsAllocated()) {
			// Call move-constructors in the new region							
			region.template CallUnknownCopyConstructors<false>(other.mValue.mCount, other.mValue);
			return region.mReserved;
		}

		return 0;
	}

	/// Copy-insert all elements of a block either at the start or at end		
	///	@tparam INDEX - either IndexBack or IndexFront								
	///	@tparam T - type of the block to traverse (deducible)						
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::NotAbandonedOrDisowned T>
	Count Block::InsertBlock(const T& other) {
		static_assert(CT::Block<T>, "T must be a block type");

		// Type may mutate, but never deepen										
		Mutate<false>(other.mType);

		// Allocate the required memory - this will not initialize it		
		Allocate<false>(mCount + other.mCount);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"));

			CropInner(other.mCount, 0, mCount)
				.template CallUnknownMoveConstructors<false>(
					mCount, CropInner(0, mCount, mCount)
				);

			CropInner(0, 0, other.mCount)
				.template CallUnknownCopyConstructors<true>(other.mCount, other);
		}
		else {
			CropInner(mCount, 0, other.mCount)
				.template CallUnknownCopyConstructors<true>(other.mCount, other);
		}

		return other.mCount;
	}

	/// Move-insert all elements of a block either at the start or at end		
	///	@tparam INDEX - either IndexBack or IndexFront								
	///	@tparam T - type of the block to traverse (deducible)						
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::NotAbandonedOrDisowned T>
	Count Block::InsertBlock(T&& other) {
		static_assert(CT::Block<T>, "T must be a block type");

		// Type may mutate, but never deepen										
		Mutate<false>(other.mType);

		// Allocate the required memory - this will not initialize it		
		Allocate<false>(mCount + other.mCount);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"));

			CropInner(other.mCount, 0, mCount)
				.template CallUnknownMoveConstructors<false>(
					mCount, CropInner(0, mCount, mCount)
				);

			CropInner(0, 0, other.mCount)
				.template CallUnknownMoveConstructors<false>(other.mCount, Forward<Block>(other));
		}
		else {
			CropInner(mCount, 0, other.mCount)
				.template CallUnknownMoveConstructors<false>(other.mCount, Forward<Block>(other));
		}

		// Fully reset the source block, if it has ownership behavior		
		if constexpr (!CT::Same<Block, T>) {
			const auto pushed = other.mCount;
			other.Reset();
			return pushed;
		}
		else return other.mCount;
	}

	/// Move-insert all elements of an abandoned block either at start/end		
	///	@tparam INDEX - either IndexBack or IndexFront								
	///	@tparam T - type of the block to traverse (deducible)						
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::Data T>
	Count Block::InsertBlock(Abandoned<T>&& other) {
		static_assert(CT::Block<T>, "T must be a block type");

		// Type may mutate, but never deepen										
		Mutate<false>(other.mValue.mType);

		// Allocate the required memory - this will not initialize it		
		Allocate<false>(mCount + other.mValue.mCount);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"));

			CropInner(other.mValue.mCount, 0, mCount)
				.template CallUnknownMoveConstructors<false>(
					mCount, CropInner(0, mCount, mCount)
				);

			CropInner(0, 0, other.mValue.mCount)
				.template CallUnknownMoveConstructors<false>(other.mValue.mCount, Forward<Block>(other.mValue));
		}
		else {
			CropInner(mCount, 0, other.mValue.mCount)
				.template CallUnknownMoveConstructors<false>(other.mValue.mCount, Forward<Block>(other.mValue));
		}

		// Fully reset the source block												
		other.mValue.Free();
		other.mValue.mEntry = nullptr;
		return other.mValue.mCount;
	}

	/// Copy-insert all elements of a disowned block either at start/end			
	///	@tparam INDEX - either IndexBack or IndexFront								
	///	@tparam T - type of the block to traverse (deducible)						
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::Data T>
	Count Block::InsertBlock(Disowned<T>&& other) {
		static_assert(CT::Block<T>, "T must be a block type");

		// Type may mutate, but never deepen										
		Mutate<false>(other.mValue.mType);

		// Allocate the required memory - this will not initialize it		
		Allocate<false>(mCount + other.mValue.mCount);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			SAFETY(if (GetUses() > 1)
				Throw<Except::Reference>(
					"Moving elements that are used from multiple places"));

			CropInner(other.mValue.mCount, 0, mCount)
				.template CallUnknownMoveConstructors<false>(
					mCount, CropInner(0, mCount, mCount)
				);

			CropInner(0, 0, other.mValue.mCount)
				.template CallUnknownCopyConstructors<false>(other.mValue.mCount, other.mValue);
		}
		else {
			CropInner(mCount, 0, other.mValue.mCount)
				.template CallUnknownCopyConstructors<false>(other.mValue.mCount, other.mValue);
		}

		return other.mValue.mCount;
	}

	/// Copy-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	///	@attention assumes simple index is in container's limits					
	///	@param other - the block to insert												
	///	@param index - special/simple index to insert at							
	///	@return the number of inserted elements										
	template<CT::NotAbandonedOrDisowned T, class INDEX>
	Count Block::MergeBlockAt(const T& other, INDEX index) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(CT::SameAsOneOf<INDEX, Index, Offset>,
			"INDEX bust be an index type");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.GetCount(); ++i) {
			auto right = other.GetElementResolved(i);
			if (!FindRTTI(right))
				inserted += InsertBlockAt(right, index);
		}

		return inserted;
	}

	/// Move-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	/// The moved elements will be removed from the source container				
	///	@attention assumes simple index is in container's limits					
	///	@param other - the block to insert												
	///	@param index - special/simple index to insert at							
	///	@return the number of inserted elements										
	template<CT::NotAbandonedOrDisowned T, class INDEX>
	Count Block::MergeBlockAt(T&& other, INDEX index) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(CT::SameAsOneOf<INDEX, Index, Offset>,
			"INDEX bust be an index type");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.GetCount(); ++i) {
			auto right = other.GetElementResolved(i);
			if (!FindRTTI(right)) {
				inserted += InsertBlockAt(Abandon(right), index); //TODO abandon only if other has one use only!
				i -= other.RemoveIndex(i);
			}
		}

		return inserted;
	}

	/// Copy-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	///	@attention assumes simple index is in container's limits					
	///	@param other - the block to insert												
	///	@param index - special/simple index to insert at							
	///	@return the number of inserted elements										
	template<CT::Data T, class INDEX>
	Count Block::MergeBlockAt(Disowned<T>&& other, INDEX index) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(CT::SameAsOneOf<INDEX, Index, Offset>,
			"INDEX bust be an index type");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.GetCount(); ++i) {
			auto right = other.GetElementResolved(i);
			if (!FindRTTI(right))
				inserted += InsertBlockAt(Disown(right), index);
		}

		return inserted;
	}

	/// Move-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	///	@attention assumes simple index is in container's limits					
	///	@param other - the block to insert												
	///	@param index - special/simple index to insert at							
	///	@return the number of inserted elements										
	template<CT::Data T, class INDEX>
	Count Block::MergeBlockAt(Abandoned<T>&& other, INDEX index) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(CT::SameAsOneOf<INDEX, Index, Offset>,
			"INDEX bust be an index type");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.mValue.GetCount(); ++i) {
			auto right = other.mValue.GetElementResolved(i);
			if (!FindRTTI(right))
				inserted += InsertBlockAt(Abandon(right), index); //TODO abandon only if other has one use only!
		}

		other.mValue.Free();
		return inserted;
	}

	/// Copy-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	/// Insertions will be appended either at the front, or at the back			
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::NotAbandonedOrDisowned T>
	Count Block::MergeBlock(const T& other) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(INDEX == IndexFront || INDEX == IndexBack,
			"INDEX bust be either IndexFront or IndexBack");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.GetCount(); ++i) {
			auto right = other.GetElementResolved(i);
			if (!FindRTTI(right))
				inserted += InsertBlock<INDEX>(right);
		}

		return inserted;
	}

	/// Move-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	/// The moved elements will be removed from the source container				
	/// Insertions will be appended either at the front, or at the back			
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::NotAbandonedOrDisowned T>
	Count Block::MergeBlock(T&& other) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(INDEX == IndexFront || INDEX == IndexBack,
			"INDEX bust be either IndexFront or IndexBack");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.GetCount(); ++i) {
			auto right = other.GetElementResolved(i);
			if (!FindRTTI(right)) {
				inserted += InsertBlock<INDEX>(Abandon(right)); //TODO abandon only if other has one use only!
				i -= other.RemoveIndex(i);
			}
		}

		return inserted;
	}

	/// Copy-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	/// Insertions will be appended either at the front, or at the back			
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::Data T>
	Count Block::MergeBlock(Disowned<T>&& other) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(INDEX == IndexFront || INDEX == IndexBack,
			"INDEX bust be either IndexFront or IndexBack");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.GetCount(); ++i) {
			auto right = other.GetElementResolved(i);
			if (!FindRTTI(right))
				inserted += InsertBlock<INDEX>(Disown(right));
		}

		return inserted;
	}

	/// Move-insert each block element that is not found in this container		
	/// One by one, by using a slow and tedious RTTI copies and compares			
	/// Insertions will be appended either at the front, or at the back			
	///	@param other - the block to insert												
	///	@return the number of inserted elements										
	template<Index INDEX, CT::Data T>
	Count Block::MergeBlock(Abandoned<T>&& other) {
		static_assert(CT::Block<T>,
			"T must be a block type");
		static_assert(INDEX == IndexFront || INDEX == IndexBack,
			"INDEX bust be either IndexFront or IndexBack");
		//TODO do a pass first and allocate & move once instead of each time?
		Count inserted {};
		for (Count i = 0; i < other.mValue.GetCount(); ++i) {
			auto right = other.mValue.GetElementResolved(i);
			if (!FindRTTI(right))
				inserted += InsertBlock<INDEX>(Abandon(right)); //TODO abandon only if other has one use only!
		}

		other.mValue.Free();
		return inserted;
	}

} // namespace Langulus::Anyness

