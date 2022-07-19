///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Block.hpp"
#include "Any.hpp"

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
		return CanFitState(other) && CanFit(other);
	}

	/// Check if a type can be inserted														
	///	@param other - check if a given type is insertable to the block		
	///	@return true if able to insert													
	bool Block::IsInsertable(DMeta other) const noexcept {
		if (IsStatic() || IsConstant() || IsDeep() != other->mIsDeep)
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
		if (mEntry)
			// We already own this memory, don't touch anything				
			return;

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
		if (base.mBinaryCompatible) {
			return {
				DataState::ConstantMember, meta,
				GetCount() * base.mCount,
				Get<Byte*>()
			};
		}

		if (IsEmpty())
			return {DataState::Constant, meta};

		return {
			DataState::ConstantMember, meta, 1,
			Get<Byte*>(0, base.mOffset)
		};
	}

	/// Get the memory block corresponding to a base									
	///	@param meta - the descriptor to scan for a base								
	///	@param base - the base to search for											
	///	@return the block for the base (static and immutable)						
	Block Block::GetBaseMemory(DMeta meta, const RTTI::Base& base) {
		if (base.mBinaryCompatible) {
			return {
				DataState::Member, meta,
				GetCount() * base.mCount, 
				Get<Byte*>()
			};
		}

		if (IsEmpty())
			return Block {meta};

		return {
			DataState::Member, meta, 1,
			Get<Byte*>(0, base.mOffset)
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

		if (IsDense() && mType->mIsPOD) {
			// Hash everything at once													
			return HashBytes(GetRaw(), GetByteSize());
		}

		// If reached, we'll be hashing elements one by one, and then		
		// rehash all the combined hashes											
		::std::vector<Hash> h {mCount};

		if (IsDense() && mType->mHasher) {
			// All elements share the same hasher - use it						
			// Then do a cumulative hash by mixing up all the hashes			
			for (Count i = 0; i < mCount; ++i) {
				auto element = GetElementDense(i);
				h.emplace_back(mType->mHasher(element.mRaw));
			}
		}
		else if (mType->mResolver) {
			// Resolve each element and hash it										
			// Then do a cumulative hash by mixing up all the hashes			
			for (Count i = 0; i < mCount; ++i) {
				h.emplace_back(GetElementResolved(i).GetHash());
			}
		}
		else Throw<Except::Access>("Unhashable type");

		return HashBytes<DefaultHashSeed, false>(h.data(), h.size() * sizeof(Hash));
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

	/// Check if you can push a type to this container									
	/// Beware, direction matters (this is the inverse of InterpretsAs)			
	///	@param type - the type to check if able to fit								
	///	@return true if able to interpret current type to 'type'					
	bool Block::CanFit(DMeta type) const {
		if (IsSparse())
			return type && type->CastsTo<true>(mType);
		else
			return type && type->CastsTo(mType);
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
	bool Block::CastsToMeta(DMeta type) const {
		if (IsSparse())
			return mType && mType->CastsTo<true>(type);
		else
			return mType && mType->CastsTo(type);
	}

	/// Check if contained data can be interpreted as a given coung of type		
	/// For example: a vec4 can interpret as float[4]									
	/// Beware, direction matters (this is the inverse of CanFit)					
	///	@param type - the type check if current type interprets to				
	///	@param count - the number of elements to interpret as						
	///	@return true if able to interpret current type to 'type'					
	bool Block::CastsToMeta(DMeta type, Count count) const {
		return !mType || !type || mType->CastsTo(type, count);
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
	Block Block::ReinterpretAs(const Block&) const {
		return {}; //TODO
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

	/// Get the dense block of an element inside the block							
	///	@attention the element might be empty if a sparse nullptr				
	///	@param index - index of the element inside the block						
	///	@return the dense memory block for the element								
	Block Block::GetElementDense(Offset index) {
		auto element = GetElement(index);
		if (IsSparse()) {
			element.mRaw = element.GetRawSparse()->mPointer;
			element.mEntry = element.GetRawSparse()->mEntry;
			element.mState -= DataState::Sparse;
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

	/// A slow runtime state reset, that retains sparseness and constraints		
	void Block::ResetStateRTTI() noexcept {
		if (IsTypeConstrained()) {
			if (IsSparse())
				ResetState<true, true>();
			else
				ResetState<true, false>();
		}
		else {
			if (IsSparse())
				ResetState<false, true>();
			else
				ResetState<false, false>();
		}
	}

	/// Remove sequential special indices													
	///	@param index - special index														
	///	@param count - number of items to remove										
	///	@return the number of removed elements											
	Count Block::RemoveIndex(const Index index, const Count count) {
		if (index == Index::All) {
			const auto oldCount = mCount;
			Free();
			ResetMemory();
			ResetStateRTTI();
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
		SAFETY(if (starter >= mCount || count > mCount || starter + count > mCount)
			Throw<Except::Access>("Index out of range"));
		SAFETY(if (GetUses() > 1)
			Throw<Except::Reference>("Attempting to remove elements from a used memory block"));

		if (IsConstant() || IsStatic()) {
			if (mType->mIsPOD && starter + count >= mCount) {
				// If data is POD and elements are on the back, we can get	
				// around constantness and staticness, by simply				
				// truncating the count without any reprecussions				
				const auto removed = mCount - starter;
				mCount = starter;
				return removed;
			}
			else {
				if (IsConstant())
					Throw<Except::Access>(
						"Attempting to RemoveIndex in a constant container");

				if (IsStatic())
					Throw<Except::Access>(
						"Attempting to RemoveIndex in a static container");

				return 0;
			}
		}

		// First call the destructors on the correct region					
		const auto ender = std::min(starter + count, mCount);
		const auto removed = ender - starter;
		CropInner(starter, removed, removed).CallUnknownDestructors();

		if (ender < mCount) {
			// Fill gap	if any by invoking move constructions					
			CropInner(starter, 0, mCount - ender)
				.template CallUnknownMoveConstructors<false>(
					mCount - ender,
					CropInner(ender, mCount - ender, mCount - ender)
				);
		}

		// Change count																	
		mCount -= removed;
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
			count += output.InsertBlock<Index::Back>(input);
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
			return output.InsertBlock<Index::Back>(*this);
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
				if (direction == Index::Front) {
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
				return output.SmartPush(localOutput);
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
		ResetStateRTTI();
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
