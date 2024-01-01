///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block-Indexing.inl"


namespace Langulus::Anyness
{

   /// Get a size based on reflected allocation page and count                
   ///   @param count - the number of elements to request                     
   ///   @return both the provided byte size and reserved count               
   template<CT::Block THIS> LANGULUS(INLINED)
   AllocationRequest Block::RequestSize(const Count& count) const
   IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         if constexpr (CT::Fundamental<T> or CT::Exact<T, Byte>) {
            AllocationRequest result;
            result.mByteSize = ::std::max(Roof2(count * sizeof(T)), Alignment);
            result.mElementCount = result.mByteSize / sizeof(T);
            return result;
         }
         else return GetType()->RequestSize(count);
      }
      else {
         LANGULUS_ASSUME(DevAssumes, IsTyped(),
            "Requesting allocation size for an untyped container");
         return mType->RequestSize(count);
      }
   }

   /// Reserve a number of elements without initializing them                 
   /// If reserved data is smaller than currently initialized count, the      
   /// excess elements will be destroyed                                      
   ///   @param count - number of elements to reserve                         
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::Reserve(Count count) {
      if (count < mCount)
         AllocateLess<THIS>(count);
      else 
         AllocateMore<THIS>(count);
   }
   
   /// Allocate a number of elements, relying on the type of the container    
   ///   @attention assumes a valid and non-abstract type, if dense           
   ///   @tparam CREATE - true to call constructors and set count             
   ///   @tparam SETSIZE - true to set count, despite not constructing        
   ///   @param elements - number of elements to allocate                     
   template<CT::Block THIS, bool CREATE, bool SETSIZE>
   void Block::AllocateMore(Count elements) {
      LANGULUS_ASSUME(DevAssumes, elements > mCount, "Bad element count");

      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;

         // Allocate/reallocate                                         
         const auto request = RequestSize<THIS>(elements);
         if (mEntry) {
            if (mReserved >= elements) {
               // Required memory is already available                  
               if constexpr (CREATE) {
                  // But is not yet initialized, so initialize it       
                  if (mCount < elements) {
                     const auto count = elements - mCount;
                     CropInner(mCount, count)
                        .template CallKnownDefaultConstructors<T>(count);
                  }
               }

               if constexpr (CREATE or SETSIZE)
                  mCount = elements;
               return;
            }

            // Reallocate                                               
            Block previousBlock {*this};
            mEntry = Allocator::Reallocate(
               request.mByteSize * (CT::Sparse<T> ? 2 : 1),
               const_cast<Allocation*>(mEntry)
            );
            LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
            mReserved = request.mElementCount;

            if (mEntry != previousBlock.mEntry) {
               // Memory moved, and we should move all elements in it   
               // We're moving to new memory, so no reverse required    
               if (mEntry->GetUses() == 1) {
                  // Memory is used only once and it is safe to move it 
                  // Make note, that Allocator::Reallocate doesn't copy 
                  // anything, it doesn't use realloc for various       
                  // reasons, so we still have to call move construction
                  // for all elements if entry moved (enabling          
                  // MANAGED_MEMORY feature significantly reduces the   
                  // chance for a move). Sparse containers have         
                  // additional memory allocated for each pointer's     
                  // entry, if managed memory is enabled                
                  if constexpr (CT::AbandonMakable<T> or CT::MoveMakable<T> or CT::CopyMakable<T>) {
                     mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
                     CallKnownSemanticConstructors<T>(
                        previousBlock.mCount, Abandon(previousBlock)
                     );

                     // Also, we should free the previous allocation    
                     previousBlock.Free();
                  }
                  else LANGULUS_THROW(Construct, "Memory moved, but T is not move-constructible");
               }
               else {
                  // Memory is used from multiple locations, and we must
                  // copy the memory for this block - we can't move it! 
                  // This will throw, if data is not copy-constructible 
                  if constexpr (CT::DisownMakable<T> or CT::CopyMakable<T>) {
                     AllocateFresh(request);
                     CallKnownSemanticConstructors<T>(
                        previousBlock.mCount, Copy(previousBlock)
                     );
                  }
                  else LANGULUS_THROW(Construct, "Memory moved, but T is not copy-constructible");
               }
            }
            else {
               // Memory didn't move, but reserved count changed        
               if constexpr (CT::Sparse<T>) {
                  // Move entry data to its new place                   
                  MoveMemory(
                     GetEntries(), previousBlock.GetEntries(), mCount
                  );
               }
            }

            if constexpr (CREATE) {
               // Default-construct the rest                            
               const auto count = elements - mCount;
               CropInner(mCount, count)
                  .template CallKnownDefaultConstructors<T>(count);
            }
         }
         else {
            // Allocate a fresh set of elements                         
            mType = MetaDataOf<T>();
            AllocateFresh(request);

            if constexpr (CREATE) {
               // Default-construct everything                          
               CropInner(mCount, elements)
                  .template CallKnownDefaultConstructors<T>(elements);
            }
         }
      }
      else {
         if (not mType or (mType->mIsAbstract and not mType->mIsSparse)) {
            if (not mType)
               Logger::Error("Can't instantiate unknown type");
            else
               Logger::Error("Unable to instantiate ", elements,
                  " elements of abstract type ", mType);

            LANGULUS_THROW(Allocate, "Allocating untyped/abstract/sparse data");
         }

         if (mReserved >= elements) {
            // Required memory is already available                     
            if constexpr (CREATE) {
               // But is not yet initialized, so initialize it          
               if (mCount < elements) {
                  const auto count = elements - mCount;
                  CropInner(mCount, count)
                     .CallUnknownDefaultConstructors(count);
               }
            }
         }
         else AllocateInner<THIS, CREATE>(elements);
      }

      if constexpr (CREATE or SETSIZE)
         mCount = elements;
   }

   /// Shrink the block, depending on currently reserved	elements             
   /// Initialized elements on the back will be destroyed                     
   ///   @attention assumes 'elements' is smaller than the current reserve    
   ///   @param elements - number of elements to allocate                     
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::AllocateLess(Count elements) {
      LANGULUS_ASSUME(DevAssumes, elements < mReserved, "Bad element count");

      if (mCount > elements) {
         // Destroy back entries on smaller allocation                  
         // Allowed even when container is static and out of            
         // jurisdiction, as in that case this acts as a simple count   
         // decrease, and no destructors shall be called                
         Trim<THIS>(elements);
         return;
      }

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         // Shrink the memory block                                     
         // Guaranteed that entry doesn't move                          
         const auto request = RequestSize<THIS>(elements);
         if (request.mElementCount != mReserved) {
            if constexpr (CT::Typed<THIS>) {
               using T = TypeOf<THIS>;

               if constexpr (CT::Sparse<T>) {
                  // Move entry data to its new place                   
                  MoveMemory(
                     GetEntries() - mReserved + request.mElementCount,
                     GetEntries(), mCount
                  );
               }

               mEntry = Allocator::Reallocate(
                  request.mByteSize * (CT::Sparse<T> ? 2 : 1),
                  const_cast<Allocation*>(mEntry)
               );
            }
            else {
               LANGULUS_ASSUME(DevAssumes, mType, "Invalid type");

               if (mType->mIsSparse) {
                  // Move entry data to its new place                   
                  MoveMemory(
                     GetEntries() - mReserved + request.mElementCount,
                     GetEntries(), mCount
                  );
               }

               mEntry = Allocator::Reallocate(
                  request.mByteSize * (mType->mIsSparse ? 2 : 1),
                  const_cast<Allocation*>(mEntry)
               );
            }

            mReserved = request.mElementCount;
         }
      #endif
   }

   /// Duplicate all elements inside this memory block into another block,    
   /// that is owned by us. Preserve hierarchy, density and state, but remove 
   /// size constraints and constness. If we already own this block's memory, 
   /// then nothing happens                                                   
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::TakeAuthority() {
      if (mEntry or not mRaw)
         return;

      // Copy all elements                                              
      auto& me = reinterpret_cast<THIS&>(*this);
      Block clone {*this};
      clone.AllocateFresh(me.template RequestSize<THIS>(mCount));
      if constexpr (CT::Typed<THIS>)
         clone.CallKnownSemanticConstructors<TypeOf<THIS>>(mCount, Copy(*this));
      else
         clone.CallUnknownSemanticConstructors(mCount, Copy(*this));

      // Overwrite this block directly                                  
      CopyMemory(this, &clone);
   }

   /// Allocate a number of elements, relying on the type of the container    
   ///   @attention assumes a valid and non-abstract type, if dense           
   ///   @tparam CREATE - true to call constructors and set count             
   ///   @param elements - number of elements to allocate                     
   template<CT::Block THIS, bool CREATE>
   void Block::AllocateInner(const Count& elements) {
      LANGULUS_ASSERT(mType, Allocate,
         "Invalid type");
      LANGULUS_ASSERT(not mType->mIsAbstract or IsSparse(), Allocate,
         "Abstract dense type");

      // Retrieve the required byte size                                
      const auto request = RequestSize<THIS>(elements);
      
      // Allocate/reallocate                                            
      if (mEntry) {
         // Reallocate                                                  
         Block previousBlock {*this};
         mEntry = Allocator::Reallocate(
            request.mByteSize * (mType->mIsSparse ? 2 : 1),
            const_cast<Allocation*>(mEntry)
         );
         LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
         mReserved = request.mElementCount;

         if (mEntry != previousBlock.mEntry) {
            // Memory moved, and we should call abandon-construction    
            // We're moving to a new allocation, so no reverse needed   
            if (mEntry->GetUses() == 1) {
               // Memory is used only once and it is safe to move it    
               // Make note, that Allocator::Reallocate doesn't copy    
               // anything, it doesn't use realloc for various reasons, 
               // so we still have to call move construction for all    
               // elements if entry moved (enabling MANAGED_MEMORY      
               // feature significantly reduces the chance for a move)  
               // Also, make sure to free the previous mEntry if moved  
               // Sparse containers have additional memory allocated for
               // each pointer's entry, if managed memory is enabled    
               mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
               CallUnknownSemanticConstructors(previousBlock.mCount,
                  Abandon(previousBlock));
            }
            else {
               // Memory is used from multiple locations, and we must      
               // copy the memory for this block - we can't move it!       
               AllocateFresh(request);
               CallUnknownSemanticConstructors(previousBlock.mCount,
                  Copy(previousBlock));
               previousBlock.Free<Any>();
            }
         }
         else {
            // Memory didn't move, but reserved count changed        
            if (mType->mIsSparse) {
               // Move entry data to its new place                   
               MoveMemory(
                  GetEntries(), previousBlock.GetEntries(), mCount
               );
            }
         }
      }
      else AllocateFresh(request);

      if constexpr (CREATE) {
         // Default-construct the rest                                  
         const auto count = elements - mCount;
         CropInner(mCount, count)
            .CallUnknownDefaultConstructors(count);
         mCount = elements;
      }
   }

   /// Allocate a fresh allocation (inner function)                           
   ///   @attention changes entry, memory and reserve count                   
   ///   @param request - request to fulfill                                  
   LANGULUS(INLINED)
   void Block::AllocateFresh(const AllocationRequest& request) {
      // Sparse containers have additional memory allocated             
      // for each pointer's entry                                       
      mEntry = Allocator::Allocate(
         mType, request.mByteSize * (mType->mIsSparse ? 2 : 1)
      );
      LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
      mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
      mReserved = request.mElementCount;
   }

   /// Reference memory block if we own it                                    
   ///   @param times - number of references to add                           
   LANGULUS(INLINED)
   void Block::Reference(const Count& times) const noexcept {
      if (mEntry)
         const_cast<Allocation*>(mEntry)->Keep(times);
   }
   
   /// Reference memory block once                                            
   LANGULUS(INLINED)
   void Block::Keep() const noexcept {
      Reference(1);
   }

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @attention this never modifies any state, except mEntry              
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::Free() {
      if (not mEntry)
         return;

      LANGULUS_ASSUME(DevAssumes, 
         mEntry->GetUses() >= 1, "Bad memory dereferencing");

      if (mEntry->GetUses() == 1) {
         // Destroy all elements                                        
         if constexpr (CT::Typed<THIS>) {
            using T = TypeOf<THIS>;
            if constexpr (CT::Sparse<T> or CT::Destroyable<T>) {
               if (mCount)
                  CallKnownDestructors<T>();
            }
         }
         else {
            // Call type-erased destructors                             
            if (mCount)
               CallUnknownDestructors();
         }

         // Free memory                                                 
         Allocator::Deallocate(const_cast<Allocation*>(mEntry));
      }
      else const_cast<Allocation*>(mEntry)->Free(1);

      mEntry = nullptr;
   }
   
   /// Sets the currently interfaced memory                                   
   ///   @attention for internal use only, use only if you know what you're   
   ///              doing!                                                    
   LANGULUS(INLINED)
   void Block::SetMemory(const DataState& state
      , DMeta meta
      , Count count
      , const void* raw
   ) IF_UNSAFE(noexcept) {
      SetMemory(
         state + DataState::Constant, meta, count, 
         const_cast<void*>(raw)
      );
   }

   /// Sets the currently interfaced memory                                   
   ///   @attention for internal use only, use only if you know what you're   
   ///              doing!                                                    
   LANGULUS(INLINED)
   void Block::SetMemory(const DataState& state
      , DMeta meta
      , Count count
      , void* raw
   ) IF_UNSAFE(noexcept) {
      SetMemory(
         state, meta, count, 
         const_cast<void*>(raw),
         Allocator::Find(meta, raw)
      );
   }

   /// Sets the currently interfaced memory                                   
   ///   @attention for internal used only, use only if you know what you're  
   ///              doing!                                                    
   LANGULUS(INLINED) IF_UNSAFE(constexpr)
   void Block::SetMemory(const DataState& state
      , DMeta meta
      , Count count
      , const void* raw
      , const Allocation* entry
   ) {
      SetMemory(
         state + DataState::Constant, meta, count, 
         const_cast<void*>(raw), entry
      );
   }

   /// Sets the currently interfaces memory                                   
   ///   @attention for internal used only, use only if you know what you're  
   ///              doing!                                                    
   LANGULUS(INLINED) IF_UNSAFE(constexpr)
   void Block::SetMemory(const DataState& state
      , DMeta meta
      , Count count
      , void* raw
      , const Allocation* entry
   ) {
      LANGULUS_ASSUME(DevAssumes, raw, "Invalid data pointer");
      LANGULUS_ASSUME(DevAssumes, meta, "Invalid data type");
      LANGULUS_ASSUME(DevAssumes, not meta->mIsSparse,
         "Sparse raw data initialization is not allowed");

      mRaw = static_cast<Byte*>(raw);
      mState = state;
      mCount = count;
      mReserved = count;
      mType = meta;
      mEntry = entry;
   }

} // namespace Langulus::Anyness