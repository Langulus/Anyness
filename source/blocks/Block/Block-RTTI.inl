///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Block.hpp"

namespace Langulus::Anyness
{

   /// Check if contained data loosely matches a given type, ignoring         
   /// density and constness                                                  
   ///   @param type - the type to check for                                  
   ///   @return true if this block contains similar data                     
   LANGULUS(INLINED)
   bool Block::Is(DMeta type) const noexcept {
      return mType == type or (mType and mType->Is(type));
   }

   /// Check if this container's data is similar to one of the listed types,  
   /// ignoring density and constness                                         
   ///   @tparam T... - the types to compare against                          
   ///   @return true if data type is similar to at least one of the types    
   template<CT::Data... T>
   LANGULUS(INLINED)
   bool Block::Is() const {
      return (Is(MetaData::Of<T>()) or ...);
   }

   /// Check if this container's data is exactly as one of the listed types,  
   /// including by density and constness                                     
   ///   @tparam T... - the types to compare against                          
   ///   @return true if data type exactly matches at least one type          
   template<CT::Data... T>
   LANGULUS(INLINED)
   bool Block::IsExact() const {
      return (IsExact(MetaData::Of<T>()) or ...);
   }

   /// Check if this container's data is exactly the provided type,           
   /// including the density and constness                                    
   ///   @param type - the type to match                                      
   ///   @return true if data type matches type exactly                       
   LANGULUS(INLINED)
   bool Block::IsExact(DMeta type) const noexcept {
      return mType == type or (mType and mType->IsExact(type));
   }

   /// Check if contained data can be interpreted as a given type             
   ///   @attention direction matters, if block is dense                      
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current type to 'type'             
   template<bool BINARY_COMPATIBLE>
   LANGULUS(INLINED)
   bool Block::CastsToMeta(DMeta type) const {
      return mType and (mType->mIsSparse
         ? mType->CastsTo<true>(type)
         : mType->CastsTo(type));
   }

   /// Check if contained data can be interpreted as a number of a type       
   /// For example: a Vec4 can interpret as float[4]                          
   ///   @attention direction matters, if block is dense                      
   ///   @param type - the type check if current type interprets to           
   ///   @param count - the number of elements to interpret as                
   ///   @return true if able to interpret current type to 'type'             
   template<bool BINARY_COMPATIBLE>
   LANGULUS(INLINED)
   bool Block::CastsToMeta(DMeta type, Count count) const {
      return not mType or not type or mType->CastsTo(type, count);
   }

   /// Check if this container's data can be represented as type T            
   /// with nothing more than pointer arithmetic                              
   ///   @tparam T - the type to compare against                              
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @return true if contained data is reinterpretable as T               
   template<CT::Data T, bool BINARY_COMPATIBLE>
   LANGULUS(INLINED)
   bool Block::CastsTo() const {
      return CastsToMeta<BINARY_COMPATIBLE>(MetaData::Of<T>());
   }

   /// Check if this container's data can be represented as a specific number 
   /// of elements of type T, with nothing more than pointer arithmetic       
   ///   @tparam T - the type to compare against                              
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @param count - the number of elements of T                           
   ///   @return true if contained data is reinterpretable as T               
   template<CT::Data T, bool BINARY_COMPATIBLE>
   LANGULUS(INLINED)
   bool Block::CastsTo(Count count) const {
      return CastsToMeta<BINARY_COMPATIBLE>(MetaData::Of<T>(), count);
   }
   
   /// Reinterpret contents of this Block as the type and state of another    
   /// You can interpret Vec4 as float[4] for example, or any other such      
   /// reinterpretation, as long as data remains tightly packed               
   ///   @param pattern - the type of data to try interpreting as             
   ///   @return a block representing this block, interpreted as the pattern  
   inline Block Block::ReinterpretAs(const Block& pattern) const {
      if (IsEmpty() or IsSparse() or IsUntyped() or pattern.IsUntyped())
         return {};

      RTTI::Base common {};
      if (not CompareTypes(pattern, common) or not common.mBinaryCompatible)
         return {};

      const Size baseBytes = (common.mType->mSize * common.mCount)
         / pattern.GetStride();
      const Size resultSize = pattern.IsEmpty()
         ? baseBytes : (baseBytes / pattern.mCount) * pattern.mCount;

      return {
         pattern.mState + DataState::Static,
         pattern.mType, resultSize, 
         mRaw, mEntry
      };
   }

   /// Reinterpret contents of this Block as a collection of a static type    
   /// You can interpret Vec4 as float[4] for example, or any other such      
   /// reinterpretation, as long as data remains tightly packed               
   ///   @tparam T - the type of data to try interpreting as                  
   ///   @return a block representing this block, interpreted as T            
   template<CT::Data T>
   LANGULUS(INLINED)
   Block Block::ReinterpretAs() const {
      static_assert(CT::Dense<T>, "T must be dense");
      return ReinterpretAs(Block::From<T>());
   }

