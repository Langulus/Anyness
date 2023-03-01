///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block.hpp"

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
         return Compare(rhs) || CompareSingleValue<T>(rhs);
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

      if (mType != right.mType && (IsUntyped() || right.IsUntyped())) {
         // Cheap early return for blocks of differing undefined type   
         VERBOSE(Logger::Red,
            "One of the containers is untyped: ", 
            GetToken(), " != ", right.GetToken());
         return false;
      }

      if (!CompareStates(right)) {
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
         else if (mType->mIsPOD || mType->mIsSparse) {
            // Batch-compare memory if POD or sparse                    
            return 0 == memcmp(mRaw, right.mRaw, GetByteSize());
         }
         else if (mType->mComparer) {
            // Call compare operator for each element pair              
            auto lhs = GetRaw();
            auto rhs = right.GetRaw();
            const auto lhsEnd = GetRawEnd();
            while (lhs != lhsEnd) {
               if (!mType->mComparer(lhs, rhs))
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
         if (!IsResolvable() && !right.IsResolvable() && !CompareTypes(right, baseForComparison)) {
            // Types differ and are not resolvable                      
            VERBOSE(Logger::Red,
               "Data types are not related: ",
               GetToken(), " != ", right.GetToken());
            return false;
         }
      }
      else {
         // We won't be resolving, so we have only one global type      
         if (!CompareTypes(right, baseForComparison)) {
            // Types differ                                             
            VERBOSE(Logger::Red,
               "Data types are not related: ",
               GetToken(), " != ", right.GetToken());
            return false;
         }
      }

      if (  (IsSparse() && baseForComparison.mBinaryCompatible)
         || (baseForComparison.mType->mIsPOD && baseForComparison.mBinaryCompatible)
      ) {
         // Just compare the memory directly (optimization)             
         // Regardless if types are sparse or dense, as long as they    
         // are of the same density, of course                          
         VERBOSE("Batch-comparing POD memory / pointers");
         const auto code = memcmp(mRaw, right.mRaw, GetByteSize());

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
                  if (!lhs.CompareTypes(rhs, baseForComparison)) {
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

                  if (!lhs.CallComparer(rhs, baseForComparison)) {
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

               if (!lhs.CallComparer(rhs, baseForComparison)) {
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
   ///   @attention order matters, so you might want to normalize data first  
   ///   @return the hash                                                     
   inline Hash Block::GetHash() const {
      if (!mType || !mCount)
         return {};

      if (mCount == 1) {
         // Exactly one element means exactly one hash                  
         // This also eliminates asymmetries when getting hash of block 
         // and of templated element equivalents                        
         if (mType->mIsSparse)
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
      if (mType->mIsSparse) {
         auto h = Block::From<Hash, true>();
         h.AllocateFresh(h.RequestSize(mCount));

         for (Count i = 0; i < mCount; ++i)
            h.InsertInner(Abandon(GetElementResolved(i).GetHash()), i);

         const auto result = HashBytes<DefaultHashSeed, false>(
            h.GetRaw(), static_cast<int>(h.GetByteSize())
         );

         h.Free();
         return result;
      }
      else if (mType->mHasher) {
         auto h = Block::From<Hash, true>();
         h.AllocateFresh(h.RequestSize(mCount));

         for (Count i = 0; i < mCount; ++i) {
            const auto element = GetElement(i);
            h.InsertInner(Abandon(mType->mHasher(element.mRaw)), i);
         }

         const auto result = HashBytes<DefaultHashSeed, false>(
            h.GetRaw(), static_cast<int>(h.GetByteSize()));

         h.Free();
         return result;
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
         if (IsDense() && IsDeep() && Owns(&item)) {
            const Offset index = &item - GetRawAs<T>();
            // Check is required, because Owns tests in reserved range  
            return index < mCount ? index : IndexNone;
         }
      }
      else {
         // Search for a conventional item                              
         if (IsExact<T>() && Owns(&item)) {
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
      if (item.IsDense() && item.IsDeep()) {
         // Deep items have a bit looser type requirements              
         if (IsDense() && IsDeep() && Owns(item.mRaw)) {
            const Offset index = (item.mRaw - mRaw) / sizeof(Block);
            // Check is required, because Owns tests in reserved range  
            return index < mCount ? index : IndexNone;
         }
      }
      else {
         // Search for a conventional item                              
         if (IsExact(item.GetType()) && Owns(item.mRaw)) {
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
            return true;
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
   LANGULUS(ALWAYSINLINE)
   bool Block::CompareSingleValue(const T& rhs) const {
      if (mCount != 1 || IsUntyped())
         return false;

      if constexpr (CT::Deep<T>) {
         // Deep types can be more loosely compared                     
         if (mType->mIsSparse || !mType->mIsDeep)
            return false;
         return GetRawAs<Block>()->Compare(rhs);
      }
      else {
         // Non-deep element compare                                    
         static_assert(CT::Sparse<T> || CT::Comparable<T, T>,
            "T must either be pointer, or has a reflected == operator");
         if (!mType->template IsExact<T>())
            return false;
         return *GetRawAs<T>() == rhs;
      }
   }
   
   /// Compare the relevant states of two blocks                              
   ///   @param rhs - the memory block to compare against                     
   ///   @return true if the two memory blocks' revelant states are identical 
   LANGULUS(ALWAYSINLINE)
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
      LANGULUS_ASSUME(DevAssumes, IsTyped(), "LHS block is not typed");
      LANGULUS_ASSUME(DevAssumes, right.IsTyped(), "RHS block is not typed");

      if (!mType->Is(right.mType)) {
         // Types differ, dig deeper to find out why                    
         if (!mType->GetBase(right.mType, 0, common)) {
            // Other type is not base for this one, can't compare them  
            // Let's check in reverse                                   
            if (!right.mType->GetBase(mType, 0, common)) {
               // Other type is not derived from this one, can't        
               // compare them                                          
               return false;
            }

            // Other is derived from this, but it has to be binary      
            // compatible to be able to compare them                    
            if (!common.mBinaryCompatible) {
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
            if (!common.mBinaryCompatible) {
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
   LANGULUS(ALWAYSINLINE)
   bool Block::CallComparer(const Block& right, const RTTI::Base& base) const {
      return mRaw == right.mRaw || (
         mRaw && right.mRaw && base.mType->mComparer(mRaw, right.mRaw)
      );
   }

   /// Gather items from input container, and fill output                     
   /// Output type acts as a filter to what gets gathered                     
   ///   @tparam REVERSE - are we gathering in reverse?                       
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @return the number of gathered elements                              
   template<bool REVERSE>
   Count Block::GatherInner(const Block& input, Block& output) {
      Count count {};
      if (input.IsDeep() && !output.IsDeep()) {
         ForEach<REVERSE>([&](const Block& i) {
            count += GatherInner<REVERSE>(i, output);
         });
         return count;
      }

      if (output.IsConcatable(input)) {
         // Catenate input if compatible                                
         count += output.InsertBlock<IndexBack>(input);
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
   Count Block::GatherPolarInner(DMeta type, const Block& input, Block& output, DataState state) {
      if (input.GetState() % state) {
         if (input.IsNow() && input.IsDeep()) {
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
      if (!type) {
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