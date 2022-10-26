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
         return Block {member.mType};

      return { 
         DataState::Member, member.mType, 
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
         if (trait && member.mTrait != trait)
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
         if (data && !member.mType->CastsTo(data))
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
         if (trait && member.mTrait != trait)
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
         if (data && !member.mType->CastsTo(data))
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

   /// Find first matching element(s) position inside container               
   /// This is a slow and tedious RTTI search                                 
   ///   @param item - block with a single item to search for                 
   ///   @param idx - index to start searching from                           
   ///   @return the index of the found item, or uiNone if not found          
   inline Index Block::FindRTTI(const Block& item, Index idx) const {
      if (item.IsEmpty())
         return IndexNone;

      // Setup the iterator                                             
      Index starti, istep;
      if (idx == IndexFront) {
         starti = 0;
         istep = 1;
      }
      else if (idx == IndexBack) {
         starti = mCount - 1;
         istep = -1;
      }
      else {
         starti = Constrain(idx).mIndex;
         istep = 1;
         if (starti + 1 >= mCount)
            return IndexNone;
      }

      // Compare all elements                                           
      for (Index i = starti; i < mCount && i >= 0; i += istep) {
         auto left = GetElementResolved(i.GetOffset());
         bool failure = false;
         for (Index j = 0; j < item.GetCount() && !failure && (i + istep * j) >= 0 && (i + istep * j) < mCount; ++j) {
            auto right = item.GetElementResolved(j.GetOffset());
            if (!left.Compare(right)) {
               failure = true;
               break;
            }
         }

         if (!failure)
            return i;
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }

} // namespace Langulus::Anyness

#undef VERBOSE