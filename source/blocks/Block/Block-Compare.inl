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
#include "Block-Indexing.inl"
#include <RTTI/Hash.hpp>

#define VERBOSE(...) //Logger::Verbose(_VA_ARGS_)
#define VERBOSE_TAB(...) //auto tab = Logger::Section(__VA_ARGS__)

namespace Langulus::Anyness
{
   
   /// Compare to any other kind of deep container, or single custom element  
   ///   @tparam T - type to use for comparison (deducible)                   
   ///   @param rhs - element to compare against                              
   ///   @return true if containers match                                     
   template<CT::NotSemantic T>
   bool Block::operator == (const T& rhs) const {
      if constexpr (CT::Deep<T>)
         return Compare(rhs) or CompareSingleValue<T>(rhs);
      else
         return CompareSingleValue<T>(rhs);
   }
   
   /// Compare two block's contents for equality                              
   ///   @tparam RESOLVE - whether or not to resolve each element, before     
   ///                     comparing                                          
   ///   @param right - the memory block to compare against                   
   ///   @return true if the two memory blocks are identical                  
   template<bool RESOLVE>
   bool Block::Compare(const Block& right) const {
      VERBOSE_TAB("Comparing ",
         Logger::Push, Logger::White, mCount, " elements of ", GetToken(),
         Logger::Pop, " with ",
         Logger::Push, Logger::White, right.mCount, " elements of ", right.GetToken()
      );

      if (mCount != right.mCount) {
         // Cheap early return for differently sized blocks             
         VERBOSE(Logger::Red,
            "Data count is different: ", 
            mCount, " != ", right.mCount);
         return false;
      }

      if (mCount and mType != right.mType) {
         if (IsUntyped() or right.IsUntyped()) {
            // Cheap early return if differing undefined types, when    
            // packs are not empty                                      
            VERBOSE(Logger::Red,
               "One of the containers is untyped: ",
               GetToken(), " != ", right.GetToken());
            return false;
         }
      }
      else if (not mCount or IsUntyped()) {
         // Both blocks are untyped or empty, just compare states       
         return CompareStates(right);
      }

      if (not CompareStates(right)) {
         // Cheap early return for blocks of differing states           
         VERBOSE(Logger::Red,
            "Data states are not compatible");
         return false;
      }

      if (mType->IsExact(right.mType)) {
         // Types are exactly the same                                  
         if (mRaw == right.mRaw) {
            // Quickly return if memory is exactly the same             
            VERBOSE(Logger::Green,
               "Blocks are the same ", Logger::Cyan, "(optimal)");
            return true;
         }
         else if (mType->mIsPOD or mType->mIsSparse) {
            // Batch-compare memory if POD or sparse                    
            return 0 == memcmp(mRaw, right.mRaw, GetBytesize());
         }
         else if (mType->mComparer) {
            // Call compare operator for each element pair              
            auto lhs = GetRaw();
            auto rhs = right.GetRaw();
            const auto lhsEnd = GetRawEnd();
            while (lhs != lhsEnd) {
               if (not mType->mComparer(lhs, rhs))
                  return false;
               lhs += mType->mSize;
               rhs += mType->mSize;
            }

            return true;
         }
         else LANGULUS_THROW(Compare, "No == operator reflected");
      }

      // If this is reached, then an advanced comparison commences      
      RTTI::Base baseForComparison {};
      if constexpr (RESOLVE) {
         // We will test type for each resolved element, individually   
         if (not IsResolvable()and not right.IsResolvable() and not CompareTypes(right, baseForComparison)) {
            // Types differ and are not resolvable                      
            VERBOSE(Logger::Red,
               "Data types are not related: ",
               GetToken(), " != ", right.GetToken());
            return false;
         }
      }
      else {
         // We won't be resolving, so we have only one global type      
         if (not CompareTypes(right, baseForComparison)) {
            // Types differ                                             
            VERBOSE(Logger::Red,
               "Data types are not related: ",
               GetToken(), " != ", right.GetToken());
            return false;
         }
      }

      if (  (IsSparse() and baseForComparison.mBinaryCompatible)
         or (baseForComparison.mType->mIsPOD and baseForComparison.mBinaryCompatible)
      ) {
         // Just compare the memory directly (optimization)             
         // Regardless if types are sparse or dense, as long as they    
         // are of the same density, of course                          
         VERBOSE("Batch-comparing POD memory / pointers");
         const auto code = memcmp(mRaw, right.mRaw, GetBytesize());

         if (code != 0) {
            VERBOSE(Logger::Red, "POD/pointers are not the same ", Logger::Yellow, "(fast)");
            return false;
         }
         
         VERBOSE(Logger::Green, "POD/pointers memory is the same ", Logger::Yellow, "(fast)");
      }
      else if (baseForComparison.mType->mComparer) {
         if (IsSparse()) {
            if constexpr (RESOLVE) {
               // Resolve all elements one by one and compare them by   
               // their common resolved base                            
               for (Count i = 0; i < mCount; ++i) {
                  auto lhs = GetElementResolved(i);
                  auto rhs = right.GetElementResolved(i);
                  if (not lhs.CompareTypes(rhs, baseForComparison)) {
                     // Fail comparison on first mismatch               
                     VERBOSE(Logger::Red,
                        "Pointers at ", i, " have unrelated types: ", 
                        lhs.GetToken(), " != ", rhs.GetToken());
                     return false;
                  }

                  // Compare pointers only                              
                  if (lhs.mRaw != rhs.mRaw) {
                     // Fail comparison on first mismatch               
                     VERBOSE(Logger::Red,
                        "Pointers at ", i, " differ: ",
                        lhs.GetToken(), " != ", rhs.GetToken());
                     return false;
                  }
               }

               VERBOSE(Logger::Green,
                  "Data is the same, all pointers match ", 
                  Logger::DarkYellow, "(slow)");
            }
            else {
               // Call the reflected == operator in baseForComparison   
               VERBOSE("Comparing using reflected operator == for ",
                  baseForComparison.mType->mToken);

               for (Count i = 0; i < mCount; ++i) {
                  // Densify and compare all elements by the binary     
                  // compatible base                                    
                  auto lhs = GetElementDense(i);
                  auto rhs = right.GetElementDense(i);

                  if (not lhs.CallComparer(rhs, baseForComparison)) {
                     // Fail comparison on first mismatch               
                     VERBOSE(Logger::Red, "Elements at ", i, " differ");
                     return false;
                  }
               }

               VERBOSE(Logger::Green,
                  "Data is the same, all elements match ", 
                  Logger::Yellow, "(slow)");
            }
         }
         else {
            // Call the reflected == operator in baseForComparison      
            VERBOSE("Comparing using reflected operator == for ",
               baseForComparison.mType->mToken);

            for (Count i = 0; i < mCount; ++i) {
               // Compare all elements by the binary compatible base    
               auto lhs = GetElement(i);
               auto rhs = right.GetElement(i);

               if (not lhs.CallComparer(rhs, baseForComparison)) {
                  // Fail comparison on first mismatch                  
                  VERBOSE(Logger::Red, "Elements at ", i, " differ");
                  return false;
               }
            }

            VERBOSE(Logger::Green,
               "Data is the same, all elements match ", 
               Logger::Yellow, "(slow)");
         }
      }
      else {
         VERBOSE(Logger::Red,
            "Can't compare related types because no == operator is reflected, "
            "and they're not POD - common base for comparison was: ",
            baseForComparison.mType->mToken);
         return false;
      }

      return true;
   }
   
