///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Construct.hpp"
#include "Neat.inl"

namespace Langulus::Anyness
{

   /// Default constructor                                                    
   LANGULUS(INLINED)
   constexpr Construct::Construct() noexcept
      : Neat {}
      , Charge {} {}
   
   /// Shallow-copy constructor                                               
   ///   @param other - construct to shallow-copy                             
   LANGULUS(INLINED)
   Construct::Construct(const Construct& other) noexcept
      : Construct {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - construct to move                                     
   LANGULUS(INLINED)
   Construct::Construct(Construct&& other) noexcept
      : Construct {Move(other)} {}

   /// Semantic constructor                                                   
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the construct and semantic                            
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct::Construct(S&& other) requires (CT::Construct<TypeOf<S>>)
      : Neat {other.template Forward<Neat>()}
      , Charge {*other}
      , mType {other->mType} {
      if constexpr (S::Move and S::Keep) {
         other->ResetCharge();
         other->mType = nullptr;
      }
   }

   /// Construct from a type                                                  
   ///   @param type - the type of the content                                
   LANGULUS(INLINED)
   Construct::Construct(DMeta type)
      : mType {type ? type->mOrigin : nullptr} {}

   /// Shallow-copying manual construct constructor                           
   ///   @tparam T - the type of arguments (deducible)                        
   ///   @param type - the type of the construct                              
   ///   @param arguments - the arguments for construction                    
   ///   @param charge - the charge for the construction                      
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, const T& arguments, const Charge& charge)
      : Construct {type, Copy(arguments), charge} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, T& arguments, const Charge& charge)
      : Construct {type, Copy(arguments), charge} {}

   /// Moving manual construct constructor                                    
   ///   @tparam T - the type of arguments (deducible)                        
   ///   @param type - the type of the construct                              
   ///   @param arguments - the arguments for construction                    
   ///   @param charge - the charge for the construction                      
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, T&& arguments, const Charge& charge)
      : Construct {type, Move(arguments), charge} {}
   
   /// Semantic manual construct constructor                                  
   ///   @tparam S - the semantic and type of arguments (deducible)           
   ///   @param type - the type of the construct                              
   ///   @param arguments - the arguments for construction                    
   ///   @param charge - the charge for the construction                      
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, S&& arguments, const Charge& charge)
      : Neat {arguments.Forward()}
      , Charge {charge}
      , mType {type ? type->mOrigin : nullptr} { }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Construct from a type token                                            
   ///   @param type - the type of the content                                
   LANGULUS(INLINED)
   Construct::Construct(const Token& token)
      : mType {RTTI::GetMetaData(token)->mOrigin} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, const T& arguments, const Charge& charge)
      : Construct {token, Copy(arguments), charge} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, T& arguments, const Charge& charge)
      : Construct {token, Copy(arguments), charge} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, T&& arguments, const Charge& charge)
      : Construct {token, Move(arguments), charge} {}

   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, S&& arguments, const Charge& charge)
      : Construct {RTTI::GetMetaData(token), arguments.Forward(), charge} {}
