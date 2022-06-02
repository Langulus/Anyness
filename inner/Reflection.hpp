///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
/// Include this only when building standalone											
#pragma once
#include "DataState.hpp"

/// You can mark types as deep by using LANGULUS(DEEP) true / false inside		
/// class, but to fit into CT::Deep concept, your type must also inherit Block
#define LANGULUS_DEEP() public: static constexpr bool CTTI_Deep = 

/// You can mark types as POD (Plain Old Data) by using LANGULUS(POD) true or	
/// false inside class. POD types are batch-copied via memcpy, and do not		
/// call constructors or destructors when contained (unless nullifiable)		
#define LANGULUS_POD() public: static constexpr bool CTTI_POD = 

/// You can mark types as nullifiable, by using LANGULUS(NULLIFIABLE) true or	
/// false inside class. Nullifiable classes are batch-constructed and			
/// batch-destroyed via memset(0)															
#define LANGULUS_NULLIFIABLE() public: static constexpr bool CTTI_Nullifiable = 

/// You can make types concretizable, by using LANGULUS(CONCRETIZABLE) Type	
/// When dynamically creating objects of your type, the most concrete type		
/// in the chain will be used instead														
#define LANGULUS_CONCRETIZABLE() public: using CTTI_Concretizable = 

/// You can define an allocation page (number of elements) by using				
/// LANGULUS(ALLOCATION_PAGE) X. When allocating memory for your type, X		
/// will be the minimum amount of elements to allocate, aligned to the			
/// nearest upper power-of-two amount of bytes. By default, allocation page	
/// size	is never lower than LANGULUS(ALIGN)												
#define LANGULUS_ALLOCATION_PAGE() public: constexpr Count CTTI_AllocationPage = 

/// Make a type abstract																		
#define LANGULUS_ABSTRACT() public: static constexpr bool CTTI_Abstract = 

/// Reflect a list of members																	
#define LANGULUS_MEMBERS(...) public: using CTTI_Members = ::Langulus::TTypeList<__VA_ARGS__>

/// Reflect a list of bases																	
#define LANGULUS_BASES(...) public: using CTTI_Bases = ::Langulus::TTypeList<__VA_ARGS__>

/// Reflect a list of verbs																	
#define LANGULUS_VERBS(...) public: using CTTI_Verbs = ::Langulus::TTypeList<__VA_ARGS__>


namespace Langulus::Anyness
{

	/// A simple request for allocating memory											
	/// It is used as optimization to avoid divisions by stride						
	struct AllocationRequest {
		Size mByteSize;
		Count mElementCount;
	};

   /// Round to the upper power-of-two														
	///	@tparam SAFE - set to true if you want it to throw on overflow			
	///	@tparam T - the unsigned integer type (deducible)							
	///	@param x - the unsigned integer to round up									
	///	@return the closest upper power-of-two to x									
   template<bool SAFE = false, CT::Unsigned T>
	constexpr T Roof2(const T& x) noexcept(!SAFE) {
		T n = x;
		--n;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		if constexpr (sizeof(T) > 1)
			n |= n >> 8;
		if constexpr (sizeof(T) > 2)
			n |= n >> 16;
		if constexpr (sizeof(T) > 4)
			n |= n >> 32;
		if constexpr (sizeof(T) > 8)
			TODO();

		if constexpr (SAFE) {
			if (x != 0 && n == ::std::numeric_limits<T>::max())
				Throw<Except::Overflow>("Roof2 overflowed");
		}

		++n;
		return n;
	}

	/// Get the minimum allocation page size of the type (in bytes)				
	/// This guarantees two things:															
	///	1. The byte size is always a power-of-two										
	///	2. The byte size is never smaller than LANGULUS(ALIGN)					
	template<class T>
	constexpr Size GetAllocationPageOf() SAFETY_NOEXCEPT() {
		if constexpr (requires {{Decay<T>::CTTI_AllocationPage} -> CT::Same<Size>;}) {
			constexpr Size candidate = Decay<T>::CTTI_AllocationPage * sizeof(T);
			if constexpr (candidate < Alignment)
				return Alignment;
			else 
				return Roof2<LANGULUS(SAFE)>(candidate);
		}
		else if constexpr (sizeof(T) < Alignment)
			return Alignment;
		else 
			return Roof2<LANGULUS(SAFE)>(sizeof(T));
	}

	
	///																								
	///	These methods are sought in each reflected type								
	///																								
	/// The default constructor, wrapped in a lambda expression if available	
	/// Takes a pointer for a placement-new expression									
	using FDefaultConstruct = TFunctor<void(void*)>;

	/// The copy constructor, wrapped in a lambda expression if available		
	/// Takes a pointer for a placement-new expression, and a source				
	using FCopyConstruct = TFunctor<void(void*, const void*)>;