   /// Hash data inside memory block                                          
   ///   @attention order matters, so you might want to Neat data first       
   ///   @return the hash                                                     
   inline Hash Block::GetHash() const {
      if (not mType or not mCount)
         return {};

      if (mCount == 1) {
         // Exactly one element means exactly one hash                  
         if (mType->mIsSparse) {
            if (*GetRawSparse() == nullptr)
               return {};

         #if LANGULUS_FEATURE(MANAGED_REFLECTION)
            // When reflection is managed, meta pointer can be used     
            // instead of the hash - this speeds up hashing             
            return HashOf(*GetRawSparse());
         #else
            // Otherwise always dereference the metas and get their hash
            // as there may be many meta instances scattered in memory  
            try { return As<RTTI::Meta>().GetHash(); }
            catch (...) { }
            return HashOf(*GetRawSparse());
         #endif
         }
         else if (mType->Is<Hash>())
            return Get<Hash>();
         else if (mType->mHasher)
            return mType->mHasher(mRaw);
         else if (mType->mIsPOD)
            return HashBytes(mRaw, static_cast<int>(mType->mSize));
         else
            LANGULUS_OOPS(Access, "Unhashable type", ": ", GetToken());
      }

      // Hashing multiple elements                                      
      if (mType->mIsSparse) {
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         // When reflection is managed, meta pointer can be used        
         // instead of the hash - this speeds up hashing in containers  
         return HashBytes(mRaw, static_cast<int>(GetBytesize()));
      #else
         // Otherwise always dereference the metas and get their hash   
         // as there may be many meta instances scattered in memory     
         if (mType->CastsTo<RTTI::Meta>()) {
            auto h = Block::From<Hash, true>();
            h.AllocateFresh(h.RequestSize(mCount));

            for (Count i = 0; i < mCount; ++i)
               h.InsertInner(Abandon(As<RTTI::Meta>(i).GetHash()), i);

            const auto result = HashBytes<DefaultHashSeed, false>(
               h.GetRaw(), static_cast<int>(h.GetBytesize()));

            h.Free();
            return result;
         }
         else return HashBytes(mRaw, static_cast<int>(GetBytesize()));
      #endif
      }
      else if (mType->mIsPOD and not mType->mHasher) {
         // Hash all PODs at once                                       
         return HashBytes(mRaw, static_cast<int>(GetBytesize()));
      }
      else if (mType->mHasher) {
         // Use the reflected hasher for each element, and then combine 
         // hashes for a final one                                      
         auto h = Block::From<Hash, true>();
         h.AllocateFresh(h.RequestSize(mCount));

         for (Count i = 0; i < mCount; ++i) {
            const auto element = GetElement(i);
            h.InsertInner(Abandon(mType->mHasher(element.mRaw)), i);
         }

         const auto result = HashBytes<DefaultHashSeed, false>(
            h.GetRaw(), static_cast<int>(h.GetBytesize()));

         h.Free();
         return result;
      }
      else LANGULUS_OOPS(Access, "Unhashable type", ": ", GetToken());
      return {};
   }
   
