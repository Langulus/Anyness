///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Block.hpp"

#define VERBOSE(a) //pcLogFuncVerbose << a

namespace Langulus::Anyness
{

   /// Get the memory block corresponding to a local member variable          
   /// Never references data                                                  
   ///   @param member - the member to get                                    
   ///   @return a static memory block                                        
   inline Block Block::GetMember(const RTTI::Member& member) {
      if (!IsAllocated())
         return Block {RTTI::Database.GetMetaData(member.mType)};

      return { 
         DataState::Member, RTTI::Database.GetMetaData(member.mType),
         member.mCount, member.Get(mRaw)
      };
   }

   /// Get the memory Block corresponding to a local member variable (const)  
   ///   @param member - the member to get                                    
   ///   @return a static constant memory block                               
   inline Block Block::GetMember(const RTTI::Member& member) const {
      auto result = const_cast<Block*>(this)->GetMember(member);
      result.MakeConst();
      return result;
   }

   inline Block Block::GetMember(TMeta trait) const {
      // Scan members                                                   
      for (auto& member : mType->mMembers) {
         if (trait && member.mTrait != trait->mToken)
            continue;

         // Found one                                                   
         auto found = GetMember(member);
         VERBOSE("Selected " << GetToken() << "::" << member.mName
            << " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
               << "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
         );

         return found;
      }

      // No such trait found, so check in bases                         
      //TODO fix indices shadowing later bases
      for (auto& base : mType->mBases) {
         auto found = GetBaseMemory(base.mType, base).GetMember(trait);
         if (!found.IsUntyped())
            return found;
      }

      return {};
   }

   inline Block Block::GetMember(TMeta trait) {
      auto result = const_cast<const Block*>(this)->GetMember(trait);
      result.MakeConst();
      return result;
   }

   inline Block Block::GetMember(DMeta data) const {
      // Scan members                                                   
      for (auto& member : mType->mMembers) {
         if (data && !RTTI::Database.GetMetaData(member.mType)->CastsTo(data))
            continue;

         // Found one                                                   
         auto found = GetMember(member);
         VERBOSE("Selected " << GetToken() << "::" << member.mName
            << " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
               << "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
         );

         return found;
      }

      // No such data found, so check in bases                          
      //TODO fix indices shadowing later bases
      for (auto& base : mType->mBases) {
         auto found = GetBaseMemory(base.mType, base).GetMember(data);
         if (!found.IsUntyped())
            return found;
      }

      return {};
   }

   inline Block Block::GetMember(DMeta data) {
      auto result = const_cast<const Block*>(this)->GetMember(data);
      result.MakeConst();
      return result;
   }

   inline Block Block::GetMember(std::nullptr_t) const {
      if (mType->mMembers.empty())
         return {};
      return GetMember(mType->mMembers[0]);
   }

   inline Block Block::GetMember(std::nullptr_t) {
      if (mType->mMembers.empty())
         return {};
      return GetMember(mType->mMembers[0]);
   }

   /// Select a member Block via trait or index (or both)                     
   ///   @param trait - the trait to get                                      
   ///   @param index - the trait index to get                                
   ///   @return a static memory block (constant if block is constant)        
   template<CT::Index INDEX>
   Block Block::GetMember(TMeta trait, const INDEX& index) {
      Offset offset;
      if constexpr (CT::Same<INDEX, Index>)
         offset = index.Constrained(mType->GetMemberCount()).GetOffset();
      else if constexpr (CT::Signed<INDEX>) {
         if (index < 0)
            offset = mType->GetMemberCount() - static_cast<Offset>(-index);
         else
            offset = static_cast<Offset>(index);
      }
      else offset = index;

      // Scan members                                                   
      Offset counter = 0;
      for (auto& member : mType->mMembers) {
         if (trait && member.mTrait != trait->mToken)
            continue;

         // Matched, but check index first                              
         if (counter < offset) {
            ++counter;
            continue;
         }

         // Found one                                                   
         auto found = GetMember(member);
         VERBOSE("Selected " << GetToken() << "::" << member.mName
            << " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
               << "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
         );

         return found;
      }

      // No such trait found, so check in bases                         
      //TODO fix indices shadowing later bases
      offset -= counter;
      for (auto& base : mType->mBases) {
         auto found = GetBaseMemory(base.mType, base).GetMember(trait, offset);
         if (!found.IsUntyped())
            return found;
      }

      return {};
   }

   /// Select a member Block via trait or index (or both) (const)             
   ///   @param trait - the trait to get                                      
   ///   @param index - the trait index to get                                
   ///   @return a static constant memory block                               
   template<CT::Index INDEX>
   Block Block::GetMember(TMeta trait, const INDEX& index) const {
      auto result = const_cast<Block*>(this)->GetMember(trait, index);
      result.MakeConst();
      return result;
   }
   
   /// Select a member Block via type or index (or both)                      
   ///   @param data - the type to get                                        
   ///   @param index - the member index to get                               
   ///   @return a static memory block (constant if block is constant)        
   template<CT::Index INDEX>
   Block Block::GetMember(DMeta data, const INDEX& index) {
      Offset offset;
      if constexpr (CT::Same<INDEX, Index>)
         offset = index.Constrained(mType->GetMemberCount()).GetOffset();
      else if constexpr (CT::Signed<INDEX>) {
         if (index < 0)
            offset = mType->GetMemberCount() - static_cast<Offset>(-index);
         else
            offset = static_cast<Offset>(index);
      }
      else offset = index;

      // Scan members                                                   
      Offset counter = 0;
      for (auto& member : mType->mMembers) {
         if (data && !RTTI::Database.GetMetaData(member.mType)->CastsTo(data))
            continue;

         // Matched, but check index first                              
         if (counter < offset) {
            ++counter;
            continue;
         }

         // Found one                                                   
         auto found = GetMember(member);
         VERBOSE("Selected " << GetToken() << "::" << member.mName
            << " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
            << "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
         );

         return found;
      }

      // No such data found, so check in bases                          
      //TODO fix indices shadowing later bases
      offset -= counter;
      for (auto& base : mType->mBases) {
         auto found = GetBaseMemory(base.mType, base).GetMember(data, offset);
         if (!found.IsUntyped())
            return found;
      }

      return {};
   }

