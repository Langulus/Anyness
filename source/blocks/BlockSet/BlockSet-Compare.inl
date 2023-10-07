///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockSet.hpp"

namespace Langulus::Anyness
{

   /// Checks if both tables contain the same entries                         
   ///   @attention assumes both sets are of same orderness                   
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   inline bool BlockSet::operator == (const BlockSet& other) const {
      if (other.GetCount() != GetCount() or not IsTypeCompatibleWith(other))
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            const auto lhs = info - GetInfo();
            const auto rhs = other.FindInnerUnknown(GetInner(lhs));
            if (rhs == InvalidOffset)
               return false;
         }

         ++info;
      }

      return true;
   }

   /// Get hash of the set contents                                           
   ///   @attention the hash is not cached, so this is a slow operation       
   ///   @return the hash                                                     
   LANGULUS(INLINED)
   Hash BlockSet::GetHash() const {
      TAny<Hash> hashes;
      for (auto element : *this)
         hashes << element.GetHash();
      return hashes.GetHash();
   }

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   LANGULUS(INLINED)
   bool BlockSet::Contains(const CT::NotSemantic auto& key) const {
      using T = Decvq<Deref<decltype(key)>>;
      if (IsEmpty() or not mKeys.mType or not mKeys.mType->template IsExact<T>()) 
         return false;

      return FindInner<BlockSet, T>(key) != InvalidOffset;
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   LANGULUS(INLINED)
   Index BlockSet::Find(const CT::NotSemantic auto& key) const {
      const auto offset = FindInner(key);
      return offset != InvalidOffset ? Index {offset} : IndexNone;
   }

   /// Find the index of a pair by key                                        
   ///   @attention assumes map is not empty                                  
   ///   @attention assumes keys are of the exactly same type                 
   ///   @tparam THIS - set to interpret this one as, for optimal bucketing   
   ///   @param match - the key to search for                                 
   ///   @return the index, or mValues.mReserved if not found                 
   template<class THIS>
   Offset BlockSet::FindInner(const CT::NotSemantic auto& match) const {
      using K = Decvq<Deref<decltype(match)>>;
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(), "Set is empty");
      LANGULUS_ASSUME(DevAssumes, mKeys.mType
         and mKeys.mType->template IsExact<K>(), "Type mismatch");

      static_assert(CT::Set<THIS>, "THIS must be a set type");
      auto& This = reinterpret_cast<const THIS&>(*this);

      // Get the starting index based on the key hash                   
      const auto start = This.GetBucket(GetReserved() - 1, match);
      auto info = GetInfo() + start;
      if (not *info)
         return InvalidOffset;

      // Test first candidate                                           
      auto key = &GetRaw<K>(start);
      if (*key == match)
         return start;

      // Test all candidates on the right up until the end              
      ++key;
      ++info;

      const auto infoEnd = GetInfoEnd();
      const auto starti = static_cast<::std::ptrdiff_t>(start);
      while (info != infoEnd) {
         if (not *info)
            return InvalidOffset;

         const ::std::ptrdiff_t index = info - GetInfo();
         if (index - *info > starti)
            return InvalidOffset;

         if (*key == match)
            return static_cast<Offset>(index);

         ++key; ++info;
      }

      // Reached only if info has reached the end                       
      // Keys might loop around, continue the search from the start     
      info = GetInfo();
      if (GetReserved() - *info > start)
         return InvalidOffset;

      key = &GetRaw<K>(0);
      if (*key == match)
         return 0;

      ++key;
      ++info;

      while (info != infoEnd) {
         if (not *info)
            return InvalidOffset;

         const Offset index = info - GetInfo();
         if (GetReserved() - index - *info > start)
            return InvalidOffset;

         if (*key == match)
            return index;

         ++key; ++info;
      }

      return InvalidOffset;
   }
   
   /// Find the index of a pair by an unknown type-erased key                 
   ///   @attention assumes map is not empty                                  
   ///   @attention assumes keys are of the exactly same type                 
   ///   @tparam THIS - set to interpret this one as, for optimal bucketing   
   ///   @param match - the key to search for                                 
   ///   @return the index, or mValues.mReserved if not found                 
   template<class THIS>
   Offset BlockSet::FindInnerUnknown(const Block& match) const {
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(), "Set is empty");
      LANGULUS_ASSUME(DevAssumes, mKeys.mType
         and mKeys.mType->IsExact(match.GetType()), "Type mismatch");

      static_assert(CT::Set<THIS>, "THIS must be a set type");
      auto& This = reinterpret_cast<const THIS&>(*this);

      // Get the starting index based on the key hash                   
      const auto start = This.GetBucketUnknown(GetReserved() - 1, match);
      auto info = GetInfo() + start;
      if (not *info)
         return InvalidOffset;

      // Test first candidate                                           
      auto key = GetInner(start);
      if (key == match)
         return start;

      // Test all candidates on the right up until the end              
      key.Next();
      ++info;

      const auto infoEnd = GetInfoEnd();
      const auto starti = static_cast<::std::ptrdiff_t>(start);
      while (info != infoEnd) {
         if (not *info)
            return InvalidOffset;

         const ::std::ptrdiff_t index = info - GetInfo();
         if (index - *info > starti)
            return InvalidOffset;

         if (key == match)
            return static_cast<Offset>(index);

         ++info;
         key.Next();
      }

      // Reached only if info has reached the end                       
      // Keys might loop around, continue the search from the start     
      info = GetInfo();
      if (GetReserved() - *info > start)
         return InvalidOffset;

      key = GetInner(0);
      if (key == match)
         return 0;

      key.Next();
      ++info;

      while (info != infoEnd) {
         if (not *info)
            return InvalidOffset;

         const Offset index = info - GetInfo();
         if (GetReserved() - index - *info > start)
            return InvalidOffset;

         if (key == match)
            return index;

         ++info;
         key.Next();
      }
      
      // No such key was found                                          
      return InvalidOffset;
   }

} // namespace Langulus::Anyness
