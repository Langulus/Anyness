///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "inner/Index.hpp"
#include "inner/Allocator.hpp"

namespace Langulus::Anyness
{
	
	/// Predeclarations																			
	class Any;
	template<CT::Data>
	class TAny;
	
	class Map;
	template<CT::Data, CT::Data>
	class TMap;
	
	class Set;
	template<CT::Data>
	class TSet;
	
	class Bytes;
	class Text;
	class Path;
	
	template<CT::Data>
	class TOwned;
	template<CT::Data, bool>
	class TPointer;

	template<CT::Data, CT::Data>
	class THashMap;

	/// Compression types, analogous to zlib's											
	enum class Compression {
		None = 0,
		Fastest = 1,
		Balanced = 5,
		Smallest = 9,
		
		Default = Fastest
	};
	
	/// Data can have a temporal phase														
	/// Temporal data phases are used extensively by Langulus, but not at all	
	/// in standalone use. Either way - overhead is literally two bits, so		
	/// I've not taken the initiative to remove them - use them as you wish		
	/// Two bits for free! Free bits, yo! Check Block::GetState() and				
	/// DataState::Phased and DataState::Future for the aforementioned 2 bits	
	/// Remember, these are free only if you're using Anyness as standalone		
	enum class Phase : int {
		Past = -1,
		Now = 0,
		Future = 1
	};
	
	
	///																								
	///	BLOCK																						
	///																								
	///	Wraps an allocated memory block; acts as base to all containers.		
	///	This is an inner structure, that doesn't reference any memory,			
	/// only provides the functionality to do so. You can use Block as a			
	/// lightweight intermediate structure for iteration, etc.						
	///																								
	class Block {
		LANGULUS(DEEP) true;

		friend class Any;
		template<CT::Data>
		friend class TAny;

		friend class Map;
		template<CT::Data, CT::Data>
		friend class TMap;

		friend class Set;
		template<CT::Data>
		friend class TSet;

		friend class Bytes;
		friend class Text;
		friend class Path;

		template<CT::Data>
		friend class TOwned;
		template<CT::Data, bool REFERENCED>
		friend class TPointer;

		template<CT::Data, CT::Data>
		friend class THashMap;

	protected:
		// A structure used to represent an element of a sparse container	
		struct KnownPointer {
			Byte* mPointer;
			Inner::Allocation* mEntry;
		};

		union { 
			#if LANGULUS_DEBUG()
				char* mRawChar;
			#endif
			// Raw pointer to first element inside the memory block			
			Byte* mRaw {};
			KnownPointer* mRawSparse;
		};
	
		// The data state																	
		DataState mState {DataState::Default};
		// Number of initialized elements inside memory block					
		Count mCount {};
		// Number of allocated elements in the memory block					
		Count mReserved {};
		// Meta data about the elements inside the memory block				
		DMeta mType {};
		// Pointer to the allocated block											
		// If entry is zero, then data is static									
		Inner::Allocation* mEntry {};

	public:
		constexpr Block() noexcept = default;
		constexpr Block(const Block&) noexcept = default;
		constexpr Block(Block&&) noexcept = default;

		constexpr Block(Disowned<Block>&&) noexcept;
		constexpr Block(Abandoned<Block>&&) noexcept;
			
		explicit constexpr Block(DMeta) noexcept;
		constexpr Block(const DataState&, DMeta) noexcept;

		Block(const DataState&, DMeta, Count, const void*) noexcept;
		Block(const DataState&, DMeta, Count, void*) noexcept;
		Block(const DataState&, DMeta, Count, const void*, Inner::Allocation*) noexcept;
		Block(const DataState&, DMeta, Count, void*, Inner::Allocation*) noexcept;
	
