///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Block.hpp"
#include "Any.hpp"
#include "TAny.hpp"

namespace Langulus::Anyness
{

	/// Check if a memory block can be concatenated to this one						
	///	@param other - the block to concatenate										
	///	@return true if able to concatenate												
	bool Block::IsConcatable(const Block& other) const noexcept {
		// Check if unmovable or constant											
		if (IsStatic() || IsConstant())
			return false;

		// Check if types are compatible												
		return CanFitState(other) && Is(other.mType);
	}

	/// Check if a type can be inserted														
	///	@param other - check if a given type is insertable to the block		
	///	@return true if able to insert													
	bool Block::IsInsertable(DMeta other) const noexcept {
		if (!other || IsStatic() || IsConstant() || IsDeep() != other->mIsDeep)
			return false;
		return CastsToMeta(other);
	}

	/// Shrink the block, depending on currently reserved	elements					
	///	@param elements - number of elements to shrink by (relative)			
	void Block::Shrink(Count elements) {
		Allocate<false>(mReserved - ::std::min(elements, mReserved));
	}

	/// Clone all elements inside a new memory block									
	/// If we have jurisdiction, the memory won't move									
	void Block::TakeAuthority() {
		if (mEntry) {
			// We already own this memory, don't touch anything				
			return;
		}

		// Clone everything and overwrite this block								
		// At the end it should have exactly one reference						
		Block clone;
		Clone(clone);
		Free();
		operator = (clone);
	}

	/// Get the memory block corresponding to a base (constant)						
	///	@param meta - the descriptor to scan for a base								
	///	@param base - the base to search for											
	///	@return the block for the base (static and immutable)						
	Block Block::GetBaseMemory(DMeta meta, const RTTI::Base& base) const {
		if (IsEmpty())
			return {};

		if (base.mBinaryCompatible) {
			return {
				DataState::ConstantMember, meta,
				GetCount() * base.mCount,
				Get<Byte*>(), mEntry
			};
		}

		return {
			DataState::ConstantMember, meta, 1,
			Get<Byte*>(0, base.mOffset), mEntry
		};
	}

	/// Get the memory block corresponding to a base									
	///	@param meta - the descriptor to scan for a base								
	///	@param base - the base to search for											
	///	@return the block for the base (static and immutable)						
	Block Block::GetBaseMemory(DMeta meta, const RTTI::Base& base) {
		if (IsEmpty())
			return {};

		if (base.mBinaryCompatible) {
			return {
				DataState::Member, meta,
				GetCount() * base.mCount, 
				Get<Byte*>(), mEntry
			};
		}

		return {
			DataState::Member, meta, 1,
			Get<Byte*>(0, base.mOffset), mEntry
		};
	}

	/// Get the constant memory block corresponding to a base						
	/// This performs only pointer arithmetic												
	///	@param base - the base to search for											
	///	@return the block for the base (static and immutable)						
	Block Block::GetBaseMemory(const RTTI::Base& base) const {
		return GetBaseMemory(base.mType, base);
	}

	/// Get the mutable memory block corresponding to a base							
	/// This performs only pointer arithmetic												
	///	@param base - the base to search for											
	///	@return the block for the base (static but mutable)						
	Block Block::GetBaseMemory(const RTTI::Base& base) {
		return GetBaseMemory(base.mType, base);
	}

