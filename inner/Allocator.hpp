#pragma once
#include "Integration.hpp"
#include "Reflection.hpp"

namespace Langulus::Anyness
{

	class Pool;
	
	template<IsUnsigned T>
	NOD() constexpr T Roof2(const T&) noexcept;

	///																								
	///	Memory entry																			
	///																								
	/// This is a single allocation record inside a memory pool						
	///																								
	class Entry {
	public:
		// Allocated bytes for this chunk											
		Size mAllocatedBytes;
		// The number of references to this memory								
		Count mReferences;
		// The pool that owns the memory entry										
		Pool* mOwner;

	public:
		constexpr Entry() = delete;
		constexpr Entry(const Size&, Pool*) noexcept;
		~Entry() noexcept;

	public:
		static constexpr Size GetSize() noexcept;
		constexpr bool IsInUse() const noexcept;
		const Byte* GetBlockStart() const noexcept;
		Byte* GetBlockStart() noexcept;
		constexpr Size GetTotalSize() const noexcept;
		constexpr const Size& Allocated() const noexcept;
		bool Contains(const Byte*) const noexcept;
		bool CollisionFree(const Entry&) const noexcept;
	};


	///																								
	///	Memory allocator																		
	///																								
	/// The primary memory management interface											
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
		// Standard functionality														
		//																						
		NOD() static Entry* Allocate(Size);
		NOD() static Entry* Reallocate(Size, Entry*);
		static void Deallocate(DMeta, Entry*);

		//																						
		// More functionality, when feature MANAGED_MEMORY is enabled		
		//																						
		NOD() static Entry* Find(DMeta, const void*);
		NOD() static bool CheckAuthority(DMeta, const void*);
		NOD() static Count GetReferences(DMeta, const void*);
		static void Keep(DMeta, const void*, Count);
		NOD() static bool Free(DMeta, const void*, Count);
		NOD() static const Statistics& GetStatistics() noexcept;
	};

} // namespace Langulus::Anyness

#include "Allocator.inl"
