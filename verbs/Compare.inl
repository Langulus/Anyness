///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "../Block.hpp"

#define VERBOSE(...) //Logger::Verbose(__VA_ARGS__)
#define VERBOSE_TAB(...) //auto tab = Logger::Section(__VA_ARGS__)

namespace Langulus::Anyness
{

	/// Compare the relevant states of two blocks										
	///	@param right - the memory block to compare against							
	///	@return true if the two memory blocks' revelant states are identical	
	inline bool Block::CompareStates(const Block& right) const noexcept {
		return GetUnconstrainedState() == right.GetUnconstrainedState();
	}

	/// Compare types of two blocks															
	///	@param right - the type to the right											
	///	@param common - the common base													
	///	@return true if a common base has been found									
	inline bool Block::CompareTypes(const Block& right, RTTI::Base& common) const noexcept {
		if (!Is(right.mType)) {
			// Types differ, dig deeper to find out why							
			if (!mType->GetBase(right.mType, 0, common)) {
				// Other type is not base for this one, can't compare them	
				// Let's check in reverse												
				if (!right.mType->GetBase(mType, 0, common)) {
					// Other type is not derived from this one, can't			
					// compare them														
					return false;
				}

				// Other is derived from this, but it has to be binary		
				// compatible to be able to compare them							
				if (!common.mBinaryCompatible) {
					VERBOSE(Logger::Red,
						"Data types are related, but not binary compatible: ",
						GetToken(), " != ", right.GetToken());
					return false;
				}

				return true;
			}
			else {
				// This is derived from other, but it has to be binary		
				// compatible to be able to compare them							
				if (!common.mBinaryCompatible) {
					VERBOSE(Logger::Red,
						"Data types are related, but not binary compatible: ",
						GetToken(), " != ", right.GetToken());
					return false;
				}

				return true;
			}
		}
		else {
			// Types match exactly														
			common.mType = mType;
			common.mBinaryCompatible = true;
			return true;
		}
	}

	/// Invoke a comparator in base, comparing this block against right one		
	///	@param right - the right block													
	///	@param base - the base to use														
	///	@return true if comparison returns true										
	inline bool Block::CallComparer(const Block& right, const RTTI::Base& base) const {
		return mRaw == right.mRaw || (
			mRaw && right.mRaw && base.mType->mComparer(mRaw, right.mRaw)
		);
	}

