///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Many.hpp"
#include "../DataState.inl"
#include "../one/Handle.inl"
#include "../blocks/Block/Block-Capsulation.inl"
#include "../blocks/Block/Block-Iteration.inl"
#include "../blocks/Block/Block-Indexing.inl"
#include "../blocks/Block/Block-Construct.inl"
#include "../blocks/Block/Block-RTTI.inl"
#include "../blocks/Block/Block-Remove.inl"
#include "../blocks/Block/Block-Memory.inl"
#include "../blocks/Block/Block-Insert.inl"
#include "../blocks/Block/Block-Convert.inl"
#include "../blocks/Block/Block-Compare.inl"


namespace Langulus::Anyness
{

   /// Refer constructor                                                      
   ///   @param other - the container to refer to                             
   LANGULUS(INLINED)
   Many::Many(const Many& other)
      : Many {Refer(other)} {}

   /// Move constructor                                                       
   ///   @param other - the container to move                                 
   LANGULUS(INLINED)
   Many::Many(Many&& other) noexcept
      : Many {Move(other)} {}

   /// Unfold constructor                                                     
   /// If there's one deep argument, it will be absorbed                      
   /// If any of the element types don't match exactly, the container becomes 
   /// deep in order to incorporate them                                      
   ///   @param t1 - first element (can be semantic)                          
   ///   @param tn... - the rest of the elements (optional, can be semantic)  
   template<class T1, class...TN> requires CT::UnfoldInsertable<T1, TN...>
   LANGULUS(INLINED) Many::Many(T1&& t1, TN&&...tn) {
      Base::BlockCreate(Forward<T1>(t1), Forward<TN>(tn)...);
   }

   /// Destruction                                                            
   LANGULUS(INLINED)
   Many::~Many() {
      Base::Free();
   }

   /// Create an empty Many from a dynamic type and state                     
   ///   @param type - type of the container                                  
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Many Many::FromMeta(DMeta type, DataState state) noexcept {
      return Many {Base {state, type}};
   }

   /// Create an empty Many by copying type and state of a block              
   ///   @param block - the source of type and state                          
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Many Many::FromBlock(const CT::Block auto& block, DataState state) noexcept {
      return Many::FromMeta(block.GetType(), block.GetUnconstrainedState() + state);
   }

   /// Create an empty Many by copying only state of a block                  
   ///   @param block - the source of the state                               
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Many Many::FromState(const CT::Block auto& block, DataState state) noexcept {
      return Many::FromMeta(nullptr, block.GetUnconstrainedState() + state);
   }

   /// Create an empty Many from a static type and state                      
   ///   @tparam T - the contained type                                       
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   template<CT::Data T> LANGULUS(INLINED)
   Many Many::From(DataState state) noexcept {
      return Base {state, MetaDataOf<T>()};
   }
   
   /// Insert the provided elements, making sure to insert and never absorb   
   ///   @tparam AS - the type to wrap elements as, use void to auto-deduce   
   ///   @param items... - items to insert                                    
   ///   @returns the new container containing the data                       
   template<class AS, CT::Data...TN> LANGULUS(INLINED)
   Many Many::Wrap(TN&&...items) {
      if constexpr (CT::TypeErased<AS>)
         return WrapBlock<Many>(Forward<TN>(items)...);
      else {
         // Forcefully type-unlock after wrapping in a TMany block      
         Many result = WrapBlock<TMany<AS>>(Forward<TN>(items)...);
         result.mState -= DataState::Typed;
         return result;
      }
   }

   /// Refer assignment                                                       
   ///   @param rhs - the container to refer to                               
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Many& Many::operator = (const Many& rhs) {
      static_assert(CT::DeepAssignable<void, Referred<Many>>);
      return operator = (Refer(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the container to move and reset                         
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Many& Many::operator = (Many&& rhs) noexcept {
      static_assert(CT::DeepAssignable<void, Moved<Many>>);
      return operator = (Move(rhs));
   }

   /// Element/container assignment, semantic or not                          
   ///   @param rhs - the element, or container to assign                     
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Many& Many::operator = (CT::UnfoldInsertable auto&& rhs) {
      return Base::BlockAssign<Many>(Forward<decltype(rhs)>(rhs));
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Many& Many::operator << (CT::UnfoldInsertable auto&& other) {
      Base::Insert<Many>(IndexBack, Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Many& Many::operator >> (CT::UnfoldInsertable auto&& other) {
      Base::Insert<Many>(IndexFront, Forward<decltype(other)>(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Many& Many::operator <<= (CT::UnfoldInsertable auto&& other) {
      Base::Merge<Many>(IndexBack, Forward<decltype(other)>(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Many& Many::operator >>= (CT::UnfoldInsertable auto&& other) {
      Base::Merge<Many>(IndexFront, Forward<decltype(other)>(other));
      return *this;
   }

   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(INLINED)
   Many Many::Crop(const Offset start, const Count count) IF_UNSAFE(noexcept) {
      return Base::Crop<Many>(start, count);
   }

   LANGULUS(INLINED)
   Many Many::Crop(const Offset start, const Count count) const IF_UNSAFE(noexcept) {
      return Base::Crop<Many>(start, count);
   }

   /// Concatenate with any deep type, semantically or not                    
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Many Many::operator + (CT::UnfoldInsertable auto&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      return ConcatBlock<Many>(S::Nest(rhs));
   }

   /// Destructive concatenate with any deep type, semantically or not        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Many& Many::operator += (CT::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      InsertBlock<void>(IndexBack, S::Nest(rhs));
      return *this;
   }
   
} // namespace Langulus::Anyness
