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
      if constexpr (S::Move)
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
               mConstructs[type] << Construct {type};
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
               // Normalize contents and push sort it by type           
               if (construct.IsDeep()) {
                  mConstructs[construct.GetType()] << Construct {
                     construct.GetType(), Neat {construct}
                  };
               }
               else mConstructs[construct.GetType()] << construct;
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
      if constexpr (S::Move)
         other->mHash = {};
      return *this;
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
            if (construct.GetArgument().Is<Neat>()) {
               constructs << Construct(
                  construct.GetType(),
                  construct.GetArgument().Get<Neat>().MakeMessy(),
                  construct.GetCharge()
               );
            }
            else constructs << construct;
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

   /// Compare neat container                                                 
   ///   @attention order doesn't matter                                      
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

} // namespace Langulus::Anyness
