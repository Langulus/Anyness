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
      : mType {meta}
      , mState {state} { }
   
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
   Block::Block(const DataState& state, DMeta meta, Count count, void* raw) IF_UNSAFE(noexcept)
      : Block {state, meta, count, raw, Allocator::Find(meta, raw)} {}

   /// Manual construction from mutable data                                  
   ///   @attention assumes data is not sparse                                
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count) IF_UNSAFE(noexcept)
      : Block {state, meta, count, (void*) nullptr, nullptr} {}
   
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
   ///   @attention assumes 'meta' is not sparse, if 'raw' is provided        
   ///   @attention assumes a valid 'raw' pointer provided, if 'entry' is     
   ///   @attention assumes 'meta' is always valid                            
   ///   @param state - the initial state of the container                    
   ///   @param meta - the type of the memory block                           
   ///   @param count - initial element count and reserve                     
   ///   @param raw - pointer to the mutable memory                           
   ///   @param entry - the memory entry                                      
   LANGULUS(INLINED)
   Block::Block(const DataState& state, DMeta meta, Count count, void* raw, const Allocation* entry)
   IF_UNSAFE(noexcept)
      : mRaw {static_cast<Byte*>(raw)}
      , mCount {count}
      , mReserved {count}
      , mType {meta}
      , mEntry {entry}
      , mState {state} {
      LANGULUS_ASSUME(DevAssumes, not entry or raw,
         "Invalid data pointer");
      LANGULUS_ASSUME(DevAssumes, meta,
         "Invalid data type");
      LANGULUS_ASSUME(DevAssumes, not raw or not meta->mIsSparse or not mEntry,
         "Sparse raw data initialization is not allowed, "
         "unless mEntry is zero");

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
   template<class TYPE> template<bool CONSTRAIN_TYPE> LANGULUS(INLINED)
   Block<TYPE> Block<TYPE>::From(auto&& what, Count count) requires TypeErased {
      using S  = SemanticOf<decltype(what)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Array<ST>) {
         // ... from a bounded array                                    
         count *= ExtentOf<ST>;
         return {
            DataState::Constrained,
            MetaDataOf<Deext<ST>>(), count,
            DesemCast(what), nullptr
         };
      }
      else if constexpr (CT::Sparse<ST>) {
         // ... from a pointer                                          
         return {
            DataState::Constrained,
            MetaDataOf<Deptr<ST>>(), count,
            DesemCast(what), nullptr
         };
      }
      else {
         // ... from a value                                            
         if constexpr (CT::Resolvable<ST>) {
            // Resolve a runtime-resolvable value                       
            Block result = DesemCast(what).GetBlock();
            if constexpr (CONSTRAIN_TYPE)
               result.MakeTypeConstrained();
            return result;
         }
         else if constexpr (CT::Deep<ST>) {
            // Static cast to Block if CT::Deep                         
            Block result = DesemCast(what);
            if constexpr (CONSTRAIN_TYPE)
               result.MakeTypeConstrained();
            return result;
         }
         else {
            // Any other value gets wrapped inside a temporary Block    
            return {
               DataState::Constrained,
               MetaDataOf<ST>(), 1,
               &DesemCast(what), nullptr
            };
         }
      }
   }

   /// Create an empty typed block                                            
   ///   @tparam T - the type of the container                                
   ///   @tparam CONSTRAIN - makes container type-constrained                 
   ///   @return the block                                                    
   template<class TYPE> template<CT::Data T, bool CONSTRAIN> LANGULUS(INLINED)
   Block<TYPE> Block<TYPE>::From() requires TypeErased {
      if constexpr (CONSTRAIN)
         return {DataState::Typed, MetaDataOf<T>()};
      else
         return {MetaDataOf<T>()};
   }

   /// Insert the provided elements, making sure to insert, and never absorb  
   ///   @tparam AS - the type to wrap elements as, use void to auto-deduce   
   ///   @param t1 - first element                                            
   ///   @param tn... - the rest of the elements (optional)                   
   ///   @returns the new container containing the data                       
   template<class TYPE> template<class AS, CT::Data T1, CT::Data...TN>
   LANGULUS(INLINED) auto Block<TYPE>::Wrap(T1&& t1, TN&&...tn) requires TypeErased {
      if constexpr (CT::TypeErased<AS>) {
         using DT1 = Deref<Decvq<Desem<T1>>>;
         if constexpr (sizeof...(TN) > 0) {
            if constexpr (CT::Similar<DT1, DT1, Deref<Desem<TN>>...>) {
               TMany<DT1> result;
               result.Insert(IndexBack, Forward<T1>(t1), Forward<TN>(tn)...);
               return result;
            }
            else {
               TMany<Many> result;
               result.Insert(IndexBack, Forward<T1>(t1), Forward<TN>(tn)...);
               return result;
            }
         }
         else if constexpr (CT::Handle<DT1>) {
            TMany<Decvq<TypeOf<DT1>>> result;
            result.Insert(IndexBack, Forward<T1>(t1));
            return result;
         }
         else {
            TMany<DT1> result;
            result.Insert(IndexBack, Forward<T1>(t1));
            return result;
         }
      }
      else {
         TMany<AS> result;
         result.Insert(IndexBack, Forward<T1>(t1), Forward<TN>(tn)...);
         return result;
      }
   }

   /// Semantically transfer the members of one block onto another with the   
   /// smallest number of instructions possible                               
   ///   @attention will not set mType if TO is type-constrained              
   ///   @attention will not set mRaw, mReserved, mEntry, if 'from' is empty  
   ///   @param from - the block and semantic to transfer from                
   template<class TYPE> template<template<class> class S, CT::Block FROM>
   requires CT::Semantic<S<FROM>> LANGULUS(INLINED)
   void Block<TYPE>::BlockTransfer(S<FROM>&& from) {
      using SS = S<FROM>;

      if constexpr (TypeErased) {
         // We can safely overwrite type and state                      
         mType  = from->mType;
         mState = from->mState;
      }
      else {
         // Block is typed, so we never touch mType, and we make sure   
         // that we don't affect Typed state                            
         mState = from->mState + DataState::Typed;
      }

      if constexpr (SS::Shallow) {
         // Move/Copy/Refer/Abandon/Disown other                        
         if constexpr (SS::Keep) {
            // Move/Copy/Refer other                                    
            if constexpr (SS::Move) {
               // Move                                                  
               mEntry = from->mEntry;
               mRaw = from->mRaw;
               mReserved = from->mReserved;
               mCount = from->mCount;

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
            else {
               // Copy/Refer other                                      
               if constexpr (CT::Referred<SS>) {
                  // Just reference                                     
                  mRaw = from->mRaw;
                  mReserved = from->mReserved;
                  mEntry = from->mEntry;
                  mCount = from->mCount;
                  Keep();
               }
               else {
                  // Do a shallow copy                                  
                  // We're cloning first layer, so we guarantee, that   
                  // data is no longer static and constant (unless      
                  // mType is constant)                                 
                  mState -= DataState::Static | DataState::Constant;
                  if (0 == from->mCount)
                     return;

                  // Pick a preferably typed block to optimize          
                  using B = Conditional<CT::Typed<FROM>, FROM, Block<TYPE>>;
                  if constexpr (CT::Untyped<B>) {
                     // A runtime check is required before allocating            
                     LANGULUS_ASSERT(mType->mReferConstructor, Construct,
                        "Can't refer-construct elements"
                        " - no refer-constructor was reflected for type ", mType);
                  }
                  else {
                     static_assert(CT::ReferMakable<TypeOf<B>>,
                        "Contained type is not refer-constructible");
                  }

                  auto& thisb = reinterpret_cast<B&>(*this);
                  thisb.AllocateFresh(thisb.RequestSize(from->mCount));
                  thisb.CreateSemantic(Refer(from));
                  // This validates elements, do it last in case        
                  // something throws along the way                     
                  mCount = from->mCount;
               }
            }
         }
         else if constexpr (SS::Move) {
            // Abandon                                                  
            mRaw = from->mRaw;
            mReserved = from->mReserved;
            mEntry = from->mEntry;
            mCount = from->mCount;
            from->mEntry = nullptr;
         }
         else {
            // Disown                                                   
            mRaw = from->mRaw;
            mReserved = from->mReserved;
            mCount = from->mCount;
         }
      }
      else {
         // We're cloning, so we guarantee, that data is no longer      
         // static and constant (unless mType is constant)              
         mState -= DataState::Static | DataState::Constant;
         if (0 == from->mCount)
            return;
         
         // Pick a preferably typed block to optimize the construction  
         using B = Conditional<CT::Typed<FROM>, FROM, Block<TYPE>>;
         if constexpr (CT::Untyped<B>) {
            // A runtime check is required before allocating            
            LANGULUS_ASSERT(mType->mCloneConstructor, Construct,
               "Can't clone-construct elements"
               " - no clone-constructor was reflected for type ", mType);
         }
         else {
            static_assert(CT::CloneMakable<TypeOf<B>>,
               "Contained type is not clone-constructible");
         }

         auto& thisb = reinterpret_cast<B&>(*this);
         thisb.AllocateFresh(thisb.RequestSize(from->mCount));
         thisb.CreateSemantic(from.Forward());

         // This validates elements, do it last in case something throw 
         mCount = from->mCount;
      }
   }

} // namespace Langulus::Anyness