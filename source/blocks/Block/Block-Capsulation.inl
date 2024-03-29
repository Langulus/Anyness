///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block-Iteration.inl"
#include "../../inner/DataState.inl"


namespace Langulus::Anyness
{
   
   /// Overwrite the current data state                                       
   ///   @attention you can not add/remove constraints like that              
   ///   @param state - the state to overwrite with                           
   LANGULUS(INLINED)
   constexpr void Block::SetState(DataState state) noexcept {
      mState = state - DataState::Constrained;
   }

   /// Add a state                                                            
   ///   @attention you can not add constraints like that                     
   ///   @param state - the state to add to the current                       
   LANGULUS(INLINED)
   constexpr void Block::AddState(DataState state) noexcept {
      mState += state - DataState::Constrained;
   }

   /// Remove a state                                                         
   ///   @attention you can not remove constraints like that                  
   ///   @param state - the state to remove from the current                  
   LANGULUS(INLINED)
   constexpr void Block::RemoveState(DataState state) noexcept {
      mState -= state - DataState::Constrained;
   }

   /// Explicit bool cast operator, for use in if statements                  
   ///   @return true if block contains at least one valid element            
   LANGULUS(INLINED)
   constexpr Block::operator bool() const noexcept {
      return not IsEmpty();
   }

   /// Check if a pointer is anywhere inside the block's reserved memory      
   ///   @attention doesn't check deep or sparse data regions                 
   ///   @param ptr - the pointer to check                                    
   ///   @return true if inside the immediate reserved memory block range     
   LANGULUS(INLINED)
   bool Block::Owns(const void* ptr) const noexcept {
      return ptr >= mRaw and ptr < mRaw + GetReservedSize();
   }

   /// Check if we have jurisdiction over the contained memory                
   ///   @return true if memory is under our authority                        
   constexpr bool Block::HasAuthority() const noexcept {
      return mEntry != nullptr;
   }

   /// Get the number of references for the allocated memory block            
   ///   @return the references for the memory block, or 0 if memory is       
   ///           outside authority (or unallocated)                           
   LANGULUS(INLINED)
   constexpr Count Block::GetUses() const noexcept {
      return mEntry ? mEntry->GetUses() : 0;
   }
   
   /// Get the contained type                                                 
   ///   @return the meta data                                                
   LANGULUS(INLINED)
   constexpr DMeta Block::GetType() const noexcept {
      return mType;
   }

   /// Get the number of initialized elements                                 
   ///   @return the number of initialized elements                           
   LANGULUS(INLINED)
   constexpr Count Block::GetCount() const noexcept {
      return mCount;
   }

   /// Get the number of reserved (maybe uninitialized) elements              
   ///   @return the number of reserved (maybe uninitialized) elements        
   LANGULUS(INLINED)
   constexpr Count Block::GetReserved() const noexcept {
      return mReserved;
   }
   
   /// Get the number of reserved bytes                                       
   ///   @attention this doesn't include bytes reserved for entries in sparse 
   ///              containers, when managed memory is enabled                
   ///   @return the number of reserved bytes                                 
   LANGULUS(INLINED)
   constexpr Size Block::GetReservedSize() const noexcept {
      return mType ? mReserved * mType->mSize : 0;
   }
   
   /// Get the number of sub-blocks (this one included)                       
   ///   @return the number of contained blocks, including this one           
   inline Count Block::GetCountDeep() const noexcept {
      if (IsEmpty() or not IsDeep())
         return 1;

      Count counter = 1;
      const_cast<Block*>(this)->IterateInner<void, const Block&>(
         mCount, [&counter](const Block& block) noexcept {
            counter += block.GetCountDeep();
         }
      );
      return counter;
   }

   /// Get the sum of initialized non-deep elements in all sub-blocks         
   ///   @return the number of contained non-deep elements                    
   inline Count Block::GetCountElementsDeep() const noexcept {
      if (IsEmpty() or not mType)
         return 0;

      if (not IsDeep())
         return mCount;

      Count counter = 0;
      const_cast<Block*>(this)->IterateInner<void, const Block&>(
         mCount, [&counter](const Block& block) noexcept {
            counter += block.GetCountElementsDeep();
         }
      );
      return counter;
   }
   