   /// Find first matching element position inside container                  
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @tparam T - type to use for comparison (deducible)                   
   ///   @param item - the item to search for                                 
   ///   @param cookie - continue search from a given offset                  
   ///   @return the index of the found item, or IndexNone if not found       
   template<bool REVERSE, CT::NotSemantic T>
   Index Block::FindKnown(const T& item, const Offset& cookie) const {
      // First check if element is contained inside this block's        
      // memory, because if so, we can easily find it, without calling  
      // a single compare function                                      
      if constexpr (CT::Deep<T>) {
         // Deep items have a bit looser type requirements              
         if (IsDense() and IsDeep() and Owns(&item)) {
            const Offset index = &item - GetRawAs<T>();
            // Check is required, because Owns tests in reserved range  
            return index < mCount ? index : IndexNone;
         }
      }
      else {
         // Search for a conventional item                              
         if (IsExact<T>() and Owns(&item)) {
            const Offset index = &item - GetRawAs<T>();
            // Check is required, because Owns tests in reserved range  
            return index < mCount ? index : IndexNone;
         }
      }

      // Item is not in this block's memory, so we start comparing by   
      // values                                                         
      Offset i = REVERSE ? mCount - 1 - cookie : cookie;
      while (i < mCount) {
         if (GetElement(i) == item) {
            // Match found                                              
            return i;
         }

         if constexpr (REVERSE)
            --i;
         else
            ++i;
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }
   
   /// Find first matching element position inside container                  
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - block with a single item to search for                 
   ///   @param cookie - continue search from a given offset                  
   ///   @return the index of the found item, or IndexNone if not found       
   template<bool REVERSE>
   Index Block::FindUnknown(const Block& item, const Offset& cookie) const {
      // First check if element is contained inside this block's        
      // memory, because if so, we can easily find it, without calling  
      // a single compare function                                      
      if (item.IsDense() and item.IsDeep()) {
         // Deep items have a bit looser type requirements              
         if (IsDense() and IsDeep() and Owns(item.mRaw)) {
            const Offset index = (item.mRaw - mRaw) / sizeof(Block);
            // Check is required, because Owns tests in reserved range  
            return index < mCount ? index : IndexNone;
         }
      }
      else {
         // Search for a conventional item                              
         if (IsExact(item.GetType()) and Owns(item.mRaw)) {
            const Offset index = (item.mRaw - mRaw) / mType->mSize;
            // Check is required, because Owns tests in reserved range  
            return index < mCount ? index : IndexNone;
         }
      }

      // Item is not in this block's memory, so we start comparing by   
      // values                                                         
      Offset i = REVERSE ? mCount - 1 - cookie : cookie;
      while (i < mCount) {
         if (GetElement(i) == item.GetElement(i)) {
            // Match found                                              
            return i;
         }

         if constexpr (REVERSE)
            --i;
         else
            ++i;
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }

   /// Find first matching element position inside container, deeply          
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @tparam T - type to use for comparison (deducible)                   
   ///   @param item - the item to search for                                 
   ///   @param cookie - continue search from a given offset                  
   ///   @return the index of the found item, or IndexNone if not found       
   template<bool REVERSE, CT::NotSemantic T>
   Index Block::FindDeep(const T& item, Offset cookie) const {
      Index found;
      ForEachDeep<REVERSE>([&](const Block& group) {
         if (cookie) {
            --cookie;
            return Flow::Continue;
         }

         found = group.template FindKnown<REVERSE>(item);
         return !found;
      });

      return found;
   }

   /// Compare with one single value, if exactly one element is contained     
   ///   @param rhs - the value to compare against                            
   ///   @return true if elements are the same                                
   template<class T>
   LANGULUS(INLINED)
   bool Block::CompareSingleValue(const T& rhs) const {
      if (mCount != 1 or IsUntyped())
         return false;

      if constexpr (CT::Deep<T>) {
         // Deep types can be more loosely compared                     
         if (mType->mIsSparse or not mType->mIsDeep)
            return false;
         return GetRawAs<Block>()->Compare(rhs);
      }
      else {
         // Non-deep element compare                                    
         static_assert(CT::Inner::Comparable<T>,
            "T is not equality-comparable");

         if (not mType->template IsExact<T>())
            return false;
         return *GetRawAs<T>() == rhs;
      }
   }
   
   /// Compare the relevant states of two blocks                              
   ///   @param rhs - the memory block to compare against                     
   ///   @return true if the two memory blocks' revelant states are identical 
   LANGULUS(INLINED)
   bool Block::CompareStates(const Block& rhs) const noexcept {
      return GetUnconstrainedState() == rhs.GetUnconstrainedState();
   }

   /// Compare types of two blocks, and a produce a common type whose         
   /// comparison function to use                                             
   ///   @attention assumes that both blocks are typed                        
   ///   @param right - the type to the right                                 
   ///   @param common - [out] the common base                                
   ///   @return true if a common base has been found                         
   inline bool Block::CompareTypes(const Block& right, RTTI::Base& common) const {
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "LHS block is not typed", " comparing with RHS: ", right.GetType());
      LANGULUS_ASSUME(DevAssumes, right.IsTyped(),
         "RHS block is not typed", " comparing with LHS: ", GetType());

      if (not mType->Is(right.mType)) {
         // Types differ, dig deeper to find out why                    
         if (not mType->GetBase(right.mType, 0, common)) {
            // Other type is not base for this one, can't compare them  
            // Let's check in reverse                                   
            if (not right.mType->GetBase(mType, 0, common)) {
               // Other type is not derived from this one, can't        
               // compare them                                          
               return false;
            }

            // Other is derived from this, but it has to be binary      
            // compatible to be able to compare them                    
            if (not common.mBinaryCompatible) {
               VERBOSE(Logger::Red,
                  "Data types are related, but not binary compatible: ",
                  GetToken(), " != ", right.GetToken());
               return false;
            }

            return true;
         }
         else {
            // This is derived from other, but it has to be binary      
            // compatible to be able to compare them                    
            if (not common.mBinaryCompatible) {
               VERBOSE(Logger::Red,
                  "Data types are related, but not binary compatible: ",
                  GetToken(), " != ", right.GetToken());
               return false;
            }

            return true;
         }
      }
      else {
         // Types match exactly, or their origins match exactly         
         if (mType->mOrigin) {
            common.mType = mType->mOrigin;
            common.mBinaryCompatible = true;
            return true;
         }
         else {
            // Types match exactly, but we're interested only in an     
            // origin type base. Unfortunately, the type is incomplete, 
            // so comparison is not possible                            
            return false;
         }
      }
   }

   /// Invoke a comparator in base, comparing this block against right one    
   ///   @param right - the right block                                       
   ///   @param base - the base to use                                        
   ///   @return true if comparison returns true                              
   LANGULUS(INLINED)
   bool Block::CallComparer(const Block& right, const RTTI::Base& base) const {
      return mRaw == right.mRaw 
          or (mRaw and right.mRaw and base.mType->mComparer(mRaw, right.mRaw));
   }

   /// Gather items from input container, and fill output                     
   /// Output type acts as a filter to what gets gathered                     
   ///   @tparam REVERSE - are we gathering in reverse?                       
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @return the number of gathered elements                              
   template<bool REVERSE>
   Count Block::GatherInner(const Block& input, CT::Data auto& output) {
      static_assert(CT::Block<decltype(output)>,
         "Output must be a block type");

      Count count {};
      if (input.IsDeep() and not output.IsDeep()) {
         ForEach<REVERSE>([&](const Block& i) {
            count += GatherInner<REVERSE>(i, output);
         });
         return count;
      }

      if (output.IsConcatable(input)) {
         // Catenate input if compatible                                
         count += output.template InsertBlock<IndexBack>(input);
      }

      return count;
   }

   /// Gather items of specific phase from input container and fill output    
   ///   @tparam REVERSE - are we gathering in reverse?                       
   ///   @param type - type to search for                                     
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @param state - the data state filter                                 
   ///   @return the number of gathered elements                              
   template<bool REVERSE>
   Count Block::GatherPolarInner(DMeta type, const Block& input, CT::Data auto& output, DataState state) {
      static_assert(CT::Block<decltype(output)>,
         "Output must be a block type");

      if (input.GetState() % state) {
         if (input.IsNow() and input.IsDeep()) {
            // Phases don't match, but we can dig deeper if deep        
            // and neutral, since Phase::Now is permissive              
            Block localOutput {input.GetUnconstrainedState(), type};
            ForEach<REVERSE>([&](const Block& i) {
               GatherPolarInner<REVERSE>(type, i, localOutput, state);
            });
            localOutput.MakeNow();
            const auto inserted = output.SmartPush(Abandon(localOutput));
            localOutput.Free();
            return inserted;
         }

         // Polarity mismatch                                           
         return 0;
      }

      // Input is flat and neutral/same                                 
      if (not type) {
         // Output is any, so no need to iterate                        
         return output.SmartPush(input);
      }

      // Iterate subpacks if any                                        
      Block localOutput {input.GetState(), type};
      GatherInner<REVERSE>(input, localOutput);
      localOutput.MakeNow();
      const auto inserted = output.InsertBlock(Abandon(localOutput));
      localOutput.Free();
      return inserted;
   }

} // namespace Langulus::Anyness

#undef VERBOSE_TAB
#undef VERBOSE