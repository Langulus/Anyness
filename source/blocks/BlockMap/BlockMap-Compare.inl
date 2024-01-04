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
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   template<CT::Map THIS> LANGULUS(INLINED)
   bool BlockMap::ContainsKey(const CT::NotSemantic auto& key) const {
      return FindInner<THIS>(key) != InvalidOffset;
   }

   /// Search for a value inside the table                                    
   ///   @param value - the value to search for                               
   ///   @return true if value is found, false otherwise                      
   template<CT::Map THIS> LANGULUS(INLINED)
   bool BlockMap::ContainsValue(const CT::NotSemantic auto& value) const {
      if (IsEmpty())
         return false;

      auto& me = reinterpret_cast<const THIS&>(*this);
      using V = Decvq<Deref<decltype(value)>>;
      if (not me.template ValueIsSimilar<V>()
      and not (CT::Typed<THIS> and ::std::equality_comparable_with<typename THIS::Value, V>))
         return false;

      auto elem = &GetRawValue<V>(0);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();

      while (info != infoEnd) {
         if (*info and *elem == value)
            return true;

         ++elem; ++info;
      }

      return false;
   }

   /// Search for a pair inside the table                                     
   ///   @param pair - the pair to search for                                 
   ///   @return true if pair is found, false otherwise                       
   template<CT::Map THIS> LANGULUS(INLINED)
   bool BlockMap::ContainsPair(const CT::Pair auto& pair) const {
      if (IsEmpty())
         return false;

      using P = Deref<decltype(pair)>;
      auto& me = reinterpret_cast<THIS&>(*this);

      if constexpr (CT::TypedPair<P>) {
         using V = typename P::Value;

         if (not me.template ValueIsSimilar<V>()
         and not (CT::Typed<THIS> and ::std::equality_comparable_with<typename THIS::Value, V>))
            return false;

         const auto found = FindInner<THIS>(pair.mKey);
         return found != InvalidOffset and me.GetValueInner(found) == pair.mValue;
      }
      else {
         if (not me.ValueIsSimilar(pair.GetValueType()))
            return false;

         const auto found = FindInnerUnknown<THIS>(pair.mKey);
         return found != InvalidOffset and me.GetValueInner(found) == pair.mValue;
      }
   }
   
   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   template<CT::Map THIS> LANGULUS(INLINED)
   Index BlockMap::Find(const CT::NotSemantic auto& key) const {
      const auto offset = FindInner<THIS>(key);
      return offset != InvalidOffset ? Index {offset} : IndexNone;
   }

   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::Iterator BlockMap::FindIt(const CT::NotSemantic auto& key) {
      const auto offset = FindInner<THIS>(key);
      if (offset == InvalidOffset)
         return end();

      return {
         GetInfo() + offset, GetInfoEnd(),
         GetKeyInner(offset),
         GetValueInner(offset)
      };
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<CT::Map THIS> LANGULUS(INLINED)
   BlockMap::ConstIterator BlockMap::FindIt(const CT::NotSemantic auto& key) const {
      return const_cast<BlockMap*>(this)->template FindIt<THIS>(key);
   }

   /// Returns a reference to the value found for key                         
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return the value, wrapped in a type-erased block                    
   template<CT::Map THIS> LANGULUS(INLINED)
   Block BlockMap::At(const CT::NotSemantic auto& key) {
      const auto found = FindInner<THIS>(key);
      LANGULUS_ASSERT(found != InvalidOffset, OutOfRange, "Key not found");
      return GetValueInner(found);
   }
   
   /// Returns a reference to the value found for key (const)                 
   /// Throws Except::OutOfRange if element cannot be found                   
   ///   @param key - the key to search for                                   
   ///   @return the value, wrapped in a type-erased block                    
   template<CT::Map THIS> LANGULUS(INLINED)
   Block BlockMap::At(const CT::NotSemantic auto& key) const {
      return const_cast<BlockMap*>(this)->template At<THIS>(key);
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
   ///   @param match - the key to search for                                 
   ///   @return the index, or InvalidOffset if not found                     
   template<CT::Map THIS>
   Offset BlockMap::FindInner(const CT::NotSemantic auto& match) const {
      if (IsEmpty())
         return InvalidOffset;

      using K = Deref<decltype(match)>;
      auto& me = reinterpret_cast<const THIS&>(*this);

      if constexpr (CT::StringLiteral<K>) {
         if (me.template KeyIsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return FindInner<THIS>(Text {Disown(match)});
         }
         else if (me.template KeyIsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return FindInner<THIS>(static_cast<const Deext<K>*>(match));
         }
         else return InvalidOffset;
      }
      else {
         if (not me.template KeyIsSimilar<K>())
            return InvalidOffset;

         // Get the starting index based on the key hash                
         const auto start = GetBucket(GetReserved() - 1, match);
         auto info = GetInfo() + start;
         if (not *info)
            return InvalidOffset;

         // Test first candidate                                        
         auto key = &GetRawKey<THIS>(start);
         if (*key == match)
            return start;

         // Test all candidates on the right up until the end           
         ++key;
         ++info;

         auto infoEnd = GetInfoEnd();
         while (info != infoEnd) {
            if (not *info)
               return InvalidOffset;

            if (*key == match)
               return info - GetInfo();

            ++key; ++info;
         }

         // Reached only if info has reached the end                    
         // Keys might loop around, continue the search from the start  
         info = GetInfo();
         if (not *info)
            return InvalidOffset;

         key = &GetRawKey<THIS>(0);
         if (*key == match)
            return 0;

         ++key;
         ++info;

         infoEnd = GetInfo() + start;
         while (info != infoEnd) {
            if (not *info)
               return InvalidOffset;

            if (*key == match)
               return info - GetInfo();

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

      auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (not *info)
            return InvalidOffset;

         if (key == match)
            return info - GetInfo();

         ++info;
         key.Next();
      }

      // Reached only if info has reached the end                       
      // Keys might loop around, continue the search from the start     
      info = GetInfo();
      if (not *info)
         return InvalidOffset;

      key = GetKeyInner(0);
      if (key == match)
         return 0;

      key.Next();
      ++info;

      infoEnd = GetInfo() + start;
      while (info != infoEnd) {
         if (not *info)
            return InvalidOffset;

         if (key == match)
            return info - GetInfo();

         ++info;
         key.Next();
      }
      
      // No such key was found                                          
      return InvalidOffset;
   }
   
} // namespace Langulus::Anyness
