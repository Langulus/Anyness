///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Block.hpp"


namespace Langulus::Anyness
{

   /// Get a size based on reflected allocation page and count                
   ///   @param count - the number of elements to request                     
   ///   @return both the provided byte size and reserved count               
   template<class TYPE> LANGULUS(INLINED)
   AllocationRequest Block<TYPE>::RequestSize(const Count count) const
   IF_UNSAFE(noexcept) {
      if constexpr (TypeErased) {
         LANGULUS_ASSUME(DevAssumes, IsTyped(),
            "Requesting allocation size for an untyped container");
         return mType->RequestSize(count);
      }
      else if constexpr (CT::Fundamental<TYPE> or CT::Exact<TYPE, Byte>) {
         AllocationRequest result;
         result.mByteSize = ::std::max(Roof2(count * sizeof(TYPE)), Alignment);
         result.mElementCount = result.mByteSize / sizeof(TYPE);
         return result;
      }
      else return GetType()->RequestSize(count);
   }

   /// Reserve a number of elements without initializing them                 
   /// If reserved data is smaller than currently initialized count, the      
   /// excess elements will be destroyed                                      
   ///   @tparam SETSIZE - whether or not to set size, too                    
   ///   @attention using SETSIZE will NOT construct any elements, use only   
   ///      if you know what you're doing                                     
   ///   @param count - number of elements to reserve                         
   template<class TYPE> template<bool SETSIZE> LANGULUS(INLINED)
   void Block<TYPE>::Reserve(const Count count) {
      if (count < mCount)
         AllocateLess(count);
      else 
         AllocateMore(count);

      if constexpr (SETSIZE)
         mCount = count;
   }
   
   /// Allocate a number of elements, relying on the type of the container    
   ///   @attention assumes a valid and non-abstract type, if dense           
   ///   @tparam CREATE - true to call constructors and set count             
   ///   @tparam SETSIZE - true to set count, despite not constructing        
   ///   @param elements - number of elements to allocate                     
   template<class TYPE> template<bool CREATE, bool SETSIZE>
   void Block<TYPE>::AllocateMore(const Count elements) {
      LANGULUS_ASSUME(DevAssumes, elements > mCount, "Bad element count");

      if constexpr (not TypeErased) {
         // Allocate/reallocate                                         
         const auto request = RequestSize(elements);
         if (mEntry) {
            if (mReserved >= elements) {
               // Required memory is already available                  
               if constexpr (CREATE) {
                  // But is not yet initialized, so initialize it       
                  if (mCount < elements) {
                     const auto count = elements - mCount;
                     CropInner(mCount, count).CreateDefault();
                  }
               }

               if constexpr (CREATE or SETSIZE)
                  mCount = elements;
               return;
            }

            // Reallocate                                               
            Block previousBlock {*this};
            mEntry = Allocator::Reallocate(
               request.mByteSize * (Sparse ? 2 : 1),
               const_cast<Allocation*>(mEntry)
            );
            LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
            mReserved = request.mElementCount;

            if (mEntry != previousBlock.mEntry) {
               if (not previousBlock.mCount) {
                  // Memory moved, but nothing was initialized, so      
                  // just update mRaw pointer                           
                  mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
               }
               else {
                  // Memory moved, and we should move all elements in it
                  // We're moving to new memory, so no reverse required 
                  if (mEntry->GetUses() == 1) {
                     // Memory is used only once and it is safe to move 
                     // it. Make note, that Allocator::Reallocate       
                     // doesn't copy anything, it doesn't use realloc   
                     // for various reasons, so we still have to call   
                     // move construction for all elements if entry     
                     // moved (enabling MANAGED_MEMORY feature          
                     // significantly reduces the chance for a move).   
                     // Sparse containers have additional memory        
                     // allocated for each pointer's entry, if managed  
                     // memory is enabled.                              
                     if constexpr (CT::AbandonMakable<TYPE>
                        or CT::MoveMakable<TYPE>
                        or CT::ReferMakable<TYPE>
                        or CT::CopyMakable<TYPE>
                        ) {
                        mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
                        CreateSemantic(Abandon(previousBlock));
                        previousBlock.Free();
                     }
                     else LANGULUS_THROW(Construct,
                        "Memory moved, but T is not move-constructible");
                  }
                  else {
                     // Memory is used from multiple locations, and we  
                     // must copy the memory for this block - we can't  
                     // move it! This will throw, if data is not        
                     // copiable/referable.                             
                     if constexpr (CT::ReferMakable<TYPE>) {
                        AllocateFresh(request);
                        CreateSemantic(Refer(previousBlock));
                     }
                     else if constexpr (CT::CopyMakable<TYPE>) {
                        AllocateFresh(request);
                        CreateSemantic(Copy(previousBlock));
                     }
                     else LANGULUS_THROW(Construct,
                        "Memory moved, but T is not refer/copy-constructible");
                  }
               }
            }
            else {
               // Memory didn't move, but reserved count changed        
               if constexpr (Sparse) {
                  // Move entry data to its new place                   
                  MoveMemory(
                     GetEntries(),
                     previousBlock.GetEntries(),
                     mCount
                  );
               }
            }

            if constexpr (CREATE) {
               // Default-construct the rest                            
               const auto count = elements - mCount;
               CropInner(mCount, count).CreateDefault();
            }
         }
         else {
            // Allocate a fresh set of elements                         
            mType = MetaDataOf<TYPE>();
            AllocateFresh(request);

            if constexpr (CREATE) {
               // Default-construct everything                          
               CropInner(mCount, elements).CreateDefault();
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
                  CropInner(mCount, count).CreateDefault();
               }
            }
         }
         else AllocateInner<CREATE>(elements);
      }

