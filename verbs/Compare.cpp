///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "../Any.hpp"

#define VERBOSE(...) //Logger::Verbose(__VA_ARGS__)
#define VERBOSE_TAB(...) //auto tab = Logger::Section(__VA_ARGS__)

namespace Langulus::Anyness
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
		VERBOSE_TAB("Comparing ", 
			Logger::Push, Logger::White, mCount, " elements of ", GetToken(), 
			Logger::Pop, " with ", 
			Logger::Push, Logger::White, right.mCount, " elements of ", right.GetToken()
		);

		if (mCount != right.mCount) {
			// Cheap early return for differently sized blocks					
			VERBOSE(Logger::Red, 
				"Data count is different: ", mCount, " != ", right.mCount);
			return false;
		}

		if (mType != right.mType && (IsUntyped() || right.IsUntyped())) {
			// Cheap early return for blocks of differing undefined type	
			VERBOSE(Logger::Red, 
				"One of the containers is untyped: ", GetToken(), " != ", right.GetToken());
			return false;
		}

		if (!CompareStates(right)) {
			// Quickly return if memory and relevant states are same			
			VERBOSE(Logger::Red, 
				"Data states are not compatible");
			return false;
		}

		if (mType == right.mType && mRaw == right.mRaw) {
			// Quickly return if memory and relevant states are same			
			VERBOSE(Logger::Green, 
				"Blocks are the same ", Logger::Cyan, "(optimal)");
			return true;
		}

		if (!CastsToMeta(right.mType)) {
			// Data is not similar at all, because either types or states	
			// are incompatible															
			VERBOSE(Logger::Red, 
				"Data types are not compatible: ", GetToken(), " != ", right.GetToken());
			return false;
		}

		if (mType->mComparer) {
			// Call the reflected == operator										
			VERBOSE("Comparing using reflected operator ==");
			for (Count i = 0; i < mCount; ++i) {
				auto lhs = resolve ? GetElementResolved(i) : GetElement(i);
				auto rhs = resolve ? right.GetElementResolved(i) : right.GetElement(i);
				if (lhs.GetType() != rhs.GetType()) {
					// Fail comparison on first mismatch							
					VERBOSE(Logger::Red, 
						"Element ", i, " differs by type: ", lhs.GetToken(), " != ", rhs.GetToken());
					return false;
				}

				if (lhs.mRaw == rhs.mRaw)
					continue;

				if (!lhs.mRaw || !rhs.mRaw || !mType->mComparer(lhs.mRaw, rhs.mRaw)) {
					// Fail comparison on first mismatch							
					VERBOSE(Logger::Red, 
						"Element ", i, " differs: ", lhs.GetToken(), " != ", rhs.GetToken());
					return false;
				}
			}

			VERBOSE(Logger::Green, 
				"Data is the same, all elements match ", Logger::DarkYellow, "(slow)");
			return true;
		}

		if (mType->mIsPOD && right.mType->mIsPOD && mType->mSize == right.mType->mSize) {
			// Just compare the memory directly (optimization)					
			VERBOSE("Comparing POD memory");
			const auto code = memcmp(mRaw, right.mRaw, mCount * mType->mSize);
			if (code == 0) 
				VERBOSE(Logger::Green, "POD memory is the same (fast)");
			else
				VERBOSE(Logger::Red, "POD memory is not the same ", Logger::Red, "(fast)");
			return code == 0;
		}

		// Compare all elements, one by one											
		Count compared_members = 0;
		for (Count i = 0; i < mCount; ++i) {
			auto lhs = resolve ? GetElementResolved(i) : GetElement(i);
			auto rhs = resolve ? right.GetElementResolved(i) : right.GetElement(i);
			if (!lhs.CompareMembers(rhs, compared_members)) {
				// Fail comparison on first mismatch								
				VERBOSE(Logger::Red, 
					"Members in element ", i, " differ: ", lhs.GetToken(), " != ", rhs.GetToken());
				return false;
			}
		}

		SAFETY(if (compared_members == 0 && mCount > 0)
			Logger::Error("Comparing checked no members"));

		VERBOSE(Logger::Green, 
			"Data is the same, all members match ", Logger::Red, "(slowest)");
		return true;
	}

	/// Compare the members of dense elements and their bases						
	///	@param right - block to compare against										
	///	@param compared - [in/out] the number of compared members				
	///	@return false if any member differs												
	bool Block::CompareMembers(const Block& right, Count& compared) const {
		VERBOSE_TAB("Comparing the members of ", GetToken());

		// Take care of memory blocks directly										
		if (Is<Block>() || Is<Any>()) {
			++compared;
			auto& lhs_block = Get<Block>();
			auto& rhs_block = right.Get<Block>();
			return lhs_block.Compare(rhs_block);
		}

		// First we check all bases													
		for (auto& base : mType->mBases) {
			auto& baseMeta = base.mType;
			if (baseMeta && baseMeta->mSize > 0) {
				++compared;
				auto lhs_base = GetBaseMemory(baseMeta, base);
				auto rhs_base = right.GetBaseMemory(baseMeta, base);
				if (!lhs_base.Compare(rhs_base, false))
					return false;
			}
		}

		// Iterate members for each element											
		for (auto& member : mType->mMembers) {
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

} // namespace Langulus::Anyness
