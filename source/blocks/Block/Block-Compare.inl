///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../Block.hpp"
#include "../../text/Text.hpp"

#if 0
   #define VERBOSE(...)     Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_TAB(...) const auto tab = Logger::Verbose(__VA_ARGS__, Logger::Tabs {})
#else
   #define VERBOSE(...)     LANGULUS(NOOP)
   #define VERBOSE_TAB(...) LANGULUS(NOOP)
#endif


namespace Langulus::Anyness
{
   
   /// Compare to any other kind of deep container                            
   ///   @param rhs - element to compare against                              
   ///   @return true if containers match                                     
   template<class TYPE>
   bool Block<TYPE>::operator == (const CT::Block auto& rhs) const {
      return Compare<true>(rhs) or CompareSingleValue(rhs);
   }
   
   /// Compare to any other kind of deep container, or single custom element  
   ///   @param rhs - element to compare against                              
   ///   @return true if containers match                                     
   template<class TYPE> template<CT::NotBlock T1>
   bool Block<TYPE>::operator == (const T1& rhs) const
   requires (TypeErased or CT::Comparable<TYPE, T1>) {
      return CompareSingleValue(rhs);
   }
   
   /// Compare two block's contents for equality                              
   ///   @tparam RESOLVE - true to resolve each element before comparing      
   ///   @param right - the memory block to compare against                   
   ///   @return true if the two memory blocks are identical                  
   template<class TYPE> template<bool RESOLVE>
   bool Block<TYPE>::Compare(const CT::Block auto& right) const {
      using RHS = Deref<decltype(right)>;
      VERBOSE_TAB("Comparing ",
         Logger::PushWhite, mCount, " elements of ", GetToken(),
         Logger::Pop, " with ",
         Logger::PushWhite, right.mCount, " elements of ", right.GetToken()
      );

      if constexpr (not TypeErased and not RHS::TypeErased) {
         // Both blocks are statically typed - leverage it              
         if constexpr (not CT::Similar<TYPE, TypeOf<RHS>>) {
            // Types are different                                      
            return false;
         }
         else {
            // Types are similar                                        
            if (mRaw == right.mRaw)
               return mCount == right.mCount;
            else if (mCount != right.mCount)
               return false;

            if constexpr (CT::POD<TYPE>) {
               // Batch compare pods or pointers                        
               return 0 == ::std::memcmp(mRaw, right.mRaw, GetBytesize());
            }
            else if constexpr (CT::Comparable<TYPE, TYPE>) {
               // Use comparison operator between all elements          
               auto t1 = GetRaw();
               auto t2 = right.GetRaw();
               const auto t1end = t1 + mCount;
               while (t1 < t1end and *t1 == *t2) {
                  ++t1;
                  ++t2;
               }
               return t1 == t1end;
            }
            else return false;
         }
      }
      else if constexpr (not TypeErased or not RHS::TypeErased) {
         // One of the blocks is statically typed - a runtime type      
         // check is required                                           
         if ((mCount or right.mCount) and not IsSimilar(right))
            return false;

         if constexpr (not TypeErased)
            return Compare<RESOLVE>(reinterpret_cast<const Block<TYPE>&>(right));
         else
            return right.template Compare<RESOLVE>(reinterpret_cast<const RHS&>(*this));
      }
      else {
         // Type-erased blocks                                          
         if (mCount != right.mCount) {
            // Cheap early return for differently sized blocks          
            VERBOSE(Logger::Red,
               "Data count is different: ", mCount, " != ", right.mCount);
            return false;
         }

         if (mCount and not mType->IsSimilar(right.mType)) {
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
            VERBOSE(Logger::Red, "Data states are not compatible");
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
               return 0 == memcmp(mRaw, right.mRaw, GetBytesize());
            }
            else if (mType->mComparer) {
               // Call compare operator for each element pair           
               auto lhs = mRaw;
               auto rhs = right.mRaw;
               const auto lhsEnd = GetRawEnd<Byte>();
               while (lhs != lhsEnd) {
                  if (not mType->mComparer(lhs, rhs))
                     return false;
                  lhs += mType->mSize;
                  rhs += mType->mSize;
               }

               return true;
            }
            else LANGULUS_OOPS(Compare, "No == operator reflected for type ", mType);
         }

         // If this is reached, then an advanced comparison commences   
         RTTI::Base baseForComparison {};
         if constexpr (RESOLVE) {
            // We will test type for each resolved element, individually
            if (not IsResolvable() and not right.IsResolvable()
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

         if ((IsSparse() and baseForComparison.mBinaryCompatible)
         or (baseForComparison.mType and baseForComparison.mType->mIsPOD
         and baseForComparison.mBinaryCompatible)) {
            // Just compare the memory directly (optimization)          
            // Regardless if types are sparse or dense, as long as they 
            // are of the same density, of course                       
            VERBOSE("Batch-comparing POD memory / pointers");
            const auto code = memcmp(mRaw, right.mRaw, GetBytesize());
            if (code != 0) {
               VERBOSE(Logger::Red, "POD/pointers are not the same ",
                  Logger::Yellow, "(fast)");
               return false;
            }

            VERBOSE(Logger::Green, "POD/pointers memory is the same ",
               Logger::Yellow, "(fast)");
         }
         else if (baseForComparison.mType and baseForComparison.mType->mComparer) {
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
         else if (baseForComparison.mType) {
            VERBOSE(Logger::Red,
               "Can't compare related types because no == operator is reflected, "
               "and they're not POD - common base for comparison was: ",
               baseForComparison.mType);
            return false;
         }
         else {
            VERBOSE(Logger::Red,
               "Can't compare unrelated types ", mType, " and ", right.mType);
            return false;
         }

         return true;
      }
   }
   
   /// Compare with one single value, if exactly one element is contained     
   ///   @param rhs - the value to compare against                            
   ///   @return true if elements are the same                                
   template<class TYPE> LANGULUS(INLINED)
   bool Block<TYPE>::CompareSingleValue(const CT::NoIntent auto& rhs) const {
      using T = Deref<decltype(rhs)>;
      if (mCount != 1)
         return false;

      if constexpr (not TypeErased) {
         // Both sides are statically typed                             
         if constexpr (CT::Similar<TYPE, T> and CT::Comparable<TYPE, T>)
            return *GetRaw() == rhs;
         else if constexpr (CT::Owned<T> and CT::Similar<TYPE, TypeOf<T>>
         and CT::Comparable<TYPE, TypeOf<T>>)
            return *GetRaw() == rhs.Get();
         else
            return false;
      }
      else {
         // THIS is type-erased, do runtime type checks                 
         if (IsUntyped())
            return false;

         if constexpr (CT::Deep<T>) {
            // Deep types can be more loosely compared                  
            if (mType->mIsSparse or not mType->mIsDeep)
               return false;
            return GetDeep() == rhs;
         }
         else if constexpr (CT::StringLiteral<T>) {
            if (mType->template IsSimilar<Text>()) {
               // Implicitly make a text container on string literal    
               return *GetRaw<Text>() == Text {Disown(rhs)};
            }
            else if (mType->template IsSimilar<char*, wchar_t*>()) {
               // Cast away the extent, compare against pointer         
               return GetRaw<void*>() == static_cast<const void*>(rhs);
            }
            else return false;
         }
         else if constexpr (CT::Comparable<T, T>) {
            // Non-deep element compare                                 
            if (mType->template IsSimilar<T>())
               return *GetRaw<T>() == rhs;
            else if constexpr (CT::Owned<T>) {
               if (mType->template IsSimilar<TypeOf<T>>())
                  return *GetRaw<T>() == rhs.Get();
               else
                  return false;
            }
            else return false;
         }
         else return false;
      }
   }
   
   /// Hash data inside memory block                                          
   ///   @attention order matters, so you might want to Neat data first       
   ///   @return the hash                                                     
   template<class TYPE>
   Hash Block<TYPE>::GetHash() const requires (TypeErased or CT::Hashable<TYPE>) {
      if constexpr (not TypeErased) {
         if (not mCount)
            return {};

         if (mCount == 1) {
            // Exactly one element means exactly one hash               
            return HashOf(*GetRaw());
         }

         // Hashing multiple elements                                   
         if constexpr (Sparse) {
            return HashBytes<DefaultHashSeed, false>(
               mRaw, static_cast<int>(GetBytesize()));
         }
         else if constexpr (CT::POD<TYPE> and not CT::Inner::HasGetHashMethod<TYPE>) {
            // Hash all PODs at once                                    
            return HashBytes<DefaultHashSeed, alignof(TYPE) < Bitness / 8> (
               mRaw, static_cast<int>(GetBytesize()));
         }
         else {
            // Hash each element, and then combine hashes in a final one
            TMany<Hash> h;
            h.Reserve(mCount);
            for (auto& element : *this)
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
               return HashOf(*mRawSparse);
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
               mRaw, static_cast<int>(GetBytesize()));
         }
         else if (mType->mHasher) {
            // Use the reflected hasher for each element, and then      
            // combine hashes for a final one                           
            TMany<Hash> h;
            h.Reserve(mCount);
            ForEachElement<false>([&](const Block<>& element) {
               h << mType->mHasher(element.mRaw);
            });
            return h.GetHash();
         }
         else if (mType->mIsPOD) {
            if (mType->mAlignment < Bitness / 8) {
               return HashBytes<DefaultHashSeed, true>(
                  mRaw, static_cast<int>(GetBytesize()));
            }
            else {
               return HashBytes<DefaultHashSeed, false>(
                  mRaw, static_cast<int>(GetBytesize()));
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
   template<class TYPE> template<bool REVERSE, CT::NoIntent T1>
   Index Block<TYPE>::Find(const T1& item, Offset cookie) const noexcept
   requires (TypeErased or CT::Comparable<TYPE, T1>) {
      if constexpr (not TypeErased) {
         auto start = REVERSE
            ? GetRawEnd() - 1 - cookie
            : GetRaw() + cookie;
         const auto end = REVERSE
            ? start - mCount
            : start + mCount;

         while (start != end) {
            if (*start == item)
               return start - GetRaw();

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
   ///   @param item - a block containing a sequence of items to search for   
   ///   @param index - continue search from a given offset                   
   ///   @return the index of the found item, or IndexNone if not found       
   template<class TYPE> template<bool REVERSE>
   Index Block<TYPE>::FindBlock(const CT::Block auto& item, CT::Index auto index) const noexcept {
      auto cookie = SimplifyIndex(index);
      if (cookie >= mCount or item.mCount > mCount - cookie)
         return IndexNone;

      using RHS = Deref<decltype(item)>;
      using LB = Block<Conditional<     TypeErased, TypeOf<RHS>, TYPE>>;
      using RB = Block<Conditional<RHS::TypeErased, TYPE, TypeOf<RHS>>>;

      auto& lb = reinterpret_cast<const LB&>(*this);
      auto& rb = reinterpret_cast<const RB&>( item);

      if constexpr (not TypeErased or not RHS::TypeErased) {
         // One of the participating blocks is statically typed         
         // Let's check type compatibility first                        
         if constexpr (not TypeErased and not RHS::TypeErased) {
            // Leverage the fact, that both participants are typed      
            if constexpr (not CT::Comparable<TypeOf<LB>, TypeOf<RB>>)
               return IndexNone;
         }
         else {
            // One or none of the participants is typed                 
            if (not IsSimilar(item))
               return IndexNone;
         }

         // If this is reached reached, then types are comparable       
         auto rhs = rb.GetRaw();
         auto lhs = REVERSE
            ? lb.GetRawEnd() - cookie - item.GetCount()
            : lb.GetRaw() + cookie;

         const auto rhsEnd = rb.GetRawEnd();
         const auto lhsEnd = REVERSE
            ? lb.GetRaw() - 1
            : lb.GetRawEnd() - item.GetCount() + 1;

         // This byte size is used ONLY IF both types are binary        
         // compatible. It is simply precomputed here, so that it isn't 
         // recomputed in the loop.                                     
         UNUSED() const auto bytesize = lb.GetBytesize();

         while (lhs != lhsEnd) {
            if (*lhs == *rhs) {
               cookie = REVERSE
                  ? lb.GetRawEnd() - lhs - 1
                  : lhs - lb.GetRaw();

               ++lhs;
               ++rhs;

               if constexpr (CT::BinaryCompatible<TypeOf<LB>, TypeOf<RB>>
               and CT::POD<TypeOf<LB>, TypeOf<RB>>) {
                  // We can use batch-compare                           
                  if (0 == memcmp(rhs, lhs, bytesize))
                     return cookie;
               }
               else {
                  // Types are not batch-comparable, so compare them    
                  // one by one                                         
                  while (rhs != rhsEnd and *lhs == *rhs) {
                     ++lhs;
                     ++rhs;
                  }

                  if (rhs == rhsEnd)
                     return cookie;
               }

               lhs = REVERSE
                  ? lb.GetRawEnd() - cookie - 1
                  : lb.GetRaw() + cookie;
               rhs = rb.GetRaw();
            }

            if constexpr (REVERSE) --lhs;
            else                   ++lhs;
         }

         return IndexNone;
      }
      else {
         // All of the participators are type-erased                    
         // We do a slow and tedious RTTI-based compare                 

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

         Offset i = REVERSE ? mCount - 1 - cookie : cookie;
         const auto iend = REVERSE
            ? static_cast<Offset>(-1)
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
   template<class TYPE> LANGULUS(INLINED)
   bool Block<TYPE>::CompareStates(const Block& rhs) const noexcept {
      return GetUnconstrainedState() == rhs.GetUnconstrainedState();
   }

   /// Compare types of two blocks, and a produce a common type whose         
   /// comparison function to use                                             
   ///   @attention assumes that both blocks are typed                        
   ///   @param right - the type to the right                                 
   ///   @param common - [out] the common base                                
   ///   @return true if a common base has been found                         
   template<class TYPE>
   bool Block<TYPE>::CompareTypes(const CT::Block auto& right, RTTI::Base& common) const {
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
   template<class TYPE> LANGULUS(INLINED)
   bool Block<TYPE>::CallComparer(const Block& right, const RTTI::Base& base) const {
      return  mRaw == right.mRaw or (mRaw and right.mRaw
         and  base.mType->mComparer(mRaw, right.mRaw));
   }

   /// Gather items from input container, and fill output                     
   /// Output type acts as a filter to what gets gathered                     
   ///   @tparam REVERSE - are we gathering in reverse?                       
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @return the number of gathered elements                              
   template<class TYPE> template<bool REVERSE>
   Count Block<TYPE>::GatherInner(CT::Block auto& output) const {
      using OUT = Deref<decltype(output)>;

      if constexpr (not TypeErased and not OUT::TypeErased) {
         // Both containers are statically typed - leverage it          
         TODO();
      }
      else {
         if (IsDeep() and not output.IsDeep()) {
            Count count = 0;
            ForEach<REVERSE>([&](const Block<>& i) {
               count += i.template GatherInner<REVERSE>(output);
            });
            return count;
         }

         if constexpr (not OUT::TypeErased) {
            // Output container is strictly typed, we can't make loose  
            // matches                                                  
            if (IsSimilar<Block<TYPE>, TypeOf<OUT>>())
               return output.InsertBlock(IndexBack, *this);
            else
               return 0;
         }
         else {
            if (output.IsTypeConstrained()) {
               // Output container is strictly typed, we can't make     
               // loose matches                                         
               if (IsSimilar(output))
                  return output.template InsertBlock<void, false>(IndexBack, *this);
               else
                  return 0;
            }
            else {
               // Output is not strictly typed, so we can afford a      
               // looser coparison                                      
               return output.InsertBlock(IndexBack, *this);
            }
         }
      }
   }

   /// Gather items of specific phase from input container and fill output    
   ///   @tparam REVERSE - are we gathering in reverse?                       
   ///   @param type - type to search for                                     
   ///   @param input - source container                                      
   ///   @param output - [in/out] container that collects results             
   ///   @param state - the data state filter                                 
   ///   @return the number of gathered elements                              
   template<class TYPE> template<bool REVERSE>
   Count Block<TYPE>::GatherPolarInner(
      DMeta type, CT::Block auto& output, DataState state
   ) const {
      if (GetState() % state) {
         if (IsNow() and IsDeep()) {
            // Phases don't match, but we can dig deeper if deep        
            // and neutral, since Phase::Now is permissive              
            Block<> localOutput {GetUnconstrainedState(), type};
            ForEach<REVERSE>([&](const Block<>& i) {
               i.GatherPolarInner<REVERSE>(type, localOutput, state);
            });
            localOutput.MakeNow();
            const auto inserted = output.SmartPush(IndexBack, Abandon(localOutput));
            localOutput.Free();
            return inserted;
         }

         // Polarity mismatch                                           
         return 0;
      }

      // Input is flat and neutral/same                                 
      if (not type) {
         // Output is any, so no need to iterate                        
         return output.SmartPush(IndexBack, *this);
      }

      // Iterate subpacks if any                                        
      Block<> localOutput {GetState(), type};
      GatherInner<REVERSE>(localOutput);
      localOutput.MakeNow();
      const auto inserted = output.InsertBlock(IndexBack, Abandon(localOutput));
      localOutput.Free();
      return inserted;
   }
   
   /// Compare loosely with another TMany, ignoring case                      
   /// This function applies only if T is character                           
   ///   @param other - container to compare with                             
   ///   @return true if both containers match loosely                        
   template<class TYPE> LANGULUS(INLINED)
   bool Block<TYPE>::CompareLoose(const CT::Block auto& other) const noexcept {
      return (IsEmpty() and other.IsEmpty() and IsSimilar(other))
          or MatchesLoose(other) == mCount;
   }

   /// Count how many consecutive elements match in two containers            
   ///   @param other - container to compare with                             
   ///   @return the number of matching items                                 
   template<class TYPE> LANGULUS(INLINED)
   Count Block<TYPE>::Matches(const CT::Block auto& other) const noexcept {
      if (IsEmpty() or other.IsEmpty())
         return 0;

      using OTHER = Deref<decltype(other)>;
      if constexpr (not TypeErased and not OTHER::TypeErased) {
         using T2 = TypeOf<OTHER>;

         if constexpr (CT::Comparable<TYPE, T2>) {
            auto t1 = GetRaw();
            auto t2 = other.GetRaw();
            const auto t1end = GetRawEnd();
            const auto t2end = other.GetRawEnd();
            while (t1 != t1end and t2 != t2end and *t1 == *t2) {
               ++t1;
               ++t2;
            }
            return t1 - GetRaw();
         }
         else return false;
      }
      else TODO();
   }

   /// Compare loosely with another, ignoring upper-case if both blocks       
   /// contain letters                                                        
   ///   @param other - container to compare with                             
   ///   @return the number of loosely matching elements                      
   template<class TYPE> LANGULUS(INLINED)
   Count Block<TYPE>::MatchesLoose(const CT::Block auto& other) const noexcept {
      if (IsEmpty() or other.IsEmpty())
         return 0;

      using OTHER = Deref<decltype(other)>;
      if constexpr (not TypeErased and not OTHER::TypeErased) {
         using T2 = TypeOf<OTHER>;

         if constexpr (CT::Character<TYPE> and CT::Similar<TYPE, T2>) {
            auto t1 = GetRaw();
            auto t2 = other.GetRaw();
            const auto tend = GetRawEnd();
            while (t1 < tend and ::std::tolower(*t1) == ::std::tolower(*t2)) {
               ++t1;
               ++t2;
            }
            return t1 - GetRaw();
         }
         else TODO();
      }
      else TODO();
   }
   
   /// Check if anything comparable exists in the block                       
   ///   @param what - the element to search for                              
   ///   @return true if element is found, false otherwise                    
   template<class TYPE> LANGULUS(INLINED)
   bool Block<TYPE>::Contains(const CT::NoIntent auto& what) const {
      return Find(what) != IndexNone;
   }

   /// Sort the contents of this container using a static type                
   ///   @attention assumes T is the type of the container                    
   ///   @attention this is the most naive of sorting algorithms              
   ///   @tparam ASCEND - whether to sort in ascending order (123)            
   template<class TYPE> template<bool ASCEND>
   void Block<TYPE>::Sort() requires (TypeErased or CT::Sortable<TYPE, TYPE>) {
      auto lhs = GetRaw();
      const auto lhsEnd = lhs + mCount;
      while (lhs != lhsEnd) {
         auto rhs = GetRaw();
         while (rhs != lhs) {
            if constexpr (ASCEND) {
               if (*lhs < *rhs)
                  ::std::swap(*lhs, *rhs);
            }
            else {
               if (*lhs > *rhs)
                  ::std::swap(*lhs, *rhs);
            }

            ++rhs;
         }

         ++rhs;

         while (rhs != lhsEnd) {
            if constexpr (ASCEND) {
               if (*lhs < *rhs)
                  ::std::swap(*lhs, *rhs);
            }
            else {
               if (*lhs > *rhs)
                  ::std::swap(*lhs, *rhs);
            }

            ++rhs;
         }

         ++lhs;
      }
   }

} // namespace Langulus::Anyness

#undef VERBOSE_TAB
#undef VERBOSE