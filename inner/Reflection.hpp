/// Include this only when building standalone											
#pragma once

namespace Langulus::Flow
{
	class Verb;
}

namespace Langulus::Anyness
{

	namespace Inner
	{
		class Block;
	}

	/// Reflected data																			
	struct MetaData {
		const ::std::type_info* mID;
	};

	using DMeta = const MetaData&;

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


	///																								
	/// Used to reflect a member variable													
	/// You can reflect arrays of elements, tag members as traits, etc.			
	///																								
	struct Member {
		constexpr Member() noexcept = default;
		constexpr Member(DMeta, Offset, Count, const Token&, const TraitID&) noexcept;

		template<class UNIQUE, class T>
		NOD() static Member From(Offset, const Token&, TraitID);

		NOD() constexpr bool operator == (const Member&) const noexcept;
		NOD() constexpr bool operator != (const Member&) const noexcept;

	public:
		// Type of data																	
		DataID mType = udInvalid;
		// Member offset. This is relative to the type it is offsetted		
		// in! If accessed through a derived type, that offset might		
		// be wrong! Type must be resolved first!									
		Offset mOffset = 0;
		// Number of elements in mData (in case of an array)					
		Count mCount = 1;
		// Trait tag																		
		TraitID mTrait = utInvalid;
		// Member token																	
		Token mName;
	};

	using MemberList = ::std::span<const Member>;

	///																								
	///	Used to reflect data capabilities												
	///																								
	struct Ability {
		constexpr Ability() = default;
		Ability(const VerbID&, const FVerb&, const Token& = {}) noexcept;

		NOD() constexpr bool operator == (const Ability&) const noexcept;
		NOD() constexpr bool operator != (const Ability&) const noexcept;

	public:
		// The verb ID																		
		VerbID mVerb = uvInvalid;
		// Address of function to call												
		FVerb mFunction = nullptr;
	};

	using AbilityList = ::std::span<const Ability>;


	///																								
	///	Used to reflect a base for a type												
	///																								
	struct Base {
		// Type of the base																
		DataID mType = udInvalid;
		// Number of bases that fit in the type									
		Count mCount = 1;
		// Offset of the base, relative to the derived type					
		Offset mOffset = 0;
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
	///	TReflect																				
	///																							
	/// Base for RTTI reflection primitives											
	///																							
	template<class INTERNAL>
	struct TReflect {
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
	///	ReflectData																			
	///																							
	/// Used for constructing meta data definitions and RTTI. May contain	
	/// member descriptions, abilities, traits, info. Useful for cloning,	
	/// serialization, conversion.														
	///																							
	struct ReflectData : public TReflect<DataID> {
		// List of statically reflected members								
		MemberList mMembers;
		// List of statically reflected abilities								
		AbilityList mAbilities;
		// List of statically reflected bases									
		BaseList mBases;
		// Default concretization													
		DataID mConcrete = udInvalid;
		// True if reflected data is POD (optimization)						
		// POD data can be directly memcpy-ed, or binary-serialized		
		bool mPOD = false;
		// True if reflected data is nullifiable (optimization)			
		// Nullifiable data can be constructed AND destructed via		
		// memset(0) without hitting undefined behavior						
		bool mNullifiable = false;
		// Dynamic producer of the type											
		// Types with producers can be created only via a verb			
		DataID mProducer = udInvalid;
		// If reflected type is a sparse (pointer) type						
		bool mIsSparse = false;
		// If reflected type is a constant type								
		bool mIsConst = false;
		// If reflected type is abstract											
		bool mIsAbstract = false;
		// If type is named (will avoid scope decoration)					
		bool mIsNamed = false;
		// If type will be interpreted as a memory block and iterated	
		bool mIsDeep = false;
		// Size of the reflected type (in bytes)								
		Stride mSize = 0;
		// Alignof (in bytes)														
		Stride mAlignment = 0;
		// File extensions used, separated by commas							
		Token mFileExtension;

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
		NOD() static ReflectData From(const Token&, const Token& = {});

		template<class T>
		NOD() ReflectData Finalize() const;

		template<class UNIQUE>
		void SetBases() noexcept;

		template<class UNIQUE, class... Args>
		void SetBases(Base&&, Args&& ...) noexcept;

		template<class UNIQUE>
		void SetAbilities() noexcept;

		template<class UNIQUE, class... Args>
		void SetAbilities(Ability&&, Args&& ...) noexcept;

		template<class UNIQUE>
		void SetMembers() noexcept;

		template<class UNIQUE, class... Args>
		void SetMembers(Member&&, Args&& ...) noexcept;

		void MakeAbstract() noexcept;
	};


	///																								
	///	ReflectTrait																			
	///																								
	/// Used for constructing meta trait definitions and RTTI						
	///																								
	class ReflectTrait : public TReflect<TraitID> {
		// Data filter for the trait													
		DataID mDataType = udInvalid;

	public:
		template<ReflectedTrait T>
		NOD() static ReflectTrait From(const Token&, const Token& = {});
		NOD() bool operator == (const ReflectTrait&) const noexcept;
	};


	///																								
	///	ReflectVerb																				
	///																								
	/// Used for constructing meta verb definitions and RTTI							
	///																								
	class ReflectVerb : public TReflect<VerbID> {
		Token mTokenReverse;

	public:
		NOD() bool operator == (const ReflectVerb&) const noexcept;
	};


	///																								
	///	ReflectConst																			
	///																								
	/// Used for constructing meta const definitions and RTTI						
	///																								
	class ReflectConst : public TReflect<ConstID> {
		// The type of data contained in mData										
		DataID mDataType = udInvalid;
		// Contained data																	
		const void* mData = nullptr;

	public:
		NOD() bool operator == (const ReflectConst&) const noexcept;
	};

} // namespace Langulus::Anyness
