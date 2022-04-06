#include "inner/Block.hpp"

#define VERBOSE(a) //pcLogFuncVerbose << a

namespace Langulus::Anyness::Inner
{

	/// Get the memory block corresponding to a local member variable				
	/// Never references data																	
	///	@param member - the member to get												
	///	@return a static memory block														
	Block Block::GetMember(const LinkedMember& member) {
		if (!IsAllocated())
			return Block(DataState::Default, member.mType);

		return { 
			DataState::Static + DataState::Typed, member.mType, 
			member.mStaticMember.mCount, const_cast<void*>(member.Get(mRaw))
		};
	}

	/// Get the memory block corresponding to a local member variable (const)	
	/// Never references data																	
	///	@param member - the member to get												
	///	@return a static constant memory block											
	const Block Block::GetMember(const LinkedMember& member) const {
		auto result = const_cast<Block*>(this)->GetMember(member);
		result.mState += DataState::Constant;
		return result;
	}

	/// Select a member via trait or index (or both)									
	/// Never references data																	
	///	@param trait - the trait to get													
	///	@param index - the trait index to get											
	///	@return a static memory block (constant if block is constant)			
	Block Block::GetMember(TMeta trait, Count index) {
		// Scan members																	
		Count counter = 0;
		for (auto& member : mType->GetMemberList()) {
			if (trait && member.mTrait != trait)
				continue;

			// Matched, but check index first										
			if (counter < index) {
				++counter;
				continue;
			}

			// Found one																	
			auto found = GetMember(member);
			VERBOSE("Selected " << GetToken() << "::" << member.mName
				<< " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
					<< "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
			);

			return found;
		}

		// No such trait found, so check in bases									
		//TODO fix indices shadowing later bases
		index -= counter;
		for (auto& base : mType->GetBaseList()) {
			auto found = GetBaseMemory(base.mBase, base).GetMember(trait, index);
			if (!found.IsUntyped())
				return found;
		}

		return {};
	}

	/// Select a member via trait or index (or both) (const)							
	/// Never references data																	
	///	@param trait - the trait to get													
	///	@param index - the trait index to get											
	///	@return a static constant memory block											
	const Block Block::GetMember(TMeta trait, Count index) const {
		auto result = const_cast<Block*>(this)->GetMember(trait, index);
		result.mState += DataState::Constant;
		return result;
	}
	
	/// Select a member via type or index (or both)										
	/// Never references data																	
	///	@param data - the type to get														
	///	@param index - the member index to get											
	///	@return a static memory block (constant if block is constant)			
	Block Block::GetMember(DMeta data, Count index) {
		// Scan members																	
		Count counter = 0;
		for (auto& member : mType->GetMemberList()) {
			if (data && !member.mType->InterpretsAs(data))
			//if (data && !data->InterpretsAs(member.mType))
				continue;

			// Matched, but check index first										
			if (counter < index) {
				++counter;
				continue;
			}

			// Found one																	
			auto found = GetMember(member);
			VERBOSE("Selected " << GetToken() << "::" << member.mName
				<< " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
				<< "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
			);

			return found;
		}

		// No such data found, so check in bases									
		//TODO fix indices shadowing later bases
		index -= counter;
		for (auto& base : mType->GetBaseList()) {
			auto found = GetBaseMemory(base.mBase, base).GetMember(data, index);
			if (!found.IsUntyped())
				return found;
		}

		return {};
	}

	/// Select a member via data type or index (or both) (const)					
	/// Never references data																	
	///	@param data - the type to get														
	///	@param index - the trait index to get											
	///	@return a static constant memory block											
	const Block Block::GetMember(DMeta data, Count index) const {
		auto result = const_cast<Block*>(this)->GetMember(data, index);
		result.mState += DataState::Constant;
		return result;
	}

	/// Select a member via type or index (or both)										
	/// Never references data																	
	///	@param data - the type to get														
	///	@param index - the member index to get											
	///	@return a static memory block (constant if block is constant)			
	Block Block::GetMember(std::nullptr_t, Count index) {
		if (index < mType->GetMemberList().GetCount()) {
			auto& member = mType->GetMemberList()[index];
			auto found = GetMember(member);
			VERBOSE("Selected " << GetToken() << "::" << member.mName
				<< " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
					<< "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
			);

			return found;
		}

		// No such data found, so check in bases									
		//TODO fix indices shadowing later bases
		index -= mType->GetMemberList().GetCount();
		for (auto& base : mType->GetBaseList()) {
			auto found = GetBaseMemory(base.mBase, base).GetMember(nullptr, index);
			if (!found.IsUntyped())
				return found;
		}

		return {};
	}

	/// Select a member via data type or index (or both) (const)					
	/// Never references data																	
	///	@param data - the type to get														
	///	@param index - the trait index to get											
	///	@return a static constant memory block											
	const Block Block::GetMember(std::nullptr_t, Count index) const {
		auto result = const_cast<Block*>(this)->GetMember(nullptr, index);
		result.mState += DataState::Constant;
		return result;
	}

	/// Find first matching element(s) position inside container					
	/// This is a slow and tedious RTTI search											
	///	@param item - block with a single item to search for						
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	Index Block::FindRTTI(const Block& item, const Index& idx) const {
		if (item.IsEmpty())
			return uiNone;

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

		// Compare all elements															
		for (pcidx i = starti; i < pcount && i >= 0; i += istep) {
			auto left = GetElementResolved(i);
			bool failure = false;
			for (pcidx j = 0; j < pcidx(item.GetCount()) && !failure && (i + istep * j) >= 0 && (i + istep * j) < pcount; ++j) {
				auto right = item.GetElementResolved(j);
				if (!left.Compare(right)) {
					failure = true;
					break;
				}
			}

			if (!failure)
				return i;
		}

		// If this is reached, then no match was found							
		return uiNone;
	}

} // namespace Langulus::Anyness::Inner
