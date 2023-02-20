///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block.hpp"

namespace Langulus::Anyness
{
   
   /// Semantic copy (block has no ownership, so always just shallow copy)    
   ///   @tparam S - the semantic to use (irrelevant)                         
   ///   @param other - the block to shallow-copy                             
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr Block::Block(S&& other) noexcept
      : Block {static_cast<const Block&>(other.mValue)} {}

   /// Manual construction via type                                           
   ///   @param meta - the type of the memory block                           
   LANGULUS(ALWAYSINLINE)
   constexpr Block::Block(DMeta meta) noexcept
      : mType {meta} { }

   /// Manual construction via state and type                                 
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   LANGULUS(ALWAYSINLINE)
   constexpr Block::Block(const DataState& state, DMeta meta) noexcept
      : mState {state}
      , mType {meta} { }
   
   /// Manual construction from mutable data                                  
   /// This constructor has runtime overhead if managed memory is enabled     
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the mutable memory                           
   LANGULUS(ALWAYSINLINE)
   Block::Block(const DataState& state
      , DMeta meta
      , Count count
      , void* raw
   ) SAFETY_NOEXCEPT()
      : mRaw {static_cast<Byte*>(raw)}
      , mState {state}
      , mCount {count}
      , mReserved {count}
      , mType {meta}
      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         , mEntry {Inner::Allocator::Find(meta, raw)}
      #else
         , mEntry {nullptr}
      #endif
   {
      LANGULUS_ASSUME(DevAssumes, raw != nullptr,
         "Invalid data pointer");
      LANGULUS_ASSUME(DevAssumes, meta != nullptr,
         "Invalid data type");
      LANGULUS_ASSUME(DevAssumes, !meta->mIsSparse,
         "Sparse raw data initialization is not allowed");
   }
   
   /// Manual construction from constant data                                 
   /// This constructor has runtime overhead if managed memory is enabled     
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the constant memory                          
   LANGULUS(ALWAYSINLINE)
   Block::Block(const DataState& state
      , DMeta meta
      , Count count
      , const void* raw
   ) SAFETY_NOEXCEPT()
      : Block {state + DataState::Constant, meta, count, const_cast<void*>(raw)}
   { }

   /// Manual construction from mutable data and known entry                  
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the mutable memory                           
   ///   @param entry - the memory entry                                      
   LANGULUS(ALWAYSINLINE)
   Block::Block(const DataState& state
      , DMeta meta
      , Count count
      , void* raw
      , Inner::Allocation* entry
   ) SAFETY_NOEXCEPT()
      : mRaw {static_cast<Byte*>(raw)}
      , mState {state}
      , mCount {count}
      , mReserved {count}
      , mType {meta}
      , mEntry {entry}
   {
      LANGULUS_ASSUME(DevAssumes, raw != nullptr,
         "Invalid data pointer");
      LANGULUS_ASSUME(DevAssumes, meta != nullptr,
         "Invalid data type");
      LANGULUS_ASSUME(DevAssumes, !meta->mIsSparse,
         "Sparse raw data initialization is not allowed");
   }
   
   /// Manual construction from constant data and known entry                 
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the constant memory                          
   ///   @param entry - the memory entry                                      
   LANGULUS(ALWAYSINLINE)
   Block::Block(const DataState& state
      , DMeta meta
      , Count count
      , const void* raw
      , Inner::Allocation* entry
   ) SAFETY_NOEXCEPT()
      : Block {state + DataState::Constant, meta, count, const_cast<void*>(raw), entry}
   {}
   
   /// Create a dense memory block, by interfacing a single pointer           
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @tparam T - the type of the pointer to wrap (deducible)              
   ///   @param value - the pointer to interface                              
   ///   @return the block                                                    
   template<bool CONSTRAIN, CT::Data T>
   LANGULUS(ALWAYSINLINE)
   Block Block::From(T value) requires CT::Sparse<T> {
      if constexpr (CONSTRAIN)
         return {DataState::Member, MetaData::Of<Deptr<T>>(), 1, value};
      else
         return {DataState::Static, MetaData::Of<Deptr<T>>(), 1, value};
   }

   /// Create a memory block from a count-terminated array pointer            
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @tparam T - the type of the pointer to wrap (deducible)              
   ///   @param value - the pointer to the first element                      
   ///   @param count - the number of elements                                
   ///   @return the block                                                    
   template<bool CONSTRAIN, CT::Data T>
   LANGULUS(ALWAYSINLINE)
   Block Block::From(T value, Count count) requires CT::Sparse<T> {
      if constexpr (CONSTRAIN)
         return {DataState::Member, MetaData::Of<Deptr<T>>(), count, value};
      else
         return {DataState::Static, MetaData::Of<Deptr<T>>(), count, value};
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
   LANGULUS(ALWAYSINLINE)
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
            MetaData::Of<Decvq<Deref<T>>>(), 
            1, &value
         };

