#include "Block.hpp"
#include "../Any.hpp"

namespace Langulus::Anyness::Inner
{

	/// Move-construction for block															
	///	@param other - the block instance to move										
	Block::Block(Block&& other) noexcept
		: mRaw {other.mRaw}
		, mType {other.mType}
		, mCount {other.mCount}
		, mReserved {other.mReserved}
		, mState {other.mState} {
		other.ResetInner();
	}

	/// Get the token of the contained type												
	///	@return the token																		
	Token Block::GetToken() const noexcept {
		return IsUntyped() ? MetaData::DefaultToken : mType->mToken;
	}

	/// Check if a memory block can be concatenated to this one						
	///	@param other - the block to concatenate										
	///	@return true if able to concatenate												
	bool Block::IsConcatable(const Block& other) const noexcept {
		// Check if unmovable or constant											
		if (IsStatic() || IsConstant())
			return false;

		// Check if types are compatible												
		return CanFitState(other) && CanFit(other);
	}

	/// Check if a type can be inserted														
	///	@param other - check if a given type is insertable to the block		
	///	@return true if able to insert													
	bool Block::IsInsertable(DMeta other) const noexcept {
		if (IsStatic() || IsConstant() || IsDeep() != other->mIsDeep) {
			// If unmovable or constant - then unresizable						
			return false;
		}
		else if (!IsSparse() && !InterpretsAs(other)) {
			// If dense and not forward compatible - fail						
			return false;
		}
		else if (IsSparse() && !other->InterpretsAs(mType)) {
			// If sparse and not backwards compatible - fail					
			return false;
		}
		return true;
	}

	/// (Re)allocation of memory with optional default-construction				
	/// If elements are less than the count, the excess will be deconstructed	
	///	@param elements - number of elements to allocate							
	///	@param construct - set to true in order to construct memory & count	
	///	@param setcount - set the count even if not constructed					
	void Block::Allocate(Count elements, bool construct, bool setcount) {
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
			// Remove last few entries on smaller allocation					
			RemoveIndex(elements, mCount - elements);
			return;
		}

		if (mReserved >= elements) {
			// Required memory is already available								
			if (construct && mCount < elements) {
				// But is not yet initialized, so initialize it					
				CropInner(mCount, elements - mCount).CallDefaultConstructors();
				mCount = elements;
				return;
			}

			if (setcount)
				mCount = elements;
			return;
		}
		
		// Check if we're allocating abstract data								
		auto concrete = mType->mConcrete;
		if (concrete->mIsAbstract) {
			throw Except::Allocate(Logger::Error()
				<< "Allocating abstract data without any concretization: " << GetToken());
		}

		// Request the memory block from the memory manager					
		const auto stride = concrete->mSize;
		if (IsAllocated()) {
			if (IsStatic()) {
				throw Except::Allocate(Logger::Error()
					<< "Attempting to reallocate unmovable block");
			}

			mRaw = PCMEMORY.Reallocate(concrete, mRaw, stride * elements, stride * mReserved);
		}
		else mRaw = PCMEMORY.Allocate(concrete, stride * elements);

		if (!mRaw) {
			throw Except::Allocate(Logger::Error()
				<< "(Re)allocation returned nullptr");
		}
		
		mReserved = elements;
		
		// Construct elements (and set count) if requested						
		if (construct && mCount < elements) {
			CropInner(mCount, elements - mCount).CallDefaultConstructors();
			mCount = elements;
			return;
		}

