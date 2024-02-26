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
   AllocationRequest Block::RequestSize(const Count count) const
   IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         if constexpr (CT::Fundamental<T> or CT::Exact<T, Byte>) {
            AllocationRequest result;
            result.mByteSize = ::std::max(Roof2(count * sizeof(T)), Alignment);
            result.mElementCount = result.mByteSize / sizeof(T);
            return result;
         }
         else return GetType<THIS>()->RequestSize(count);
      }
      else {
         LANGULUS_ASSUME(DevAssumes, IsTyped<THIS>(),
            "Requesting allocation size for an untyped container");
         return mType->RequestSize(count);
      }
   }

   /// Reserve a number of elements without initializing them                 
   /// If reserved data is smaller than currently initialized count, the      
   /// excess elements will be destroyed                                      
   ///   @tparam SETSIZE - whether or not to set size, too                    
   ///   @attention using SETSIZE will NOT construct any elements, use only   
   ///      if you know what you're doing                                     
   ///   @param count - number of elements to reserve                         
   template<bool SETSIZE, CT::Block THIS> LANGULUS(INLINED)
   void Block::Reserve(const Count count) {
      if (count < mCount)
         AllocateLess<THIS>(count);
      else 
         AllocateMore<THIS>(count);

      if constexpr (SETSIZE)
         mCount = count;
   }
   
   /// Allocate a number of elements, relying on the type of the container    
   ///   @attention assumes a valid and non-abstract type, if dense           
   ///   @tparam CREATE - true to call constructors and set count             
   ///   @tparam SETSIZE - true to set count, despite not constructing        
   ///   @param elements - number of elements to allocate                     
   template<CT::Block THIS, bool CREATE, bool SETSIZE>
   void Block::AllocateMore(const Count elements) {
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
                     CropInner(mCount, count).CreateDefault<THIS>();
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
                  if constexpr (CT::Inner::AbandonMakable<T>
                            or  CT::Inner::MoveMakable<T>
                            or  CT::Inner::ReferMakable<T>
                            or  CT::Inner::CopyMakable<T>
                  ) {
                     mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
                     CreateSemantic<THIS>(Abandon(previousBlock));
                     previousBlock.Free();
                  }
                  else LANGULUS_THROW(Construct,
                     "Memory moved, but T is not move-constructible");
               }
               else {
                  // Memory is used from multiple locations, and we must
                  // copy the memory for this block - we can't move it! 
                  // This will throw, if data is not copiable/referable 
                  if constexpr (CT::Inner::ReferMakable<T>) {
                     AllocateFresh<THIS>(request);
                     CreateSemantic<THIS>(Refer(previousBlock));
                  }
                  else if constexpr (CT::Inner::CopyMakable<T>) {
                     AllocateFresh<THIS>(request);
                     CreateSemantic<THIS>(Copy(previousBlock));
                  }
                  else LANGULUS_THROW(Construct,
                     "Memory moved, but T is not refer/copy-constructible");
               }
            }
            else {
               // Memory didn't move, but reserved count changed        
               if constexpr (CT::Sparse<T>) {
                  // Move entry data to its new place                   
                  MoveMemory(
                     GetEntries<THIS>(),
                     previousBlock.GetEntries<THIS>(),
                     mCount
                  );
               }
            }

            if constexpr (CREATE) {
               // Default-construct the rest                            
               const auto count = elements - mCount;
               CropInner(mCount, count).CreateDefault<THIS>();
            }
         }
         else {
            // Allocate a fresh set of elements                         
            mType = MetaDataOf<T>();
            AllocateFresh<THIS>(request);

            if constexpr (CREATE) {
               // Default-construct everything                          
               CropInner(mCount, elements).CreateDefault<THIS>();
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
                  CropInner(mCount, count).CreateDefault<THIS>();
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
   void Block::AllocateLess(const Count elements) {
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
                     GetEntries<THIS>() - mReserved + request.mElementCount,
                     GetEntries<THIS>(), mCount
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
                     GetEntries<THIS>() - mReserved + request.mElementCount,
                     GetEntries<THIS>(), mCount
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
   //TODO now can be substituded using the Copy semantic, instead of the Refer semantic
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::TakeAuthority() {
      if (mEntry or not mRaw)
         return;

      // Copy all elements                                              
      Block clone {*this};
      clone.AllocateFresh<THIS>(RequestSize<THIS>(mCount));
      clone.CreateSemantic<THIS>(Refer(*this));

      // Discard constness and staticness                               
      clone.mState -= DataState::Static | DataState::Constant;

      // Overwrite this block directly                                  
      CopyMemory(this, &clone);
   }

   /// Allocate a number of elements, relying on the type of the container    
   ///   @attention assumes a valid and non-abstract type, if dense           
   ///   @tparam CREATE - true to call constructors and set count             
   ///   @param elements - number of elements to allocate                     
   template<CT::Block THIS, bool CREATE>
   void Block::AllocateInner(Count elements) {
      LANGULUS_ASSERT(IsTyped<THIS>(), Allocate,
         "Invalid type");
      LANGULUS_ASSERT(not GetType<THIS>()->mIsAbstract or IsSparse<THIS>(), Allocate,
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
               CreateSemantic<THIS>(Abandon(previousBlock));
            }
            else {
               // Memory is used from multiple locations, and we must   
               // copy the memory for this block - we can't move it!    
               AllocateFresh<THIS>(request);
               CreateSemantic<THIS>(Refer(previousBlock));
               previousBlock.Free<Any>();
            }
         }
         else {
            // Memory didn't move, but reserved count changed           
            if (mType->mIsSparse) {
               // Move entry data to its new place                      
               MoveMemory(
                  GetEntries<THIS>(),
                  previousBlock.GetEntries<THIS>(),
                  mCount
               );
            }
         }
      }
      else AllocateFresh<THIS>(request);

      if constexpr (CREATE) {
         // Default-construct the rest                                  
         const auto count = elements - mCount;
         CropInner(mCount, count).CreateDefault<THIS>();
         mCount = elements;
      }
   }

   /// Allocate a fresh allocation (inner function)                           
   ///   @attention changes entry, memory and reserve count                   
   ///   @param request - request to fulfill                                  
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::AllocateFresh(const AllocationRequest& request) {
      // Sparse containers have additional memory allocated             
      // for each pointer's entry                                       
      mEntry = Allocator::Allocate(GetType<THIS>(),
         request.mByteSize * (IsSparse<THIS>() ? 2 : 1));

      LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
      mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
      mReserved = request.mElementCount;
   }

   /// Reference memory block if we own it                                    
   ///   @param times - number of references to add                           
   LANGULUS(INLINED)
   void Block::Reference(Count times) const noexcept {
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
         if (mCount)
            Destroy<THIS>();

         // Free memory                                                 
         Allocator::Deallocate(const_cast<Allocation*>(mEntry));
      }
      else {
         const_cast<Allocation*>(mEntry)->Free();
      }

      mEntry = nullptr;
   }

} // namespace Langulus::Anyness