	/// The move constructor, wrapped in a lambda expression if available		
	/// Takes a pointer for a placement-new expression, and a source				
	using FMoveConstruct = TFunctor<void(void*, void*)>;

	/// The destructor, wrapped in a lambda expression									
	/// Takes the pointer to the instance for destruction								
	using FDestroy = TFunctor<void(void*)>;

	/// The cloner, wrapped in a lambda expression if available						
	/// Clone one instance to another														
	using FClone = TFunctor<void(const void*, void*)>;

	/// The == operator, wrapped in a lambda expression if available				
	/// Compares two instances for equality												
	using FCompare = TFunctor<bool(const void*, const void*)>;

	/// The = operator, wrapped in a lambda expression if available				
	/// Does a shallow copy from one instance to another								
	using FCopy = TFunctor<void(const void*, void*)>;

	/// The move-copy operator, wrapped in a lambda expression if available		
	/// Does a move-copy from one instance to another									
	using FMove = TFunctor<void(void*, void*)>;

	/// The class type function, wrapped in a lambda expression						
	/// Returns the typed memory block of the class instance							
	using FResolve = TFunctor<Block(const void*)>;

	/// The hash getter, wrapped in a lambda expression								
	/// Takes the pointer to the instance for hashing									
	/// Returns the hash																			
	using FHash = TFunctor<Hash(const void*)>;

	/// A custom verb dispatcher, wrapped in a lambda expression					
	/// Takes the pointer to the instance that will dispatch, and a verb			
	using FDispatch = TFunctor<void(void*, Flow::Verb&)>;
	using FVerb = FDispatch;


	///																								
	/// Used to reflect a member variable													
	/// You can reflect arrays of elements, tag members as traits, etc.			
	///																								
	struct Member {
		// Type of data																	
		DMeta mType {};
		// State of the data																
		DataState mState {};
		// Member offset. This is relative to the type it is offsetted		
		// in! If accessed through a derived type, that offset might		
		// be wrong! Type must be resolved first!									
		Offset mOffset {};
		// Number of elements in mData (in case of an array)					
		Count mCount {1};
		// Trait tag																		
		TMeta mTrait {};
		// Member token																	
		Token mName {};

	public:
		constexpr Member() noexcept = default;

		template<CT::Data OWNER, CT::Data DATA>
		NOD() static Member From(Offset, const Token& = {}, TMeta trait = {});

		NOD() constexpr bool operator == (const Member&) const noexcept;
		NOD() constexpr bool operator != (const Member&) const noexcept;
		
		template<CT::Data T>
		NOD() constexpr bool Is() const noexcept;
		
		template<CT::Data T>
		NOD() const T& As(const Byte*) const noexcept;
		template<CT::Data T>
		NOD() T& As(Byte*) const noexcept;
		
		NOD() constexpr const Byte* Get(const Byte*) const noexcept;
		NOD() constexpr Byte* Get(Byte*) const noexcept;
	};

	using MemberList = ::std::span<const Member>;

	
	///																								
	///	Used to reflect data capabilities												
	///																								
	struct Ability {
		// The verb ID																		
		VMeta mVerb {};
		// Address of function to call												
		FVerb mFunction {};
		
	public:		
		constexpr Ability() noexcept = default;

		NOD() constexpr bool operator == (const Ability&) const noexcept;
		NOD() constexpr bool operator != (const Ability&) const noexcept;

		template<CT::Dense T, CT::Dense VERB>
		NOD() static Ability From() noexcept;
	};

	using AbilityList = ::std::span<const Ability>;


	///																								
	///	Used to reflect a base for a type												
	///																								
	struct Base {
		// Type of the base																
		DMeta mType {};
		// CT::Number of bases that fit in the type								
		Count mCount {1};
		// Offset of the base, relative to the derived type					
		Offset mOffset {};
		// Used to map one type onto another										
		// Usually true when base completely fills the derived type			
		bool mBinaryCompatible {false};
		// Whether or not this base is considered an imposed base or not	
		// Basically, imposed bases are not serialized and don't act in	
		// distance computation or dispatching										
		// An imposed base can be added only manually							
		bool mImposed {false};

	public:
		constexpr Base() noexcept = default;

		NOD() constexpr bool operator == (const Base&) const noexcept;
		NOD() constexpr bool operator != (const Base&) const noexcept;

		template<CT::Dense T, CT::Dense BASE>
		NOD() static Base From() SAFETY_NOEXCEPT();

		template<class T, class BASE, Count COUNT>
		NOD() static Base Map() noexcept;
	};

	using BaseList = ::std::span<const Base>;

	namespace Inner
	{
		template<class DERIVED, class BASE>
		struct DBPair {
			NOD() static Base Get() noexcept {
				return Base::From<DERIVED, BASE>();
			}
		};
	}


