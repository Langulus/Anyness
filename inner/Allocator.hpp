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
	
	/// Fast log2																					
	/// https://stackoverflow.com/questions/11376288									
	///	@param u - number																		
	///	@return the log2																		
	constexpr Size FastLog2(Size x) noexcept {
		return x < 2 
			? 0 : Size{8 * sizeof(Size)} - ::std::countl_zero(x) - Size{1};
	}

	/// Get least significant bit																
	/// https://stackoverflow.com/questions/757059										
	///	@param n - number																		
	///	@return the least significant bit												
	constexpr Size LSB(const Size& n) noexcept {
		#if LANGULUS(BITNESS) == 32
			constexpr Size DeBruijnBitPosition[32] = {
				0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
				31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
			};
			constexpr Size f = 0x077CB531u;
			return DeBruijnBitPosition[(Size {n & (0 - n)} * f) >> Size {27}];
		#elif LANGULUS(BITNESS) == 64
			constexpr Size DeBruijnBitPosition[64] = {
				0,   1,  2, 53,  3,  7, 54, 27,  4, 38, 41,  8, 34, 55, 48, 28,
				62,  5, 39, 46, 44, 42, 22,  9, 24, 35, 59, 56, 49, 18, 29, 11,
				63, 52,  6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
				51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12
			};
			constexpr Size f = 0x022fdd63cc95386dul;
			return DeBruijnBitPosition[(Size {n & (0 - n)} * f) >> Size {58}];
		#else
			#error Implement for your architecture
		#endif
	}


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
				// Number of registered entries										
				Count mEntries {};

				#if LANGULUS_FEATURE(MANAGED_MEMORY)
					// Number of registered pools										
					Count mPools {};
				#endif

				#if LANGULUS_FEATURE(MANAGED_REFLECTION)
					// Number of registered meta datas								
					Count mDataDefinitions {};
					// Number of registered meta traits								
					Count mTraitDefinitions {};
					// Number of registered meta verbs								
					Count mVerbDefinitions {};
				#endif

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
			NOD() static Allocation* Find(RTTI::DMeta, const void*) SAFETY_NOEXCEPT();
			NOD() static bool CheckAuthority(RTTI::DMeta, const void*) SAFETY_NOEXCEPT();
			NOD() static Pool* AllocatePool(const Size&) SAFETY_NOEXCEPT();
			static void DeallocatePool(Pool*) SAFETY_NOEXCEPT();
			static void CollectGarbage();
		#endif
	};

} // namespace Langulus::Anyness