   /// Get the memory block corresponding to a local member variable          
   ///   @attention assumes block is not empty                                
   ///   @param member - the member to get                                    
   ///   @return a static memory block                                        
   LANGULUS(INLINED)
   Block Block::GetMember(const RTTI::Member& member) {
      return { 
         DataState::Member, member.GetType(),
         member.mCount, member.Get(mRaw),
         mEntry
      };
   }

   /// Get the memory Block corresponding to a local member variable (const)  
   ///   @attention assumes block is not empty                                
   ///   @param member - the member to get                                    
   ///   @return a static constant memory block                               
   LANGULUS(INLINED)
   Block Block::GetMember(const RTTI::Member& member) const {
      auto result = const_cast<Block*>(this)->GetMember(member);
      result.MakeConst();
      return result;
   }
   
   /// Get a trait-tagged member from first element inside block              
   ///   @attention assumes block is not empty                                
   ///   @param trait - the trait tag to search for                           
   ///   @return the static mutable block corresponding to that member        
   inline Block Block::GetMember(TMeta trait) {
      // Scan members                                                   
      for (auto& member : mType->mMembers) {
         if (member.TraitIs(trait))
            return GetMember(member);
      }

      // No such trait found, so check in bases                         
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember(trait);
         if (found.IsTyped())
            return found;
      }

