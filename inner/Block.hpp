#pragma once
#include "Integration.hpp"
#include "DataState.hpp"

namespace Langulus::Anyness
{
	
	/// Predeclarations																			
	class Any;
	template<ReflectedData T>
	class TAny;
	
	class Map;
	template<ReflectedData K, ReflectedData V>
	class TMap;
	template<ReflectedData K, ReflectedData V>
	class THashMap;
	
	class Set;
	template<ReflectedData T>
	class TSet;
	
	class Bytes;
	class Text;
	class Path;
	
	template<ReflectedData T>
	class TOwned;
	template<ReflectedData T, bool REFERENCED>
	class TPointer;

	/// Compression types, analogous to zlib's											
	enum class Compression {
		Nothing = 0,
		Fastest = 1,
		Balanced = 5,
		Smallest = 9,
		
		Default = Fastest
	};
	
	/// Data can have a temporal phase														
	enum class Phase : int {
		Past = -1,
		Now = 0,
		Future = 1
	};
	
	
	namespace Inner
	{
		
		///																							
		///	BLOCK																					
		///																							
		///	Wraps an allocated memory block; acts as base to all containers.	
		///	This is an inner structure, that doesn't reference any memory,		
		/// only provides the functionality to do so. Avoid handling Block		
		/// instances unless you know exactly what you're doing.						
		///																							
		class Block {
			LANGULUS(DEEP);
		public:	
			constexpr Block() noexcept = default;
			constexpr Block(const Block&) noexcept = default;
			
			Block(Block&&) noexcept;
			constexpr Block(const DataState&, DMeta) noexcept;
			constexpr Block(const DataState&, DMeta, Count, const void*) noexcept;
			constexpr Block(const DataState&, DMeta, Count, void*) noexcept;
	
			template<ReflectedData T>
			NOD() static Block From(T) requires Sparse<T>;
			template<ReflectedData T>
			NOD() static Block From(T, Count) requires Sparse<T>;
			template<ReflectedData T>
			NOD() static Block From(T&) requires Dense<T>;
			template<ReflectedData T>
			NOD() static Block From();

			constexpr Block& operator = (const Block&) noexcept;
			Block& operator = (Block&&) noexcept;
			
			Block& TakeJurisdiction();
			void Optimize();
	
		public:
			void SetType(DMeta, bool);
			template<ReflectedData T>
			void SetType(bool);
	
			void SetPhase(const Phase) noexcept;
			void SetState(DataState) noexcept;
	
			NOD() constexpr DMeta GetMeta() const noexcept;
			NOD() constexpr Count GetCount() const noexcept;
			NOD() Count GetCountDeep() const noexcept;
			NOD() Count GetCountElementsDeep() const noexcept;
			NOD() constexpr Count GetReserved() const noexcept;
			NOD() constexpr const Byte* GetBytes() const noexcept;
			NOD() constexpr Byte* GetBytes() noexcept;
			NOD() const void* const* GetPointers() const noexcept;
			NOD() void** GetPointers() noexcept;
			NOD() constexpr bool IsAllocated() const noexcept;
			NOD() constexpr bool IsPast() const noexcept;
			NOD() constexpr bool IsFuture() const noexcept;
			NOD() constexpr bool IsNow() const noexcept;
			NOD() constexpr bool IsMissing() const noexcept;
			NOD() bool IsMissingDeep() const;
			NOD() constexpr bool IsUntyped() const noexcept;
			NOD() constexpr bool IsTypeConstrained() const noexcept;
			NOD() constexpr bool IsPhased() const noexcept;
			NOD() constexpr bool IsEncrypted() const noexcept;
			NOD() constexpr bool IsCompressed() const noexcept;
			NOD() constexpr bool IsConstant() const noexcept;
			NOD() constexpr bool IsStatic() const noexcept;
			NOD() bool IsAbstract() const noexcept;
			NOD() bool IsConstructible() const noexcept;
			NOD() constexpr bool IsOr() const noexcept;
			NOD() constexpr bool IsEmpty() const noexcept;
			NOD() constexpr bool IsValid() const noexcept;
			NOD() constexpr bool IsInvalid() const noexcept;
			NOD() bool IsDense() const;
			NOD() bool IsSparse() const;
			NOD() bool IsDeep() const;
			NOD() constexpr Phase GetPhase() const noexcept;
			NOD() bool CanFitPhase(const Phase&) const noexcept;
			NOD() bool CanFitState(const Block&) const noexcept;
			NOD() Count GetSize() const noexcept;
			NOD() bool IsConcatable(const Block&) const noexcept;
	
			NOD() bool IsInsertable(DMeta) const noexcept;
	
			template<ReflectedData T>
			NOD() bool IsInsertable() const noexcept;
	
