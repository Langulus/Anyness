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

   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type is similar to at least one of the types    
   template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   bool Block::Is() const {
      return mType and mType->template Is<T1, TN...>();
   }

   /// Check if type origin is the same as another                            
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @param type - the type to check for                                  
   ///   @return true if this block contains similar data                     
   LANGULUS(INLINED)
   bool Block::Is(DMeta type) const noexcept {
      return mType &= type;
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type is similar to at least one of the types    
   template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   bool Block::IsSimilar() const {
      return mType and mType->template IsSimilar<T1, TN...>();
   }

   /// Check if unqualified type is the same as another                       
   ///   @attention ignores only cv-qualifiers                                
   ///   @param type - the type to check for                                  
   ///   @return true if this block contains similar data                     
   LANGULUS(INLINED)
   bool Block::IsSimilar(DMeta type) const noexcept {
      return mType |= type;
   }

   /// Check if this type is exactly one of the provided types                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   bool Block::IsExact() const {
      return mType and mType->template IsExact<T1, TN...>();
   }

   /// Check if this type is exactly another                                  
   ///   @param type - the type to match                                      
   ///   @return true if data type matches type exactly                       
   LANGULUS(INLINED)
   bool Block::IsExact(DMeta type) const noexcept {
      return mType == type;
   }

   /// Check if contained data can be interpreted as a given type             
   ///   @attention direction matters, if block is dense                      
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current type to 'type'             
   template<bool BINARY_COMPATIBLE> LANGULUS(INLINED)
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
   template<bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool Block::CastsToMeta(DMeta type, Count count) const {
      return not mType or not type or mType->CastsTo(type, count);
   }

   /// Check if this container's data can be represented as type T            
   /// with nothing more than pointer arithmetic                              
   ///   @tparam T - the type to compare against                              
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @return true if contained data is reinterpretable as T               
   template<CT::Data T, bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool Block::CastsTo() const {
      return CastsToMeta<BINARY_COMPATIBLE>(MetaDataOf<T>());
   }

   /// Check if this container's data can be represented as a specific number 
   /// of elements of type T, with nothing more than pointer arithmetic       
   ///   @tparam T - the type to compare against                              
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @param count - the number of elements of T                           
   ///   @return true if contained data is reinterpretable as T               
   template<CT::Data T, bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool Block::CastsTo(Count count) const {
      return CastsToMeta<BINARY_COMPATIBLE>(MetaDataOf<T>(), count);
   }
   
   /// Reinterpret contents of this Block as the type and state of another    
   /// You can interpret Vec4 as float[4] for example, or any other such      
   /// reinterpretation, as long as data remains tightly packed and aligned   
   /// No real conversion is performed, only pointer arithmetic               
   ///   @param pattern - the type of data to try interpreting as             
   ///   @return a block representing this block, interpreted as the pattern  
   template<CT::Block THIS> LANGULUS(INLINED)
   auto Block::ReinterpretAs(const CT::Block auto& pattern) const {
      using RHS = Deref<decltype(pattern)>;

      if (IsEmpty() or IsSparse<THIS>()
      or IsUntyped<THIS>() or pattern.template IsUntyped<RHS>())
         return {};

      if constexpr (CT::Typed<THIS, RHS>) {
         using T1 = TypeOf<THIS>;
         using T2 = TypeOf<RHS>;

         // Both containers are statically typed                        
         if constexpr (CT::BinaryCompatible<T1, T2>) {
            // 1:1 view for binary compatible types                     
            return RHS {Disown(Block{
               pattern.GetState() + DataState::Static,
               pattern.GetType(), mCount,
               mRaw, mEntry
            })};
         }
         else if constexpr (CT::POD<T1, T2>) {
            if constexpr (sizeof(T1) >= sizeof(T2) and (sizeof(T1) % sizeof(T2)) == 0) {
               // Larger view for binary compatible types               
               return RHS {Disown(Block{
                  pattern.GetState() + DataState::Static,
                  pattern.GetType(), mCount * (sizeof(T1) / sizeof(T2)),
                  mRaw, mEntry
               })};
            }
            else if constexpr (sizeof(T1) <= sizeof(T2) and (sizeof(T2) % sizeof(T1)) == 0) {
               // Smaller view for binary compatible types              
               return RHS {Disown(Block{
                  pattern.GetState() + DataState::Static,
                  pattern.GetType(), mCount / (sizeof(T2) / sizeof(T1)),
                  mRaw, mEntry
               })};
            }
            else LANGULUS_ERROR("Can't reinterpret POD types - not alignable");
         }
         else LANGULUS_ERROR("Can't reinterpret blocks - types are not binary compatible");
         //TODO add imposed base reinterprets here too, by statically scanning reflected bases?
      }
      else {
         // One of the blocks is type-erased, so do RTTI checks         
         // This also includes imposed base reinterpretations           
         // First, compare types and get a common base type if any      
         RTTI::Base common {};
         if (not CompareTypes(pattern, common) or not common.mBinaryCompatible)
            return {};

         // Find how elements fit from one to another                   
         const Size baseBytes = (common.mType->mSize * common.mCount)
            / pattern.GetStride();
         const Size resultSize = pattern.IsEmpty()
            ? baseBytes : (baseBytes / pattern.mCount) * pattern.mCount;

         // Create a static view of the desired type                    
         return RHS {Disown(Block{
            pattern.mState + DataState::Static,
            pattern.mType, resultSize,
            mRaw, mEntry
         })};
      }
   }

   /// Reinterpret contents of this Block as the type and state of another    
   /// You can interpret Vec4 as float[4] for example, or any other such      
   /// reinterpretation, as long as data remains tightly packed and aligned   
   /// No real conversion is performed, only pointer arithmetic               
   ///   @tparam T - the type of data to try interpreting as                  
   ///   @return a block representing this block, interpreted as the pattern  
   template<CT::Block THIS, CT::Data T> LANGULUS(INLINED)
   TAny<T> Block::ReinterpretAs() const {
      static_assert(CT::Dense<T>, "T must be dense");
      return ReinterpretAs(Block::From<T>());
   }

   /// Get the memory block corresponding to a local member variable          
   ///   @attention assumes block is not empty                                
   ///   @param member - the member to get                                    
   ///   @return a static memory block                                        
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(const RTTI::Member& member) {
      LANGULUS_ASSUME(DevAssumes, not IsEmpty(),
         "Getting member from an empty block");

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
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(const RTTI::Member& member) const {
      auto result = const_cast<Block*>(this)->GetMember<THIS>(member);
      result.MakeConst();
      return result;
   }
   
   /// Get a trait-tagged member from first element inside block              
   ///   @attention assumes block is not empty                                
   ///   @param trait - the trait tag to search for                           
   ///   @return the static mutable block corresponding to that member        
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(TMeta trait) {
      // Scan members                                                   
      for (auto& member : mType->mMembers) {
         if (member.GetTrait() == trait)
            return GetMember<THIS>(member);
      }

      // No such trait found, so check in bases                         
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember<Block>(trait);
         if (found.IsTyped<Block>()) //TODO is this check still required?
            return found;
      }

      return {};
   }

   /// Get a trait-tagged member from first element inside block (const)      
   ///   @attention assumes block is not empty                                
   ///   @param trait - the trait tag to search for                           
   ///   @return the static constant block corresponding to that member       
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(TMeta trait) const {
      auto result = const_cast<Block*>(this)->GetMember<THIS>(trait);
      result.MakeConst();
      return result;
   }
   
   /// Get a member of specific type, from first element inside block         
   ///   @attention assumes block is not empty                                
   ///   @param data - the type to search for                                 
   ///   @return the static block corresponding to that member                
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(DMeta data) {
      // Scan members                                                   
      for (auto& member : mType->mMembers) {
         if (member.GetType()->CastsTo(data))
            return GetMember<THIS>(member);
      }

      // No such data found, so check in bases                          
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember<Block>(data);
         if (found.IsTyped<Block>()) //TODO is this check still required?
            return found;
      }

      return {};
   }

   /// Get a member of specific type, from first element inside block (const) 
   ///   @attention assumes block is not empty                                
   ///   @param data - the type to search for                                 
   ///   @return the static constant block corresponding to that member       
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(DMeta data) const {
      auto result = const_cast<Block*>(this)->GetMember<THIS>(data);
      result.MakeConst();
      return result;
   }

   /// Get the first member of the first element inside block                 
   ///   @attention assumes block is not empty                                
   ///   @return the static mutable block corresponding to that member        
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(::std::nullptr_t) {
      if (mType->mMembers.empty())
         return {};
      return GetMember<THIS>(mType->mMembers[0]);
   }

   /// Get the first member of the first element inside block (const)         
   ///   @attention assumes block is not empty                                
   ///   @return the static constant block corresponding to that member       
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(::std::nullptr_t) const {
      if (mType->mMembers.empty())
         return {};
      return GetMember<THIS>(mType->mMembers[0]);
   }
      
   /// Select a member by trait or index (or both)                            
   ///   @attention assumes block is not empty                                
   ///   @param trait - the trait to get                                      
   ///   @param index - the trait index to get                                
   ///   @return the static memory block of the member                        
   template<CT::Block THIS>
   Block Block::GetMember(TMeta trait, CT::Index auto index) {
      // Scan immediate members                                         
      Offset offset = SimplifyMemberIndex(index);
      Offset counter = 0;
      for (auto& member : mType->mMembers) {
         if (trait and member.GetTrait() != trait)
            continue;

         // Matched, but check index first                              
         if (counter < offset) {
            ++counter;
            continue;
         }

         // Found                                                       
         return GetMember<THIS>(member);
      }

      // If reached, then nothing found in local members, so check bases
      offset -= counter;
      for (auto& base : mType->mBases) {
         auto found = GetBaseMemory(base.mType, base)
            .GetMember<Block>(trait, offset);
         if (found.IsTyped<Block>()) //TODO is this check still required?
            return found;

         offset -= base.mType->GetMemberCount();
      }

      return {};
   }

   /// Select a member by trait or index (or both) (const)                    
   ///   @attention assumes block is not empty                                
   ///   @param trait - the trait to get                                      
   ///   @param index - the trait index to get                                
   ///   @return the static constant memory block of the member               
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(TMeta trait, CT::Index auto index) const {
      auto result = const_cast<Block*>(this)->GetMember<THIS>(trait, index);
      result.MakeConst();
      return result;
   }
   
   /// Select a member by data type or index (or both)                        
   ///   @attention assumes block is not empty                                
   ///   @param data - the type to search for                                 
   ///   @param index - the member index to get                               
   ///   @return the static memory block of the member                        
   template<CT::Block THIS>
   Block Block::GetMember(DMeta data, CT::Index auto index) {
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
         return GetMember<THIS>(member);
      }

      // If reached, then nothing found in local members, so check bases
      offset -= counter;
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember<Block>(data, offset);
         if (found.IsTyped<Block>()) //TODO is this check still required?
            return found;

         offset -= base.mType->GetMemberCount();
      }

      return {};
   }

   /// Select a member by data type or index (or both) (const)                
   ///   @attention assumes block is not empty                                
   ///   @param data - the type to search for                                 
   ///   @param index - the trait index to get                                
   ///   @return the static constant memory block of the member               
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(DMeta data, CT::Index auto index) const {
      auto result = const_cast<Block*>(this)->GetMember<THIS>(data, index);
      result.MakeConst();
      return result;
   }

   /// Select a member by index only                                          
   ///   @attention assumes block is not empty                                
   ///   @param index - the member index to get                               
   ///   @return the static memory block of the member                        
   template<CT::Block THIS>
   Block Block::GetMember(std::nullptr_t, CT::Index auto index) {
      // Check immediate members first                                  
      Offset offset = SimplifyMemberIndex(index);
      if (offset < mType->mMembers.size())
         return GetMember<THIS>(mType->mMembers[offset]);

      // If reached, then nothing found in local members, so check bases
      offset -= mType->mMembers.size();
      for (auto& base : mType->mBases) {
         const auto found = GetBaseMemory(base.mType, base)
            .GetMember<Block>(nullptr, offset);
         if (found.IsTyped<Block>()) //TODO is this check still required?
            return found;

         offset -= base.mType->GetMemberCount();
      }

      return {};
   }

   /// Select a member by index only                                          
   ///   @attention assumes block is not empty                                
   ///   @param index - the member index to get                               
   ///   @return the static constant memory block of the member               
   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetMember(std::nullptr_t, CT::Index auto index) const {
      auto result = const_cast<Block*>(this)->GetMember<THIS>(nullptr, index);
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
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @return true if block was deepened to incorporate the new type       
   template<CT::Block THIS, CT::Data T, class FORCE> LANGULUS(INLINED)
   bool Block::Mutate() {
      return Mutate<THIS, FORCE>(MetaDataOf<T>());
   }
   
   /// Mutate to another compatible type, deepening the container if allowed  
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param meta - the type to mutate into                                
   ///   @return true if block was deepened to incorporate the new type       
   template<CT::Block THIS, class FORCE>
   bool Block::Mutate(DMeta meta) {
      if (IsUntyped<THIS>() or (not mState.IsTyped() and mType->mIsAbstract
                                and IsEmpty() and meta->CastsTo(mType))
      ) {
         // Undefined/abstract containers can mutate freely             
         SetType<THIS, false>(meta);
      }
      else if (mType->IsSimilar(meta)) {
         // No need to mutate - types are compatible                    
         return false;
      }
      else if (not IsInsertable<THIS>(meta)) {
         // Not insertable because of reasons                           
         if constexpr (not CT::Void<FORCE>) {
            if (not IsTypeConstrained<THIS>()) {
               // Container is not type-constrained, so we can safely   
               // deepen it, to incorporate the new data                
               Deepen<FORCE, true, THIS>();
               return true;
            }

            LANGULUS_OOPS(Mutate, "Attempting to mutate incompatible "
               "type-constrained container");
         }
         else LANGULUS_OOPS(Mutate, "Can't mutate to incompatible type");
      }

      // Block may have mutated, but it didn't happen                   
      return false;
   }
   
   /// Set the data ID - use this only if you really know what you're doing   
   ///   @tparam CONSTRAIN - whether or not to enable type-constraint         
   ///   @param type - the type meta to set                                   
   template<CT::Block THIS, bool CONSTRAIN>
   void Block::SetType(DMeta type) {
      static_assert(not CT::Typed<THIS>,
         "Can't set type of a statically typed container");

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

      LANGULUS_ASSERT(not IsTypeConstrained<THIS>(), Mutate,
         "Incompatible type");

      if (mType->CastsTo(type)) {
         // Type is compatible, but only sparse data can mutate freely  
         // Dense containers can't mutate because their destructors     
         // might be wrong later                                        
         LANGULUS_ASSERT(IsSparse<THIS>(), Mutate, "Incompatible type");
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
   template<CT::Block THIS, CT::Data T, bool CONSTRAIN> LANGULUS(INLINED)
   void Block::SetType() {
      SetType<THIS, CONSTRAIN>(MetaDataOf<T>());
   }
   
   /// Reset the type of the block, unless it's type-constrained              
   /// Typed THIS makes sure, that this is a no-op                            
   template<CT::Block THIS> LANGULUS(INLINED)
   constexpr void Block::ResetType() noexcept {
      if constexpr (not CT::Typed<THIS>) {
         if (not IsTypeConstrained<THIS>())
            mType = nullptr;
      }
   }

   /// Simplify an index by constraining it to the number of reflected members
   ///   @param index - the index to simplify                                 
   ///   @return the simple index                                             
   template<CT::Index INDEX> LANGULUS(INLINED)
   Offset Block::SimplifyMemberIndex(const INDEX& index) const 
   noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>) {
      if constexpr (CT::Same<INDEX, Index>)
         return index.Constrained(mType->GetMemberCount()).GetOffset();
      else if constexpr (CT::BuiltinInteger<INDEX>) {
         LANGULUS_ASSUME(DevAssumes, index > 0,
            "Negative raw index not allowed - "
            "for indexing in reverse, use negative Index instead"
         );
         return static_cast<Offset>(index);
      }
      else return index;
   }

} // namespace Langulus::Anyness