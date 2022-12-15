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
      : Block {static_cast<const Block&>(other)} {
      Keep();
   }

   /// Construct by moving another container                                  
   ///   @param other - the container to move                                 
   inline Any::Any(Any&& other) noexcept
      : Block {static_cast<const Block&>(other)} {
      other.ResetMemory();
      other.ResetState();
   }

   /// Copy construct - does a shallow copy, and references                   
   ///   @param other - the container to shallow-copy                         
   template<CT::Deep T>
   Any::Any(const T& other)
      : Block {static_cast<const Block&>(other)} {
      Keep();
   }

   template<CT::Deep T>
   Any::Any(T& other)
      : Block {static_cast<const Block&>(other)} {
      Keep();
   }

   /// Construct by moving another container                                  
   ///   @param other - the container to move                                 
   template<CT::Deep T>
   Any::Any(T&& other) requires CT::Mutable<T>
      : Block {static_cast<const Block&>(other)} {
      if constexpr (CT::Same<T, Block>) {
         // Since we are not aware if that block is referenced or not   
         // we reference it just in case, and we also do not reset      
         // 'other' to avoid leaks. When using raw Blocks, it's your    
         // responsibility to take care of ownership.                   
         Keep();
      }
      else {
         other.ResetMemory();
         other.ResetState();
      }
   }

   /// Same as copy-construction, but doesn't reference anything              
   ///   @param other - the block to copy                                     
   template<CT::Deep T>
   constexpr Any::Any(Disowned<T>&& other) noexcept
      : Block {other.template Forward<Block>()} {
      mEntry = nullptr;
   }
   
   /// Same as move-construction but doesn't fully reset other, saving some   
   /// instructions                                                           
   ///   @param other - the block to move                                     
   template<CT::Deep T>
   constexpr Any::Any(Abandoned<T>&& other) noexcept
      : Block {other.template Forward<Block>()} {
      other.mValue.mEntry = nullptr;
   }

   /// Construct by copying/referencing value of non-block type               
   ///   @tparam T - the data type to push (deducible)                        
   ///   @param other - the dense value to shallow-copy                       
   template<CT::CustomData T>
   Any::Any(const T& other) {
      if constexpr (CT::Sparse<T>)
         MakeSparse();
      SetType<T, false>();
      Insert<IndexBack, true, false>(&other, &other + 1);
   }

   template<CT::CustomData T>
   Any::Any(T& other)
      : Any {const_cast<const T&>(other)} { }

   /// Construct by moving a dense value of non-block type                    
   ///   @tparam T - the data type to push (deducible)                        
   ///   @param other - the dense value to forward and emplace	               
   template<CT::CustomData T>
   Any::Any(T&& other) requires CT::Mutable<T> {
      if constexpr (CT::Sparse<T>)
         MakeSparse();
      SetType<T, false>();
      Insert<IndexBack, true, false>(Move(other));
   }

   /// Construct by inserting a disowned value of non-block type              
   ///   @tparam T - the data type to push (deducible)                        
   ///   @param other - the disowned value                                    
   template<CT::CustomData T>
   Any::Any(Disowned<T>&& other) {
      if constexpr (CT::Sparse<T>)
         MakeSparse();
      SetType<T, false>();
      Insert<IndexBack, false, false>(&other.mValue, &other.mValue + 1);
   }

   /// Construct by inserting an abandoned value of non-block type            
   ///   @tparam T - the data type to push (deducible)                        
   ///   @param other - the abandoned value                                   
   template<CT::CustomData T>
   Any::Any(Abandoned<T>&& other) {
      if constexpr (CT::Sparse<T>)
         MakeSparse();
      SetType<T, false>();
      Insert<IndexBack, false, false>(Move(other.mValue));
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
         AllocateInner<false>(sizeof...(TAIL) + 1);

         Insert<IndexBack, true, false>(Forward<HEAD>(head));
         (Insert<IndexBack, true, false>(Forward<TAIL>(tail)) | ...);
      }
      else {
         SetType<Any, false>();
         AllocateInner<false>(sizeof...(TAIL) + 1);

         Insert<IndexBack, false, false>(Any {Forward<HEAD>(head)});
         (Insert<IndexBack, false, false>(Any {Forward<TAIL>(tail)}) | ...);
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
      if constexpr (CT::Sparse<T>)
         return Any {Block{state + DataState::Sparse, MetaData::Of<Decay<T>>()}};
      else
         return Any {Block{state, MetaData::Of<Decay<T>>()}};
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
      if (this == &other)
         return *this;

      // Since Any is type-erased, we have to make a runtime type check 
      LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(other.mType),
         Except::Copy, "Incompatible types");

      Free();
      Block::operator = (other);
      Keep();
      return *this;
   }
   
   template<CT::Deep T>
   Any& Any::operator = (T& other) {
      return operator = (const_cast<const T&>(other));
   }

   /// Move assignment of anything deep                                       
   ///   @param other - the container to move and reset                       
   ///   @return a reference to this container                                
   template<CT::Deep T>
   Any& Any::operator = (T&& other) requires CT::Mutable<T>{
      if (this == &other)
         return *this;

      // Since Any is type-erased, we have to make a runtime type check 
      LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(other.mType),
         Except::Move, "Incompatible types");

      Free();
      Block::operator = (Forward<Block>(other));
      if constexpr (CT::Same<T, Block>) {
         // Since we are not aware if that block is referenced or not   
         // we reference it just in case, and we also do not reset      
         // 'other' to avoid leaks. When using raw Blocks, it's your    
         // responsibility to take care of ownership.                   
         Keep();
      }
      else {
         other.ResetMemory();
         other.ResetState();
      }
      return *this;
   }

   /// Shallow-copy a disowned container (doesn't reference anything)         
   ///   @param other - the container to copy                                 
   ///   @return a reference to this container                                
   template<CT::Deep T>
   Any& Any::operator = (Disowned<T>&& other) {
      if (this == &other.mValue)
         return *this;

      // Since Any is type-erased, we have to make a runtime type check 
      LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(other.mValue.mType),
         Except::Copy, "Incompatible types");

      Free();
      mRaw = other.mValue.mRaw;
      mCount = other.mValue.mCount;
      mReserved = other.mValue.mReserved;
      mState = other.mValue.mState;
      mType = other.mValue.mType;
      // No need to copy entry - it's been reset by Free(), and this is 
      // a disowned copy                                                
      return *this;
   }
   
   /// Move an abandoned container, minimally resetting the source            
   ///   @param other - the container to move and reset                       
   ///   @return a reference to this container                                
   template<CT::Deep T>
   Any& Any::operator = (Abandoned<T>&& other) {
      if (this == &other.mValue)
         return *this;

      // Since Any is type-erased, we have to make a runtime type check 
      LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(other.mValue.mType),
         Except::Move, "Incompatible types");

      Free();
      Block::operator = (Forward<Block>(other.mValue));
      other.mValue.mEntry = nullptr;
      return *this;
   }

   /// Helper function for preparing reassignment                             
   template<class T>
   void Any::PrepareForReassignment() {
      const auto meta = MetaData::Of<Decay<T>>();

      // Since Any is type-erased, we have to make a runtime type check 
      LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(meta),
         Except::Copy, "Incompatible types");

      if (GetUses() == 1 && meta->Is(mType)) {
         // Just destroy and reuse memory                               
         // Even better - types match, so we know this container        
         // is filled with T too, therefore we can use statically       
         // optimized routines for destruction                          
         CallKnownDestructors<T>();
         mCount = 0;
      }
      else {
         // Reset and allocate new memory                               
         Reset();
         mType = meta;
         if constexpr (CT::Sparse<T>)
            MakeSparse();
         else
            MakeDense();
         AllocateInner<false>(1);
      }
   }

   /// Assign by shallow-copying anything non-deep                            
   ///   @tparam T - type to copy (deducible)                                 
   ///   @param other - the item to copy                                      
   ///   @return a reference to this container                                
   template<CT::CustomData T>
   Any& Any::operator = (const T& other) {
      PrepareForReassignment<T>();
      InsertInner<true>(&other, &other + 1, 0);
      return *this;
   }

   template<CT::CustomData T>
   Any& Any::operator = (T& other) {
      return operator = (const_cast<const T&>(other));
   }

   /// Assign by moving anything non-deep                                     
   ///   @param other - the item to move                                      
   ///   @return a reference to this container                                
   template<CT::CustomData T>
   Any& Any::operator = (T&& other) requires CT::Mutable<T> {
      PrepareForReassignment<T>();
      InsertInner<true>(Forward<T>(other), 0);
      return *this;
   }

   /// Assign by disowning anything non-block                                 
   ///   @param other - the disowned element to push                          
   ///   @return a reference to this container                                
   template<CT::CustomData T>
   Any& Any::operator = (Disowned<T>&& other) {
      // Since Any is type-erased, we have to make a runtime type check 
      const auto meta = MetaData::Of<Decay<T>>();
      LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(meta),
         Except::Copy, "Incompatible types");

      if (GetUses() != 1 || IsSparse() != CT::Sparse<T> || !meta->Is(mType)) {
         // Reset and allocate new memory                               
         // Disowned-construction will be used if possible              
         Reset();
         operator << (other.Forward());
      }
      else {
         // Just destroy and reuse memory                               
         // Even better - types match, so we know this container        
         // is filled with T too, therefore we can use statically       
         // optimized routines for destruction and creation             
         if constexpr (CT::Sparse<T>) {
            CallKnownDestructors<T>();
            mCount = 1;
            new (mRawSparse) KnownPointer {
               reinterpret_cast<Byte*>(other.mValue), nullptr};
         }
         else {
            CallKnownDestructors<T>();
            mCount = 1;
            if constexpr (CT::DisownMakable<T>)
               new (mRaw) T {other.Forward()};
            else if constexpr (CT::CopyMakable<T>)
               new (mRaw) T {other.mValue};
            else
               LANGULUS_ERROR("T is not disown/copy-makable");
         }
      }

      return *this;
   }

   /// Assign by abandoning anything non-block                                
   ///   @param other - the abandoned element to push                         
   ///   @return a reference to this container                                
   template<CT::CustomData T>
   Any& Any::operator = (Abandoned<T>&& other) {
      // Since Any is type-erased, we have to make a runtime type check 
      const auto meta = MetaData::Of<Decay<T>>();
      LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(meta),
         Except::Copy, "Incompatible types");

      if (GetUses() != 1 || IsSparse() != CT::Sparse<T> || !meta->Is(mType)) {
         // Reset and allocate new memory                               
         // Abandoned-construction will be used if possible             
         Reset();
         operator << (other.Forward());
      }
      else {
         // Just destroy and reuse memory                               
         // Even better - types match, so we know this container        
         // is filled with T too, therefore we can use statically       
         // optimized routines for destruction and creation             
         if constexpr (CT::Sparse<T>) {
            CallKnownDestructors<T>();
            mCount = 1;
            new (mRawSparse) KnownPointer {
               reinterpret_cast<Byte*>(other.mValue), nullptr};
         }
         else {
            CallKnownDestructors<T>();
            mCount = 1;
            if constexpr (CT::AbandonMakable<T>)
               new (mRaw) T {other.Forward()};
            else if constexpr (CT::MoveMakable<T>)
               new (mRaw) T {Forward<T>(other.mValue)};
            else
               LANGULUS_ERROR("T is not abandon/move-makable");
         }
      }

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
   template<CT::Data T>
   Any& Any::operator << (const T& other) {
      Insert<IndexBack, true, true>(&other, &other + ExtentOf<T>);
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::Data T>
   Any& Any::operator << (T& other) {
      return operator << (const_cast<const T&>(other));
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Data T>
   Any& Any::operator << (T&& other) {
      if constexpr (CT::Abandoned<T>)
         Insert<IndexBack, false, true>(Move(other.mValue));
      else if constexpr (CT::Disowned<T>)
         Insert<IndexBack, false, true>(&other.mValue, &other.mValue + 1);
      else
         Insert<IndexBack, true, true>(Forward<T>(other));
      return *this;
   }

   /// Copy-insert an element (including arrays) at the front                 
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Data T>
   Any& Any::operator >> (const T& other) {
      Insert<IndexFront, true, true>(&other, &other + ExtentOf<T>);
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::Data T>
   Any& Any::operator >> (T& other) {
      return operator >> (const_cast<const T&>(other));
   }

   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Data T>
   Any& Any::operator >> (T&& other) {
      if constexpr (CT::Abandoned<T>)
         Insert<IndexFront, false, true>(Move(other.mValue));
      else if constexpr (CT::Disowned<T>)
         Insert<IndexFront, false, true>(&other.mValue, &other.mValue + 1);
      else
         Insert<IndexFront, true, true>(Forward<T>(other));
      return *this;
   }

   /// Merge data (including arrays) at the back                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Data T>
   Any& Any::operator <<= (const T& other) {
      Merge<IndexBack, true>(&other, &other + ExtentOf<T>);
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::Data T>
   Any& Any::operator <<= (T& other) {
      return operator <<= (const_cast<const T&>(other));
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Data T>
   Any& Any::operator <<= (T&& other) {
      Merge<IndexBack, true>(Forward<T>(other));
      return *this;
   }

   /// Merge data at the front                                                
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Data T>
   Any& Any::operator >>= (const T& other) {
      Merge<IndexFront, true>(&other, &other + ExtentOf<T>);
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::Data T>
   Any& Any::operator >>= (T& other) {
      return operator >>= (const_cast<const T&>(other));
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Data T>
   Any& Any::operator >>= (T&& other) {
      Merge<IndexFront, true>(Forward<T>(other));
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
      Allocate<false>(mCount + 1);

      const auto index = Block::SimplifyIndex<void, false>(idx);
      if (index < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
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
      Allocate<false>(mCount + 1);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
            "Moving elements that are used from multiple places");

         // We need to shift elements right from the insertion point    
         // Therefore, we call move constructors in reverse, to avoid   
         // memory overlap                                              
         CropInner(1, 0, mCount)
            .template CallUnknownMoveConstructors<false, true>(
               mCount, CropInner(0, mCount, mCount)
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
      other = ::std::exchange(*this, Move(other));
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

   /// An inner concatenation routine using copy/disown                       
   ///   @tparam WRAPPER - the type of the concatenated container             
   ///   @tparam KEEP - true to use copy, false to use disown                 
   ///   @tparam T - block type to concatenate with (deducible)               
   ///   @param rhs - block to concatenate                                    
   ///   @return the concatenated container                                   
   template<CT::Block WRAPPER, bool KEEP, CT::Block T>
   WRAPPER Any::Concatenate(const T& rhs) const {
      if (IsEmpty()) {
         if constexpr (KEEP)
            return WRAPPER {rhs};
         else
            return WRAPPER {Disown(rhs)};
      }
      else if (rhs.IsEmpty())
         return reinterpret_cast<const WRAPPER&>(*this);

      WRAPPER result;
      if constexpr (!CT::Typed<WRAPPER>)
         result.template SetType<false>(mType);
      result.Allocate(mCount + rhs.mCount);
      result.InsertBlock(reinterpret_cast<const WRAPPER&>(*this));
      if constexpr (KEEP)
         result.InsertBlock(rhs);
      else
         result.InsertBlock(Disown(rhs));
      return Abandon(result);
   }

   /// An inner concatenation routine using move/abandon                      
   ///   @tparam WRAPPER - the type of the concatenated container             
   ///   @tparam KEEP - true to use move, false to use abandon                
   ///   @tparam T - block type to concatenate with (deducible)               
   ///   @param rhs - block to concatenate                                    
   ///   @return the concatenated container                                   
   template<CT::Block WRAPPER, bool KEEP, CT::Block T>
   WRAPPER Any::Concatenate(T&& rhs) const {
      if (IsEmpty()) {
         if constexpr (KEEP)
            return WRAPPER {Forward<T>(rhs)};
         else
            return WRAPPER {Abandon(Forward<T>(rhs))};
      }
      else if (rhs.IsEmpty())
         return reinterpret_cast<const WRAPPER&>(*this);

      WRAPPER result;
      if constexpr (!CT::Typed<WRAPPER>)
         result.template SetType<false>(mType);
      result.Allocate(mCount + rhs.mCount);
      result.InsertBlock(reinterpret_cast<const WRAPPER&>(*this));
      if constexpr (KEEP)
         result.InsertBlock(Forward<T>(rhs));
      else
         result.InsertBlock(Abandon(Forward<T>(rhs)));
      return Abandon(result);
   }

   /// Copy-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   Any Any::operator + (const T& rhs) const requires CT::Dense<T> {
      return Concatenate<Any, true>(rhs);
   }

   template<CT::Deep T>
   Any Any::operator + (T& rhs) const requires CT::Dense<T> {
      return operator + (const_cast<const T&>(rhs));
   }

   /// Move-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   Any Any::operator + (T&& rhs) const requires CT::Dense<T> {
      return Concatenate<Any, true>(Forward<T>(rhs));
   }

   /// Disown-concatenate with any deep type                                  
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   Any Any::operator + (Disowned<T>&& rhs) const requires CT::Dense<T> {
      return Concatenate<Any, false>(rhs.mValue);
   }

   /// Abandon-concatenate with any deep type                                 
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   Any Any::operator + (Abandoned<T>&& rhs) const requires CT::Dense<T> {
      return Concatenate<Any, false>(Forward<T>(rhs.mValue));
   }

   /// Destructive copy-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   Any& Any::operator += (const T& rhs) requires CT::Dense<T> {
      InsertBlock(rhs);
      return *this;
   }

   template<CT::Deep T>
   Any& Any::operator += (T& rhs) requires CT::Dense<T> {
      return operator += (const_cast<const T&>(rhs));
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   Any& Any::operator += (T&& rhs) requires CT::Dense<T> {
      InsertBlock(Forward<T>(rhs));
      return *this;
   }

   /// Destructive disown-concatenate with any deep type                      
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   Any& Any::operator += (Disowned<T>&& rhs) requires CT::Dense<T> {
      InsertBlock(rhs.Forward());
      return *this;
   }

   /// Destructive abandon-concatenate with any deep type                     
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   Any& Any::operator += (Abandoned<T>&& rhs) requires CT::Dense<T> {
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
