/// Include this only when building standalone											
#pragma once
#include "DataState.hpp"
#include "TypeList.hpp"
#include "Exceptions.hpp"

/// You can mark types as deep by using LANGULUS(DEEP) true / false inside		
/// class, but to fit into IsDeep concept, your type must also inherit Block	
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
#define LANGULUS_ABSTRACT() private: virtual void StayAbstractForever() = 0

/// Reflect a list of members																	
#define LANGULUS_MEMBERS(...) public: using CTTI_Members = ::Langulus::Anyness::TTypeList<__VA_ARGS__>

/// Reflect a list of bases																	
#define LANGULUS_BASES(...) public: using CTTI_Bases = ::Langulus::Anyness::TTypeList<__VA_ARGS__>

/// Reflect a list of verbs																	
#define LANGULUS_VERBS(...) public: using CTTI_Verbs = ::Langulus::Anyness::TTypeList<__VA_ARGS__>

namespace Langulus::Flow
{
	class Verb;
}

namespace Langulus::Anyness
{
	
	class Block;
	class Trait;
	struct Member;
	struct Base;
	struct Ability;
	struct Meta;
	struct MetaData;
	struct MetaVerb;
	struct MetaTrait;

	using DMeta = const MetaData*;
	using TMeta = const MetaTrait*;
	using VMeta = const MetaVerb*;


	/// A reflected type is a type that has a public Reflection field				
	/// This field is automatically added when using LANGULUS(REFLECT) macro	
	/// inside the type you want to reflect												
	template<class T>
	concept Reflectable = requires {
		{Decay<T>::Reflect()} -> IsSame<MetaData>;
	};
	
	/// A reflected data type is any type that is not void, and is either		
	/// manually reflected, or an implicitly reflected fundamental type			
	template<class T>
	concept ReflectedData = not ::std::is_void_v<Decay<T>>;

	/// A reflected verb type is any type that inherits Verb							
	template<class T>
	concept ReflectedVerb = Inherits<T, Flow::Verb>;

	/// A reflected trait type is any type that inherits Trait						
	template<class T>
	concept ReflectedTrait = Inherits<T, Trait>;

	/// Checks if T inherits Block															
	template<class T>
	concept IsBlock = Inherits<T, Block>;
	
	/// A deep type is any type with a static member T::CTTI_Deep set to true	
	/// and a common interface with Block													
	/// If no such member/base exists, the type is assumed NOT deep by default	
	/// Deep types are considered iteratable, and verbs are executed in each	
	/// of their elements, instead on the container itself							
	/// Use LANGULUS(DEEP) macro as member to tag deep types							
	template<class T>
	concept IsDeep = IsBlock<T> && Decay<T>::CTTI_Deep == true;
	
	/// A POD (Plain Old Data) type is any type with a static member				
	/// T::CTTI_POD set to true. If no such member exists, the type is assumed	
	/// NOT POD by default, unless ::std::is_trivial.									
	/// POD types improve construction, destruction, copying, and cloning by	
	/// using some batching runtime optimizations										
	/// All POD types are also directly serializable to binary						
	/// Use LANGULUS(POD) macro as member to tag POD types							
	template<class T>
	concept IsPOD = ::std::is_trivial_v<Decay<T>> || Decay<T>::CTTI_POD == true;
	
	/// A nullifiable type is any type with a static member							
	/// T::CTTI_Nullifiable set to true. If no such member exists, the type is 
	/// assumed	NOT nullifiable by default													
	/// Nullifiable types improve construction by using some batching runtime	
	/// optimizations																				
	/// Use LANGULUS(NULLIFIABLE) macro as member to tag nullifiable types		
	template<class T>
	concept IsNullifiable = Decay<T>::CTTI_Nullifiable == true;
	
