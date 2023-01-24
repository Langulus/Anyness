///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Trait.hpp"

namespace Langulus::Anyness
{

   /// Shallow copy constructor                                               
   ///   @param other - the trait to copy                                     
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(const Trait& other)
      : Any {static_cast<const Any&>(other)}
      , mTraitType {other.mTraitType} {}

   /// Move constructor                                                       
   ///   @param other - the trait to move                                     
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(Trait&& other) noexcept
      : Any {Forward<Any>(other)}
      , mTraitType {other.mTraitType} {}

   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(const T& data) requires NotRelated<T>
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T& data) requires NotRelated<T>
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by movement                                  
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T&& data) requires NotRelated<T>
      : Trait {Langulus::Move(data)} {}

   /// Semantic trait construction                                            
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(S&& data) requires NotRelated<TypeOf<S>>
      : Any {data.Forward()} {}
   
   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(const T& data) requires Related<T>
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T& data) requires Related<T>
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by movement                                  
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T&& data) requires Related<T>
      : Trait {Langulus::Move(data)} {}

   /// Semantic trait construction                                            
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(S&& data) requires Related<TypeOf<S>>
      : Any {data.template Forward<Any>()}
      , mTraitType {data.mValue.GetTrait()} {}

   /// Pack any number of elements sequentially                               
   /// If any of the types doesn't match exactly, the container becomes deep  
   /// to incorporate all elements                                            
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   template<CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1)
      : Any {Forward<HEAD>(head), Forward<TAIL>(tail)...} { }

   /// Create a trait from a trait and data types                             
   ///   @tparam T - type of trait                                            
   ///   @tparam D - type of data                                             
   ///   @return a trait preconfigured with the provided types                
   template<CT::Data T, CT::Data D>
   LANGULUS(ALWAYSINLINE)
   Trait Trait::From() {
      static_assert(CT::Trait<T>, "T must be a trait definition");
      Trait temp {Block::From<D>()};
      temp.SetTrait<T>();
      return Abandon(temp);
   }

   /// Create a trait from a trait definition and data                        
   ///   @tparam T - type of trait                                            
   ///   @tparam D - type of data (deducible)                                 
   ///   @param stuff - the data to set inside trait                          
   ///   @return a trait preconfigured with the provided types                
   template<CT::Data T, CT::Data D>
   LANGULUS(ALWAYSINLINE)
   Trait Trait::From(const D& stuff) {
      static_assert(CT::Trait<T>, "T must be a trait definition");
      Trait temp {stuff};
      temp.SetTrait<T>();
      return Abandon(temp);
   }

   /// Create a trait from a trait definition by moving data                  
   ///   @tparam T - type of trait                                            
   ///   @tparam D - type of data (deducible)                                 
   ///   @param stuff - the data to set inside trait                          
   ///   @return a trait preconfigured with the provided types                
   template<CT::Data T, CT::Data D>
   LANGULUS(ALWAYSINLINE)
   Trait Trait::From(D&& stuff) {
      static_assert(CT::Trait<T>, "T must be a trait definition");
      Trait temp {Forward<D>(stuff)};
      temp.SetTrait<T>();
      return Abandon(temp);
   }

   /// Create a trait from a trait definition and copy of data                
   ///   @tparam DATA - type of data to shallow-copy                          
   ///   @param meta - the trait definition                                   
   ///   @param stuff - the data to copy                                      
   ///   @return the trait                                                    
   template<CT::Data DATA>
   LANGULUS(ALWAYSINLINE)
   Trait Trait::From(TMeta meta, const DATA& stuff) {
      Trait result {stuff};
      result.SetTrait(meta);
      return Abandon(result);
   }

   /// Create a trait from a trait definition and moved data                  
   ///   @tparam DATA - type of data to move in                               
   ///   @param meta - the trait definition                                   
   ///   @param stuff - the data to copy                                      
   ///   @return the trait                                                    
   template<CT::Data DATA>
   LANGULUS(ALWAYSINLINE)
   Trait Trait::From(TMeta meta, DATA&& stuff) {
      Trait result {Forward<DATA>(stuff)};
      result.SetTrait(meta);
      return Abandon(result);
   }

   /// Create a trait from a trait definition and data                        
   LANGULUS(ALWAYSINLINE)
   Trait Trait::FromMeta(TMeta tmeta, DMeta dmeta) {
      Trait result {Block(DataState::Default, dmeta)};
      result.SetTrait(tmeta);
      return Abandon(result);
   }

   /// Set the trait type via a static type                                   
   ///   @tparam T - the trait                                                
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   void Trait::SetTrait() noexcept {
      static_assert(CT::Trait<T>, "TRAIT must be a trait definition");
      mTraitType = MetaTrait::Of<T>();
   }

   /// Set the trait type via a dynamic type                                  
   ///   @tparam trait - the trait                                            
   LANGULUS(ALWAYSINLINE)
   constexpr void Trait::SetTrait(TMeta trait) noexcept {
      mTraitType = trait;
   }
   
   /// Clone the trait                                                        
   ///   @return the cloned trait                                             
   LANGULUS(ALWAYSINLINE)
   Trait Trait::Clone() const {
      return Trait::From(mTraitType, Any::Clone());
   }

   /// Get the trait type                                                     
   ///   @return the trait type                                               
   LANGULUS(ALWAYSINLINE)
   TMeta Trait::GetTrait() const noexcept {
      return mTraitType;
   }

   /// Check if trait is valid                                                
   ///   @return true if trait is valid                                       
   LANGULUS(ALWAYSINLINE)
   bool Trait::IsTraitValid() const noexcept {
      return mTraitType && !Any::IsEmpty();
   }

   /// Check if trait is similar to another                                   
   ///   @param other - the trait to test against                             
   ///   @return true if trait is valid                                       
   LANGULUS(ALWAYSINLINE)
   bool Trait::IsSimilar(const Trait& other) const noexcept {
      return mTraitType->Is(other.mTraitType) && other.CastsToMeta(GetType());
   }

   /// Check if trait is a specific type                                      
   ///   @param traitId - the id to match                                     
   ///   @return true if ID matches                                           
   LANGULUS(ALWAYSINLINE)
   bool Trait::TraitIs(TMeta trait) const {
      return mTraitType == trait || (mTraitType && mTraitType->Is(trait));
   }

   /// Check if trait has correct data (always true if trait has no filter)   
   ///   @return true if trait definition filter is compatible                
   LANGULUS(ALWAYSINLINE)
   bool Trait::HasCorrectData() const {
      if (!mTraitType)
         return true;
      return CastsToMeta(mTraitType->mDataType);
   }

   /// Check if a trait matches a static definition                           
   ///   @tparam T - the trait                                                
   ///   @return true if this trait is of the given type                      
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   bool Trait::TraitIs() const {
      static_assert(CT::Trait<T>, "TRAIT must be a trait definition");
      return TraitIs(MetaTrait::Of<T>());
   }

   /// Compare traits                                                         
   ///   @param other - the thing to compare with                             
   ///   @return true if things are the same                                  
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   bool Trait::operator == (const T& other) const {
      if constexpr (CT::Trait<T>)
         return TraitIs(DenseCast(other).mTraitType)
            && Any::operator == (static_cast<const Any&>(DenseCast(other)));
      else
         return Any::operator == (other);
   }

   /// Copy-assign trait                                                      
   ///   @param other - the trait to copy                                     
   ///   @return a reference to this trait                                    
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (const Trait& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move a trait                                                           
   ///   @param other - the trait to move                                     
   ///   @return a reference to this trait                                    
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (Trait&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (const T& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (T& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (T&& rhs) {
      return operator = (Langulus::Move(rhs));
   }

   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (S&& rhs) {
      if constexpr (CT::Deep<TypeOf<S>>) {
         Any::operator = (rhs.template Forward<Any>());
      }
      else if constexpr (CT::Trait<TypeOf<S>>) {
         Any::operator = (rhs.template Forward<Any>());
         mTraitType = rhs.mValue.GetTrait();
      }
      else Any::operator = (rhs.Forward());

      return *this;
   }


   ///                                                                        
   ///   Static trait implementation                                          
   ///                                                                        
   
   /// Default trait construction                                             
   template<class TRAIT>
   StaticTrait<TRAIT>::StaticTrait() {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   StaticTrait<TRAIT>::StaticTrait(const StaticTrait& other)
      : Trait {Langulus::Copy(other)} {}

   template<class TRAIT>
   StaticTrait<TRAIT>::StaticTrait(StaticTrait&& other)
      : Trait {Langulus::Move(other)} {}

   template<class TRAIT>
   template<CT::NotSemantic T>
   StaticTrait<TRAIT>::StaticTrait(const T& other)
      : Trait {Langulus::Copy(other)} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   StaticTrait<TRAIT>::StaticTrait(T& other)
      : Trait {Langulus::Copy(other)} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   StaticTrait<TRAIT>::StaticTrait(T&& other)
      : Trait {Langulus::Move(other)} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   template<CT::Semantic S>
   StaticTrait<TRAIT>::StaticTrait(S&& other)
      : Trait {other.Forward()} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   template<CT::Data HEAD, CT::Data... TAIL>
   StaticTrait<TRAIT>::StaticTrait(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1)
      : Trait {Forward<HEAD>(head), Forward<TAIL>(tail)...} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (const StaticTrait& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   template<class TRAIT>
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (StaticTrait&& rhs) {
      return operator = (Langulus::Move(rhs));
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (const T& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (T& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (T&& rhs) {
      return operator = (Langulus::Move(rhs));
   }

   template<class TRAIT>
   template<CT::Semantic S>
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (S&& rhs) {
      if constexpr (CT::Deep<TypeOf<S>> || CT::Trait<TypeOf<S>>)
         Any::operator = (rhs.template Forward<Any>());
      else
         Any::operator = (rhs.Forward());
      return *this;
   }

   /// Concatenate the contents of a trait to this trait                      
   ///   @param rhs - the deep container to concatenate                       
   ///   @return the concatenated trait                                       
   template<class TRAIT>
   TRAIT StaticTrait<TRAIT>::operator + (const Trait& other) const {
      return TRAIT {Any::operator + (static_cast<const Any&>(other))};
   }

   /// Concatenate the contents of a deep container to this trait             
   ///   @param rhs - the deep container to concatenate                       
   ///   @return the concatenated trait                                       
   template<class TRAIT>
   template<CT::Deep T>
   TRAIT StaticTrait<TRAIT>::operator + (const T& rhs) const {
      return TRAIT {Any::operator + (rhs)};
   }

   /// Concatenate the contents of a trait to this trait                      
   ///   @param rhs - the deep container to concatenate                       
   ///   @return a reference to this trait                                    
   template<class TRAIT>
   TRAIT& StaticTrait<TRAIT>::operator += (const Trait& rhs) {
      Any::operator += (static_cast<const Any&>(rhs));
      return *this;
   }

   /// Concatenate the contents of a deep container to this trait             
   ///   @param rhs - the deep container to concatenate                       
   ///   @return a reference to this trait                                    
   template<class TRAIT>
   template<CT::Deep T>
   TRAIT& StaticTrait<TRAIT>::operator += (const T& rhs) {
      Any::operator += (rhs);
      return *this;
   }

   /// Compare two traits                                                     
   ///   @param rhs - the trait to compare against                            
   ///   @return true if traits match by contents and type                    
   template<class TRAIT>
   template<CT::Data T>
   bool StaticTrait<TRAIT>::operator == (const T& rhs) const {
      if constexpr (CT::Same<T, StaticTrait<TRAIT>>)
         return Any::operator == (static_cast<const Any&>(DenseCast(rhs)));
      else if constexpr (CT::Trait<T>)
         return Trait::operator == (static_cast<const Trait&>(DenseCast(rhs)));
      else
         return Any::operator == (rhs);
   }

   /// Clone the static trait                                                 
   ///   @return the cloned static trait                                      
   template<class TRAIT>
   TRAIT StaticTrait<TRAIT>::Clone() const {
      return TRAIT {Any::Clone()};
   }



   /// Get the memory block corresponding to a local member variable          
   /// Never references data                                                  
   ///   @tparam T - a trait or data type to search for in the reflection     
   ///   @return a static memory block                                        
   template<class T>
   LANGULUS(ALWAYSINLINE)
   Block Block::GetMember() const {
      if constexpr (CT::Trait<T>)
         return GetMember(MetaTrait::Of<Decay<T>>());
      else
         return GetMember(MetaData::Of<Decay<T>>());
   }

   /// Get the memory block corresponding to a local member variable          
   /// Never references data                                                  
   ///   @tparam T - a trait or data type to search for in the reflection     
   ///   @return a static memory block                                        
   template<class T>
   LANGULUS(ALWAYSINLINE)
   Block Block::GetMember() {
      if constexpr (CT::Trait<T>)
         return GetMember(MetaTrait::Of<Decay<T>>());
      else
         return GetMember(MetaData::Of<Decay<T>>());
   }

   
   /// Select a member Block via trait/data or index (or both)                
   ///   @param index - the trait index to get                                
   ///   @return a static memory block (constant if block is constant)        
   template<class T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Block Block::GetMember(const INDEX& index) const {
      if constexpr (CT::Trait<T>)
         return GetMember(MetaTrait::Of<Decay<T>>(), index);
      else
         return GetMember(MetaData::Of<Decay<T>>(), index);
   }

   /// Select a member Block via trait/data or index (or both)                
   ///   @param index - the trait index to get                                
   ///   @return a static memory block (constant if block is constant)        
   template<class T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Block Block::GetMember(const INDEX& index) {
      if constexpr (CT::Trait<T>)
         return GetMember(MetaTrait::Of<Decay<T>>(), index);
      else
         return GetMember(MetaData::Of<Decay<T>>(), index);
   }

} // namespace Langulus::Anyness
