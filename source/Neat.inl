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

   /// Semantic initialization from another neat container                    
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - the container to use                                  
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Neat::Neat(S&& other)
      : mTraits {S::Nest(other->mTraits)}
      , mConstructs {S::Nest(other->mConstructs)}
      , mAnythingElse {S::Nest(other->mAnythingElse)}
      , mHash {other->mHash} {
      static_assert(CT::Exact<TypeOf<S>, Neat>, "S type must be Neat");

      // Reset remote hash if moving                                    
      if constexpr (S::Move && S::Keep)
         other->mHash = {};
   }

   /// Compile a messy container, by removing Traits::Parent, and grouping    
   /// elements in predictable ways, ensuring further comparisons are fast &  
   /// orderless. Nested contents are normalized only if deep                 
   ///   @param messy - the messy container to normalize                      
   LANGULUS(INLINED)
   Neat::Neat(const Messy& messy) {
      messy.ForEachDeep([this](const Messy& group) {
         if (group.IsOr())
            TODO();

         if (group.ForEach(
            [this](const Trait& trait) {
               // Always skip parent traits                             
               if (trait.TraitIs<Traits::Parent>())
                  return;
               
               // Normalize trait contents and push sort it by its      
               // trait type                                            
               if (trait.IsDeep())
                  mTraits[trait.GetTrait()] << Neat {trait};
               else
                  mTraits[trait.GetTrait()] << static_cast<const Any&>(trait);
            },
            [this](const MetaData* type) {
               // Insert an empty Construct to signify solo type ID     
               mConstructs[type] << DeConstruct {};
            },
            [this](const MetaTrait* type) {
               // Insert an empty Any to signify trait without content  
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
               // Construct's arguments are always Neat, just push them 
               mConstructs[construct.GetType()] << DeConstruct {
                  construct.GetHash(),
                  construct.GetCharge(),
                  construct.GetArgument()
               };
            }
         )) return;

         // If reached, just propagate the block without changing it    
         // But still sort it by block type                             
         mAnythingElse[group.GetType()] << group;
      });
   }

   /// Semantic assignment with another normalized descriptor                 
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - normalized descriptor to assign                       
   ///   @return a reference to this descriptor                               
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Neat& Neat::operator = (S&& other) {
      static_assert(CT::Exact<TypeOf<S>, Neat>, "S type must be Neat");

      mTraits = S::Nest(other->mTraits);
      mConstructs = S::Nest(other->mConstructs);
      mAnythingElse = S::Nest(other->mAnythingElse);
      mHash = other->mHash;

      // Reset remote hash if moving                                    
      if constexpr (S::Move && S::Keep)
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
            if (data.Is<Neat>()) {
               traits << Trait::From(
                  pair.mKey,
                  data.Is<Neat>()
                     ? data.Get<Neat>().MakeMessy()
                     : data
               );
            }
            else traits << Trait::From(pair.mKey, data);
         }
      }
      
      // Un-neat and push all constructs                                
      TAny<Construct> constructs;
      for (auto pair : mConstructs) {
         for (auto& construct : pair.mValue) {
            constructs << Construct {
               pair.mKey,
               construct.mData.Is<Neat>()
                  ? construct.mData.Get<Neat>().MakeMessy()
                  : construct.mData,
               construct.mCharge
            };
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

      if (traits)
         result << Abandon(traits);
      if (constructs)
         result << Abandon(constructs);
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

   /// Get list of traits, corresponding to a type                            
   ///   @tparam T - trait type to search for                                 
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   template<CT::Trait T>
   LANGULUS(INLINED)
   TAny<Any>* Neat::GetTraits() {
      auto found = mTraits.Find(T::GetTrait());
      if (not found)
         return nullptr;
      
      return &mTraits.GetValue(found);
   }

   /// Get list of traits, corresponding to a type (const)                    
   ///   @tparam T - trait type to search for                                 
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   template<CT::Trait T>
   LANGULUS(INLINED)
   const TAny<Any>* Neat::GetTraits() const {
      return const_cast<Neat*>(this)->template GetTraits<T>();
   }
   
   /// Get list of data, corresponding to a type                              
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T>
   LANGULUS(INLINED)
   TAny<Messy>* Neat::GetData() {
      auto found = mAnythingElse.Find(MetaData::Of<T>());
      if (not found)
         return nullptr;
      
      return &mAnythingElse.GetValue(found);
   }

   /// Get list of data, corresponding to a type (const)                      
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Messy>* Neat::GetData() const {
      return const_cast<Neat*>(this)->template GetData<T>();
   }

   /// Get list of constructs, corresponding to a type                        
   ///   @tparam T - type to search for                                       
   ///   @return the construct list, or nullptr if no such list exists        
   template<CT::Data T>
   LANGULUS(INLINED)
   TAny<Construct>* Neat::GetConstructs() {
      auto found = mConstructs.Find(MetaData::Of<T>());
      if (not found)
         return nullptr;
      
      return &mConstructs.GetValue(found);
   }

   /// Get list of constructs, corresponding to a type (const)                
   ///   @tparam T - type to search for                                       
   ///   @return the construct list, or nullptr if no such list exists        
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Construct>* Neat::GetConstructs() const {
      return const_cast<Neat*>(this)->template GetConstructs<T>();
   }

   /// Set a default trait, if such wasn't already set                        
   ///   @tparam T - trait to set                                             
   ///   @tparam D - type of data to set it to (deducible)                    
   ///   @param value - the value to assign                                   
   template<CT::Trait T, CT::Data D>
   LANGULUS(INLINED)
   void Neat::SetDefaultTrait(D&& value) {
      auto found = GetTraits<T>();
      if (found and *found)
         return;

      *found = Forward<D>(value);
   }

   /// Overwrite trait, or add a new one, if not already set                  
   ///   @tparam T - trait to set                                             
   ///   @tparam D - type of data to set it to (deducible)                    
   ///   @param value - the value to assign                                   
   template<CT::Trait T, CT::Data D>
   LANGULUS(INLINED)
   void Neat::OverwriteTrait(D&& value) {
      // Trait was found, overwrite it                                  
      mTraits[T::GetTrait()] = Forward<D>(value);
   }

   /// Extract a trait from the descriptor                                    
   ///   @tparam T - the trait we're searching for                            
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param values - [out] where to save the value, if found              
   ///   @return true if value changed                                        
   template<CT::Trait T, CT::Data... D>
   LANGULUS(INLINED)
   bool Neat::ExtractTrait(D&... values) const {
      auto found = GetTraits<T>();
      if (found) {
         return ExtractTraitInner(
            *found,
            ::std::make_integer_sequence<Offset, sizeof...(D)> {},
            values...
         );
      }
      return false;
   }

   ///                                                                        
   template<CT::Data... D, Offset... IDX>
   bool Neat::ExtractTraitInner(
      const TAny<Any>& found, 
      ::std::integer_sequence<Offset, IDX...>, 
      D&... values
   ) const {
      return (ExtractTraitInnerInner<IDX, D>(found, values) or ...);
   }
   
   ///                                                                        
   template<Offset IDX, CT::Data D>
   bool Neat::ExtractTraitInnerInner(const TAny<Any>& found, D& value) const {
      if (IDX >= found.GetCount())
         return false;

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
   
   /// Extract data of an exact type                                          
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param value - [out] where to save the value, if found               
   ///   @return true if value changed                                        
   template<CT::Data D>
   LANGULUS(INLINED)
   bool Neat::ExtractData(D& value) const {
      auto found = GetData<D>();
      if (found) {
         value = found->Last().template Get<D>();
         return true;
      }

      return false;
   }
   
   /// Extract any data, convertible to D                                     
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param value - [out] where to save the value, if found               
   ///   @return true if value changed                                        
   template<CT::Data D>
   LANGULUS(INLINED)
   bool Neat::ExtractDataAs(D& value) const {
      for (auto pair : mAnythingElse) {
         for (auto& group : pair.mValue) {
            try {
               value = group.AsCast<D>();
               return true;
            }
            catch (...) {}
         }
      }

      return false;
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

} // namespace Langulus::Anyness
