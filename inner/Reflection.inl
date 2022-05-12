#pragma once
#include "Reflection.hpp"
#include "NameOf.hpp"
#include "Utilities.hpp"
#include "Exceptions.hpp"
#include "Hashing.hpp"
#include <memory>

namespace Langulus::Anyness
{
   
   ///                                                                        
   ///   Member implementation                                                
   ///                                                                        
   
   /// Check if member is a specific type                                     
   ///   @return true if member exactly matches the provided type             
   template<ReflectedData T>
   constexpr bool Member::Is() const noexcept {
      return mType->Is<T>();
   }
   
   /// Reinterpret the member as a given type and access it (const, unsafe)   
   ///   @param instance - pointer to the beginning of the owning type        
   ///   @return a reinterpreted constant reference to member                 
   template<ReflectedData T>
   const T& Member::As(const Byte* instance) const noexcept {
      return *reinterpret_cast<const T*>(Get(instance));
   }
   
   /// Reinterpret the member as a given type and access it (unsafe)          
   ///   @param instance - pointer to the beginning of the owning type        
   ///   @return a reinterpreted reference to member                          
   template<ReflectedData T>
   T& Member::As(Byte* instance) const noexcept {
      return *reinterpret_cast<T*>(Get(instance));
   }
   
   /// Directly get a pointer to the type-erased member (const, unsafe)       
   ///   @param instance - pointer to the beginning of the owning type        
   ///   @return a raw constant pointer to the member inside the instance     
   constexpr const Byte* Member::Get(const Byte* instance) const noexcept {
      return instance + mOffset;
   }
   
   /// Directly get a pointer to the type-erased member (unsafe)              
   ///   @param instance - pointer to the beginning of the owning type        
   ///   @return a raw pointer to the member inside the instance              
   constexpr Byte* Member::Get(Byte* instance) const noexcept {
      return instance + mOffset;
   }
   

	///                                                                        
   ///   Base implementation																	
   ///                                                                        

	/// Compare bases for equality															
	constexpr bool Base::operator == (const Base& other) const noexcept {
		return mType == other.mType && mCount == other.mCount;

	}

	/// Compare bases for inequality															
	constexpr bool Base::operator != (const Base& other) const noexcept {
		return !(*this == other);

	}

	/// Create a base descriptor for the derived type T								
	///	@return the generated base descriptor											
	template<IsDense T, IsDense BASE>
	Base Base::From() SAFETY_NOEXCEPT() {
		static_assert(!IsSame<T, BASE>, 
			"Base duplication not allowed to avoid regress");

		Base result;
		result.mType = MetaData::Of<BASE>();

		if constexpr (Inherits<T, BASE>) {
			// This will fail if base is private									
			// This is detectable by is_convertible_v								
			if constexpr (::std::is_convertible_v<T*, BASE*>) {
				// The devil's work, right here										
				const Byte storage[sizeof(T)] = {};
				// First reinterpret the storage as T								
				const auto derived = reinterpret_cast<const T*>(storage);
				// Then cast it down to base											
				const auto base = static_cast<const BASE*>(derived);
				// Then reinterpret back to byte arrays and get difference	
				const auto offset = 
					reinterpret_cast<const Byte*>(derived) 
				 - reinterpret_cast<const Byte*>(base);
				SAFETY(if (offset > 0)
					throw Except::Access("Base class is laid (memorywise) before the derived"));
				result.mOffset = static_cast<Offset>(offset);
			}
		}

		// If sizes match and there's no byte offset, then the base and	
		// the derived type are binary compatible									
		if constexpr (sizeof(BASE) == sizeof(T))
			result.mBinaryCompatible = (0 == result.mOffset);
		return result;
	}

