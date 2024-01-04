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

   /// Checks if both sets contain the same entries                           
   ///   @param other - the set to compare against, type-erased or not        
   ///   @return true if sets are the same                                    
   template<CT::Set THIS>
   bool BlockSet::operator == (CT::Set auto const& rhs) const {
      if (rhs.GetCount() != GetCount()
      or not IsTypeCompatibleWith<THIS>(rhs))
         return false;

      // If reached, then both sets contain similar types of data       
      using RHS = Conditional<CT::Typed<THIS>, THIS, Deref<decltype(rhs)>>;
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            // Compare each valid entry...                              
            const auto index = info - GetInfo();
            if constexpr (CT::Typed<RHS>) {
               // ...with known types, if any of the sets are typed     
               if (rhs.template FindInner<RHS>(GetRaw<RHS>(index)) == InvalidOffset)
                  return false;
            }
            else {
               // ...via rtti, since all sets are type-erased           
               if (rhs.FindInnerUnknown(GetInner(index)) == InvalidOffset)
                  return false;
            }
         }

         ++info;
      }

      // If reached, then both maps are the same                        
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
   template<CT::Set THIS> LANGULUS(INLINED)
   bool BlockSet::Contains(const CT::NotSemantic auto& key) const {
      return FindInner<THIS>(key) != InvalidOffset;
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   template<CT::Set THIS> LANGULUS(INLINED)
   Index BlockSet::Find(const CT::NotSemantic auto& key) const {
      const auto offset = FindInner<THIS>(key);
      return offset != InvalidOffset ? Index {offset} : IndexNone;
   }

   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<CT::Set THIS> LANGULUS(INLINED)
   BlockSet::Iterator BlockSet::FindIt(const CT::NotSemantic auto& key) {
      const auto offset = FindInner<THIS>(key);
      if (offset == InvalidOffset)
         return end();

      return {
         GetInfo() + offset, GetInfoEnd(),
         GetInner(offset)
      };
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<CT::Set THIS> LANGULUS(INLINED)
   BlockSet::ConstIterator BlockSet::FindIt(const CT::NotSemantic auto& key) const {
      return const_cast<BlockSet*>(this)->template FindIt<THIS>(key);
   }

   /// Find the index of a pair by key                                        
   /// The key may not match the contained key type                           
   ///   @param match - the key to search for                                 
   ///   @return the index, or InvalidOffset if not found                     
   template<CT::Set THIS>
   Offset BlockSet::FindInner(const CT::NotSemantic auto& match) const {
      using K = Deref<decltype(match)>;
      if (IsEmpty())
         return InvalidOffset;

      auto& me = reinterpret_cast<const THIS&>(*this);
      if constexpr (CT::StringLiteral<K>) {
         if (me.template IsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return FindInner<THIS>(Text {Disown(match)});
         }
         else if (me.template IsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return FindInner<THIS>(static_cast<const Deext<K>*>(match));
         }
         else return InvalidOffset;
      }
      else {
         if (not me.template IsSimilar<K>())
            return InvalidOffset;

         // Get the starting index based on the key hash                
         const auto start = GetBucket(GetReserved() - 1, match);
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

         key = &GetRaw<K>(0);
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
   
   /// Find the index of a pair by an unknown type-erased key                 
   ///   @attention assumes map is not empty                                  
   ///   @attention assumes keys are of the exactly same type                 
   ///   @param match - the key to search for                                 
   ///   @return the index, or InvalidOffset if not found                     
   inline Offset BlockSet::FindInnerUnknown(const Block& match) const {
      if (IsEmpty() or not IsSimilar(match.GetType()))
         return InvalidOffset;

      // Get the starting index based on the key hash                   
      const auto start = GetBucketUnknown(GetReserved() - 1, match);
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

      key = GetInner(0);
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
