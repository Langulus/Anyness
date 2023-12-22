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


namespace Langulus::A
{

   /// Manual construction via type                                           
   ///   @param meta - the type of the memory block                           
   LANGULUS(INLINED)
   constexpr Block::Block(DMeta meta) noexcept
      : mType {meta} { }

   /// Manual construction via state and type                                 
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   LANGULUS(INLINED)
   constexpr Block::Block(const DataState& state, DMeta meta) noexcept
      : mState {state}
      , mType {meta} { }
   
   /// Manual construction via state and a reflected constant                 
   ///   @param state - the initial state of the container                    
   ///   @param meta - the constant definition                                
   LANGULUS(INLINED)
   Block::Block(const DataState& state, CMeta meta) IF_UNSAFE(noexcept)
      : Block {
         state + DataState::Constrained,
         meta->mValueType, 1, meta->mPtrToValue, nullptr
      } {}
   
   /// Manual construction from mutable data                                  
   /// This constructor has runtime overhead if managed memory is enabled     
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the mutable memory                           
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, void* raw)
      IF_UNSAFE(noexcept)
      : Block {
         state + DataState::Member,
         meta, count, raw, Allocator::Find(meta, raw)
      } {}
   
   /// Manual construction from constant data                                 
   /// This constructor has runtime overhead if managed memory is enabled     
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the constant memory                          
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, const void* raw)
      IF_UNSAFE(noexcept)
      : Block {state + DataState::Constant, meta, count, const_cast<void*>(raw)}
   {}

   /// Manual construction from mutable data and known entry                  
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the mutable memory                           
   ///   @param entry - the memory entry                                      
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, void* raw, const Allocation* entry)
      IF_UNSAFE(noexcept)
      : mRaw {static_cast<Byte*>(raw)}
      , mState {state}
      , mCount {count}
      , mReserved {count}
      , mType {meta}
      , mEntry {entry} {
      LANGULUS_ASSUME(DevAssumes, raw, "Invalid data pointer");
      LANGULUS_ASSUME(DevAssumes, meta, "Invalid data type");
      LANGULUS_ASSUME(DevAssumes, not meta->mIsSparse,
         "Sparse raw data initialization is not allowed");
   }
   
   /// Manual construction from constant data and known entry                 
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the constant memory                          
   ///   @param entry - the memory entry                                      
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, const void* raw, const Allocation* entry)
      IF_UNSAFE(noexcept)
      : Block {state + DataState::Constant, meta, count, const_cast<void*>(raw), entry}
   {}

} // namespace Langulus::A


namespace Langulus::Anyness
{

   /// Manual construction via type                                           
   ///   @param meta - the type of the memory block                           
   /*LANGULUS(INLINED)
   constexpr Block::Block(DMeta meta) noexcept
      : A::Block {meta} {}

   /// Manual construction via state and type                                 
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   LANGULUS(INLINED)
   constexpr Block::Block(const DataState& state, DMeta meta) noexcept
      : A::Block {state, meta} {}
   
   /// Manual construction via state and a reflected constant                 
   ///   @param state - the initial state of the container                    
   ///   @param meta - the constant definition                                
   LANGULUS(INLINED)
   Block::Block(const DataState& state, CMeta meta) IF_UNSAFE(noexcept)
      : A::Block {state, meta} {}
   
   /// Manual construction from mutable data                                  
   /// This constructor has runtime overhead if managed memory is enabled     
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the mutable memory                           
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, void* raw)
      IF_UNSAFE(noexcept)
      : A::Block {state, meta, count, raw} {}
   
   /// Manual construction from constant data                                 
   /// This constructor has runtime overhead if managed memory is enabled     
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the constant memory                          
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, const void* raw)
      IF_UNSAFE(noexcept)
      : A::Block {state, meta, count, raw} {}

   /// Manual construction from mutable data and known entry                  
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the mutable memory                           
   ///   @param entry - the memory entry                                      
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, void* raw, const Allocation* entry)
      IF_UNSAFE(noexcept)
      : A::Block {state, meta, count, raw, entry} {}
   
   /// Manual construction from constant data and known entry                 
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the constant memory                          
   ///   @param entry - the memory entry                                      
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, const void* raw, const Allocation* entry)
      IF_UNSAFE(noexcept)
      : A::Block {state, meta, count, raw, entry} {}*/
   
