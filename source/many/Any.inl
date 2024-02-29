///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Any.hpp"
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
   Any::Any(const Any& other)
      : Any {Refer(other)} {}

   /// Move constructor                                                       
   ///   @param other - the container to move                                 
   LANGULUS(INLINED)
   Any::Any(Any&& other) noexcept
      : Any {Move(other)} {}

   /// Unfold constructor                                                     
   /// If there's one deep argument, it will be absorbed                      
   /// If any of the element types don't match exactly, the container becomes 
   /// deep in order to incorporate them                                      
   ///   @param t1 - first element (can be semantic)                          
   ///   @param tn... - the rest of the elements (optional, can be semantic)  
   template<class T1, class...TN>
   requires CT::Inner::UnfoldInsertable<T1, TN...>
   LANGULUS(INLINED) Any::Any(T1&& t1, TN&&...tn) {
      if constexpr (sizeof...(TN) == 0) {
         using S = SemanticOf<decltype(t1)>;
         using T = TypeOf<S>;

         if constexpr (CT::Deep<T>)
            Block::BlockTransfer<Any>(S::Nest(t1));
         else
            Block::Insert<Any, Any, true>(IndexBack, Forward<T1>(t1));
      }
      else Block::Insert<Any, Any, true>(IndexBack, Forward<T1>(t1), Forward<TN>(tn)...);
   }

   /// Destruction                                                            
   LANGULUS(INLINED)
   Any::~Any() {
      Block::Free<Any>();
   }

   /// Create an empty Any from a dynamic type and state                      
   ///   @param type - type of the container                                  
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Any Any::FromMeta(DMeta type, DataState state) noexcept {
      return Any {Block {state, type}};
   }

   /// Create an empty Any by copying type and state of a block               
   ///   @param block - the source of type and state                          
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Any Any::FromBlock(const CT::Block auto& block, DataState state) noexcept {
      return Any::FromMeta(block.GetType(), block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any by copying only state of a block                   
   ///   @param block - the source of the state                               
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Any Any::FromState(const CT::Block auto& block, DataState state) noexcept {
      return Any::FromMeta(nullptr, block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any from a static type and state                       
   ///   @tparam T - the contained type                                       
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   template<CT::Data T> LANGULUS(INLINED)
   Any Any::From(DataState state) noexcept {
      return Block {state, MetaDataOf<T>()};
   }
   
   /// Refer assignment                                                       
   ///   @param rhs - the container to refer to                               
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (const Any& rhs) {
      return operator = (Refer(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the container to move and reset                         
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (Any&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   /// Element/container assignment, semantic or not                          
   ///   @param rhs - the element, or container to assign                     
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         // Potentially absorb a container                              
         if (static_cast<const Block*>(this)
          == static_cast<const Block*>(&DesemCast(rhs)))
            return *this;

         Block::Free();
         new (this) Any {S::Nest(rhs)};
      }
      else if (IsSimilar<Unfold<T>>()) {
         // Unfold-insert by reusing memory                             
         Block::Clear();
         Block::UnfoldInsert<Any, void, true>(IndexFront, S::Nest(rhs));
      }
      else {
         // Allocate anew and unfold-insert                             
         Block::Free();
         new (this) Any {S::Nest(rhs)};
      }

      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator << (CT::Inner::UnfoldInsertable auto&& other) {
      Block::Insert<Any, Any, true>(IndexBack, Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >> (CT::Inner::UnfoldInsertable auto&& other) {
      Block::Insert<Any, Any, true>(IndexFront, Forward<decltype(other)>(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator <<= (CT::Inner::UnfoldInsertable auto&& other) {
      Block::Merge<Any, Any, true>(IndexBack, Forward<decltype(other)>(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >>= (CT::Inner::UnfoldInsertable auto&& other) {
      Block::Merge<Any, Any, true>(IndexFront, Forward<decltype(other)>(other));
      return *this;
   }

   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(INLINED)
   Any Any::Crop(const Offset start, const Count count) {
      return Block::Crop<Any>(start, count);
   }

   LANGULUS(INLINED)
   Any Any::Crop(const Offset start, const Count count) const {
      return Block::Crop<Any>(start, count);
   }
        
   /// Iterate each element block and execute F for it                        
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - function to execute for each element block             
   ///   @return the number of executions                                     
   template<bool REVERSE> LANGULUS(INLINED)
   Count Any::ForEachElement(auto&& call) const {
      return Block::ForEachElement<REVERSE, const Any>(
         Forward<Deref<decltype(call)>>(call));
   }

   template<bool REVERSE> LANGULUS(INLINED)
   Count Any::ForEachElement(auto&& call) {
      return Block::ForEachElement<REVERSE, Any>(
         Forward<Deref<decltype(call)>>(call));
   }

   /// Execute functions for each element inside container                    
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE> LANGULUS(INLINED)
   Count Any::ForEach(auto&&...call) const {
      return Block::ForEach<REVERSE, const Any>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   template<bool REVERSE> LANGULUS(INLINED)
   Count Any::ForEach(auto&&...call) {
      return Block::ForEach<REVERSE, Any>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   /// Execute functions in each sub-block, inclusively                       
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. None of the provided     
   /// functions are ignored.                                                 
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count Any::ForEachDeep(auto&&...call) const {
      return Block::ForEachDeep<REVERSE, SKIP, const Any>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count Any::ForEachDeep(auto&&...call) {
      return Block::ForEachDeep<REVERSE, SKIP, Any>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type is similar to at least one of the types    
   template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool Any::Is() const noexcept {
      return Block::Is<Any, T1, TN...>();
   }

   /// Check if type origin is the same as another                            
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @param type - the type to check for                                  
   ///   @return true if this block contains similar data                     
   LANGULUS(INLINED)
   bool Any::Is(DMeta type) const noexcept {
      return Block::Is(type);
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type is similar to at least one of the types    
   template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool Any::IsSimilar() const noexcept {
      return Block::IsSimilar<Any, T1, TN...>();
   }

   /// Check if unqualified type is the same as another                       
   ///   @attention ignores only cv-qualifiers                                
   ///   @param type - the type to check for                                  
   ///   @return true if this block contains similar data                     
   LANGULUS(INLINED)
   bool Any::IsSimilar(DMeta type) const noexcept {
      return Block::IsSimilar(type);
   }

   /// Check if this type is exactly one of the provided types                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool Any::IsExact() const noexcept {
      return Block::IsExact<Any, T1, TN...>();
   }

   /// Check if this type is exactly another                                  
   ///   @param type - the type to match                                      
   ///   @return true if data type matches type exactly                       
   LANGULUS(INLINED)
   bool Any::IsExact(DMeta type) const noexcept {
      return Block::IsExact(type);
   }

   /// Concatenate with any deep type, semantically or not                    
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Any Any::operator + (CT::Inner::UnfoldInsertable auto&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      return Block::ConcatBlock<Any>(S::Nest(rhs));
   }

   /// Destructive concatenate with any deep type, semantically or not        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Any& Any::operator += (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      Block::InsertBlock<Any, void>(IndexBack, S::Nest(rhs));
      return *this;
   }
   
} // namespace Langulus::Anyness
