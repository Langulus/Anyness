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

#define VERBOSE(...)     //Logger::Verbose(_VA_ARGS_)
#define VERBOSE_TAB(...) //auto tab = Logger::Section(__VA_ARGS__)


namespace Langulus::Anyness
{
   
   /// Compare to any other kind of deep container, or single custom element  
   ///   @param rhs - element to compare against                              
   ///   @return true if containers match                                     
   template<CT::Block THIS>
   bool Block::operator == (const CT::NotSemantic auto& rhs) const {
      if constexpr (CT::Deep<decltype(rhs)>)
         return Compare<true, THIS>(rhs) or CompareSingleValue<THIS>(rhs);
      else
         return CompareSingleValue<THIS>(rhs);
   }
   
   /// Compare two block's contents for equality                              
   ///   @tparam RESOLVE - whether or not to resolve each element, before     
   ///                     comparing                                          
   ///   @param right - the memory block to compare against                   
   ///   @return true if the two memory blocks are identical                  
   template<bool RESOLVE, CT::Block THIS>
   bool Block::Compare(const CT::Block auto& right) const {
      using RHS = Deref<decltype(right)>;
      VERBOSE_TAB("Comparing ",
         Logger::Push, Logger::White, mCount, " elements of ", GetToken(),
         Logger::Pop, " with ",
         Logger::Push, Logger::White, right.mCount, " elements of ", right.GetToken()
      );

      if constexpr (CT::Typed<THIS> and CT::Typed<RHS>) {
         // Both blocks are statically typed - leverage it              
         using T = TypeOf<THIS>;
         if constexpr (not CT::Similar<T, TypeOf<RHS>>) {
            // Types are different                                      
            return false;
         }
         else {
            // Types are similar                                        
            if (mRaw == right.mRaw)
               return mCount == right.mCount;
            else if (mCount != right.mCount)
               return false;

            if constexpr (CT::Sparse<T> or CT::POD<T>) {
               // Batch compare pods or pointers                        
               return 0 == ::std::memcmp(mRaw, right.mRaw, GetBytesize<THIS>());
            }
            else if constexpr (CT::Inner::Comparable<T>) {
               // Use comparison operator between all elements          
               auto t1 = GetRaw<THIS>();
               auto t2 = right.GetRaw();
               const auto t1end = t1 + mCount;
               while (t1 < t1end and *t1 == *t2) {
                  ++t1;
                  ++t2;
               }
               return t1 == t1end;
            }
            else LANGULUS_ERROR("Elements not comparable");
         }
      }
      else if constexpr (CT::Typed<THIS> or CT::Typed<RHS>) {
         // One of the blocks is statically typed - a runtime type      
         // check is required                                           
         if ((mCount or right.mCount) and not IsSimilar<THIS>(right.GetType()))
            return false;

         if constexpr (CT::Typed<THIS>)
            return Compare<RESOLVE, THIS>(reinterpret_cast<const THIS&>(right));
         else
            return Compare<RESOLVE, RHS>(right);
      }
      else {
         // Type-erased blocks                                          
         if (mCount != right.mCount) {
            // Cheap early return for differently sized blocks          
            VERBOSE(Logger::Red,
               "Data count is different: ",
               mCount, " != ", right.mCount);
            return false;
         }

         if (mCount and not mType->IsSimilar(right.mType)) {
            if (IsUntyped<THIS>() or right.IsUntyped()) {
               // Cheap early return if differing undefined types, when 
               // packs are not empty                                   
               VERBOSE(Logger::Red,
                  "One of the containers is untyped: ",
                  GetToken(), " != ", right.GetToken());
               return false;
            }
         }
         else if (not mCount or IsUntyped<THIS>()) {
            // Both blocks are untyped or empty, just compare states    
            return CompareStates(right);
         }

         if (not CompareStates(right)) {
            // Cheap early return for blocks of differing states        
            VERBOSE(Logger::Red,
               "Data states are not compatible");
            return false;
         }

         if (mType->IsSimilar(right.mType)) {
            // Types are exactly the same                               
            if (mRaw == right.mRaw) {
               // Quickly return if memory is exactly the same          
               VERBOSE(Logger::Green,
                  "Blocks are the same ", Logger::Cyan, "(optimal)");
               return true;
            }
            else if (mType->mIsPOD or mType->mIsSparse) {
               // Batch-compare memory if POD or sparse                 
               return 0 == memcmp(mRaw, right.mRaw, GetBytesize<THIS>());
            }
            else if (mType->mComparer) {
               // Call compare operator for each element pair           
               auto lhs = GetRawAs<Byte, THIS>();
               auto rhs = right.template GetRawAs<Byte, THIS>();
               const auto lhsEnd = GetRawEndAs<Byte, THIS>();
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
            if (not IsResolvable<THIS>() and not right.IsResolvable()
               and not CompareTypes(right, baseForComparison)) {
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

         if ((IsSparse<THIS>() and baseForComparison.mBinaryCompatible)
         or (baseForComparison.mType->mIsPOD and baseForComparison.mBinaryCompatible)) {
            // Just compare the memory directly (optimization)          
            // Regardless if types are sparse or dense, as long as they 
            // are of the same density, of course                       
            VERBOSE("Batch-comparing POD memory / pointers");
            const auto code = memcmp(mRaw, right.mRaw, GetBytesize<THIS>());
            if (code != 0) {
               VERBOSE(Logger::Red, "POD/pointers are not the same ", Logger::Yellow, "(fast)");
               return false;
            }

            VERBOSE(Logger::Green, "POD/pointers memory is the same ", Logger::Yellow, "(fast)");
         }
         else if (baseForComparison.mType->mComparer) {
            if (IsSparse<THIS>()) {
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
                  auto lhs = GetElementInner(i);
                  auto rhs = right.GetElementInner(i);

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
   }
   
   /// Compare with one single value, if exactly one element is contained     
   ///   @param rhs - the value to compare against                            
   ///   @return true if elements are the same                                
   template<CT::Block THIS> LANGULUS(INLINED)
   bool Block::CompareSingleValue(const CT::NotSemantic auto& rhs) const {
      using T = Deref<decltype(rhs)>;
      if (mCount != 1)
         return false;

      if constexpr (CT::Typed<THIS>) {
         // Both sides are statically typed                             
         if constexpr (CT::Inner::Comparable<TypeOf<THIS>, T>)
            return *GetRaw<THIS>() == rhs;
         else
            return false;
      }
      else {
         // THIS is type-erased, do runtime type checks                 
         if (IsUntyped<THIS>())
            return false;

         if constexpr (CT::Deep<T>) {
            // Deep types can be more loosely compared                  
            if (mType->mIsSparse or not mType->mIsDeep)
               return false;
            return *GetRawAs<Block, THIS>() == rhs;
         }
         else if constexpr (CT::StringLiteral<T>) {
            if (mType->template IsSimilar<Text>()) {
               // Implicitly make a text container on string literal    
               return *GetRawAs<Text, THIS>() == Text {Disown(rhs)};
            }
            else if (mType->template IsSimilar<char*, wchar_t*>()) {
               // Cast away the extent, compare against pointer         
               return *GetRawSparse<THIS>() == static_cast<const void*>(rhs);
            }
            else return false;
         }
         else if constexpr (CT::Inner::Comparable<T>) {
            // Non-deep element compare                                 
            if (not mType->template IsSimilar<T>())
               return false;
            return *GetRawAs<T, THIS>() == rhs;
         }
         else return false;
      }
   }
   
   /// Hash data inside memory block                                          
   ///   @attention order matters, so you might want to Neat data first       
   ///   @return the hash                                                     
   template<CT::Block THIS>
   Hash Block::GetHash() const {
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         if (not mCount)
            return {};

         if (mCount == 1) {
            // Exactly one element means exactly one hash               
            return HashOf(*GetRaw<THIS>());
         }

         // Hashing multiple elements                                   
         if constexpr (CT::Sparse<T>) {
            return HashBytes<DefaultHashSeed, false>(
               mRaw, static_cast<int>(GetBytesize<THIS>()));
         }
         else if constexpr (CT::POD<T> and not CT::Inner::HasGetHashMethod<T>) {
            // Hash all PODs at once                                    
            return HashBytes<DefaultHashSeed, alignof(T) < Bitness / 8> (
               mRaw, static_cast<int>(GetBytesize<THIS>()));
         }
         else {
            // Hash each element, and then combine hashes in a final one
            TAny<Hash> h;
            h.Reserve(mCount);
            for (auto& element : reinterpret_cast<const THIS&>(*this))
               h << HashOf(element);
            return h.GetHash();
         }
      }
      else {
         if (not mType or not mCount)
            return {};

         if (mCount == 1) {
            // Exactly one element means exactly one hash               
            if (mType->mIsSparse)
               return HashOf(*GetRawSparse<THIS>());
            else if (mType->Is<Hash>())
               return Get<Hash>();
            else if (mType->mHasher)
               return mType->mHasher(mRaw);
            else if (mType->mIsPOD) {
               if (mType->mAlignment < Bitness / 8) {
                  return HashBytes<DefaultHashSeed, true>(
                     mRaw, static_cast<int>(mType->mSize));
               }
               else {
                  return HashBytes<DefaultHashSeed, false>(
                     mRaw, static_cast<int>(mType->mSize));
               }
            }
            else LANGULUS_OOPS(Access, "Unhashable type", ": ", GetToken());
         }

         // Hashing multiple elements                                   
         if (mType->mIsSparse) {
            return HashBytes<DefaultHashSeed, false>(
               mRaw, static_cast<int>(GetBytesize<THIS>()));
         }
         else if (mType->mHasher) {
            // Use the reflected hasher for each element, and then      
            // combine hashes for a final one                           
            TAny<Hash> h;
            h.Reserve(mCount);
            ForEachElement<false, THIS>([&](const Block& element) {
               h << mType->mHasher(element.mRaw);
            });
            return h.GetHash();
         }
         else if (mType->mIsPOD) {
            if (mType->mAlignment < Bitness / 8) {
               return HashBytes<DefaultHashSeed, true>(
                  mRaw, static_cast<int>(GetBytesize<THIS>()));
            }
            else {
               return HashBytes<DefaultHashSeed, false>(
                  mRaw, static_cast<int>(GetBytesize<THIS>()));
            }
         }
         else LANGULUS_OOPS(Access, "Unhashable type", ": ", GetToken());
         return {};
      }
   }

   /// Find a single element's index inside container                         
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @param cookie - resume search from a given index                     
   ///   @return the index of the found item, or IndexNone if none found      
   template<bool REVERSE, CT::Block THIS>
   Index Block::Find(const CT::NotSemantic auto& item, Offset cookie) const noexcept {
      using ALT_T = Deref<decltype(item)>;

      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         if constexpr (not CT::Inner::Comparable<T, ALT_T>)
            return IndexNone;

         auto start = REVERSE
            ? GetRawEnd<THIS>() - 1 - cookie
            : GetRaw<THIS>() + cookie;
         const auto end = REVERSE
            ? start - mCount
            : start + mCount;

         while (start != end) {
            if (*start == item)
               return start - GetRaw<THIS>();

            if constexpr (REVERSE)
               --start;
            else
               ++start;
         }
      }
      else {
         // First check if element is contained inside this block's     
         // memory, because if so, we can easily find it, without       
         // calling a single compare function                           
         //TODO these heurisrics are bad! the pattern may appear in memory,
         // but there may be other occurences before that memory point! at best, we should
         // simply narrow the search scope, and after scanning it, fallback to the
         // one found pointer-wise
         /*if constexpr (CT::Deep<ALT_T>) {
            // Deep items have a bit looser type requirements           
            if (IsDense<THIS>() and IsDeep<THIS>() and Owns<THIS>(&item)) {
               const Offset index = &item - GetRawAs<ALT_T, THIS>();
               // Check required, because Owns tests in reserved range  
               return index < mCount ? index : IndexNone;
            }
         }
         else {
            // Search for a conventional item                           
            if (IsExact<ALT_T>() and Owns<THIS>(&item)) {
               const Offset index = &item - GetRawAs<ALT_T, THIS>();
               // Check required, because Owns tests in reserved range  
               return index < mCount ? index : IndexNone;
            }
         }*/

         // Item is not in this block's memory, so we start comparing by
         // values                                                      
         Offset i = REVERSE ? mCount - 1 - cookie : cookie;
         while (i < mCount) {
            if (GetElementInner(i) == item) {
               // Match found                                           
               return i;
            }

            if constexpr (REVERSE)
               --i;
            else
               ++i;
         }
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }
   
   /// Find a matching sequence of one or more matching elements              
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - block with a single item to search for                 
   ///   @param cookie - continue search from a given offset                  
   ///   @return the index of the found item, or IndexNone if not found       
   template<bool REVERSE, CT::Block THIS>
   Index Block::FindBlock(const CT::Block auto& item, CT::Index auto index) const noexcept {
      auto cookie = SimplifyIndex<THIS>(index);
      if (cookie >= mCount or item.mCount > mCount - cookie)
         return IndexNone;

      using B = Deref<decltype(item)>;
      if constexpr (CT::Typed<THIS> or CT::Typed<B>) {
         using TL = Conditional<CT::Typed<THIS>, TypeOf<THIS>, TypeOf<B>>;
         using TR = Conditional<CT::Typed<B>,    TypeOf<B>,    TypeOf<THIS>>;

         if constexpr (CT::Typed<THIS, B>) {
            // Leverage the fact, that both participants are typed      
            if constexpr (not CT::Inner::Comparable<TL, TR>)
               return IndexNone;
         }
         else {
            // One or none of the participants is typed, make sure types
            // match at runtime                                         
            if (not IsSimilar(item.GetType()))
               return IndexNone;
         }

         // If reached, types are comparable                            
         auto lhs = REVERSE ? GetRawEnd<TAny<TL>>() - cookie - item.GetCount()
                            : GetRaw<TAny<TL>>() + cookie;
         auto rhs = item.template GetRawAs<TR>();
         const auto lhsEnd = REVERSE ? GetRaw<TAny<TL>>() - 1
                                     : GetRawEnd<TAny<TL>>() - item.GetCount() + 1;
         const auto rhsEnd = item.template GetRawEnd<TAny<TR>>();
         UNUSED() const auto bytesize = item.Block::template GetBytesize<TAny<TL>>();
         while (lhs != lhsEnd) {
            if (*lhs == *rhs) {
               cookie = REVERSE ? GetRawEnd<TAny<TL>>() - lhs - 1
                                : lhs - GetRaw<TAny<TL>>();
               ++lhs;
               ++rhs;

               if constexpr (CT::Inner::BinaryCompatible<TL, TR>
                        and  CT::Inner::POD<TL, TR>) {
                  // We can use batch-compare                           
                  if (0 == memcmp(rhs, lhs, bytesize))
                     return cookie;
               }
               else {
                  // Types are not batch-comparable                     
                  while (rhs != rhsEnd and *lhs == *rhs) {
                     ++lhs;
                     ++rhs;
                  }

                  if (rhs == rhsEnd)
                     return cookie;
               }

               lhs = REVERSE ? GetRawEnd<TAny<TL>>() - cookie - 1
                             : GetRaw<TAny<TL>>() + cookie;
               rhs = item.template GetRaw<TAny<TR>>();
            }

            if constexpr (REVERSE)
               --lhs;
            else 
               ++lhs;
         }

         return IndexNone;
      }
      else {
         // One of the participators is type-erased                     
         // First check if element is contained inside this block's     
         // memory, because if so, we can easily find it, without       
         // calling a single compare function                           

         //TODO these heurisrics are bad! the pattern may appear in memory,
         // but there may be other occurences before that memory point! at best, we should
         // simply narrow the search scope, and after scanning it, fallback to the
         // one found pointer-wise

         /*if (item.IsDense() and item.IsDeep()) {
            // Deep items have a bit looser type requirements           
            if (IsDense<THIS>() and IsDeep<THIS>() and Owns<THIS>(item.mRaw)) {
               const Offset index = (item.mRaw - mRaw) / sizeof(Block);
               return index + item.GetCount() <= mCount ? index : IndexNone;
            }
         }
         else {
            // Search for a conventional item                           
            if (IsExact(item.GetType()) and Owns<THIS>(item.mRaw)) {
               const Offset index = (item.mRaw - mRaw) / mType->mSize;
               return index + item.GetCount() <= mCount ? index : IndexNone;
            }
         }*/

         // Slow and tedious RTTI-based compare                         
         Offset i = REVERSE ? mCount - 1 - cookie : cookie;
         const auto iend = REVERSE ? static_cast<Offset>(-1)
                                   : mCount - item.GetCount() + 1;
         while (i != iend) {
            if (CropInner(i, item.GetCount()) == item)
               return i;

            if constexpr (REVERSE)
               --i;
            else
               ++i;
         }

         // If this is reached, then no match was found                 
         return IndexNone;
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
      LANGULUS_ASSUME(DevAssumes, IsTyped<Block>(),
         "LHS block is not typed", " comparing with RHS: ", right.GetType<Block>());
      LANGULUS_ASSUME(DevAssumes, right.IsTyped<Block>(),
         "RHS block is not typed", " comparing with LHS: ", GetType<Block>());

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
      return  mRaw == right.mRaw 
          or (mRaw and right.mRaw
         and  base.mType->mComparer(mRaw, right.mRaw));
   }

   /// Gather items from input container, and fill output                     
   /// Output type acts as a filter to what gets gathered                     
   ///   @tparam REVERSE - are we gathering in reverse?                       
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @return the number of gathered elements                              
   template<bool REVERSE>
   Count Block::GatherInner(const CT::Block auto& input, CT::Block auto& output) {
      if (input.IsDeep() and not output.IsDeep()) {
         Count count = 0;
         ForEach<REVERSE>([&](const Block& i) {
            count += GatherInner<REVERSE>(i, output);
         });
         return count;
      }

      return output.InsertBlock(IndexBack, input);
   }

   /// Gather items of specific phase from input container and fill output    
   ///   @tparam REVERSE - are we gathering in reverse?                       
   ///   @param type - type to search for                                     
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @param state - the data state filter                                 
   ///   @return the number of gathered elements                              
   template<bool REVERSE>
   Count Block::GatherPolarInner(
      DMeta type, const CT::Block auto& input, CT::Block auto& output, DataState state
   ) {
      if (input.GetState() % state) {
         if (input.IsNow() and input.IsDeep()) {
            // Phases don't match, but we can dig deeper if deep        
            // and neutral, since Phase::Now is permissive              
            Block localOutput {input.GetUnconstrainedState(), type};
            ForEach<REVERSE>([&](const Block& i) {
               GatherPolarInner<REVERSE>(type, i, localOutput, state);
            });
            localOutput.MakeNow();
            const auto inserted = output.SmartPush(IndexBack, Abandon(localOutput));
            localOutput.Free<Any>();
            return inserted;
         }

         // Polarity mismatch                                           
         return 0;
      }

      // Input is flat and neutral/same                                 
      if (not type) {
         // Output is any, so no need to iterate                        
         return output.SmartPush(IndexBack, input);
      }

      // Iterate subpacks if any                                        
      Block localOutput {input.GetState(), type};
      GatherInner<REVERSE>(input, localOutput);
      localOutput.MakeNow();
      const auto inserted = output.InsertBlock(IndexBack, Abandon(localOutput));
      localOutput.Free<Any>();
      return inserted;
   }
   
   /// Compare loosely with another TAny, ignoring case                       
   /// This function applies only if T is character                           
   ///   @param other - text to compare with                                  
   ///   @return true if both containers match loosely                        
   template<CT::Block THIS> LANGULUS(INLINED)
   bool Block::CompareLoose(const CT::Block auto& other) const noexcept {
      return MatchesLoose<THIS>(other) == mCount;
   }

   /// Count how many consecutive elements match in two containers            
   ///   @param other - container to compare with                             
   ///   @return the number of matching items                                 
   template<CT::Block THIS> LANGULUS(INLINED)
   Count Block::Matches(const CT::Block auto& other) const noexcept {
      using OTHER = Deref<decltype(other)>;

      if constexpr (CT::Typed<THIS> and CT::Typed<OTHER>) {
         using T1 = TypeOf<THIS>;
         using T2 = TypeOf<OTHER>;

         if constexpr (CT::Inner::Comparable<T1, T2>) {
            if constexpr (CT::Similar<T1, T2>) {
               if (mRaw == other.mRaw)
                  return ::std::min(mCount, other.mCount);
            }

            auto t1 = GetRaw<THIS>();
            auto t2 = other.template GetRaw<OTHER>();
            const auto t1end = GetRawEnd<THIS>();
            const auto t2end = other.template GetRawEnd<OTHER>();
            while (t1 != t1end and t2 != t2end and *t1 == *(t2++))
               ++t1;
            return t1 - GetRaw();
         }
         else return false;
      }
      else TODO();
   }

   /// Compare loosely with another, ignoring upper-case                      
   /// Count how many consecutive letters match in two strings                
   ///   @param other - text to compare with                                  
   ///   @return the number of matching symbols                               
   template<CT::Block THIS> LANGULUS(INLINED)
   Count Block::MatchesLoose(const CT::Block auto& other) const noexcept {
      using OTHER = Deref<decltype(other)>;

      if constexpr (CT::Typed<THIS> and CT::Typed<OTHER>) {
         using T1 = TypeOf<THIS>;
         using T2 = TypeOf<OTHER>;

         if constexpr (CT::Character<T1> and CT::Similar<T1, T2>) {
            if (mRaw == other.mRaw)
               return mCount == other.mCount;
            else if (mCount != other.mCount)
               return false;

            auto t1 = GetRaw<THIS>();
            auto t2 = other.template GetRaw<THIS>();
            const auto tend = GetRawEnd<THIS>();
            while (t1 < tend and ::std::tolower(*t1)
                              == ::std::tolower(*(t2++)))
               ++t1;
            return GetRaw<THIS>() - t1;
         }
         else TODO();
      }
      else TODO();
   }

} // namespace Langulus::Anyness

#undef VERBOSE_TAB
#undef VERBOSE