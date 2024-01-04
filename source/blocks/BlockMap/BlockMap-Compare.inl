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

   /// Compare this map against another map, type-erased or not               
   ///   @param rhs - map to compare against                                  
   ///   @return true if contents of both maps are the same                   
   template<CT::Map THIS>
   bool BlockMap::operator == (CT::Map auto const& rhs) const {
      if (rhs.GetCount() != GetCount()
      or not IsTypeCompatibleWith<THIS>(rhs))
         return false;

      // If reached, then both maps contain similar types of data       
      using RHS = Conditional<CT::Typed<THIS>, THIS, Deref<decltype(rhs)>>;
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            // Compare each valid pair...                               
            const auto index = info - GetInfo();
            if constexpr (CT::Typed<RHS>) {
               // ...with known types, if any of the maps are typed     
               const auto r = rhs.template FindInner<RHS>(GetRawKey<RHS>(index));
               if (r == InvalidOffset
               or GetRawValue<RHS>(index) != rhs.GetRawValue<RHS>(r))
                  return false;
            }
            else {
               // ...via rtti, since all maps are type-erased           
               const auto r = rhs.FindInnerUnknown(GetKeyInner(index));
               if (r == InvalidOffset
               or GetValueInner(index) != rhs.GetValueInner(r))
                  return false;
            }
         }

         ++info;
      }

      // If reached, then both maps are the same                        
      return true;
   }

   /// Compare this map against a pair, type-erased or not                    
   ///   @param rhs - pair to compare against                                 
   ///   @return true this map contains only this exact pair                  
   template<CT::Map THIS>
   bool BlockMap::operator == (CT::Pair auto const& rhs) const {
      if (1 != GetCount() or not IsTypeCompatibleWith<THIS>(rhs))
         return false;

      // If reached, then pair contains similar types of data           
      using P = Deref<decltype(rhs)>;
      using RHS = Conditional<CT::Typed<THIS>, THIS, TMap<typename P::Key,
                                                          typename P::Value,
                                                          THIS::Ordered>>;
      if constexpr (CT::Typed<RHS>) {
         // ...with known types, if any of the maps are typed           
         const auto r = FindInner<RHS>(rhs.mKey);
         if (r == InvalidOffset or GetRawValue<RHS>(0) != rhs.mValue)
            return false;
      }
      else {
         // ...via rtti, since all maps are type-erased                 
         const auto r = FindInnerUnknown(rhs.mKey);
         if (r == InvalidOffset or GetValueInner(0) != rhs.mValue)
            return false;
      }

      // If reached, then map contains that exact pair                  
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
      using V = Deref<decltype(value)>;
      if (IsEmpty())
         return false;

      if constexpr (CT::Typed<THIS>) {
         // Search in a statically-typed map                            
         static_assert(::std::equality_comparable_with<V, typename THIS::Value>,
            "Provided value is not comparable to map's value type");
         auto test = &GetRawValue<THIS>(0);
         auto info = GetInfo();
         const auto infoEnd = GetInfoEnd();

         while (info != infoEnd) {
            if (*info and *test == value)
               return true;

            ++test; ++info;
         }
      }
      else {
         // Search in a type-erased map                                 
         if (not ValueIsSimilar<V>())
            return false;

         auto elem = &mValues.template Get<V>(0);
         auto info = GetInfo();
         const auto infoEnd = GetInfoEnd();

         while (info != infoEnd) {
            if (*info and *elem == value)
               return true;

            ++elem; ++info;
         }
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
