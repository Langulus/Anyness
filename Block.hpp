///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "inner/Reflection.hpp"
#include "inner/DataState.hpp"
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

	namespace Inner
	{
		template<bool DENSE, Count MaxLoadFactor100, CT::Data K, class V>
		class Table;
	}

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

		template<bool DENSE, Count MaxLoadFactor100, CT::Data K, class V>
		class Table;

	protected:
		union { 
			#if LANGULUS_DEBUG()
				char* mRawChar;
			#endif
			// Raw pointer to first element inside the memory block			
			Byte* mRaw {};
			Byte** mRawSparse;
		};
	
		// The data state																	
		DataState mState {DataState::Default};
		// Number of initialized instances inside memory block				
		Count mCount {};
		// Number of allocated instances in the memory block					
		Count mReserved {};
		// Type of the instances inside the memory block						
		DMeta mType {};
		// Pointer to the allocated block											
		// If entry is zero, then data is static									
		Entry* mEntry {};

	public:
		constexpr Block() noexcept = default;
		constexpr Block(const Block&) noexcept;
		constexpr Block(Block&) noexcept = default;
		constexpr Block(Block&&) noexcept = default;
			
		explicit constexpr Block(DMeta) noexcept;
		constexpr Block(const DataState&, DMeta) noexcept;

		Block(DMeta, Count, const void*) noexcept;
		Block(DMeta, Count, void*) noexcept;
		Block(DMeta, Count, const void*, Entry*) noexcept;
		Block(DMeta, Count, void*, Entry*) noexcept;

		Block(const DataState&, DMeta, Count, const void*) noexcept;
		Block(const DataState&, DMeta, Count, void*) noexcept;
		Block(const DataState&, DMeta, Count, const void*, Entry*) noexcept;
		Block(const DataState&, DMeta, Count, void*, Entry*) noexcept;
	
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
	
	public:
		template<bool SPARSE, bool CONSTRAIN>
		void SetType(DMeta);
		template<CT::Data T, bool CONSTRAIN>
		void SetType();
	
		constexpr void SetPhase(const Phase) noexcept;
		constexpr void SetState(DataState) noexcept;
	
		NOD() constexpr const DMeta& GetType() const noexcept;
		NOD() constexpr const Count& GetCount() const noexcept;
		NOD() constexpr const Count& GetReserved() const noexcept;
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
		NOD() constexpr bool IsConstructible() const noexcept;
		NOD() constexpr bool IsOr() const noexcept;
		NOD() constexpr bool IsEmpty() const noexcept;
		NOD() constexpr bool IsValid() const noexcept;
		NOD() constexpr bool IsInvalid() const noexcept;
		NOD() constexpr bool IsDense() const;
		NOD() constexpr bool IsSparse() const;
		NOD() constexpr bool IsDeep() const noexcept;
		NOD() constexpr Phase GetPhase() const noexcept;
		NOD() constexpr bool CanFitPhase(const Phase&) const noexcept;
		NOD() constexpr bool CanFitState(const Block&) const noexcept;
		NOD() constexpr Count GetSize() const noexcept;
		NOD() constexpr Token GetToken() const noexcept;
		NOD() constexpr Size GetStride() const noexcept;
		NOD() constexpr const DataState& GetState() const noexcept;
		NOD() constexpr DataState GetUnconstrainedState() const noexcept;
		NOD() constexpr Byte* GetRaw() noexcept;
		NOD() constexpr const Byte* GetRaw() const noexcept;
		NOD() constexpr Byte* GetRawEnd() noexcept;
		NOD() constexpr const Byte* GetRawEnd() const noexcept;
		NOD() constexpr Byte** GetRawSparse() noexcept;
		NOD() constexpr const Byte* const* GetRawSparse() const noexcept;
		
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
		NOD() bool operator != (const Block&) const noexcept;
	
		NOD() Byte* At(const Offset& = 0);
		NOD() const Byte* At(const Offset& = 0) const;
	
		template<CT::Data T>
		NOD() decltype(auto) Get(const Offset& = 0, const Offset& = 0);
		template<CT::Data T>
		NOD() decltype(auto) Get(const Offset& = 0, const Offset& = 0) const;
	
		template<CT::Data T>
		NOD() decltype(auto) As(const Offset& = 0);
		template<CT::Data T>
		NOD() decltype(auto) As(const Offset& = 0) const;
	
		template<CT::Data T>
		NOD() decltype(auto) As(const Index&);
		template<CT::Data T>
		NOD() decltype(auto) As(const Index&) const;
	
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
	
		Count ForEachElement(TFunctor<bool(const Block&)>&&) const;
		Count ForEachElement(TFunctor<bool(Block&)>&&);
		Count ForEachElement(TFunctor<void(const Block&)>&&) const;
		Count ForEachElement(TFunctor<void(Block&)>&&);
	
		template<bool MUTABLE = true, class F>
		Count ForEach(F&&);
		template<class F>
		Count ForEach(F&&) const;
	
		template<bool MUTABLE = true, class F>
		Count ForEachRev(F&&);
		template<class F>
		Count ForEachRev(F&&) const;
	
		template<bool SKIP = true, bool MUTABLE = true, class F>
		Count ForEachDeep(F&&);
		template<bool SKIP = true, class F>
		Count ForEachDeep(F&&) const;
	
		template<bool SKIP = true, bool MUTABLE = true, class F>
		Count ForEachDeepRev(F&&);
		template<bool SKIP = true, class F>
		Count ForEachDeepRev(F&&) const;
	
	protected:
		void CheckRange(const Offset& start, const Count& count) const;
		
	private:
		template<class R, CT::Data A, bool REVERSE, bool MUTABLE = true>
		Count ForEachInner(TFunctor<R(A)>&&);
		template<class R, CT::Data A, bool REVERSE>
		Count ForEachInner(TFunctor<R(A)>&&) const;
	
		template<class R, CT::Data A, bool REVERSE, bool SKIP, bool MUTABLE = true>
		Count ForEachDeepInner(TFunctor<R(A)>&&);
		template<class R, CT::Data A, bool REVERSE, bool SKIP>
		Count ForEachDeepInner(TFunctor<R(A)>&&) const;
	
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
	
		NOD() const Block GetMember(const Member&) const;
		NOD() Block GetMember(const Member&);
		NOD() const Block GetMember(TMeta, Offset = 0) const;
		NOD() Block GetMember(TMeta, Offset = 0);
		NOD() const Block GetMember(DMeta, Offset = 0) const;
		NOD() Block GetMember(DMeta, Offset = 0);
		NOD() const Block GetMember(std::nullptr_t, Offset = 0) const;
		NOD() Block GetMember(std::nullptr_t, Offset = 0);
	
		NOD() Block GetBaseMemory(DMeta, const Base&) const;
		NOD() Block GetBaseMemory(DMeta, const Base&);
		NOD() Block GetBaseMemory(const Base&) const;
		NOD() Block GetBaseMemory(const Base&);
	
		template<CT::Data T, CT::Deep WRAPPER>
		NOD() bool Mutate();
		template<CT::Deep WRAPPER>
		NOD() bool Mutate(DMeta);

		constexpr void MakeMissing() noexcept;
		constexpr void MakeStatic() noexcept;
		constexpr void MakeConstant() noexcept;
		constexpr void MakeTypeConstrained() noexcept;
		constexpr void MakeOr() noexcept;
		constexpr void MakeAnd() noexcept;
		constexpr void MakePast() noexcept;
		constexpr void MakeFuture() noexcept;
		constexpr void MakeSparse() noexcept;
	
		Count Copy(Block&, bool allocate = false) const;
		Count Clone(Block&) const;
	
		NOD() bool CompareMembers(const Block&, Count& compared) const;
		NOD() bool CompareStates(const Block&) const noexcept;
		NOD() bool Compare(const Block&, bool resolve = true) const;
	
		template<bool CREATE = false>
		void Allocate(const Count&);
		
		void Shrink(Count);
	
		#if LANGULUS_FEATURE(ZLIB)
			Size Compress(Block&, Compression = Compression::Default) const;
			Size Decompress(Block&) const;
		#endif

		Size Encrypt(Block&, const Hash*, const Count&) const;
		Size Decrypt(Block&, const Hash*, const Count&) const;
	
		NOD() Hash GetHash() const;
	
		template<CT::Data T>
		NOD() Index Find(const T&, const Index& = Index::Front) const;
	
		template<CT::Data T>
		NOD() Index FindDeep(const T&, const Index& = Index::Front) const;
		NOD() Index FindRTTI(const Block&, const Index& = Index::Front) const;
	
		Count Gather(Block&, Index = Index::Front) const;
		Count Gather(Block&, Phase, Index = Index::Front) const;
	
		template<CT::Data T>
		void Swap(Offset, Offset);
		template<CT::Data T>
		void Swap(Index, Index);
	
		template<CT::Data T, bool MUTABLE = true, CT::Data WRAPPER>
		Count Emplace(T&&, const Index& = Index::Back);
	
		template<CT::Data T, bool MUTABLE = true, CT::Data WRAPPER>
		Count Insert(const T*, Count = 1, const Index& = Index::Back);
		Count InsertBlock(const Block&, const Index& = Index::Back);
		Count InsertBlock(Block&&, const Index& = Index::Back);
	
		template<bool ALLOW_CONCAT = true, bool ALLOW_DEEPEN = true, CT::Data T, CT::Deep WRAPPER = Any>
		Count SmartPush(T&, DataState = {}, Index = Index::Back);
		template<bool ALLOW_CONCAT = true, bool ALLOW_DEEPEN = true, CT::Data T, CT::Deep WRAPPER = Any>
		Count SmartPush(T&&, DataState = {}, Index = Index::Back);
	
		template<CT::Deep T, bool MOVE_STATE = true>
		T& Deepen();
	
		template<CT::Data T, bool MUTABLE = true, CT::Data WRAPPER>
		Count Merge(const T*, Count = 1, const Index& = Index::Back);
		Count MergeBlock(const Block&, const Index& = Index::Back);
	
		template<CT::Data T>
		Count Remove(const T*, Count = 1, const Index& = Index::Front);
		Count RemoveIndex(const Index&, Count = 1);
		Count RemoveIndex(Offset, Count = 1);
		Count RemoveIndexDeep(Offset);
	
		Block& Trim(Offset);
	
		NOD() constexpr Index Constrain(const Index&) const noexcept;
	
		template<CT::Data T>
		NOD() Index ConstrainMore(const Index&) const noexcept;
	
		template<CT::Data T>
		NOD() Index GetIndexMax() const noexcept requires CT::Sortable<T>;
	
		template<CT::Data T>
		NOD() Index GetIndexMin() const noexcept requires CT::Sortable<T>;
	
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
		template<CT::Data T>
		NOD() bool Is() const;
		
		NOD() Block ReinterpretAs(const Block&) const;
	
		void Clear();
		void Reset();

	protected:
		template<bool CREATE = false>
		void AllocateInner(const Count&);

		Size RequestByteSize(const Count&) const noexcept;
	
		template<CT::Data T>
		void EmplaceInner(T&&, const Offset&);
		template<CT::Data T>
		void InsertInner(const T*, const Count&, const Offset&);

		static void CopyMemory(const void*, void*, const Size&) noexcept;
		static void MoveMemory(const void*, void*, const Size&) noexcept;
		static void FillMemory(void*, Byte, const Size&) noexcept;
		NOD() static int CompareMemory(const void*, const void*, const Size&) noexcept;
		
		constexpr void ClearInner() noexcept;
		constexpr void ResetMemory() noexcept;
		template<bool TYPED>
		constexpr void ResetState() noexcept;
	
		void Reference(const Count&) const noexcept;
		void Reference(const Count&) noexcept;
		void Keep() const noexcept;
		void Keep() noexcept;
		
		template<bool DESTROY>
		bool Dereference(const Count&);
		bool Free();
	
		void CallDefaultConstructors(const Count&);
		void CallCopyConstructors(const Count&, const Block&);

		void CallUnknownMoveConstructors(Count, Block&&);
		template<class T>
		void CallKnownMoveConstructors(Count, Block&&);

		void CallUnknownDestructors();
		template<class T>
		void CallKnownDestructors();
	
		Size AllocateRegion(const Block&, const Index&, Block&);
	
		template<class FROM, class TO>
		static Count ConvertSymmetric(const Block&, Block&);
		template<class FROM>
		static Count ConvertDataBatched(const Block&, Block&, const Index&);
		template<class FROM>
		static Count ConvertToTextBlock(const Block&, Block&);
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

#include "Block.inl"
