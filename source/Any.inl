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

   /// Copy constructor - does only a shallow copy                            
   ///   @param other - the container to shallow-copy                         
   LANGULUS(INLINED)
   Any::Any(const Any& other)
      : Any {Copy(other)} {}

   /// Move constructor - transfers ownership                                 
   ///   @param other - the container to move                                 
   LANGULUS(INLINED)
   Any::Any(Any&& other) noexcept
      : Any {Move(other)} {}

   /// Construct by shallow-copying element/container                         
   ///   @param other - the element/container to shallow-copy                 
   LANGULUS(INLINED)
   Any::Any(const CT::NotSemantic auto& other)
      : Any {Copy(other)} {}

   /// Construct by shallow-copying element/container                         
   ///   @param other - the element/container to shallow-copy                 
   LANGULUS(INLINED)
   Any::Any(CT::NotSemantic auto& other)
      : Any {Copy(other)} {}

   /// Construct by moving element/container                                  
   ///   @param other - the element/container to move                         
   LANGULUS(INLINED)
   Any::Any(CT::NotSemantic auto&& other)
      : Any {Move(other)} {}

   /// Semantic constructor from deep container or custom data element        
   ///   @param other - the element/container to initialize with              
   LANGULUS(INLINED)
   Any::Any(CT::Semantic auto&& other) noexcept {
      CreateFrom(other.Forward());
   }

   /// Pack any number of elements sequentially                               
   /// If any of the types doesn't match exactly, the container becomes deep  
   /// to incorporate all elements                                            
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   template<CT::Data T1, CT::Data T2, CT::Data... TN>
   Any::Any(T1&& t1, T2&& t2, TN&&... tail) {
      if constexpr (CT::Exact<T1, T2, TN...>) {
         // All types are the same, so pack them tightly                
         SetType<Decvq<Deref<T1>>>();
         AllocateFresh(RequestSize(sizeof...(TN) + 2));
         Insert(Forward<T1>(t1));
         Insert(Forward<T2>(t2));
         (Insert(Forward<TN>(tail)), ...);
      }
      else {
         // Types differ, so wrap each of them in a separate Any        
         SetType<Any>();
         AllocateFresh(RequestSize(sizeof...(TN) + 2));
         Insert(Abandon(Any {Forward<T1>(t1)}));
         Insert(Abandon(Any {Forward<T2>(t2)}));
         (Insert(Abandon(Any {Forward<TN>(tail)})), ...);
      }
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
   Any Any::FromMeta(DMeta type, const DataState& state) noexcept {
      return Any {Block {state, type}};
   }

   /// Create an empty Any by copying type and state of a block               
   ///   @param block - the source of type and state                          
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Any Any::FromBlock(const Block& block, const DataState& state) noexcept {
      return Any::FromMeta(block.GetType(), block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any by copying only state of a block                   
   ///   @param block - the source of the state                               
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(INLINED)
   Any Any::FromState(const Block& block, const DataState& state) noexcept {
      return Any::FromMeta(nullptr, block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any from a static type and state                       
   ///   @tparam T - the contained type                                       
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   template<CT::Data T>
   LANGULUS(INLINED)
   Any Any::From(const DataState& state) noexcept {
      return Block {state, MetaData::Of<T>()};
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
   LANGULUS(INLINED)
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
   ///   @param other - the container to copy                                 
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (const Any& other) {
      return operator = (Copy(other));
   }

   /// Move assignment                                                        
   ///   @param other - the container to move and reset                       
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (Any&& other) noexcept {
      return operator = (Move(other));
   }

   /// Shallow copy assignment of anything                                    
   ///   @param other - the value to copy                                     
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (const CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }
   
   LANGULUS(INLINED)
   Any& Any::operator = (CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }

   /// Move assignment of anything                                            
   ///   @param other - the value to move in                                  
   ///   @return a reference to this container                                
   LANGULUS(INLINED)
   Any& Any::operator = (CT::NotSemantic auto&& other) {
      return operator = (Move(other));
   }

   /// Semantic assignment                                                    
   ///   @param other - the container to semantically assign                  
   ///   @return a reference to this container                                
   Any& Any::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;
      static_assert(CT::Insertable<T>, "T must be an insertable type");

      if constexpr (CT::Deep<T>) {
         // Assign a container                                          
         if (this == &*other)
            return *this;

         // Since Any is type-erased, we make a runtime type check      
         LANGULUS_ASSERT(
            not IsTypeConstrained() or CastsToMeta(other->GetType()),
            Assign, "Incompatible types on assignment (deep)",
            " of ", other->GetType(), " to ", mType
         );

         Free();
         new (this) Any {other.Forward()};
      }
      else if constexpr (CT::CustomData<T>) {
         // Assign a non-deep value                                     
         if constexpr (CT::Array<T>) {
            using E = Deext<T>;
            constexpr auto extent = ExtentOf<T>;

            if constexpr (CT::StringLiteral<T>) {
               // Assign a text literal, by implicitly creating a Text  
               // container and wrapping it inside                      
               CheckType<Text>();

               if (GetUses() != 1 or mType->mIsSparse) {
                  // Reset and allocate fresh memory                    
                  Reset();
                  operator << (Text {other.Forward()});
               }
               else {
                  // Destroy all elements, except the first one, and    
                  // assign to it                                       
                  if (mCount > 1) {
                     CropInner(1, mCount - 1).
                        template CallKnownDestructors<Text>();
                     mCount = 1;
                  }
                  SemanticAssign(Get<Text>(), Abandon(Text {other.Forward()}));
               }
            }
            else {
               // Assign an array of elements                           
               CheckType<E>();

               if (GetUses() != 1 or mType->mIsSparse != CT::Sparse<E>) {
                  // Reset and allocate fresh memory                    
                  Reset();
                  SetType<E>();
                  AllocateFresh(RequestSize(extent));
                  InsertInner<S>(*other, *other + extent, 0);
               }
               else {
                  // Destroy all elements, except the required number,  
                  // and assign to them                                 
                  if (mCount > extent) {
                     CropInner(extent, mCount - extent).
                        template CallKnownDestructors<E>();
                     mCount = extent;
                  }

                  for (Offset i = 0; i < extent; ++i)
                     GetHandle<E>(i).Assign(S::Nest((*other)[i]));
               }
            }
         }
         else {
            // Assign a single element                                  
            CheckType<T>();

            if (GetUses() != 1 or mType->mIsSparse != CT::Sparse<T>) {
               // Reset and allocate fresh memory                       
               Reset();
               operator << (other.Forward());
            }
            else {
               // Destroy all elements, except the first one, and       
               // assign to it                                          
               if (mCount > 1) {
                  CropInner(1, mCount - 1).
                     template CallKnownDestructors<T>();
                  mCount = 1;
               }
               GetHandle<T>(0).Assign(other.Forward());
            }
         }
      }
      else LANGULUS_ERROR("Bad semantic constructor argument");
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

   /// Copy-insert an element (including arrays) at the back                  
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator << (const CT::NotSemantic auto& other) {
      Insert<IndexBack>(Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   LANGULUS(INLINED)
   Any& Any::operator << (CT::NotSemantic auto& other) {
      Insert<IndexBack>(Copy(other));
      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator << (CT::NotSemantic auto&& other) {
      Insert<IndexBack>(Move(other));
      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator << (CT::Semantic auto&& other) {
      Insert<IndexBack>(other.Forward());
      return *this;
   }

   /// Copy-insert an element (including arrays) at the front                 
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >> (const CT::NotSemantic auto& other) {
      Insert<IndexFront>(Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   LANGULUS(INLINED)
   Any& Any::operator >> (CT::NotSemantic auto& other) {
      Insert<IndexFront>(Copy(other));
      return *this;
   }

   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >> (CT::NotSemantic auto&& other) {
      Insert<IndexFront>(Move(other));
      return *this;
   }
   
   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >> (CT::Semantic auto&& other) {
      Insert<IndexFront>(other.Forward());
      return *this;
   }

   /// Merge data (including arrays) at the back                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator <<= (const CT::NotSemantic auto& other) {
      Merge<IndexBack, true>(Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   LANGULUS(INLINED)
   Any& Any::operator <<= (CT::NotSemantic auto& other) {
      Merge<IndexBack, true>(Copy(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator <<= (CT::NotSemantic auto&& other) {
      Merge<IndexBack, true>(Move(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator <<= (CT::Semantic auto&& other) {
      Merge<IndexBack, true>(other.Forward());
      return *this;
   }

   /// Merge data at the front                                                
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >>= (const CT::NotSemantic auto& other) {
      Merge<IndexFront, true>(Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   LANGULUS(INLINED)
   Any& Any::operator >>= (CT::NotSemantic auto& other) {
      Merge<IndexFront, true>(Copy(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >>= (CT::NotSemantic auto&& other) {
      Merge<IndexFront, true>(Move(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   LANGULUS(INLINED)
   Any& Any::operator >>= (CT::Semantic auto&& other) {
      Merge<IndexFront, true>(other.Forward());
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
      return Any {Block::Crop(start, count)};
   }

   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(INLINED)
   Any Any::Crop(const Offset& start, const Count& count) {
      return Any {Block::Crop(start, count)};
   }




   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// An inner concatenation routine using semantics                         
   ///   @tparam WRAPPER - the type of the concatenated container             
   ///   @attention assumes TypeOf<S> is binary compatible to WRAPPER,        
   ///              despite being a block to reduce compilation time and RAM  
   ///   @param rhs - block and semantic to concatenate                       
   ///   @return the concatenated container                                   
   template<CT::Block WRAPPER>
   WRAPPER Any::Concatenate(CT::Semantic auto&& other) const {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;
      using NestedT = Conditional<S::Move, WRAPPER&, const WRAPPER&>;
      static_assert(CT::Exact<T, Block>,
         "S type must be exactly Block (build-time optimization)");

      auto& lhs = reinterpret_cast<const WRAPPER&>(*this);
      auto& rhs = reinterpret_cast<NestedT>(*other);
      if (IsEmpty())
         return S::Nest(rhs);
      else if (rhs.IsEmpty())
         return lhs;

      WRAPPER result;
      if constexpr (not CT::Typed<WRAPPER>)
         result.SetType(mType);

      result.AllocateFresh(result.RequestSize(mCount + rhs.mCount));
      result.InsertBlock(lhs);
      result.InsertBlock(S::Nest(rhs));
      return Abandon(result);
   }

   /// Copy-concatenate with any deep type                                    
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Any Any::operator + (const CT::Deep auto& rhs) const {
      return Concatenate<Any>(Copy(static_cast<const Block&>(rhs)));
   }

   LANGULUS(INLINED)
   Any Any::operator + (CT::Deep auto& rhs) const {
      return Concatenate<Any>(Copy(static_cast<const Block&>(rhs)));
   }

   /// Move-concatenate with any deep type                                    
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Any Any::operator + (CT::Deep auto&& rhs) const {
      return Concatenate<Any>(Move(Forward<Block>(rhs)));
   }

   /// Semantically concatenate with any deep type                            
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   LANGULUS(INLINED)
   Any Any::operator + (CT::Semantic auto&& rhs) const {
      return Concatenate<Any>(rhs.template Forward<Block>());
   }

   /// Destructive copy-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Any& Any::operator += (const CT::Deep auto& rhs) {
      InsertBlock(Copy(rhs));
      return *this;
   }

   LANGULUS(INLINED)
   Any& Any::operator += (CT::Deep auto& rhs) {
      InsertBlock(Copy(rhs));
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Any& Any::operator += (CT::Deep auto&& rhs) {
      InsertBlock(Move(rhs));
      return *this;
   }

   /// Destructive semantically concatenate with any deep type                
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   LANGULUS(INLINED)
   Any& Any::operator += (CT::Semantic auto&& rhs) {
      InsertBlock(rhs.Forward());
      return *this;
   }
   
   /// Find element(s) index inside container                                 
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @return the index of the found item, or IndexNone if none found      
   template<bool REVERSE>
   LANGULUS(INLINED)
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
   template<class... A>
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
   template<bool MUTABLE>
   LANGULUS(INLINED)
   Any::TIterator<MUTABLE>::TIterator(const Block& value) noexcept
      : mValue {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename Any::TIterator<MUTABLE>& Any::TIterator<MUTABLE>::operator ++ () noexcept {
      mValue.mRaw += mValue.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE>
   LANGULUS(INLINED)
   typename Any::TIterator<MUTABLE> Any::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare block entries                                                  
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE>
   LANGULUS(INLINED)
   bool Any::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mValue.mRaw == rhs.mValue.mRaw;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE>
   LANGULUS(INLINED)
   const Block& Any::TIterator<MUTABLE>::operator * () const noexcept {
      return mValue;
   }

} // namespace Langulus::Anyness
