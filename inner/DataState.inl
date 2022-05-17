///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "DataState.hpp"

namespace Langulus::Anyness
{
   
   /// Manual construction                                                    
   ///   @param state - the state                                             
   constexpr DataState::DataState(const Type& state) noexcept
      : mState {state} {}

   /// Explicit convertion to bool                                            
   ///   @return true if state is not default                                 
   constexpr DataState::operator bool() const noexcept {
      return !IsDefault();
   }
   
   /// Combine two states                                                     
   ///   @param rhs - the other state                                         
   ///   @return a new combined state                                         
   constexpr DataState DataState::operator + (const DataState& rhs) const noexcept {
      return mState | rhs.mState;
   }
   
   /// Remove rhs state from this state                                       
   ///   @param rhs - the other state                                         
   ///   @return a new leftover state                                         
   constexpr DataState DataState::operator - (const DataState& rhs) const noexcept {
      return mState & (~rhs.mState);
   }
   
   /// Destructively add state                                                
   ///   @param rhs - the other state                                         
   ///   @return a reference to this state                                    
   constexpr DataState& DataState::operator += (const DataState& rhs) noexcept {
      mState |= rhs.mState;
      return *this;
   }
   
   /// Destructively remove state                                             
   ///   @param rhs - the other state                                         
   ///   @return a reference to this state                                    
   constexpr DataState& DataState::operator -= (const DataState& rhs) noexcept {
      mState &= ~rhs.mState;
      return *this;
   }
   
   constexpr bool DataState::operator & (const DataState& rhs) const noexcept {
      return (mState & rhs.mState) == rhs.mState;
   }
   
   constexpr bool DataState::operator % (const DataState& rhs) const noexcept {
      return (mState & rhs.mState) == 0;
   }
   
   /// Check if default data state                                            
   /// Default state is inclusive, mutable, nonpolar, nonvacuum, nonstatic,   
   /// nonencrypted, noncompressed, untyped, and dense                        
   constexpr bool DataState::IsDefault() const noexcept {
      return mState == DataState::Default;
   }
   
   /// Check if state is phased                                               
   constexpr bool DataState::IsPhased() const noexcept {
      return mState & DataState::Phased;
   }
   
   /// Check if state is marked missing                                       
   constexpr bool DataState::IsMissing() const noexcept {
      return mState & DataState::Missing;
   }
   
   /// Check if data is compressed                                            
   constexpr bool DataState::IsCompressed() const noexcept {
      return mState & DataState::Compressed;
   }
   
   /// Check if data is encrypted                                             
   constexpr bool DataState::IsEncrypted() const noexcept {
      return mState & DataState::Encrypted;
   }
   
   /// Check if data is marked exlusive (OR)                                  
   constexpr bool DataState::IsOr() const noexcept {
      return mState & DataState::Or;
   }
   
   /// Check if data is future-phased                                         
   constexpr bool DataState::IsFuture() const noexcept {
      return mState & DataState::Future;
   }
   
   /// Check if data is past-phased                                           
   constexpr bool DataState::IsPast() const noexcept {
      return mState & DataState::Past;
   }
   
   /// Check if data is static (size-constrained)                             
   constexpr bool DataState::IsStatic() const noexcept {
      return mState & DataState::Static;
   }
   
   /// Check if data is constant (change-constrained)                         
   constexpr bool DataState::IsConstant() const noexcept {
      return mState & DataState::Constant;
   }
   
   /// Check if data is type-constrained                                      
   constexpr bool DataState::IsTyped() const noexcept {
      return mState & DataState::Typed;
   }
   
   /// Check if data is made of pointers                                      
   constexpr bool DataState::IsSparse() const noexcept {
      return mState & DataState::Sparse;
   }
   
   /// Check if data is either size-, change- or type-constrained             
   constexpr bool DataState::IsConstrained() const noexcept {
      return mState & DataState::Constrained;
   }
   
   /// Reset the state to default                                             
   constexpr void DataState::Reset() noexcept {
      mState = DataState::Default;
   }
   
} // namespace Langulus::Anyness
