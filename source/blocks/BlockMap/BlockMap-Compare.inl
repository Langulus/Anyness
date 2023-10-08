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
            const auto rhs = other.FindInnerUnknown(GetKeyInner(lhs));
            if (rhs == InvalidOffset or GetValueInner(lhs) != other.GetValueInner(rhs))
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
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   template<class MAP>
   LANGULUS(INLINED)
   bool BlockMap::ContainsKey(const CT::NotSemantic auto& key) const {
      return FindInner<MAP>(key) != InvalidOffset;
   }

   /// Search for a value inside the table                                    
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param value - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   template<class MAP>
   LANGULUS(INLINED)
   bool BlockMap::ContainsValue(const CT::NotSemantic auto& value) const {
      static_assert(CT::Map<MAP>, "MAP must be a map type");
      auto& THIS = reinterpret_cast<const MAP&>(*this);
      using V = Decvq<Deref<decltype(value)>>;
      if (IsEmpty() or not THIS.template ValueIsSimilar<V>())
         return false;

      auto elem = &GetRawValue<V>(0);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();

      while (info != infoEnd) {
         if (*info and value == *elem)
            return true;

         ++elem; ++info;
      }

      return false;
   }

   /// Search for a pair inside the table                                     
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @tparam K - key type                                                 
   ///   @tparam V - value type                                               
   ///   @param pair - the pair to search for                                 
   ///   @return true if pair is found, false otherwise                       
   template<class MAP, CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(INLINED)
   bool BlockMap::ContainsPair(const TPair<K, V>& pair) const {
      static_assert(CT::Map<MAP>, "MAP must be a map type");
      auto& THIS = reinterpret_cast<MAP&>(*this);
      if (IsEmpty() or not THIS.template ValueIsSimilar<V>())
         return false;

      const auto found = FindInner<MAP>(pair.mKey);
      return found != InvalidOffset and GetValueInner(found) == pair.mValue;
   }
   
   /// Search for a key inside the table, and return it if found              
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   template<class MAP>
   LANGULUS(INLINED)
   Index BlockMap::Find(const CT::NotSemantic auto& key) const {
      const auto offset = FindInner<MAP>(key);
      return offset != InvalidOffset ? Index {offset} : IndexNone;
   }

   /// Search for a key inside the table, and return an iterator to it        
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<class MAP>
   LANGULUS(INLINED)
   BlockMap::Iterator BlockMap::FindIt(const CT::NotSemantic auto& key) {
      const auto offset = FindInner<MAP>(key);
      if (offset == InvalidOffset)
         return end();

      return {
         GetInfo() + offset, GetInfoEnd(),
         GetKeyInner(offset),
         GetValueInner(offset)
      };
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<class MAP>
   LANGULUS(INLINED)
   BlockMap::ConstIterator BlockMap::FindIt(const CT::NotSemantic auto& key) const {
      return const_cast<BlockMap*>(this)->template FindIt<MAP>(key);
   }

   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the value, wrapped in a type-erased block                    
   template<class MAP>
   LANGULUS(INLINED)
   Block BlockMap::At(const CT::NotSemantic auto& key) {
      const auto found = FindInner<MAP>(key);
      LANGULUS_ASSERT(found != InvalidOffset, OutOfRange, "Key not found");
      return GetValueInner(found);
   }
   
   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the value, wrapped in a type-erased block                    
   template<class MAP>
   LANGULUS(INLINED)
   Block BlockMap::At(const CT::NotSemantic auto& key) const {
      return const_cast<BlockMap*>(this)->template At<MAP>(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return the value, wrapped in a type-erased block                    
   LANGULUS(INLINED)
   Block BlockMap::operator[] (const CT::NotSemantic auto& key) {
      return At(key);
   }

   /// Access value by key                                                    
   ///   @param key - the key to find                                         
   ///   @return the value, wrapped in a type-erased block                    
   LANGULUS(INLINED)
   Block BlockMap::operator[] (const CT::NotSemantic auto& key) const {
      return At(key);
   }

   /// Find the index of a pair by key                                        
   /// The key may not match the contained key type                           
   ///   @tparam MAP - map we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param match - the key to search for                                 
   ///   @return the index, or InvalidOffset if not found                     
   template<class MAP>
   Offset BlockMap::FindInner(const CT::NotSemantic auto& match) const {
      using K = Deref<decltype(match)>;
      if (IsEmpty())
         return InvalidOffset;

      static_assert(CT::Map<MAP>, "MAP must be a map type");
      auto& THIS = reinterpret_cast<const MAP&>(*this);
      if constexpr (CT::Array<K> and CT::ExactAsOneOf<Decvq<Deext<K>>, char, wchar_t>) {
         if (THIS.template KeyIsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return FindInner<MAP>(Text {Disown(match)});
         }
         else if (THIS.template KeyIsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return FindInner<MAP>(static_cast<const Deext<K>*>(match));
         }
         else return 0;
      }
      else {
         if (not THIS.template KeyIsSimilar<K>())
            return InvalidOffset;

         // Get the starting index based on the key hash                
         const auto start = GetBucket(GetReserved() - 1, match);
         auto info = GetInfo() + start;
         if (not *info)
            return InvalidOffset;

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

         key = &GetRawKey<K>(0);
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

         // No such key was found                                       
         return InvalidOffset;
      }
   }
   
   /// Find the index of a pair by a type-erased key                          
   ///   @param match - the key to search for                                 
   ///   @return the index, or InvalidOffset if not found                     
   inline Offset BlockMap::FindInnerUnknown(const Block& match) const {
      if (IsEmpty() or not KeyIsSimilar(match.GetType()))
         return InvalidOffset;

      // Get the starting index based on the key hash                   
      const auto start = GetBucketUnknown(GetReserved() - 1, match);
      auto info = GetInfo() + start;
      if (not *info)
         return InvalidOffset;

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

      key = GetKeyInner(0);
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