		if (setcount)
			mCount = elements;
		return;
	}

	/// Extend the block, depending on currently reserved elements					
	///	@param elements - number of elements to append								
	///	@param construct - true to default-initialize new memory					
	///	@param setcount - true to force set count without initializing			
	void Block::Extend(Count elements, bool construct, bool setcount) {
		Allocate(mReserved + elements, construct, setcount);
	}

	/// Shrink the block, depending on currently reserved	elements					
	///	@param elements - number of elements to shrink by (relative)			
	void Block::Shrink(Count elements) {
		Allocate(mReserved - ::std::min(elements, mReserved));
	}

	/// Dereference memory block once														
	///	@return the remaining references for the block								
	RefCount Block::Free() {
		return ReferenceBlock(-1);
	}

	/// Reference memory block once															
	///	@return the remaining references for the block								
	RefCount Block::Keep() {
		return ReferenceBlock(1);
	}
		
	/// Select region from the memory block - unsafe and may return memory		
	/// that has not been initialized yet - use only at your own risk				
	/// Never references																			
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the block representing the region										
	Block Block::CropInner(const Count start, const Count count) {
		Block result {
			mState, mType, 
			std::min(start < mCount ? mCount - start : 0, count), 
			At(start * mType->mSize)
		};

		result.mReserved = std::min(count, mReserved - start);
		return result;
	}

	/// Select an initialized region from the memory block					 		
	/// Never references																			
	///	@param start - starting element index											
	///	@param count - number of elements to remain after 'start'				
	///	@return the block representing the region										
	Block Block::Crop(Offset start, Count count) {
		#if LANGULUS_SAFE()
			if (start > mCount) {
				start = mCount;
				throw Except::Access(Logger::Error()
					<< "Crop left offset is out of limits");
			}
			if (start + count > mCount) {
				count = mCount - start;
				throw Except::Access(Logger::Error()
					<< "Crop count is out of limits");
			}
		#endif

		if (count == 0)
			return {mState, mType};

		return {
			mState.mState | DataState::Member, 
			mType, count, At(start * mType->mSize)
		};
	}

	/// Select a constant region from the memory block 								
	/// Never references																			
	///	@param start - starting element index											
	///	@param count - number of elements												
	///	@return the block representing the region										
	Block Block::Crop(const Offset start, const Count count) const {
		auto result = const_cast<Block*>(this)->Crop(start, count);
		result.mState.mState |= DataState::Constant;
		return result;
	}

	/// Check if we have jurisdiction over the contained memory						
	///	@return true if memory is managed												
	bool Block::CheckJurisdiction() const {
		return PCMEMORY.CheckJurisdiction(mType, mRaw);
	}

	/// Check if we have jurisdiction over the contained memory						
	///	@return true if memory is managed												
	bool Block::CheckUsage() const {
		return PCMEMORY.CheckUsage(mType, mRaw);
	}

	/// Reallocates the same memory inside our memory manager						
	/// If we have jurisdiction, the memory won't move									
	///	@return a reference to this byte sequence										
	Block& Block::TakeAuthority() {
		if (mRaw && mType && mReserved) {
			auto newRaw = PCMEMORY.Reallocate(mType, mRaw, mReserved, mReserved);
			if (newRaw != mRaw) {
				// Memory moved, which means data is transfered to MMS,		
				// so it's about time to remove the static constraints		
				mState.mState &= ~DataState::Static;
			}
		}

		return *this;
	}

	/// Get the number of references for the allocated memory block				
	///	@attention always returns 1 if memory is outside authority				
	///	@return the references for the memory block									
	RefCount Block::GetBlockReferences() const {
		return PCMEMORY.GetReferences(mType, mRaw);
	}

	/// Get the memory block corresponding to a base (constant)						
	///	@param meta - the descriptor to scan for a base								
	///	@param base - the base to search for											
	///	@return the block for the base (static and immutable)						
	Block Block::GetBaseMemory(DMeta meta, const Base& base) const {
		if (base.mMapping) {
			return {
				DataState::ConstantMember, meta,
				GetCount() * base.mCount,
				Get<Byte*>()
			};
		}

		if (IsEmpty()) {
			return {DataState::Constant, meta};
		}

		return {
			DataState::ConstantMember, meta, 1,
			Get<Byte*>(0, base.mOffset)
		};
	}

	/// Get the memory block corresponding to a base									
	///	@param meta - the descriptor to scan for a base								
	///	@param base - the base to search for											
	///	@return the block for the base (static and immutable)						
	Block Block::GetBaseMemory(DMeta meta, const Base& base) {
		if (base.mMapping) {
			return {
				DataState::Member, meta,
				GetCount() * base.mCount, 
				Get<Byte*>()
			};
		}

		if (IsEmpty()) {
			return {meta};
		}

		return {
			DataState::Member, meta, 1,
			Get<Byte*>(0, base.mOffset)
		};
	}

	/// Get the memory block corresponding to a base									
	/// This performs only pointer arithmetic												
	///	@param base - the base to search for											
	///	@return the block for the base (static and immutable)						
	Block Block::GetBaseMemory(const Base& base) const {
		return GetBaseMemory(base.mType, base);
	}

	Block Block::GetBaseMemory(const Base& base) {
		return GetBaseMemory(base.mType, base);
	}

	/// Mutate to another compatible type, deepening the container if allowed	
	///	@param meta - the type to mutate into											
	///	@return true if block was deepened												
	bool Block::Mutate(DMeta meta) {
		if (IsUntyped()) {
			// Undefined containers can mutate freely								
			SetType(meta, false);
		}
		else if (mType->Is(meta)) {
			// No need to mutate - types are the same								
			return false;
		}
		else if (IsAbstract() && IsEmpty() && meta->InterpretsAs(mType)) {
			// Abstract compatible containers can be concretized				
			SetType(meta, false);
		}
		else if (!IsInsertable(meta)) {
			// Not insertable due to some reasons									
			if (!IsTypeConstrained()) {
				// Container is not type-constrained, so we can safely		
				// deepen it, to incorporate the new data							
				Deepen<Any>();
				return true;
			}
			else throw Except::Mutate(Logger::Error()
				<< "Attempting to deepen incompatible type-constrained container from "
				<< GetToken() << " to " << meta->mToken);
		}

		SAFETY(if (!InterpretsAs(meta)) {
			throw Except::Mutate(Logger::Error()
				<< "Mutation results in incompatible data " << meta->mToken
				<< " (container of type " << GetToken() << ")");
		})

		return false;
	}

	/// Toggle memory state																		
	///	@param state - the state to toggle												
	///	@param toggle - whether to enable the forementioned state or not		
	void Block::ToggleState(const DataState& state, bool toggle) {
		if (toggle)	mState.mState |= state.mState;
		else			mState.mState &= ~state.mState;
	}

	/// Make memory block vacuum (a.k.a. missing)										
	///	@return reference to itself														
	Block& Block::MakeMissing() {
		mState.mState |= DataState::Missing;
		return *this;
	}

	/// Make memory block static (unmovable and unresizable)							
	///	@return reference to itself														
	Block& Block::MakeStatic() {
		mState.mState |= DataState::Static;
		return *this;
	}

	/// Make memory block constant (unresizable and unchangable)					
	///	@return reference to itself														
	Block& Block::MakeConstant() {
		mState.mState |= DataState::Constant;
		return *this;
	}

	/// Make memory block type-immutable													
	///	@return reference to itself														
	Block& Block::MakeTypeConstrained() {
		mState.mState |= DataState::Typed;
		return *this;
	}

	/// Make memory block exlusive (a.k.a. OR container)								
	///	@return reference to itself														
	Block& Block::MakeOr() {
		mState.mState |= DataState::Or;
		return *this;
	}

	/// Make memory block inclusive (a.k.a. AND container)							
	///	@return reference to itself														
	Block& Block::MakeAnd() {
		mState.mState &= ~DataState::Or;
		return *this;
	}

	/// Set memory block phase to past														
	///	@return reference to itself														
	Block& Block::MakePast() {
		SetPhase(Phase::Past);
		return *this;
	}

	/// Set memory block phase to future													
	///	@return reference to itself														
	Block& Block::MakeFuture() {
		SetPhase(Phase::Future);
		return *this;
	}

	/// Hash data inside memory block														
	///	@returns the hash																		
	Hash Block::GetHash() const {
		if (!mType || !mCount)
			return {};

		if (!mType->mResolver && mType->mHasher) {
			// All elements share the same hasher - use it						
			// Then do a cumulative hash by mixing up all the hashes			
			Hash cumulativeHash {};
			for (Count i = 0; i < mCount; ++i) {
				auto element = GetElementDense(i);
				const auto h = mType->mHasher(element.mRaw);
				cumulativeHash = (cumulativeHash + (324723947 + h)) ^ 93485734985;
			}

			return cumulativeHash;
		}

		// Resolve each element and hash it											
		// Then do a cumulative hash by mixing up all the hashes				
		Hash cumulativeHash {};
		for (Count i = 0; i < mCount; ++i) {
			const auto h = GetElementResolved(i).GetHash();
			cumulativeHash = (cumulativeHash + (324723947 + h)) ^ 93485734985;
		}

		return cumulativeHash;
	}

	/// Set the data ID - use this only if you really know what you're doing	
	///	@param type - the type meta to set												
	///	@param constrain - whether or not to enable type-constraints			
	void Block::SetType(DMeta type, bool constrain) {
		if (mType == type) {
			if (constrain)
				MakeTypeConstrained();
			return;
		}
		else if (!mType) {
			mType = type;
			if (constrain)
				MakeTypeConstrained();
			return;
		}

		// At this point, the container has and initialized type				
		if (IsTypeConstrained()) {
			// You can't set type of an initialized typed block				
			throw Except::Mutate(Logger::Error()
				<< "Changing typed block is disallowed: from " 
				<< GetToken() << " to " << type->mToken);
		}

		if (mType->InterpretsAs(type)) {
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

		if (constrain)
			MakeTypeConstrained();
	}

	/// Get the number of sub-blocks (this one included)								
	///	@return at least 1																	
	Count Block::GetCountDeep() const noexcept {
		if (!IsDeep())
			return 1;

		Count counter {1};
		for (Count i = 0; i < mCount; ++i)
			counter += As<Block>(i).GetCountDeep();
		return counter;
	}

	/// Get the sum of elements in all sub-blocks										
	///	@returns the deep count of elements												
	Count Block::GetCountElementsDeep() const noexcept {
		if (!mType)
			return 0;
		if (!IsDeep())
			return mCount;

		Count counter {};
		for (Count i = 0; i < mCount; ++i)
			counter += As<Block>(i).GetCountElementsDeep();
		return counter;
	}

	/// Check if contained type is abstract												
	///	@returns true if the type of this pack is abstract							
	bool Block::IsAbstract() const noexcept {
		return mType && mType->mIsAbstract;
	}

	/// Check if contained type is constructible											
	/// Some are only referencable, such as abstract types							
	///	@returns true if the contents of this pack are constructible			
	bool Block::IsConstructible() const noexcept {
		return mType && mType->mDefaultConstructor;
	}

	/// Check if block contains pointers													
	///	@return true if the block contains pointers									
	bool Block::IsSparse() const {
		return mState.mState & DataState::Sparse;
	}

	/// Get the size of a single element (in bytes)										
	///	@attention this returns size of pointer if container is sparse			
	///	@attention this returns zero if block is untyped							
	///	@return the size is bytes															
	Stride Block::GetStride() const noexcept {
		return mState.mState & DataState::Sparse 
			? sizeof(void*) 
			: (mType ? mType->mSize : 0);
	}

	/// Check if you can push a type to this container									
	/// Beware, direction matters (this is the inverse of InterpretsAs)			
	///	@param type - the type to check if able to fit								
	///	@return true if able to interpret current type to 'type'					
	bool Block::CanFit(DMeta type) const {
		return !mType || !type || type->InterpretsAs(mType);
	}

	/// Check if two containers are concatenable											
	///	@param pack - the memory block to check if fittable to this				
	///	@return true if fittable															
	bool Block::CanFit(const Block& pack) const {
		return CanFit(pack.mType);
	}

	/// Check if contained data can be interpreted as a given type					
	/// Beware, direction matters (this is the inverse of CanFit)					
	///	@param type - the type check if current type interprets to				
	///	@return true if able to interpret current type to 'type'					
	bool Block::InterpretsAs(DMeta type) const {
		return !mType || !type || mType->InterpretsAs(type);
	}

	/// Check if contained data can be interpreted as a given coung of type		
	/// For example: a vec4 can interpret as float[4]									
	/// Beware, direction matters (this is the inverse of CanFit)					
	///	@param type - the type check if current type interprets to				
	///	@param count - the number of elements to interpret as						
	///	@return true if able to interpret current type to 'type'					
	bool Block::InterpretsAs(DMeta type, Count count) const {
		return !mType || !type || mType->InterpretsAs(type, count);
	}

	/// Check if contained data exactly matches a given type							
	///	@param type - the type to check for												
	///	@return if this block contains data of exactly 'type'						
	bool Block::Is(DMeta type) const noexcept {
		return mType == type || (mType && mType->Is(type));
	}

	/// Get a specific element block (unsafe)												
	///	@param index - the element's index												
	///	@return the element's block														
	Block Block::GetElement(Offset index) noexcept {
		return {
			(mState.mState | DataState::Static) & ~DataState::Or,
			mType, 1, At(index * mType->mSize)
		};
	}

	/// Get a specific element block (const, unsafe)									
	///	@param index - the index element													
	///	@return the element's block														
	const Block Block::GetElement(Offset index) const noexcept {
		return {
			(mState.mState | DataState::Static) & ~DataState::Or,
			mType, 1, At(index * mType->mSize)
		};
	}

	/// Get the dense block of an element inside the block							
	///	@attention the element might be empty if a sparse nullptr				
	///	@param index - index of the element inside the block						
	///	@return the dense memory block for the element								
	Block Block::GetElementDense(Offset index) {
		auto element = GetElement(index);
		if (IsSparse()) {
			element.mState.mState &= ~DataState::Sparse;
			element.mRaw = *element.GetRawSparse();
			if (!element.mRaw)
				return {};
		}

		return element;
	}

	/// Get the dense block of an element inside the block							
	///	@param index - index of the element inside the block						
	///	@return the dense memory block for the element								
	const Block Block::GetElementDense(Offset index) const {
		return const_cast<Block*>(this)->GetElementDense(index);
	}
	
	/// Get the dense and most concrete block of an element inside the block	
	///	@attention the element might be empty if resolved a sparse nullptr	
	///	@param index - index of the element inside the block						
	///	@return the dense resolved memory block for the element					
	Block Block::GetElementResolved(Offset index) {
		auto element = GetElementDense(index);
		if (!element.mRaw || !mType->mResolver)
			return element;

		return mType->mResolver(element.mRaw).GetElementDense(0);
	}

	/// Get the dense const block of an element inside the block					
	///	@param index - index of the element inside the block						
	///	@return the dense resolved memory block for the element					
	const Block Block::GetElementResolved(Count index) const {
		return const_cast<Block*>(this)->GetElementResolved(index);
	}
	
	/// Get a deep memory sub-block															
	///	@param index - the index to get, where 0 corresponds to this			
	///	@return a pointer to the block or nullptr if index is invalid			
	Block* Block::GetBlockDeep(Count index) noexcept {
		if (index == 0)
			return this;
		if (!IsDeep())
			return nullptr;

		--index;
		for (Count i = 0; i < mCount; i += 1) {
			auto ith = As<Block*>(i);
			const auto count = ith->GetCountDeep();
			if (index <= count) {
				auto subpack = ith->GetBlockDeep(index);
				if (subpack != nullptr)
					return subpack;
			}

			index -= count;
		}

		return nullptr;
	}

	/// Get a deep memory sub-block (const)												
	///	@param index - the index to get													
	///	@return a pointer to the block or nullptr if index is invalid			
	const Block* Block::GetBlockDeep(Count index) const noexcept {
		return const_cast<Block*>(this)->GetBlockDeep(index);
	}

	/// Get a deep element block																
	///	@param index - the index to get													
	///	@return the element block															
	Block Block::GetElementDeep(Count index) noexcept {
		if (!mType)
			return {};

		if (!IsDeep())
			return index < mCount ? GetElement(index) : Block();

		for (Count i = 0; i != mCount; i += 1) {
			auto ith = As<Block*>(i);
			const auto count = ith->GetCountElementsDeep();
			if (index < count) 
				return ith->GetElementDeep(index);

			index -= count;
		}

		return {};
	}

	/// Get a deep element block (const)													
	///	@param index - the index to get													
	///	@return the element block															
	const Block Block::GetElementDeep(Count index) const noexcept {
		return const_cast<Block*>(this)->GetElementDeep(index);
	}

	/// Call default constructors in a region and initialize memory				
	///	@attention this operates on uninitialized memory only, and any			
	///		misuse will result in loss of data and undefined behavior			
	///	@param newCount - the new count that will be saved after init			
	void Block::CallDefaultConstructors() {
		if (mType->mNullifiable) {
			// Just zero the memory (optimization)									
			pcFillMemory(mRaw, {}, mReserved * GetStride());
			return;
		}
		else if (!mType->mDefaultConstructor) {
			throw Except::Construct(Logger::Error()
				<< "Can't default-construct " << mReserved - mCount << " elements of "
				<< GetToken() << " because no default constructor was reflected");
		}

		// Construct every UNINITIALIZED element									
		for (Count i = 0; i < mReserved; ++i) {
			auto element = GetElement(i);
			mType->mDefaultConstructor(element.mRaw);
		}
	}

	/// Call copy constructors in a region and initialize memory					
	///	@attention this operates on uninitialized memory only, and any			
	///		misuse will result in loss of data and undefined behavior			
	///	@attention source must have a binary-compatible type						
	///	@attention this must have zero count, and mReserved						
	///	@param source - the elements to copy											
	void Block::CallCopyConstructors(const Block& source) {
		if ((IsSparse() && source.IsSparse()) || mType->mPOD) {
			// Just copy the POD/pointer memory (optimization)					
			pcCopyMemory(source.mRaw, mRaw, GetStride() * mReserved);

			if (IsSparse()) {
				// Since we're copying pointers, we have to reference the	
				// dense memory behind each one of them							
				auto pointers = GetRawSparse();
				Count c = 0;
				while (c < mReserved) {
					// Reference each pointer											
					PCMEMORY.Reference(mType, pointers[c], 1);
					++c;
				}
			}

			return;
		}

		// Construct element by element												
		if (IsSparse()) {
			// LHS is pointer, RHS must be dense									
			// Copy each pointer from RHS, and reference it						
			auto pointers = GetRawSparse();
			for (Count i = 0; i < mReserved; ++i) {
				const auto element = source.GetElement(i);
				pointers[i] = element.mRaw;
				PCMEMORY.Reference(mType, pointers[i], 1);
			}
		}
		else if (source.IsSparse()) {
			// RHS is pointer, LHS must be dense									
			// Copy each dense element from RHS										
			if (Is<Block>()) {
				// Block's copy constructors don't reference, so we must		
				// compensate for that here											
				auto pointers = source.GetRawSparse();
				for (Count i = 0; i < mReserved; ++i) {
					Block& block = Get<Block>(i);
					new (&block) Block {*reinterpret_cast<const Block*>(pointers[i])};
					block.Keep();
				}
			}
			else {
				// Call the reflected copy-constructor for each element		
				if (!mType->mCopyConstructor) {
					throw Except::Construct(Logger::Error()
						<< "Can't copy-construct " << source.mCount << " elements of "
						<< GetToken() << " because no copy constructor was reflected");
				}

				auto pointers = source.GetRawSparse();
				for (Count i = 0; i < mReserved; ++i) {
					auto element = GetElement(i);
					mType->mCopyConstructor(element.mRaw, pointers[i]);
				}
			}
		}
		else  {
			// Both RHS and LHS must be dense										
			if (Is<Block>()) {
				// Block's copy constructors don't reference, so we must		
				// compensate for that here											
				for (Count i = 0; i < mReserved; ++i) {
					Block& blockTo = Get<Block>(i);
					const Block& blockFrom = source.Get<Block>(i);
					new (&blockTo) Block {blockFrom};
					blockTo.Keep();
				}
			}
			else {
				// Call the reflected copy-constructor for each element		
				if (!mType->mCopyConstructor) {
					throw Except::Construct(Logger::Error()
						<< "Can't copy-construct " << source.mCount << " elements of "
						<< GetToken() << " because no copy constructor was reflected");
				}

				for (Count i = 0; i < mReserved; ++i) {
					auto lhs = GetElement(i);
					auto rhs = source.GetElement(i);
					mType->mCopyConstructor(lhs.mRaw, rhs.mRaw);
				}
			}
		}
	}

	/// Call move constructors in a region and initialize memory					
	///	@attention this operates on uninitialized memory only, and any			
	///		misuse will result in loss of data and undefined behavior			
	///	@attention source must have a binary-compatible type						
	///	@attention this must have zero count, and source.mCount reserved		
	///	@param source - the elements to move											
	void Block::CallMoveConstructors(Block&& source) {
		if (mType->mPOD || (IsSparse() && source.IsSparse())) {
			// Copy pointers, and then null them									
			const auto count = GetStride() * mReserved;
			pcMoveMemory(source.mRaw, mRaw, count);
		}
		else if (source.IsSparse()) {
			// RHS is pointer, LHS must be dense									
			// Copy each dense element from RHS										
			if (!mType->mMoveConstructor) {
				throw Except::Construct(Logger::Error()
					<< "Can't move-construct " << source.mCount << " elements of "
					<< GetToken() << " because no move constructor was reflected");
			}

			auto pointers = source.GetRawSparse();
			for (Count i = 0; i < mReserved; ++i) {
				auto element = GetElement(i);
				mType->mMoveConstructor(element.mRaw, pointers[i]);
			}
		}
		else if (IsSparse()) {
			// LHS is pointer, RHS must be dense									
			// Copy each element pointer from RHS and reference it			
			auto pointers = GetRawSparse();
			for (Count i = 0; i < mReserved; ++i)
				pointers[i] = source.GetElement(i).mRaw;

			// Can't actually move, you know, just reference rhs by count	
			PCMEMORY.Reference(source.mType, source.mRaw, static_cast<RefCount>(mReserved));
		}
		else {
			// Both RHS and LHS must be dense										
			if (!mType->mMoveConstructor) {
				throw Except::Construct(Logger::Error()
					<< "Can't move-construct " << source.mCount << " elements of "
					<< GetToken() << " because no move constructor was reflected");
			}

			for (Count i = 0; i < mReserved; ++i) {
				auto lhs = GetElement(i);
				auto rhs = source.GetElement(i);
				mType->mMoveConstructor(lhs.mRaw, rhs.mRaw);
			}
		}

		// Reset the block																
		source.ResetInner();
	}

	/// Call destructors in a region - after this call the memory is not			
	/// considered initialized, but mCount is still valid, so be careful			
	/// This function is intended for internal use										
	///	@attention this operates on initialized memory only, and any			
	///				  misuse will result in undefined behavior						
	void Block::CallDestructors() {
		if (IsSparse()) {
			// We dereference each pointer - destructors will be called		
			// only if data behind those pointers is fully dereferenced		
			for (Count i = 0; i < mCount; ++i) {
				auto element = GetElementResolved(i);
				element.ReferenceBlock(-1);
			}

			// Always null the pointers after destruction						
			// It is quite obscure, but this is where TPointers are reset	
			pcFillMemory(mRaw, {}, GetSize());
			return;
		}
		else if (mType->Is<Block>()) {
			// Special care for Blocks, because their destructors don't		
			// dereference - we must compensate										
			for (Count i = 0; i < mCount; ++i) {
				Block& block = Get<Block>(i);
				block.Free();
				block.ResetInner();
			}
		}
		else if (!mType->mPOD) {
			// Destroy every dense element, one by one, using the 			
			// reflected destructors													
			if (!mType->mDestructor) {
				throw Except::Destruct(Logger::Error()
					<< "Can't destroy " << GetToken()
					<< " because no destructor was reflected");
			}

			for (Count i = 0; i < mCount; ++i) {
				auto element = GetElement(i);
				mType->mDestructor(element.mRaw);
			}
		}

		#if LANGULUS_PARANOID()
			// Nullify upon destruction only if we're paranoid					
			pcFillMemory(mRaw, pcbyte(0), GetSize());
		#endif
	}

	/// Check if the memory block contains memory blocks								
	///	@return true if the memory block contains memory blocks					
	bool Block::IsDeep() const {
		return mType && mType->mIsDeep;
	}

	/// Deep (slower) check if there's anything missing inside nested blocks	
	///	@return true if the deep or flat memory block contains missing stuff	
	bool Block::IsMissingDeep() const {
		if (IsMissing())
			return true;

		bool result = false;
		ForEachDeep([&result](const Block& group) {
			result = group.IsMissing();
			return !result;
		});

		return result;
	}

	/// Remove sequential special indices													
	///	@param index - special index														
	///	@param count - number of items to remove										
	///	@return the number of removed elements											
	Count Block::RemoveIndex(const Index& index, const Count count) {
		if (index == Index::All) {
			const auto oldCount = mCount;
			Free();
			ResetInner();
			return oldCount;
		}

		// Constrain the index															
		const auto starter = Constrain(index);
		if (starter.IsSpecial())
			return 0;

		return RemoveIndex(starter.GetOffset(), count);
	}

	/// Remove sequential raw indices in a given range									
	///	@param starter - simple index to start removing from						
	///	@param count - number of elements to remove									
	///	@return the number of removed elements											
	Count Block::RemoveIndex(const Count starter, const Count count) {
		SAFETY(if (starter >= mCount)
			throw Except::Access(Logger::Error()
				<< "Index " << starter << " out of range " << mCount));
		SAFETY(if (count > mCount || starter + count > mCount)
			throw Except::Access(Logger::Error()
				<< "Index " << starter << " out of range " << mCount));
		SAFETY(if (GetBlockReferences() > 1)
			throw Except::Reference(Logger::Error()
				<< "Removing elements from a memory block, that is used from multiple places"));

		if (IsConstant() || IsStatic()) {
			if (mType->mPOD && starter + count >= mCount) {
				// If data is POD and elements are on the back, we can get	
				// around constantness and staticness, by simply				
				// truncating the count without any reprecussions				
				const auto removed = mCount - starter;
				mCount = starter;
				return removed;
			}
			else {
				if (IsConstant()) {
					Logger::Error() << "Attempting to RemoveIndex in a "
						"constant container";
				}
				if (IsStatic()) {
					Logger::Error() << "Attempting to RemoveIndex in a "
						"static container";
				}
				return 0;
			}
		}

		// First call the destructors on the correct region					
		const auto ender = std::min(starter + count, mCount);
		const auto removed = ender - starter;
		CropInner(starter, removed).CallDestructors();

		if (ender < mCount) {
			// Fill gap	if any by invoking move constructions					
			CropInner(starter, mCount - ender)
				.CallMoveConstructors(
					CropInner(ender, mCount - ender));
		}

		// Change count																	
		mCount -= removed;
		if (mCount == 0) {
			// It is safe to release the memory - it's no longer in use		
			PCMEMORY.Reference(mType, mRaw, -1);
			mRaw = nullptr;
			mReserved = 0;
			mState.mState &= ~(DataState::Static | DataState::Constant);
		}

		return removed;
	}

	/// Remove a raw deep index corresponding to a whole block inside				
	///	@param index - simple index to remove											
	///	@return 1 if removed																	
	Count Block::RemoveIndexDeep(Count index) {
		if (!IsDeep())
			return 0;

		--index;
		for (Count i = 0; i != mCount; i += 1) {
			if (index == 0)
				return RemoveIndex(i);

			auto ith = As<Block*>(i);
			const auto count = ith->GetCountDeep();
			if (index <= count && ith->RemoveIndexDeep(index))
				return 1;

			index -= count;
		}

		return 0;
	}

	/// Remove elements on the back															
	///	@param count - the new count														
	///	@return a reference to this block												
	Block& Block::Trim(const Count count) {
		if (count >= mCount)
			return *this;

		RemoveIndex(count, mCount - count);
		return *this;
	}

	/// A helper function, that allocates and moves inner memory					
	/// @param other - the memory we'll be inserting									
	/// @param idx - the place we'll be inserting at									
	/// @param region - the newly allocated region (no mCount, only mReserved)	
	/// @return number if inserted items in case of mutation							
	Count Block::AllocateRegion(const Block& other, const Index& idx, Block& region) {
		if (other.IsEmpty())
			return 0;

		// Type may mutate																
		if (Mutate(other.mType)) {
			// Block was deepened, so emplace a container inside				
			return Emplace<Any>(Any {other}, idx);
		}

		// Allocate the required memory - this will not initialize it		
		const auto starter = Constrain(idx).GetOffset();
		Allocate(mCount + other.mCount);

		// Move memory if required														
		if (starter < mCount) {
			SAFETY(if (GetBlockReferences() > 1)
				throw Except::Reference(Logger::Error()
					<< "Moving elements that are used from multiple places"));

			CropInner(starter + other.mCount, mCount - starter)
				.CallMoveConstructors(
					CropInner(starter, mCount - starter));
		}

		// Pick the region that should be overwritten with new stuff		
		region = CropInner(starter, other.mCount);
		return 0;
	}

	/// Insert a block using a shallow copy for each element							
	///	@param other - the block to insert												
	///	@param idx - place to insert them at											
	///	@return the number of inserted elements										
	Count Block::InsertBlock(const Block& other, const Index& idx) {
		Block region;
		if (AllocateRegion(other, idx, region))
			return 1;

		if (region.IsAllocated()) {
			// Call copy-constructors in the new region							
			region.CallCopyConstructors(other);
			mCount += region.mReserved;
			return region.mReserved;
		}

		return 0;
	}

	/// Insert a block using a move-copy for each element								
	///	@param other - the block to move													
	///	@param idx - place to insert them at											
	///	@return the number of moved elements											
	Count Block::InsertBlock(Block&& other, const Index& idx) {
		Block region;
		if (AllocateRegion(other, idx, region))
			return 1;

		if (region.IsAllocated()) {
			// Call move-constructors in the new region							
			region.CallMoveConstructors(Forward<Block>(other));
			mCount += region.mReserved;
			return region.mReserved;
		}

		return 0;
	}

	/// Merge a block using a slow and tedious RTTI copies and compares			
	///	@param other - the block to insert												
	///	@param idx - place to insert them at											
	///	@return the number of inserted elements										
	Count Block::MergeBlock(const Block& other, const Index& idx) {
		Count inserted {};
		for (Count i = 0; i < other.GetCount(); ++i) {
			auto right = other.GetElementResolved(i);
			if (!FindRTTI(right))
				inserted += InsertBlock(right, idx);
		}

		return inserted;
	}

	/// Gather items from input container, and fill output							
	/// Output type acts as a filter to what gets gathered							
	///	@param input - source container													
	///	@param output - [in/out] container that collects results					
	///	@param direction - the direction to search from								
	///	@return the number of gathered elements										
	Count GatherInner(const Block& input, Block& output, const Index direction) {
		Count count {};
		if (input.IsDeep() && !output.IsDeep()) {
			// Iterate all subpacks														
			if (direction == Index::Front) {
				for (Count i = 0; i < input.GetCount(); ++i) {
					count += GatherInner(input.As<Block>(i), output, direction);
				}
			}
			else {
				for (Count i = input.GetCount(); i > 0; --i) {
					count += GatherInner(input.As<Block>(i - 1), output, direction);
				}
			}

			return count;
		}

		if (output.IsConcatable(input)) {
			// Catenate input if compatible											
			count += output.InsertBlock(input);
		}

		return count;
	}

	/// Gather items from this container, and fill output								
	/// Output type acts as a filter to what gets gathered							
	///	@param output - [in/out] container that collects results					
	///	@param direction - the direction to search from								
	///	@return the number of gathered elements										
	Count Block::Gather(Block& output, const Index direction) const {
		if (output.IsUntyped())
			return output.InsertBlock(*this);
		return GatherInner(*this, output, direction);
	}

	/// Gather items of specific polarity from input container,						
	/// and fill output - output type acts as a filter to what gets gathered	
	///	@param target_type - original type to search for							
	///	@param input - source container													
	///	@param output - [in/out] container that collects results					
	///	@param direction - the direction to search from								
	///	@param polarity - polarity filter												
	///	@return the number of gathered elements										
	Count GatherPolarInner(DMeta target_type, const Block& input, Block& output, const Index direction, Phase phase) {
		if (input.GetPhase() != phase) {
			if (input.GetPhase() == Phase::Now && input.IsDeep()) {
				// Polarities don't match, but we can dig deeper if deep		
				// and neutral, since neutral polarity is permissive			
				auto localOutput = Any::From(target_type, input.GetUnconstrainedState());
				if (direction == Index::Front) {
					for (Count i = 0; i < input.GetCount(); ++i) {
						GatherPolarInner(target_type, input.As<Block>(i), 
							localOutput, direction, phase);
					}
				}
				else {
					for (Count i = input.GetCount(); i > 0; --i) {
						GatherPolarInner(target_type, input.As<Block>(i - 1), 
							localOutput, direction, phase);
					}
				}

				localOutput.SetPhase(Phase::Now);
				return output.SmartPush(localOutput);
			}

			// Polarity mismatch															
			return 0;
		}

		// Input is flat and neutral/same											
		if (!target_type) {
			// Output is any, so no need to iterate								
			return output.SmartPush(Any {input});
		}

		// Iterate subpacks if any														
		auto localOutput = Any::From(target_type, input.GetState());
		GatherInner(input, localOutput, direction);
		localOutput.SetPhase(Phase::Now);
		return output.InsertBlock(localOutput);
	}

	/// Gather items from this container based on polarity. Output type			
	/// matters - it decides what you'll gather. Preserves hierarchy only if	
	/// output is deep																			
	Count Block::Gather(Block& output, Phase phase, const Index direction) const {
		return GatherPolarInner(output.GetType(), *this, output, direction, phase);
	}

	/// Flattens unnecessarily deep containers and combines their states			
	/// when possible. Discards ORness if container has only one element			
	void Block::Optimize() {
		if (IsOr() && GetCount() == 1)
			MakeAnd();

		while (GetCount() == 1 && IsDeep()) {
			auto& subPack = As<Block>();
			if (!CanFitState(subPack)) {
				subPack.Optimize();
				if (subPack.IsEmpty()) {
					Free();
					ResetInner();
				}

				return;
			}

			Block temporary {Move(subPack)};
			Free();
			*this = Move(temporary);
		}

		if (GetCount() > 1 && IsDeep()) {
			for (Count i = 0; i < mCount; ++i) {
				auto& subBlock = As<Block>(i);
				subBlock.Optimize();
				if (subBlock.IsEmpty()) {
					RemoveIndex(i);
					--i;
				}
			}
		}
	}

	/// Iterate each element block															
	///	@param call - the call to execute for each element block					
	///	@return the number of iterations done											
	Count Block::ForEachElement(TFunctor<bool(const Block&)>&& call) const {
		Count index {};
		while (index < mCount) {
			auto block = GetElement(index);
			if (!call(block))
				return index + 1;
			++index;
		}

		return index;
	}

	/// Iterate each element block															
	///	@param call - the call to execute for each element block					
	///	@return the number of iterations done											
	Count Block::ForEachElement(TFunctor<void(const Block&)>&& call) const {
		Count index {};
		while (index < mCount) {
			auto block = GetElement(index);
			call(block);
			++index;
		}

		return index;
	}

	/// Iterate each element block															
	///	@param call - the call to execute for each element block					
	///	@return the number of iterations done											
	Count Block::ForEachElement(TFunctor<bool(Block&)>&& call) {
		Count index {};
		while (index < GetCount()) {
			auto block = GetElement(index);
			if (!call(block))
				return index + 1;
			++index;
		}

		return index;
	}

	/// Iterate each element block															
	///	@param call - the call to execute for each element block					
	///	@return the number of iterations done											
	Count Block::ForEachElement(TFunctor<void(Block&)>&& call) {
		Count index {};
		while (index < GetCount()) {
			auto block = GetElement(index);
			call(block);
			++index;
		}

		return index;
	}

} // namespace Langulus::Anyness