		template<CT::Data T, bool CONSTRAIN = false>
		NOD() static Block From(T) requires CT::Sparse<T>;
		template<CT::Data T, bool CONSTRAIN = false>
		NOD() static Block From(T, Count) requires CT::Sparse<T>;
		template<CT::Data T, bool CONSTRAIN = false>
		NOD() static Block From(T&) requires CT::Dense<T>;
		template<CT::Data T, bool CONSTRAIN = false>
		NOD() static Block From();

		constexpr Block& operator = (const Block&) noexcept = default;
		constexpr Block& operator = (Block&&) noexcept = default;
			
		void TakeAuthority();
		void Optimize();

	protected:
		template<bool CONSTRAIN>
		void SetType(DMeta);
		template<CT::Data T, bool CONSTRAIN>
		void SetType();

	public:
		//																						
		//	Capsulation and access														
		//																						
		constexpr void SetPhase(Phase) noexcept;
		constexpr void SetState(DataState) noexcept;
		constexpr void AddState(DataState) noexcept;
		constexpr void RemoveState(DataState) noexcept;
	
		NOD() constexpr const DMeta& GetType() const noexcept;
		NOD() constexpr const Count& GetCount() const noexcept;
		NOD() constexpr const Count& GetReserved() const noexcept;
		NOD() constexpr Size GetReservedSize() const noexcept;
		NOD() Count GetCountDeep() const noexcept;
		NOD() Count GetCountElementsDeep() const noexcept;
		NOD() constexpr bool IsAllocated() const noexcept;
		NOD() constexpr bool IsPast() const noexcept;
		NOD() constexpr bool IsFuture() const noexcept;
		NOD() constexpr bool IsNow() const noexcept;
		NOD() constexpr bool IsMissing() const noexcept;
		NOD() constexpr bool IsUntyped() const noexcept;
		NOD() constexpr bool IsTypeConstrained() const noexcept;
		NOD() constexpr bool IsPhased() const noexcept;
		NOD() constexpr bool IsEncrypted() const noexcept;
		NOD() constexpr bool IsCompressed() const noexcept;
		NOD() constexpr bool IsConstant() const noexcept;
		NOD() constexpr bool IsStatic() const noexcept;
		NOD() constexpr bool IsAbstract() const noexcept;
		NOD() constexpr bool IsDefaultable() const noexcept;
		NOD() constexpr bool IsOr() const noexcept;
		NOD() constexpr bool IsEmpty() const noexcept;
		NOD() constexpr bool IsValid() const noexcept;
		NOD() constexpr bool IsInvalid() const noexcept;
		NOD() constexpr bool IsDense() const noexcept;
		NOD() constexpr bool IsSparse() const noexcept;
		NOD() constexpr bool IsPOD() const noexcept;
		NOD() constexpr bool IsResolvable() const noexcept;
		NOD() constexpr bool IsNullifiable() const noexcept;
		NOD() constexpr bool IsDeep() const noexcept;
		NOD() constexpr Phase GetPhase() const noexcept;
		NOD() constexpr bool CanFitPhase(const Phase&) const noexcept;
		NOD() constexpr bool CanFitState(const Block&) const noexcept;
		NOD() constexpr Count GetByteSize() const noexcept;
		NOD() constexpr Token GetToken() const noexcept;
		NOD() constexpr Size GetStride() const noexcept;
		NOD() constexpr const DataState& GetState() const noexcept;
		NOD() constexpr DataState GetUnconstrainedState() const noexcept;
		NOD() constexpr Byte* GetRaw() noexcept;
		NOD() constexpr const Byte* GetRaw() const noexcept;
		NOD() constexpr Byte* GetRawEnd() noexcept;
		NOD() constexpr const Byte* GetRawEnd() const noexcept;
		NOD() constexpr KnownPointer* GetRawSparse() noexcept;
		NOD() constexpr const KnownPointer* GetRawSparse() const noexcept;
		
		NOD() constexpr bool IsMissingDeep() const;
		
		template<CT::Data T>
		NOD() T* GetRawAs() noexcept;
		template<CT::Data T>
		NOD() const T* GetRawAs() const noexcept;
		