         if constexpr (CT::Constant<T>)
            result.MakeConst();
      }
      
      if constexpr (CONSTRAIN)
         result.MakeTypeConstrained();
      return result;
   }

   /// Create an empty typed block                                            
   ///   @tparam T - the type of the container                                
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @return the block                                                    
   template<CT::Data T, bool CONSTRAIN>
   LANGULUS(ALWAYSINLINE)
   Block Block::From() {
      if constexpr (CONSTRAIN)
         return {DataState::Typed, MetaData::Of<T>()};
      else
         return {MetaData::Of<T>()};
   }

   /// Semantic assignment                                                    
   /// Blocks have no ownership, so this always results in a block transfer   
   ///   @attention will never affect RHS                                     
   ///   @tparam S - semantic to use (irrelevant)                             
   ///   @param rhs - the block to shallow copy                               
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   constexpr Block& Block::operator = (S&& rhs) noexcept {
      return operator = (static_cast<const Block&>(rhs.mValue));
   }
   
   /// Semantically transfer the members of one block onto another            
   ///   @attention will not set mType if TO is type-constrained              
   ///   @attention will combine states if TO is type-constrained             
   ///   @tparam TO - the type of block we're transferring to                 
   ///   @tparam S - the semantic to use for the transfer (deducible)         
   ///   @param from - the block and semantic to transfer from                
   template<class TO, CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   void Block::BlockTransfer(S&& from) {
      using Container = TypeOf<S>;
      static_assert(CT::Block<TO>, "TO must be a block type");
      static_assert(CT::Block<Container>, "Container must be a block type");

      mRaw = from.mValue.mRaw;
      mCount = from.mValue.mCount;
      mReserved = from.mValue.mReserved;

      if constexpr (CT::Typed<TO>) {
         // Never touch the type of statically typed blocks             
         // Also, combine states, because state might contain sparsity  
         // and type-constraint                                         
         mState += from.mValue.mState;
      }
      else {
         // Container is not statically typed, so we can safely         
         // overwrite type and state directly                           
         mType = from.mValue.mType;
         mState = from.mValue.mState;
      }

      if constexpr (S::Keep) {
         // Move/Copy other                                             
         mEntry = from.mValue.mEntry;

         if constexpr (S::Move) {
            if constexpr (!Container::Ownership) {
               // Since we are not aware if that block is referenced    
               // or not we reference it just in case, and we also      
               // do not reset 'other' to avoid leaks. When using       
               // raw Blocks, it's your responsibility to take care     
               // of ownership.                                         
               Keep();
            }
            else {
               from.mValue.ResetMemory();
               from.mValue.ResetState();
            }
         }
         else Keep();
      }
      else if constexpr (S::Move) {
         // Abandon other                                               
         mEntry = from.mValue.mEntry;
         from.mValue.mEntry = nullptr;
      }
   }
   
   /// Swap contents of this block, with the contents of another, using       
   /// a temporary block, completely type-erased and as efficient as possible 
   ///   @attention assumes both containers have same initialized count       
   ///   @attention assumes both containers have same type                    
   ///   @param rhs - the block to swap with                                  
   template<CT::Semantic S>
   void Block::SwapUnknown(S&& rhs) {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");

      LANGULUS_ASSUME(DevAssumes, rhs.mValue.mCount == mCount,
         "Count mismatch");
      LANGULUS_ASSUME(DevAssumes, mCount,
         "Can't swap zero count");
      LANGULUS_ASSUME(DevAssumes, IsExact(rhs.mValue.GetType()),
         "Type mismatch");

      Block temporary {mState, mType};
      temporary.AllocateFresh(temporary.RequestSize(mCount));
      temporary.mCount = mCount;

      // Abandon this to temporary                                      
      temporary.CallUnknownSemanticConstructors(mCount, Abandon(*this));
      // Destroy elements in this                                       
      CallUnknownDestructors();
      // Abandon rhs to this                                            
      CallUnknownSemanticConstructors(rhs.mValue.mCount, rhs.Forward());
      // Destroy elements in rhs                                        
      rhs.mValue.CallUnknownDestructors();
      // Abandon temporary to rhs                                       
      rhs.mValue.CallUnknownSemanticConstructors(temporary.mCount, Abandon(temporary));

      // Cleanup temporary                                              
      temporary.CallUnknownDestructors();
      Inner::Allocator::Deallocate(temporary.mEntry);
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
      LANGULUS_ASSUME(DevAssumes, IsExact<T>() && rhs.IsExact<T>(),
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
      Inner::Allocator::Deallocate(temporary.mEntry);
   }

} // namespace Langulus::Anyness