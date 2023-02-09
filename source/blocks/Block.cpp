///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Block.hpp"
#include "../TAny.hpp"

namespace Langulus::Anyness
{

   /// Clone all elements inside this memory block, preserving hierarchy and  
   /// density, but removing size constraints and constness                   
   /// If we already have jurisdiction, then nothing happens                  
   void Block::TakeAuthority() {
      if (mEntry) {
         // We already own this memory, don't touch anything            
         return;
      }

      // Clone everything and overwrite this block                      
      // At the end it, and all its subelements should have exactly one 
      // reference                                                      
      Block clone;
      Clone(clone);
      Free();
      operator = (clone);
   }

   /// Get the memory block corresponding to a base (constant)                
   ///   @param meta - the descriptor to scan for a base                      
   ///   @param base - the base to search for                                 
   ///   @return the block for the base (static and immutable)                
   Block Block::GetBaseMemory(DMeta meta, const RTTI::Base& base) const {
      if (IsEmpty())
         return {};

      if (base.mBinaryCompatible) {
         return {
            DataState::ConstantMember, meta,
            GetCount() * base.mCount,
            Get<Byte*>(), mEntry
         };
      }

      return {
         DataState::ConstantMember, meta, 1,
         Get<Byte*>(0, base.mOffset), mEntry
      };
   }

   /// Get the memory block corresponding to a base                           
   ///   @param meta - the descriptor to scan for a base                      
   ///   @param base - the base to search for                                 
   ///   @return the block for the base (static and immutable)                
   Block Block::GetBaseMemory(DMeta meta, const RTTI::Base& base) {
      if (IsEmpty())
         return {};

      if (base.mBinaryCompatible) {
         return {
            DataState::Member, meta,
            GetCount() * base.mCount, 
            Get<Byte*>(), mEntry
         };
      }

      return {
         DataState::Member, meta, 1,
         Get<Byte*>(0, base.mOffset), mEntry
      };
   }

   /// Get the constant memory block corresponding to a base                  
   /// This performs only pointer arithmetic                                  
   ///   @param base - the base to search for                                 
   ///   @return the block for the base (static and immutable)                
   Block Block::GetBaseMemory(const RTTI::Base& base) const {
      return GetBaseMemory(base.mType, base);
   }

   /// Get the mutable memory block corresponding to a base                   
   /// This performs only pointer arithmetic                                  
   ///   @param base - the base to search for                                 
   ///   @return the block for the base (static but mutable)                  
   Block Block::GetBaseMemory(const RTTI::Base& base) {
      return GetBaseMemory(base.mType, base);
   }

   /// Hash data inside memory block                                          
   ///   @return the hash                                                     
   Hash Block::GetHash() const {
      if (!mType || !mCount)
         return {};

      if (mCount == 1) {
         // Exactly one element means exactly one hash                  
         // This also eliminates asymmetries when getting hash of block 
         // and of templated element equivalents                        
         if (IsSparse())
            return GetElementResolved(0).GetHash();
         else if (mType->mHasher)
            return mType->mHasher(mRaw);
         else if (mType->mIsPOD)
            return HashBytes(mRaw, static_cast<int>(mType->mSize));
         else {
            Logger::Error("Unhashable type ", GetToken());
            LANGULUS_THROW(Access, "Unhashable type");
         }
      }

      // Hashing multiple elements one by one, and then rehash all      
      // the combined hashes                                            
      if (IsSparse()) {
         TAny<Hash> h;
         h.AllocateFresh(h.RequestSize(mCount));
         for (Count i = 0; i < mCount; ++i)
            h.InsertInner(Abandon(GetElementResolved(i).GetHash()), i);

         return HashBytes<DefaultHashSeed, false>(
            h.GetRaw(), static_cast<int>(h.GetByteSize()));
      }
      else if (mType->mHasher) {
         TAny<Hash> h;
         h.AllocateFresh(h.RequestSize(mCount));
         for (Count i = 0; i < mCount; ++i) {
            const auto element = GetElement(i);
            h.InsertInner(Abandon(mType->mHasher(element.mRaw)), i);
         }
         return HashBytes<DefaultHashSeed, false>(
            h.GetRaw(), static_cast<int>(h.GetByteSize()));
      }
      else if (mType->mIsPOD) {
         // POD data is an exception - just batch-hash it               
         return HashBytes(mRaw, static_cast<int>(GetByteSize()));
      }
      else {
         Logger::Error("Unhashable type ", GetToken());
         LANGULUS_THROW(Access, "Unhashable type");
      }
   }

   /// Reinterpret contents of this Block as the type and state of another    
   /// You can interpret Vec4 as float[4] for example, or any other such      
   /// reinterpretation, as long as data remains tightly packed               
   ///   @param pattern - the type of data to try interpreting as             
   ///   @return a block representing this block, interpreted as the pattern  
   Block Block::ReinterpretAs(const Block& pattern) const {
      if (IsEmpty())
         return {};

      RTTI::Base common {};
      if (!CompareTypes(pattern, common) || !common.mBinaryCompatible)
         return {};

      const auto baseBytes = (common.mType->mSize * common.mCount)
         / pattern.GetStride();

      if (pattern.IsEmpty()) {
         return {
            pattern.mState + DataState::Static, pattern.mType,
            baseBytes,
            mRaw, mEntry
         };
      }
      else {
         return {
            pattern.mState + DataState::Static, pattern.mType,
            (baseBytes / pattern.mCount) * pattern.mCount,
            mRaw, mEntry
         };
      }
   }

