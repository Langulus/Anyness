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
		#if LANGULUS_FEATURE(MEMORY_STATISTICS)
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
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			static Pool* mDefaultPool;
			static Pool* mLastFoundPool;
		#endif

	public:
		//																						
		//	Standard functionality														
		//																						
		NOD() static Allocation* Allocate(const Size&) SAFETY_NOEXCEPT();
		NOD() static Allocation* Reallocate(const Size&, Allocation*) SAFETY_NOEXCEPT();
		static void Deallocate(Allocation*) SAFETY_NOEXCEPT();

		#if LANGULUS_FEATURE(MEMORY_STATISTICS)
			NOD() static const Statistics& GetStatistics() noexcept;
		#endif

		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			//																					
			// More functionality, when feature MANAGED_MEMORY is enabled	
			//																					
			NOD() static Allocation* Find(DMeta, const void*) SAFETY_NOEXCEPT();
			NOD() static bool CheckAuthority(DMeta, const void*) SAFETY_NOEXCEPT();
			NOD() static Pool* AllocatePool(const Size&) SAFETY_NOEXCEPT();
			static void DeallocatePool(Pool*) SAFETY_NOEXCEPT();
			static void CollectGarbage();
		#endif
	};

} // namespace Langulus::Anyness