	/// Create a mapping to a type															
	/// This will check if types are binary compatible by size, but no			
	/// further checks are done. Use at your own risk									
	/// It also makes the base imposed, which excludes it from serialization	
	///	@return the generated base descriptor											
	template<class T, class BASE, Count COUNT>
	Base Base::Map() noexcept {
		static_assert(!IsSame<T, BASE>, 
			"Base duplication not allowed to avoid regress");
		static_assert(sizeof(BASE) * COUNT == sizeof(T),
			"Size mismatch while mapping types");
		static_assert(COUNT > 0,
			"Invalid mapping of zero count");
		static_assert(IsAbstract<BASE>,
			"Can't map to an abstract type - size is always zero");

		Base result;
		result.mBinaryCompatible = true;
		result.mType = MetaData::Of<BASE>();
		result.mCount = COUNT;
		result.mImposed = true;
		return result;
	}



	///                                                                        
   ///   Meta implementation																	
   ///                                                                        

	/// Get the constexpr hash of a type													
	///	@return the hash of the type														
	template<ReflectedData T>
	constexpr Hash Meta::GetHash() noexcept {
		const auto name = Inner::NameOf<T>();
		return ::std::hash<::std::u8string_view>()(name);
	}
   
	/// Get the constexpr name of a type													
	///	@return the hash of the type														
	template<ReflectedData T>
	constexpr Token Meta::GetName() noexcept {
		return Inner::NameOf<T>();
	}
   


   ///                                                                        
   ///   MetaData implementation                                              
   ///                                                                        
   