		template<CT::Data T>
		NOD() T* GetRawEndAs() noexcept;
		template<CT::Data T>
		NOD() const T* GetRawEndAs() const noexcept;

		NOD() bool IsConcatable(const Block&) const noexcept;
		
		NOD() bool IsInsertable(DMeta) const noexcept;
		template<CT::Data T>
		NOD() bool IsInsertable() const noexcept;
	
		NOD() bool operator == (const Block&) const noexcept;
		NOD() bool operator == (::std::nullptr_t) const noexcept;
	
		NOD() Byte* At(const Offset& = 0);
		NOD() const Byte* At(const Offset& = 0) const;
	
		template<CT::Data T>
		NOD() decltype(auto) Get(const Offset& = 0, const Offset& = 0);
		template<CT::Data T>
		NOD() decltype(auto) Get(const Offset& = 0, const Offset& = 0) const;
	
		template<CT::Data T, CT::Index IDX = Offset>
		NOD() decltype(auto) As(const IDX& = {});
		template<CT::Data T, CT::Index IDX = Offset>
		NOD() decltype(auto) As(const IDX& = {}) const;
	
		template<CT::Data T, bool FATAL_FAILURE = true>
		NOD() T AsCast(Index) const;
		template<CT::Data T, bool FATAL_FAILURE = true>
		NOD() T AsCast() const;
	
		NOD() Block GetElementDense(Offset);
		NOD() const Block GetElementDense(Offset) const;
	
		NOD() Block GetElementResolved(Offset);
		NOD() const Block GetElementResolved(Offset) const;
	
		NOD() Block GetElement(Offset) noexcept;
		NOD() const Block GetElement(Offset) const noexcept;
	
		NOD() Block* GetBlockDeep(Offset) noexcept;
		NOD() const Block* GetBlockDeep(Offset) const noexcept;
	
		NOD() Block GetElementDeep(Offset) noexcept;
		NOD() const Block GetElementDeep(Offset) const noexcept;
	
		//																						
		//	Iteration																		
		//																						
		Count ForEachElement(TFunctor<bool(const Block&)>&&) const;
		Count ForEachElement(TFunctor<bool(Block&)>&&);
		Count ForEachElement(TFunctor<void(const Block&)>&&) const;
		Count ForEachElement(TFunctor<void(Block&)>&&);
	
		template<bool MUTABLE = true, class... F>
		Count ForEach(F&&...);
		template<class... F>
		Count ForEach(F&&...) const;
	
		template<bool MUTABLE = true, class... F>
		Count ForEachRev(F&&...);
		template<class... F>
		Count ForEachRev(F&&...) const;
	
		template<bool SKIP = true, bool MUTABLE = true, class... F>
		Count ForEachDeep(F&&...);
		template<bool SKIP = true, class... F>
		Count ForEachDeep(F&&...) const;
	
		template<bool SKIP = true, bool MUTABLE = true, class... F>
		Count ForEachDeepRev(F&&...);
		template<bool SKIP = true, class... F>
		Count ForEachDeepRev(F&&...) const;
	
	protected:
		void CheckRange(const Offset&, const Count&) const;
		
	private:
		template<bool MUTABLE, bool REVERSE, class F>
		Count ForEachSplitter(F&&);
		template<bool SKIP, bool MUTABLE, bool REVERSE, class F>
		Count ForEachDeepSplitter(F&&);

		template<class R, CT::Data A, bool REVERSE, bool MUTABLE>
		Count ForEachInner(TFunctor<R(A)>&&);
		template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE>
		Count ForEachDeepInner(TFunctor<R(A)>&&);
	
		/// This function declaration is used to decompose a lambda				
		/// You can use it to extract the argument type of the lambda, using	
		/// decltype on the return type. Useful for template deduction in		
		/// the ForEach functions above, purely for convenience					
		template<typename R, typename F, typename A>
		A GetLambdaArgument(R(F::*)(A) const) const;
	