   /// Get next element by incrementing data pointer (for inner use)          
   void Block::Next() noexcept {
      mRaw += GetStride();
   }

   /// Get previous element by decrementing data pointer (for inner use)      
   void Block::Prev() noexcept {
      mRaw -= GetStride();
   }

   /// Get next element by incrementing data pointer (for inner use)          
   ///   @return a new block with the incremented pointer                     
   Block Block::Next() const noexcept {
      return {mState, mType, mCount, mRaw + GetStride(), mEntry};
   }

   /// Get previous element by decrementing data pointer (for inner use)      
   ///   @return a new block with the decremented pointer                     
   Block Block::Prev() const noexcept {
      return {mState, mType, mCount, mRaw - GetStride(), mEntry};
   }

   /// Remove elements on the back                                            
   ///   @param count - the new count                                         
   ///   @return a reference to this block                                    
   Block& Block::Trim(const Count count) {
      if (count >= mCount)
         return *this;

      RemoveIndex(count, mCount - count);
      return *this;
   }

   /// Gather items from input container, and fill output                     
   /// Output type acts as a filter to what gets gathered                     
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @param direction - the direction to search from                      
   ///   @return the number of gathered elements                              
   Count Block::GatherInner(const Block& input, Block& output, const Index direction) {
      Count count {};
      if (input.IsDeep() && !output.IsDeep()) {
         // Iterate all subpacks                                        
         if (direction == IndexFront) {
            for (Count i = 0; i < input.GetCount(); ++i) {
               count += GatherInner(input.As<Block>(i), output, direction);
            }
         }
         else {
            for (Count i = input.GetCount(); i > 0; --i) {
               count += GatherInner(input.As<Block>(i - 1), output, direction);
            }
         }

         return count;
      }

      if (output.IsConcatable(input)) {
         // Catenate input if compatible                                
         count += output.InsertBlock<IndexBack>(input);
      }

      return count;
   }

   /// Gather items of specific phase from input container and fill output    
   ///   @param type - type to search for                                     
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @param direction - the direction to search from                      
   ///   @param phase - phase filter                                          
   ///   @return the number of gathered elements                              
   Count Block::GatherPolarInner(DMeta type, const Block& input, Block& output, const Index direction, DataState state) {
      if (input.GetState() % state) {
         if (input.IsNow() && input.IsDeep()) {
            // Phases don't match, but we can dig deeper if deep        
            // and neutral, since Phase::Now is permissive              
            auto localOutput = Any::FromMeta(type, input.GetUnconstrainedState());
            if (direction == IndexFront) {
               for (Count i = 0; i < input.GetCount(); ++i) {
                  GatherPolarInner(type, input.As<Block>(i),
                     localOutput, direction, state);
               }
            }
            else {
               for (Count i = input.GetCount(); i > 0; --i) {
                  GatherPolarInner(type, input.As<Block>(i - 1),
                     localOutput, direction, state);
               }
            }

            localOutput.MakeNow();
            return output.SmartPush(Abandon(localOutput));
         }

         // Polarity mismatch                                           
         return 0;
      }

      // Input is flat and neutral/same                                 
      if (!type) {
         // Output is any, so no need to iterate                        
         return output.SmartPush(Any {input});
      }

      // Iterate subpacks if any                                        
      auto localOutput = Any::FromMeta(type, input.GetState());
      GatherInner(input, localOutput, direction);
      localOutput.MakeNow();
      return output.InsertBlock(localOutput);
   }

   /// Destroy all elements, but don't deallocate memory                      
   void Block::Clear() {
      if (!mEntry) {
         // Data is either static or unallocated                        
         // Don't call destructors, just clear it up                    
         mRaw = nullptr;
         mCount = mReserved = 0;
         return;
      }

      if (mEntry->GetUses() == 1) {
         // Destroy all elements but don't deallocate the entry         
         CallUnknownDestructors();
         mCount = 0;
         return;
      }
      
      // If reached, then data is referenced from multiple places       
      // Don't call destructors, just clear it up and dereference       
      mEntry->Free();
      mRaw = nullptr;
      mEntry = nullptr;
      mCount = mReserved = 0;
   }

   /// Destroy all elements, deallocate block and reset state                 
   void Block::Reset() {
      Free();
      ResetMemory();
      ResetState();
   }

   /// Flattens unnecessarily deep containers and combines their states       
   /// when possible. Discards ORness if container has only one element       
   void Block::Optimize() {
      if (IsOr() && GetCount() == 1)
         MakeAnd();

      while (GetCount() == 1 && IsDeep()) {
         auto& subPack = As<Block>();
         if (!CanFitState(subPack)) {
            subPack.Optimize();
            if (subPack.IsEmpty())
               Reset();
            return;
         }

         Block temporary {subPack};
         subPack.ResetMemory();
         Free();
         *this = temporary;
      }

      if (GetCount() > 1 && IsDeep()) {
         for (Count i = 0; i < mCount; ++i) {
            auto& subBlock = As<Block>(i);
            subBlock.Optimize();
            if (subBlock.IsEmpty()) {
               RemoveIndex(i);
               --i;
            }
         }
      }
   }

} // namespace Langulus::Anyness
