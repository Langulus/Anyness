#include "../inner/Block.hpp"

#define VERBOSE(a) //pcLogFuncVerbose << a
#define VERBOSE_TAB(a) //ScopedTab tab; pcLogFuncVerbose << a << tab

namespace Langulus::Anyness::Inner
{

	/// Compare the relevant states of two blocks										
	///	@param right - the memory block to compare against							
	///	@return true if the two memory blocks' revelant states are identical	
	bool Block::CompareStates(const Block& right) const noexcept {
		return GetUnconstrainedState() == right.GetUnconstrainedState();
	}

	/// Compare any data using RTTI															
	/// Nested for each memory subregion, including sparse links					
	///	@param right - the memory block to compare against							
	///	@return true if the two memory blocks are identical						
	bool Block::Compare(const Block& right, bool resolve) const {
		VERBOSE_TAB("Comparing "
			<< ccPush << ccWhite << mCount << " elements of " << GetToken()
			<< ccPop << " with "
			<< ccPush << ccWhite << right.mCount << " elements of " << right.GetToken());

		if (mCount != right.mCount) {
			// Cheap early return for differently sized blocks					
			VERBOSE(ccRed << "Data count is different: " 
				<< mCount << " != " << right.mCount);
			return false;
		}

		if (mType != right.mType && (IsUntyped() || right.IsUntyped())) {
			// Cheap early return for blocks of differing undefined type	
			VERBOSE(ccRed << "One of the containers is untyped: "
				<< GetToken() << " != " << right.GetToken());
			return false;
		}

		if (!CompareStates(right)) {
			// Quickly return if memory and relevant states are same			
			VERBOSE(ccRed << "Data states are not compatible");
			return false;
		}

		if (mType == right.mType && mRaw == right.mRaw) {
			// Quickly return if memory and relevant states are same			
			VERBOSE(ccGreen << "Blocks are the same "
				<< ccCyan << "(optimal)");
			return true;
		}

		if (!mType || !mType->InterpretsAs(right.mType)) {
			// Data is not similar at all, because either types or states	
			// are incompatible															
			VERBOSE(ccRed << "Data types are not compatible: "
				<< GetToken() << " != " << right.GetToken());
			return false;
		}

		if (mType->mStaticDescriptor.mComparer) {
			// Call the reflected == operator										
			VERBOSE("Comparing using reflected operator ==");
			for (Count i = 0; i < mCount; ++i) {
				auto lhs = resolve ? GetElementResolved(i) : GetElement(i);
				auto rhs = resolve ? right.GetElementResolved(i) : right.GetElement(i);
				if (lhs.GetDataID() != rhs.GetDataID()) {
					// Fail comparison on first mismatch							
					VERBOSE(ccRed << "Element " << i << " differs by type: "
						<< lhs.GetToken() << " != " << rhs.GetToken());
					return false;
				}

				if (lhs.mRaw == rhs.mRaw)
					continue;

				if (!lhs.mRaw || !rhs.mRaw || !mType->mStaticDescriptor.mComparer(lhs.mRaw, rhs.mRaw)) {
					// Fail comparison on first mismatch							
					VERBOSE(ccRed << "Element " << i << " differs: "
						<< lhs.GetToken() << " != " << rhs.GetToken());
					return false;
				}
			}

			VERBOSE(ccGreen << "Data is the same, all elements match "
				<< ccDarkYellow << "(slow)");
			return true;
		}

		if (mType->IsPOD() && right.mType->IsPOD() && mType->GetStride() == right.mType->GetStride()) {
			// Just compare the memory directly (optimization)					
			VERBOSE("Comparing POD memory");
			const auto code = memcmp(mRaw, right.mRaw, mCount * mType->GetStride());
			if (code == 0) 
				VERBOSE(ccGreen << "POD memory is the same (fast)");
			else
				VERBOSE(ccRed << "POD memory is not the same "
					<< ccRed << "(fast)");
			return code == 0;
		}

		// Compare all elements, one by one											
		Count compared_members = 0;
		for (Count i = 0; i < mCount; ++i) {
			auto lhs = resolve ? GetElementResolved(i) : GetElement(i);
			auto rhs = resolve ? right.GetElementResolved(i) : right.GetElement(i);
			if (!lhs.CompareMembers(rhs, compared_members)) {
				// Fail comparison on first mismatch								
				VERBOSE(ccRed << "Members in element " << i << " differ: "
					<< lhs.GetToken() << " != " << rhs.GetToken());
				return false;
			}
		}

		SAFETY(if (compared_members == 0 && mCount > 0)
			pcLogFuncError << "Comparing checked no members");

		VERBOSE(ccGreen << "Data is the same, all members match " 
			<< ccRed << "(slowest)");
		return true;
	}

	/// Compare the members of dense elements and their bases						
	///	@param right - block to compare against										
	///	@param compared - [in/out] the number of compared members				
	///	@return false if any member differs												
	bool Block::CompareMembers(const Block& right, Count& compared) const {
		VERBOSE_TAB("Comparing the members of " << GetToken());

		// Take care of memory blocks directly										
		if (Is<Block>() || Is<Any>()) {
			++compared;
			auto& lhs_block = Get<Block>();
			auto& rhs_block = right.Get<Block>();
			return lhs_block.Compare(rhs_block);
		}

		// First we check all bases													
		for (auto& base : mType->GetBaseList()) {
			auto baseMeta = base.mBase;
			if (baseMeta && baseMeta->GetStride() > 0) {
				++compared;
				auto lhs_base = GetBaseMemory(baseMeta, base);
				auto rhs_base = right.GetBaseMemory(baseMeta, base);
				if (!lhs_base.Compare(rhs_base, false))
					return false;
			}
		}

		// Iterate members for each element											
		for (auto& member : mType->GetMemberList()) {
			if (member.Is<Block>() || member.Is<Any>()) {
				// If member is a memory block, nest								
				++compared;
				auto& lhs_block = member.As<Block>(mRaw);
				auto& rhs_block = member.As<Block>(right.mRaw);
				if (!lhs_block.Compare(rhs_block))
					return false;
			}
			else {
				// Handle all the rest of the members								
				++compared;
				auto lhs_member = GetMember(member);
				auto rhs_member = right.GetMember(member);
				if (!lhs_member.Compare(rhs_member))
					return false;
			}
		}

		return true;
	}

} // namespace Langulus::Anyness::Inner
