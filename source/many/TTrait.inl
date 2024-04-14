///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TTrait.hpp"
#include "Trait.inl"

#define TEMPLATE()   template<class TRAIT>
#define TME()        TTrait<TRAIT>


namespace Langulus::Anyness
{

   /// Refer-constructor                                                      
   ///   @param other - the trait to refer to                                 
   TEMPLATE() LANGULUS(INLINED)
   TME()::TTrait(const TTrait& other)
      : Trait {Refer(static_cast<const Any&>(other))} {}

   /// Move-constructor                                                       
   ///   @param other - the trait to move                                     
   TEMPLATE() LANGULUS(INLINED)
   TME()::TTrait(TTrait&& other)
      : Trait {Move(Forward<Any>(other))} {}

   /// Never absorb different traits, if known at compile-time                
   ///   @param other - the trait to semantically construct with              
   TEMPLATE() template<class T> requires (CT::Trait<Desem<T>>
   and not CT::Same<typename T::TraitType, TRAIT>) LANGULUS(INLINED)
   TME()::TTrait(T&& other) {
      *this << SemanticOf<decltype(other)>::Nest(other);
   }

   /// Refer-assignment                                                       
   ///   @param rhs - the trait to refer-assign                               
   TEMPLATE() LANGULUS(INLINED)
   TRAIT& TME()::operator = (const TTrait& rhs) {
      Trait::operator = (Refer(static_cast<const Any&>(rhs)));
      return static_cast<TRAIT&>(*this);
   }

   /// Move-assignment                                                        
   ///   @param rhs - the trait to move-assign                                
   TEMPLATE() LANGULUS(INLINED)
   TRAIT& TME()::operator = (TTrait&& rhs) {
      Trait::operator = (Move(Forward<Any>(rhs)));
      return static_cast<TRAIT&>(*this);
   }
   
   /// Unfold assignment, semantic or not                                     
   /// If argument is deep or trait, it will be absorbed                      
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this trait                                    
   TEMPLATE() LANGULUS(INLINED)
   TRAIT& TME()::operator = (CT::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::Trait<T>) {
         if constexpr (not CT::Same<typename T::TraitType, TRAIT>) {
            // Never absorb different CT::Trait, insert it instead      
            Block::Reset();
            *this << SemanticOf<decltype(rhs)>::Nest(rhs);
         }
         else Any::operator = (S::Nest(rhs).template Forward<Any>());
      }
      else if constexpr (CT::TraitBased<T> or CT::Deep<T>)
         Any::operator = (S::Nest(rhs).template Forward<Any>());
      else
         Any::operator = (S::Nest(rhs));
      return static_cast<TRAIT&>(*this);
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
      return Trait::GetTrait<TRAIT>();
   }

   /// Check if trait is valid, that is, it's typed and has contents          
   ///   @return true if trait is valid                                       
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::IsTraitValid() const noexcept {
      return Trait::IsTraitValid<TRAIT>();
   }

   /// Check if trait and data types match another trait                      
   ///   @param other - the trait to test against                             
   ///   @return true if traits are similar                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::IsTraitSimilar(const CT::TraitBased auto& other) const noexcept {
      return Trait::IsTraitSimilar<TRAIT>(other);
   }

   /// Check if a trait matches one of a set of trait types                   
   ///   @tparam T... - the trait list                                        
   ///   @return true if this trait is one of the given types                 
   TEMPLATE() template<CT::Trait T1> LANGULUS(INLINED)
   constexpr bool TME()::IsTrait() const {
      return Trait::IsTrait<T1, TRAIT>();
   }

   /// Check if a trait matches one of a set of trait types                   
   ///   @param t1... - the first trait to match                              
   ///   @param tN... - other traits to match                                 
   ///   @return true if this trait is one of the given types                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::IsTrait(TMeta t1, auto...tN) const {
      return Trait::IsTrait<TRAIT>(t1)
         or (Trait::IsTrait<TRAIT>(tN) or ...);
   }

   /// Check if trait has correct data (always true if trait has no filter)   
   ///   @return true if trait definition filter is compatible                
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::HasCorrectData() const {
      return Trait::HasCorrectData<TRAIT>();
   }
   
   /// Compare traits with anything                                           
   ///   @attention function signature must match Block::operator ==          
   ///      otherwise function resolution doesn't work properly on MSVC       
   ///   @param other - the thing to compare with                             
   ///   @return true if things are the same                                  
   TEMPLATE() LANGULUS(INLINED)
   bool TME()::operator == (const CT::NotSemantic auto& rhs) const {
      return Trait::operator == <TRAIT> (rhs);
   }

   /// Concatenate with traits/deep types, semantically or not                
   ///   @param rhs - the right operand                                       
   ///   @return the combined trait                                           
   TEMPLATE() LANGULUS(INLINED)
   TRAIT TME()::operator + (CT::UnfoldInsertable auto&& rhs) const {
      return Trait::operator + <TRAIT> (rhs);
   }

   /// Destructively concatenate with traits/deep types, semantically or not  
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified trait                           
   TEMPLATE() LANGULUS(INLINED)
   TRAIT& TME()::operator += (CT::UnfoldInsertable auto&& rhs) {
      return Trait::operator += <TRAIT> (rhs);
   }

   /// Serialize the trait to anything text-based                             
   TEMPLATE() LANGULUS(INLINED)
   Count TME()::Serialize(CT::Serial auto& to) const {
      return Trait::Serialize<TRAIT>(to);
   }

} // namespace Langulus::Anyness

#undef TME
#undef TEMPLATE
