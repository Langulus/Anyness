/// Include this only when building standalone											
#pragma once
#include "Integration.hpp"

#define LANGULUS_DEEP() public: static constexpr bool CTTI_Deep = true

namespace Langulus::Flow
{
	class Verb;
}

namespace Langulus::Anyness
{
	
	class Trait;
	
	/// A reflected type is a type that has a public Reflection field				
	/// This field is automatically added when using LANGULUS(REFLECT) macro	
	/// inside the type you want to reflect												
	template<class T>
	concept Reflected = requires { Decay<T>::Reflection; };
	
	/// A reflected data type is any type that is not void, and is either		
	/// manually reflected, or an implicitly reflected fundamental type			
	template<class T>
	concept ReflectedData = !::std::is_void_v<Decay<T>> && (Reflected<T> || ::std::is_fundamental_v<Decay<T>>);

	/// A reflected verb type is any type that inherits Verb							
	template<class T>
	concept ReflectedVerb = ::std::is_base_of_v<Flow::Verb, T>;

	/// A reflected trait type is any type that inherits Trait						
	template<class T>
	concept ReflectedTrait = ::std::is_base_of_v<Trait, T>;

	
	/// A deep type is any type with a static member T::Deep set to true			
	/// If no such member exists, the type is assumed NOT deep by default		
	/// Deep types are considered iteratable, and verbs are executed in each	
	/// of their elements, instead on the container itself							
	template<class T>
	concept Deep = Decay<T>::CTTI_Deep == true;
	
	namespace Inner
	{
		class Block;
	}

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
	using FResolve = TFunctor<Inner::Block(const void*)>;

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
		// Each reflection primitive has a token. Token must be				
		// unique for the given type of primitive.								
		// Some primitives, like data reflections, support multiple			
		// tokens, separated by commas												
		Token mToken;
		// Each reflection may or may not have some info string				
		// attached to it.																
		Token mInfo;
		// Each reflected type has an unique hash									
		Hash mHash;
	};


	///																								
	///	Meta data																				
	///																								
	/// Contains member descriptions, abilities, traits, information				
	///																								
	struct MetaData : public Meta {
		// List of reflected members													
		MemberList mMembers {};
		// List of reflected abilities												
		AbilityList mAbilities {};
		// List of reflected bases														
		BaseList mBases {};
		// Default concretization													
		DMeta mConcrete {};
		// True if reflected data is POD (optimization)						
		// POD data can be directly memcpy-ed, or binary-serialized		
		bool mPOD = false;
		// True if reflected data is nullifiable (optimization)			
		// Nullifiable data can be constructed AND destructed via		
		// memset(0) without hitting undefined behavior						
		bool mNullifiable = false;
		// Dynamic producer of the type											
		// Types with producers can be created only via a verb			
		DMeta mProducer {};
		// If reflected type is a constant type								
		bool mIsConst = false;
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
		template<Dense T>
		NOD() static DMeta Of();

		template<Dense T, class... Args>
		void SetBases(Base&&, Args&& ...) noexcept;

		template<Dense T, class... Args>
		void SetAbilities(Ability&&, Args&& ...) noexcept;

		template<Dense T, class... Args>
		void SetMembers(Member&&, Args&& ...) noexcept;

		void MakeAbstract() noexcept;

		constexpr auto& GetStride() const noexcept;

		NOD() bool GetBase(DMeta, Offset, Base&) const;
		template<ReflectedData T>
		NOD() bool GetBase(Offset, Base&) const;

		NOD() bool HasBase(DMeta) const;
		template<ReflectedData T>
		NOD() bool HasBase() const;

		NOD() bool IsChildOf(DMeta) const;
		template<ReflectedData T>
		NOD() bool IsChildOf() const;

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

		NOD() Count GetDistanceTo(DMeta) const;
		template<ReflectedData T>
		NOD() Count GetDistanceTo() const;

		NOD() bool Is(DMeta) const;
		template<ReflectedData T>
		NOD() bool Is() const;

		NOD() bool operator == (const MetaData&) const noexcept;
		NOD() bool operator != (const MetaData&) const noexcept;
	};


	///																								
	///	Meta trait																				
	///																								
	/// A trait definition																		
	///																								
	struct MetaTrait : public Meta {
		// Data filter for the trait													
		DMeta mDataType {};

	public:
		template<ReflectedTrait T>
		NOD() static TMeta Of();
		NOD() bool operator == (const MetaTrait&) const noexcept;
	};


	///																								
	///	Meta verb																				
	///																								
	/// A verb definition																		
	///																								
	struct MetaVerb : public Meta {
		Token mTokenReverse;

	public:
		template<ReflectedVerb T>
		NOD() static VMeta Of();
		NOD() bool operator == (const MetaVerb&) const noexcept;
	};
	
} // namespace Langulus::Anyness
