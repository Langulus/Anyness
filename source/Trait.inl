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
      : Trait {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - the trait to move                                     
   LANGULUS(INLINED)
   Trait::Trait(Trait&& other) noexcept
      : Trait {Move(other)} {}

   /// Unfold constructor, any element can be a semantic                      
   /// If there's only one argument, that is deep or a trait, it will be      
   /// absorbed                                                               
   ///   @param t1 - first argument                                           
   ///   @param tail - the rest of the arguments (optional)                   
   template<class T1, class...TAIL> LANGULUS(INLINED)
   Trait::Trait(T1&& t1, TAIL&&... tail)
   requires CT::Inner::UnfoldInsertable<T1, TAIL...> {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using T = TypeOf<S>;

         if constexpr (CT::TraitBased<T>) {
            BlockTransfer<Any>(S::Nest(t1).template Forward<Any>());
            mTraitType = DesemCast(t1).GetTrait();
         }
         else if constexpr (CT::Deep<T>) {
            BlockTransfer<Any>(S::Nest(t1));
         }
         else {
            Insert<Any>(0, Forward<T1>(t1));
         }
      }
      else Insert<Any>(0, Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Create a trait from a trait and data types                             
   ///   @tparam T - type of trait                                            
   ///   @tparam D - type of data                                             
   ///   @return a trait preconfigured with the provided types                
   template<CT::Trait T, CT::Data D> LANGULUS(INLINED)
   Trait Trait::From() {
      Trait temp {Block::From<D>()};
      temp.SetTrait<T>();
      return Abandon(temp);
   }

   /// Create a trait from a trait definition and data                        
   ///   @tparam T - type of trait                                            
   ///   @param stuff - the data to initialize trait with                     
   ///   @return a trait preconfigured with the provided types and contents   
   template<CT::Trait T> LANGULUS(INLINED)
   Trait Trait::From(auto&& stuff) {
      Trait temp {Forward<Deref<decltype(stuff)>>(stuff)};
      temp.SetTrait<T>();
      return Abandon(temp);
   }

   /// Create a trait from a runtime trait definition and data                
   ///   @param meta - the trait definition                                   
   ///   @param stuff - the data to initialize trait with                     
   ///   @return the trait                                                    
   LANGULUS(INLINED)
   Trait Trait::From(TMeta meta, auto&& stuff) {
      Trait temp {Forward<Deref<decltype(stuff)>>(stuff)};
      temp.SetTrait(meta);
      return Abandon(temp);
   }

   /// Create a trait from a runtime trait definition and data type           
   LANGULUS(INLINED)
   Trait Trait::FromMeta(TMeta tmeta, DMeta dmeta) {
      Trait temp {Block(DataState::Default, dmeta)};
      temp.SetTrait(tmeta);
      return Abandon(temp);
   }

   /// Set the trait type statically                                          
   ///   @tparam T - the trait                                                
   template<CT::Trait T> LANGULUS(INLINED)
   void Trait::SetTrait() noexcept {
      mTraitType = MetaTraitOf<T>();
   }

   /// Set the trait type dynamically                                         
   ///   @param trait - the trait                                             
   LANGULUS(INLINED)
   void Trait::SetTrait(TMeta trait) noexcept {
      mTraitType = trait;
   }

   /// Get the trait type                                                     
   ///   @return the trait type                                               
   LANGULUS(INLINED)
   TMeta Trait::GetTrait() const noexcept {
      return mTraitType;
   }

   /// Check if trait is valid, that is, it's typed and has contents          
   ///   @return true if trait is valid                                       
   LANGULUS(INLINED)
   bool Trait::IsTraitValid() const noexcept {
      return mTraitType and not IsEmpty();
   }

   /// Check if trait and data types match another trait                      
   ///   @param other - the trait to test against                             
   ///   @return true if traits are similar                                   
   LANGULUS(INLINED)
   bool Trait::IsTraitSimilar(const CT::TraitBased auto& other) const noexcept {
      return mTraitType == other.GetTrait() and other.CastsToMeta(GetType());
   }

   /// Check if a trait matches one of a set of trait types                   
   ///   @tparam T... - the trait list                                        
   ///   @return true if this trait is one of the given types                 
   template<CT::Trait T1, CT::Trait...TN> LANGULUS(INLINED)
   bool Trait::TraitIs() const {
      return TraitIs(MetaTraitOf<T1>(), MetaTraitOf<TN>()...);
   }

   /// Check if a trait matches one of a set of trait types                   
   ///   @param traits... - the traits to match                               
   ///   @return true if this trait is one of the given types                 
   template<class T1, class...TN> LANGULUS(INLINED)
   bool Trait::TraitIs(T1 t1, TN...tN) const requires CT::Exact<TMeta, T1, TN...> {
      return mTraitType == t1 or ((mTraitType == tN) or ...);
   }

   /// Check if trait has correct data (always true if trait has no filter)   
   ///   @return true if trait definition filter is compatible                
   LANGULUS(INLINED)
   bool Trait::HasCorrectData() const {
      if (not mTraitType)
         return true;
      return CastsToMeta(mTraitType->mDataType);
   }

   /// Compare traits with other traits, or by contents                       
   ///   @param other - the thing to compare with                             
   ///   @return true if things are the same                                  
   LANGULUS(INLINED)
   bool Trait::operator == (const auto& rhs) const {
      using T = Deref<decltype(rhs)>;

      if constexpr (CT::TraitBased<T>) {
         return TraitIs(rhs.GetTrait())
            and Any::operator == (static_cast<const Any&>(rhs));
      }
      else return Any::operator == (rhs);
   }

   /// Copy-assignment                                                        
   ///   @param other - the trait to copy                                     
   ///   @return a reference to this trait                                    
   LANGULUS(INLINED)
   Trait& Trait::operator = (const Trait& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move-assignment                                                        
   ///   @param other - the trait to move                                     
   ///   @return a reference to this trait                                    
   LANGULUS(INLINED)
   Trait& Trait::operator = (Trait&& rhs) {
      return operator = (Move(rhs));
   }

   /// Unfold assignment, semantic or not                                     
   /// If argument is deep or trait, it will be absorbed                      
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this trait                                    
   LANGULUS(INLINED)
   Trait& Trait::operator = (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         Any::operator = (S::Nest(rhs).template Forward<Any>());
         mTraitType = DesemCast(rhs).GetTrait();
      }
      else if constexpr (CT::Deep<T>)
         Any::operator = (S::Nest(rhs).template Forward<Any>());
      else
         Any::operator = (S::Nest(rhs).Forward());
      return *this;
   }

   /// Concatenate with traits/deep types, semantically or not                
   ///   @attention trait type will be set, if not set yet, and rhs is trait  
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Trait Trait::operator + (CT::Inner::UnfoldInsertable auto&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         auto result = Any::operator + (S::Nest(rhs).template Forward<Any>());
         return Trait::From(
            GetTrait() ? GetTrait() : DesemCast(rhs).GetTrait(),
            Abandon(result)
         );
      }
      else {
         auto result = Any::operator + (S::Nest(rhs).Forward());
         return Trait::From(GetTrait(), Abandon(result));
      }
   }

   /// Destructively concatenate with traits/deep types, semantically or not  
   ///   @attention trait type will be set, if not set yet, and rhs is trait  
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Trait& Trait::operator += (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::TraitBased<T>) {
         Any::operator += (S::Nest(rhs).template Forward<Any>());
         if (not GetTrait())
            SetTrait(DesemCast(rhs).GetTrait());
      }
      else Any::operator += (S::Nest(rhs).Forward());
      return *this;
   }


   ///                                                                        
   ///   Static trait implementation                                          
   ///                                                                        
   #define TEMPLATE() template<class TRAIT>
   #define TME() StaticTrait<TRAIT>

   TEMPLATE() LANGULUS(INLINED)
   TME()::StaticTrait(const StaticTrait& other)
      : Trait {Copy(other)} {}

   TEMPLATE() LANGULUS(INLINED)
   TME()::StaticTrait(StaticTrait&& other)
      : Trait {Move(other)} {}

   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (const StaticTrait& rhs) {
      Trait::operator = (Copy(rhs));
      return *this;
   }

   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (StaticTrait&& rhs) {
      Trait::operator = (Move(rhs));
      return *this;
   }
   
   /// Create a similar trait, that has a specific data type, but no contents 
   ///   @tparam T - the data type to set                                     
   ///   @return the empty trait of the given type                            
   TEMPLATE() template<CT::Data T> LANGULUS(INLINED)
   TRAIT TME()::OfType() {
      TRAIT instance;
      instance.template SetType<T>();
      return instance;
   }
   
   /// Get the trait type                                                     
   ///   @return the trait type                                               
   TEMPLATE() LANGULUS(INLINED)
   TMeta TME()::GetTrait() const noexcept {
      return (mTraitType = MetaTraitOf<TRAIT>());
   }

   /// Check if trait is valid, that is, it's typed and has contents          
   ///   @return true if trait is valid                                       
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::IsTraitValid() const noexcept {
      return not IsEmpty();
   }

   /// Check if trait and data types match another trait                      
   ///   @param other - the trait to test against                             
   ///   @return true if traits are similar                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::IsTraitSimilar(const CT::TraitBased auto& other) const noexcept {
      using T = Deref<decltype(other)>;
      if constexpr (CT::Trait<T>)
         return CT::Similar<TRAIT, T>;
      else
         return Trait::IsTraitSimilar(other);
   }

   /// Check if a trait matches one of a set of trait types                   
   ///   @tparam T... - the trait list                                        
   ///   @return true if this trait is one of the given types                 
   TEMPLATE() template<CT::Trait T1, CT::Trait...TN> LANGULUS(INLINED)
   constexpr bool TME()::TraitIs() const {
      return CT::ExactAsOneOf<TRAIT, T1, TN...>;
   }

   /// Check if a trait matches one of a set of trait types                   
   ///   @param traits... - the traits to match                               
   ///   @return true if this trait is one of the given types                 
   TEMPLATE() template<class T1, class...TN> LANGULUS(INLINED)
   constexpr bool TME()::TraitIs(T1 t1, TN...tN) const requires CT::Exact<TMeta, T1, TN...> {
      (void) GetTrait();
      return mTraitType == t1 or ((mTraitType == tN) or ...);
   }

   /// Check if trait has correct data (always true if trait has no filter)   
   ///   @return true if trait definition filter is compatible                
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::HasCorrectData() const {
      return CastsToMeta(GetTrait()->mDataType);
   }

   #undef TME
   #undef TEMPLATE

} // namespace Langulus::Anyness
