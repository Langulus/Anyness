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

   /// Checks if both tables contain the same entries                         
   ///   @attention assumes both sets are unordered                           
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   template<class T>
   bool BlockSet::operator == (const T& other) const {
      static_assert(CT::Set<T>, "T must be a set type");
      if (other.GetCount() != GetCount() 
         || !GetType()->IsExact(other.GetType()))
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            const auto lhs = info - GetInfo();
            if constexpr (CT::Typed<T>) {
               using K = TypeOf<T>;
               const auto rhs = other.FindIndex(GetRaw<K>(lhs));
               if (rhs == other.GetReserved())
                  return false;
            }
            else {
               const auto rhs = other.template FindIndexUnknown<T>(GetValue(lhs));
               if (rhs == other.GetReserved())
                  return false;
            }
         }

         ++info;
      }

      return true;
   }
   
   template<class T>
   bool BlockSet::operator != (const T& other) const {
      return !(operator == (other));
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
      return FindIndex(key) != GetReserved();
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   LANGULUS(INLINED)
   Index BlockSet::Find(const CT::NotSemantic auto& key) const {
      const auto offset = FindIndex(key);
      return offset != GetReserved() ? Index {offset} : IndexNone;
   }

   /// Find the index of a pair by key                                        
   ///   @tparam THIS - set to interpret this one as, for optimal bucketing   
   ///   @tparam K - key type to use for comparison                           
   ///   @param match - the key to search for                                 
   ///   @return the index, or mValues.mReserved if not found                 
   template<class THIS, CT::NotSemantic K>
   Offset BlockSet::FindIndex(const K& match) const {
      if (IsEmpty())
         return GetReserved();

      static_assert(CT::Set<THIS>, "THIS must be a set type");
      auto& This = reinterpret_cast<const THIS&>(*this);

      // Get the starting index based on the key hash                   
      const auto start = This.GetBucket(GetReserved() - 1, match);
      auto info = GetInfo() + start;
      if (!*info)
         return GetReserved();

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
         const ::std::ptrdiff_t index = info - GetInfo();
         if (index - *info > starti)
            return GetReserved();

         if (*key == match)
            return static_cast<Offset>(index);

         ++key; ++info;
      }

      // Reached only if info has reached the end                       
      // Keys might loop around, continue the search from the start     
      info = GetInfo();
      if (GetReserved() - *info > start)
         return GetReserved();

      key = &GetRaw<K>(0);
      if (*key == match)
         return 0;

      ++key;
      ++info;

      while (info != infoEnd) {
         const Offset index = info - GetInfo();
         if (GetReserved() - index - *info > start)
            return GetReserved();

         if (*key == match)
            return index;

         ++key; ++info;
      }
      
      // No such key was found                                          
      return GetReserved();
   }
   
   /// Find the index of a pair by an unknown type-erased key                 
   ///   @tparam THIS - set to interpret this one as, for optimal bucketing   
   ///   @param match - the key to search for                                 
   ///   @return the index, or mValues.mReserved if not found                 
   template<class THIS>
   Offset BlockSet::FindIndexUnknown(const Block& match) const {
      if (IsEmpty())
         return GetReserved();

      static_assert(CT::Set<THIS>, "THIS must be a set type");
      auto& This = reinterpret_cast<const THIS&>(*this);

      // Get the starting index based on the key hash                   
      const auto start = This.GetBucketUnknown(GetReserved() - 1, match);
      auto info = GetInfo() + start;
      if (!*info)
         return GetReserved();

      // Test first candidate                                           
      auto key = GetValue(start);
      if (key == match)
         return start;

      // Test all candidates on the right up until the end              
      key.Next();
      ++info;

      const auto infoEnd = GetInfoEnd();
      const auto starti = static_cast<::std::ptrdiff_t>(start);
      while (info != infoEnd) {
         const ::std::ptrdiff_t index = info - GetInfo();
         if (index - *info > starti)
            return GetReserved();

         if (key == match)
            return index;

         ++info;
         key.Next();
      }

      // Reached only if info has reached the end                       
      // Keys might loop around, continue the search from the start     
      info = GetInfo();
      if (GetReserved() - *info > start)
         return GetReserved();

      key = Get(0);
      if (key == match)
         return 0;

      key.Next();
      ++info;

      while (info != infoEnd) {
         const Offset index = info - GetInfo();
         if (GetReserved() - index - *info > start)
            return GetReserved();

         if (key == match)
            return index;

         ++info;
         key.Next();
      }
      
      // No such key was found                                          
      return GetReserved();
   }

} // namespace Langulus::Anyness