		NOD() Block CropInner(const Offset&, const Count&, const Count&) const noexcept;
	
	public:
		NOD() bool Owns(const void*) const noexcept;
		NOD() constexpr bool HasAuthority() const noexcept;
		NOD() constexpr Count GetUses() const noexcept;
		NOD() Block Crop(const Offset&, const Count&);
		NOD() Block Crop(const Offset&, const Count&) const;
	
		NOD() Block GetMember(const RTTI::Member&) const;
		NOD() Block GetMember(const RTTI::Member&);
		NOD() Block GetMember(TMeta, Offset = 0) const;
		NOD() Block GetMember(TMeta, Offset = 0);
		NOD() Block GetMember(DMeta, Offset = 0) const;
		NOD() Block GetMember(DMeta, Offset = 0);
		NOD() Block GetMember(std::nullptr_t, Offset = 0) const;
		NOD() Block GetMember(std::nullptr_t, Offset = 0);
	
		NOD() Block GetBaseMemory(DMeta, const RTTI::Base&) const;
		NOD() Block GetBaseMemory(DMeta, const RTTI::Base&);
		NOD() Block GetBaseMemory(const RTTI::Base&) const;
		NOD() Block GetBaseMemory(const RTTI::Base&);
	
		template<CT::Data T, bool ALLOW_DEEPEN, CT::Data WRAPPER = Any>
		bool Mutate();
		template<bool ALLOW_DEEPEN, CT::Data WRAPPER = Any>
		bool Mutate(DMeta);

		constexpr void MakeMissing() noexcept;
		constexpr void MakeStatic() noexcept;
		constexpr void MakeConst() noexcept;
		constexpr void MakeTypeConstrained() noexcept;
		constexpr void MakeOr() noexcept;
		constexpr void MakeAnd() noexcept;
		constexpr void MakePast() noexcept;
		constexpr void MakeFuture() noexcept;
		constexpr void MakeSparse() noexcept;
		constexpr void MakeDense() noexcept;
	
		Count Copy(Block&) const;
		Count Clone(Block&) const;
	
		NOD() bool CompareMembers(const Block&, Count& compared) const;
		NOD() bool CompareStates(const Block&) const noexcept;
		NOD() bool Compare(const Block&, bool resolve = true) const;
	
		template<bool CREATE = false, bool SETSIZE = false>
		void Allocate(const Count&);
		
		void Shrink(Count);
	
		#if LANGULUS_FEATURE(ZLIB)
			Size Compress(Block&, Compression = Compression::Default) const;
			Size Decompress(Block&) const;
		#endif

		Size Encrypt(Block&, const ::std::size_t*, const Count&) const;
		Size Decrypt(Block&, const ::std::size_t*, const Count&) const;
	
		NOD() Hash GetHash() const;
	
		template<CT::Data T>
		NOD() Index Find(const T&, const Index& = IndexFront) const;
	
		template<CT::Data T>
		NOD() Index FindDeep(const T&, const Index& = IndexFront) const;
		NOD() Index FindRTTI(const Block&, const Index& = IndexFront) const;
	
		Count Gather(Block&, Index = IndexFront) const;
		Count Gather(Block&, Phase, Index = IndexFront) const;
	
		template<CT::Data T>
		void Swap(Offset, Offset);
		template<CT::Data T>
		void Swap(Index, Index);
	

		//																						
		//	Insertion																		
		//																						
		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count InsertAt(const T*, const T*, Index);
		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count InsertAt(const T*, const T*, Offset);
		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count InsertAt(T&&, Index);
		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count InsertAt(T&&, Offset);

		template<Index INDEX = IndexBack, bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count Insert(const T*, const T*);
		template<Index INDEX = IndexBack, bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count Insert(T&&);

		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count MergeAt(const T*, const T*, Index);
		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count MergeAt(const T*, const T*, Offset);
		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count MergeAt(T&&, Index);
		template<bool KEEP, bool MUTABLE, CT::Data WRAPPER = Any, CT::NotAbandonedOrDisowned T>
		Count MergeAt(T&&, Offset);

