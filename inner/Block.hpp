#pragma once
#include "TElement.hpp"

namespace Langulus::Anyness
{
	
	/// Predeclarations																			
	class Block;
	
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
	
	
	namespace Inner {
		
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
			NOD() DataID GetDataID() const noexcept;
			NOD() Count GetDataSwitch() const noexcept;
	
			void SetDataID(DataID, bool);
			void SetDataID(DMeta, bool);
	
			template<ReflectedData T>
			void SetDataID(bool);
	
			void SetPhase(const Phase) noexcept;
			void SetState(DataState) noexcept;
	
			NOD() const ReflectData* GetDescriptor() const noexcept;
			NOD() constexpr DMeta GetMeta() const noexcept;
			NOD() constexpr Count GetCount() const noexcept { return mCount; }
			NOD() Count GetCountDeep() const noexcept;
			NOD() Count GetCountElementsDeep() const noexcept;
			NOD() constexpr Count GetReserved() const noexcept;
			NOD() PC_LEAKSAFETY const pcbyte* GetBytes() const noexcept;
			NOD() PC_LEAKSAFETY pcbyte* GetBytes() noexcept;
			NOD() const void* const* GetPointers() const noexcept;
			NOD() void** GetPointers() noexcept;
			NOD() constexpr bool IsAllocated() const noexcept;
			NOD() constexpr bool IsLeft() const noexcept;
			NOD() constexpr bool IsRight() const noexcept;
			NOD() constexpr bool IsNeutral() const noexcept;
			NOD() constexpr bool IsMissing() const noexcept;
			NOD() bool IsMissingDeep() const;
			NOD() constexpr bool IsUntyped() const noexcept;
			NOD() constexpr bool IsTypeConstrained() const noexcept;
			NOD() constexpr bool IsPolar() const noexcept;
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
			NOD() constexpr Polarity GetPolarity() const noexcept;
			NOD() bool CanFitPolarity(const Polarity&) const noexcept;
			NOD() bool CanFitState(const Block&) const noexcept;
			NOD() pcptr GetSize() const noexcept;
			NOD() bool IsConcatable(const Block&) const noexcept;
	
			NOD() bool IsInsertable(DMeta) const noexcept;
	
			template<RTTI::ReflectedData T>
			NOD() bool IsInsertable() const noexcept;
	
			NOD() LiteralText GetToken() const noexcept;
			NOD() pcptr GetStride() const noexcept;
			NOD() void* GetRaw() noexcept;
			NOD() const void* GetRaw() const noexcept;
			NOD() const void* GetRawEnd() const noexcept;
			template<RTTI::ReflectedData T>
			NOD() T* GetRawAs() noexcept;
			template<RTTI::ReflectedData T>
			NOD() const T* GetRawAs() const noexcept;
			template<RTTI::ReflectedData T>
			NOD() const T* GetRawEndAs() const noexcept;
			NOD() constexpr DState GetState() const noexcept;
			NOD() constexpr DState GetUnconstrainedState() const noexcept;
	
			NOD() bool operator == (const Block&) const noexcept;
			NOD() bool operator != (const Block&) const noexcept;
	
			NOD() pcbyte* At(pcptr = 0);
			NOD() const pcbyte* At(pcptr = 0) const;
	
			template<RTTI::ReflectedData T>
			NOD() decltype(auto) Get(pcptr = 0, pcptr = 0);
	
			template<RTTI::ReflectedData T>
			NOD() decltype(auto) Get(pcptr = 0, pcptr = 0) const;
	
			template<RTTI::ReflectedData T>
			NOD() decltype(auto) As(pcptr = 0);
	
			template<RTTI::ReflectedData T>
			NOD() decltype(auto) As(pcptr = 0) const;
	
			template<RTTI::ReflectedData T>
			NOD() decltype(auto) As(Index);
	
			template<RTTI::ReflectedData T>
			NOD() decltype(auto) As(Index) const;
	
			template<RTTI::ReflectedData T, bool FATAL_FAILURE = true>
			NOD() T AsCast(Index) const;
			template<RTTI::ReflectedData T, bool FATAL_FAILURE = true>
			NOD() T AsCast() const;
	
			NOD() Block GetElementDense(pcptr);
			NOD() const Block GetElementDense(pcptr) const;
	
			NOD() Block GetElementResolved(pcptr);
			NOD() const Block GetElementResolved(pcptr) const;
	
			NOD() Block GetElement(pcptr) noexcept;
			NOD() const Block GetElement(pcptr) const noexcept;
	
			NOD() Block* GetBlockDeep(pcptr) noexcept;
			NOD() const Block* GetBlockDeep(pcptr) const noexcept;
	
			NOD() Block GetElementDeep(pcptr) noexcept;
			NOD() const Block GetElementDeep(pcptr) const noexcept;
	
			pcptr ForEachElement(TFunctor<bool(const Block&)>&&) const;
			pcptr ForEachElement(TFunctor<bool(Block&)>&&);
			pcptr ForEachElement(TFunctor<void(const Block&)>&&) const;
			pcptr ForEachElement(TFunctor<void(Block&)>&&);
	