	/// Compare any data using RTTI															
	/// Nested for each memory subregion, including sparse links					
	///	@param right - the memory block to compare against							
	///	@return true if the two memory blocks are identical						
	template<bool RESOLVE>
	bool Block::Compare(const Block& right) const {
		VERBOSE_TAB("Comparing ",
			Logger::Push, Logger::White, mCount, " elements of ", GetToken(),
			Logger::Pop, " with ",
			Logger::Push, Logger::White, right.mCount, " elements of ", right.GetToken()
		);

		if (mCount != right.mCount) {
			// Cheap early return for differently sized blocks					
			VERBOSE(Logger::Red,
				"Data count is different: ", 
				mCount, " != ", right.mCount);
			return false;
		}

		if (mType != right.mType && (IsUntyped() || right.IsUntyped())) {
			// Cheap early return for blocks of differing undefined type	
			VERBOSE(Logger::Red,
				"One of the containers is untyped: ", 
				GetToken(), " != ", right.GetToken());
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

		RTTI::Base baseForComparison {};
		if constexpr (RESOLVE) {
			// We will test type for each resolved element, individually	
			if (!IsResolvable() && !right.IsResolvable() && !CompareTypes(right, baseForComparison)) {
				// Types differ and are not resolvable								
				VERBOSE(Logger::Red,
					"Data types are not related: ",
					GetToken(), " != ", right.GetToken());
				return false;
			}
		}
		else {
			// We won't be resolving, so we have only one global type		
			if (!CompareTypes(right, baseForComparison)) {
				// Types differ															
				VERBOSE(Logger::Red,
					"Data types are not related: ",
					GetToken(), " != ", right.GetToken());
				return false;
			}
		}

		if (baseForComparison.mType->mIsPOD && baseForComparison.mBinaryCompatible) {
			// Just compare the memory directly (optimization)					
			VERBOSE("Batch-comparing POD memory");
			const auto code = memcmp(mRaw, right.mRaw, 
				mCount * baseForComparison.mType->mSize * baseForComparison.mCount);

			if (code != 0) {
				VERBOSE(Logger::Red, "POD memory is not the same ", Logger::Yellow, "(fast)");
				return false;
			}
			
			VERBOSE(Logger::Green, "POD memory is the same ", Logger::Yellow, "(fast)");
		}
		else if (baseForComparison.mType->mComparer) {
			if (IsSparse()) {
				if constexpr (RESOLVE) {
					// Resolve all elements one by one and compare them by	
					// their common resolved base										
					for (Count i = 0; i < mCount; ++i) {
						auto lhs = GetElementResolved(i);
						auto rhs = right.GetElementResolved(i);
						if (!lhs.CompareTypes(rhs, baseForComparison)) {
							// Fail comparison on first mismatch					
							VERBOSE(Logger::Red,
								"Elements at ", i, " have unrelated types: ", 
								lhs.GetToken(), " != ", rhs.GetToken());
							return false;
						}

						if (!lhs.CallComparer(rhs, baseForComparison)) {
							// Fail comparison on first mismatch					
							VERBOSE(Logger::Red,
								"Elements at ", i, " differ: ", 
								lhs.GetToken(), " != ", rhs.GetToken());
							return false;
						}
					}

					VERBOSE(Logger::Green,
						"Data is the same, all elements match ", 
						Logger::DarkYellow, "(slow)");
				}
				else {
					// Call the reflected == operator in baseForComparison	
					VERBOSE("Comparing using reflected operator == for ",
						baseForComparison.mType->mToken);

					for (Count i = 0; i < mCount; ++i) {
						// Densify and compare all elements by the binary		
						// compatible base												
						auto lhs = GetElementDense(i);
						auto rhs = right.GetElementDense(i);

						if (!lhs.CallComparer(rhs, baseForComparison)) {
							// Fail comparison on first mismatch					
							VERBOSE(Logger::Red, "Elements at ", i, " differ");
							return false;
						}
					}

					VERBOSE(Logger::Green,
						"Data is the same, all elements match ", 
						Logger::Yellow, "(slow)");
				}
			}
			else {
				// Call the reflected == operator in baseForComparison	
				VERBOSE("Comparing using reflected operator == for ",
					baseForComparison.mType->mToken);

				for (Count i = 0; i < mCount; ++i) {
					// Compare all elements by the binary compatible base		
					auto lhs = GetElement(i);
					auto rhs = right.GetElement(i);

					if (!lhs.CallComparer(rhs, baseForComparison)) {
						// Fail comparison on first mismatch						
						VERBOSE(Logger::Red, "Elements at ", i, " differ");
						return false;
					}
				}

				VERBOSE(Logger::Green,
					"Data is the same, all elements match ", 
					Logger::Yellow, "(slow)");
			}
		}
		else {
			VERBOSE(Logger::Red,
				"Can't compare related types because no == operator is reflected, "
				"and they're not POD - common base for comparison was: ",
				baseForComparison.mType->mToken);
			return false;
		}
		/*else {
			// Compare all elements reflected bases and members, one by one
			// We're desperate at this point											
			Count compared_members = 0;
			for (Count i = 0; i < mCount; ++i) {
				auto lhs = RESOLVE ? GetElementResolved(i) : GetElement(i);
				auto rhs = RESOLVE ? right.GetElementResolved(i) : right.GetElement(i);
				if (!lhs.CompareMembers(rhs, compared_members)) {
					// Fail comparison on first mismatch							
					VERBOSE(Logger::Red,
						"Members in element ", i, " differ: ", 
						lhs.GetToken(), " != ", rhs.GetToken());
					return false;
				}
			}

			SAFETY(if (compared_members == 0 && mCount > 0)
				Logger::Error("Comparing checked no members"));

			VERBOSE(Logger::Green,
				"Data is the same, all members match ", 
				Logger::Red, "(slowest)");
		}*/

		return true;
	}

	/// Compare the reflected members, and bases' members of two blocks			
	///	@param right - block to compare against										
	///	@param compared - [in/out] the number of compared members				
	///	@return false if any member differs												
	/*bool Block::CompareMembers(const Block& right, Count& compared) const {
		VERBOSE_TAB("Comparing the members of ", GetToken());

		// Take care of memory blocks directly										
		if (Is<Block>() || Is<Any>()) {
			++compared;
			auto& lhs_block = Get<Block>();
			auto& rhs_block = right.template Get<Block>();
			return lhs_block.Compare(rhs_block);
		}

		// First we check all bases													
		for (auto& base : mType->mBases) {
			auto& baseMeta = base.mType;
			if (baseMeta && baseMeta->mSize > 0) {
				++compared;
				auto lhs_base = GetBaseMemory(baseMeta, base);
				auto rhs_base = right.GetBaseMemory(baseMeta, base);
				if (!lhs_base.template Compare<false>(rhs_base))
					return false;
			}
		}

		// Iterate members for each element											
		for (auto& member : mType->mMembers) {
			if (member.template Is<Block>() || member.template Is<Any>()) {
				// If member is a memory block, nest								
				++compared;
				auto& lhs_block = member.template As<Block>(mRaw);
				auto& rhs_block = member.template As<Block>(right.mRaw);
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
	}*/

} // namespace Langulus::Anyness

#undef VERBOSE
#undef VERBOSE_TAB