		template<CT::NotAbandonedOrDisowned T>
		Count InsertBlockAt(const T&, Index);
		template<CT::NotAbandonedOrDisowned T>
		Count InsertBlockAt(const T&, Offset);
		template<CT::NotAbandonedOrDisowned T>
		Count InsertBlockAt(T&&, Index);
		template<CT::NotAbandonedOrDisowned T>
		Count InsertBlockAt(T&&, Offset);

		template<CT::Data T>
		Count InsertBlockAt(Disowned<T>&&, Index);
		template<CT::Data T>
		Count InsertBlockAt(Disowned<T>&&, Offset);
		template<CT::Data T>
		Count InsertBlockAt(Abandoned<T>&&, Index);
		template<CT::Data T>
		Count InsertBlockAt(Abandoned<T>&&, Offset);

		template<Index INDEX = IndexBack, CT::NotAbandonedOrDisowned T>
		Count InsertBlock(const T&);
		template<Index INDEX = IndexBack, CT::NotAbandonedOrDisowned T>
		Count InsertBlock(T&&);

		template<Index INDEX = IndexBack, CT::Data T>
		Count InsertBlock(Disowned<T>&&);
		template<Index INDEX = IndexBack, CT::Data T>
		Count InsertBlock(Abandoned<T>&&);

		template<CT::NotAbandonedOrDisowned T, class INDEX>
		Count MergeBlockAt(const T&, INDEX);
		template<CT::NotAbandonedOrDisowned T, class INDEX>
		Count MergeBlockAt(T&&, INDEX);

		template<CT::Data T, class INDEX>
		Count MergeBlockAt(Disowned<T>&&, INDEX);
		template<CT::Data T, class INDEX>
		Count MergeBlockAt(Abandoned<T>&&, INDEX);
	
		template<Index INDEX = IndexBack, CT::NotAbandonedOrDisowned T>
		Count MergeBlock(const T&);
		template<Index INDEX = IndexBack, CT::NotAbandonedOrDisowned T>
		Count MergeBlock(T&&);

		template<Index INDEX = IndexBack, CT::Data T>
		Count MergeBlock(Disowned<T>&&);
		template<Index INDEX = IndexBack, CT::Data T>
		Count MergeBlock(Abandoned<T>&&);
	
		template<CT::Data T, bool MOVE_STATE = true>
		T& Deepen();

		template<bool CONCAT = true, bool DEEPEN = true, CT::NotAbandonedOrDisowned T, CT::Data INDEX, CT::Data WRAPPER = Any>
		Count SmartPushAt(const T&, INDEX, DataState = {});
		template<bool CONCAT = true, bool DEEPEN = true, CT::NotAbandonedOrDisowned T, CT::Data INDEX, CT::Data WRAPPER = Any>
		Count SmartPushAt(T&, INDEX, DataState = {});
		template<bool CONCAT = true, bool DEEPEN = true, CT::NotAbandonedOrDisowned T, CT::Data INDEX, CT::Data WRAPPER = Any>
		Count SmartPushAt(T&&, INDEX, DataState = {});
		template<bool CONCAT = true, bool DEEPEN = true, CT::Data T, CT::Data INDEX, CT::Data WRAPPER = Any>
		Count SmartPushAt(Disowned<T>&&, INDEX, DataState = {});
		template<bool CONCAT = true, bool DEEPEN = true, CT::Data T, CT::Data INDEX, CT::Data WRAPPER = Any>
		Count SmartPushAt(Abandoned<T>&&, INDEX, DataState = {});