   /// Reflect or return an already reflected type meta definition            
   /// Reflection is done only on decayed types to reduce statics             
   ///   @tparam T - the type to reflect (will always be decayed)             
   template<ReflectedData T>
   DMeta MetaData::Of() requires IsDecayed<T> {
		// This check is not standard, but doesn't hurt afaik					
		static_assert(sizeof(T) > 0, "Can't reflect an incomplete type");

		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			static constinit ::std::weak_ptr<MetaData> meta;
		#else
			static constinit ::std::unique_ptr<MetaData> meta;
		#endif

		// Never proceed with reflection, if already reflected				
		if (meta)
			return meta.get();

		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			// Try to get the definition, type might have been reflected	
			// previously in another translation unit. This is available	
			// only if MANAGED_REFLECTION feature is enabled					
			meta = TODO();
			if (meta)
				return meta;
		#endif

		// If this is reached, then type is not defined yet					
		// We'll try to explicitly or implicitly reflect it					

		if constexpr (Reflectable<T>) {
			// The type is explicitly reflected with a custom function		
			// Let's call it...															
			meta = ::std::make_unique<MetaData>(T::Reflect());
			return meta.get();
		}
		else {
			// Type is implicitly reflected, so let's do our best				
			meta = ::std::make_unique<MetaData>();
			meta->mToken = Meta::GetName<T>();
			meta->mInfo = u8"<no info provided due to implicit reflection>";
			meta->mName = Meta::GetName<T>();
			meta->mHash = Meta::GetHash<T>();
			meta->mIsAbstract = IsAbstract<T>;
			meta->mIsNullifiable = IsNullifiable<T>;
			meta->mSize = IsAbstract<T> ? 0 : sizeof(T);
			meta->mAlignment = alignof(T);
			meta->mAllocationPage = GetAllocationPageOf<T>();
			meta->mIsPOD = IsPOD<T>;
			meta->mIsDeep = IsDeep<T>;
			
			if constexpr (IsConcretizable<T>)
				meta->mConcrete = MetaData::Of<Decay<typename T::CTTI_Concrete>>();

			// Wrap the default constructor of the type inside a lambda		
			if constexpr (IsDefaultConstructible<T>) {
				meta->mDefaultConstructor = [](void* at) {
					new (at) T {};
				};
			}

			// Wrap the copy constructor of the type inside a lambda			
			if constexpr (IsCopyConstructible<T>) {
				meta->mCopyConstructor = [](void* at, const void* from) {
					auto fromInstance = static_cast<const T*>(from);
					new (at) T {*fromInstance};
				};
			}

			// Wrap the move constructor of the type inside a lambda			
			if constexpr (IsMoveConstructible<T>) {
				meta->mMoveConstructor = [](void* at, void* from) {
					auto fromInstance = static_cast<T*>(from);
					new (at) T {Forward<T>(*fromInstance)};
				};
			}

			// Wrap the destructor of the type inside a lambda					
			if constexpr (IsDestructible<T>) {
				meta->mDestructor = [](void* at) {
					auto instance = static_cast<T*>(at);
					instance->~T();
				};
			}

			// Wrap the cloners of the type inside a lambda						
			if constexpr (IsClonable<T>) {
				if constexpr (IsCopyConstructible<T> || IsMoveConstructible<T>) {
					meta->mCloneInUninitilizedMemory = [](const void* from, void* to) {
						auto fromInstance = static_cast<const T*>(from);
						new (to) T {fromInstance->Clone()};
					};
				}

				if constexpr (IsCopyable<T> || IsMovable<T>) {
					meta->mCloneInInitializedMemory = [](const void* from, void* to) {
						auto toInstance = static_cast<T*>(to);
						auto fromInstance = static_cast<const T*>(from);
						*toInstance = fromInstance->Clone();
					};
				}
			}

			// Wrap the == operator of the type inside a lambda				
			if constexpr (IsComparable<T>) {
				meta->mComparer = [](const void* t1, const void* t2) {
					auto t1Instance = static_cast<const T*>(t1);
					auto t2Instance = static_cast<const T*>(t2);
					return *t1Instance == *t2Instance;
				};
			}

			// Wrap the copy operator of the type inside a lambda				
			if constexpr (IsCopyable<T>) {
				meta->mCopier = [](const void* from, void* to) {
					auto toInstance = static_cast<T*>(to);
					auto fromInstance = static_cast<const T*>(from);
					*toInstance = *fromInstance;
				};
			}

			// Wrap the move operator of the type inside a lambda				
			if constexpr (IsMovable<T>) {
				meta->mMover = [](void* from, void* to) {
					auto toInstance = static_cast<T*>(to);
					auto fromInstance = static_cast<T*>(from);
					*toInstance = Move(*fromInstance);
				};
			}

			// Wrap the GetBlock method of the type inside a lambda			
			if constexpr (IsResolvable<T>) {
				meta->mResolver = [](const void* at) {
					auto instance = static_cast<const T*>(at);
					return instance->GetBlock();
				};
			}

			// Wrap the GetHash() method inside a lambda							
			if constexpr (IsHashable<T> || IsNumber<T> || IsPOD<T>) {
				meta->mHasher = [](const void* at) {
					auto instance = static_cast<const T*>(at);
					return HashData(*instance);
				};
			}

			// Wrap the Do verb method inside a lambda							
			if constexpr (IsDispatcher<T>) {
				meta->mDispatcher = [](void* at, Flow::Verb& verb) {
					auto instance = static_cast<T*>(at);
					instance->Do(verb);
				};
			}

			// Set reflected bases														
			if constexpr (requires { typename T::CTTI_Bases; })
				meta->SetBases<T>(typename T::CTTI_Bases {});

			// Set reflected abilities													
			if constexpr (requires { typename T::CTTI_Verbs; })
				meta->SetAbilities<T>(typename T::CTTI_Verbs {});

			// Set some additional stuff if T is fundamental					
			if constexpr (IsFundamental<T>)
				meta->ReflectFundamentalType<T>();

			return meta.get();
		}
	}