	/// A concretizable type is any type with a member type CTTI_Concrete 		
	/// If no such member exists, the type is assumed NOT concretizable by		
	/// default. Concretizable types provide a default concretization for when	
	/// allocating abstract types																
	/// Use LANGULUS(CONCRETIZABLE) macro as member to tag such types				
	template<class T>
	concept IsConcretizable = requires {
		typename Decay<T>::CTTI_Concrete;
	};
	
   /// Round to the upper power-of-two														
	///	@param x - the unsigned integer to round up									
	///	@return the closest upper power-of-two to x									
   template<IsUnsigned T>
	constexpr T Roof2(const T& x) noexcept {
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
		++n;
		return n;
	}

	/// Get the allocation page size of the type (in bytes)							
	template<class T>
	constexpr Size GetAllocationPageOf() noexcept {
		if constexpr (requires {{Decay<T>::CTTI_AllocationPage} -> IsSame<Size>;}) {
			constexpr Size candidate = Decay<T>::CTTI_AllocationPage * sizeof(T);
			if constexpr (candidate < LANGULUS(ALIGN))
				return LANGULUS(ALIGN);
			else 
				return Roof2(candidate);
		}
		else if constexpr (sizeof(T) < LANGULUS(ALIGN))
			return LANGULUS(ALIGN);
		else 
			return Roof2(sizeof(T));
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

		template<ReflectedData OWNER, ReflectedData DATA>
		NOD() static Member From(Offset, const Token& = {}, TMeta trait = {});

		NOD() constexpr bool operator == (const Member&) const noexcept;
		NOD() constexpr bool operator != (const Member&) const noexcept;
		
		template<ReflectedData T>
		NOD() constexpr bool Is() const noexcept;
		
		template<ReflectedData T>
		NOD() const T& As(const Byte*) const noexcept;
		template<ReflectedData T>
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

		template<IsDense T, IsDense VERB>
		NOD() static Ability From() noexcept;
	};

	using AbilityList = ::std::span<const Ability>;


	///																								
	///	Used to reflect a base for a type												
	///																								
	struct Base {
		// Type of the base																
		DMeta mType {};
		// IsNumber of bases that fit in the type									
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

		template<IsDense T, IsDense BASE>
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

		template<ReflectedData T>
		static constexpr Hash GetHash() noexcept;
		template<ReflectedData T>
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
		// Allocation page, in bytes													
		Size mAllocationPage {};
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
		template<ReflectedData T>
		NOD() static DMeta Of() requires IsDecayed<T>;

	protected:
		template<IsFundamental T>
		void ReflectFundamentalType() noexcept;

	public:
		NOD() DMeta GetMostConcrete() const noexcept;

		template<class T, IsDense... Args>
		void SetBases(TTypeList<Args...>) noexcept;

		template<class T, IsDense... Args>
		void SetAbilities(TTypeList<Args...>) noexcept;

		template<IsDense... Args>
		void SetMembers(Args&&...) noexcept requires (... && IsSame<Args, Member>);

		NOD() bool GetBase(DMeta, Offset, Base&) const;
		template<ReflectedData T>
		NOD() bool GetBase(Offset, Base&) const;

		NOD() bool HasBase(DMeta) const;
		template<ReflectedData T>
		NOD() bool HasBase() const;

		NOD() bool HasDerivation(DMeta) const;
		template<ReflectedData T>
		NOD() bool HasDerivation() const;

		NOD() bool IsAbleTo(VMeta) const;
		template<ReflectedVerb T>
		NOD() bool IsAbleTo() const;

		template<bool ADVANCED = false>
		NOD() bool CastsTo(DMeta) const;
		NOD() bool CastsTo(DMeta, Count) const;

		template<ReflectedData T, bool ADVANCED = false>
		NOD() bool CastsTo() const;
		template<ReflectedData T>
		NOD() bool CastsTo(Count) const;

		NOD() bool IsRelatedTo(DMeta) const;
		template<ReflectedData T>
		NOD() bool IsRelatedTo() const;

		NOD() Distance GetDistanceTo(DMeta) const;
		template<ReflectedData T>
		NOD() Distance GetDistanceTo() const;

