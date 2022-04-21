#pragma once
#include "Reflection.hpp"
#include "NameOf.hpp"
#include "Utilities.hpp"

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
   ///   Meta implementation																	
   ///                                                                        

	/// Get the constexpr hash of a type													
	///	@return the hash of the type														
	template<ReflectedData T>
	constexpr Hash Meta::Hash() noexcept {
		const auto name = Inner::NameOf<T>();
		return ::std::hash<::std::u8string_view>()(name);
	}
   
	/// Get the constexpr name of a type													
	///	@return the hash of the type														
	template<ReflectedData T>
	constexpr Token Meta::Name() noexcept {
		return Inner::NameOf<T>();
	}
   


   ///                                                                        
   ///   MetaData implementation                                              
   ///                                                                        
   
   /// Reflect or return an already reflected type meta definition            
   ///   @attention reflection is done only on the decayed T                  
   ///   @tparam T - the type to reflect (will always be decayed)             
   template<ReflectedData T>
   DMeta MetaData::Of() {
		using Decayed = Decay<T>;

		// This check is not standard, but doesn't hurt afaik					
		static_assert(sizeof(Decayed) > 0, "Can't reflect an incomplete type");

		// Never proceed with reflection, if already reflected				
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			static constinit ::std::weak_ptr<MetaData> meta;
		#else
			static constinit ::std::unique_ptr<MetaData> meta;
		#endif

		if (meta)
			return meta.get();

		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			// Try to get the definition, type might have been reflected	
			// previously in another translation unit. This is available	
			// only if MANAGED_REFLECTION feature is enabled					
			meta = TODO;
			if (meta)
				return meta;
		#endif

		// If this is reached, then type is not defined yet					
		// We'll try to explicitly or implicitly reflect it					

		if constexpr (Reflectable<Decayed>) {
			// The type is explicitly reflected with a custom function		
			// Let's call it...															
			meta = ::std::make_unique<MetaData>(Decayed::Reflect());
			return meta.get();
		}
		else {
			// Type is implicitly reflected, so let's do our best				
			meta = ::std::make_unique<MetaData>();
			meta->mToken = Meta::Name<Decayed>();
			meta->mInfo = u8"<no info provided due to implicit reflection>";
			meta->mName = Meta::Name<Decayed>();
			meta->mHash = Meta::Hash<Decayed>();
			meta->mIsAbstract = Abstract<Decayed>;
			meta->mIsNullifiable = Nullifiable<Decayed>;
			meta->mSize = Abstract<Decayed> ? 0 : sizeof(Decayed);
			meta->mAlignment = alignof(Decayed);
			meta->mIsPOD = POD<T>;
			meta->mIsDeep = Deep<T>;

			// Wrap the default constructor of the type inside a lambda		
			if constexpr (DefaultConstructible<Decayed>) {
				meta->mDefaultConstructor = [](void* at) {
					new (at) Decayed {};
				};
			}

			// Wrap the copy constructor of the type inside a lambda			
			if constexpr (CopyConstructible<Decayed>) {
				meta->mCopyConstructor = [](void* at, const void* from) {
					auto fromInstance = static_cast<const Decayed*>(from);
					new (at) Decayed {*fromInstance};
				};
			}

			// Wrap the move constructor of the type inside a lambda			
			if constexpr (MoveConstructible<Decayed>) {
				meta->mMoveConstructor = [](void* at, void* from) {
					auto fromInstance = static_cast<Decayed*>(from);
					new (at) Decayed {Forward<Decayed>(*fromInstance)};
				};
			}

			// Wrap the destructor of the type inside a lambda					
			meta->mDestructor = [](void* at) {
				auto instance = static_cast<Decayed*>(at);
				instance->~Decayed();
			};

			// Wrap the cloners of the type inside a lambda						
			if constexpr (Clonable<Decayed>) {
				if constexpr (CopyConstructible<Decayed> || MoveConstructible<Decayed>) {
					meta->mCloneInUninitilizedMemory = [](const void* from, void* to) {
						auto fromInstance = static_cast<const Decayed*>(from);
						new (to) Decayed {fromInstance->Clone()};
					};
				}

				if constexpr (Copyable<Decayed> || Movable<Decayed>) {
					meta->mCloneInInitializedMemory = [](const void* from, void* to) {
						auto toInstance = static_cast<Decayed*>(to);
						auto fromInstance = static_cast<const Decayed*>(from);
						*toInstance = fromInstance->Clone();
					};
				}
			}

			// Wrap the == operator of the type inside a lambda				
			if constexpr (Comparable<Decayed, Decayed>) {
				meta->mComparer = [](const void* t1, const void* t2) {
					auto t1Instance = static_cast<const Decayed*>(t1);
					auto t2Instance = static_cast<const Decayed*>(t2);
					return *t1Instance == *t2Instance;
				};
			}

			// Wrap the copy operator of the type inside a lambda				
			if constexpr (Copyable<Decayed>) {
				meta->mCopier = [](const void* from, void* to) {
					auto toInstance = static_cast<Decayed*>(to);
					auto fromInstance = static_cast<const Decayed*>(from);
					*toInstance = *fromInstance;
				};
			}

			// Wrap the move operator of the type inside a lambda				
			if constexpr (Movable<Decayed>) {
				meta->mMover = [](void* from, void* to) {
					auto toInstance = static_cast<Decayed*>(to);
					auto fromInstance = static_cast<Decayed*>(from);
					*toInstance = Move(*fromInstance);
				};
			}

			// Wrap the GetBlock method of the type inside a lambda			
			if constexpr (Resolvable<Decayed>) {
				meta->mResolver = [](const void* at) {
					auto instance = static_cast<const Decayed*>(at);
					return instance->GetBlock();
				};
			}

			// Wrap the GetHash() method inside a lambda							
			if constexpr (Hashable<Decayed>) {
				meta->mHasher = [](const void* at) {
					auto instance = static_cast<const Decayed*>(at);
					return instance->GetHash();
				};
			}

			// Wrap the Do verb method inside a lambda							
			if constexpr (Dispatcher<Decayed>) {
				meta->mDispatcher = [](void* at, Flow::Verb& verb) {
					auto instance = static_cast<Decayed*>(at);
					instance->Do(verb);
				};
			}

			return meta.get();
		}
	}

   /// Set the list of bases for a given meta definition                      
   ///   @tparam Args... - all the bases                                      
	template<Dense... Args>
	void MetaData::SetBases(Args&&... items) noexcept requires (... && Same<Args, Base>) {
		mBases = {Forward<Base>(items)...};
	}

   /// Set the list of abilities for a given meta definition                  
   ///   @tparam Args... - all the abilities                                  
	template<Dense... Args>
	void MetaData::SetAbilities(Args&&... items) noexcept requires (... && Same<Args, Ability>) {
		mAbilities = {Forward<Ability>(items)...};
	}

   /// Set the list of members for a given meta definition                    
   ///   @tparam Args... - all the members                                    
	template<Dense... Args>
	void MetaData::SetMembers(Args&&... items) noexcept requires (... && Same<Args, Member>) {
		mMembers = {Forward<Member>(items)...};
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
		return GetBase(MetaData::Of<T>(), offset, base);
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
	bool MetaData::InterpretsAs(DMeta other) const {
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
	bool MetaData::InterpretsAs() const {
		return InterpretsAs<ADVANCED>(MetaData::Of<T>());
	}

	/// Check if this type interprets as an exact number of another without		
	/// conversion																					
	///	@param other - the type to try interpreting as								
	///	@param count - the number of items to interpret as							
	///	@return true if this type interprets as other								
	inline bool MetaData::InterpretsAs(DMeta other, Count count) const {
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
	bool MetaData::InterpretsAs(Count count) const {
		return InterpretsAs(MetaData::Of<T>(), count);
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
		return Is(MetaData::Of<T>());
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