			template<bool MUTABLE = true, class FUNCTION> pcptr ForEach(FUNCTION&&);
			template<class FUNCTION> pcptr ForEach(FUNCTION&&) const;
	
			template<bool MUTABLE = true, class FUNCTION> pcptr ForEachRev(FUNCTION&&);
			template<class FUNCTION> pcptr ForEachRev(FUNCTION&&) const;
	
			template<bool SKIP_DEEP_OR_EMPTY = true, bool MUTABLE = true, class FUNCTION>
			pcptr ForEachDeep(FUNCTION&&);
			template<bool SKIP_DEEP_OR_EMPTY = true, class FUNCTION>
			pcptr ForEachDeep(FUNCTION&&) const;
	
			template<bool SKIP_DEEP_OR_EMPTY = true, bool MUTABLE = true, class FUNCTION>
			pcptr ForEachDeepRev(FUNCTION&&);
			template<bool SKIP_DEEP_OR_EMPTY = true, class FUNCTION>
			pcptr ForEachDeepRev(FUNCTION&&) const;
	
		private:
			template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE, bool MUTABLE = true>
			pcptr ForEachInner(TFunctor<RETURN(ARGUMENT)>&&);
			template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE>
			pcptr ForEachInner(TFunctor<RETURN(ARGUMENT)>&&) const;
	
			template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY, bool MUTABLE = true>
			pcptr ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&&);
			template<class RETURN, RTTI::ReflectedData ARGUMENT, bool REVERSE, bool SKIP_DEEP_OR_EMPTY>
			pcptr ForEachDeepInner(TFunctor<RETURN(ARGUMENT)>&&) const;
	
			/// This function declaration is used to decompose a lambda				
			/// You can use it to extract the argument type of the lambda, using	
			/// decltype on the return type. Useful for template deduction in the
			/// ForEach functions above, purely for convenience						
			template<typename RETURN, typename FUNCTION, typename ARGUMENT>
			ARGUMENT GetLambdaArgument(RETURN(FUNCTION::*)(ARGUMENT) const) const;
	
			NOD() Block CropInner(pcptr, pcptr);
	
		public:
			NOD() bool Owns(const void*) const noexcept;
			NOD() bool CheckJurisdiction() const;
			NOD() bool CheckUsage() const;
			NOD() pcref GetBlockReferences() const;
			NOD() Block Crop(pcptr, pcptr);
			NOD() Block Crop(pcptr, pcptr) const;
	
			NOD() const Block GetMember(const Memory::LinkedMember&) const;
			NOD() Block GetMember(const Memory::LinkedMember&);
			NOD() const Block GetMember(TMeta, pcptr = 0) const;
			NOD() Block GetMember(TMeta, pcptr = 0);
			NOD() const Block GetMember(DMeta, pcptr = 0) const;
			NOD() Block GetMember(DMeta, pcptr = 0);
			NOD() const Block GetMember(std::nullptr_t, pcptr = 0) const;
			NOD() Block GetMember(std::nullptr_t, pcptr = 0);
	
			NOD() Block GetBaseMemory(DMeta, const Memory::LinkedBase&) const;
			NOD() Block GetBaseMemory(DMeta, const Memory::LinkedBase&);
			NOD() Block GetBaseMemory(const Memory::LinkedBase&) const;
			NOD() Block GetBaseMemory(const Memory::LinkedBase&);
	
			NOD() Block Decay(DMeta) const;
	
			template<RTTI::ReflectedData T>
			NOD() Block Decay() const;
	
			NOD() bool Mutate(DMeta);
	
			template<RTTI::ReflectedData T>
			NOD() bool Mutate();
	
			void ToggleState(const DState&, bool toggle = true);
			Block& MakeMissing();
			Block& MakeStatic();
			Block& MakeConstant();
			Block& MakeTypeConstrained();
			Block& MakeOr();
			Block& MakeAnd();
			Block& MakeLeft();
			Block& MakeRight();
	
			pcptr Copy(Block&, bool allocate = false) const;
			pcptr Clone(Block&) const;
	
			NOD() bool CompareMembers(const Block&, pcptr& compared) const;
			NOD() bool CompareStates(const Block&) const noexcept;
			NOD() bool Compare(const Block&, bool resolve = true) const;
	
			void Allocate(pcptr, bool construct = false, bool setcount = false);
	
			template<RTTI::ReflectedData T>
			void Allocate(pcptr count, bool construct = false, bool setcount = false);
	
			void Extend(pcptr, bool construct = false, bool setcount = false);
			void Shrink(pcptr);
	
			pcptr Compress(Block&, Compression = Compression::Default) const;
			pcptr Decompress(Block&) const;
	
			pcptr Encrypt(Block&, const pcu32* keys, pcptr key_count) const;
			pcptr Decrypt(Block&, const pcu32* keys, pcptr key_count) const;
	
			NOD() Hash GetHash() const;
	
			template<RTTI::ReflectedData T>
			NOD() Index Find(const T&, const Index& = uiFront) const;
	
