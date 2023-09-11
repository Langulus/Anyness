///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Trait.hpp"
#include "Any.inl"


namespace Langulus::Anyness
{

   /// Shallow copy constructor                                               
   ///   @param other - the trait to copy                                     
   LANGULUS(INLINED)
   Trait::Trait(const Trait& other)
      : Any {static_cast<const Any&>(other)}
      , mTraitType {other.mTraitType} {}

   /// Move constructor                                                       
   ///   @param other - the trait to move                                     
   LANGULUS(INLINED)
   Trait::Trait(Trait&& other) noexcept
      : Any {Forward<Any>(other)}
      , mTraitType {other.mTraitType} {}

   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   LANGULUS(INLINED)
   Trait::Trait(const CT::NotSemantic auto& data)
      : Trait {Copy(data)} {}

   /// Manual trait construction by copy                                      
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to copy inside trait                              
   LANGULUS(INLINED)
   Trait::Trait(CT::NotSemantic auto& data)
      : Trait {Copy(data)} {}

   /// Manual trait construction by movement                                  
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   LANGULUS(INLINED)
   Trait::Trait(CT::NotSemantic auto&& data)
      : Trait {Move(data)} {}

   /// Semantic trait construction                                            
   ///   @tparam T - type of the contained data                               
   ///   @param data - data to move inside trait                              
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Trait::Trait(S&& data) {
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         BlockTransfer<Any>(data.template Forward<Any>());
         mTraitType = data->GetTrait();
      }
      else Any::CreateFrom(data.Forward());
   }

   /// Pack any number of elements sequentially                               
   /// If any of the types doesn't match exactly, the container becomes deep  
   /// to incorporate all elements                                            
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   template<CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(INLINED)
   Trait::Trait(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1)
      : Any {Forward<HEAD>(head), Forward<TAIL>(tail)...} { }

   /// Create a trait from a trait and data types                             
   ///   @tparam T - type of trait                                            
   ///   @tparam D - type of data                                             
   ///   @return a trait preconfigured with the provided types                
   template<CT::Data T, CT::Data D>
   LANGULUS(INLINED)
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
   LANGULUS(INLINED)
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
   LANGULUS(INLINED)
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
   LANGULUS(INLINED)
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
   LANGULUS(INLINED)
   Trait Trait::From(TMeta meta, DATA&& stuff) {
      Trait result {Forward<DATA>(stuff)};
      result.SetTrait(meta);
      return Abandon(result);
   }

   /// Create a trait from a trait definition and data                        
   LANGULUS(INLINED)
   Trait Trait::FromMeta(TMeta tmeta, DMeta dmeta) {
      Trait result {Block(DataState::Default, dmeta)};
      result.SetTrait(tmeta);
      return Abandon(result);
   }

   /// Set the trait type via a static type                                   
   ///   @tparam T - the trait                                                
   template<CT::Data T>
   LANGULUS(INLINED)
   void Trait::SetTrait() noexcept {
      static_assert(CT::Trait<T>, "TRAIT must be a trait definition");
      mTraitType = T::GetTrait();
   }

   /// Set the trait type via a dynamic type                                  
   ///   @tparam trait - the trait                                            
   LANGULUS(INLINED)
   constexpr void Trait::SetTrait(TMeta trait) noexcept {
      mTraitType = trait;
   }

   /// Get the trait type                                                     
   ///   @return the trait type                                               
   LANGULUS(INLINED)
   TMeta Trait::GetTrait() const noexcept {
      return mTraitType;
   }

   /// Check if trait is valid                                                
   ///   @return true if trait is valid                                       
   LANGULUS(INLINED)
   bool Trait::IsTraitValid() const noexcept {
      return mTraitType and not Any::IsEmpty();
   }

   /// Check if trait is similar to another                                   
   ///   @param other - the trait to test against                             
   ///   @return true if trait is valid                                       
   LANGULUS(INLINED)
   bool Trait::IsSimilar(const Trait& other) const noexcept {
      return mTraitType->Is(other.mTraitType) and other.CastsToMeta(GetType());
   }

   /// Check if trait is a specific type                                      
   ///   @param traitId - the id to match                                     
   ///   @return true if ID matches                                           
   LANGULUS(INLINED)
   bool Trait::TraitIs(TMeta trait) const {
      return mTraitType == trait or (mTraitType and mTraitType->Is(trait));
   }

   /// Check if trait has correct data (always true if trait has no filter)   
   ///   @return true if trait definition filter is compatible                
   LANGULUS(INLINED)
   bool Trait::HasCorrectData() const {
      if (not mTraitType)
         return true;
      return CastsToMeta(mTraitType->mDataType);
   }

   /// Check if a trait matches one of a set of trait types                   
   ///   @tparam T... - the trait list                                        
   ///   @return true if this trait is one of the given types                 
   template<CT::Data... T>
   LANGULUS(INLINED)
   bool Trait::TraitIs() const {
      static_assert(CT::Trait<T...>, "Ts must be trait definitions");
      return (TraitIs(T::GetTrait()) or ...);
   }

   /// Compare traits                                                         
   ///   @param other - the thing to compare with                             
   ///   @return true if things are the same                                  
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Trait::operator == (const T& other) const {
      if constexpr (CT::TraitBased<T>)
         return TraitIs(DenseCast(other).mTraitType)
            and Any::operator == (static_cast<const Any&>(DenseCast(other)));
      else
         return Any::operator == (other);
   }

   /// Copy-assign trait                                                      
   ///   @param other - the trait to copy                                     
   ///   @return a reference to this trait                                    
   LANGULUS(INLINED)
   Trait& Trait::operator = (const Trait& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move a trait                                                           
   ///   @param other - the trait to move                                     
   ///   @return a reference to this trait                                    
   LANGULUS(INLINED)
   Trait& Trait::operator = (Trait&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   LANGULUS(INLINED)
   Trait& Trait::operator = (const CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }

   LANGULUS(INLINED)
   Trait& Trait::operator = (CT::NotSemantic auto& rhs) {
      return operator = (Copy(rhs));
   }

   LANGULUS(INLINED)
   Trait& Trait::operator = (CT::NotSemantic auto&& rhs) {
      return operator = (Move(rhs));
   }

   LANGULUS(INLINED)
   Trait& Trait::operator = (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;

      if constexpr (CT::Deep<TypeOf<S>>)
         Any::operator = (rhs.template Forward<Any>());
      else if constexpr (CT::TraitBased<TypeOf<S>>) {
         Any::operator = (rhs.template Forward<Any>());
         mTraitType = rhs->GetTrait();
      }
      else Any::operator = (rhs.Forward());
      return *this;
   }
   
   /// Copy-concatenate with any other trait                                  
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Trait Trait::operator + (const Trait& rhs) const {
      auto result = Any::operator + (static_cast<const Block&>(rhs));
      return Trait::From(
         GetTrait() ? GetTrait() : rhs.GetTrait(),
         Abandon(result)
      );
   }
   
   /// Move-concatenate with any other trait                                  
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Trait Trait::operator + (Trait&& rhs) const {
      auto result = Any::operator + (Forward<Block>(rhs));
      return Trait::From(
         GetTrait() ? GetTrait() : rhs.GetTrait(),
         Abandon(result)
      );
   }
   
   /// Copy-concatenate with any deep type                                    
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Trait Trait::operator + (const CT::Deep auto& rhs) const {
      return operator + (Copy(rhs));
   }

   LANGULUS(INLINED)
   Trait Trait::operator + (CT::Deep auto& rhs) const {
      return operator + (Copy(rhs));
   }

   /// Move-concatenate with any deep type                                    
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Trait Trait::operator + (CT::Deep auto&& rhs) const {
      return operator + (Move(rhs));
   }

   /// Semantically concatenate with any deep type                            
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Trait Trait::operator + (CT::Semantic auto&& rhs) const {
      auto result = Any::operator + (rhs.template Forward<Block>());
      return Trait::From(GetTrait(), Abandon(result));
   }

   /// Destructive copy-concatenate with any trait                            
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Trait& Trait::operator += (const Trait& rhs) {
      return operator += (Copy(static_cast<const Block&>(rhs)));
   }
   
   /// Destructive copy-concatenate with any trait                            
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Trait& Trait::operator += (Trait&& rhs) {
      return operator += (Forward<Block>(rhs));
   }
   
   /// Destructive copy-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Trait& Trait::operator += (const CT::Deep auto& rhs) {
      return operator += (Copy(rhs));
   }

   LANGULUS(INLINED)
   Trait& Trait::operator += (CT::Deep auto& rhs) {
      return operator += (Copy(rhs));
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Trait& Trait::operator += (CT::Deep auto&& rhs) {
      return operator += (Move(rhs));
   }

   /// Destructive semantically concatenate with any deep type                
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Trait& Trait::operator += (CT::Semantic auto&& rhs) {
      Any::operator += (rhs.template Forward<Block>());
      return *this;
   }




   ///                                                                        
   ///   Static trait implementation                                          
   ///                                                                        
   
   /// Default trait construction                                             
   template<class TRAIT>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>::StaticTrait() {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>::StaticTrait(const StaticTrait& other)
      : Trait {Copy(other)} {}

   template<class TRAIT>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>::StaticTrait(StaticTrait&& other)
      : Trait {Move(other)} {}

   template<class TRAIT>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>::StaticTrait(const T& other)
      : Trait {Copy(other)} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>::StaticTrait(T& other)
      : Trait {Copy(other)} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>::StaticTrait(T&& other)
      : Trait {Move(other)} {
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
   LANGULUS(INLINED)
   StaticTrait<TRAIT>::StaticTrait(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1)
      : Trait {Forward<HEAD>(head), Forward<TAIL>(tail)...} {
      SetTrait<TRAIT>();
   }

   template<class TRAIT>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (const StaticTrait& rhs) {
      return operator = (Copy(rhs));
   }

   template<class TRAIT>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (StaticTrait&& rhs) {
      return operator = (Move(rhs));
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (const T& rhs) {
      return operator = (Copy(rhs));
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (T& rhs) {
      return operator = (Copy(rhs));
   }

   template<class TRAIT>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (T&& rhs) {
      return operator = (Move(rhs));
   }

   template<class TRAIT>
   template<CT::Semantic S>
   LANGULUS(INLINED)
   StaticTrait<TRAIT>& StaticTrait<TRAIT>::operator = (S&& rhs) {
      if constexpr (CT::Deep<TypeOf<S>> or CT::TraitBased<TypeOf<S>>)
         Any::operator = (rhs.template Forward<Any>());
      else
         Any::operator = (rhs.Forward());
      return *this;
   }

   /// Concatenate the contents of a trait to this trait                      
   ///   @param rhs - the deep container to concatenate                       
   ///   @return the concatenated trait                                       
   template<class TRAIT>
   LANGULUS(INLINED)
   TRAIT StaticTrait<TRAIT>::operator + (const Trait& other) const {
      return TRAIT {Any::operator + (static_cast<const Any&>(other))};
   }

   /// Concatenate the contents of a deep container to this trait             
   ///   @param rhs - the deep container to concatenate                       
   ///   @return the concatenated trait                                       
   template<class TRAIT>
   template<CT::Deep T>
   LANGULUS(INLINED)
   TRAIT StaticTrait<TRAIT>::operator + (const T& rhs) const {
      return TRAIT {Any::operator + (rhs)};
   }

   /// Concatenate the contents of a trait to this trait                      
   ///   @param rhs - the deep container to concatenate                       
   ///   @return a reference to this trait                                    
   template<class TRAIT>
   LANGULUS(INLINED)
   TRAIT& StaticTrait<TRAIT>::operator += (const Trait& rhs) {
      Any::operator += (static_cast<const Any&>(rhs));
      return *this;
   }

   /// Concatenate the contents of a deep container to this trait             
   ///   @param rhs - the deep container to concatenate                       
   ///   @return a reference to this trait                                    
   template<class TRAIT>
   template<CT::Deep T>
   LANGULUS(INLINED)
   TRAIT& StaticTrait<TRAIT>::operator += (const T& rhs) {
      Any::operator += (rhs);
      return *this;
   }

   /// Compare two traits                                                     
   ///   @param rhs - the trait to compare against                            
   ///   @return true if traits match by contents and type                    
   template<class TRAIT>
   template<CT::Data T>
   LANGULUS(INLINED)
   bool StaticTrait<TRAIT>::operator == (const T& rhs) const {
      if constexpr (CT::Same<T, StaticTrait<TRAIT>>)
         return Any::operator == (static_cast<const Any&>(DenseCast(rhs)));
      else if constexpr (CT::TraitBased<T>)
         return Trait::operator == (static_cast<const Trait&>(DenseCast(rhs)));
      else
         return Any::operator == (rhs);
   }

   /// Set the data type of the trait                                         
   ///   @tparam T - the data type to set                                     
   ///   @return the empty trait of the given type                            
   template<class TRAIT>
   template<CT::Data T>
   LANGULUS(INLINED)
   TRAIT StaticTrait<TRAIT>::OfType() {
      TRAIT instance;
      instance.template SetType<T>();
      return instance;
   }
   
   /// Get the trait meta definition                                          
   ///   @return the meta trait                                               
   template<class TRAIT>
   LANGULUS(INLINED)
   TMeta StaticTrait<TRAIT>::GetTrait() {
      return RTTI::MetaTrait::Of<TRAIT>();
   }

} // namespace Langulus::Anyness