			NOD() Block GetToken() const noexcept;
			NOD() Stride GetStride() const noexcept;
			NOD() void* GetRaw() noexcept;
			NOD() const void* GetRaw() const noexcept;
			NOD() const void* GetRawEnd() const noexcept;
			template<ReflectedData T>
			NOD() T* GetRawAs() noexcept;
			template<ReflectedData T>
			NOD() const T* GetRawAs() const noexcept;
			template<ReflectedData T>
			NOD() const T* GetRawEndAs() const noexcept;
			NOD() constexpr DataState GetState() const noexcept;
			NOD() constexpr DataState GetUnconstrainedState() const noexcept;
	
			NOD() bool operator == (const Block&) const noexcept;
			NOD() bool operator != (const Block&) const noexcept;
	
			NOD() Byte* At(Offset = 0);
			NOD() const Byte* At(Offset = 0) const;
	
			template<ReflectedData T>
			NOD() decltype(auto) Get(Offset = 0, Offset = 0);
	
			template<ReflectedData T>
			NOD() decltype(auto) Get(Offset = 0, Offset = 0) const;
	
			template<ReflectedData T>
			NOD() decltype(auto) As(Offset = 0);
	
			template<ReflectedData T>
			NOD() decltype(auto) As(Offset = 0) const;
	
			template<ReflectedData T>
			NOD() decltype(auto) As(Index);
	
			template<ReflectedData T>
			NOD() decltype(auto) As(Index) const;
	
			template<ReflectedData T, bool FATAL_FAILURE = true>
			NOD() T AsCast(Index) const;
			template<ReflectedData T, bool FATAL_FAILURE = true>
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
	
			template<bool MUTABLE = true, class FUNCTION>
			Count ForEach(FUNCTION&&);
			template<class FUNCTION>
			Count ForEach(FUNCTION&&) const;
	
			template<bool MUTABLE = true, class FUNCTION>
			Count ForEachRev(FUNCTION&&);
			template<class FUNCTION>
			Count ForEachRev(FUNCTION&&) const;
	
			template<bool SKIP_DEEP_OR_EMPTY = true, bool MUTABLE = true, class FUNCTION>
			Count ForEachDeep(FUNCTION&&);
			template<bool SKIP_DEEP_OR_EMPTY = true, class FUNCTION>
			Count ForEachDeep(FUNCTION&&) const;
	
			template<bool SKIP_DEEP_OR_EMPTY = true, bool MUTABLE = true, class FUNCTION>
			Count ForEachDeepRev(FUNCTION&&);
			template<bool SKIP_DEEP_OR_EMPTY = true, class FUNCTION>
			Count ForEachDeepRev(FUNCTION&&) const;
	
		private:
			template<class RETURN, ReflectedData ARGUMENT, bool REVERSE, bool MUTABLE = true>
			Count ForEachInner(TFunctor<RETURN(ARGUMENT)>&&);
			template<class RETURN, ReflectedData ARGUMENT, bool REVERSE>
			Count ForEachInner(TFunctor<RETURN(ARGUMENT)>&&) const;
	
			template<class RETURN, ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY, bool MUTABLE = true>
			Count ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&&);
			template<class RETURN, ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY>
			Count ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&&) const;
	
			/// This function declaration is used to decompose a lambda				
			/// You can use it to extract the argument type of the lambda, using	
			/// decltype on the return type. Useful for template deduction in the
			/// ForEach functions above, purely for convenience						
			template<typename RETURN, typename FUNCTION, typename ARGUMENT>
			ARGUMENT GetLambdaArgument(RETURN(FUNCTION::*)(ARGUMENT) const) const;
	
			NOD() Block CropInner(Offset, Count);
	
		public:
			NOD() bool Owns(const void*) const noexcept;
			NOD() bool CheckJurisdiction() const;
			NOD() bool CheckUsage() const;
			NOD() RefCount GetBlockReferences() const;
			NOD() Block Crop(Offset, Count);
			NOD() Block Crop(Offset, Count) const;
	
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
	
			NOD() Block Decay(DMeta) const;
	
			template<ReflectedData T>
			NOD() Block Decay() const;
	
			NOD() bool Mutate(DMeta);
	
			template<ReflectedData T>
			NOD() bool Mutate();
	
			void ToggleState(const DataState&, bool toggle = true);
			Block& MakeMissing();
			Block& MakeStatic();
			Block& MakeConstant();
			Block& MakeTypeConstrained();
			Block& MakeOr();
			Block& MakeAnd();
			Block& MakeLeft();
			Block& MakeRight();
	
			Count Copy(Block&, bool allocate = false) const;
			Count Clone(Block&) const;
	
			NOD() bool CompareMembers(const Block&, Count& compared) const;
			NOD() bool CompareStates(const Block&) const noexcept;
			NOD() bool Compare(const Block&, bool resolve = true) const;
	
			void Allocate(Count, bool construct = false, bool setcount = false);
	
			template<ReflectedData T>
			void Allocate(Count, bool construct = false, bool setcount = false);
	
			void Extend(Count, bool construct = false, bool setcount = false);
			void Shrink(Count);
	
			Stride Compress(Block&, Compression = Compression::Default) const;
			Stride Decompress(Block&) const;
	
