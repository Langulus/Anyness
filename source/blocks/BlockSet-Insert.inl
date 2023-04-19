///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet.hpp"

namespace Langulus::Anyness
{

   /// Merge an element via copy                                              
   ///   @param value - the value to merge                                    
   ///   @return 1 if item was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count BlockSet::Insert(const CT::NotSemantic auto& value) {
      return Insert(Copy(value));
   }
   
   /// Merge an element via move                                              
   ///   @param value - the value to merge                                    
   ///   @return 1 if item was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count BlockSet::Insert(CT::NotSemantic auto&& value) {
      return Insert(Move(value));
   }

   /// Merge an element via semantic                                          
   ///   @param value - the value to merge                                    
   ///   @return 1 if item was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count BlockSet::Insert(CT::Semantic auto&& value) {
      using T = TypeOf<decltype(value)>;
      Mutate<T>();
      Allocate(GetCount() + 1);
      InsertInner<true>(GetBucket(value.mValue), value.Forward());
      return 1;
   }

   /// Insert a single value inside table via copy (unknown version)          
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count BlockSet::InsertUnknown(const Block& value) {
      return InsertUnknown(Langulus::Copy(value));
   }

   /// Insert a single pair inside table via move (unknown version)           
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   LANGULUS(INLINED)
   Count BlockSet::InsertUnknown(Block&& value) {
      return InsertUnknown(Langulus::Move(value));
   }
   
   /// Insert a single pair inside table via move (unknown version)           
   ///   @param value - the value to add                                      
   ///   @return 1 if item was inserted, zero otherwise                       
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Count BlockSet::InsertUnknown(S&& value) requires (CT::Block<TypeOf<S>>) {
      Mutate(value.mValue.mType);
      Reserve(GetCount() + 1);
      const auto index = value.mValue.GetHash().mHash & (GetReserved() - 1);
      InsertInnerUnknown<true>(index, value.Forward());
      return 1;
   }
   
   /// Merge the contents of both sets in this set by doing shallow copies    
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   LANGULUS(INLINED)
   Count BlockSet::Merge(const BlockSet& set) {
      return Merge(Copy(set));
   }

   /// Merge the contents of both sets in this set by moving elements         
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   LANGULUS(INLINED)
   Count BlockSet::Merge(BlockSet&& set) {
      return Merge(Move(set));
   }

   /// Merge the contents of both sets in this set by using a semantic        
   ///   @param set - the set to merge with this one                          
   ///   @return the number of elements that were inserted                    
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Count BlockSet::Merge(S&& set) {
      static_assert(CT::Set<TypeOf<S>>, "You can only merge other sets");
      Count inserted {};
      for (auto it : set.mValue)
         inserted += InsertUnknown(S::Nest(it));
      return inserted;
   }

   /// Copy-insert a templated pair inside the map                            
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (const CT::NotSemantic auto& item) {
      Insert(Copy(item));
      return *this;
   }

   /// Move-insert a templated pair inside the map                            
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (CT::NotSemantic auto&& item) {
      Insert(Move(item));
      return *this;
   }

   /// Move-insert a templated pair inside the map                            
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (CT::Semantic auto&& item) {
      Insert(item.Forward());
      return *this;
   }

   /// Copy-insert a type-erased item inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (const Block& item) {
      InsertUnknown(item);
      return *this;
   }

   /// Move-insert a type-erased item inside the map                          
   ///   @param item - the pair to insert                                     
   ///   @return a reference to this map for chaining                         
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator << (Block&& item) {
      InsertUnknown(Forward<Block>(item));
      return *this;
   }
   
   /// Request a new size of keys and info via the value container            
   /// The memory layout is:                                                  
   ///   [keys for each bucket]                                               
   ///         [padding for alignment]                                        
   ///               [info for each bucket]                                   
   ///                     [one sentinel byte for terminating loops]          
   ///   @attention assumes key type has been set                             
   ///   @param infoStart - [out] the offset at which info bytes start        
   ///   @return the requested byte size                                      
   LANGULUS(INLINED)
   Size BlockSet::RequestKeyAndInfoSize(const Count request, Offset& infoStart) noexcept {
      const Size keymemory = request * mKeys.GetStride();
      infoStart = keymemory + Alignment - (keymemory % Alignment);
      return infoStart + request + 1;
   }

   /// Rehashes and reinserts each pair in the same block                     
   ///   @attention assumes count and oldCount are power-of-two               
   ///   @attention assumes count > oldCount                                  
   ///   @param count - the new number of pairs                               
   ///   @param oldCount - the old number of pairs                            
   inline void BlockSet::Rehash(const Count& count, const Count& oldCount) {
      LANGULUS_ASSUME(DevAssumes, count > oldCount,
         "New count is not larger than oldCount");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(count),
         "New count is not a power-of-two");
      LANGULUS_ASSUME(DevAssumes, IsPowerOfTwo(oldCount),
         "Old count is not a power-of-two");

      auto oldInfo = GetInfo();
      const auto oldInfoEnd = oldInfo + oldCount;
      const auto hashmask = count - 1;

      // Prepare a set of preallocated swappers                         
      Block keyswap {mKeys.GetState(), GetType()};
      keyswap.AllocateFresh(keyswap.RequestSize(1));

      // For each old existing key...                                   
      while (oldInfo != oldInfoEnd) {
         if (*oldInfo) {
            // Rehash and check if hashes match                         
            const Offset oldIndex = oldInfo - GetInfo();
            auto oldKey = GetValue(oldIndex);
            const Offset newIndex = oldKey.GetHash().mHash & hashmask;
            if (oldIndex != newIndex) {
               // Move key & value to swapper                           
               // No chance of overlap, so do it forwards               
               keyswap.CallUnknownSemanticConstructors<false>(1, Abandon(oldKey));
               keyswap.mCount = 1;

               // Destroy the key and info                              
               oldKey.CallUnknownDestructors();
               *oldInfo = 0;
               --mKeys.mCount;

               if (oldIndex != InsertInnerUnknown<false>(
                  newIndex, Abandon(keyswap))) {
                  continue;
               }
            }
         }

         ++oldInfo;
      }

      // Free the allocated swapper memory                              
      keyswap.Free();
   }
   
   /// Inner insertion function based on reflected move-assignment            
   ///   @attention after this call, key and/or value might be empty          
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param value - value to move in                                      
   template<bool CHECK_FOR_MATCH, CT::Semantic S>
   Offset BlockSet::InsertInnerUnknown(const Offset& start, S&& value) {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();
         if constexpr (CHECK_FOR_MATCH) {
            const auto candidate = GetValue(index);
            if (candidate == value.mValue) {
               // Neat, the key already exists - just return            
               return index;
            }
         }

         if (attempts > *psl) {
            // The pair we're inserting is closer to bucket, so swap    
            GetValue(index).SwapUnknown(value.Forward());
            ::std::swap(attempts, *psl);
         }

         ++attempts;

         // Wrap around and start from the beginning if needed          
         if (psl < pslEnd - 1)
            ++psl;
         else
            psl = GetInfo();
      }

      // If reached, empty slot reached, so put the pair there	         
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless element exists and returns early    
      // We're moving only a single element, so no chance of overlap    
      const auto index = psl - GetInfo();
      GetValue(index)
         .CallUnknownSemanticConstructors(1, value.Forward());

      if constexpr (S::Move) {
         value.mValue.CallUnknownDestructors();
         value.mValue.mCount = 0;
      }

      *psl = attempts;
      ++mKeys.mCount;
      return index;
   }
   
   /// Inner insertion function                                               
   ///   @tparam CHECK_FOR_MATCH - false if you guarantee key doesn't exist   
   ///   @param start - the starting index                                    
   ///   @param value - value to move in                                      
   template<bool CHECK_FOR_MATCH, CT::Semantic S>
   Offset BlockSet::InsertInner(const Offset& start, S&& value) {
      using T = TypeOf<S>;
      HandleLocal<T> swapper {value.Forward()};

      // Get the starting index based on the key hash                   
      auto psl = GetInfo() + start;
      const auto pslEnd = GetInfoEnd();
      InfoType attempts {1};
      while (*psl) {
         const auto index = psl - GetInfo();

         if constexpr (CHECK_FOR_MATCH) {
            const auto& candidate = GetRaw<T>(index);
            if (swapper.Compare(candidate)) {
               // Neat, the value already exists - just return          
               return index;
            }
         }

         if (attempts > *psl) {
            // The value we're inserting is closer to bucket, so swap   
            GetHandle<T>(index).Swap(swapper);
            ::std::swap(attempts, *psl);
         }

         ++attempts;

         // Wrap around and start from the beginning if we have to      
         if (psl < pslEnd - 1)
            ++psl;
         else 
            psl = GetInfo();
      }

      // If reached, empty slot reached, so put the value there         
      // Might not seem like it, but we gave a guarantee, that this is  
      // eventually reached, unless key exists and returns early        
      const auto index = psl - GetInfo();
      GetHandle<T>(index).New(Abandon(swapper));

      *psl = attempts;
      ++mKeys.mCount;
      return index;
   }

} // namespace Langulus::Anyness
