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
#include "../text/Text.hpp"


namespace Langulus::Anyness
{
   
   /// Refer constructor                                                      
   ///   @param other - construct to refer to                                 
   LANGULUS(INLINED)
   Construct::Construct(const Construct& other) noexcept
      : Construct {Refer(other)} {}

   /// Move constructor                                                       
   ///   @param other - construct to move                                     
   LANGULUS(INLINED)
   Construct::Construct(Construct&& other) noexcept
      : Construct {Move(other)} {}

   /// Intent constructor                                                     
   ///   @param other - the construct and intent                              
   template<template<class> class S> requires CT::Intent<S<Construct>>
   LANGULUS(INLINED) Construct::Construct(S<Construct>&& other)   
      : mType        {other->mType}
      , mDescriptor  {S<Neat> {other->mDescriptor}}
      , mCharge      {other->mCharge} {
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

   /// Manual constructor                                                     
   ///   @param type - the type of the construct                              
   ///   @param args - the arguments for construction, with or without intent 
   ///   @param charge - the charge for the construction                      
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, auto&& args, const Charge& charge)
      : mType        {type ? type->mOrigin : nullptr}
      , mDescriptor  {Forward<Deref<decltype(args)>>(args)}
      , mCharge      {charge} { }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Construct from a type token                                            
   ///   @param type - the type of the content                                
   LANGULUS(INLINED)
   Construct::Construct(const Token& token)
      : mType {RTTI::GetMetaData(token)->mOrigin} {}

   LANGULUS(INLINED)
   Construct::Construct(const Token& token, auto&& args, const Charge& charge)
      : Construct {
         RTTI::GetMetaData(token),
         Forward<Deref<decltype(args)>>(args),
         charge
      } {}
#endif

   /// Refer-assignment                                                       
   ///   @param rhs - the construct to shallow-copy                           
   ///   @return a reference to this construct                                
   LANGULUS(INLINED)
   Construct& Construct::operator = (const Construct& rhs) noexcept {
      return operator = (Refer(rhs));
   }