   /// Check if memory is allocated                                           
   ///   @return true if the block contains any reserved memory               
   LANGULUS(INLINED)
   constexpr bool Block::IsAllocated() const noexcept {
      return mRaw != nullptr;
   }

   /// Check if block is marked as past                                       
   ///   @return true if this container is marked as past                     
   LANGULUS(INLINED)
   constexpr bool Block::IsPast() const noexcept {
      return mState.IsPast();
   }

   /// Check if block is marked as future                                     
   ///   @return true if this container is marked as future                   
   LANGULUS(INLINED)
   constexpr bool Block::IsFuture() const noexcept {
      return mState.IsFuture();
   }

   /// Check if block is not phased (is neither past, nor future)             
   ///   @return true if this container is not phased                         
   LANGULUS(INLINED)
   constexpr bool Block::IsNow() const noexcept {
      return mState.IsNow();
   }

   /// Check if block is marked as missing                                    
   ///   @return true if this container is marked as missing                  
   LANGULUS(INLINED)
   constexpr bool Block::IsMissing() const noexcept {
      return mState.IsMissing();
   }

   /// Check if block has a data type                                         
   ///   @return true if data contained in this pack is specified             
   LANGULUS(INLINED)
   constexpr bool Block::IsTyped() const noexcept {
      return mType != nullptr;
   }

   /// Check if block has a data type                                         
   ///   @return true if data contained in this pack is unspecified           
   LANGULUS(INLINED)
   constexpr bool Block::IsUntyped() const noexcept {
      return not IsTyped();
   }

   /// Check if block has a data type, and is type-constrained                
   ///   @return true if type-constrained                                     
   LANGULUS(INLINED)
   constexpr bool Block::IsTypeConstrained() const noexcept {
      return mType and mState.IsTyped();
   }

   /// Check if block is encrypted                                            
   ///   @return true if the contents of this pack are encrypted              
   LANGULUS(INLINED)
   constexpr bool Block::IsEncrypted() const noexcept {
      return mState.IsEncrypted();
   }

   /// Check if block is compressed                                           
   ///   @return true if the contents of this pack are compressed             
   LANGULUS(INLINED)
   constexpr bool Block::IsCompressed() const noexcept {
      return mState.IsCompressed();
   }

   /// Check if block is constant                                             
   ///   @return true if the contents are constant                            
   LANGULUS(INLINED)
   constexpr bool Block::IsConstant() const noexcept {
      return mState.IsConstant();
   }

   /// Check if block is mutable                                              
   ///   @return true if the contents are mutable                             
   LANGULUS(INLINED)
   constexpr bool Block::IsMutable() const noexcept {
      return not IsConstant();
   }

   /// Check if block is static (size-constrained)                            
   ///   @return true if the contents are static (size-constrained)           
   LANGULUS(INLINED)
   constexpr bool Block::IsStatic() const noexcept {
      return mRaw and (mState.IsStatic() or not mEntry);
   }
   
   /// Check if contained type is abstract                                    
   ///   @return true if the type of this pack is abstract                    
   LANGULUS(INLINED)
   constexpr bool Block::IsAbstract() const noexcept {
      return mType and mType->mIsAbstract;
   }
   
   /// Check if block is inhibitory (or) container                            
   ///   @return true if this is an inhibitory container                      
   LANGULUS(INLINED)
   constexpr bool Block::IsOr() const noexcept {
      return mState.IsOr();
   }

   /// Check if block contains no initialized elements                        
   ///   @return true if this is an empty container                           
   LANGULUS(INLINED)
   constexpr bool Block::IsEmpty() const noexcept {
      return mCount == 0;
   }

   /// Check if block contains either created elements, or relevant state     
   ///   @return true if block either contains state, or has inserted stuff   
   LANGULUS(INLINED)
   constexpr bool Block::IsValid() const noexcept {
      return mCount or GetUnconstrainedState();
   }

   /// Check if block contains no elements and no relevant state              
   ///   @return true if this is an empty stateless container                 
   LANGULUS(INLINED)
   constexpr bool Block::IsInvalid() const noexcept {
      return not IsValid();
   }
   