			template<RTTI::ReflectedData T>
			NOD() Index FindDeep(const T&, const Index& = uiFront) const;
			NOD() Index FindRTTI(const Block&, const Index& = uiFront) const;
	
			pcptr Gather(Block&, Index = uiFront) const;
			pcptr Gather(Block&, Polarity, Index = uiFront) const;
	
			template<RTTI::ReflectedData T>
			void Swap(pcptr, pcptr);
	
			template<RTTI::ReflectedData T>
			void Swap(Index, Index);
	
			template<RTTI::ReflectedData T, bool MUTABLE = true>
			pcptr Emplace(T&&, const Index& = uiBack);
	
			template<RTTI::ReflectedData T, bool MUTABLE = true>
			pcptr Insert(const T*, pcptr = 1, const Index& = uiBack);
			pcptr InsertBlock(const Block&, const Index& = uiBack);
			pcptr InsertBlock(Block&&, const Index& = uiBack);
	
			template<RTTI::ReflectedData T>
			pcptr SmartPush(const T&, DState = {}
				, bool attemptConcat = true
				, bool attemptDeepen = true
				, Index = uiBack
			);
	
			template<RTTI::ReflectedData T>
			T& Deepen(bool moveState = true);
	
			template<RTTI::ReflectedData T>
			Block& operator << (const T&);
	
			template<RTTI::ReflectedData T>
			Block& operator << (T&);
	
			template<RTTI::ReflectedData T>
			Block& operator << (T&&);
	
			template<RTTI::ReflectedData T>
			Block& operator >> (const T&);
	
			template<RTTI::ReflectedData T>
			Block& operator >> (T&); 
	
			template<RTTI::ReflectedData T>
			Block& operator >> (T&&);
	
			template<RTTI::ReflectedData T, bool MUTABLE = true>
			pcptr Merge(const T*, pcptr = 1, const Index& = uiBack);
			pcptr MergeBlock(const Block&, const Index& = uiBack);
	
			template<RTTI::ReflectedData T>
			Block& operator <<= (const T&);
	
			template<RTTI::ReflectedData T>
			Block& operator >>= (const T&);
	
			template<RTTI::ReflectedData T>
			pcptr Remove(const T*, pcptr = 1, const Index& = uiFront);
			pcptr RemoveIndex(const Index&, pcptr = 1);
			pcptr RemoveIndex(pcptr, pcptr = 1);
			pcptr RemoveIndexDeep(pcptr);
	
			Block& Trim(pcptr);
	
			NOD() constexpr Index Constrain(const Index&) const noexcept;
	
			template<RTTI::ReflectedData T>
			NOD() Index ConstrainMore(const Index&) const noexcept;
	
			template<RTTI::ReflectedData T>
			NOD() Index GetIndexMax() const noexcept requires Sortable<T>;
	
			template<RTTI::ReflectedData T>
			NOD() Index GetIndexMin() const noexcept requires Sortable<T>;
	
			template<RTTI::ReflectedData T>
			NOD() Index GetIndexMode(pcptr&) const noexcept;
	
			template<RTTI::ReflectedData T>
			void Sort(const Index&) noexcept;
	
			NOD() bool CanFit(DMeta) const;
			NOD() bool CanFit(const Block&) const;
	
			template<RTTI::ReflectedData T>
			NOD() bool CanFit() const;
	
			NOD() bool InterpretsAs(DMeta) const;
	
			template<RTTI::ReflectedData T>
			NOD() bool InterpretsAs() const;
	
			NOD() bool InterpretsAs(DMeta, pcptr) const;
	
			template<RTTI::ReflectedData T>
			NOD() bool InterpretsAs(pcptr) const;
	
			NOD() bool Is(DataID) const;
	
			template<RTTI::ReflectedData T>
			NOD() bool Is() const;
	
		protected:
			constexpr void ClearInner() noexcept;
			constexpr void ResetInner() noexcept;
	
			pcref ReferenceBlock(pcref times);
			pcref Keep();
			pcref Free();
	
			void CallDefaultConstructors();
			void CallCopyConstructors(const Block&);
			void CallMoveConstructors(Block&&);
			void CallDestructors();
	
			pcptr AllocateRegion(const Block&, const Index&, Block&);
	
			template<class FROM, class TO>
			static pcptr ConvertSymmetric(const Block&, Block&);
			template<class FROM>
			static pcptr ConvertDataBatched(const Block&, Block&, const Index&);
			template<class FROM>
			static pcptr ConvertToTextBlock(const Block&, Block&);
	
			static_assert(sizeof(pcptr) <= std::numeric_limits<pcu8>::max(), 
				"Pointer type is too big");
	
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
			DMeta mType = nullptr;
			// Number of written instances inside memory block					
			pcptr mCount = 0;
			// Number of allocated instances in the memory block				
			pcptr mReserved = 0;
			// The data state																
			DState mState = DState::Default;
		};

	} // namespace Langulus::Anyness::Inner
} // namespace Langulus::Anyness

#include "Block.inl"
