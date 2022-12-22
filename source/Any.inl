///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

   /// Copy construct - does a shallow copy, and references                   
   ///   @param other - the container to shallow-copy                         
   inline Any::Any(const Any& other)
      : Any {Langulus::Copy(other)} {}

   /// Construct by moving another container                                  
   ///   @param other - the container to move                                 
   inline Any::Any(Any&& other) noexcept
      : Any {Langulus::Move(other)} {}

   /// Copy construct - does a shallow copy, and references                   
   ///   @param other - the container to shallow-copy                         
   template<CT::Deep T>
   Any::Any(const T& other)
      : Any {Langulus::Copy(other)} {}

   template<CT::Deep T>
   Any::Any(T& other)
      : Any {Langulus::Copy(other)} {}

   /// Construct by moving another container                                  
   ///   @param other - the container to move                                 
   template<CT::Deep T>
   Any::Any(T&& other) requires CT::Mutable<T>
      : Any {Langulus::Move(other)} {}

   /// Same as copy-construction, but doesn't reference anything              
   ///   @param other - the block to copy                                     
   template<CT::Semantic S>
   constexpr Any::Any(S&& other) noexcept requires(CT::Deep<TypeOf<S>>)
      : Block {static_cast<const Block&>(other.mValue)} {
      if constexpr (!S::Move) {
         if constexpr (S::Keep)
            Keep();
         else 
            mEntry = nullptr;
      }
      else {
         if constexpr (S::Keep) {
            if constexpr (CT::Exact<TypeOf<S>, Block>) {
               // Since we are not aware if that block is referenced or 
               // not we reference it just in case, and we also do not  
               // reset 'other' to avoid leaks. When using raw Blocks,  
               // it's your responsibility to take care of ownership.   
               Keep();
            }
            else {
               other.mValue.ResetMemory();
               other.mValue.ResetState();
            }
         }
         else other.mValue.mEntry = nullptr;
      }
   }

   /// Construct by copying/referencing value of non-block type               
   ///   @tparam T - the data type to push (deducible)                        
   ///   @param other - the dense value to shallow-copy                       
   template<CT::CustomData T>
   Any::Any(const T& other) 
      : Any {Langulus::Copy(other)} {}

   template<CT::CustomData T>
   Any::Any(T& other)
      : Any {Langulus::Copy(other)} {}

   /// Construct by moving a dense value of non-block type                    
   ///   @tparam T - the data type to push (deducible)                        
   ///   @param other - the dense value to forward and emplace	               
   template<CT::CustomData T>
   Any::Any(T&& other) requires CT::Mutable<T>
      : Any {Langulus::Move(other)} {}

   /// Construct by inserting a disowned value of non-block type              
   ///   @tparam T - the data type to push (deducible)                        
   ///   @param other - the disowned value                                    
   template<CT::Semantic S>
   Any::Any(S&& other) requires (CT::CustomData<TypeOf<S>>) {
      using T = TypeOf<S>;
      if constexpr (CT::Sparse<T>)
         MakeSparse();
      SetType<T, false>();
      AllocateFresh(RequestSize(1));
      InsertInner(other.Forward(), 0);
   }

   /// Pack any number of elements sequentially                               
   /// If any of the types doesn't match exactly, the container becomes deep  
   /// to incorporate all elements                                            
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   template<CT::Data HEAD, CT::Data... TAIL>
   Any::Any(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1) {
      if constexpr (CT::Exact<HEAD, TAIL...>) {
         if constexpr (CT::Sparse<HEAD>)
            MakeSparse();

         SetType<Decay<HEAD>, false>();
         AllocateFresh(RequestSize(sizeof...(TAIL) + 1));

         InsertInner(Forward<HEAD>(head), 0);
         InsertStatic<1>(Forward<TAIL>(tail)...);
      }
      else {
         SetType<Any, false>();
         AllocateFresh(RequestSize(sizeof...(TAIL) + 1));

         InsertInner(Abandon(Any {Forward<HEAD>(head)}), 0);
         InsertStatic<1>(Abandon(Any {Forward<TAIL>(tail)})...);
      }
   }

   /// Destruction                                                            
   inline Any::~Any() {
      Free();
   }

   /// Create an empty Any from a dynamic type and state                      
   ///   @param type - type of the container                                  
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   inline Any Any::FromMeta(DMeta type, const DataState& state) noexcept {
      return Any {Block {state, type}};
   }

   /// Create an empty Any by copying type and state of a block               
   ///   @param block - the source of type and state                          
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   inline Any Any::FromBlock(const Block& block, const DataState& state) noexcept {
      return Any::FromMeta(block.GetType(), block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any by copying only state of a block                   
   ///   @param block - the source of the state                               
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   inline Any Any::FromState(const Block& block, const DataState& state) noexcept {
      return Any::FromMeta(nullptr, block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any from a static type and state                       
   ///   @tparam T - the contained type                                       
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   template<CT::Data T>
   Any Any::From(const DataState& state) noexcept {
      const auto meta = MetaData::Of<Decay<T>>();
      if constexpr (CT::Sparse<T>)
         return Block {state + DataState::Sparse, meta};
      else
         return Block {state, meta};
   }

   /// Pack any number of elements sequentially                               
   /// If any of the elements doesn't match the rest, the container becomes   
   /// deep to incorporate all elements                                       
   ///   @tparam LIST... - the list of element types (deducible)              
   ///   @param elements - sequential elements                                
   ///   @returns the pack containing the data                                
   template<CT::Data... LIST>
   Any Any::Wrap(LIST&&... elements) {
      if constexpr (sizeof...(LIST) == 0)
         return {};
      else
         return {Forward<LIST>(elements)...};
   }

   /// Pack any number of similarly typed elements sequentially               
   ///   @tparam AS - the type to wrap elements as                            
   ///                use 'void' to deduce AS from the HEAD                   
   ///                (void by default)                                       
   ///   @tparam HEAD - the first element type (deducible)                    
   ///   @tparam TAIL... - the rest of the element types (deducible)          
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   ///   @returns the new container containing the data                       
   template<class AS, CT::Data HEAD, CT::Data... TAIL>
   Any Any::WrapAs(HEAD&& head, TAIL&&... tail) {
      if constexpr (sizeof...(TAIL) == 0)
         return {};
      else if constexpr (CT::Void<AS>) {
         static_assert(CT::Exact<HEAD, TAIL...>, "Type mismatch");
         return {Forward<HEAD>(head), Forward<HEAD>(tail)...};
      }
      else {
         static_assert(CT::DerivedFrom<HEAD, AS>, "Head not related");
         static_assert((CT::DerivedFrom<TAIL, AS> && ...), "Tail not related");
         return {Forward<AS>(head), Forward<AS>(tail)...};
      }
   }
   
   /// Shallow-copy assignment                                                
   ///   @param other - the container to copy                                 
   ///   @return a reference to this container                                
   inline Any& Any::operator = (const Any& other) {
      return Any::template operator = <Any>(other);
   }

   /// Move assignment                                                        
   ///   @param other - the container to move and reset                       
   ///   @return a reference to this container                                
   inline Any& Any::operator = (Any&& other) noexcept {
      return Any::template operator = <Any>(Forward<Any>(other));
   }

   /// Shallow-copy assignment of anything deep                               
   ///   @param other - the container to copy                                 
   ///   @return a reference to this container                                
   template<CT::Deep T>
   Any& Any::operator = (const T& other) {
      return operator = (Langulus::Copy(other));
   }
   
   template<CT::Deep T>
   Any& Any::operator = (T& other) {
      return operator = (Langulus::Copy(other));
   }

   /// Move assignment of anything deep                                       
   ///   @param other - the container to move and reset                       
   ///   @return a reference to this container                                
   template<CT::Deep T>
   Any& Any::operator = (T&& other) requires CT::Mutable<T> {
      return operator = (Langulus::Move(other));
   }

   /// Assign by shallow-copying anything non-deep                            
   ///   @tparam T - type to copy (deducible)                                 
   ///   @param other - the item to copy                                      
   ///   @return a reference to this container                                
   template<CT::CustomData T>
   Any& Any::operator = (const T& other) {
      return operator = (Langulus::Copy(other));
   }

   template<CT::CustomData T>
   Any& Any::operator = (T& other) {
      return operator = (Langulus::Copy(other));
   }

   /// Assign by moving anything non-deep                                     
   ///   @param other - the item to move                                      
   ///   @return a reference to this container                                
   template<CT::CustomData T>
   Any& Any::operator = (T&& other) requires CT::Mutable<T> {
      return operator = (Langulus::Move(other));
   }

   /// Semantic assignment                                                    
   ///   @param other - the container to semantically assign                  
   ///   @return a reference to this container                                
   template<CT::Semantic S>
   Any& Any::operator = (S&& other) {
      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         if (this == &other.mValue)
            return *this;

         // Since Any is type-erased, we make a runtime type check      
         LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(other.mValue.mType),
            Assign, "Incompatible types");

         Free();
         Block::operator = (other.mValue);

         if constexpr (S::Move) {
            if constexpr (S::Keep) {
               if constexpr (CT::Exact<T, Block>) {
                  // Since we are not aware if that block is referenced 
                  // or not we reference it just in case, and we also   
                  // do not reset 'other' to avoid leaks. When using    
                  // raw Blocks, it's your responsibility to take care  
                  // of ownership.                                      
                  Keep();
               }
               else {
                  other.mValue.ResetMemory();
                  other.mValue.ResetState();
               }
            }
         }
         else {
            if constexpr (S::Keep)
               Keep();
            /*mRaw = other.mValue.mRaw;
            mCount = other.mValue.mCount;
            mReserved = other.mValue.mReserved;
            mState = other.mValue.mState;
            mType = other.mValue.mType;*/
            // No need to copy entry - it's been reset by Free(), and   
            // this is a disowned copy                                  
         }
      }
      else if constexpr (CT::CustomData<T>) {
         const auto meta = MetaData::Of<Decay<T>>();

         LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(meta),
            Assign, "Incompatible types");

         if (GetUses() != 1 || IsSparse() != CT::Sparse<T> || !meta->Is(mType)) {
            // Reset and allocate new memory                            
            // Disowned-construction will be used if possible           
            /*Reset();
            operator << (other.Forward());*/

            // Just destroy and reuse memory                               
            // Even better - types match, so we know this container        
            // is filled with T too, therefore we can use statically       
            // optimized routines for destruction                          
            CallKnownDestructors<T>();
            mCount = 0;
         }
         else {
            Reset();
            mType = meta;
            if constexpr (CT::Sparse<T>)
               MakeSparse();
            else
               MakeDense();

            AllocateFresh(RequestSize(1));
         }

         InsertInner(other.Forward(), 0);
      }
      else LANGULUS_ERROR("Unsupported data on assignment");

      return *this;
   }

   /// Clone container                                                        
   ///   @return the cloned container                                         
   inline Any Any::Clone() const {
      Any clone;
      Block::Clone(clone);
      return Abandon(clone);
   }

   /// Destroy all elements, but retain allocated memory if possible          
   inline void Any::Clear() {
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
   template<CT::NotSemantic T>
   Any& Any::operator << (const T& other) {
      Insert<IndexBack>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   Any& Any::operator << (T& other) {
      Insert<IndexBack>(Langulus::Copy(other));
      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   Any& Any::operator << (T&& other) {
      Insert<IndexBack>(Langulus::Move(other));
      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   Any& Any::operator << (S&& other) {
      Insert<IndexBack>(other.Forward());
      return *this;
   }

   /// Copy-insert an element (including arrays) at the front                 
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   Any& Any::operator >> (const T& other) {
      Insert<IndexFront>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   Any& Any::operator >> (T& other) {
      Insert<IndexFront>(Langulus::Copy(other));
      return *this;
   }

   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   Any& Any::operator >> (T&& other) {
      Insert<IndexFront>(Langulus::Move(other));
      return *this;
   }
   
   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   Any& Any::operator >> (S&& other) {
      Insert<IndexFront>(other.Forward());
      return *this;
   }

   /// Merge data (including arrays) at the back                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   Any& Any::operator <<= (const T& other) {
      Merge<IndexBack, true>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   Any& Any::operator <<= (T& other) {
      Merge<IndexBack, true>(Langulus::Copy(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   Any& Any::operator <<= (T&& other) {
      Merge<IndexBack, true>(Langulus::Move(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   Any& Any::operator <<= (S&& other) {
      Merge<IndexBack, true>(other.Forward());
      return *this;
   }

   /// Merge data at the front                                                
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   Any& Any::operator >>= (const T& other) {
      Merge<IndexFront, true>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   Any& Any::operator >>= (T& other) {
      Merge<IndexFront, true>(Langulus::Copy(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   Any& Any::operator >>= (T&& other) {
      Merge<IndexFront, true>(Langulus::Move(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   Any& Any::operator >>= (S&& other) {
      Merge<IndexFront, true>(other.Forward());
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
   ///   @tparam IDX - type of indexing to use (deducible)                    
   ///   @tparam A... - argument types (deducible)                            
   ///   @param idx - the index to emplace at                                 
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return 1 if the element was emplace successfully                    
   template<CT::Index IDX, class... A>
   Count Any::EmplaceAt(const IDX& idx, A&&... arguments) {
      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + 1);

      const auto index = Block::SimplifyIndex<void, false>(idx);
      if (index < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We need to shift elements right from the insertion point    
         // Therefore, we call move constructors in reverse, to avoid   
         // memory overlap                                              
         const auto moved = mCount - index;
         CropInner(index + 1, 0, moved)
            .template CallUnknownMoveConstructors<false, true>(
               moved, CropInner(index, moved, moved)
            );
      }

      // Pick the region that should be overwritten with new stuff      
      auto region = CropInner(index, 0, 1);
      if constexpr (sizeof...(A) == 0) {
         // Attempt default construction                                
         //TODO if stuff moved, we should move stuff back if this throws...
         region.CallUnknownDefaultConstructors(1);
      }
      else {
         // Attempt move-construction, if available                     
         if constexpr (sizeof...(A) == 1) {
            if (Is<A...>() && IsSparse() == CT::Sparse<A...>) {
               // Single argument matches                               
               if constexpr (CT::Sparse<A...>) {
                  // Can't directly interface pointer of pointers, we   
                  // have to wrap it first                              
                  Any wrapped;
                  (wrapped << ... << Forward<A>(arguments));
                  //TODO if stuff moved, we should move stuff back if this throws...
                  region.template CallKnownMoveConstructors<A...>(
                     1, wrapped
                  );
               }
               else {
                  //TODO if stuff moved, we should move stuff back if this throws...
                  region.template CallKnownMoveConstructors<Decay<A>...>(
                     1, Block::From(SparseCast(arguments)...)
                  );
               }

               ++mCount;
               return 1;
            }
         }

         // Attempt descriptor-construction, if available               
         //TODO if stuff moved, we should move stuff back if this throws...
         const auto descriptor = Any::Wrap(Forward<A>(arguments)...);
         region.CallUnknownDescriptorConstructors(1, descriptor);
      }

      ++mCount;
      return 1;
   }

   /// Construct an item of this container's type at front/back               
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
   ///   @tparam INDEX - the index to emplace at, IndexFront or IndexBack     
   ///   @tparam A... - argument types (deducible)                            
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return 1 if the element was emplace successfully                    
   template<Index INDEX, class... A>
   Count Any::Emplace(A&&... arguments) {
      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + 1);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We need to shift elements right from the insertion point    
         // Therefore, we call move constructors in reverse, to avoid   
         // potential memory overlap                                    
         CropInner(1, 0, mCount)
            .template CallUnknownSemanticConstructors<true>(
               mCount, Abandon(CropInner(0, mCount, mCount))
            );
      }

      // Pick the region that should be overwritten with new stuff      
      auto region = CropInner(INDEX == IndexFront ? 0 : mCount, 0, 1);

      if constexpr (sizeof...(A) == 0) {
         // Attempt default construction                                
         //TODO if stuff moved, we should move stuff back if this throws...
         region.CallUnknownDefaultConstructors(1);
      }
      else {
         // Attempt move-construction, if available                     
         if constexpr (sizeof...(A) == 1) {
            if (Is<A...>() && IsSparse() == CT::Sparse<A...>) {
               // Single argument matches                               
               if constexpr (CT::Sparse<A...>) {
                  // Can't directly interface pointer of pointers, we   
                  // have to wrap it first                              
                  Any wrapped {Forward<A>(arguments)};

                  //TODO if stuff moved, we should move stuff back if this throws...
                  region.template CallKnownSemanticConstructors<A...>(
                     1, Abandon(wrapped)
                  );
               }
               else {
                  //TODO if stuff moved, we should move stuff back if this throws...
                  region.template CallKnownSemanticConstructors<Decay<A>...>(
                     1, Abandon(Block::From(SparseCast(arguments)...))
                  );
               }

               ++mCount;
               return 1;
            }
         }

         // Attempt descriptor-construction, if available               
         //TODO if stuff moved, we should move stuff back if this throws...
         const Any descriptor {Forward<A>(arguments)...};
         region.CallUnknownDescriptorConstructors(1, descriptor);
      }

      ++mCount;
      return 1;
   }

   /// Reset the container                                                    
   inline void Any::Reset() {
      Free();
      mRaw = nullptr;
      mCount = mReserved = 0;
      ResetState();
   }

   /// Reset container state                                                  
   constexpr void Any::ResetState() noexcept {
      mState = mState.mState & (DataState::Typed | DataState::Sparse);
      ResetType();
   }

   /// Swap two container's contents                                          
   ///   @param other - [in/out] the container to swap contents with          
   inline void Any::Swap(Any& other) noexcept {
      other = ::std::exchange(*this, ::std::move(other));
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   inline Any Any::Crop(const Offset& start, const Count& count) const {
      return Any {Block::Crop(start, count)};
   }

   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   inline Any Any::Crop(const Offset& start, const Count& count) {
      return Any {Block::Crop(start, count)};
   }




   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// An inner concatenation routine using move/abandon                      
   ///   @tparam WRAPPER - the type of the concatenated container             
   ///   @tparam T - block type to concatenate with (deducible)               
   ///   @param rhs - block to concatenate                                    
   ///   @return the concatenated container                                   
   template<CT::Block WRAPPER, CT::Semantic S>
   WRAPPER Any::Concatenate(S&& rhs) const {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");

      if (IsEmpty())
         return {rhs.Forward()};
      else if (rhs.mValue.IsEmpty())
         return reinterpret_cast<const WRAPPER&>(*this);

      WRAPPER result;
      if constexpr (!CT::Typed<WRAPPER>)
         result.template SetType<false>(mType);

      result.AllocateFresh(result.RequestSize(mCount + rhs.mValue.mCount));
      result.InsertBlock(reinterpret_cast<const WRAPPER&>(*this));
      result.InsertBlock(rhs.Forward());
      return Abandon(result);
   }

   /// Copy-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   Any Any::operator + (const T& rhs) const requires CT::Dense<T> {
      return Concatenate<Any>(Langulus::Copy(rhs));
   }

   template<CT::Deep T>
   Any Any::operator + (T& rhs) const requires CT::Dense<T> {
      return Concatenate<Any>(Langulus::Copy(rhs));
   }

   /// Move-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   Any Any::operator + (T&& rhs) const requires CT::Dense<T> {
      return Concatenate<Any>(Langulus::Move(rhs));
   }

   /// Move-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Semantic S>
   Any Any::operator + (S&& rhs) const requires (CT::Deep<typename S::Type>&& CT::Dense<typename S::Type>) {
      return Concatenate<Any>(rhs.Forward());
   }

   /// Destructive copy-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   Any& Any::operator += (const T& rhs) requires CT::Dense<T> {
      InsertBlock(Langulus::Copy(rhs));
      return *this;
   }

   template<CT::Deep T>
   Any& Any::operator += (T& rhs) requires CT::Dense<T> {
      InsertBlock(Langulus::Copy(rhs));
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   Any& Any::operator += (T&& rhs) requires CT::Dense<T> {
      InsertBlock(Langulus::Move(rhs));
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Semantic S>
   Any& Any::operator += (S&& rhs) requires (CT::Deep<typename S::Type>&& CT::Dense<typename S::Type>) {
      InsertBlock(rhs.Forward());
      return *this;
   }
   
   /// Find element(s) index inside container                                 
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @tparam BY_ADDRESS_ONLY - true to compare addresses only             
   ///   @param item - the item to search for                                 
   ///   @return the index of the found item, or IndexNone if none found      
   template<bool REVERSE, bool BY_ADDRESS_ONLY, CT::Data T>
   Index Any::Find(const T& item, const Offset& cookie) const {
      return Block::template FindKnown<REVERSE, BY_ADDRESS_ONLY>(item, cookie);
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   inline typename Any::Iterator Any::begin() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const Any*>(this)->begin();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   inline typename Any::Iterator Any::end() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const Any*>(this)->end();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   inline typename Any::Iterator Any::last() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const Any*>(this)->last();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   inline typename Any::ConstIterator Any::begin() const noexcept {
      return IsEmpty() ? end() : GetElement();
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   inline typename Any::ConstIterator Any::end() const noexcept {
      return Block {mState, mType, 0, GetRawEnd(), nullptr};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   inline typename Any::ConstIterator Any::last() const noexcept {
      if (IsEmpty())
         return end();
      return Block {mState, mType, 1, GetRawEnd() - GetStride(), mEntry};
   }


   ///                                                                        
   ///   Block iterator                                                       
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param value - pointer to the value element                          
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   Any::TIterator<MUTABLE>::TIterator(const Block& value) noexcept
      : mValue {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename Any::TIterator<MUTABLE>& Any::TIterator<MUTABLE>::operator ++ () noexcept {
      mValue.mRaw += mValue.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename Any::TIterator<MUTABLE> Any::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare block entries                                                  
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   bool Any::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mValue.mRaw == rhs.mValue.mRaw;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const Block& Any::TIterator<MUTABLE>::operator * () const noexcept {
      return mValue;
   }

} // namespace Langulus::Anyness
