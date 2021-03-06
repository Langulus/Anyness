///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "../Any.hpp"

#define VERBOSE(a) //pcLogFuncVerbose << a

namespace Langulus::Anyness
{

	/// Get the memory block corresponding to a local member variable				
	/// Never references data																	
	///	@param member - the member to get												
	///	@return a static memory block														
	Block Block::GetMember(const RTTI::Member& member) {
		if (!IsAllocated())
			return Block {member.mType};

		return { 
			DataState::Member, member.mType, 
			member.mCount, member.Get(mRaw)
		};
	}

	/// Get the memory Block corresponding to a local member variable (const)	
	///	@param member - the member to get												
	///	@return a static constant memory block											
	Block Block::GetMember(const RTTI::Member& member) const {
		auto result = const_cast<Block*>(this)->GetMember(member);
		result.MakeConst();
		return result;
	}

	/// Select a member Block via trait or index (or both)							
	///	@param trait - the trait to get													
	///	@param index - the trait index to get											
	///	@return a static memory block (constant if block is constant)			
	Block Block::GetMember(TMeta trait, Count index) {
		// Scan members																	
		Count counter = 0;
		for (auto& member : mType->mMembers) {
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
		for (auto& base : mType->mBases) {
			auto found = GetBaseMemory(base.mType, base).GetMember(trait, index);
			if (!found.IsUntyped())
				return found;
		}

		return {};
	}

	/// Select a member Block via trait or index (or both) (const)					
	///	@param trait - the trait to get													
	///	@param index - the trait index to get											
	///	@return a static constant memory block											
	Block Block::GetMember(TMeta trait, Count index) const {
		auto result = const_cast<Block*>(this)->GetMember(trait, index);
		result.MakeConst();
		return result;
	}
	
	/// Select a member Block via type or index (or both)								
	///	@param data - the type to get														
	///	@param index - the member index to get											
	///	@return a static memory block (constant if block is constant)			
	Block Block::GetMember(DMeta data, Count index) {
		// Scan members																	
		Count counter = 0;
		for (auto& member : mType->mMembers) {
			if (data && !member.mType->CastsTo(data))
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
		for (auto& base : mType->mBases) {
			auto found = GetBaseMemory(base.mType, base).GetMember(data, index);
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
	Block Block::GetMember(DMeta data, Count index) const {
		auto result = const_cast<Block*>(this)->GetMember(data, index);
		result.MakeConst();
		return result;
	}

	/// Select a member via type or index (or both)										
	/// Never references data																	
	///	@param data - the type to get														
	///	@param index - the member index to get											
	///	@return a static memory block (constant if block is constant)			
	Block Block::GetMember(std::nullptr_t, Count index) {
		if (index < mType->mMembers.size()) {
			auto& member = mType->mMembers[index];
			auto found = GetMember(member);
			VERBOSE("Selected " << GetToken() << "::" << member.mName
				<< " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
					<< "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
			);

			return found;
		}

		// No such data found, so check in bases									
		//TODO fix indices shadowing later bases
		index -= mType->mMembers.size();
		for (auto& base : mType->mBases) {
			auto found = GetBaseMemory(base.mType, base).GetMember(nullptr, index);
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
	Block Block::GetMember(std::nullptr_t, Count index) const {
		auto result = const_cast<Block*>(this)->GetMember(nullptr, index);
		result.MakeConst();
		return result;
	}

	/// Find first matching element(s) position inside container					
	/// This is a slow and tedious RTTI search											
	///	@param item - block with a single item to search for						
	///	@param idx - index to start searching from									
	///	@return the index of the found item, or uiNone if not found				
	Index Block::FindRTTI(const Block& item, const Index& idx) const {
		if (item.IsEmpty())
			return IndexNone;

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
			starti = Constrain(idx).mIndex;
			istep = 1;
			if (starti + 1 >= mCount)
				return IndexNone;
		}

		// Compare all elements															
		for (Index i = starti; i < mCount && i >= 0; i += istep) {
			auto left = GetElementResolved(i.GetOffset());
			bool failure = false;
			for (Index j = 0; j < item.GetCount() && !failure && (i + istep * j) >= 0 && (i + istep * j) < mCount; ++j) {
				auto right = item.GetElementResolved(j.GetOffset());
				if (!left.Compare(right)) {
					failure = true;
					break;
				}
			}

			if (!failure)
				return i;
		}

		// If this is reached, then no match was found							
		return IndexNone;
	}

} // namespace Langulus::Anyness
