///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Allocation.hpp"

namespace Langulus::Anyness::Inner
{

	///																								
	///	Memory allocator																		
	///																								
	/// The lowest-level memory management interface									
	/// Basically an overcomplicated wrapper for malloc/free							
	///																								
	class Allocator {
	private:
		struct Statistics {
			// The real allocated bytes, provided by malloc in backend		
			Size mBytesAllocatedByBackend {};
			// The bytes allocated by the frontend									
			Size mBytesAllocatedByFrontend {};
			// Number of registered pools												
			Count mPools {};
			// Number of registered entries											
			Count mEntries {};
			// Number of registered meta datas										
			Count mDataDefinitions {};
			// Number of registered meta traits										
			Count mTraitDefinitions {};
			// Number of registered meta verbs										
			Count mVerbDefinitions {};

			bool operator == (const Statistics&) const noexcept = default;
			bool operator != (const Statistics&) const noexcept = default;
		};
		
		static Statistics mStatistics;

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			static Pool* mDefaultPool;
		#endif

	public:
		//																						
		//	Standard functionality														
		//																						
		NOD() static Allocation* Allocate(const Size&);
		NOD() static Allocation* Reallocate(const Size&, Allocation*);
		static void Deallocate(Allocation*);

		//																						
		// More functionality, when feature MANAGED_MEMORY is enabled		
		//																						
		NOD() static Allocation* Find(DMeta, const void*);
		NOD() static bool CheckAuthority(DMeta, const void*);
		NOD() static Count GetReferences(DMeta, const void*);
		static void Keep(DMeta, const void*, Count);
		NOD() static bool Free(DMeta, const void*, Count);
		NOD() static const Statistics& GetStatistics() noexcept;
	};

} // namespace Langulus::Anyness