			Stride Encrypt(Block&, const Hash*, Count) const;
			Stride Decrypt(Block&, const Hash*, Count) const;
	
			NOD() Hash GetHash() const;
	
			template<ReflectedData T>
			NOD() Index Find(const T&, const Index& = uiFront) const;
	
			template<ReflectedData T>
			NOD() Index FindDeep(const T&, const Index& = uiFront) const;
			NOD() Index FindRTTI(const Block&, const Index& = uiFront) const;
	
			Count Gather(Block&, Index = uiFront) const;
			Count Gather(Block&, Phase, Index = uiFront) const;
	
			template<ReflectedData T>
			void Swap(Offset, Offset);
	
			template<ReflectedData T>
			void Swap(Index, Index);
	
			template<ReflectedData T, bool MUTABLE = true>
			Count Emplace(T&&, const Index& = uiBack);
	
			template<ReflectedData T, bool MUTABLE = true>
			Count Insert(const T*, Count = 1, const Index& = uiBack);
			Count InsertBlock(const Block&, const Index& = uiBack);
			Count InsertBlock(Block&&, const Index& = uiBack);
	
			template<ReflectedData T>
			Count SmartPush(const T&, DataState = {}
				, bool attemptConcat = true
				, bool attemptDeepen = true
				, Index = uiBack
			);
	
			template<ReflectedData T>
			T& Deepen(bool moveState = true);
	
			template<ReflectedData T>
			Block& operator << (const T&);
	
			template<ReflectedData T>
			Block& operator << (T&);
	
			template<ReflectedData T>
			Block& operator << (T&&);
	
			template<ReflectedData T>
			Block& operator >> (const T&);
	
			template<ReflectedData T>
			Block& operator >> (T&); 
	
			template<ReflectedData T>
			Block& operator >> (T&&);
	
			template<ReflectedData T, bool MUTABLE = true>
			Count Merge(const T*, Count = 1, const Index& = uiBack);
			Count MergeBlock(const Block&, const Index& = uiBack);
	
			template<ReflectedData T>
			Block& operator <<= (const T&);
	
			template<ReflectedData T>
			Block& operator >>= (const T&);
	
			template<ReflectedData T>
			Count Remove(const T*, Count = 1, const Index& = uiFront);
			Count RemoveIndex(const Index&, Count = 1);
			Count RemoveIndex(Offset, Count = 1);
			Count RemoveIndexDeep(Offset);
	
			Block& Trim(Offset);
	
			NOD() constexpr Index Constrain(const Index&) const noexcept;
	
			template<ReflectedData T>
			NOD() Index ConstrainMore(const Index&) const noexcept;
	
			template<ReflectedData T>
			NOD() Index GetIndexMax() const noexcept requires Sortable<T>;
	
			template<ReflectedData T>
			NOD() Index GetIndexMin() const noexcept requires Sortable<T>;
	
			template<ReflectedData T>
			NOD() Index GetIndexMode(Count&) const noexcept;
	
			template<ReflectedData T>
			void Sort(const Index&) noexcept;
	
			NOD() bool CanFit(DMeta) const;
			NOD() bool CanFit(const Block&) const;
	
			template<ReflectedData T>
			NOD() bool CanFit() const;
	
			NOD() bool InterpretsAs(DMeta) const;
	
			template<ReflectedData T>
			NOD() bool InterpretsAs() const;
	
			NOD() bool InterpretsAs(DMeta, Count) const;
	
			template<ReflectedData T>
			NOD() bool InterpretsAs(Count) const;
	
			NOD() bool Is(DMeta) const;
	
			template<ReflectedData T>
			NOD() bool Is() const;
	
		protected:
			constexpr void ClearInner() noexcept;
			constexpr void ResetInner() noexcept;
	
			RefCount ReferenceBlock(RefCount times);
			RefCount Keep();
			RefCount Free();
	
			void CallDefaultConstructors();
			void CallCopyConstructors(const Block&);
			void CallMoveConstructors(Block&&);
			void CallDestructors();
	
			Stride AllocateRegion(const Block&, const Index&, Block&);
	
			template<class FROM, class TO>
			static Count ConvertSymmetric(const Block&, Block&);
			template<class FROM>
			static Count ConvertDataBatched(const Block&, Block&, const Index&);
			template<class FROM>
			static Count ConvertToTextBlock(const Block&, Block&);

			// The raw pointer to the first element inside the memory block
			#if LANGULUS_DEBUG()
				union { 
					char* mRawChar;
					void* mRaw = nullptr;
				};
			#else
				void* mRaw = nullptr;
			#endif
	
			// Type of the instances inside the memory block					
			DMeta mType {};
			// Number of written instances inside memory block					
			Count mCount {};
			// Number of allocated instances in the memory block				
			Count mReserved {};
			// The data state																
			DataState mState {DataState::Default};
		};

	} // namespace Langulus::Anyness::Inner
} // namespace Langulus::Anyness

#include "Block.inl"