      if constexpr (CREATE or SETSIZE)
         mCount = elements;
   }

   /// Shrink the block, depending on currently reserved	elements             
   /// Initialized elements on the back will be destroyed                     
   ///   @attention assumes 'elements' is smaller than the current reserve    
   ///   @param elements - number of elements to allocate                     
   template<class TYPE> LANGULUS(INLINED)
   void Block<TYPE>::AllocateLess(const Count elements) {
      LANGULUS_ASSUME(DevAssumes, elements < mReserved, "Bad element count");

      if (mCount > elements) {
         // Destroy back entries on smaller allocation                  
         // Allowed even when container is static and out of            
         // jurisdiction, as in that case this acts as a simple count   
         // decrease, and no destructors shall be called                
         Trim(elements);
         return;
      }

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         // Shrink the memory block                                     
         // Guaranteed that entry doesn't move                          
         const auto request = RequestSize(elements);
         if (request.mElementCount != mReserved) {
            if constexpr (not TypeErased) {
               if constexpr (Sparse) {
                  // Move entry data to its new place                   
                  MoveMemory(
                     GetEntries() - mReserved + request.mElementCount,
                     GetEntries(), mCount
                  );
               }

               mEntry = Allocator::Reallocate(
                  request.mByteSize * (Sparse ? 2 : 1),
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

   /// Shallow-copy all elements inside this memory block into another block, 
   /// that is owned by us. Preserve hierarchy, density and state, but remove 
   /// size constraints and constness. If we already own this block's memory, 
   /// then only a Keep() is done                                             
   template<class TYPE> LANGULUS(INLINED)
   void Block<TYPE>::TakeAuthority() {
      if (not mRaw)
         return;

      if (mEntry) {
         // We already have authority, but to enforce it, we must ref   
         const_cast<Allocation*>(mEntry)->Keep();
         return;
      }

      // Shallow-copy all elements (equivalent to a Copy semantic)      
      Block shallowCopy {*this};
      shallowCopy.AllocateFresh(RequestSize(mCount));
      shallowCopy.CreateSemantic(Refer(*this));
      shallowCopy.mState -= DataState::Static | DataState::Constant;
      CopyMemory(this, &shallowCopy);
   }

   /// Allocate a number of elements, relying on the type of the container    
   ///   @attention assumes a valid and non-abstract type, if dense           
   ///   @tparam CREATE - true to call constructors and set count             
   ///   @param elements - number of elements to allocate                     
   template<class TYPE> template<bool CREATE>
   void Block<TYPE>::AllocateInner(Count elements) {
      LANGULUS_ASSERT(IsTyped(), Allocate,
         "Invalid type");
      LANGULUS_ASSERT(not GetType()->mIsAbstract or IsSparse(), Allocate,
         "Abstract dense type");

      // Retrieve the required byte size                                
      const auto request = RequestSize(elements);
      
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
               CreateSemantic(Abandon(previousBlock));
            }
            else {
               // Memory is used from multiple locations, and we must   
               // copy the memory for this block - we can't move it!    
               AllocateFresh(request);
               CreateSemantic(Refer(previousBlock));
               previousBlock.Free();
            }
         }
         else {
            // Memory didn't move, but reserved count changed           
            if (mType->mIsSparse) {
               // Move entry data to its new place                      
               MoveMemory(
                  GetEntries(),
                  previousBlock.GetEntries(),
                  mCount
               );
            }
         }
      }
      else AllocateFresh(request);

      if constexpr (CREATE) {
         // Default-construct the rest                                  
         const auto count = elements - mCount;
         CropInner(mCount, count).CreateDefault();
         mCount = elements;
      }
   }

   /// Allocate a fresh allocation (inner function)                           
   ///   @attention changes entry, memory and reserve count                   
   ///   @param request - request to fulfill                                  
   template<class TYPE> LANGULUS(INLINED)
   void Block<TYPE>::AllocateFresh(const AllocationRequest& request) {
      // Sparse containers have additional memory allocated             
      // for each pointer's entry                                       
      mEntry = Allocator::Allocate(GetType(),
         request.mByteSize * (IsSparse() ? 2 : 1));

      LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
      mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
      mReserved = request.mElementCount;
   }

   /// Reference memory block once                                            
   template<class TYPE> LANGULUS(INLINED)
   void Block<TYPE>::Keep() const noexcept {
      if (mEntry)
         const_cast<Allocation*>(mEntry)->Keep(1);
   }

   /// Dereference memory block once and destroy all elements if data was     
   /// fully dereferenced                                                     
   ///   @attention this never modifies any state, except mEntry              
   template<class TYPE> LANGULUS(INLINED)
   void Block<TYPE>::Free() {
      if (not mEntry)
         return;

      LANGULUS_ASSUME(DevAssumes, mEntry->GetUses() >= 1,
         "Bad memory dereferencing");

      if (mEntry->GetUses() == 1) {
         // Free memory                                                 
         LANGULUS_ASSUME(DevAssumes, not IsStatic(),
            "Last reference, but container was marked static"
            " - make sure initialization of this container was correct, "
            "did you forget to add a reference?");

         if (mCount)
            Destroy();

         Allocator::Deallocate(const_cast<Allocation*>(mEntry));
      }
      else {
         // Dereference memory                                          
         if (mCount and not mState.IsStatic())
            Destroy<false>();

         const_cast<Allocation*>(mEntry)->Free();
      }

      mEntry = nullptr;
   }

} // namespace Langulus::Anyness