   /// Create a dense memory block, by interfacing a single pointer           
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @tparam T - the type of the pointer to wrap (deducible)              
   ///   @param value - the pointer to interface                              
   ///   @return the block                                                    
   template<bool CONSTRAIN, CT::Data T>
   LANGULUS(INLINED)
   Block Block::From(T value) requires CT::Sparse<T> {
      if constexpr (CONSTRAIN)
         return {DataState::Member, MetaDataOf<Deptr<T>>(), 1, value};
      else
         return {DataState::Static, MetaDataOf<Deptr<T>>(), 1, value};
   }

   /// Create a memory block from a count-terminated array pointer            
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @tparam T - the type of the pointer to wrap (deducible)              
   ///   @param value - the pointer to the first element                      
   ///   @param count - the number of elements                                
   ///   @return the block                                                    
   template<bool CONSTRAIN, CT::Data T>
   LANGULUS(INLINED)
   Block Block::From(T value, Count count) requires CT::Sparse<T> {
      if constexpr (CONSTRAIN)
         return {DataState::Member, MetaDataOf<Deptr<T>>(), count, value};
      else
         return {DataState::Static, MetaDataOf<Deptr<T>>(), count, value};
   }

   /// Create a dense memory block, by interfacing a single value             
   /// If value is resolvable, GetBlock() will produce the Block              
   /// If value is deep, the value's block will be copied                     
   /// Anything else will be interfaced via a new Block                       
   ///   @attention value's memory lifetime is your responsibility            
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @tparam T - the type of the value to wrap (deducible)                
   ///   @return a block that wraps the dense value                           
   template<bool CONSTRAIN, CT::Data T>
   LANGULUS(INLINED)
   Block Block::From(T& value) requires CT::Dense<T> {
      Block result;
      if constexpr (CT::Resolvable<T>) {
         // Resolve a runtime-resolvable value                          
         result = value.GetBlock();
      }
      else if constexpr (CT::Deep<T>) {
         // Static cast to Block if CT::Deep                            
         result.operator = (value);
      }
      else {
         // Any other value gets wrapped inside a temporary Block       
         result = {
            DataState::Static, 
            MetaDataOf<Decvq<Deref<T>>>(),
            1, &value
         };
      }
      
      if constexpr (CONSTRAIN)
         result.MakeTypeConstrained();
      return result;
   }

   /// Create an empty typed block                                            
   ///   @tparam T - the type of the container                                
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @return the block                                                    
   template<CT::Data T, bool CONSTRAIN> LANGULUS(INLINED)
   Block Block::From() {
      if constexpr (CONSTRAIN)
         return {DataState::Typed, MetaDataOf<T>()};
      else
         return {MetaDataOf<T>()};
   }
   
   /// Semantically transfer the members of one block onto another with the   
   /// smallest number of instructions possible                               
   ///   @attention will not set mType if TO is type-constrained              
   ///   @attention will not set mRaw, mReserved, mEntry, if 'from' is empty  
   ///   @tparam TO - the type of block we're transferring to                 
   ///   @param from - the block and semantic to transfer from                
   template<class TO>
   LANGULUS(INLINED)
   void Block::BlockTransfer(CT::Semantic auto&& from) {
      using S = Decay<decltype(from)>;
      using FROM = TypeOf<S>;
      static_assert(CT::Block<TO>, "TO must be a block type");
      static_assert(CT::Block<FROM>, "FROM must be a block type");

      mCount = from->mCount;

      if constexpr (not CT::Typed<TO>) {
         // TO is not statically typed, so we can safely                
         // overwrite type and state                                    
         mType = from->mType;
         mState = from->mState;
      }
      else {
         // TO is typed, so we never touch mType, and we make sure that 
         // we don't affect Typed state                                 
         mState = from->mState + DataState::Typed;
      }

      if constexpr (S::Shallow) {
         // We're transferring via a shallow semantic                   
         mRaw = from->mRaw;
         mReserved = from->mReserved;

         if constexpr (S::Keep) {
            // Move/Copy other                                          
            mEntry = from->mEntry;

            if constexpr (S::Move) {
               if constexpr (not FROM::Ownership) {
                  // Since we are not aware if that block is referenced 
                  // or not we reference it just in case, and we also   
                  // do not reset 'other' to avoid leaks. When using    
                  // raw Blocks, it's your responsibility to take care  
                  // of ownership.                                      
                  Keep();
               }
               else {
                  from->ResetMemory();
                  from->ResetState();
               }
            }
            else Keep();
         }
         else if constexpr (S::Move) {
            // Abandon other                                            
            mEntry = from->mEntry;
            from->mEntry = nullptr;
         }
      }
      else {
         // We're cloning, so we guarantee, that data is no longer      
         // static and constant (unless mType is constant)              
         mState -= DataState::Static | DataState::Constant;
         if (0 == mCount)
            return;
         
         if constexpr (CT::Typed<FROM>) {
            auto asTo = reinterpret_cast<FROM*>(this);
            asTo->AllocateFresh(asTo->RequestSize(mCount));
            CallKnownSemanticConstructors<TypeOf<FROM>>(
               mCount, from.template Forward<Block>());
         }
         else if constexpr (CT::Typed<TO>) {
            auto asTo = reinterpret_cast<TO*>(this);
            asTo->AllocateFresh(asTo->RequestSize(mCount));
            CallKnownSemanticConstructors<TypeOf<TO>>(
               mCount, from.template Forward<Block>());
         }
         else {
            AllocateFresh(RequestSize(mCount));
            CallUnknownSemanticConstructors(
               mCount, from.template Forward<Block>());
         }
      }
   }
   