   /// Check if block contains dense data                                     
   ///   @returns true if this container refers to dense memory               
   LANGULUS(INLINED)
   constexpr bool Block::IsDense() const noexcept {
      return not IsSparse();
   }

   /// Check if block contains pointers                                       
   ///   @return true if the block contains pointers                          
   LANGULUS(INLINED)
   constexpr bool Block::IsSparse() const noexcept {
      return mType ? mType->mIsSparse : false;
   }
   
   /// Check if block contains POD items - if so, it's safe to directly copy  
   /// raw memory from container. Note, that this doesn't only consider the   
   /// standard c++ type traits, like trivially_constructible.                
   /// Want a non-trivial type to be handled as POD in these containers?      
   /// - You can explicitly reflect any type with `LANGULUS(POD) true` member 
   ///   @return true if contained data is plain old data                     
   LANGULUS(INLINED)
   constexpr bool Block::IsPOD() const noexcept {
      return mType and mType->mIsPOD;
   }

   /// Check if block contains resolvable items, that is, items that have a   
   /// reflected GetBlock() function, that can be used to represent           
   /// themselves as their most concretely typed block                        
   ///   @return true if contained data can be resolved on element basis      
   LANGULUS(INLINED)
   constexpr bool Block::IsResolvable() const noexcept {
      return mType and mType->mIsSparse and mType->mResolver;
   }

   /// Check if block data can be safely set to zero bytes                    
   /// This is tied to `LANGULUS(NULLIFIABLE) true` reflection member         
   ///   @return true if contained data can be zeroed safely                  
   LANGULUS(INLINED)
   constexpr bool Block::IsNullifiable() const noexcept {
      return mType and mType->mIsNullifiable;
   }

   /// Check if the memory block contains memory blocks considered deep       
   ///   @return true if the memory block contains deep memory blocks         
   LANGULUS(INLINED)
   constexpr bool Block::IsDeep() const noexcept {
      return mType and mType->mIsDeep and mType->template CastsTo<Block, true>();
   }

   /// Check if the memory block contains memory blocks                       
   ///   @return true if the memory block contains memory blocks              
   LANGULUS(INLINED)
   constexpr bool Block::IsBlock() const noexcept {
      return mType and mType->template CastsTo<Block, true>();
   }
   
   /// Check phase compatibility                                              
   ///   @param other - the block to check                                    
   ///   @return true if phase is compatible                                  
   LANGULUS(INLINED)
   constexpr bool Block::CanFitPhase(const Block& other) const noexcept {
      return IsNow() or other.IsNow() or IsFuture() == other.IsFuture();
   }

   /// Check state compatibility                                              
   ///   @param other - the block to check                                    
   ///   @return true if state is compatible                                  
   LANGULUS(INLINED)
   constexpr bool Block::CanFitState(const Block& other) const noexcept {
      return IsInvalid() or (
         IsMissing() == other.IsMissing()
         and (not IsTypeConstrained() or other.IsExact(mType))
         and CanFitOrAnd(other)
         and CanFitPhase(other)
      );
   }

   /// Check state compatibility regarding orness                             
   ///   @param other - the block to check                                    
   ///   @return true if state is compatible                                  
   LANGULUS(INLINED)
   constexpr bool Block::CanFitOrAnd(const Block& other) const noexcept {
      return mCount <= 1 or other.mCount <= 1 or IsOr() == other.IsOr();
   }

   /// Get the size of the contained data, in bytes                           
   ///   @return the byte size                                                
   LANGULUS(INLINED)
   constexpr Size Block::GetBytesize() const noexcept {
      return mCount * GetStride();
   }

   /// Get the token of the contained type                                    
   ///   @return the token                                                    
   LANGULUS(INLINED)
   constexpr Token Block::GetToken() const noexcept {
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         return IsUntyped()
            ? MetaData::DefaultToken
            : mType->GetShortestUnambiguousToken();
      #else
         return IsUntyped()
            ? MetaData::DefaultToken
            : mType->mToken;
      #endif
   }
   