      return {};
   }

   /// Get a trait-tagged member from first element inside block (const)      
   ///   @attention assumes block is not empty                                
   ///   @param trait - the trait tag to search for                           
   ///   @return the static constant block corresponding to that member       
   LANGULUS(INLINED)
   Block Block::GetMember(TMeta trait) const {
      auto result = const_cast<Block*>(this)->GetMember(trait);
      result.MakeConst();
      return result;
   }
   
   /// Get a member of specific type, from first element inside block         
   ///   @attention assumes block is not empty                                
   ///   @param data - the type to search for                                 
   ///   @return the static block corresponding to that member                
   inline Block Block::GetMember(DMeta data) {
      // Scan members                                                   
      for (auto& member : mType->mMembers) {
         if (member.GetType()->CastsTo(data))
            return GetMember(member);
      }

      // No such data found, so check in bases                          
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember(data);
         if (found.IsTyped())
            return found;
      }

      return {};
   }

   /// Get a member of specific type, from first element inside block (const) 
   ///   @attention assumes block is not empty                                
   ///   @param data - the type to search for                                 
   ///   @return the static constant block corresponding to that member       
   LANGULUS(INLINED)
   Block Block::GetMember(DMeta data) const {
      auto result = const_cast<Block*>(this)->GetMember(data);
      result.MakeConst();
      return result;
   }

   /// Get the first member of the first element inside block                 
   ///   @attention assumes block is not empty                                
   ///   @return the static mutable block corresponding to that member        
   LANGULUS(INLINED)
   Block Block::GetMember(::std::nullptr_t) {
      if (mType->mMembers.empty())
         return {};
      return GetMember(mType->mMembers[0]);
   }

   /// Get the first member of the first element inside block (const)         
   ///   @attention assumes block is not empty                                
   ///   @return the static constant block corresponding to that member       
   LANGULUS(INLINED)
   Block Block::GetMember(::std::nullptr_t) const {
      if (mType->mMembers.empty())
         return {};
      return GetMember(mType->mMembers[0]);
   }
      
   /// Select a member by trait or index (or both)                            
   ///   @attention assumes block is not empty                                
   ///   @tparam INDEX - type of index (deducible)                            
   ///   @param trait - the trait to get                                      
   ///   @param index - the trait index to get                                
   ///   @return the static memory block of the member                        
   template<CT::Index INDEX>
   Block Block::GetMember(TMeta trait, const INDEX& index) {
      // Scan immediate members                                         
      Offset offset = SimplifyMemberIndex(index);
      Offset counter = 0;
      for (auto& member : mType->mMembers) {
         if (trait and not member.TraitIs(trait))
            continue;

         // Matched, but check index first                              
         if (counter < offset) {
            ++counter;
            continue;
         }

         // Found                                                       
         return GetMember(member);
      }

      // If reached, then nothing found in local members, so check bases
      offset -= counter;
      for (auto& base : mType->mBases) {
         auto found = GetBaseMemory(base.mType, base)
            .GetMember(trait, offset);
         if (found.IsTyped())
            return found;

         offset -= base.mType->GetMemberCount();
      }

      return {};
   }

   /// Select a member by trait or index (or both) (const)                    
   ///   @attention assumes block is not empty                                
   ///   @tparam INDEX - type of index (deducible)                            
   ///   @param trait - the trait to get                                      
   ///   @param index - the trait index to get                                
   ///   @return the static constant memory block of the member               
   template<CT::Index INDEX>
   LANGULUS(INLINED)
   Block Block::GetMember(TMeta trait, const INDEX& index) const {
      auto result = const_cast<Block*>(this)->GetMember(trait, index);
      result.MakeConst();
      return result;
   }
   
   /// Select a member by data type or index (or both)                        
   ///   @attention assumes block is not empty                                
   ///   @tparam INDEX - type of index (deducible)                            
   ///   @param data - the type to search for                                 
   ///   @param index - the member index to get                               
   ///   @return the static memory block of the member                        
   template<CT::Index INDEX>
   Block Block::GetMember(DMeta data, const INDEX& index) {
      // Scan immediate members                                         
      Offset offset = SimplifyMemberIndex(index);
      Offset counter = 0;
      for (auto& member : mType->mMembers) {
         if (data and not member.GetType()->CastsTo(data))
            continue;

         // Matched, but check index first                              
         if (counter < offset) {
            ++counter;
            continue;
         }

         // Found                                                       
         return GetMember(member);
      }

      // If reached, then nothing found in local members, so check bases
      offset -= counter;
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember(data, offset);
         if (found.IsTyped())
            return found;

         offset -= base.mType->GetMemberCount();
      }

      return {};
   }

   /// Select a member by data type or index (or both) (const)                
   ///   @attention assumes block is not empty                                
   ///   @tparam INDEX - type of index (deducible)                            
   ///   @param data - the type to search for                                 
   ///   @param index - the trait index to get                                
   ///   @return the static constant memory block of the member               
   template<CT::Index INDEX>
   LANGULUS(INLINED)
   Block Block::GetMember(DMeta data, const INDEX& index) const {
      auto result = const_cast<Block*>(this)->GetMember(data, index);
      result.MakeConst();
      return result;
   }

   /// Select a member by index only                                          
   ///   @attention assumes block is not empty                                
   ///   @tparam INDEX - type of index (deducible)                            
   ///   @param index - the member index to get                               
   ///   @return the static memory block of the member                        
   template<CT::Index INDEX>
   Block Block::GetMember(std::nullptr_t, const INDEX& index) {
      // Check immediate members first                                  
      Offset offset = SimplifyMemberIndex(index);
      if (offset < mType->mMembers.size())
         return GetMember(mType->mMembers[offset]);

      // If reached, then nothing found in local members, so check bases
      offset -= mType->mMembers.size();
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember(nullptr, offset);
         if (found.IsTyped())
            return found;

         offset -= base.mType->GetMemberCount();
      }

      return {};
   }

   /// Select a member by index only                                          
   ///   @attention assumes block is not empty                                
   ///   @tparam INDEX - type of index (deducible)                            
   ///   @param index - the member index to get                               
   ///   @return the static constant memory block of the member               
   template<CT::Index INDEX>
   LANGULUS(INLINED)
   Block Block::GetMember(std::nullptr_t, const INDEX& index) const {
      auto result = const_cast<Block*>(this)->GetMember(nullptr, index);
      result.MakeConst();
      return result;
   }

   /// Get the memory block corresponding to a base                           
   ///   @attention assumes block is not empty                                
   ///   @param meta - the type of the resulting base block                   
   ///   @param base - the base reflection to use                             
   ///   @return the static block for the base                                
   LANGULUS(INLINED)
   Block Block::GetBaseMemory(DMeta meta, const RTTI::Base& base) {
      return {
         DataState::Member, meta,
         base.mCount * (base.mBinaryCompatible ? GetCount() : 1),
         mRaw + base.mOffset, mEntry
      };
   }

   /// Get the memory block corresponding to a base (const)                   
   ///   @attention assumes block is not empty                                
   ///   @param meta - the type of the resulting base block                   
   ///   @param base - the base reflection to use                             
   ///   @return the static immutable block for the base                      
   LANGULUS(INLINED)
   Block Block::GetBaseMemory(DMeta meta, const RTTI::Base& base) const {
      auto result = const_cast<Block*>(this)->GetBaseMemory(meta, base);
      result.MakeConst();
      return result;
   }

   /// Get the memory block corresponding to a base                           
   ///   @attention assumes block is not empty                                
   ///   @param base - the base reflection to use                             
   ///   @return the static block for the base                                
   LANGULUS(INLINED)
   Block Block::GetBaseMemory(const RTTI::Base& base) {
      return GetBaseMemory(base.mType, base);
   }

   /// Get the memory block corresponding to a base (const)                   
   ///   @attention assumes block is not empty                                
   ///   @param base - the base reflection to use                             
   ///   @return the static immutable block for the base                      
   LANGULUS(INLINED)
   Block Block::GetBaseMemory(const RTTI::Base& base) const {
      return GetBaseMemory(base.mType, base);
   }
   
   /// Mutate the block to a different type, if possible                      
   ///   @tparam T - the type to change to                                    
   ///   @tparam ALLOW_DEEPEN - are we allowed to mutate to WRAPPER?          
   ///   @tparam WRAPPER - container to use for deepening, if allowed         
   ///   @return true if block was deepened to incorporate the new type       
   template<CT::Data T, bool ALLOW_DEEPEN, CT::Data WRAPPER>
   LANGULUS(INLINED)
   bool Block::Mutate() {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");
      return Mutate<ALLOW_DEEPEN, WRAPPER>(MetaData::Of<T>());
   }
   
   /// Mutate to another compatible type, deepening the container if allowed  
   ///   @tparam ALLOW_DEEPEN - are we allowed to mutate to WRAPPER?          
   ///   @tparam WRAPPER - container to use for deepening, if allowed         
   ///   @param meta - the type to mutate into                                
   ///   @return true if block was deepened to incorporate the new type       
   template<bool ALLOW_DEEPEN, CT::Data WRAPPER>
   bool Block::Mutate(DMeta meta) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");

      if (not mType or (not mState.IsTyped() and mType->mIsAbstract and IsEmpty() and meta->CastsTo(mType))) {
         // Undefined/abstract containers can mutate freely             
         SetType<false>(meta);
      }
      else if (mType->IsExact(meta)) {
         // No need to mutate - types are exactly the same              
         return false;
      }
      else if (not IsInsertable(meta)) {
         // Not insertable due to some reasons                          
         if constexpr (ALLOW_DEEPEN) {
            if (not IsTypeConstrained()) {
               // Container is not type-constrained, so we can safely   
               // deepen it, to incorporate the new data                
               Deepen<WRAPPER>();
               return true;
            }

            LANGULUS_THROW(Mutate,
               "Attempting to mutate incompatible type-constrained container");
         }
         else LANGULUS_THROW(Mutate, "Can't mutate to incompatible type");
      }

      // Block may have mutated, but it hadn't deepened                 
      return false;
   }
   
   /// Set the data ID - use this only if you really know what you're doing   
   ///   @tparam CONSTRAIN - whether or not to enable type-constraint         
   ///   @param type - the type meta to set                                   
   template<bool CONSTRAIN>
   void Block::SetType(DMeta type) {
      if (mType == type) {
         if constexpr (CONSTRAIN)
            MakeTypeConstrained();
         return;
      }
      else if (not mType) {
         mType = type;
         if constexpr (CONSTRAIN)
            MakeTypeConstrained();
         return;
      }

      LANGULUS_ASSERT(not IsTypeConstrained(), Mutate, "Incompatible type");

      if (mType->CastsTo(type)) {
         // Type is compatible, but only sparse data can mutate freely  
         // Dense containers can't mutate because their destructors     
         // might be wrong later                                        
         LANGULUS_ASSERT(IsSparse(), Mutate, "Incompatible type");
         mType = type;
      }
      else {
         // Type is not compatible, but container is not typed, so if   
         // it has no constructed elements, we can still mutate it      
         LANGULUS_ASSERT(IsEmpty(), Mutate, "Incompatible type");
         mType = type;
      }

      if constexpr (CONSTRAIN)
         MakeTypeConstrained();
   }
   
   /// Set the contained data type                                            
   ///   @tparam T - the contained type                                       
   ///   @tparam CONSTRAIN - whether or not to enable type-constraints        
   template<CT::Data T, bool CONSTRAIN>
   LANGULUS(INLINED)
   void Block::SetType() {
      SetType<CONSTRAIN>(MetaData::Of<Deref<T>>());
   }
   
   /// Reset the type of the block, unless it's type-constrained              
   LANGULUS(INLINED)
   constexpr void Block::ResetType() noexcept {
      if (not IsTypeConstrained())
         mType = nullptr;
   }

   /// Simplify an index by constraining it into number of members            
   ///   @attention assumes block is typed                                    
   ///   @tparam INDEX - the index to simplify (deducible)                    
   ///   @param index - the index to simplify                                 
   ///   @return the simple index                                             
   template<CT::Index INDEX>
   LANGULUS(INLINED)
   Offset Block::SimplifyMemberIndex(const INDEX& index) const noexcept {
      if constexpr (CT::Same<INDEX, Index>)
         return index.Constrained(mType->GetMemberCount()).GetOffset();
      else if constexpr (CT::Signed<INDEX>) {
         if (index < 0)
            return mType->GetMemberCount() - static_cast<Offset>(-index);
         else
            return static_cast<Offset>(index);
      }
      else return index;
   }

} // namespace Langulus::Anyness