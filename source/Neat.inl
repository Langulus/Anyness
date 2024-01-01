///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Neat.hpp"
#include "Trait.inl"
#include "Construct.inl"
#include "TUnorderedMap.inl"
#include "inner/Charge.inl"


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

   /// Semantic constructor                                                   
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - the container to use                                  
   template<template<class> class S> LANGULUS(INLINED)
   Neat::Neat(S<Neat>&& other) requires CT::Semantic<S<Neat>>
      : mHash {other->mHash}
      , mTraits {S<Neat>::Nest(other->mTraits)}
      , mConstructs {S<Neat>::Nest(other->mConstructs)}
      , mAnythingElse {S<Neat>::Nest(other->mAnythingElse)} {
      // Reset remote hash if moving                                    
      if constexpr (S<Neat>::Move and S<Neat>::Keep)
         other->mHash = {};
   }

   /// Tidy up any number of elements sequentially, each element can be       
   /// semantic or not. Deep contents are normalized only for CT::Deep        
   ///   @param t1 - first element                                            
   ///   @param tail... - the rest of the elements (optional)                 
   template<class T1, class...TAIL> LANGULUS(INLINED)
   Neat::Neat(T1&& t1, TAIL&&... tail)
   requires CT::Inner::UnfoldInsertable<T1, TAIL...> {
      Insert(Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Semantic assignment with another normalized descriptor                 
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - normalized descriptor to assign                       
   ///   @return a reference to this descriptor                               
   template<template<class> class S> LANGULUS(INLINED)
   Neat& Neat::operator = (S<Neat>&& other) requires CT::Semantic<S<Neat>> {
      mTraits = S<Neat>::Nest(other->mTraits);
      mConstructs = S<Neat>::Nest(other->mConstructs);
      mAnythingElse = S<Neat>::Nest(other->mAnythingElse);
      mHash = other->mHash;

      // Reset remote hash if moving                                    
      if constexpr (S<Neat>::Move and S<Neat>::Keep)
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

   /// Get the hash of a neat container (and cache it)                        
   ///   @attention Traits::Parent never participate in hashing/comparison    
   ///   @return the hash                                                     
   LANGULUS(INLINED)
   Hash Neat::GetHash() const {
      if (mHash)
         return mHash;

      // Traits::Parent never participates in the hash                  
      Hash traitHash {};
      for (auto pair : mTraits) {
         if (pair.mKey->Is<Traits::Parent>())
            continue;
         traitHash.mHash ^= Trait::From(pair.mKey, pair.mValue).GetHash().mHash;
      }

      // Cache hash so we don't recompute it all the time               
      mHash = HashOf(traitHash, mConstructs, mAnythingElse);
      return mHash;
   }

   /// Check if the container is empty                                        
   ///   @return true if empty                                                
   LANGULUS(INLINED)
   bool Neat::IsEmpty() const noexcept {
      return mTraits.IsEmpty()
         and mConstructs.IsEmpty()
         and mAnythingElse.IsEmpty();
   }
   
   /// Check if the container has missing entries                             
   ///   @return true if there's at least one missing entry                   
   LANGULUS(INLINED)
   bool Neat::IsMissing() const {
      // Buckets are flattened anyways, so same as IsMissingDeep        
      return IsMissingDeep();
   }
   
   /// Check if the container has missing entries, nest-scan                  
   ///   @return true if there's at least one missing entry                   
   LANGULUS(INLINED)
   bool Neat::IsMissingDeep() const {
      return mTraits.IsMissingDeep()
          or mConstructs.IsMissingDeep()
          or mAnythingElse.IsMissingDeep();
   }

   /// Check if the container is not empty                                    
   ///   @return true if not empty                                            
   LANGULUS(INLINED)
   Neat::operator bool() const noexcept {
      return not IsEmpty();
   }

   /// Compare neat container                                                 
   ///   @attention order matters only for data and traits of the same type   
   ///   @attention Traits::Parent are never compared                         
   ///   @param rhs - the container to compare with                           
   ///   @return true if descriptors match                                    
   LANGULUS(INLINED)
   bool Neat::operator == (const Neat& rhs) const {
      if (GetHash() != rhs.GetHash()
      or mTraits.GetCount() != rhs.mTraits.GetCount())
         return false;

      for (Offset i = 0; i < mTraits.GetCount(); ++i) {
         auto lp = mTraits.GetPair(i);
         auto rp = rhs.mTraits.GetPair(i);
         if (lp.mKey != rp.mKey)
            // Early failure on key mismatch                            
            return false;

         // Traits::Parent never participate in the comparison          
         if (lp.mKey->Is<Traits::Parent>())
            continue;

         if (lp.mValue != rp.mValue)
            // Early failure on value mismatch                          
            return false;
      }

      // If reached - traits match, so check the rest                   
      return mConstructs == rhs.mConstructs
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
   template<CT::Trait T> LANGULUS(INLINED)
   TAny<Any>* Neat::GetTraits() {
      return GetTraits(MetaTraitOf<T>());
   }

   /// Get list of traits, corresponding to a static trait (const)            
   ///   @tparam T - trait type to search for                                 
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   template<CT::Trait T> LANGULUS(INLINED)
   const TAny<Any>* Neat::GetTraits() const {
      return GetTraits(MetaTraitOf<T>());
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
   template<CT::Data T> LANGULUS(INLINED)
   TAny<Messy>* Neat::GetData() {
      return GetData(MetaDataOf<Decay<T>>());
   }

   /// Get list of data, corresponding to a static type (const)               
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T> LANGULUS(INLINED)
   const TAny<Messy>* Neat::GetData() const {
      return GetData(MetaDataOf<Decay<T>>());
   }
      
   /// Get list of data, corresponding to a type                              
   ///   @param d - type to search for                                        
   ///   @return the data list, or nullptr if no such list exists             
   LANGULUS(INLINED)
   TAny<Messy>* Neat::GetData(DMeta d) {
      auto found = mAnythingElse.Find(d ? d->mOrigin : nullptr);
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
   template<CT::Data T> LANGULUS(INLINED)
   TAny<Inner::DeConstruct>* Neat::GetConstructs() {
      return GetConstructs(MetaDataOf<Decay<T>>());
   }

   /// Get list of constructs, corresponding to a static type (const)         
   ///   @tparam T - type to search for                                       
   ///   @return the construct list, or nullptr if no such list exists        
   template<CT::Data T> LANGULUS(INLINED)
   const TAny<Inner::DeConstruct>* Neat::GetConstructs() const {
      return GetConstructs(MetaDataOf<Decay<T>>());
   }
   
   /// Get list of constructs, corresponding to a type                        
   ///   @param d - type to search for                                        
   ///   @return the construct list, or nullptr if no such list exists        
   LANGULUS(INLINED)
   TAny<Inner::DeConstruct>* Neat::GetConstructs(DMeta d) {
      auto found = mConstructs.Find(d ? d->mOrigin : nullptr);
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
   //TODO isn't this like simply merge?? also make merge test only by trait id when merging traits!
   ///   @tparam T - trait to set                                             
   ///   @param value - the value to assign                                   
   template<CT::Trait T> LANGULUS(INLINED)
   void Neat::SetDefaultTrait(CT::Data auto&& value) {
      auto found = GetTraits<T>();
      if (found and *found)
         return;

      *found = ::std::move(value);
   }

   /// Overwrite trait, or add a new one, if not already set                  
   ///   @tparam T - trait to set                                             
   ///   @param value - the value to assign                                   
   template<CT::Trait T> LANGULUS(INLINED)
   void Neat::OverwriteTrait(CT::Data auto&& value) {
      // Trait was found, overwrite it                                  
      mTraits[MetaTraitOf<T>()] = ::std::move(value);
   }

   /// Extract a trait from the descriptor                                    
   ///   @tparam T... - trait(s) we're searching for                          
   ///   @param values - [out] where to save the value, if found              
   ///   @return true if value changed                                        
   template<CT::Trait...T> LANGULUS(INLINED)
   bool Neat::ExtractTrait(CT::Data auto&... values) const {
      return (ExtractTraitInner<T>(values...) or ...);
   }
   
   /// Extract a trait from the descriptor                                    
   ///   @tparam T - trait we're searching for                                
   ///   @param values - [out] where to save the value, if found              
   ///   @return true if value changed                                        
   template<CT::Trait T> LANGULUS(INLINED)
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
   template<Offset...IDX>
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

   /// Push and sort anything semantically                                    
   ///   @attention hash will be recomputed on demand                         
   ///   @param item - the thing to push, as well as the semantic to use      
   LANGULUS(INLINED)
   void Neat::InsertInner(auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         // Insert trait to its bucket                                  
         AddTrait(S::Nest(item));
      }
      else if constexpr (CT::Same<T, DMeta>) {
         // Insert an empty data to signify solo type ID                
         AddData(S::Nest(item));
      }
      else if constexpr (CT::Same<T, TMeta>) {
         // Insert empty Any to signify trait without content           
         AddTrait(S::Nest(item));
      }
      else if constexpr (CT::Same<T, CMeta>) {
         // Expand the constant, and push it                            
         operator << (Clone(Block {{}, DesemCast(item)}));
      }
      else if constexpr (CT::Construct<T>) {
         // Construct's arguments are always Neat                       
         AddConstruct(S::Nest(item));
      }
      else {
         // RHS is nothing special, just add it as it is                
         const auto meta = MetaDataOf<Decay<T>>();
         const auto found = mAnythingElse.FindIt(meta);
         if (found)
            found->mValue << Messy {S::Nest(item)};
         else
            mAnythingElse.Insert(meta, TAny<Messy> {Messy {S::Nest(item)}});
      }

      // Demand a new hash on the next compare                          
      mHash = {};
   }

   /// Insert an element, array of elements, or another set                   
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   LANGULUS(INLINED)
   Count Neat::UnfoldInsert(auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;

      if constexpr (CT::Array<T>) {
         if constexpr (CT::StringLiteral<T>) {
            // Implicitly convert string literals to Text containers    
            InsertInner(Text {S::Nest(item)});
            return 1;
         }
         else {
            // Unfold-insert anything else                              
            Count inserted = 0;
            for (auto& key : item)
               inserted += UnfoldInsert(S::Nest(key));
            return inserted;
         }
      }
      else if constexpr (CT::Neat<T>) {
         // Insert Neat, by inserting each element from it              
         Count inserted = 0;
         DesemCast(item).ForEach([&](const Any& subitem) {
            inserted += UnfoldInsert(
               S::Nest(const_cast<Any&>(subitem)));
         });
         return inserted;
      }
      else if constexpr (CT::Deep<T>) {
         // Push anything deep here, flattening it, unless it has state 
         if (DesemCast(item).GetUnconstrainedState()) {
            // Item has state, so just push it as it is, to preserve it 
            InsertInner(S::Nest(item));
            return 1;
         }
         else if (DesemCast(item).IsDeep()) {
            // Item is deep, flatten it                                 
            Count inserted = 0;
            DesemCast(item).ForEach([&](const Any& subitem) {
               inserted += UnfoldInsert(
                  S::Nest(const_cast<Any&>(subitem)));
            });
            return inserted;
         }
         else {
            // Item is not deep, filter based on type                   
            const auto inserted = DesemCast(item).ForEach(
               [&](const Construct& c) {
                  InsertInner(S::Nest(const_cast<Construct&>(c)));
               },
               [&](const Neat& neat) {
                  UnfoldInsert(S::Nest(const_cast<Neat&>(neat)));
               },
               [&](const Trait& trait) {
                  InsertInner(S::Nest(const_cast<Trait&>(trait)));
               },
               [&](const DMeta& meta) {InsertInner(meta);},
               [&](const TMeta& meta) {InsertInner(meta);},
               [&](const CMeta& meta) {InsertInner(meta);}
            );

            if (not inserted) {
               // Item contains nothing special, just add it as it is   
               InsertInner(S::Nest(item));
               return 1;
            }

            return inserted;
         }
      }
      else {
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         InsertInner(S::Nest(item));
         return 1;
      }
   }

   /// Push and sort anything, semantically or not                            
   ///   @param key - the key to add                                          
   ///   @return 1 if pair was inserted, zero otherwise                       
   template<class T1, class... TAIL> LANGULUS(INLINED)
   Count Neat::Insert(T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
      inserted   += UnfoldInsert(Forward<T1>(t1));
      ((inserted += UnfoldInsert(Forward<TAIL>(tail))), ...);
      return inserted;
   }
   
   /// Push and sort anything, semantically or not                            
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   LANGULUS(INLINED)
   Neat& Neat::operator << (auto&& rhs) {
      Insert(Forward<Deref<decltype(rhs)>>(rhs));
      return *this;
   }

   /// Merge anything semantically, if it doesn't exist yet                   
   ///   @attention hash will be recomputed on demand, if anything was pushed 
   ///   @param rhs - the thing to push, as well as the semantic to use       
   ///   @return a reference to this Neat container                           
   LANGULUS(INLINED)
   Neat& Neat::operator <<= (auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;
      decltype(auto) rhsd = DesemCast(rhs);

      if constexpr (CT::TraitBased<T>) {
         // Check if the trait already exists, before pushing it        
         if (not GetTraits(rhsd.GetTrait()))
            return operator << (S::Nest(rhs));
      }
      else if constexpr (CT::Same<T, TMeta>) {
         // Check if the trait already exists, before pushing it        
         if (not GetTraits(rhsd))
            return operator << (S::Nest(rhs));
      }
      else if constexpr (CT::Construct<T>) {
         // Check if the construct already exists, before pushing it    
         if (not GetConstructs(rhsd.GetType()))
            return operator << (S::Nest(rhs));
      }
      else if constexpr (CT::Same<T, DMeta>) {
         // Check if the construct already exists, before pushing it    
         if (not GetData(rhsd))
            return operator << (S::Nest(rhs));
      }
      else if constexpr (CT::Deep<T>) {
         // Check anything deep here, flattening it, unless it is OR    
         if (rhsd.GetUnsconstrainedState())
            return operator << (S::Nest(rhs));

         rhsd.ForEach([&](const Any& group) {
            if (not GetData(group.GetType()))
               operator << (S::Nest(const_cast<Any&>(group)));
         });
      }
      else {
         // Check anything else                                         
         if (not GetData<T>())
            return operator << (S::Nest(rhs));
      }

      return *this;
   }
   
   /// Set a tagged argument inside constructor by moving                     
   ///   @attention hash will be recomputed on demand                         
   ///   @param trait - trait to set                                          
   ///   @param index - the index we're interested with if repeated           
   ///   @return a reference to this construct for chaining                   
   inline Neat& Neat::Set(CT::TraitBased auto&& trait, Offset index) {
      const auto meta = trait.GetTrait();
      auto found = mTraits.FindIt(meta);

      if (found) {
         // A group of similar traits was found                         
         auto& group = found->mValue;
         if (group.GetCount() > index)
            group[index] = Forward<Any>(trait);
         else
            group << Forward<Any>(trait);
      }
      else {
         // If reached, a new trait group to be inserted                
         mTraits.Insert(meta, Forward<Any>(trait));
      }

      mHash = {};
      return *this;
   }

   /// Push a trait to the appropriate bucket                                 
   ///   @attention this is an inner function that doesn't affect the hash    
   ///   @param messy - the trait (and semantic) to insert                    
   LANGULUS(INLINED)
   void Neat::AddTrait(auto&& messy) {
      using S = SemanticOf<decltype(messy)>;
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         // Insert a trait container                                    
         Any wrapper;
         if (messy->IsDeep())
            wrapper = Neat {messy.template Forward<Any>()};
         else
            wrapper = messy.template Forward<Any>();

         const auto meta = messy->GetTrait();
         auto found = mTraits.FindIt(meta);
         if (found)
            found->mValue << Abandon(wrapper);
         else
            mTraits.Insert(meta, TAny<Any> {Abandon(wrapper)});
      }
      else if constexpr (CT::Exact<T, TMeta>) {
         // Insert trait without contents                               
         auto trait = DesemCast(messy);
         auto found = mTraits.FindIt(trait);
         if (found)
            found->mValue << Any {};
         else
            mTraits.Insert(trait, TAny<Any> { Any {} });
      }
      else LANGULUS_ERROR("Can't insert trait");
   }
   
   /// Push data to the appropriate bucket                                    
   ///   @attention this is an inner function that doesn't affect the hash    
   ///   @param messy - the data (and semantic) to insert                     
   LANGULUS(INLINED)
   void Neat::AddData(auto&& messy) {
      using S = SemanticOf<decltype(messy)>;
      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         // Insert deep data - we have to flatten it                    
         const auto meta = messy->GetType()
            ? messy->GetType()->mOrigin : nullptr;
         auto found = mAnythingElse.FindIt(meta);
         if (found)
            found->mValue << messy.Forward();
         else
            mAnythingElse.Insert(meta, TAny<Messy> {messy.Forward()});
      }
      else if constexpr (CT::Exact<T, DMeta>) {
         // Insert data without contents                                
         auto meta = DesemCast(messy);
         auto dmeta = meta ? meta->mOrigin : nullptr;
         auto found = mAnythingElse.FindIt(dmeta);
         if (found)
            found->mValue << Any {};
         else
            mAnythingElse.Insert(dmeta, TAny<Messy> {Any {}});
      }
      else LANGULUS_ERROR("Can't insert data");
   }

   /// Push a construct to the appropriate bucket                             
   ///   @attention this is an inner function that doesn't affect the hash    
   ///   @param messy - the construct and semantic to use                     
   LANGULUS(INLINED)
   void Neat::AddConstruct(auto&& messy) {
      using S = SemanticOf<decltype(messy)>;
      using T = TypeOf<S>;

      if constexpr (CT::Construct<T>) {
         const auto meta = messy->GetType()
            ? messy->GetType()->mOrigin : nullptr;
         const auto found = mConstructs.FindIt(meta);
         if (found) {
            found->mValue << Inner::DeConstruct {
               messy->GetHash(),
               messy->GetCharge(),
               S::Nest(messy->GetDescriptor())
            };
         }
         else {
            mConstructs.Insert(meta, Inner::DeConstruct {
               messy->GetHash(),
               messy->GetCharge(),
               S::Nest(messy->GetDescriptor())
            });
         }
      }
      else LANGULUS_ERROR("Can't insert construct");
   }

   /// Get a tagged argument inside constructor                               
   ///   @param meta - trait to search for                                    
   ///   @param index - the index we're interested in, if repeated            
   ///   @return selected data or nullptr if none was found                   
   ///   @attention if not nullptr, returned Any might contain a Neat         
   inline const Any* Neat::Get(TMeta meta, Offset index) const {
      auto found = mTraits.FindIt(meta);
      if (found) {
         auto& group = found->mValue;
         if (group.GetCount() > index)
            return &group[index];
      }

      // Not found                                                      
      return nullptr;
   }

   /// Get traits from constructor                                            
   ///   @tparam T - the type of trait to search for                          
   ///   @return selected data or nullptr if none was found                   
   ///   @attention if not nullptr, returned Any might contain a Neat         
   template<CT::Trait T> LANGULUS(INLINED)
   const Any* Neat::Get(Offset index) const {
      return Get(MetaTraitOf<T>(), index);
   }

   /// Iterate through all relevant bucketed items                            
   /// Depending on F's arguments, different portions of the container will   
   /// be iterated. Use a generic Block/Any type to iterate everything.       
   ///   @attention since Trait and Construct are disassembled when inserted  
   ///      in this container, temporary instances will be created on the     
   ///      stack when iterated. If MUTABLE is true, any changes to these     
   ///      temporary instances will be used to overwite the real contents.   
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @tparam F - the function(s) signature(s) (deducible)                 
   ///   @param call - the function(s) to execute for each element            
   ///   @return the number of executions of 'call'                           
   template<bool MUTABLE, class... F>
   Count Neat::ForEach(F&&... call) {
      if (IsEmpty())
         return 0;

      Count result = 0;
      (void) (... or (0 != (result = ForEachInner<MUTABLE>(
         Forward<F>(call)
      ))));
      return result;
   }

   ///                                                                        
   template<class... F>
   Count Neat::ForEach(F&&... call) const {
      return const_cast<Neat*>(this)->template 
         ForEach<false>(Forward<F>(call)...);
   }
   
   /// Iterate through all relevant bucketed items, inclusively               
   ///   @attention since Trait and Construct are disassembled when inserted  
   ///      in this container, temporary instances will be created on the     
   ///      stack when iterated. If MUTABLE is true, any changes to these     
   ///      temporary instances will be used to overwite the real contents.   
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @tparam F - the function(s) signature(s) (deducible)                 
   ///   @param call - the function(s) to execute for each element            
   ///   @return the number of executions of all calls                        
   template<bool MUTABLE, class... F>
   Count Neat::ForEachDeep(F&&... call) {
      Count executions = 0;
      ((executions += ForEachInner<MUTABLE>(Forward<F>(call))), ...);
      return executions;
   }

   /// Neat containers are always flat, so deep iteration is same as flat one 
   template<class... F>
   Count Neat::ForEachDeep(F&&... call) const {
      return const_cast<Neat*>(this)->template
         ForEachDeep<false>(Forward<F>(call)...);
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
      static_assert(CT::Constant<Deptr<A>> or MUTABLE,
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
   ///   @tparam MUTABLE - whether changes inside container are allowed       
   ///   @tparam F - the function signature (deducible)                       
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

      Count index = 0;
      if constexpr (CT::Trait<A>) {
         // Static trait provided, extract filter                       
         const auto filter = MetaTraitOf<Decay<A>>();
         const auto found = mTraits.FindIt(filter);
         if (not found)
            return index;

         // Iterate all relevant traits                                 
         for (auto& data : found->mValue) {
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
      }
      else {
         // Iterate all traits                                          
         for (auto group : mTraits) {
            for (auto& data : group.mValue) {
               // Create a temporary trait                              
               Conditional<CT::Deep<A>, Any, Trait> temporaryTrait
                  = Trait::From(group.mKey, data);

               if constexpr (CT::Bool<R>) {
                  // If F returns bool, you can decide when to break    
                  // the loop by simply returning Flow::Break           
                  if (not call(temporaryTrait)) {
                     if constexpr (CT::Mutable<A>) {
                        // Make sure change is committed                
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
                        data.mData = temporaryConstruct.template Get<Construct>().GetDescriptor();
                        data.mCharge = temporaryConstruct.template Get<Construct>().GetCharge();
                     }
                     else {
                        data.mHash = temporaryConstruct.GetHash();
                        data.mData = temporaryConstruct.GetDescriptor();
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
                     data.mData = temporaryConstruct.template Get<Construct>().GetDescriptor();
                     data.mCharge = temporaryConstruct.template Get<Construct>().GetCharge();
                  }
                  else {
                     data.mHash = temporaryConstruct.GetHash();
                     data.mData = temporaryConstruct.GetDescriptor();
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
      static_assert(CT::Constant<Deptr<A>> or MUTABLE,
         "Non constant iterator for constant Neat block");

      if constexpr (CT::Deep<A> and CT::Typed<A>) {
         // Statically typed container provided, extract filter         
         const auto filter = MetaDataOf<Decay<TypeOf<A>>>;
         const auto found = mAnythingElse.FindIt(filter);
         if (not found)
            return 0;

         // Iterate all relevant datas                                  
         Count index {};
         for (auto& data : found->mValue) {
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
         const auto filter = MetaDataOf<Decay<A>>();
         const auto found = mAnythingElse.FindIt(filter);
         if (not found)
            return 0;

         // Iterate all relevant datas                                  
         Count index {};
         for (auto& data : found->mValue) {
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

   /// Remove data, that matches the given type                               
   ///   @tparam T - type of data to remove                                   
   ///   @tparam EMPTY_TOO - use true, to remove all empty data entries, that 
   ///                       are usually produced, by pushing DMeta           
   ///                       (disabled by default)                            
   ///   @return the number of removed data entries                           
   template<CT::Data T, bool EMPTY_TOO>
   Count Neat::RemoveData() {
      const auto filter = MetaDataOf<Decay<T>>();
      const auto found = mAnythingElse.FindIt(filter);
      if (not found)
         return 0;

      if constexpr (EMPTY_TOO) {
         // Remove everything                                           
         const auto count = found->mValue.GetCount();
         mAnythingElse.RemoveIt(found);
         return count;
      }

      Count count = 0;
      for (auto data : KeepIterator(found->mValue)) {
         if (not *data)
            continue;

         // Remove only matching data entries, that aren't empty        
         data = found->mValue.RemoveIt(data);
         ++count;
      }

      if (not found->mValue)
         mAnythingElse.RemoveIt(found);
      return count;
   }

   /// Remove constructs, that match the given type                           
   ///   @tparam T - type of construct to remove                              
   ///   @return the number of removed constructs                             
   template<CT::Data T>
   Count Neat::RemoveConstructs() {
      const auto filter = MetaDataOf<Decay<T>>();
      const auto found = mConstructs.FindIt(filter);
      if (not found)
         return 0;

      Count count = 0;
      for (auto data : KeepIterator(found->mValue)) {
         if (not *data)
            continue;

         data = found->mValue.RemoveIt(data);
         ++count;
      }

      if (not found->mValue)
         mConstructs.RemoveIt(found);
      return count;
   }

   /// Remove traits, that match the given trait type                         
   ///   @tparam T - type of trait to remove                                  
   ///   @tparam EMPTY_TOO - use true, to remove all empty trait entries, that
   ///                       are usually produced, by pushing TMeta           
   ///                       (disabled by default)                            
   ///   @return the number of removed trait entries                          
   template<CT::Trait T, bool EMPTY_TOO>
   Count Neat::RemoveTrait() {
      const auto filter = MetaTraitOf<T>();
      const auto found = mTraits.FindIt(filter);
      if (not found)
         return 0;

      if constexpr (EMPTY_TOO) {
         // Remove everything                                           
         const auto count = found->mValue.GetCount();
         mTraits.RemoveIt(found);
         return count;
      }

      Count count = 0;
      for (auto data : KeepIterator(found->mValue)) {
         if (not *data)
            continue;

         // Remove only matching trait entries, that aren't empty       
         data = found->mValue.RemoveIt(data);
         ++count;
      }

      if (not found->mValue)
         mTraits.RemoveIt(found);
      return count;
   }



   template<template<class> class S>
   Inner::DeConstruct::DeConstruct(Hash hash, const Charge& charge, S<Neat>&& data)
      : mHash {hash}
      , mCharge {charge}
      , mData {data.Forward()} {}

   template<template<class> class S> LANGULUS(INLINED)
   Inner::DeConstruct::DeConstruct(S<DeConstruct>&& other)
      : mHash {other->mHash}
      , mCharge {other->mCharge}
      , mData {S<Any> {other->mData}} {}

   LANGULUS(INLINED)
   Hash Inner::DeConstruct::GetHash() const noexcept {
      return mHash;
   }

   LANGULUS(INLINED)
   bool Inner::DeConstruct::operator == (const DeConstruct& rhs) const {
      return mHash == rhs.mHash
         and mCharge == rhs.mCharge
         and mData == rhs.mData;
   }

} // namespace Langulus::Anyness
