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

   /// Manual construction from mutable data                                  
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count)
      IF_UNSAFE(noexcept)
      : Block {
         state + DataState::Member,
         meta, count, (void*) nullptr, nullptr
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
   
   /// Construct manually by interfacing memory directly                      
   /// Data will only be interfaced, never owned or copied, so semantic is    
   /// ignored, but left out for compatiblity with other interfaces           
   ///   @tparam CONSTRAIN_TYPE - whether to make the resulting Block         
   ///      type-constrained                                                  
   ///   @param what - data to interface                                      
   ///   @param count - number of items, in case 'what' is sparse             
   ///   @return the provided data, wrapped inside a Block                    
   template<bool CONSTRAIN_TYPE> LANGULUS(INLINED)
   Block Block::From(auto&& what, Count count) {
      using S = SemanticOf<decltype(what)>;
      using ST = TypeOf<S>;

      Block result;
      if constexpr (CT::Array<ST>) {
         // ... from a bounded array                                    
         count *= ExtentOf<ST>;
         result.SetMemory(
            DataState::Constrained,
            MetaDataOf<Deext<ST>>(), count,
            DesemCast(what), nullptr
         );
      }
      else if constexpr (CT::Sparse<ST>) {
         // ... from a pointer                                          
         result.SetMemory(
            DataState::Constrained,
            MetaDataOf<Deptr<ST>>(), count,
            DesemCast(what), nullptr
         );
      }
      else {
         // ... from a value                                            
         if constexpr (CT::Resolvable<ST>) {
            // Resolve a runtime-resolvable value                       
            result = DesemCast(what).GetBlock();
         }
         else if constexpr (CT::Deep<ST>) {
            // Static cast to Block if CT::Deep                         
            result.operator = (DesemCast(what));
         }
         else {
            // Any other value gets wrapped inside a temporary Block    
            result.SetMemory(
               DataState::Constrained,
               MetaDataOf<ST>(), 1,
               &DesemCast(what), nullptr
            );
         }
      }

      if constexpr (CONSTRAIN_TYPE)
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
   template<CT::Block TO, template<class> class S, CT::Block FROM>
   requires CT::Semantic<S<FROM>> LANGULUS(INLINED)
   void Block::BlockTransfer(S<FROM>&& from) {
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

      if constexpr (S<FROM>::Shallow) {
         // We're transferring via a shallow semantic                   
         mRaw = from->mRaw;
         mReserved = from->mReserved;

         if constexpr (S<FROM>::Keep) {
            // Move/Copy other                                          
            mEntry = from->mEntry;

            if constexpr (S<FROM>::Move) {
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
         else if constexpr (S<FROM>::Move) {
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
         
         // Pick a preferably typed block to optimize the construction  
         using B = Conditional<CT::Typed<FROM>, FROM, TO>;
         AllocateFresh<B>(RequestSize<B>(mCount));
         CreateSemantic<B>(from.Forward());
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
         UnfoldInsert<false>(0, other.Forward());
   }

} // namespace Langulus::Anyness