	/// Integrate fundamental types with the reflection system						
	/// Like, for example, implicitly adding a ANumber bases to number types	
	template<IsFundamental T>
	void MetaData::ReflectFundamentalType() noexcept {
		if constexpr (IsBool<T>) {
			using Bases = TTypeList<ABool>;
			SetBases<T>(Bases {});
		}
		else if constexpr (IsCharacter<T>) {
			using Bases = TTypeList<AText>;
			SetBases<T>(Bases {});
		}
		else if constexpr (IsSignedInteger<T>) {
			using Bases = TTypeList<ASignedInteger>;
			SetBases<T>(Bases {});
		}
		else if constexpr (IsUnsignedInteger<T>) {
			using Bases = TTypeList<AUnsignedInteger>;
			SetBases<T>(Bases {});
		}
		else if constexpr (IsReal<T>) {
			using Bases = TTypeList<AReal>;
			SetBases<T>(Bases {});
		}
		else LANGULUS_ASSERT("Unimplemented fundamental type reflector");
	}

   /// Set the list of bases for a given meta definition                      
   ///   @tparam Args... - all the bases                                      
	template<class T, IsDense... BASE>
	void MetaData::SetBases(TTypeList<BASE...>) noexcept {
		static Base list[] = {Base::From<T, BASE>()...};
		mBases = {list};
	}

   /// Set the list of abilities for a given meta definition                  
   ///   @tparam Args... - all the abilities                                  
	template<class T, IsDense... VERB>
	void MetaData::SetAbilities(TTypeList<VERB...>) noexcept {
		static Ability list[] = {Ability::From<T, VERB>()...};
		mAbilities = {list};
	}

   /// Set the list of members for a given meta definition                    
   ///   @tparam Args... - all the members                                    
	template<IsDense... Args>
	void MetaData::SetMembers(Args&&... items) noexcept requires (... && IsSame<Args, Member>) {
		mMembers = {Forward<Member>(items)...};
	}
	
	/// Get the most concrete type															
	///	@return the most concrete type													
	inline DMeta MetaData::GetMostConcrete() const noexcept {
		auto concrete = this;
		while (concrete->mConcrete)
			concrete = concrete->mConcrete;
		return concrete;
	}

	/// Get a reflected base linked to this meta data definition					
	/// Traverses the whole inheritance tree, so can return distant bases		
	///	@param type - the type of base to search for, nullptr for any			
	///	@param offset - use this to get bases by index								
	///	@param base - [in/out] base info ends up here if found					
	///	@return true if a base is available												
	inline bool MetaData::GetBase(DMeta type, Offset offset, Base& base) const {
		Count scanned {};
		for (auto& b : mBases) {
			// Check base																	
			if (type->Is(b.mType)) {
				if (scanned == offset) {
					base = b;
					return true;
				}
				else ++scanned;
			}

			// Dig deeper																	
			Base local {};
			Offset index {};
			while (b.mType->GetBase(type, index, local)) {
				if (scanned == offset) {
					local.mOffset += b.mOffset;
					local.mCount *= b.mCount;
					local.mBinaryCompatible = 
						b.mBinaryCompatible && local.mBinaryCompatible;
					local.mImposed = b.mImposed || local.mImposed;
					base = local;
					return true;
				}
				else ++scanned;
				
				++index;
			}
		}

		return false;
	}
	
	/// Get a reflected base linked to this meta data definition					
	/// Traverses the whole inheritance tree, so can return distant bases		
	///	@tparam T - the type of base to search for, void for any					
	///	@param offset - use this to get bases by index								
	///	@param base - [in/out] base info ends up here if found					
	///	@return true if a base is available												
	template<ReflectedData T>
	bool MetaData::GetBase(Offset offset, Base& base) const {
		return GetBase(MetaData::Of<Decay<T>>(), offset, base);
	}

	/// A simple check if a reflected base is linked to this meta data			
	/// Traverses the whole inheritance tree, so can return distant bases		
	///	@param type - the type of base to search for									
	///	@return true if a base is available												
	inline bool MetaData::HasBase(DMeta type) const {
		for (auto& b : mBases) {
			if (type->Is(b.mType) || b.mType->HasBase(type))
				return true;
		}

		return false;
	}
	
	/// A simple check if a reflected base is linked to this meta data			
	/// Traverses the whole inheritance tree, so can return distant bases		
	///	@tparam T - the type of base to search for									
	///	@return true if a base is available												
	template<ReflectedData T>
	bool MetaData::HasBase() const {
		return HasBase(MetaData::Of<T>());
	}