		template<Index INDEX = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::NotAbandonedOrDisowned T, CT::Data WRAPPER = Any>
		Count SmartPush(const T&, DataState = {});
		template<Index INDEX = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::NotAbandonedOrDisowned T, CT::Data WRAPPER = Any>
		Count SmartPush(T&, DataState = {});
		template<Index INDEX = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::NotAbandonedOrDisowned T, CT::Data WRAPPER = Any>
		Count SmartPush(T&&, DataState = {});
		template<Index INDEX = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data T, CT::Data WRAPPER = Any>
		Count SmartPush(Disowned<T>&&, DataState = {});
		template<Index INDEX = IndexBack, bool CONCAT = true, bool DEEPEN = true, CT::Data T, CT::Data WRAPPER = Any>
		Count SmartPush(Abandoned<T>&&, DataState = {});

		//																						
		//	Deletion																			
		//																						
		template<CT::Data T>
		Count Remove(const T*, Count = 1, Index = IndexFront);
		Count RemoveIndex(Index, Count = 1);
		Count RemoveIndex(Offset, Count = 1);
		Count RemoveIndexDeep(Offset);
	
		Block& Trim(Offset);
	
		NOD() constexpr Index Constrain(const Index&) const noexcept;
	
		template<CT::Data T>
		NOD() Index ConstrainMore(const Index&) const noexcept;
	
		template<CT::Data T>
		NOD() Index GetIndexMax() const noexcept requires CT::Sortable<T, T>;
	
		template<CT::Data T>
		NOD() Index GetIndexMin() const noexcept requires CT::Sortable<T, T>;
	
		template<CT::Data T>
		NOD() Index GetIndexMode(Count&) const noexcept;
	
		template<CT::Data T>
		void Sort(const Index&) noexcept;
	
		NOD() bool CanFit(DMeta) const;
		NOD() bool CanFit(const Block&) const;
		template<CT::Data T>
		NOD() bool CanFit() const;
	
		NOD() bool CastsToMeta(DMeta) const;
		NOD() bool CastsToMeta(DMeta, Count) const;

		template<CT::Data T>
		NOD() bool CastsTo() const;
		template<CT::Data T>
		NOD() bool CastsTo(Count) const;
	
		NOD() bool Is(DMeta) const noexcept;
		template<CT::Data... T>
		NOD() bool Is() const;
		
		NOD() Block ReinterpretAs(const Block&) const;
	
		void Clear();
		void Reset();

	protected:
		template<bool CREATE = false>
		void AllocateInner(const Count&);
		auto RequestSize(const Count&) const noexcept;
	
		template<bool KEEP, CT::NotAbandonedOrDisowned T>
		void InsertInner(const T*, const T*, Offset);
		template<bool KEEP, CT::NotAbandonedOrDisowned T>
		void InsertInner(T&&, Offset);

		static void CopyMemory(const void*, void*, const Size&) noexcept;
		static void MoveMemory(const void*, void*, const Size&) noexcept;
		static void FillMemory(void*, Byte, const Size&) noexcept;
		NOD() static int CompareMemory(const void*, const void*, const Size&) noexcept;
		
		constexpr void ClearInner() noexcept;
		constexpr void ResetMemory() noexcept;
		constexpr void ResetState() noexcept;
		constexpr void ResetType() noexcept;
	
		void Reference(const Count&) const noexcept;
		void Reference(const Count&) noexcept;
		void Keep() const noexcept;
		void Keep() noexcept;
		
		template<bool DESTROY>
		bool Dereference(const Count&);
		bool Free();
	
		void CallUnknownDefaultConstructors(Count);
		template<CT::Data T>
		void CallKnownDefaultConstructors(Count);

		template<bool KEEP>
		void CallUnknownCopyConstructors(Count, const Block&);
		template<bool KEEP, CT::Data T>
		void CallKnownCopyConstructors(Count, const Block&);

		template<bool KEEP>
		void CallUnknownMoveConstructors(Count, Block&&);
		template<bool KEEP, CT::Data T>
		void CallKnownMoveConstructors(Count, Block&&);

		void CallUnknownDestructors();
		template<CT::Data T>
		void CallKnownDestructors();
	
