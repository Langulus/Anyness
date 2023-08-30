///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Neat.hpp"
#include "Construct.hpp"

namespace Langulus::Anyness
{

   /// Copy-constructor                                                       
   ///   @param other - neat container to shallow-copy                        
   LANGULUS(INLINED)
   Neat::Neat(const Neat& other)
      : Neat {Copy(other)} {}

   /// Move-constructor                                                       
   ///   @param other - neat container to move                                
   LANGULUS(INLINED)
   Neat::Neat(Neat&& other) noexcept
      : Neat {Move(other)} {}

   /// Semantic constructor from Neat                                         
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - the container to use                                  
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Neat::Neat(S&& other) requires (CT::Neat<TypeOf<S>>)
      : mHash {other->mHash}
      , mTraits {S::Nest(other->mTraits)}
      , mConstructs {S::Nest(other->mConstructs)}
      , mAnythingElse {S::Nest(other->mAnythingElse)} {
      // Reset remote hash if moving                                    
      if constexpr (S::Move and S::Keep)
         other->mHash = {};
   }

   /// Copy-constructor from anything messy                                   
   ///   @tparam T - messy type to use (deducible)                            
   ///   @param messy - the messy thing to copy                               
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Neat::Neat(const T& messy) requires (not CT::Neat<T>)
      : Neat {Copy(messy)} {}

   /// Copy-constructor from anything messy                                   
   ///   @tparam T - messy type to use (deducible)                            
   ///   @param messy - the messy thing to copy                               
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Neat::Neat(T& messy) requires (not CT::Neat<T>)
      : Neat {Copy(messy)} {}

   /// Move-constructor from anything messy                                   
   ///   @tparam T - messy type to use (deducible)                            
   ///   @param messy - the messy thing to move                               
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Neat::Neat(T&& messy) requires (not CT::Neat<T>)
      : Neat {Move(messy)} {}

   /// Semantic constructor from anything messy                               
   /// If container, it compiles it, by grouping elements in predictable      
   /// ways, ensuring further comparisons are fast & orderless. Nested        
   /// contents are normalized only if deep                                   
   ///   @param messy - the messy stuff to normalize                          
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Neat::Neat(S&& messy) requires (not CT::Neat<TypeOf<S>>) {
      using T = TypeOf<S>;

      if constexpr (CT::Block<T>) {
         // We'll be compiling a messy container                        
         messy->ForEachDeep([this](const Messy& group) {
            if (group.IsOr())
               TODO();

            if (group.ForEach(
               [this](const Trait& trait) {
                  AddTrait(Copy(trait));
               },
               [this](const MetaData* type) {
                  // Insert an empty Construct to signify solo type ID  
                  mConstructs[type] << Inner::DeConstruct {};
               },
               [this](const MetaTrait* type) {
                  // Insert empty Any to signify trait without content  
                  mTraits[type] << Any {};
               },
               [this](const MetaConst* type) {
                  // Expand the constant, then normalize, and merge it  
                  Any wrapped = Block {{}, type};

                  // Clone it, so that we take authority over the data  
                  Any cloned = Clone(wrapped);
                  Merge(Neat {cloned});
               },
               [this](const Construct& construct) {
                  // Construct's arguments are always Neat              
                  mConstructs[construct.GetType()] << Inner::DeConstruct {
                     construct.GetHash(),
                     construct.GetCharge(),
                     construct.GetArgument()
                  };
               }
            )) return;

            // If reached, just propagate the block without changing it 
            // But still sort it by block's origin type                 
            if (group.GetType())
               mAnythingElse[group.GetType()->mOrigin] << group;
         });
      }
      else operator << (messy.Forward());
   }
   
   /// Pack any number of elements sequentially                               
   /// If any of the types doesn't match exactly, the container becomes deep  
   /// to incorporate all elements                                            
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   template<CT::Data HEAD, CT::Data... TAIL>
   Neat::Neat(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1) {
      operator << (Forward<HEAD>(head));
      (operator << (Forward<TAIL>(tail)), ...);
   }

   /// Semantic assignment with another normalized descriptor                 
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - normalized descriptor to assign                       
   ///   @return a reference to this descriptor                               
   LANGULUS(INLINED)
   Neat& Neat::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      static_assert(CT::Neat<TypeOf<S>>, "S type must be Neat");

      mTraits = S::Nest(other->mTraits);
      mConstructs = S::Nest(other->mConstructs);
      mAnythingElse = S::Nest(other->mAnythingElse);
      mHash = other->mHash;