   /// Select a member via data type or index (or both) (const)               
   /// Never references data                                                  
   ///   @param data - the type to get                                        
   ///   @param index - the trait index to get                                
   ///   @return a static constant memory block                               
   template<CT::Index INDEX>
   Block Block::GetMember(DMeta data, const INDEX& index) const {
      auto result = const_cast<Block*>(this)->GetMember(data, index);
      result.MakeConst();
      return result;
   }

   /// Select a member via type or index (or both)                            
   /// Never references data                                                  
   ///   @param data - the type to get                                        
   ///   @param index - the member index to get                               
   ///   @return a static memory block (constant if block is constant)        
   template<CT::Index INDEX>
   Block Block::GetMember(std::nullptr_t, const INDEX& index) {
      Offset offset;
      if constexpr (CT::Same<INDEX, Index>)
         offset = index.Constrained(mType->GetMemberCount()).GetOffset();
      else if constexpr (CT::Signed<INDEX>) {
         if (index < 0)
            offset = mType->GetMemberCount() - static_cast<Offset>(-index);
         else
            offset = static_cast<Offset>(index);
      }
      else offset = index;

      if (offset < mType->mMembers.size()) {
         auto& member = mType->mMembers[offset];
         auto found = GetMember(member);
         VERBOSE("Selected " << GetToken() << "::" << member.mName
            << " (" << member.mType << (member.mCount > 1 ? (pcLog << "[" << member.mCount
            << "]") : (pcLog << "")) << ", with current value(s) " << found << ")"
         );

         return found;
      }

      // No such data found, so check in bases                          
      //TODO fix indices shadowing later bases
      offset -= mType->mMembers.size();
      for (auto& base : mType->mBases) {
         auto found = GetBaseMemory(base.mType, base).GetMember(nullptr, offset);
         if (!found.IsUntyped())
            return found;
      }

      return {};
   }

   /// Select a member via data type or index (or both) (const)               
   /// Never references data                                                  
   ///   @param data - the type to get                                        
   ///   @param index - the trait index to get                                
   ///   @return a static constant memory block                               
   template<CT::Index INDEX>
   Block Block::GetMember(std::nullptr_t, const INDEX& index) const {
      auto result = const_cast<Block*>(this)->GetMember(nullptr, index);
      result.MakeConst();
      return result;
   }

   /// Find an element of unknown type                                        
   ///   @attention assumes item contains exactly one element                 
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @tparam BY_ADDRESS_ONLY - true to compare addresses only             
   ///   @param item - block with a single item to search for                 
   ///   @param cookie - continue search from a given offset                  
   ///   @return the index of the found item, or IndexNone if not found       
   template<bool REVERSE, bool BY_ADDRESS_ONLY>
   Index Block::FindUnknown(const Block& item, const Offset& cookie) const {
      LANGULUS_ASSUME(UserAssumes, item.GetCount() == 1,
         "You can search exactly one item");

      auto right = item.GetElementResolved(0);
      if constexpr (!REVERSE) {
         for (Offset i = cookie; i < mCount; ++i) {
            const auto left = GetElementResolved(i);
            if constexpr (BY_ADDRESS_ONLY) {
               if (left.mRaw == right.mRaw)
                  return {i}; // Found by pointer                       
            }
            else {
               if (left.Compare(right))
                  return {i}; // Found by value                         
            }
         }
      }
      else {
         for (Offset i = mCount - 1 - cookie; i < mCount; --i) {
            const auto left = GetElementResolved(i);
            if constexpr (BY_ADDRESS_ONLY) {
               if (left.mRaw == right.mRaw)
                  return {i}; // Found by pointer                       
            }
            else {
               if (left.Compare(right))
                  return {i}; // Found by value                         
            }
         }
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }

   /// Find first matching element position inside container                  
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @tparam BY_ADDRESS_ONLY - true to compare addresses only             
   ///   @param item - the item to search for                                 
   ///   @param cookie - continue search from a given offset                  
   ///   @return the index of the found item, or IndexNone if not found       
   template<bool REVERSE, bool BY_ADDRESS_ONLY, CT::Data T>
   Index Block::FindKnown(const T& item, const Offset& cookie) const {
      if constexpr (!REVERSE) {
         for (Offset i = cookie; i < mCount; ++i) {
            if (GetElement(i) == item)
               return i;
         }
      }
      else {
         for (Offset i = mCount - 1 - cookie; i < mCount; --i) {
            if (GetElement(i) == item)
               return i;
         }
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }
   
   /// Find first matching element position inside container, deeply          
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @param idx - index to start searching from                           
   ///   @return the index of the found item, or IndexNone if not found       
   template<bool REVERSE, CT::Data T>
   Index Block::FindDeep(const T& item, Offset cookie) const {
      Index found;
      if constexpr (!REVERSE) {
         ForEachDeep([&](const Block& group) {
            if (cookie) {
               --cookie;
               return true;
            }

            found = group.template FindKnown<REVERSE>(item);
            return !found;
         });
      }
      else {
         ForEachDeepRev([&](const Block& group) {
            if (cookie) {
               --cookie;
               return true;
            }

            found = group.template FindKnown<REVERSE>(item);
            return !found;
         });
      }

      return found;
   }

} // namespace Langulus::Anyness

#undef VERBOSE