	/// Hash data inside memory block														
	///	@return the hash																		
	Hash Block::GetHash() const {
		if (!mType || !mCount)
			return {};

		if (mCount == 1) {
			// Exactly one element means exactly one hash						
			// This also eliminates asymmetries when getting hash of block	
			// and of templated element equivalents								
			if (IsSparse())
				return GetElementResolved(0).GetHash();
			else if (mType->mHasher)
				return mType->mHasher(mRaw);
			else if (mType->mIsPOD)
				return HashBytes(mRaw, mType->mSize);
			else {
				Logger::Error("Unhashable type ", GetToken());
				Throw<Except::Access>("Unhashable type", LANGULUS_LOCATION());
			}
		}

		// Hashing multiple elements one by one, and then rehash all		
		// the combined hashes															
		if (IsSparse()) {
			TAny<Hash> h;
			h.Allocate<false>(mCount);
			for (Count i = 0; i < mCount; ++i)
				h << GetElementResolved(i).GetHash();
			return HashBytes<DefaultHashSeed, false>(h.GetRaw(), h.GetByteSize());
		}
		else if (mType->mHasher) {
			TAny<Hash> h;
			h.Allocate<false>(mCount);
			for (Count i = 0; i < mCount; ++i) {
				const auto element = GetElement(i);
				h << mType->mHasher(element.mRaw);
			}
			return HashBytes<DefaultHashSeed, false>(h.GetRaw(), h.GetByteSize());
		}
		else if (mType->mIsPOD) {
			// POD data is an exception - just batch-hash it					
			return HashBytes(mRaw, GetByteSize());
		}
		else {
			Logger::Error("Unhashable type ", GetToken());
			Throw<Except::Access>("Unhashable type", LANGULUS_LOCATION());
		}
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

	/// Check if contained data exactly matches a given type							
	///	@param type - the type to check for												
	///	@return if this block contains data of exactly 'type'						
	bool Block::Is(DMeta type) const noexcept {
		return mType == type || (mType && mType->Is(type));
	}

	/// Reinterpret contents of this Block as the type and state of another		
	/// You can interpret vec4 as float[4] for example, or any other such		
	/// reinterpretation, as long as data remains tightly packed					
	///	@param pattern - the type of data to try interpreting as					
	///	@return a block representing this block, interpreted as the pattern	
	Block Block::ReinterpretAs(const Block& pattern) const {
		RTTI::Base common {};
		if (!CompareTypes(pattern, common) || !common.mBinaryCompatible)
			return {};

		const auto baseBytes = (common.mType->mSize * common.mCount)
			/ pattern.GetStride();

		if (pattern.IsEmpty()) {
			return {
				pattern.mState + DataState::Static, pattern.mType,
				baseBytes,
				mRaw, mEntry
			};
		}
		else {
			return {
				pattern.mState + DataState::Static, pattern.mType,
				(baseBytes / pattern.mCount) * pattern.mCount,
				mRaw, mEntry
			};
		}
	}
	
	/// Get first element block (unsafe)													
	///	@return the first element's block												
	Block Block::GetElement() noexcept {
		return {
			(mState + DataState::Static) - DataState::Or,
			mType, 1, mRaw, mEntry
		};
	}

	/// Get first element block (const, unsafe)											
	///	@return the first element's block												
	const Block Block::GetElement() const noexcept {
		return {
			(mState + DataState::Static) - DataState::Or,
			mType, 1, mRaw, mEntry
		};
	}

	/// Get a specific element block (unsafe)												
	///	@param index - the element's index												
	///	@return the element's block														
	Block Block::GetElement(Offset index) noexcept {
		return {
			(mState + DataState::Static) - DataState::Or,
			mType, 1, At(index * GetStride()), mEntry
		};
	}

	/// Get a specific element block (const, unsafe)									
	///	@param index - the index element													
	///	@return the element's block														
	const Block Block::GetElement(Offset index) const noexcept {
		return {
			(mState + DataState::Static) - DataState::Or,
			mType, 1, At(index * GetStride()), mEntry
		};
	}

	/// Get next element by incrementing data pointer (for inner use)				
	void Block::Next() noexcept {
		mRaw += GetStride();
	}

	/// Get previous element by decrementing data pointer (for inner use)		
	void Block::Prev() noexcept {
		mRaw -= GetStride();
	}

	/// Get next element by incrementing data pointer (for inner use)				
	///	@return a new block with the incremented pointer							
	Block Block::Next() const noexcept {
		return {mState, mType, mCount, mRaw + GetStride(), mEntry};
	}

	/// Get previous element by decrementing data pointer (for inner use)		
	///	@return a new block with the decremented pointer							
	Block Block::Prev() const noexcept {
		return {mState, mType, mCount, mRaw - GetStride(), mEntry};
	}

	/// Get the resolved first element of this block									
	///	@attention assumes this block is valid and has exactly one element	
	///	@return the resolved first element												
	Block Block::GetResolved() noexcept {
		auto dense = GetDense();
		if (!dense.mRaw || !mType->mResolver)
			return dense;

		return mType->mResolver(dense.mRaw).GetDense();
	}

	const Block Block::GetResolved() const noexcept {
		return const_cast<Block*>(this)->GetResolved();
	}

	/// Get the dense first element of this block										
	///	@attention assumes this block is valid and has exactly one element	
	///	@return the dense first element													
	Block Block::GetDense() noexcept {
		auto copy = *this;
		if (IsSparse()) {
			copy.mEntry = mRawSparse->mEntry;
			copy.mRaw = mRawSparse->mPointer;
			copy.mState -= DataState::Sparse;
		}

		return copy;
	}

	const Block Block::GetDense() const noexcept {
		return const_cast<Block*>(this)->GetDense();
	}

	/// Get the dense block of an element inside the block							
	///	@attention the element might be empty if a sparse nullptr				
	///	@param index - index of the element inside the block						
	///	@return the dense memory block for the element								
	Block Block::GetElementDense(Offset index) {
		return GetElement(index).GetDense();
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
		return GetElement(index).GetResolved();
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

	/// Remove elements on the back															
	///	@param count - the new count														
	///	@return a reference to this block												
	Block& Block::Trim(const Count count) {
		if (count >= mCount)
			return *this;

		RemoveIndex(count, mCount - count);
		return *this;
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
			if (direction == IndexFront) {
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
			count += output.InsertBlock<IndexBack>(input);
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
			return output.InsertBlock<IndexBack>(*this);
		return GatherInner(*this, output, direction);
	}

	/// Gather items of specific phase from input container and fill output		
	///	@param type - type to search for													
	///	@param input - source container													
	///	@param output - [in/out] container that collects results					
	///	@param direction - the direction to search from								
	///	@param phase - phase filter														
	///	@return the number of gathered elements										
	Count GatherPolarInner(DMeta type, const Block& input, Block& output, const Index direction, Phase phase) {
		if (input.GetPhase() != phase) {
			if (input.GetPhase() == Phase::Now && input.IsDeep()) {
				// Phases don't match, but we can dig deeper if deep			
				// and neutral, since Phase::Now is permissive					
				auto localOutput = Any::FromMeta(type, input.GetUnconstrainedState());
				if (direction == IndexFront) {
					for (Count i = 0; i < input.GetCount(); ++i) {
						GatherPolarInner(type, input.As<Block>(i),
							localOutput, direction, phase);
					}
				}
				else {
					for (Count i = input.GetCount(); i > 0; --i) {
						GatherPolarInner(type, input.As<Block>(i - 1),
							localOutput, direction, phase);
					}
				}

				localOutput.SetPhase(Phase::Now);
				return output.SmartPush(Abandon(localOutput));
			}

			// Polarity mismatch															
			return 0;
		}

		// Input is flat and neutral/same											
		if (!type) {
			// Output is any, so no need to iterate								
			return output.SmartPush(Any {input});
		}

		// Iterate subpacks if any														
		auto localOutput = Any::FromMeta(type, input.GetState());
		GatherInner(input, localOutput, direction);
		localOutput.SetPhase(Phase::Now);
		return output.InsertBlock(localOutput);
	}

	/// Gather items from this container based on phase. Output type				
	/// matters - it decides what you'll gather. Preserves hierarchy only if	
	/// output is deep																			
	Count Block::Gather(Block& output, Phase phase, const Index direction) const {
		return GatherPolarInner(output.GetType(), *this, output, direction, phase);
	}

	/// Destroy all elements, but don't deallocate memory								
	void Block::Clear() {
		if (!mEntry) {
			// Data is either static or unallocated								
			// Don't call destructors, just clear it up							
			mRaw = nullptr;
			mCount = mReserved = 0;
			return;
		}

		if (mEntry->GetUses() == 1) {
			// Destroy all elements but don't deallocate the entry			
			CallUnknownDestructors();
			mCount = 0;
			return;
		}
		
		// If reached, then data is referenced from multiple places			
		// Don't call destructors, just clear it up and dereference			
		mEntry->Free();
		mRaw = nullptr;
		mEntry = nullptr;
		mCount = mReserved = 0;
	}

	/// Destroy all elements, deallocate block and reset state						
	void Block::Reset() {
		Free();
		ResetMemory();
		ResetState();
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
				if (subPack.IsEmpty())
					Reset();
				return;
			}

			Block temporary {subPack};
			subPack.ResetMemory();
			Free();
			*this = temporary;
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
