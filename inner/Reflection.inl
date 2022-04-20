#pragma once
#include "Reflection.hpp"
#include "NameOf.hpp"

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
			meta->mIsNamed = Named<T>;
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
		pcptr scanned = 0;
		for (auto& b : GetBaseList()) {
			// Check base																	
			if (base->Is(b.mStaticBase.mType)) {
				if (scanned == index) {
					baseOutput = b;
					return true;
				}
				else ++scanned;
			}

			// Dig deeper																	
			LinkedBase localBase;
			pcptr localIndex = 0;
			while (b.mBase->GetBase(base, localIndex, localBase)) {
				if (scanned == index) {
					// Propagate local base offset									
					localBase.mStaticBase.mLocalOffset += b.mStaticBase.mLocalOffset;

					// Multiply counts													
					localBase.mStaticBase.mCount *= b.mStaticBase.mCount;

					// Propagate mapping state if conditions allow				
					localBase.mStaticBase.mMapping = b.mStaticBase.mMapping
						&& localBase.mStaticBase.mMapping;

					// Propagate OR state if conditions allow						
					localBase.mStaticBase.mOr = b.mStaticBase.mOr
						|| localBase.mStaticBase.mOr;

					baseOutput = localBase;
					return true;
				}
				else ++scanned;
				++localIndex;
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
		for (auto& b : GetBaseList()) {
			if (base->Is(b.mStaticBase.mType) || b.mBase->HasBase(base))
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
		for (auto& v : GetAbilityList()) {
			if (v.mStaticAbility.mVerb == verb)
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
	inline bool MetaData::InterpretsAs(DMeta other) const {
		// Matching types always fit													
		if (Is(other))
			return true;

		// Different types might be compatible										
		// Check if this inherits other												
		if (IsChildOf(other)) {
			// This definition is derived from other								
			// Examples:																	
			//		R32 interprets as ANumber, because ANumber is base			
			//		Vec4 interprets as R32, because R32 is a mapping			
			return true;
		}

		// Do reverse inheritance check, in search of mappings				
		LinkedBase foundBase;
		if (IsSparse() && other->GetBase(this, 0, foundBase)) {
			// Other is derived from this												
			if (foundBase.mStaticBase.mMapping) {
				// Examples:																
				//		CGeneratorGeometry interprets as CSphere, because		
				//		the generator is mapped to CSphere exactly, despite	
				//		the inversed inheritance order								
				return true;
			}
			else {
				// Interpretation will later rely on a runtime resolve via	
				// the ClassBlock functionality -- it's anologous to an		
				// automated RTTI dynamic_cast										
				//	Examples:																
				//		AContext* is interpretable as AUnit* if runtime type	
				//		check passes, despite the inversed inheritance			
				return IsResolvable();
			}
		}

		// At this point we're pretty much sure that types						
		// aren't compatible																
		return false;
	}
	
	/// Check if this type interprets as another without conversion				
	///	@tparam T - the type to try interpreting as									
	///	@return true if this type interprets as other								
	template<ReflectedData T>
	bool MetaData::InterpretsAs() const {
		return InterpretsAs(MetaData::Of<T>());
	}

	/// Check if this type interprets as an exact number of another without		
	/// conversion																					
	///	@param other - the type to try interpreting as								
	///	@return true if this type interprets as other								
	inline bool MetaData::InterpretsAs(DMeta other, Count count) const {
		SAFETY(if (!other) throw Except::BadOperation(pcLogFuncError << "Bad type"));
		SAFETY(if (!count) throw Except::BadOperation(pcLogFuncError << "Bad count"));
		if (Is(other->GetID()) && count == 1)
			return true;

		// Do forward inheritance check and count matches						
		LinkedBase found;
		pcptr scanned = 0;
		while (GetBase(other, scanned, found)) {
			if (found.mStaticBase.mLocalOffset != 0) {
				// Base caused a memory gap, so early failure occurs			
				return false;
			}

			if ((other->IsAbstract() || found.mStaticBase.mMapping)
				&& count == found.mStaticBase.mCount)
				return true;
			scanned += found.mStaticBase.mCount;
		}

		if (scanned == count && !other->IsAbstract())
			return true;

		// At this point we're pretty much sure that types						
		// aren't compatible																
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
	inline MetaData::Distance MetaData::GetDistanceTo(DMeta) const {
		SAFETY(if (!other) throw Except::BadOperation(pcLogFuncError << "Bad type"));
		if (Is(other->GetID()))
			return 0;

		// Check bases																		
		constexpr auto maxDistance = std::numeric_limits<pcptr>::max();
		pcptr smallestDistance = maxDistance;
		for (auto& b : GetBaseList()) {
			if (b.mStaticBase.mOr)
				continue;

			auto d = b.mBase->GetDistanceTo(other);
			if (d != maxDistance && d + 1 < smallestDistance)
				smallestDistance = d + 1;
		}

		return smallestDistance;
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
	inline bool MetaData::Is(DMeta other) const {
		#if LANGULUS_FEATURE(MANAGED_REFLECTION)
			// This function is reduced to a pointer match, if the meta		
			// database is centralized, because it guarantees that definitions in		
			// separate translation units are always the same instance of MetaData		
			return this == other;
		#else
			return false;
		#endif
	}
	
	/// Check if two meta definitions match exactly										
	///	@tparam T - the type to compare against										
	///	@return true if types match														
	template<ReflectedData T>
	bool MetaData::Is() const {
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
	///	@tparam T - the trait to compare against										
	///	@return true if traits match														
	template<ReflectedTrait T>
	bool MetaTrait::Is() const {
		return Is(MetaTrait::Of<T>());
	}	
	
	
	///                                                                        
   ///   MetaVerb implementation																
   ///                                                                        
   
   /// Check if two meta definitions match exactly										
	///	@tparam T - the verb to compare against										
	///	@return true if verbs match														
	template<ReflectedVerb T>
	bool MetaVerb::Is() const {
		return Is(MetaVerb::Of<T>());
	}
	
} // namespace Langulus::Anyness

