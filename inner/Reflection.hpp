/// Include this only when building standalone											
#pragma once
#include "DataState.hpp"

#define LANGULUS_DEEP() public: static constexpr bool CTTI_Deep = true
#define LANGULUS_POD() public: static constexpr bool CTTI_POD = true
#define LANGULUS_NULLIFIABLE() public: static constexpr bool CTTI_Nullifiable = true

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
	concept Reflectable = requires { {Decay<T>::Reflect()} -> Same<MetaData>; };
	
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

	/// A deep type is any type with a static member T::CTTI_Deep set to true	
	/// and a common interface with Block													
	/// If no such member/base exists, the type is assumed NOT deep by default	
	/// Deep types are considered iteratable, and verbs are executed in each	
	/// of their elements, instead on the container itself							
	/// Use LANGULUS(DEEP) macro as member to tag deep types							
	template<class T>
	concept Deep = Inherits<T, Block> && Decay<T>::CTTI_Deep == true;
	
	/// A POD (Plain Old Data) type is any type with a static member				
	/// T::CTTI_POD set to true. If no such member exists, the type is assumed	
	/// NOT POD by default, unless ::std::is_trivial.									
	/// POD types improve construction, destruction, copying, and cloning by	
	/// using some batching runtime optimizations										
	/// All POD types are also directly serializable to binary						
	/// Use LANGULUS(POD) macro as member to tag POD types							
	template<class T>
	concept POD = ::std::is_trivial_v<Decay<T>> || Decay<T>::CTTI_POD == true;
	
	/// A nullifiable type is any type with a static member							
	/// T::CTTI_Nullifiable set to true. If no such member exists, the type is 
	/// assumed	NOT nullifiable by default													
	/// Nullifiable types improve construction by using some batching runtime	
	/// optimizations																				
	/// Use LANGULUS(NULLIFIABLE) macro as member to tag nullifiable types		
	template<class T>
	concept Nullifiable = Decay<T>::CTTI_Nullifiable == true;
	
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
		Ability(const MetaVerb&, const FVerb&) noexcept;

		NOD() constexpr bool operator == (const Ability&) const noexcept;
		NOD() constexpr bool operator != (const Ability&) const noexcept;
	};

	using AbilityList = ::std::span<const Ability>;


	///																								
	///	Used to reflect a base for a type												
	///																								
	struct Base {
		// Type of the base																
		DMeta mType {};
		// Number of bases that fit in the type									
		Count mCount {1};
		// Offset of the base, relative to the derived type					
		Offset mOffset {};
		// Whether this base is binary mapped to derived class				
		// Allows for the seamless mapping of one type to another			
		// Used for compatibility checks and  decaying containers			
		bool mMapping = false;
		// Whether this base is explicitly defined as an alternative		
		// It is not as much base, as a mapping in this case					
		// Bases marked OR are usually skipped in serialization				
		bool mOr = false;

	public:
		constexpr Base() noexcept = default;

		NOD() constexpr bool operator == (const Base&) const noexcept;
		NOD() constexpr bool operator != (const Base&) const noexcept;

		template<Dense UNIQUE, Dense BASE>
		NOD() static Base From() noexcept;

		template<class UNIQUE, class BASE, Count COUNT>
		NOD() static Base Map() noexcept;
	};

	using BaseList = ::std::span<const Base>;


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
		static constexpr Hash Hash() noexcept;
		template<ReflectedData T>
		static constexpr Token Name() noexcept;
	};


	///																								
	///	Meta data																				
	///																								
	struct MetaData : public Meta {
		using Distance = int;
		
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
		// If type is named (will avoid scope decoration)					
		bool mIsNamed = false;
		// If type will be interpreted as a memory block and iterated	
		bool mIsDeep = false;
		// Size of the reflected type (in bytes)								
		Stride mSize {};
		// Alignof (in bytes)														
		Stride mAlignment {};
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
		NOD() static DMeta Of();

		template<Dense... Args>
		void SetBases(Args&& ...) noexcept requires (... && Same<Args, Base>);

		template<Dense... Args>
		void SetAbilities(Args&& ...) noexcept requires (... && Same<Args, Ability>);

		template<Dense... Args>
		void SetMembers(Args&& ...) noexcept requires (... && Same<Args, Member>);

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

		NOD() bool InterpretsAs(DMeta) const;
		template<ReflectedData T>
		NOD() bool InterpretsAs() const;

		NOD() bool InterpretsAs(DMeta, Count) const;
		template<ReflectedData T>
		NOD() bool InterpretsAs(Count) const;

		NOD() bool IsRelatedTo(DMeta) const;
		template<ReflectedData T>
		NOD() bool IsRelatedTo() const;

		NOD() Distance GetDistanceTo(DMeta) const;
		template<ReflectedData T>
		NOD() Distance GetDistanceTo() const;

		NOD() bool Is(DMeta) const;
		template<ReflectedData T>
		NOD() bool Is() const;

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
		
		NOD() bool Is(TMeta) const;
		template<ReflectedTrait T>
		NOD() bool Is() const;
		
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
		
		NOD() bool Is(VMeta) const;
		template<ReflectedVerb T>
		NOD() bool Is() const;
		
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			NOD() bool operator == (const MetaVerb&) const noexcept;
			NOD() bool operator != (const MetaVerb&) const noexcept;
		#endif
	};
	
} // namespace Langulus::Anyness

#include "Reflection.inl"
