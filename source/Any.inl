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
   requires CT::Inner::UnfoldInsertable<T1, TAIL...> LANGULUS(INLINED)
   Any::Any(T1&& t1, TAIL&&... tail) {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using T = TypeOf<S>;

         if constexpr (CT::Deep<T>)
            BlockTransfer<Any>(S::Nest(t1));
         else
            Insert<Any>(0, Forward<T1>(t1));
      }
      else Insert<Any>(0, Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Destruction                                                            
   LANGULUS(INLINED)
   Any::~Any() {
      Free();
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
   Any Any::FromBlock(const Block& block, DataState state) noexcept {
      return Any::FromMeta(block.GetType(), block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any by copying only state of a block                   
   ///   @param block - the source of the state                               
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Any Any::FromState(const Block& block, DataState state) noexcept {
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
   template<class AS, CT::Data T1, CT::Data T2, CT::Data... TN> LANGULUS(INLINED)
   Any Any::WrapAs(T1&& t1, T2&& t2, TN&&... tail) {
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
         if (this == &*S::Nest(rhs))
            return *this;

         Free();
         new (this) Any {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Clear();
         UnfoldInsert<Any, void>(0, S::Nest(rhs));
      }

      return *this;
   }

   /// Destroy all elements, but retain allocated memory if possible          
   LANGULUS(INLINED)
   void Any::Clear() {
      if (IsEmpty())
         return;

      if (GetUses() == 1) {
         // Only one use - just destroy elements and reset count,       
         // reusing the allocation for later                            
         CallUnknownDestructors();
         ClearInner();
      }
      else {
         // We're forced to reset the memory, because it's in use       
         // Keep the type and state, though                             
         const auto state = GetUnconstrainedState();
         const auto meta = mType;
         Reset();
         mType = meta;
         mState += state;
      }
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator << (CT::Inner::UnfoldInsertable auto&& other) {
      Insert<Any>(IndexBack, Forward<Deref<decltype(other)>>(other));
      return *this;
   }
   
   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >> (CT::Inner::UnfoldInsertable auto&& other) {
      Insert<Any>(IndexFront, Forward<Deref<decltype(other)>>(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator <<= (CT::Inner::UnfoldInsertable auto&& other) {
      Merge<Any>(IndexBack, Forward<Deref<decltype(other)>>(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >>= (CT::Inner::UnfoldInsertable auto&& other) {
      Merge<Any>(IndexFront, Forward<Deref<decltype(other)>>(other));
      return *this;
   }

   /// Reset the container                                                    
   LANGULUS(INLINED)
   void Any::Reset() {
      Free();
      mRaw = nullptr;
      mCount = mReserved = 0;
      ResetState();
   }

   /// Swap two container's contents                                          
   ///   @param other - [in/out] the container to swap contents with          
   LANGULUS(INLINED)
   void Any::Swap(Any& other) noexcept {
      other = ::std::exchange(*this, ::std::move(other));
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(INLINED)
   Any Any::Crop(const Offset& start, const Count& count) const {
      return Block::Crop<Any>(start, count);
   }

   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(INLINED)
   Any Any::Crop(const Offset& start, const Count& count) {
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
   
   /// Find element(s) index inside container                                 
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @return the index of the found item, or IndexNone if none found      
   template<bool REVERSE> LANGULUS(INLINED)
   Index Any::Find(const CT::Data auto& item, const Offset& cookie) const {
      return Block::template FindKnown<REVERSE>(item, cookie);
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   LANGULUS(INLINED)
   typename Any::Iterator Any::begin() noexcept {
      return IsEmpty() ? end() : GetElement();
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   LANGULUS(INLINED)
   typename Any::Iterator Any::end() noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * mCount;
      result.mCount = 0;
      return result;
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   LANGULUS(INLINED)
   typename Any::Iterator Any::last() noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * (mCount - 1);
      result.mCount = 1;
      return result;
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   LANGULUS(INLINED)
   typename Any::ConstIterator Any::begin() const noexcept {
      return IsEmpty() ? end() : GetElement();
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   LANGULUS(INLINED)
   typename Any::ConstIterator Any::end() const noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * mCount;
      result.mCount = 0;
      return result;
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   LANGULUS(INLINED)
   typename Any::ConstIterator Any::last() const noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * (mCount - 1);
      result.mCount = 1;
      return result;
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
   ///   @tparam A... - argument types (deducible)                            
   ///   @param region - the region to emplace at                             
   ///   @param count - the number of elements to emplace                     
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return the number of emplaced elements                              
   template<CT::Block THIS, class... A>
   void Block::EmplaceInner(const Block& region, Count count, A&&... arguments) {
      if constexpr (sizeof...(A) == 0) {
         // Attempt default construction                                
         //TODO if stuff moved, we should move stuff back if this throws...
         region.CallUnknownDefaultConstructors(count);
         mCount += count;
         return;
      }
      else if constexpr (sizeof...(A) == 1) {
         using F = Decvq<Deref<FirstOf<A...>>>;

         if constexpr (CT::Exact<Neat, F>) {
            // Attempt descriptor-construction                          
            region.CallUnknownDescriptorConstructors(count, arguments...);
            mCount += count;
            return;
         }
         else if (IsExact<F>()) {
            // Attempt move-construction, if available                  
            region.template CallKnownConstructors<F>(
               count, Forward<A>(arguments)...
            );
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
      region.CallUnknownDescriptorConstructors(count, descriptor);
      mCount += count;
   }
   

   ///                                                                        
   ///   Block iterator                                                       
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param value - pointer to the value element                          
   template<bool MUTABLE> LANGULUS(INLINED)
   Any::TIterator<MUTABLE>::TIterator(const Block& value) noexcept
      : mValue {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE> LANGULUS(INLINED)
   typename Any::TIterator<MUTABLE>& Any::TIterator<MUTABLE>::operator ++ () noexcept {
      mValue.mRaw += mValue.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE> LANGULUS(INLINED)
   typename Any::TIterator<MUTABLE> Any::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare block entries                                                  
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE> LANGULUS(INLINED)
   bool Any::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mValue.mRaw == rhs.mValue.mRaw;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE> LANGULUS(INLINED)
   const Block& Any::TIterator<MUTABLE>::operator * () const noexcept {
      return mValue;
   }

} // namespace Langulus::Anyness