	/// A simple check if this meta data has a derivation								
	/// Traverses the whole inheritance tree, so can return distant bases		
	///	@param type - the type of derivation to search for							
	///	@return true if a base is available												
	inline bool MetaData::HasDerivation(DMeta type) const {
		return type->HasBase(this);
	}
	
	/// A simple check if this meta data has a derivation								
	/// Traverses the whole inheritance tree, so can return distant bases		
	///	@tparam T - the type of derivation to search for							
	///	@return true if a base is available												
	template<ReflectedData T>
	bool MetaData::HasDerivation() const {
		return MetaData::Of<T>()->HasBase(this);
	}

	/// Check if this data type is able to do something								
	///	@param verb - the verb to check if able										
	///	@return true if this data type is able to do verb							
	inline bool MetaData::IsAbleTo(VMeta verb) const {
		for (auto& v : mAbilities) {
			if (verb->Is(v.mVerb))
				return true;
		}

		return false;
	}
	
	/// Check if this data type is able to do something								
	///	@tparam T - the verb to check if able											
	///	@return true if this data type is able to do verb							
	template<ReflectedVerb T>
	bool MetaData::IsAbleTo() const {
		return IsAbleTo(MetaVerb::Of<T>());
	}

	/// Check if this type interprets as another without conversion				
	///	@param other - the type to try interpreting as								
	///	@return true if this type interprets as other								
	template<bool ADVANCED>
	bool MetaData::CastsTo(DMeta other) const {
		if (Is(other))
			return true;

		// Different types might be compatible via inheritance				
		if (HasBase(other))
			return true;

		if constexpr (ADVANCED) {
			// Do reverse inheritance check, in search of mappings			
			Base found {};
			if (other->GetBase(this, 0, found))
				return mResolver || found.mBinaryCompatible;
		}

		// At this point we're pretty sure that types are incompatible		
		return false;
	}
	
	/// Check if this type interprets as another without conversion				
	///	@tparam T - the type to try interpreting as									
	///	@return true if this type interprets as other								
	template<ReflectedData T, bool ADVANCED>
	bool MetaData::CastsTo() const {
		return CastsTo<ADVANCED>(MetaData::Of<Decay<T>>());
	}

	/// Check if this type interprets as an exact number of another without		
	/// conversion																					
	///	@param other - the type to try interpreting as								
	///	@param count - the number of items to interpret as							
	///	@return true if this type interprets as other								
	inline bool MetaData::CastsTo(DMeta other, Count count) const {
		if (Is(other) && count == 1)
			return true;

		Base found {};
		Count scanned {};
		while (GetBase(other, scanned, found)) {
			if (found.mOffset != 0)
				// Base caused a memory gap, so early failure occurs			
				// All bases must fit neatly into the original type			
				return false;

			if ((other->mIsAbstract || found.mBinaryCompatible) && count == found.mCount)
				return true;
			
			scanned += found.mCount;
		}

		if (scanned == count && !other->mIsAbstract)
			return true;

		// At this point we're pretty sure that types are incompatible		
		return false;
	}
	
	/// Check if this type interprets as an exact number of another without		
	/// conversion																					
	///	@tparam T - the type to try interpreting as									
	///	@return true if this type interprets as other								
	template<ReflectedData T>
	bool MetaData::CastsTo(Count count) const {
		return CastsTo(MetaData::Of<T>(), count);
	}

	/// Check if this type is either same, base or a derivation of other			
	///	@param other - the type to check													
	///	@return true if this type is related to other								
	inline bool MetaData::IsRelatedTo(DMeta other) const {
		return Is(other) || HasBase(other) || HasDerivation(other);
	}
	
	/// Check if this type is either same, base or a derivation of other			
	///	@tparam T - the type to check														
	///	@return true if this type is related to other								
	template<ReflectedData T>
	bool MetaData::IsRelatedTo() const {
		return IsRelatedTo(MetaData::Of<T>());
	}

