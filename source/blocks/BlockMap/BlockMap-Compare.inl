///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"

namespace Langulus::Anyness
{
   
   /// Checks if both tables contain the same entries                         
   ///   @attention assumes both maps are of same orderness                   
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   inline bool BlockMap::operator == (const BlockMap& other) const {
      if (other.GetCount() != GetCount() or not IsTypeCompatibleWith(other))
         return false;

      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            const auto lhs = info - GetInfo();
            const auto rhs = other.FindIndexUnknown(GetKeyInner(lhs));
            if (rhs == other.GetReserved()
               or GetValueInner(lhs) != other.GetValueInner(rhs))
               return false;
         }

         ++info;
      }

      return true;
   }
   
   /// Get hash of the map contents                                           
   ///   @attention the hash is not cached, so this is a slow operation       
   ///   @return the hash                                                     
   LANGULUS(INLINED)
   Hash BlockMap::GetHash() const {
      TAny<Hash> hashes;
      for (auto pair : *this)
         hashes << pair.GetHash();
      return hashes.GetHash();
   }

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   template<CT::NotSemantic K>
   LANGULUS(INLINED)
   bool BlockMap::ContainsKey(const K& key) const {
      if (IsEmpty())
         return false;
      return FindIndex(key) != GetReserved();
   }

   /// Search for a value inside the table                                    
   ///   @param value - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   template<CT::NotSemantic V>
   bool BlockMap::ContainsValue(const V& match) const {
      if (IsEmpty())
         return false;

      auto value = &GetRawValue<V>(0);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();

      while (info != infoEnd) {
         if (*info and *value == match)
            return true;

         ++value; ++info;
      }

      return false;
   }

   /// Search for a pair inside the table                                     
   ///   @param pair - the pair to search for                                 
   ///   @return true if pair is found, false otherwise                       
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(INLINED)
   bool BlockMap::ContainsPair(const TPair<K, V>& pair) const {
      const auto found = FindIndex(pair.mKey);
      return found != GetReserved() and GetValueInner(found) == pair.mValue;
   }
   
   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   template<CT::NotSemantic K>
   LANGULUS(INLINED)
   Index BlockMap::Find(const K& key) const {
      const auto offset = FindIndex(key);
      return offset != GetReserved() ? Index {offset} : IndexNone;
   }

   /// Find the index of a pair by key                                        
   ///   @tparam THIS - map to interpret this one as, for optimal bucketing   
   ///   @tparam K - key type to use for comparison                           
   ///   @param match - the key to search for                                 
   ///   @return the index, or mValues.mReserved if not found                 
   template<class THIS, CT::NotSemantic K>
   Offset BlockMap::FindIndex(const K& match) const {
      if (IsEmpty())
         return GetReserved();

      static_assert(CT::Map<THIS>, "THIS must be a map type");
      auto& This = reinterpret_cast<const THIS&>(*this);

      // Get the starting index based on the key hash                   
      const auto start = This.GetBucket(GetReserved() - 1, match);
      auto info = GetInfo() + start;
      if (not *info)
         return GetReserved();

      // Test first candidate                                           
      auto key = &GetRawKey<K>(start);
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

      key = &GetRawKey<K>(0);
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
   ///   @tparam THIS - map to interpret this one as, for optimal bucketing   
   ///   @param match - the key to search for                                 
   ///   @return the index, or mValues.mReserved if not found                 
   template<class THIS>
   Offset BlockMap::FindIndexUnknown(const Block& match) const {
      if (IsEmpty())
         return GetReserved();

      static_assert(CT::Map<THIS>, "THIS must be a map type");
      auto& This = reinterpret_cast<const THIS&>(*this);

      // Get the starting index based on the key hash                   
      const auto start = This.GetBucketUnknown(GetReserved() - 1, match);
      auto info = GetInfo() + start;
      if (not *info)
         return GetReserved();

      // Test first candidate                                           
      auto key = GetKeyInner(start);
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
            return static_cast<Offset>(index);

         ++info;
         key.Next();
      }

      // Reached only if info has reached the end                       
      // Keys might loop around, continue the search from the start     
      info = GetInfo();
      if (GetReserved() - *info > start)
         return GetReserved();

      key = GetKeyInner(0);
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
