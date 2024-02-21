///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Verb.hpp"
#include "VerbState.inl"


namespace Langulus::A
{
   
   /// Shallow-copy constructor                                               
   ///   @param other - the verb to shallow-copy                              
   LANGULUS(INLINED)
   Verb::Verb(const Verb& other)
      : Verb {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - the verb to move                                      
   LANGULUS(INLINED)
   Verb::Verb(Verb&& other)
      : Verb {Move(other)} {}
   
   /// Move constructor                                                       
   ///   @param other - the verb to move                                      
   template<template<class> class S>
   requires CT::Semantic<S<Verb>> LANGULUS(INLINED)
   Verb::Verb(S<Verb>&& other) {
      Any::operator = (other.template Forward<Any>());
      Charge::operator = (*other);
      mVerb = other->mVerb;
      mState = other->mState;
      mSource = S<Any> {other->mSource};
      mOutput = S<Any> {other->mOutput};
   }

   /// Verb shallow-copy assignment                                           
   ///   @param rhs - the verb to shallow-copy assign                         
   ///   @return a reference to this verb                                     
   LANGULUS(INLINED)
   Verb& Verb::operator = (const Verb& rhs) {
      return operator = (Copy(rhs));
   }

   /// Verb move-assignment                                                   
   ///   @param rhs - the verb to move-assign                                 
   ///   @return a reference to this verb                                     
   LANGULUS(INLINED)
   Verb& Verb::operator = (Verb&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantic assignment                                                    
   ///   @param rhs - the verb/argument and semantic to assign by             
   ///   @return a reference to this verb                                     
   template<template<class> class S>
   requires CT::Semantic<S<Verb>> LANGULUS(INLINED)
   Verb& Verb::operator = (S<Verb>&& rhs) {
      Any::operator = (rhs.template Forward<Any>());
      Charge::operator = (*rhs);
      mVerb = rhs->mVerb;
      mState = rhs->mState;
      mSource = S<Any> {rhs->mSource};
      mOutput = S<Any> {rhs->mOutput};
      return *this;
   }
   
   /// Reset all verb members and energy                                      
   LANGULUS(INLINED)
   void Verb::Reset() {
      mVerb = {};
      Any::Reset();
      Charge::Reset();
      mSource.Reset();
      mOutput.Reset();
      mSuccesses = {};
   };

   /// Check if a verb is valid for the given priority                        
   ///   @param priority - the priority to check                              
   ///   @return true if this verb's priority matches the provided one        
   LANGULUS(INLINED)
   bool Verb::Validate(Anyness::Index priority) const noexcept {
      return int(mPriority) == priority.mIndex
          or priority == Anyness::IndexAll;
   }
   
   /// Check if verb has been satisfied at least once                         
   ///   @return true if verb has been satisfied at least once                
   LANGULUS(INLINED)
   bool Verb::IsDone() const noexcept {
      return mSuccesses > 0;
   }

   /// Check if verb is multicast                                             
   ///   @return true if verb is multicast                                    
   LANGULUS(INLINED)
   constexpr bool Verb::IsMulticast() const noexcept {
      return mState.IsMulticast();
   }

   /// Check if verb is monocast                                              
   ///   @return true if verb is monocast                                     
   LANGULUS(INLINED)
   constexpr bool Verb::IsMonocast() const noexcept {
      return mState.IsMonocast();
   }

   /// Check if verb is short-circuited                                       
   ///   @return true if verb is short-circuited                              
   LANGULUS(INLINED)
   constexpr bool Verb::IsShortCircuited() const noexcept {
      return mState.IsShortCircuited();
   }

   /// Check if verb is long-circuited                                        
   ///   @return true if verb is long-circuited                               
   LANGULUS(INLINED)
   constexpr bool Verb::IsLongCircuited() const noexcept {
      return mState.IsLongCircuited();
   }

   /// Get the verb state                                                     
   ///   @return the verb state                                               
   LANGULUS(INLINED)
   Anyness::VerbState Verb::GetVerbState() const noexcept {
      return mState;
   }
   
   /// Get the number of successful execution of the verb                     
   ///   @return the number of successful executions                          
   LANGULUS(INLINED)
   Count Verb::GetSuccesses() const noexcept {
      return mSuccesses;
   }

   /// Check if anything inside the verb is missing on the surface level      
   ///   @return true if anything is missing                                  
   LANGULUS(INLINED)
   bool Verb::IsMissing() const noexcept {
      return mSource.IsMissing()
          or Any::IsMissing()
          or mOutput.IsMissing();
   }

   /// Check if anything inside the verb is missing deeply                    
   ///   @return true if anything is missing                                  
   LANGULUS(INLINED)
   bool Verb::IsMissingDeep() const noexcept {
      return mSource.IsMissingDeep()
          or Any::IsMissingDeep()
          or mOutput.IsMissingDeep();
   }

   /// Satisfy verb a number of times                                         
   LANGULUS(INLINED)
   void Verb::Done(Count c) noexcept {
      mSuccesses = c;
   }

   /// Satisfy verb once                                                      
   LANGULUS(INLINED)
   void Verb::Done() noexcept {
      ++mSuccesses;
   }

   /// Reset verb satisfaction, clear output                                  
   LANGULUS(INLINED)
   void Verb::Undo() noexcept {
      mSuccesses = 0;
      mOutput.Reset();
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has larger or equal priority                     
   LANGULUS(INLINED)
   bool Verb::operator < (const Verb& ext) const noexcept {
      return mPriority < ext.mPriority;
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has smaller or equal priority                    
   LANGULUS(INLINED)
   bool Verb::operator > (const Verb& ext) const noexcept {
      return mPriority > ext.mPriority;
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has smaller priority                             
   LANGULUS(INLINED)
   bool Verb::operator >= (const Verb& ext) const noexcept {
      return mPriority >= ext.mPriority;
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has larger priority                              
   LANGULUS(INLINED)
   bool Verb::operator <= (const Verb& rhs) const noexcept {
      return mPriority <= rhs.mPriority;
   }

   /// Get the verb id and charge                                             
   ///   @return verb charge                                                  
   LANGULUS(INLINED)
   const Anyness::Charge& Verb::GetCharge() const noexcept {
      return static_cast<const Anyness::Charge&>(*this);
   }

   /// Get the verb mass (a.k.a. magnitude)                                   
   ///   @return the current mass                                             
   LANGULUS(INLINED)
   Langulus::Real Verb::GetMass() const noexcept {
      return Charge::mMass;
   }

   /// Get the verb frequency                                                 
   ///   @return the current frequency                                        
   LANGULUS(INLINED)
   Langulus::Real Verb::GetRate() const noexcept {
      return Charge::mRate;
   }

   /// Get the verb time                                                      
   ///   @return the current time                                             
   LANGULUS(INLINED)
   Langulus::Real Verb::GetTime() const noexcept {
      return Charge::mTime;
   }

   /// Get the verb priority                                                  
   ///   @return the current priority                                         
   LANGULUS(INLINED)
   Langulus::Real Verb::GetPriority() const noexcept {
      return Charge::mPriority;
   }

   /// Get verb source                                                        
   ///   @return the verb source                                              
   LANGULUS(INLINED)
   Anyness::Any& Verb::GetSource() noexcept {
      return mSource;
   }

   /// Get verb source (constant)                                             
   ///   @return the verb source                                              
   LANGULUS(INLINED)
   const Anyness::Any& Verb::GetSource() const noexcept {
      return mSource;
   }

   /// Get verb argument                                                      
   ///   @return the verb argument                                            
   LANGULUS(INLINED)
   Anyness::Any& Verb::GetArgument() noexcept {
      return static_cast<Any&>(*this);
   }

   /// Get verb argument (constant)                                           
   ///   @return the verb argument                                            
   LANGULUS(INLINED)
   const Anyness::Any& Verb::GetArgument() const noexcept {
      return static_cast<const Any&>(*this);
   }

   /// Get verb output                                                        
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   Anyness::Any& Verb::GetOutput() noexcept {
      return mOutput;
   }

   /// Get verb output (constant)                                             
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   const Anyness::Any& Verb::GetOutput() const noexcept {
      return mOutput;
   }

   /// Convenience operator for accessing the output container inside verb    
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   const Anyness::Any* Verb::operator -> () const noexcept {
      return &mOutput;
   }

   /// Convenience operator for accessing the output container inside verb    
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   Anyness::Any* Verb::operator -> () noexcept {
      return &mOutput;
   }
   
   /// Serialize verb to any form of text                                     
   ///   @return the serialized verb                                          
   void Verb::SerializeVerb(CT::Serial auto& out) const {
      using OUT = Deref<decltype(out)>;
      if (mSuccesses) {
         // If verb has been executed, just dump the output             
         mOutput.Serialize(out);
         return;
      }

      // If reached, then verb hasn't been executed yet                 
      // Let's check if there's a source in which verb is executed      
      if (mSource.IsValid()) {
         OUT::SerializationRules::BeginScope(mSource, out);
         mSource.Serialize(out);
         OUT::SerializationRules::EndScope(mSource, out);
      }

      // Write any special qualifiers before the verb token             
      if (IsLongCircuited()) {
         if (mSource.IsValid())
            out += ' ';
         out += OUT::Operator::Long;
      }

      if (IsMonocast()) {
         if (mSource.IsValid() or IsLongCircuited())
            out += ' ';
         out += OUT::Operator::Mono;
      }

      // After the source, we decide whether to write verb token or     
      // verb operator, depending on the verb definition, state and     
      // charge                                                         
      bool writtenAsToken = false;
      if (not mVerb) {
         // An invalid verb is always written as token                  
         if (mSource.IsValid() or IsLongCircuited() or IsMonocast())
            out += ' ';
         out += NameOf<Verb>();
         writtenAsToken = true;
      }
      else {
         // A valid verb is written either as token, or as operator     
         if (mMass < 0) {
            if (not mVerb->mOperatorReverse.empty() and (GetCharge() * -1).IsDefault()) {
               // Write as operator                                     
               out += mVerb->mOperatorReverse;
            }
            else {
               // Write as token                                        
               if (mSource.IsValid() or IsLongCircuited() or IsMonocast())
                  out += ' ';
               out += mVerb->mTokenReverse;
               out += static_cast<OUT>(GetCharge() * -1);
               writtenAsToken = true;
            }
         }
         else {
            if (not mVerb->mOperator.empty() and GetCharge().IsDefault()) {
               // Write as operator                                     
               out += mVerb->mOperator;
            }
            else {
               // Write as token                                        
               if (mSource.IsValid() or IsLongCircuited() or IsMonocast())
                  out += ' ';
               out += mVerb->mToken;
               out += static_cast<OUT>(GetCharge());
               writtenAsToken = true;
            }
         }
      }

      if (not IsValid())
         return;
      
      if (not OUT::SerializationRules::BeginScope(GetArgument(), out)
      and writtenAsToken)
         out += ' ';
      GetArgument().Serialize(out);
      OUT::SerializationRules::EndScope(GetArgument(), out);
   }

   /// Serialize verb for logger                                              
   LANGULUS(INLINED)
   Verb::operator Anyness::Text() const {
      Anyness::Text result;
      SerializeVerb(result);
      return result;
   }
   
   /// Compare verbs                                                          
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   LANGULUS(INLINED)
   bool Verb::operator == (const Verb& rhs) const {
      return mVerb == rhs.mVerb
         and mSource == rhs.mSource
         and Any::operator == (rhs.GetArgument())
         and mOutput == rhs.mOutput
         and mState == rhs.mState;
   }

   /// Compare verb types for equality                                        
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   LANGULUS(INLINED)
   bool Verb::operator == (VMeta rhs) const noexcept {
      return mVerb == rhs;
   }

} // namespace Langulus::A