	/// Get the number of conversions required to map one type to another		
	///	@param other - the type to check distance to									
	///	@return the distance																	
	inline MetaData::Distance MetaData::GetDistanceTo(DMeta other) const {
		if (Is(other))
			return Distance{0};

		// Check bases																		
		Distance jumps = Distance::Infinite;
		for (auto& b : mBases) {
			if (b.mImposed)
				continue;

			auto d = b.mType->GetDistanceTo(other);
			if (d != Distance::Infinite && d + 1 < jumps)
				jumps = Distance{d + 1};
		}

		return jumps;
	}
	
	/// Get the number of conversions required to map one type to another		
	///	@tparam T - the type to check distance to										
	///	@return the distance																	
	template<ReflectedData T>
	MetaData::Distance MetaData::GetDistanceTo() const {
		return GetDistanceTo(MetaData::Of<T>());
	}

	/// Check if two meta definitions match exactly										
	///	@param other - the type to compare against									
	///	@return true if types match														
	constexpr bool MetaData::Is(DMeta other) const {
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			// This function is reduced to a pointer match, if the meta		
			// database is centralized, because it guarantees that			
			// definitions in separate translation units are always the		
			// same instance																
			return this == other;
		#else
			return mHash == other->mHash && mToken == other->mToken;
		#endif
	}
	
	/// Check if two meta definitions match exactly										
	///	@tparam T - the type to compare against										
	///	@return true if types match														
	template<ReflectedData T>
	constexpr bool MetaData::Is() const {
		return Is(MetaData::Of<Decay<T>>());
	}

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
	inline bool MetaData::operator == (const MetaData&) const noexcept {
		
	}
	
	inline bool MetaData::operator != (const MetaData&) const noexcept {
		
	}
#endif
	
	
	///                                                                        
   ///   MetaTrait implementation															
   ///                                                                        
   
   /// Check if two meta definitions match exactly										
	///	@param other - the trait to compare against									
	///	@return true if traits match														
	constexpr bool MetaTrait::Is(TMeta other) const {
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			// This function is reduced to a pointer match, if the meta		
			// database is centralized, because it guarantees that			
			// definitions in separate translation units are always the		
			// same instance																
			return this == other;
		#else
			return mHash == other->mHash && mToken == other->mToken;
		#endif
	}	
	
	/// Check if two meta definitions match exactly										
	///	@tparam T - the trait to compare against										
	///	@return true if traits match														
	template<ReflectedTrait T>
	constexpr bool MetaTrait::Is() const {
		return Is(MetaTrait::Of<T>());
	}	
	
	
	///                                                                        
   ///   MetaVerb implementation																
   ///                                                                        
   
   /// Check if two meta definitions match exactly										
	///	@param other - the verb to compare against									
	///	@return true if verbs match														
	constexpr bool MetaVerb::Is(VMeta other) const {
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			// This function is reduced to a pointer match, if the meta		
			// database is centralized, because it guarantees that			
			// definitions in separate translation units are always the		
			// same instance																
			return this == other;
		#else
			return mHash == other->mHash && mToken == other->mToken;
		#endif
	}	
   
   /// Check if two meta definitions match exactly										
	///	@tparam T - the verb to compare against										
	///	@return true if verbs match														
	template<ReflectedVerb T>
	constexpr bool MetaVerb::Is() const {
		return Is(MetaVerb::Of<T>());
	}
	
} // namespace Langulus::Anyness


namespace Langulus::RTTI
{
	template<Anyness::ReflectedData T, bool ADVANCED>
	bool CastsTo(Anyness::DMeta from) {
		return from->template CastsTo<T, ADVANCED>();
	}

	template<Anyness::ReflectedData T>
	bool CastsTo(Anyness::DMeta from, Count count) {
		return from->template CastsTo<T>(count);
	}
} // namespace Langulus::RTTI
