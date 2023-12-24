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
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   template<class SET> LANGULUS(INLINED)
   bool BlockSet::Contains(const CT::NotSemantic auto& key) const {
      return FindInner<SET>(key) != InvalidOffset;
   }

   /// Search for a key inside the table, and return it if found              
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   template<class SET> LANGULUS(INLINED)
   Index BlockSet::Find(const CT::NotSemantic auto& key) const {
      const auto offset = FindInner<SET>(key);
      return offset != InvalidOffset ? Index {offset} : IndexNone;
   }

   /// Search for a key inside the table, and return an iterator to it        
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<class SET> LANGULUS(INLINED)
   BlockSet::Iterator BlockSet::FindIt(const CT::NotSemantic auto& key) {
      const auto offset = FindInner<SET>(key);
      if (offset == InvalidOffset)
         return end();

      return {
         GetInfo() + offset, GetInfoEnd(),
         GetInner(offset)
      };
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   template<class SET> LANGULUS(INLINED)
   BlockSet::ConstIterator BlockSet::FindIt(const CT::NotSemantic auto& key) const {
      return const_cast<BlockSet*>(this)->template FindIt<SET>(key);
   }

   /// Find the index of a pair by key                                        
   /// The key may not match the contained key type                           
   ///   @tparam SET - set we're searching in, potentially providing runtime  
   ///                 optimization on type checks                            
   ///   @param match - the key to search for                                 
   ///   @return the index, or InvalidOffset if not found                     
   template<class SET>
   Offset BlockSet::FindInner(const CT::NotSemantic auto& match) const {
      using K = Deref<decltype(match)>;
      if (IsEmpty())
         return InvalidOffset;

      static_assert(CT::Set<SET>, "SET must be a set type");
      auto& THIS = reinterpret_cast<const SET&>(*this);
      if constexpr (CT::Array<K> and CT::ExactAsOneOf<Decvq<Deext<K>>, char, wchar_t>) {
         if (THIS.template IsSimilar<Text>()) {
            // Implicitly make a text container on string literal       
            return FindInner<SET>(Text {Disown(match)});
         }
         else if (THIS.template IsSimilar<char*, wchar_t*>()) {
            // Cast away the extent, search for pointer                 
            return FindInner<SET>(static_cast<const Deext<K>*>(match));
         }
         else return InvalidOffset;
      }
      else {
         if (not THIS.template IsSimilar<K>())
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
