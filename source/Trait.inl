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
   Trait::Trait(const T& data) requires (CT::Sparse<T> || !CT::DerivedFrom<T, Trait>)
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T& data) requires (CT::Sparse<T> || !CT::DerivedFrom<T, Trait>)
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by movement                                  
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T&& data) requires (CT::Sparse<T> || !CT::DerivedFrom<T, Trait>)
      : Trait {Langulus::Move(data)} {}

   /// Semantic trait construction                                            
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(S&& data) requires (CT::Sparse<TypeOf<S>> || !CT::DerivedFrom<TypeOf<S>, Trait>)
      : Any {data.Forward()} {}
   
   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(const T& data) requires (CT::Dense<T>&& CT::DerivedFrom<T, Trait>)
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T& data) requires (CT::Dense<T>&& CT::DerivedFrom<T, Trait>)
      : Trait {Langulus::Copy(data)} {}

   /// Manual trait construction by movement                                  
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(T&& data) requires (CT::Dense<T>&& CT::DerivedFrom<T, Trait>)
      : Trait {Langulus::Move(data)} {}

   /// Semantic trait construction                                            
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Trait::Trait(S&& data) requires (CT::Dense<TypeOf<S>>&& CT::DerivedFrom<TypeOf<S>, Trait>)
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
   Trait& Trait::operator = (const Trait& other) {
      Any::operator = (static_cast<const Any&>(other));
      mTraitType = other.mTraitType;
      return *this;
   }

   /// Move a trait                                                           
   ///   @param other - the trait to move                                     
   ///   @return a reference to this trait                                    
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (Trait&& other) noexcept {
      Any::operator = (Forward<Any>(other));
      mTraitType = other.mTraitType;
      return *this;
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (const T& rhs) requires (CT::Sparse<T> || !CT::DerivedFrom<T, Trait>) {
      return operator = (Langulus::Copy(rhs));
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (T& rhs) requires (CT::Sparse<T> || !CT::DerivedFrom<T, Trait>) {
      return operator = (Langulus::Copy(rhs));
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (T&& rhs) requires (CT::Sparse<T> || !CT::DerivedFrom<T, Trait>) {
      return operator = (Langulus::Move(rhs));
   }

   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (S&& rhs) requires (CT::Sparse<TypeOf<S>> || !CT::DerivedFrom<TypeOf<S>, Trait>) {
      Any::operator = (rhs.Forward());
      return *this;
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (const T& rhs) requires (CT::Dense<T>&& CT::DerivedFrom<T, Trait>) {
      return operator = (Langulus::Copy(rhs));
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (T& rhs) requires (CT::Dense<T>&& CT::DerivedFrom<T, Trait>) {
      return operator = (Langulus::Copy(rhs));
   }

   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (T&& rhs) requires (CT::Dense<T>&& CT::DerivedFrom<T, Trait>) {
      return operator = (Langulus::Move(rhs));
   }

   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Trait& Trait::operator = (S&& rhs) requires (CT::Dense<TypeOf<S>>&& CT::DerivedFrom<TypeOf<S>, Trait>) {
      Any::operator = (rhs.template Forward<Any>());
      mTraitType = rhs.mValue.GetTrait();
      return *this;
   }


   ///                                                                        
   ///   Static trait implementation                                          
   ///                                                                        
   
   /// Default trait construction                                             
   template<class TRAIT>
   StaticTrait<TRAIT>::StaticTrait()
      : Trait {} {
      SetTrait<TRAIT>();
   }

   /// Trait copy-construction with anything not abandoned or disowned        
   template<class TRAIT>
   template<CT::Data T>
   StaticTrait<TRAIT>::StaticTrait(const T& data)
      : Trait {data} {
      SetTrait<TRAIT>();
   }

   /// Trait copy-construction with anything not abandoned or disowned        
   template<class TRAIT>
   template<CT::Data T>
   StaticTrait<TRAIT>::StaticTrait(T& data)
      : Trait {data} {
      SetTrait<TRAIT>();
   }

   /// Trait move-construction with anything not abandoned or disowned        
   template<class TRAIT>
   template<CT::Data T>
   StaticTrait<TRAIT>::StaticTrait(T&& data)
      : Trait {Forward<T>(data)} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   StaticTrait<TRAIT>::StaticTrait(Disowned<TRAIT>&& other)
      : Trait {other.template Forward<Any>()} {
      SetTrait(other.mValue.GetTrait());
   }

   template<class TRAIT>
   StaticTrait<TRAIT>::StaticTrait(Abandoned<TRAIT>&& other)
      : Trait {other.template Forward<Any>()} {
      SetTrait(other.mValue.GetTrait());
   }

   template<class TRAIT>
   template<CT::Data HEAD, CT::Data... TAIL>
   StaticTrait<TRAIT>::StaticTrait(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1)
      : Trait {Forward<HEAD>(head), Forward<TAIL>(tail)...} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   template<CT::Data T>
   TRAIT& StaticTrait<TRAIT>::operator = (const T& data) {
      if constexpr (CT::Same<T, TRAIT>)
         Any::operator = (static_cast<const Any&>(data));
      else if constexpr (CT::Trait<T>)
         Trait::operator = (static_cast<const Trait&>(data));
      else
         Any::operator = (data);
      return static_cast<TRAIT&>(*this);
   }

   template<class TRAIT>
   template<CT::Data T>
   TRAIT& StaticTrait<TRAIT>::operator = (T& data) {
      return operator = (const_cast<const T&>(data));
   }

   template<class TRAIT>
   template<CT::Data T>
   TRAIT& StaticTrait<TRAIT>::operator = (T&& data) {
      if constexpr (CT::Same<T, TRAIT>)
         Any::operator = (Forward<Any>(data));
      else if constexpr (CT::Trait<T>)
         Trait::operator = (Forward<Trait>(data));
      else
         Any::operator = (Forward<T>(data));
      return static_cast<TRAIT&>(*this);
   }

   template<class TRAIT>
   TRAIT& StaticTrait<TRAIT>::operator = (Disowned<TRAIT>&& other) {
      Any::operator = (other.template Forward<Any>());
      return static_cast<TRAIT&>(*this);
   }

   template<class TRAIT>
   TRAIT& StaticTrait<TRAIT>::operator = (Abandoned<TRAIT>&& other) {
      Any::operator = (other.template Forward<Any>());
      return static_cast<TRAIT&>(*this);
   }

   template<class TRAIT>
   TRAIT StaticTrait<TRAIT>::operator + (const Trait& other) const {
      return TRAIT {
         Any::operator + (static_cast<const Any&>(other))
      };
   }

   template<class TRAIT>
   template<CT::Deep T>
   TRAIT StaticTrait<TRAIT>::operator + (const T& other) const {
      return TRAIT {Any::operator + (other)};
   }

   template<class TRAIT>
   TRAIT& StaticTrait<TRAIT>::operator += (const Trait& other) {
      return static_cast<TRAIT&>(
         Any::operator += (static_cast<const Any&>(other))
      );
   }

   template<class TRAIT>
   template<CT::Deep T>
   TRAIT& StaticTrait<TRAIT>::operator += (const T& other) {
      return static_cast<TRAIT&>(Any::operator += (other));
   }

   template<class TRAIT>
   template<CT::Data T>
   bool StaticTrait<TRAIT>::operator == (const T& other) const {
      if constexpr (CT::Same<T, StaticTrait<TRAIT>>)
         return Any::operator == (static_cast<const Any&>(DenseCast(other)));
      else if constexpr (CT::Trait<T>)
         return Trait::operator == (static_cast<const Trait&>(DenseCast(other)));
      else
         return Any::operator == (other);
   }

   template<class TRAIT>
   TRAIT StaticTrait<TRAIT>::Clone() const {
      return TRAIT {Any::Clone()};
   }

} // namespace Langulus::Anyness