	///																								
	///	Meta																						
	///																								
	/// Base for meta definitions																
	///																								
	struct Meta {
		// Each reflection primitive has a unique token, but that			
		// uniqueness is checked only if MANAGED_REFLECTION feature is		
		// enabled																			
		Token mToken;
		// Each reflection may or may not have some info						
		Token mInfo;
		// Original name of the type													
		Token mName;
		// Each reflected type has an unique hash									
		Hash mHash;

		template<CT::Data T>
		static constexpr Hash GetHash() noexcept;
		template<CT::Data T>
		static constexpr Token GetName() noexcept;
	};


	///																								
	///	Meta data																				
	///																								
	struct MetaData : public Meta {
		enum Distance : int {
			Infinite = ::std::numeric_limits<int>::max()
		};
		
		static constexpr Token DefaultToken = u8"udInvalid";
		
		// List of reflected members													
		MemberList mMembers {};
		// List of reflected abilities												
		AbilityList mAbilities {};
		// List of reflected bases														
		BaseList mBases {};
		// Default concretization														
		DMeta mConcrete {};
		// Dynamic producer of the type												
		// Types with producers can be created only via a verb				
		DMeta mProducer {};
		// True if reflected data is POD (optimization)							
		// POD data can be directly memcpy-ed, or binary-serialized			
		bool mIsPOD = false;
		// True if reflected data is nullifiable (optimization)				
		// Nullifiable data can be constructed AND destructed via			
		// memset(0) without hitting undefined behavior							
		bool mIsNullifiable = false;
		// If reflected type is abstract												
		bool mIsAbstract = false;
		// Type will be interpreted as a memory block and iterated			
		bool mIsDeep = false;
		// Size of the reflected type (in bytes)									
		Size mSize {};
		// Alignof (in bytes)															
		Size mAlignment {};
		// Minimal allocation, in bytes												
		Size mAllocationPage {};
		// Precomputed counts indexed by MSB (avoids division by stride)	
		Size mAllocationTable[sizeof(Size)*8];
		// File extensions used, separated by commas								
		Token mFileExtension {};

		// Default constructor wrapped in a lambda upon reflection			
		FDefaultConstruct mDefaultConstructor;
		// Copy constructor wrapped in a lambda upon reflection				
		FCopyConstruct mCopyConstructor;
		// Move constructor wrapped in a lambda upon reflection				
		FMoveConstruct mMoveConstructor;
		// Destructor wrapped in a lambda upon reflection						
		FDestroy mDestructor;
		// Cloner wrapped in a lambda upon reflection (placement new)		
		FClone mCloneInUninitilizedMemory;
		// Cloner wrapped in a lambda upon reflection							
		FClone mCloneInInitializedMemory;
		// The == operator, wrapped in a lambda upon reflection				
		FCompare mComparer;
		// The = operator, wrapped in a lambda upon reflection				
		FCopy mCopier;
		// The move operator, wrapped in a lambda upon reflection			
		FMove mMover;
		// The ClassBlock method, wrapped in a lambda upon reflection		
		FResolve mResolver;
		// The GetHash() method, wrapped in a lambda								
		FHash mHasher;
		// The Do verb, wrapped in a lambda											
		FDispatch mDispatcher;

	public:
		template<CT::Data T>
		NOD() static DMeta Of() requires CT::Decayed<T>;

	protected:
		template<CT::Fundamental T>
		void ReflectFundamentalType() noexcept;

	public:
		NOD() DMeta GetMostConcrete() const noexcept;

		template<class T, CT::Dense... Args>
		void SetBases(TTypeList<Args...>) noexcept;

		template<class T, CT::Dense... Args>
		void SetAbilities(TTypeList<Args...>) noexcept;

		template<CT::Dense... Args>
		void SetMembers(Args&&...) noexcept requires (... && CT::Same<Args, Member>);

		NOD() bool GetBase(DMeta, Offset, Base&) const;
		template<CT::Data T>
		NOD() bool GetBase(Offset, Base&) const;

		NOD() bool HasBase(DMeta) const;
		template<CT::Data T>
		NOD() bool HasBase() const;

		NOD() bool HasDerivation(DMeta) const;
		template<CT::Data T>
		NOD() bool HasDerivation() const;

		NOD() bool IsAbleTo(VMeta) const;
		template<CT::Verb T>
		NOD() bool IsAbleTo() const;

		template<bool ADVANCED = false>
		NOD() bool CastsTo(DMeta) const;
		NOD() bool CastsTo(DMeta, Count) const;

		template<CT::Data T, bool ADVANCED = false>
		NOD() bool CastsTo() const;
		template<CT::Data T>
		NOD() bool CastsTo(Count) const;