   /// Helper function for constructing from a single item                    
   ///   @attention if 'other' is CT::Deep, it will be absorbed               
   ///   @param other - the element/container and semantic to initialize with 
   LANGULUS(INLINED)
   void Block::CreateFrom(CT::Semantic auto&& other) {
      using T = TypeOf<decltype(other)>;
      if constexpr (CT::Deep<T>)
         BlockTransfer<Any>(other.Forward());
      else
         UnfoldInsertion<false>(0, other.Forward());
   }

   /// Swap contents of this block, with the contents of another, using       
   /// a temporary block, completely type-erased and as efficient as possible 
   ///   @attention assumes both containers have same initialized count       
   ///   @attention assumes both containers have same type                    
   ///   @param rhs - the block to swap with                                  
   void Block::SwapUnknown(CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      LANGULUS_ASSUME(DevAssumes, rhs->mCount == mCount,
         "Count mismatch");
      LANGULUS_ASSUME(DevAssumes, mCount,
         "Can't swap zero count");
      LANGULUS_ASSUME(DevAssumes, IsExact(rhs->GetType()),
         "Type mismatch");

      Block temporary {mState, mType};
      temporary.AllocateFresh(temporary.RequestSize(mCount));
      temporary.mCount = mCount;

      // Abandon this to temporary                                      
      temporary.CallUnknownSemanticConstructors(mCount, Abandon(*this));
      // Destroy elements in this                                       
      CallUnknownDestructors();
      // Abandon rhs to this                                            
      CallUnknownSemanticConstructors(rhs->mCount, rhs.Forward());
      // Destroy elements in rhs                                        
      rhs->CallUnknownDestructors();
      // Abandon temporary to rhs                                       
      rhs->CallUnknownSemanticConstructors(temporary.mCount, Abandon(temporary));

      // Cleanup temporary                                              
      temporary.CallUnknownDestructors();
      Allocator::Deallocate(const_cast<Allocation*>(temporary.mEntry));
   }

   /// Swap contents of this block, with the contents of another, using       
   /// a temporary block, statically optimized and as efficient as possible   
   ///   @attention assumes both containers have same initialized count       
   ///   @attention assumes T is the type of this and rhs                     
   ///   @param rhs - the block to swap with                                  
   template<CT::Data T>
   void Block::SwapKnown(Block& rhs) {
      LANGULUS_ASSUME(DevAssumes, rhs.mCount == mCount,
         "Count mismatch");
      LANGULUS_ASSUME(DevAssumes, mCount,
         "Can't swap zero count");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>() and rhs.IsExact<T>(),
         "Type mismatch");

      Block temporary {mState, mType};
      temporary.AllocateFresh(temporary.RequestSize(mCount));
      temporary.mCount = mCount;

      // Abandon this to temporary                                      
      temporary.CallKnownSemanticConstructors<T>(mCount, Abandon(*this));
      // Destroy elements in this                                       
      CallKnownDestructors<T>();
      // Abandon rhs to this                                            
      CallKnownSemanticConstructors<T>(rhs.mCount, Abandon(rhs));
      // Destroy elements in rhs                                        
      rhs.CallKnownDestructors<T>();
      // Abandon temporary to rhs                                       
      rhs.CallKnownSemanticConstructors<T>(temporary.mCount, Abandon(temporary));

      // Cleanup temporary                                              
      temporary.CallKnownDestructors<T>();
      Allocator::Deallocate(const_cast<Allocation*>(temporary.mEntry));
   }

} // namespace Langulus::Anyness