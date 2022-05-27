///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Reflection.hpp"

namespace Langulus::Anyness
{

	#if LANGULUS_FEATURE(MANAGED_MEMORY)
		class Pool;
	#else
		using Pool = void;
	#endif

	
	///																								
	///	Memory allocation																		
	///																								
	/// This is a single allocation record													
	///																								
	struct Allocation {
	friend class Allocator;
	protected:
		// Allocated bytes for this chunk											
		Size mAllocatedBytes;
		// The number of references to this memory								
		Count mReferences;
		// The pool that owns the memory entry										
		Pool* mPool;

	public:
		Allocation() = delete;
		Allocation(const Allocation&) = delete;
		Allocation(Allocation&&) = delete;
		constexpr Allocation(const Size&, Pool*) noexcept;

		static constexpr Size GetSize() noexcept;
		constexpr const Count& GetUses() const noexcept;
		const Byte* GetBlockStart() const noexcept;
		const Byte* GetBlockEnd() const noexcept;
		Byte* GetBlockStart() noexcept;
		constexpr Size GetTotalSize() const noexcept;
		constexpr const Size& GetAllocatedSize() const noexcept;
		bool Contains(const void*) const noexcept;
		bool CollisionFree(const Allocation&) const noexcept;
		template<class T>
		NOD() T* As() const noexcept;
		constexpr void Keep() noexcept;
		constexpr void Keep(const Count&) noexcept;
		template<bool DEALLOCATE>
		void Free() noexcept;
		template<bool DEALLOCATE>
		void Free(const Count&) SAFETY_NOEXCEPT();
	};


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

#include "Allocator.inl"