#endif

   /// Copy-assignment                                                        
   ///   @param rhs - the construct to shallow-copy                           
   ///   @return a reference to this construct                                
   LANGULUS(INLINED)
   Construct& Construct::operator = (const Construct& rhs) noexcept {
      return operator = (Copy(rhs));
   }

   /// Move-assignment                                                        
   ///   @param rhs - the construct to move                                   
   ///   @return a reference to this construct                                
   LANGULUS(INLINED)
   Construct& Construct::operator = (Construct&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   /// Semantic-assignment                                                    
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param rhs - the right hand side                                     
   ///   @return a reference to this construct                                
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct& Construct::operator = (S&& rhs) requires (CT::Construct<TypeOf<S>>) {
      Neat::operator = (rhs.template Forward<Neat>());
      Charge::operator = (rhs->GetCharge());
      if constexpr (S::Move and S::Keep) {
         rhs->ResetCharge();
         rhs->mType = nullptr;
      }
      return *this;
   }

   /// Create content descriptor from a static type and arguments by move     
   ///   @tparam T - type of the construct                                    
   ///   @tparam HEAD, TAIL - types of the arguments (deducible)              
   ///   @param head, tail  - the constructor arguments                       
   ///   @return the request                                                  
   template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(INLINED)
   Construct Construct::From(HEAD&& head, TAIL&&... tail) {
      static_assert(CT::Decayed<T>, "T must be fully decayed");
      const auto meta = RTTI::MetaData::Of<T>();
      if constexpr (sizeof...(tail) == 0)
         return Construct {meta, Forward<HEAD>(head)};
      else
         return Construct {meta, Any {Forward<HEAD>(head), Forward<TAIL>(tail)...}};
   }

   /// Create content descriptor from a static type (without arguments)       
   ///   @tparam T - type of the construct                                    
   ///   @return the request                                                  
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct Construct::From() {
      static_assert(CT::Decayed<T>, "T must be fully decayed");
      return Construct {RTTI::MetaData::Of<T>()};
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Create content descriptor from a type token and arguments by copy      
   ///   @tparam HEAD, TAIL - types of the arguments (deducible)              
   ///   @param token - the type name for the construct                       
   ///   @param head, tail  - the constructor arguments                       
   ///   @return the request                                                  
   template<CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(INLINED)
   Construct Construct::FromToken(const Token& token, HEAD&& head, TAIL&&... tail) {
      const auto meta = dynamic_cast<DMeta>(
         RTTI::DisambiguateMeta(token)
      );

      if constexpr (sizeof...(tail) == 0)
         return Construct {meta, Forward<HEAD>(head)};
      else
         return Construct {meta, Any {Forward<HEAD>(head), Forward<TAIL>(tail)...}};
   }

   /// Create content descriptor from a type token (without arguments)        
   ///   @param token - type of the construct                                 
   ///   @return the request                                                  
   LANGULUS(INLINED)
   Construct Construct::FromToken(const Token& token) {
      const auto meta = dynamic_cast<DMeta>(
         RTTI::DisambiguateMeta(token)
      );
      return Construct {meta};
   }
#endif
   
   /// Rehash the construct                                                   
   /// The hash is cached, so this is a cheap function                        
   ///   @return the hash of the content                                      
   LANGULUS(INLINED)
   Hash Construct::GetHash() const {
      if (mHash.mHash)
         return mHash;

      // Hash is the same as the Neat base, with the type on top        
      if (mType)
         mHash = HashOf(mType->mHash, Neat::GetHash());
      else
         mHash = Neat::GetHash();

      return mHash;
   }

   /// Clears arguments and charge, but doesn't deallocate                    
   LANGULUS(INLINED)
   void Construct::Clear() {
      Neat::Clear();
      Charge::Reset();
   }
   
   /// Clears and deallocates arguments and charge                            
   LANGULUS(INLINED)
   void Construct::Reset() {
      Neat::Reset();
      Charge::Reset();
   }

   /// Reset charge                                                           
   LANGULUS(INLINED)
   void Construct::ResetCharge() noexcept {
      Charge::Reset();
   }

   /// Compare constructs                                                     
   ///   @param rhs - descriptor to compare with                              
   ///   @return true if both constructs are the same                         
   LANGULUS(INLINED)
   bool Construct::operator == (const Construct& rhs) const {
      return GetHash() == rhs.GetHash()
         and (mType == rhs.mType or (mType and mType->IsExact(rhs.mType)))
         and Neat::operator == (rhs.GetArgument());
   }

   /// Check if construct type can be interpreted as another type             
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current header to 'type'           
   LANGULUS(INLINED)
   bool Construct::CastsTo(DMeta type) const {
      return not type or (mType == type or mType->CastsTo(type));
   }

   /// Check if constructor header is exactly another type                    
   ///   @param type - the type to check for (must be a dense type)           
   ///   @return true if current header is 'type'                             
   LANGULUS(INLINED)
   bool Construct::Is(DMeta type) const {
      return not type or (mType and mType->Is(type));
   }

   /// Check if construct type can be interpreted as a given static type      
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Construct::CastsTo() const {
      if (not mType)
         return false;
      return CastsTo(RTTI::MetaData::Of<T>());
   }

   /// Check if construct type fully matches a given static type              
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Construct::Is() const {
      if (not mType)
         return false;
      return Is(RTTI::MetaData::Of<T>());
   }

   /// Get the argument for the construct                                     
   ///   @return the constant arguments container                             
   LANGULUS(INLINED)
   const Neat& Construct::GetArgument() const noexcept {
      return static_cast<const Neat&>(*this);
   }

   /// Get the argument for the construct                                     
   ///   @return the arguments container                                      
   LANGULUS(INLINED)
   Neat& Construct::GetArgument() noexcept {
      return static_cast<Neat&>(*this);
   }

   /// Get construct's charge                                                 
   ///   @return the charge                                                   
   LANGULUS(INLINED)
   Charge& Construct::GetCharge() noexcept {
      return static_cast<Charge&>(*this);
   }

   /// Get construct's charge (const)                                         
   ///   @return the charge                                                   
   LANGULUS(INLINED)
   const Charge& Construct::GetCharge() const noexcept {
      return static_cast<const Charge&>(*this);
   }

   /// Get the type of the construct                                          
   ///   @return the type                                                     
   LANGULUS(INLINED)
   DMeta Construct::GetType() const noexcept {
      return mType;
   }
   
   /// Get the token of the construct's type                                  
   ///   @return the token, if type is set, or default token if not           
   LANGULUS(INLINED)
   Token Construct::GetToken() const noexcept {
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         return mType ? mType->GetShortestUnambiguousToken() : MetaData::DefaultToken;
      #else
         return mType ? mType->mToken : MetaData::DefaultToken;
      #endif
   }

   /// Get the producer of the construct                                      
   ///   @return the type of the producer                                     
   LANGULUS(INLINED)
   DMeta Construct::GetProducer() const noexcept {
      return mType ? mType->mProducer : nullptr;
   }

} // namespace Langulus::Anyness