   /// Get the size of a single element (in bytes)                            
   ///   @attention this returns zero if block is untyped                     
   ///   @return the size of a single element in bytes                        
   LANGULUS(INLINED)
   constexpr Size Block::GetStride() const noexcept {
      return mType ? mType->mSize : 0;
   }
   
   /// Get the data state of the container                                    
   ///   @return the current block state                                      
   LANGULUS(INLINED)
   constexpr DataState Block::GetState() const noexcept {
      return mState;
   }

   /// Get the relevant state when relaying one block	to another              
   /// Relevant states exclude size and type constraints                      
   ///   @return the current unconstrained block state                        
   LANGULUS(INLINED)
   constexpr DataState Block::GetUnconstrainedState() const noexcept {
      return mState - DataState::Constrained;
   }
   
   /// Deep (slower) check if there's anything missing inside nested blocks   
   ///   @return true if any deep or flat memory block contains missing data  
   LANGULUS(INLINED)
   constexpr bool Block::IsMissingDeep() const {
      if (IsMissing())
         return true;

      bool result {};
      ForEachDeep([&result](const Block& group) noexcept {
         result = group.IsMissing();
         return not result;
      });
      return result;
   }
   
   /// Check if a memory block can be concatenated to this one                
   ///   @param other - the block to concatenate                              
   ///   @return true if able to concatenate to this one                      
   LANGULUS(INLINED)
   constexpr bool Block::IsConcatable(const Block& other) const noexcept {
      return not IsStatic()
         and not IsConstant() 
         and CanFitState(other)
         and IsExact(other.mType);
   }

   /// Check if a type can be inserted to this block                          
   ///   @param other - check if a given type is insertable to this block     
   ///   @return true if able to insert an instance of the type to this block 
   LANGULUS(INLINED)
   constexpr bool Block::IsInsertable(DMeta other) const noexcept {
      return other
         and not IsStatic()
         and not IsConstant()
         and IsDeep() == other->mIsDeep
         and CastsToMeta(other);
   }
   
   /// Check if a static type can be inserted                                 
   ///   @tparam T - the type to check                                        
   ///   @return true if able to insert an instance of the type to this block 
   template<CT::Data T>
   LANGULUS(INLINED)
   constexpr bool Block::IsInsertable() const noexcept {
      return IsInsertable(MetaData::Of<T>());
   }

   /// Get the raw data inside the container                                  
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @return a pointer to the first allocated element                     
   LANGULUS(INLINED)
   constexpr Byte* Block::GetRaw() noexcept {
      return mRaw;
   }

   /// Get the raw data inside the container (const)                          
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @return a pointer to the first allocated element                     
   LANGULUS(INLINED)
   constexpr const Byte* Block::GetRaw() const noexcept {
      return mRaw;
   }

   /// Get the end raw data pointer inside the container (const)              
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @attention the resulting pointer never points to a valid element     
   ///   @return a pointer to the last+1 element (never initialized)          
   LANGULUS(INLINED)
   constexpr const Byte* Block::GetRawEnd() const noexcept {
      return mRaw + GetBytesize();
   }