		NOD() constexpr bool Is(DMeta) const;
		template<ReflectedData T>
		NOD() constexpr bool Is() const;

		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			NOD() bool operator == (const MetaData&) const noexcept;
			NOD() bool operator != (const MetaData&) const noexcept;
		#endif
	};


	///																								
	///	Meta trait																				
	///																								
	struct MetaTrait : public Meta {
		// Data filter for the trait (optional)									
		DMeta mDataType {};

	public:
		template<ReflectedTrait T>
		NOD() static TMeta Of();
		
		NOD() bool constexpr Is(TMeta) const;
		template<ReflectedTrait T>
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
		template<ReflectedVerb T>
		NOD() static VMeta Of();
		
		NOD() bool constexpr Is(VMeta) const;
		template<ReflectedVerb T>
		NOD() bool constexpr Is() const;
		
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			NOD() bool operator == (const MetaVerb&) const noexcept;
			NOD() bool operator != (const MetaVerb&) const noexcept;
		#endif
	};
	
} // namespace Langulus::Anyness


namespace Langulus
{

	namespace RTTI
	{
		template<Anyness::ReflectedData T, bool ADVANCED = false>
		NOD() bool CastsTo(Anyness::DMeta);
		template<Anyness::ReflectedData T>
		NOD() bool CastsTo(Anyness::DMeta, Count);
	}


	///																								
	/// The following abstract types are implicitly added as bases					
	/// when reflecting fundamental types. They are linked to their				
	/// corresponding concepts, so you can use them at runtime, to check if		
	/// a type is compatible with the given concept via type->InterpretsAs		
	///																								

	/// Check if a type is compatible with IsNumber										
	/// concept at runtime, via meta->InterpretsAs<ANumber>							
	class ANumber {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) Real;
		~ANumber() = delete;
	};

	/// Check if a type is compatible with IsInteger									
	/// concept at runtime, via meta->InterpretsAs<AInteger>							
	class AInteger {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) ::std::intptr_t;
		LANGULUS_BASES(ANumber);
		~AInteger() = delete;
	};

	/// Check if a type is compatible with IsSigned										
	/// concept at runtime, via meta->InterpretsAs<ASigned>							
	class ASigned {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) Real;
		LANGULUS_BASES(ANumber);
		~ASigned() = delete;
	};

	/// Check if a type is compatible with IsUnsigned									
	/// concept at runtime, via meta->InterpretsAs<AUnsigned>						
	class AUnsigned {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) ::std::uintptr_t;
		LANGULUS_BASES(ANumber);
		~AUnsigned() = delete;
	};

	/// Check if a type is compatible with IsUnsignedInteger concept at			
	/// runtime, via meta->InterpretsAs<AUnsignedInteger>								
	class AUnsignedInteger {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) ::std::uintptr_t;
		LANGULUS_BASES(AUnsigned, AInteger);
		~AUnsignedInteger() = delete;
	};

	/// Check if a type is compatible with IsReal										
	/// concept at runtime, via meta->InterpretsAs<AReal>								
	class AReal {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) Real;
		LANGULUS_BASES(ASigned);
		~AReal() = delete;
	};

	/// Check if a type is compatible with IsSignedInteger							
	/// concept at runtime, via meta->InterpretsAs<ASignedInteger>					
	class ASignedInteger {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) ::std::intptr_t;
		LANGULUS_BASES(ASigned, AInteger);
		~ASignedInteger() = delete;
	};

	/// Check if a type is compatible with IsCharacter									
	/// concept at runtime, via meta->InterpretsAs<AText>								
	class AText {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) char8_t;
		~AText() = delete;
	};

	/// Check if a type is compatible with IsBool										
	/// concept at runtime, via meta->InterpretsAs<ABool>								
	class ABool {
		LANGULUS(ABSTRACT);
		LANGULUS(CONCRETIZABLE) bool;
		~ABool() = delete;
	};

} // namespace Langulus

#include "Reflection.inl"
