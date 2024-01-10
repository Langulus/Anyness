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
#include "Neat.hpp"
#include "blocks/Block/Block-Capsulation.inl"
#include "blocks/Block/Block-Compare.inl"
#include "blocks/Block/Block-Construct.inl"
#include "blocks/Block/Block-Indexing.inl"
#include "blocks/Block/Block-Insert.inl"
#include "blocks/Block/Block-Iteration.inl"
#include "blocks/Block/Block-Memory.inl"
#include "blocks/Block/Block-Remove.inl"
#include "blocks/Block/Block-RTTI.inl"
#include <utility>


namespace Langulus::Anyness
{

   /// Shallow-copy constructor                                               
   ///   @param other - the container to shallow-copy                         
   LANGULUS(INLINED)
   Any::Any(const Any& other)
      : Any {Copy(other)} {}

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
   ///   @param tail... - the rest of the elements (optional, can be semantic)
   template<class T1, class...TAIL>
   requires CT::Inner::UnfoldInsertable<T1, TAIL...>
   LANGULUS(INLINED) Any::Any(T1&& t1, TAIL&&... tail) {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using T = TypeOf<S>;

         if constexpr (CT::Deep<T>)
            BlockTransfer<Any>(S::Nest(t1));
         else
            Insert<Any, Any, true>(IndexBack, Forward<T1>(t1));
      }
      else Insert<Any, Any, true>(IndexBack, Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Destruction                                                            
   LANGULUS(INLINED)
   Any::~Any() {
      Free<Any>();
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

   /// Pack any number of similarly typed elements sequentially               
   ///   @tparam AS - the type to wrap elements as                            
   ///                use 'void' to deduce AS from the HEAD                   
   ///                (void by default)                                       
   ///   @tparam T1 - the first element type (deducible)                      
   ///   @tparam T2 - the first element type (deducible)                      
   ///   @tparam TN... - the rest of the element types (deducible)            
   ///   @param t1 - first element                                            
   ///   @param t2 - second element                                           
   ///   @param tail... - the rest of the elements                            
   ///   @returns the new container containing the data                       
   template<class AS, CT::Data T1, CT::Data T2, CT::Data... TN>
   LANGULUS(INLINED) Any Any::WrapAs(T1&& t1, T2&& t2, TN&&... tail) {
      if constexpr (CT::Void<AS>) {
         static_assert(CT::Exact<T1, T2, TN...>, "Type mismatch");
         return {Forward<T1>(t1), Forward<T2>(t2), Forward<TN>(tail)...};
      }
      else {
         static_assert(CT::DerivedFrom<T1, AS>, "T1 not related");
         static_assert(CT::DerivedFrom<T2, AS>, "T2 not related");
         static_assert((CT::DerivedFrom<TN, AS> and ...), "Tail not related");
         return {Forward<AS>(t1), Forward<AS>(t2), Forward<AS>(tail)...};
      }
   }
   
   /// Shallow-copy assignment                                                
   ///   @param rhs - the container to copy                                   
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (const Any& rhs) {
      return operator = (Copy(rhs));
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

         Free<Any>();
         new (this) Any {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Clear();
         UnfoldInsert<Any, void, true>(IndexFront, S::Nest(rhs));
      }

      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator << (CT::Inner::UnfoldInsertable auto&& other) {
      Insert<Any, Any, true>(IndexBack, Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >> (CT::Inner::UnfoldInsertable auto&& other) {
      Insert<Any, Any, true>(IndexFront, Forward<decltype(other)>(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator <<= (CT::Inner::UnfoldInsertable auto&& other) {
      Merge<Any, Any, true>(IndexBack, Forward<decltype(other)>(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >>= (CT::Inner::UnfoldInsertable auto&& other) {
      Merge<Any, Any, true>(IndexFront, Forward<decltype(other)>(other));
      return *this;
   }

   /// Swap two container's contents                                          
   ///   @param other - [in/out] the container to swap contents with          
   LANGULUS(INLINED)
   void Any::SwapBlock(CT::Block auto& other) noexcept {
      other = ::std::exchange(*this, ::std::move(other)); //TODO sketchy, does std::exchange do what we want?
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(INLINED)
   Any Any::Crop(const Offset start, const Count count) const {
      return Block::Crop<Any>(start, count);
   }

   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(INLINED)
   Any Any::Crop(const Offset start, const Count count) {
      return Block::Crop<Any>(start, count);
   }


   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// Concatenate with any deep type, semantically or not                    
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Any Any::operator + (CT::Inner::UnfoldInsertable auto&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      return ConcatBlock<Any>(S::Nest(rhs));
   }

   /// Destructive concatenate with any deep type, semantically or not        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Any& Any::operator += (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      InsertBlock<Any, void>(IndexBack, S::Nest(rhs));
      return *this;
   }
   
   /// Construct an item of this container's type at the specified position   
   /// by forwarding A... as constructor arguments                            
   /// Since this container is type-erased and exact constructor signatures   
   /// aren't reflected, the following constructors will be attempted:        
   ///   1. If A is a single argument of exactly the same type, the reflected 
   ///      move constructor will be used, if available                       
   ///   2. If A is empty, the reflected default constructor is used          
   ///   3. If A is not empty, not exactly same as the contained type, or     
   ///      is more than a single argument, then all arguments will be        
   ///      wrapped in an Any, and then forwarded to the descriptor-          
   ///      constructor, if such is reflected                                 
   ///   If none of these constructors are available, this function throws    
   ///   Except::Construct                                                    
   ///   @param region - the region to emplace at, assumed to be part of this 
   ///   @param count - the number of elements to emplace                     
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return the number of emplaced elements                              
   template<CT::Block THIS, class... A>
   void Block::EmplaceInner(const Block& region, Count count, A&&...arguments) {
      if constexpr (sizeof...(A) == 0) {
         // Attempt default construction                                
         region.CallDefaultConstructors<THIS>(count);
         mCount += count;
         return;
      }
      else if constexpr (sizeof...(A) == 1) {
         using F = Deref<FirstOf<A...>>;
         if constexpr (CT::Similar<Describe, F>) {
            // Attempt descriptor-construction                          
            region.CallDescriptorConstructors<THIS>(count, arguments...);
            mCount += count;
            return;
         }
         else if (IsSimilar<F>()) {
            // Use constructor signature directly                       
            region.CallConstructors<TAny<F>>(count, Forward<A>(arguments)...);
            mCount += count;
            return;
         }
      }

      // Attempt wrapping argument(s) in a Neat, and doing              
      // descriptor-construction, if available                          
      LANGULUS_ASSERT(
         mType->mDescriptorConstructor, Construct,
         "Can't descriptor-construct element"
         " - no descriptor-constructor reflected"
      );

      //TODO if stuff moved, we should move stuff back if this throws...
      const Neat descriptor {arguments...};
      region.CallDescriptorConstructors<THIS>(count, descriptor);
      mCount += count;
   }
   
} // namespace Langulus::Anyness
