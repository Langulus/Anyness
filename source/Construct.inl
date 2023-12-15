///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Construct.hpp"
#include "Neat.inl"


namespace Langulus::Anyness
{
   
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
   ///   @tparam S - semantic (deducible)                                     
   ///   @param other - the construct and semantic                            
   template<template<class> class S>
   LANGULUS(INLINED)
   Construct::Construct(S<Construct>&& other)
   requires CT::Semantic<S<Construct>>
      : mType {other->mType}
      , mDescriptor {S<Neat> {other->mDescriptor}}
      , mCharge {other->mCharge} {
      if constexpr (S<Construct>::Move and S<Construct>::Keep) {
         other->ResetCharge();
         other->mType = {};
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
   ///   @param type - the type of the construct                              
   ///   @param arguments - the arguments for construction                    
   ///   @param charge - the charge for the construction                      
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, CT::Semantic auto&& arguments, const Charge& charge)
      : mType {type ? type->mOrigin : nullptr}
      , mDescriptor {arguments.Forward()}
      , mCharge {charge} { }

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

   LANGULUS(INLINED)
   Construct::Construct(const Token& token, CT::Semantic auto&& arguments, const Charge& charge)
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
   template<template<class> class S>
   LANGULUS(INLINED)
   Construct& Construct::operator = (S<Construct>&& rhs)
   requires CT::Semantic<S<Construct>> {
      mType = rhs->mType;
      mDescriptor = S<Neat> {rhs->mDescriptor};
      mCharge = rhs->mCharge;

      if constexpr (S<Construct>::Move and S<Construct>::Keep) {
         rhs->ResetCharge();
         rhs->mType = {};
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
      const auto meta = MetaDataOf<T>();
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
      return Construct {MetaDataOf<T>()};
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
      const auto meta = RTTI::DisambiguateMeta(token);
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
      return Construct {RTTI::DisambiguateMeta(token)};
   }
#endif
   
   /// Rehash the construct                                                   
   /// The hash is cached, so this is a cheap function                        
   ///   @return the hash of the content                                      
   LANGULUS(INLINED)
   Hash Construct::GetHash() const {
      // Hash is the same as the Neat base, but with the type on top    
      return mType ? HashOf(mType, mDescriptor) : mDescriptor.GetHash();
   }

   /// Clears arguments and charge, but doesn't deallocate                    
   LANGULUS(INLINED)
   void Construct::Clear() {
      mDescriptor.Clear();
      mCharge.Reset();
   }
   
   /// Clears and deallocates arguments and charge                            
   LANGULUS(INLINED)
   void Construct::Reset() {
      mDescriptor.Reset();
      mCharge.Reset();
   }

   /// Reset charge                                                           
   LANGULUS(INLINED)
   void Construct::ResetCharge() noexcept {
      mCharge.Reset();
   }

   /// Compare constructs                                                     
   ///   @param rhs - descriptor to compare with                              
   ///   @return true if both constructs are the same                         
   LANGULUS(INLINED)
   bool Construct::operator == (const Construct& rhs) const {
      return GetHash() == rhs.GetHash()
         and (mType == rhs.mType or (mType and mType->IsExact(rhs.mType)))
         and mDescriptor == rhs.mDescriptor;
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
      return CastsTo(MetaDataOf<T>());
   }

   /// Check if construct type fully matches a given static type              
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Construct::Is() const {
      if (not mType)
         return false;
      return Is(MetaDataOf<T>());
   }

   /// Get the argument for the construct                                     
   ///   @return the constant arguments container                             
   LANGULUS(INLINED)
   const Neat& Construct::GetDescriptor() const noexcept {
      return mDescriptor;
   }

   /// Get the argument for the construct                                     
   ///   @return the mutable arguments container                              
   LANGULUS(INLINED)
   Neat& Construct::GetDescriptor() noexcept {
      return mDescriptor;
   }

   /// Get construct's charge                                                 
   ///   @return the charge                                                   
   LANGULUS(INLINED)
   Charge& Construct::GetCharge() noexcept {
      return mCharge;
   }

   /// Get construct's charge (const)                                         
   ///   @return the charge                                                   
   LANGULUS(INLINED)
   const Charge& Construct::GetCharge() const noexcept {
      return mCharge;
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
      return mType.GetToken();
   }

   /// Get the producer of the construct                                      
   ///   @return the type of the producer                                     
   LANGULUS(INLINED)
   DMeta Construct::GetProducer() const noexcept {
      if (mType and mType->mProducerRetriever)
         return mType->mProducerRetriever();
      return nullptr;
   }

   /// Push anything to the descriptor                                        
   ///   @attention resets hash                                               
   ///   @param rhs - stuff to push                                           
   ///   @return a reference to this construct for chaining                   
   Construct& Construct::operator << (auto&& rhs) {
      using T = decltype(rhs);
      if constexpr (requires {mDescriptor << Forward<T>(rhs); }) {
         mDescriptor << Forward<T>(rhs);
         return *this;
      }
      else LANGULUS_ERROR("Can't push that into descriptor");
   }

   /// Merge anything to the descriptor                                       
   ///   @attention resets hash                                               
   ///   @param rhs - stuff to merge                                          
   ///   @return a reference to this construct for chaining                   
   Construct& Construct::operator <<= (auto&& rhs) {
      using T = decltype(rhs);
      if constexpr (requires {mDescriptor <<= Forward<T>(rhs); }) {
         mDescriptor <<= Forward<T>(rhs);
         return *this;
      }
      else LANGULUS_ERROR("Can't merge that into descriptor");
   }

} // namespace Langulus::Anyness