		void AllocateRegion(const Block&, Offset, Block&);
	};

		
	/// Macro used to implement the standard container interface used in			
	/// range-for-statements like for (auto& item : array)							
	/// In addition, a Reverse() function is added for reverse iteration			
	#define RANGED_FOR_INTEGRATION(containerType, iteratorType) \
		NOD() inline iteratorType* begin() noexcept { return GetRaw(); } \
		NOD() inline iteratorType* end() noexcept { return GetRawEnd(); } \
		NOD() inline iteratorType& last() noexcept { return *(GetRawEnd() - 1); } \
		NOD() inline const iteratorType* begin() const noexcept { return GetRaw(); } \
		NOD() inline const iteratorType* end() const noexcept { return GetRawEnd(); } \
		NOD() inline const iteratorType& last() const noexcept { return *(GetRawEnd() - 1); } \
		NOD() inline auto& size() const noexcept { return GetCount(); } \
		NOD() inline Inner::TReverse<const containerType, iteratorType> Reverse() const noexcept { return {*this}; } \
		NOD() inline Inner::TReverse<containerType, iteratorType> Reverse() noexcept { return {*this}; }

	namespace Inner
	{

		///																							
		/// Reverse adaptor for ranged-for expressions									
		///																							
		template<class T, class E>
		class TReverse {
		private:
			using ITERATOR = ::std::conditional_t<::std::is_const_v<T>, const E*, E*>;
		public:
			TReverse() = delete;
			TReverse(const TReverse&) = delete;
			TReverse(TReverse&&) = delete;
			TReverse(T&) noexcept;

			struct PointerWrapper {
				PointerWrapper() = delete;
				PointerWrapper(const PointerWrapper&) noexcept = default;
				PointerWrapper(PointerWrapper&&) noexcept = default;
				PointerWrapper(ITERATOR) noexcept;

				bool operator == (ITERATOR) const noexcept;
				bool operator != (ITERATOR) const noexcept;
				PointerWrapper& operator ++ () noexcept;
				PointerWrapper& operator -- () noexcept;
				operator ITERATOR () const noexcept;

				ITERATOR mPointer;
			};

			NOD() PointerWrapper begin() const noexcept;
			NOD() PointerWrapper end() const noexcept;
			NOD() E& last() const noexcept;
			NOD() Count size() const noexcept;

		private:
			T& mContainer;
		};

	} // namespace Langulus::Anyness::Inner
	
} // namespace Langulus::Anyness

namespace Langulus::CT
{

	/// A reflected block type is any type that inherits Block, and is			
	/// binary compatible to a Block - this is a mandatory requirement for		
	/// any CT::Deep type																		
	/// Keep in mind, that sparse types are never considered Block!				
	template<class... T>
	concept Block = ((DerivedFrom<T, ::Langulus::Anyness::Block>
		&& sizeof(T) == sizeof(::Langulus::Anyness::Block)) && ...);

	/// A deep type is any type with a true static member T::CTTI_Deep,			
	/// is binary compatible with Block, as well as having the same interface	
	/// If no such member/base exists, the type is assumed NOT deep by			
	/// default. Deep types are considered iteratable, and verbs are				
	/// executed in each of their elements/members, instead on the type			
	/// itself. Use LANGULUS(DEEP) macro as member to tag deep types				
	/// Keep in mind, that sparse types are never considered Deep!					
	template<class... T>
	concept Deep = ((Block<T> && Decay<T>::CTTI_Deep) && ...);

	/// Type that is not deep, see CT::Deep												
	template<class... T>
	concept Flat = ((!Deep<T>) && ...);

	/// Check if a type can be handled generically by templates, and				
	/// doesn't	require any special handling												
	template<class... T>
	concept CustomData = ((Data<T> && Flat<T> && NotAbandonedOrDisowned<T>) && ...);

} // namespace Langulus::CT

#include "Block.inl"