		NOD() bool IsRelatedTo(DMeta) const;
		template<CT::Data T>
		NOD() bool IsRelatedTo() const;

		NOD() Distance GetDistanceTo(DMeta) const;
		template<CT::Data T>
		NOD() Distance GetDistanceTo() const;

		NOD() constexpr bool Is(DMeta) const;
		template<CT::Data T>
		NOD() constexpr bool Is() const;

		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			NOD() bool operator == (const MetaData&) const noexcept;
			NOD() bool operator != (const MetaData&) const noexcept;
		#endif

		AllocationRequest RequestSize(const Size&) const noexcept;
	};


	///																								
	///	Meta trait																				
	///																								
	struct MetaTrait : public Meta {
		// Data filter for the trait (optional)									
		DMeta mDataType {};

	public:
		template<CT::Trait T>
		NOD() static TMeta Of();
		
		NOD() bool constexpr Is(TMeta) const;
		template<CT::Trait T>
		NOD() bool constexpr Is() const;
		
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			NOD() bool operator == (const MetaTrait&) const noexcept;
			NOD() bool operator != (const MetaTrait&) const noexcept;
		#endif
	};


	///																								
	///	Meta verb																				
	///																								
	struct MetaVerb : public Meta {
		// Verbs have antonyms, denoted via this 'negative' token			
		// For example, 'Destroy' is the reverse of 'Create'					
		// This is mainly syntax sugar - reverse token just does mass*=-1	
		Token mTokenReverse;

	public:
		template<CT::Verb T>
		NOD() static VMeta Of();
		
		NOD() bool constexpr Is(VMeta) const;
		template<CT::Verb T>
		NOD() bool constexpr Is() const;
		
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			NOD() bool operator == (const MetaVerb&) const noexcept;
			NOD() bool operator != (const MetaVerb&) const noexcept;
		#endif
	};
	
} //namespace Langulus::Anyness

namespace Langulus
{
	namespace RTTI
	{
		template<CT::Data T, bool ADVANCED = false>
		NOD() bool CastsTo(Anyness::DMeta);
		template<CT::Data T>
		NOD() bool CastsTo(Anyness::DMeta, Count);
	}


	///																								
	/// The following abstract types are implicitly added as bases					
	/// when reflecting fundamental types. They are linked to their				
	/// corresponding concepts, so you can use them at runtime, to check if		
	/// a type is compatible with the given concept via type->InterpretsAs		
	///																								

	/// Check if a type is compatible with CT::Number									
	/// concept at runtime, via meta->InterpretsAs<ANumber>							
	class ANumber {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) Real;
		~ANumber() = delete;
	};

	/// Check if a type is compatible with CT::Integer									
	/// concept at runtime, via meta->InterpretsAs<AInteger>							
	class AInteger {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) ::std::intptr_t;
		LANGULUS_BASES(ANumber);
		~AInteger() = delete;
	};

	/// Check if a type is compatible with CT::Signed									
	/// concept at runtime, via meta->InterpretsAs<ASigned>							
	class ASigned {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) Real;
		LANGULUS_BASES(ANumber);
		~ASigned() = delete;
	};

	/// Check if a type is compatible with IsUnsigned									
	/// concept at runtime, via meta->InterpretsAs<AUnsigned>						
	class AUnsigned {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) ::std::uintptr_t;
		LANGULUS_BASES(ANumber);
		~AUnsigned() = delete;
	};

	/// Check if a type is compatible with CT::UnsignedInteger concept at		
	/// runtime, via meta->InterpretsAs<AUnsignedInteger>								
	class AUnsignedInteger {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) ::std::uintptr_t;
		LANGULUS_BASES(AUnsigned, AInteger);
		~AUnsignedInteger() = delete;
	};

	/// Check if a type is compatible with CT::Real										
	/// concept at runtime, via meta->InterpretsAs<AReal>								
	class AReal {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) Real;
		LANGULUS_BASES(ASigned);
		~AReal() = delete;
	};

	/// Check if a type is compatible with CT::SignedInteger							
	/// concept at runtime, via meta->InterpretsAs<ASignedInteger>					
	class ASignedInteger {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) ::std::intptr_t;
		LANGULUS_BASES(ASigned, AInteger);
		~ASignedInteger() = delete;
	};

	/// Check if a type is compatible with CT::Character								
	/// concept at runtime, via meta->InterpretsAs<AText>								
	class AText {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) char8_t;
		~AText() = delete;
	};

	/// Check if a type is compatible with CT::Bool										
	/// concept at runtime, via meta->InterpretsAs<ABool>								
	class ABool {
		LANGULUS(ABSTRACT) true;
		LANGULUS(CONCRETIZABLE) bool;
		~ABool() = delete;
	};

} // namespace Langulus

#include "Reflection.inl"