      // Reset remote hash if moving                                    
      if constexpr (S::Move and S::Keep)
         other->mHash = {};
      return *this;
   }

   /// Clear the container without deallocating                               
   inline void Neat::Clear() {
      mHash = {};
      mTraits.Clear();
      mConstructs.Clear();
      mAnythingElse.Clear();
   }
   
   /// Clear and deallocate the container                                     
   inline void Neat::Reset() {
      mHash = {};
      mTraits.Reset();
      mConstructs.Reset();
      mAnythingElse.Reset();
   }

   /// Turn the neat container to a messy one                                 
   ///   @return the messy container                                          
   inline Messy Neat::MakeMessy() const {
      // Un-neat and push all traits                                    
      TAny<Trait> traits;
      for (auto pair : mTraits) {
         for (auto& data : pair.mValue) {
            if (data.Is<Neat>())
               traits << Trait::From(pair.mKey, data.Get<Neat>().MakeMessy());
            else
               traits << Trait::From(pair.mKey, data);
         }
      }
      
      // Un-neat and push all constructs                                
      TAny<Construct> constructs;
      for (auto pair : mConstructs) {
         for (auto& construct : pair.mValue) {
            if (construct.mData.Is<Neat>()) {
               constructs << Construct {
                  pair.mKey,
                  construct.mData.Get<Neat>().MakeMessy(),
                  construct.mCharge
               };
            }
            else {
               constructs << Construct {
                  pair.mKey,
                  construct.mData,
                  construct.mCharge
               };
            }
         }
      }
      
      // Un-neat and push all the rest                                  
      Messy result;
      for (auto pair : mAnythingElse) {
         if (mAnythingElse.GetCount() == 1)
            result = pair.mValue;
         else
            result << pair.mValue;
      }

      if (traits) {
         if (result)
            result << Abandon(traits);
         else
            result = Abandon(traits);
      }

      if (constructs) {
         if (result)
            result << Abandon(constructs);
         else
            result = Abandon(constructs);
      }

      return Abandon(result);
   }

   /// Convert all neat nested data to a Construct of the given type          
   ///   @tparam T - type of the construct                                    
   ///   @return a new construct                                              
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct Neat::MakeConstruct() const {
      return Construct::From<T>(MakeMessy());
   }

   /// Get the hash of a neat container (and cache it)                        
   ///   @return the hash                                                     
   LANGULUS(INLINED)
   Hash Neat::GetHash() const {
      if (mHash)
         return mHash;

      // Cache hash so we don't recompute it all the time               
      mHash = HashOf(mTraits, mConstructs, mAnythingElse);
      return mHash;
   }

   /// Check if the container is empty                                        
   ///   @return true if empty                                                
   LANGULUS(INLINED)
   constexpr bool Neat::IsEmpty() const noexcept {
      return mTraits.IsEmpty()
         and mConstructs.IsEmpty()
         and mAnythingElse.IsEmpty();
   }
   
   /// Check if the container has missing entries                             
   ///   @return true if there's at least one missing entry                   
   LANGULUS(INLINED)
   constexpr bool Neat::IsMissing() const {
      // Buckets are flattened anyways, so same as IsMissingDeep        
      return IsMissingDeep();
   }
   
   /// Check if the container has missing entries, nest-scan                  
   ///   @return true if there's at least one missing entry                   
   LANGULUS(INLINED)
   constexpr bool Neat::IsMissingDeep() const {
      return mTraits.IsMissingDeep()
          or mConstructs.IsMissingDeep()
          or mAnythingElse.IsMissingDeep();
   }

   /// Check if the container is not empty                                    
   ///   @return true if not empty                                            
   LANGULUS(INLINED)
   constexpr Neat::operator bool() const noexcept {
      return not IsEmpty();
   }

   /// Compare neat container                                                 
   ///   @attention order matters only for data and traits of the same type   
   ///   @param rhs - the container to compare with                           
   ///   @return true if descriptors match                                    
   LANGULUS(INLINED)
   bool Neat::operator == (const Neat& rhs) const {
      if (GetHash() != rhs.GetHash())
         return false;

      return mTraits == rhs.mTraits
         and mConstructs == rhs.mConstructs
         and mAnythingElse == rhs.mAnythingElse;
   }

   /// Merge two neat containers                                              
   ///   @param rhs - the container to merge                                  
   LANGULUS(INLINED)
   void Neat::Merge(const Neat& rhs) {
      mTraits += rhs.mTraits;
      mConstructs += rhs.mConstructs;
      mAnythingElse += rhs.mAnythingElse;

      // Rehash                                                         
      mHash = HashOf(mTraits, mConstructs, mAnythingElse);
   }

   /// Get list of traits, corresponding to a static trait                    
   ///   @tparam T - trait type to search for                                 
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   template<CT::Trait T>
   LANGULUS(INLINED)
   TAny<Any>* Neat::GetTraits() {
      return GetTraits(T::GetTrait());
   }

   /// Get list of traits, corresponding to a static trait (const)            
   ///   @tparam T - trait type to search for                                 
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   template<CT::Trait T>
   LANGULUS(INLINED)
   const TAny<Any>* Neat::GetTraits() const {
      return GetTraits(T::GetTrait());
   }
   
   /// Get list of traits, corresponding to a type                            
   ///   @param t - trait type to search for                                  
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   LANGULUS(INLINED)
   TAny<Any>* Neat::GetTraits(TMeta t) {
      LANGULUS_ASSUME(UserAssumes, t, "Can't get invalid trait");
      auto found = mTraits.Find(t);
      if (not found)
         return nullptr;
      
      return &mTraits.GetValue(found);
   }

   /// Get list of traits, corresponding to a type (const)                    
   ///   @param t - trait type to search for                                  
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   LANGULUS(INLINED)
   const TAny<Any>* Neat::GetTraits(TMeta t) const {
      return const_cast<Neat*>(this)->GetTraits(t);
   }
   
   /// Get list of data, corresponding to a static type                       
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T>
   LANGULUS(INLINED)
   TAny<Messy>* Neat::GetData() {
      return GetData(MetaData::Of<Decay<T>>());
   }

   /// Get list of data, corresponding to a static type (const)               
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Messy>* Neat::GetData() const {
      return GetData(MetaData::Of<Decay<T>>());
   }
      
   /// Get list of data, corresponding to a type                              
   ///   @param d - type to search for                                        
   ///   @return the data list, or nullptr if no such list exists             
   LANGULUS(INLINED)
   TAny<Messy>* Neat::GetData(DMeta d) {
      LANGULUS_ASSUME(UserAssumes, d, "Can't get invalid data");
      auto found = mAnythingElse.Find(d);
      if (not found)
         return nullptr;
      
      return &mAnythingElse.GetValue(found);
   }

   /// Get list of data, corresponding to a type (const)                      
   ///   @param d - type to search for                                        
   ///   @return the data list, or nullptr if no such list exists             
   LANGULUS(INLINED)
   const TAny<Messy>* Neat::GetData(DMeta d) const {
      return const_cast<Neat*>(this)->GetData(d);
   }

   /// Get list of constructs, corresponding to a static type                 
   ///   @tparam T - type to search for                                       
   ///   @return the construct list, or nullptr if no such list exists        
   template<CT::Data T>
   LANGULUS(INLINED)
   TAny<Inner::DeConstruct>* Neat::GetConstructs() {
      return GetConstructs(MetaData::Of<Decay<T>>());
   }

   /// Get list of constructs, corresponding to a static type (const)         
   ///   @tparam T - type to search for                                       
   ///   @return the construct list, or nullptr if no such list exists        
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Inner::DeConstruct>* Neat::GetConstructs() const {
      return GetConstructs(MetaData::Of<Decay<T>>());
   }
   
   /// Get list of constructs, corresponding to a type                        
   ///   @param d - type to search for                                        
   ///   @return the construct list, or nullptr if no such list exists        
   LANGULUS(INLINED)
   TAny<Inner::DeConstruct>* Neat::GetConstructs(DMeta d) {
      LANGULUS_ASSUME(UserAssumes, d, "Can't get invalid construct");
      auto found = mConstructs.Find(d);
      if (not found)
         return nullptr;
      
      return &mConstructs.GetValue(found);
   }

   /// Get list of constructs, corresponding to a type (const)                
   ///   @param d - type to search for                                        
   ///   @return the construct list, or nullptr if no such list exists        
   LANGULUS(INLINED)
   const TAny<Inner::DeConstruct>* Neat::GetConstructs(DMeta d) const {
      return const_cast<Neat*>(this)->GetConstructs(d);
   }

   /// Set a default trait, if such wasn't already set                        
   ///   @tparam T - trait to set                                             
   ///   @param value - the value to assign                                   
   template<CT::Trait T>
   LANGULUS(INLINED)
   void Neat::SetDefaultTrait(CT::Data auto&& value) {
      auto found = GetTraits<T>();
      if (found and *found)
         return;

      *found = ::std::move(value);
   }

   /// Overwrite trait, or add a new one, if not already set                  
   ///   @tparam T - trait to set                                             
   ///   @param value - the value to assign                                   
   template<CT::Trait T>
   LANGULUS(INLINED)
   void Neat::OverwriteTrait(CT::Data auto&& value) {
      // Trait was found, overwrite it                                  
      mTraits[T::GetTrait()] = ::std::move(value);
   }

   /// Extract a trait from the descriptor                                    
   ///   @tparam T... - trait(s) we're searching for                          
   ///   @param values - [out] where to save the value, if found              
   ///   @return true if value changed                                        
   template<CT::Trait... T>
   LANGULUS(INLINED)
   bool Neat::ExtractTrait(CT::Data auto&... values) const {
      return (ExtractTraitInner<T>(values...) || ...);
   }
   
   /// Extract a trait from the descriptor                                    
   ///   @tparam T - trait we're searching for                                
   ///   @param values - [out] where to save the value, if found              
   ///   @return true if value changed                                        
   template<CT::Trait T>
   LANGULUS(INLINED)
   bool Neat::ExtractTraitInner(CT::Data auto&... values) const {
      auto found = GetTraits<T>();
      if (found) {
         return ExtractTraitInner(
            *found,
            ::std::make_integer_sequence<Offset, sizeof...(values)> {},
            values...
         );
      }
      return false;
   }

   ///                                                                        
   template<Offset... IDX>
   bool Neat::ExtractTraitInner(
      const TAny<Any>& found, 
      ::std::integer_sequence<Offset, IDX...>, 
      CT::Data auto&... values
   ) const {
      return (ExtractTraitInnerInner<IDX>(found, values) or ...);
   }
   
   ///                                                                        
   template<Offset IDX>
   bool Neat::ExtractTraitInnerInner(const TAny<Any>& found, CT::Data auto& value) const {
      if (IDX >= found.GetCount())
         return false;

      using D = Deref<decltype(value)>;
      if constexpr (CT::Deep<D>) {
         value = found[IDX];
         return true;
      }
      else try {
         value = found[IDX].AsCast<D>();
         return true;
      }
      catch (...) {}
      return false;
   }
   
   /// Extract data of an exact type, doing only pointer arithmetic           
   ///   @param value - [out] where to save the value(s), if found            
   ///   @return the number of extracted values (always 1 if not an array)    
   LANGULUS(INLINED)
   Count Neat::ExtractData(CT::Data auto& value) const {
      using D = Deref<decltype(value)>;
      if constexpr (CT::Array<D>) {
         // Fill a bounded array                                        
         auto found = GetData<Decay<D>>();
         if (found) {
            Count scanned = 0;
            for (auto& group : *found) {
               const auto toscan = ::std::min(ExtentOf<D> - scanned, group.GetCount());
               for (Offset i = 0; i < toscan; ++i) {
                  //TODO can be optimized-out for POD
                  value[scanned + i] = group.template Get<Deext<D>>(i);
               }

               scanned += toscan;
               if (scanned >= ExtentOf<D>)
                  return ExtentOf<D>;
            }

            return scanned;
         }
      }
      else {
         // Fill a single value                                         
         auto found = GetData<Decay<D>>();
         if (found) {
            value = (*found)[0].template Get<D>();
            return 1;
         }
      }

      return 0;
   }
   
   /// Push and sort anything non-semantic by a shallow-copy                  
   ///   @param rhs - the thing to push                                       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator << (const CT::NotSemantic auto& rhs) {
      return operator << (Copy(rhs));
   }

   /// Push and sort anything non-semantic by a shallow-copy                  
   ///   @param rhs - the thing to push                                       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator << (CT::NotSemantic auto& rhs) {
      return operator << (Copy(rhs));
   }

   /// Push and sort anything non-semantic by a move                          
   ///   @param rhs - the thing to push                                       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator << (CT::NotSemantic auto&& rhs) {
      return operator << (Move(rhs));
   }

   /// Push and sort anything semantically                                    
   ///   @param rhs - the thing to push, as well as the semantic to use       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator << (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         // Insert trait to its bucket                                  
         AddTrait(rhs.Forward());
      }
      else if constexpr (CT::Same<T, MetaData>) {
         // Insert an empty Construct to signify solo type ID           
         mConstructs[SparseCast(*rhs)] << Inner::DeConstruct {};
      }
      else if constexpr (CT::Same<T, MetaTrait>) {
         // Insert empty Any to signify trait without content           
         mTraits[SparseCast(*rhs)] << Any {};
      }
      else if constexpr (CT::Same<T, MetaConst>) {
         // Expand the constant, then normalize, and merge it           
         Any wrapped = Block {{}, SparseCast(*rhs)};

         // Clone it, so that we take authority over the data           
         Any cloned = Clone(wrapped);
         Merge(Neat {cloned});
      }
      else if constexpr (CT::Construct<T>) {
         // Construct's arguments are always Neat                       
         mConstructs[rhs->GetType()] << Inner::DeConstruct {
            rhs->GetHash(),
            rhs->GetCharge(),
            rhs->GetArgument()
         };
      }
      else if constexpr (CT::Deep<T>) {
         // Push anything deep here, flattening it, unless it is OR     
         if (rhs->IsOr())
            mAnythingElse[MetaData::Of<Any>()] << rhs.template Forward<Any>();
         else {
            rhs->ForEach([&](const Any& group) {
               operator << (S::Nest(const_cast<Any&>(group)));
            });
         }
      }
      else mAnythingElse[MetaData::Of<Decay<T>>()] << rhs.Forward();

      // Demand a new hash on the next compare                          
      mHash = {};
      return *this;
   }

   /// Merge anything non-semantic by a shallow-copy, if it doesn't exist yet 
   ///   @param rhs - the thing to push                                       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator <<= (const CT::NotSemantic auto& rhs) {
      return operator <<= (Copy(rhs));
   }

   /// Merge anything non-semantic by a shallow-copy, if it doesn't exist yet 
   ///   @param rhs - the thing to push                                       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator <<= (CT::NotSemantic auto& rhs) {
      return operator <<= (Copy(rhs));
   }

   /// Merge anything non-semantic by a move, if it doesn't exist yet         
   ///   @param rhs - the thing to push                                       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator <<= (CT::NotSemantic auto&& rhs) {
      return operator <<= (Move(rhs));
   }

   /// Merge anything semantically, if it doesn't exist yet                   
   ///   @param rhs - the thing to push, as well as the semantic to use       
   ///   @return a reference to this Neat container                           
   Neat& Neat::operator <<= (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         // Check if the trait already exists, before pushing it        
         if (not GetTraits(rhs->GetTrait()))
            return operator << (rhs.Forward());
      }
      else if constexpr (CT::Same<T, MetaTrait>) {
         // Check if the trait already exists, before pushing it        
         if (not GetTraits(SparseCast(*rhs)))
            return operator << (rhs.Forward());
      }
      else if constexpr (CT::Construct<T>) {
         // Check if the construct already exists, before pushing it    
         if (not GetConstructs(rhs->GetType()))
            return operator << (rhs.Forward());
      }
      else if constexpr (CT::Same<T, MetaData>) {
         // Check if the construct already exists, before pushing it    
         if (not GetConstructs(SparseCast(*rhs)))
            return operator << (rhs.Forward());
      }
      else if constexpr (CT::Deep<T>) {
         // Check anything deep here, flattening it, unless it is OR    
         if (rhs->IsOr() and not GetData<T>())
            return operator << (rhs.Forward());
         else {
            rhs->ForEach([&](const Any& group) {
               if (not GetData(group.GetType()->mOrigin))
                  operator << (S::Nest(const_cast<Any&>(group)));
            });
         }
      }
      else {
         // Check anything else                                         
         if (not GetData<T>())
            return operator << (rhs.Forward());
      }

      return *this;
   }

   /// Set a tagged argument inside constructor                               
   ///   @param trait - trait to set                                          
   ///   @param index - the index we're interested with if repeated           
   ///   @return a reference to this construct for chaining                   
   inline Neat& Neat::Set(const Trait& trait, const Offset& index) {
      auto found = mTraits.Find(trait.GetTrait());
      if (found) {
         // A group of similar traits was found                         
         auto& group = mTraits.GetValue(found);
         if (group.GetCount() > index)
            group[index] = static_cast<const Any&>(trait);
         else
            group << static_cast<const Any&>(trait);
      }
      else {
         // If reached, a new trait group to be inserted                
         mTraits[trait.GetTrait()] << static_cast<const Any&>(trait);
      }

      mHash = {};
      return *this;
   }

   /// Push a trait to the appropriate bucket                                 
   ///   @attention this is an inner function that doesn't affect the hash    
   ///   @tparam S - the semantic to use for the insertion (deducible)        
   ///   @param messy - the trait and semantic to use                         
   ///   @return a reference to this container                                
   template<CT::Semantic S>
   LANGULUS(INLINED)
   void Neat::AddTrait(S&& messy) requires (CT::TraitBased<TypeOf<S>>) {
      // Normalize trait contents and push sort it by its               
      // trait type                                                     
      if (messy->IsDeep())
         mTraits[messy->GetTrait()] << Neat {messy.template Forward<Any>()};
      else
         mTraits[messy->GetTrait()] << messy.template Forward<Any>();
   }

   /// Get a tagged argument inside constructor                               
   ///   @param meta - trait to search for                                    
   ///   @param index - the index we're interested in, if repeated            
   ///   @return selected data or nullptr if none was found                   
   ///   @attention if not nullptr, returned Any might contain a Neat         
   inline const Any* Neat::Get(TMeta meta, const Offset& index) const {
      auto found = mTraits.Find(meta);
      if (found) {
         auto& group = mTraits.GetValue(found);
         if (group.GetCount() > index) {
            // Found                                                    
            return &group[index];
         }
      }

      // Not found                                                      
      return nullptr;
   }

   /// Get traits from constructor                                            
   ///   @tparam T - the type of trait to search for                          
   ///   @return selected data or nullptr if none was found                   
   ///   @attention if not nullptr, returned Any might contain a Neat         
   template<CT::Trait T>
   LANGULUS(INLINED)
   const Any* Neat::Get(const Offset& index) const {
      return Get(T::GetTrait(), index);
   }

   /// Iterate through all relevant bucketed items                            
   /// Depending on F's arguments, different portions of the container will   
   /// be iterated. Use a generic Block/Any type to iterate everything.       
   ///   @attention since Trait and Construct are disassembled when inserted  
   ///      in this container, temporary instances will be created on the     
   ///      stack when iterated. If MUTABLE is true, any changes to these     
   ///      temporary instances will be used to overwite the real contents.   
   ///   @tparam F - the function signature (deducible)                       
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @param call - the function to execute for each element               
   ///   @return the number of executions of 'call'                           
   template<bool MUTABLE, class... F>
   Count Neat::ForEach(F&&... call) {
      return (... or ForEachInner<MUTABLE>(Forward<F>(call)));
   }

   ///                                                                        
   template<class... F>
   Count Neat::ForEach(F&&... call) const {
      return const_cast<Neat*>(this)->template 
         ForEach<false>(Forward<F>(call)...);
   }
   
   /// Iterate through all relevant bucketed items                            
   /// Depending on F's arguments, different portions of the container will   
   /// be iterated. Use a generic Block/Any type to iterate everything.       
   ///   @attention since Trait and Construct are disassembled when inserted  
   ///      in this container, temporary instances will be created on the     
   ///      stack when iterated. If MUTABLE is true, any changes to these     
   ///      temporary instances will be used to overwite the real contents.   
   ///   @tparam F - the function signature (deducible)                       
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @param call - the function to execute for each element               
   ///   @return the number of executions of 'call'                           
   template<bool MUTABLE, class F>
   Count Neat::ForEachInner(F&& call) {
      using A = ArgumentOf<F>;

      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant Neat block");

      if constexpr (CT::Deep<A>) {
         // Iterate everything                                          
         Count counter = 0;
         counter += ForEachTrait<MUTABLE>(call);
         counter += ForEachConstruct<MUTABLE>(call);
         counter += ForEachTail<MUTABLE>(call);
         return counter;
      }
      else if constexpr (CT::TraitBased<A>) {
         // Iterate traits only                                         
         return ForEachTrait<MUTABLE>(Forward<F>(call));
      }
      else if constexpr (CT::Construct<A>) {
         // Iterate constructs only                                     
         return ForEachConstruct<MUTABLE>(Forward<F>(call));
      }
      else {
         // Iterate anything else inside the tail                       
         return ForEachTail<MUTABLE>(Forward<F>(call));
      }
   }

   ///                                                                        
   template<class F>
   Count Neat::ForEachInner(F&& call) const {
      return const_cast<Neat*>(this)->template 
         ForEach<false>(Forward<F>(call));
   }

   /// Iterate all traits                                                     
   /// You can provide a static Trait iterator, to filter based on trait type 
   ///   @attention if F's argument is a generic Block/Any type, the trait    
   ///      will be wrapped in it                                             
   ///   @attention since Trait is disassembled upon insertion in this        
   ///      container, temporary instances will be created on the stack when  
   ///      iterated. If iterator is mutable, any changes to these temporary  
   ///      instances will be used to overwite the real contents.             
   ///   @tparam F - the function signature (deducible)                       
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @param call - the function to execute for each trait                 
   ///   @return the number of executions of 'call'                           
   template<bool MUTABLE, class F>
   Count Neat::ForEachTrait(F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::TraitBased<A> or CT::Deep<A>,
         "Iterator must be either trait-based or deep");
      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant Neat block");

      if constexpr (CT::Trait<A>) {
         // Static trait provided, extract filter                       
         const auto filter = Decay<A>::GetTrait();
         const auto found = mTraits.Find(filter);
         if (not found)
            return 0;

         // Iterate all relevant traits                                 
         Count index {};
         for (auto& data : mTraits.GetValue(found)) {
            // Create a temporary trait                                 
            Decay<A> temporaryTrait {data};

            if constexpr (CT::Bool<R>) {
               // If F returns bool, you can decide when to break the   
               // loop by simply returning Flow::Break (or just false)  
               if (not call(temporaryTrait)) {
                  if constexpr (CT::Mutable<A>) {
                     // Make sure change is commited before proceeding  
                     data = static_cast<const Any&>(temporaryTrait);
                  }
                  return index + 1;
               }
            }
            else {
               call(temporaryTrait);

               if constexpr (CT::Mutable<A>) {
                  // Make sure change is commited before proceeding     
                  data = static_cast<const Any&>(temporaryTrait);
               }
            }

            ++index;
         }

         return index;
      }

      // Iterate all traits                                             
      Count index {};
      for (auto group : mTraits) {
         for (auto& data : group.mValue) {
            // Create a temporary trait                                 
            Conditional<CT::Deep<A>, Any, Trait> temporaryTrait
               = Trait::From(group.mKey, data);

            if constexpr (CT::Bool<R>) {
               // If F returns bool, you can decide when to break the   
               // loop by simply returning Flow::Break (or just false)  
               if (not call(temporaryTrait)) {
                  if constexpr (CT::Mutable<A>) {
                     // Make sure change is commited before proceeding  
                     if constexpr (CT::Deep<A>)
                        data = static_cast<const Any&>(temporaryTrait.template Get<Trait>());
                     else
                        data = static_cast<const Any&>(temporaryTrait);
                  }
                  return index + 1;
               }
            }
            else {
               call(temporaryTrait);

               if constexpr (CT::Mutable<A>) {
                  // Make sure change is commited before proceeding     
                  if constexpr (CT::Deep<A>)
                     data = static_cast<const Any&>(temporaryTrait.template Get<Trait>());
                  else
                     data = static_cast<const Any&>(temporaryTrait);
               }
            }

            ++index;
         }
      }

      return index;
   }

   ///                                                                        
   template<class F>
   Count Neat::ForEachTrait(F&& call) const {
      return const_cast<Neat*>(this)->template
         ForEachTrait<false>(Forward<F>(call));
   }

   /// Iterate all constructs                                                 
   ///   @attention if F's argument is a generic Block/Any type, construct    
   ///      will be wrapped in it                                             
   ///   @attention since Construct is disassembled upon insertion in this    
   ///      container, temporary instances will be created on the stack when  
   ///      iterated. If iterator is mutable, any changes to these temporary  
   ///      instances will be used to overwite the real contents.             
   ///   @tparam F - the function signature (deducible)                       
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @param call - the function to execute for each construct             
   ///   @return the number of executions of 'call'                           
   template<bool MUTABLE, class F>
   Count Neat::ForEachConstruct(F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Construct<A> or CT::Deep<A>,
         "Iterator must be either a Construct or deep");
      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant Neat block");

      // Iterate all constructs                                         
      Count index {};
      for (auto group : mConstructs) {
         for (auto& data : group.mValue) {
            // Create a temporary construct                             
            Conditional<CT::Deep<A>, Any, Construct> temporaryConstruct
               = Construct {group.mKey, data.mData, data.mCharge};

            if constexpr (CT::Bool<R>) {
               // If F returns bool, you can decide when to break the   
               // loop by simply returning Flow::Break (or just false)  
               if (not call(temporaryConstruct)) {
                  if constexpr (CT::Mutable<A>) {
                     // Make sure change is commited before proceeding  
                     if constexpr (CT::Deep<A>) {
                        data.mHash = temporaryConstruct.template Get<Construct>().GetHash();
                        data.mData = temporaryConstruct.template Get<Construct>().GetArgument();
                        data.mCharge = temporaryConstruct.template Get<Construct>().GetCharge();
                     }
                     else {
                        data.mHash = temporaryConstruct.GetHash();
                        data.mData = temporaryConstruct.GetArgument();
                        data.mCharge = temporaryConstruct.GetCharge();
                     }
                  }

                  return index + 1;
               }
            }
            else {
               call(temporaryConstruct);

               if constexpr (CT::Mutable<A>) {
                  // Make sure change is commited before proceeding     
                  if constexpr (CT::Deep<A>) {
                     data.mHash = temporaryConstruct.template Get<Construct>().GetHash();
                     data.mData = temporaryConstruct.template Get<Construct>().GetArgument();
                     data.mCharge = temporaryConstruct.template Get<Construct>().GetCharge();
                  }
                  else {
                     data.mHash = temporaryConstruct.GetHash();
                     data.mData = temporaryConstruct.GetArgument();
                     data.mCharge = temporaryConstruct.GetCharge();
                  }
               }
            }

            ++index;
         }
      }

      return index;
   }

   ///                                                                        
   template<class F>
   Count Neat::ForEachConstruct(F&& call) const {
      return const_cast<Neat*>(this)->template
         ForEachConstruct<false>(Forward<F>(call));
   }

   /// Iterate all other types of data                                        
   /// You can provide a TAny iterator, to filter based on data type          
   ///   @tparam F - the function signature (deducible)                       
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @param call - the function to execute for each block                 
   ///   @return the number of executions of 'call'                           
   template<bool MUTABLE, class F>
   Count Neat::ForEachTail(F&& call) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;

      static_assert(CT::Constant<A> or MUTABLE,
         "Non constant iterator for constant Neat block");

      if constexpr (CT::Deep<A> && CT::Typed<A>) {
         // Statically typed container provided, extract filter         
         const auto filter = MetaData::Of<Decay<TypeOf<A>>>;
         const auto found = mAnythingElse.Find(filter);
         if (not found)
            return 0;

         // Iterate all relevant datas                                  
         Count index {};
         for (auto& data : mAnythingElse.GetValue(found)) {
            auto& dataTyped = reinterpret_cast<Decay<A>&>(data);
            if constexpr (CT::Bool<R>) {
               // If F returns bool, you can decide when to break the   
               // loop by simply returning Flow::Break (or just false)  
               if (not call(dataTyped))
                  return index + 1;
            }
            else call(dataTyped);

            ++index;
         }

         return index;
      }
      else if constexpr (CT::Deep<A>) {
         // Iterate all datas                                           
         Count index {};
         for (auto group : mAnythingElse) {
            for (auto& data : group.mValue) {
               if constexpr (CT::Bool<R>) {
                  // If F returns bool, you can decide when to break    
                  // the loop by returning Flow::Break (or just false)  
                  if (not call(data))
                     return index + 1;
               }
               else call(data);

               ++index;
            }
         }

         return index;
      }
      else {
         // Anything else                                               
         const auto filter = MetaData::Of<Decay<A>>();
         const auto found = mAnythingElse.Find(filter);
         if (not found)
            return 0;

         // Iterate all relevant datas                                  
         Count index {};
         for (auto& data : mAnythingElse.GetValue(found)) {
            auto& dataTyped = reinterpret_cast<TAny<Deref<A>>&>(data);
            for (auto& element : dataTyped) {
               if constexpr (CT::Bool<R>) {
                  // If F returns bool, you can decide when to break    
                  // the loop by returning Flow::Break (or just false)  
                  if (not call(element))
                     return index + 1;
               }
               else call(element);
            }

            ++index;
         }

         return index;
      }
   }

   ///                                                                        
   template<class F>
   Count Neat::ForEachTail(F&& call) const {
      return const_cast<Neat*>(this)->template
         ForEachTail<false>(Forward<F>(call));
   }

} // namespace Langulus::Anyness