   /// Move-assignment                                                        
   ///   @param rhs - the construct to move                                   
   ///   @return a reference to this construct                                
   LANGULUS(INLINED)
   Construct& Construct::operator = (Construct&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   /// Intent-assignment                                                      
   ///   @param rhs - the right hand side                                     
   ///   @return a reference to this construct                                
   template<template<class> class S> requires CT::Intent<S<Construct>>
   LANGULUS(INLINED) Construct& Construct::operator = (S<Construct>&& rhs) {
      mType       = rhs->mType;
      mDescriptor = S<Neat> {rhs->mDescriptor};
      mCharge     = rhs->mCharge;

      if constexpr (S<Construct>::Move and S<Construct>::Keep) {
         rhs->ResetCharge();
         rhs->mType = {};
      }
      return *this;
   }

   /// Create content descriptor from a static type and arguments with intent 
   ///   @tparam T - type of the construct                                    
   ///   @param t1, tn  - the constructor arguments                           
   ///   @return the request                                                  
   template<CT::Data T, CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   Construct Construct::From(T1&& t1, TN&&...tn) {
      static_assert(CT::Decayed<T>, "T must be fully decayed");
      Construct result {MetaDataOf<T>()};
      result << Forward<T1>(t1);
      (void) (result << ... << Forward<TN>(tn));
      return result;
   }

   /// Create content descriptor from a static type (without arguments)       
   ///   @tparam T - type of the construct                                    
   ///   @return the request                                                  
   template<CT::Data T> LANGULUS(INLINED)
   Construct Construct::From() {
      static_assert(CT::Decayed<T>, "T must be fully decayed");
      return Construct {MetaDataOf<T>()};
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Create content descriptor from a type token and arguments with intents 
   ///   @param token - the type name for the construct                       
   ///   @param t1, tn  - the constructor arguments                           
   ///   @return the request                                                  
   template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   Construct Construct::FromToken(const Token& token, T1&& t1, TN&&...tn) {
      Construct result {RTTI::DisambiguateMeta(token)};
      result << Forward<T1>(t1);
      (void) (result << ... << Forward<TN>(tn));
      return result;
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
   /// The hash is (mostly) cached, so this is a (relatively) cheap function  
   ///   @return the hash of the content                                      
   LANGULUS(INLINED)
   Hash Construct::GetHash() const {
      // Hash is the same as the Neat base, but with the type on top    
      return mType ? HashOf(mType->mDecvq, mDescriptor) : mDescriptor.GetHash();
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
         and mType & rhs.mType
         and mDescriptor == rhs.mDescriptor;
   }
   
   /// Check if construct type can be interpreted as a given static type      
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T> LANGULUS(INLINED)
   bool Construct::CastsTo() const {
      return mType ? CastsTo(MetaDataOf<T>()) : false;
   }

   /// Check if construct type can be interpreted as another type             
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current header to 'type'           
   LANGULUS(INLINED)
   bool Construct::CastsTo(DMeta type) const {
      return mType & type or mType->CastsTo(type);
   }
   
   /// Check if construct type is similar to another type                     
   ///   @tparam T - the type to check for                                    
   template<CT::Data T> LANGULUS(INLINED)
   bool Construct::Is() const {
      return mType ? Is(MetaDataOf<T>()) : false;
   }

   /// Check if construct type is similar to another type                     
   ///   @param type - the type to check for                                  
   ///   @return true if current header is similar to 'type'                  
   LANGULUS(INLINED)
   bool Construct::Is(DMeta type) const {
      return mType & type;
   }
   
   /// Change the type of the construct                                       
   ///   @attention will cause a rehash if type differs                       
   ///   @tparam T - the new type of the construct                            
   template<CT::Data T> LANGULUS(INLINED)
   void Construct::SetType() {
      SetType(MetaDataOf<T>());
   }

   /// Check if constructor header is exactly another type                    
   ///   @param type - the type to check for (must be a dense type)           
   ///   @return true if current header is 'type'                             
   LANGULUS(INLINED)
   void Construct::SetType(DMeta type) noexcept {
      mType = type;
   }

   /// Get the argument for the construct                                     
   ///   @return the constant arguments container                             
   LANGULUS(INLINED)
   const Neat& Construct::GetDescriptor() const noexcept {
      return mDescriptor;
   }

   /// Get the argument for the construct                                     
   ///   @attention this will force a rehash to guarantee that any changes    
   ///      made through the mutable Neat reference are accounted for on next 
   ///      GetHash() request                                                 
   ///   @return the mutable arguments container                              
   LANGULUS(INLINED)
   Neat& Construct::GetDescriptor() noexcept {
      // Reset hash preventively, because we're exposing a mutable ref, 
      // which is likely to change, and thus a rehash will be needed    
      mDescriptor.mHash = {};
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

   /// Check if construct contains executable elements                        
   LANGULUS(INLINED)
   bool Construct::IsExecutable() const noexcept {
      return mDescriptor.IsExecutable();
   }
   
   /// Check if construct is typed                                            
   LANGULUS(INLINED)
   bool Construct::IsTyped() const noexcept {
      return not not mType;
   }
   
   /// Check if construct is untyped                                          
   LANGULUS(INLINED)
   bool Construct::IsUntyped() const noexcept {
      return not mType;
   }

   /// Push anything to the descriptor                                        
   ///   @attention resets hash                                               
   ///   @param rhs - stuff to push                                           
   ///   @return a reference to this construct for chaining                   
   template<class T> LANGULUS(INLINED)
   Construct& Construct::operator << (T&& rhs) {
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
   template<class T> LANGULUS(INLINED)
   Construct& Construct::operator <<= (T&& rhs) {
      if constexpr (requires {mDescriptor <<= Forward<T>(rhs); }) {
         mDescriptor <<= Forward<T>(rhs);
         return *this;
      }
      else LANGULUS_ERROR("Can't merge that into descriptor");
   }

   /// Serialize the construct to anything text-based                         
   Count Construct::Serialize(CT::Serial auto& to) const {
      const auto initial = to.GetCount();
      using OUT = Deref<decltype(to)>;
      to += GetType();
      to += static_cast<Text>(GetCharge());
      to += OUT::Operator::OpenScope;
      GetDescriptor().Serialize(to);
      to += OUT::Operator::CloseScope;
      return to.GetCount() - initial;
   }

} // namespace Langulus::Anyness