   /// Get a pointer array - useful only for sparse containers                
   ///   @return the raw data as an array of pointers                         
   LANGULUS(INLINED) IF_UNSAFE(constexpr)
   Byte** Block::GetRawSparse() IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, IsSparse(),
         "Representing dense data as sparse");
      return mRawSparse;
   }

   /// Get a constant pointer array - useful only for sparse containers       
   ///   @return the raw data as an array of constant pointers                
   LANGULUS(INLINED) IF_UNSAFE(constexpr)
   const Byte* const* Block::GetRawSparse() const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, IsSparse(),
         "Representing dense data as sparse");
      return mRawSparse;
   }
   
   /// Get the raw data inside the container, reinterpreted as some type      
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @tparam T - the type we're interpreting as                           
   ///   @return a pointer to the first element of type T                     
   template<CT::Data T>
   LANGULUS(INLINED)
   T* Block::GetRawAs() noexcept {
      return reinterpret_cast<T*>(mRaw);
   }

   /// Get the raw data inside the container, reinterpreted (const)           
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @tparam T - the type we're interpreting as                           
   ///   @return a pointer to the first element of type T                     
   template<CT::Data T>
   LANGULUS(INLINED)
   const T* Block::GetRawAs() const noexcept {
      return reinterpret_cast<const T*>(mRaw);
   }

   /// Get the end raw data pointer inside the container                      
   ///   @attention never points to a valid element                           
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @tparam T - the type we're interpreting as                           
   ///   @return a pointer to the last+1 element of type T                    
   template<CT::Data T>
   LANGULUS(INLINED)
   const T* Block::GetRawEndAs() const noexcept {
      return reinterpret_cast<const T*>(GetRawEnd());
   }
   
   /// Make memory block static (aka size-constrained)                        
   /// The state is useful to make block views, that disallow memory movement 
   /// and reallocation. Useful to interface static data, or data on stack.   
   /// Extensively used when accessing members/bases of elements in blocks.   
   ///   @param enable - whether to enable or disable the static state        
   LANGULUS(INLINED)
   constexpr void Block::MakeStatic(bool enable) noexcept {
      if (enable)
         mState += DataState::Static;
      else
         mState -= DataState::Static;
   }

   /// Make memory block constant                                             
   /// Disables the ability to access members as mutable, and disallows       
   /// memory movement and reallocation                                       
   ///   @param enable - whether to enable or disable the constant state      
   LANGULUS(INLINED)
   constexpr void Block::MakeConst(bool enable) noexcept {
      if (enable)
         mState += DataState::Constant;
      else
         mState -= DataState::Constant;
   }

   /// Make memory block type-constrained                                     
   /// Doesn't allow insertion of data types that differ from the contained   
   /// one. Disallows any type mutations. Used extensively by TAny and other  
   /// statically typed Block equivalents.                                    
   ///   @param enable - whether to enable or disable the typed state         
   LANGULUS(INLINED)
   constexpr void Block::MakeTypeConstrained(bool enable) noexcept {
      if (enable)
         mState += DataState::Typed;
      else
         mState -= DataState::Typed;
   }

   /// Make memory block exclusive (aka an `OR` block)                        
   /// OR blocks are handled differently by Langulus::Flow, when executing    
   /// verbs in them, or when used as arguments for verbs.                    
   LANGULUS(INLINED)
   constexpr void Block::MakeOr() noexcept {
      mState += DataState::Or;
   }

   /// Make memory block inclusive (aka an `AND` block)                       
   /// AND blocks are handled conventionally by Langulus::Flow, as they are   
   /// the default type of blocks                                             
   LANGULUS(INLINED)
   constexpr void Block::MakeAnd() noexcept {
      mState -= DataState::Or;
   }

   /// Set memory block phase to past                                         
   /// This is used to mark missing past symbols for Langulus::Flow           
   LANGULUS(INLINED)
   constexpr void Block::MakePast() noexcept {
      mState -= DataState::Future;
      mState += DataState::Missing;
   }

   /// Set memory block phase to future                                       
   /// This is used to mark missing future symbols for Langulus::Flow         
   LANGULUS(INLINED)
   constexpr void Block::MakeFuture() noexcept {
      mState += DataState::MissingFuture;
   }
   
   /// Set memory block phase to neutral                                      
   /// This restores the default state of any kind of missing block, so that  
   /// it is processed conventionally by Langulus::Flow                       
   LANGULUS(INLINED)
   constexpr void Block::MakeNow() noexcept {
      mState -= DataState::MissingFuture;
   }

   /// Get entry array when block is sparse (const)                           
   ///   @attention entries exist only for sparse containers                  
   ///   @return the array of entries                                         
   LANGULUS(INLINED)
   const Allocation** Block::GetEntries() IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, IsSparse(),
         "Entries do not exist for dense container");
      return const_cast<const Allocation**>(
         reinterpret_cast<Allocation**>(mRawSparse + mReserved));
   }

   /// Get entry array when block is sparse (const)                           
   ///   @attention entries exist only for sparse containers                  
   ///   @return the array of entries                                         
   LANGULUS(INLINED)
   const Allocation* const* Block::GetEntries() const IF_UNSAFE(noexcept) {
      return const_cast<Block*>(this)->GetEntries();
   }

} // namespace Langulus::Anyness