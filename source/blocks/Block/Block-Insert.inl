///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block-Construct.inl"
#include "Block-Indexing.inl"
#include "Block-Capsulation.inl"


namespace Langulus::Anyness
{
         
   /// Inner semantic insertion function for a range                          
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the offset at which to start inserting                
   ///   @param start - start of range                                        
   ///   @param end - end of range                                            
   template<CT::Block THIS, class FORCE, template<class> class S, CT::Sparse T1, CT::Sparse T2>
   void Block::InsertContiguousInner(CT::Index auto index, S<T1>&& start, T2 end)
   requires (CT::Semantic<S<T1>> and CT::Similar<T1, T2>) {
      using T = Deptr<T1>;
      static_assert(CT::Insertable<T>,
         "Dense type is not insertable");

      auto& ME = reinterpret_cast<THIS&>(*this);
      if constexpr (not CT::Void<FORCE>) {
         // Type may mutate                                             
         if (Mutate<THIS, T, FORCE>()) {
            // If reached, then type mutated to a deep type             
            FORCE temp;
            temp.template InsertContiguous<FORCE, void>(
               IndexBack, Forward<S<T1>>(start), end);
            Insert<THIS, void>(index, Abandon(temp));
            return;
         }
      }
      else {
         // Type can't mutate, but we still have to check it            
         LANGULUS_ASSERT(ME.template IsSimilar<T>(), Meta,
            "Inserting incompatible type `", MetaDataOf<T>(),
            "` to container of type `", ME.GetType(), '`'
         );
      }

      // If reached, we have compatible type, so allocate               
      const Offset idx = SimplifyIndex<T>(index);
      const auto count = end - start;
      AllocateMore<false>(mCount + count);

      if (idx < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any potential overlap                              
         const auto moved = mCount - idx;
         CropInner(idx + count, moved)
            .template CallKnownSemanticConstructors<T, true>(
               moved, Abandon(CropInner(idx, moved))
            );
      }

      if constexpr (CT::Sparse<T>) {
         if constexpr (S<T>::Shallow) {
            // Copy all pointers at once                                
            CopyMemory(GetRawAs<T>() + idx, start, count);

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               // If we're using managed memory, we can search if each  
               // pointer is owned by us, and get its allocation entry  
               // You can avoid this by using the Disowned semantic     
               if constexpr (CT::Allocatable<Deptr<T>> and S<T>::Keep) {
                  auto it = start;
                  auto entry = GetEntries() + idx;
                  while (it != end) {
                     *entry = Allocator::Find(MetaDataOf<Deptr<T>>(), it);
                     if (*entry)
                        const_cast<Allocation*>(*entry)->Keep();

                     ++it;
                     ++entry;
                  }
               }
               else
            #endif
               ZeroMemory(GetEntries() + idx, count);
         }
         else {
            // Pointer clone                                            
            TODO();
         }
      }
      else {
         // Insert dense data                                           
         static_assert(not CT::Abstract<T>,
            "Can't insert abstract item in dense container");

         auto to = GetRawAs<T>() + idx;
         auto from = *start;
         if constexpr (CT::POD<T>) {
            // Optimized POD range insertion                            
            CopyMemory(to, from, count);
         }
         else if constexpr (CT::SemanticMakable<S, T>) {
            // Call semantic construction for each element in range     
            while (to != from)
               SemanticNew(to++, S<T>::Nest(*(from++)));
         }
         else LANGULUS_ERROR("Missing semantic-constructor");
      }

      mCount += count;
   }

   /// Inner semantic insertion function                                      
   ///   @attention this is an inner function and should be used with caution 
   ///   @attention assumes required free space has been prepared at offset   
   ///   @attention assumes that TypeOf<S> is exactly this container's type   
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the offset at which to insert                         
   ///   @param item - item and semantic to insert                            
   template<CT::Block THIS, class FORCE>
   void Block::InsertInner(CT::Index auto index, auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;
      static_assert(CT::Insertable<T>,
         "Dense type is not insertable");

      auto& ME = reinterpret_cast<THIS&>(*this);
      if constexpr (not CT::Void<FORCE>) {
         // Type may mutate                                             
         if (Mutate<THIS, T, FORCE>()) {
            // If reached, then type mutated to a deep type             
            FORCE temp {S::Nest(item)};
            Insert<THIS, void>(index, Abandon(temp));
            return;
         }
      }
      else {
         // Type can't mutate, but we still have to check it            
         LANGULUS_ASSERT(ME.template IsSimilar<T>(), Meta,
            "Inserting incompatible type `", MetaDataOf<T>(),
            "` to container of type `", ME.GetType(), '`'
         );
      }

      // If reached, we have compatible type, so allocate               
      const Offset idx = SimplifyIndex<T>(index);
      AllocateMore<false>(mCount + 1);

      if (idx < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any potential overlap                              
         const auto moved = mCount - idx;
         CropInner(idx + 1, moved)
            .template CallKnownSemanticConstructors<T, true>(
               moved, Abandon(CropInner(idx, moved))
            );
      }

      LANGULUS_ASSUME(DevAssumes, IsExact<T>(), "Inserting incompatible type");
      GetHandle<T>(idx).New(S::Nest(item));
      ++mCount;
   }

   /// Insert an element, or an array of elements                             
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the index at which to insert                          
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   template<CT::Block THIS, class FORCE>
   Count Block::UnfoldInsert(CT::Index auto index, auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;
      
      if constexpr (CT::Array<T>) {
         if constexpr (CT::StringLiteral<T>) {
            // Implicitly convert string literals to Text containers    
            InsertInner<THIS, FORCE>(index, Text {S::Nest(item)});
            return 1;
         }
         else {
            // Insert the array                                         
            InsertContiguousInner<THIS, FORCE>(index, S::Nest(item));
            return ExtentOf<T>;
         }
      }
      else {
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         InsertInner<THIS, FORCE>(index, S::Nest(item));
         return 1;
      }
   }

   /// Insert an element, or an array of elements, if not found               
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the index at which to insert                          
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   template<CT::Block THIS, class FORCE>
   Count Block::UnfoldMerge(CT::Index auto index, auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;
      
      if constexpr (CT::Array<T>) {
         if constexpr (CT::StringLiteral<T>) {
            // Implicitly convert string literals to Text containers    
            if (CT::Void<FORCE> and not IsSimilar<Text>())
               return 0;

            Text text {S::Nest(item)};
            if (not IsSimilar<Text>() or not FindKnown(text)) {
               InsertInner<FORCE>(index, Abandon(text));
               return 1;
            }
         }
         else {
            // Insert the array                                         
            if (CT::Void<FORCE> and not IsSimilar<Deext<T>>())
               return 0;

            constexpr auto count = ExtentOf<T>;
            if (not IsSimilar<Deext<T>>()
            or  not FindContiguousKnown(DesemCast(item), DesemCast(item) + count)) {
               InsertContiguousInner<FORCE>(index, S::Nest(item), DesemCast(item) + count);
               return count;
            }
         }
      }
      else {
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         if (CT::Void<FORCE> and not IsSimilar<T>())
            return 0;

         if (not IsSimilar<T>() or not FindKnown(DesemCast(item))) {
            InsertInner<FORCE>(index, S::Nest(item));
            return 1;
         }
      }

      return 0;
   }

   /// Insert elements at a given index, semantically or not                  
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the index at which to insert                          
   ///   @param t1 - the first item to insert                                 
   ///   @param tail... - the rest of items to insert (optional)              
   ///   @return number of inserted elements                                  
   template<CT::Block THIS, class FORCE, class T1, class...TAIL> LANGULUS(INLINED)
   Count Block::Insert(CT::Index auto idx, T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
      inserted += UnfoldInsert<THIS, FORCE>(idx, Forward<T1>(t1));
      ((inserted += UnfoldInsert<THIS, FORCE>(idx + inserted, Forward<TAIL>(tail))), ...);
      return inserted;
   }
   
   /// Insert all elements of a block at an index, semantically or not        
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param idx - index to insert them at                                 
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, class T> LANGULUS(INLINED)
   Count Block::InsertBlock(CT::Index auto idx, T&& other)
   requires CT::Block<Desem<T>> {
      using S = SemanticOf<decltype(other)>;
      using ST = TypeOf<S>;
      auto& rhs = const_cast<Block&>(static_cast<const Block&>(DesemCast(other)));

      if (rhs.IsEmpty())
         return 0;

      // Allocate region and type-check                                 
      auto region = AllocateRegion<THIS>(rhs, SimplifyIndex<ST>(idx));

      if constexpr (CT::Typed<ST>) {
         region.template CallKnownSemanticConstructors<TypeOf<ST>>(
            rhs.GetCount(), S::Nest(rhs)
         );
      }
      else {
         region.CallUnknownSemanticConstructors(
            rhs.GetCount(), S::Nest(rhs)
         );
      }

      mCount += rhs.GetCount();

      if constexpr (S::Move and S::Keep and ST::Ownership) {
         // All elements were moved, only empty husks remain            
         // so destroy them, and discard ownership of 'other'           
         const auto pushed = rhs.GetCount();
         rhs.Free();
         rhs.mEntry = nullptr;
         return pushed;
      }
      else return rhs.GetCount();
   }
   
   /// Merge a single element by a semantic                                   
   /// Element will be pushed only if not found in block                      
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the index at which to insert                          
   ///   @param t1 - the first item to insert                                 
   ///   @param tail... - the rest of items to insert (optional)              
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, class T1, class...TAIL> LANGULUS(INLINED)
   Count Block::Merge(CT::Index auto index, T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
      inserted += UnfoldMerge<THIS, FORCE>(index, Forward<T1>(t1));
      ((inserted += UnfoldMerge<THIS, FORCE>(index + inserted, Forward<TAIL>(tail))), ...);
      return inserted;
   }

   /// Semantically insert each element that is not found in this container   
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - special/simple index to insert at                     
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, class T> LANGULUS(INLINED)
   Count Block::MergeBlock(CT::Index auto index, T&& other)
   requires CT::Block<Desem<T>> {
      using S = SemanticOf<decltype(other)>;
      decltype(auto) rhs = DesemCast(other);

      Count inserted = 0;
      for (Count i = 0; i < rhs.GetCount(); ++i) {
         auto element = rhs.GetElement(i);
         if (not FindUnknown(element))
            inserted += InsertBlock<THIS, FORCE>(index, S::Nest(element));
      }

      return inserted;
   }
   
   /// Construct an item of this container's type at the specified position   
   /// by forwarding A... as constructor arguments                            
   /// Since this container is type-erased and exact constructor signatures   
   /// aren't reflected, the following constructors will be attempted:        
   ///   1. If A is a single argument of exactly the same type, the reflected 
   ///      move constructor will be used, if available                       
   ///   2. If A is empty, the reflected default constructor is used          
   ///   3. If A is not empty, not exactly same as the contained type, or     
   ///      is more than a single argument, then all arguments will be        
   ///      wrapped in an Any, and then forwarded to the descriptor-          
   ///      constructor, if such is reflected                                 
   ///   If none of these constructors are available, or block is not typed,  
   ///   this function throws Except::Allocate                                
   ///   @tparam IDX - type of indexing to use (deducible)                    
   ///   @tparam A... - argument types (deducible)                            
   ///   @param idx - the index to emplace at                                 
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return 1 if the element was emplaced successfully                   
   template<CT::Block THIS, class... A> LANGULUS(INLINED)
   Count Block::Emplace(CT::Index auto idx, A&&... arguments) {
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;

         AllocateMore(mCount + 1);
         const auto offset = SimplifyIndex<T>(idx);
         if (offset < mCount) {
            // Move memory if required                                  
            LANGULUS_ASSERT(GetUses() == 1, Move,
               "Emplacing elements to memory block, used from multiple places, "
               "requires memory to move");

            // We're moving to the right, so make sure we do it in      
            // reverse to avoid any overlap                             
            const auto tail = mCount - offset;
            CropInner(offset + 1, 0)
               .template CallKnownSemanticConstructors<T, true>(
                  tail, Abandon(CropInner(offset, tail))
               );
         }

         CropInner(offset, 0)
            .template CallKnownConstructors<T, A...>(
               1, Forward<A>(arguments)...);

         ++mCount;
      }
      else {
         // Move memory if required                                     
         AllocateMore(mCount + 1);
         const auto offset = SimplifyIndex<void, false>(idx);
         if (offset < mCount) {
            // Move memory if required                                  
            LANGULUS_ASSERT(GetUses() == 1, Move,
               "Moving elements that are used from multiple places");

            // We need to shift elements right from the insertion point 
            // Therefore, we call move constructors in reverse, to      
            // avoid memory overlap                                     
            const auto moved = mCount - offset;
            CropInner(offset + 1, 0)
               .template CallUnknownSemanticConstructors<true>(
                  moved, Abandon(CropInner(offset, moved))
               );
         }

         // Pick the region that should be overwritten with new stuff   
         const auto region = CropInner(offset, 0);
         EmplaceInner<THIS>(region, 1, Forward<A>(arguments)...);
      }

      return 1;
   }
   
   /// Create N new elements, using the provided arguments for construction   
   /// Elements will be added to the back of the container                    
   /// If none of the constructors are available, or block is not typed,      
   /// this function throws Except::Allocate                                  
   ///   @param count - number of elements to construct                       
   ///   @param ...arguments - constructor arguments (optional)               
   ///   @return the number of new elements                                   
   template<CT::Block THIS, class... A> LANGULUS(INLINED)
   Count Block::New(Count count, A&&... arguments) {
      // Allocate the required memory - this will not initialize it     
      LANGULUS_ASSUME(UserAssumes, count, "Zero count not allowed");
      AllocateMore(mCount + count);

      // Pick the region that should be overwritten with new stuff      
      const auto region = CropInner(mCount, 0);
      EmplaceInner<THIS>(region, count, Forward<A>(arguments)...);
      return count;
   }
   
   /// Wrap all contained elements inside a sub-block, making this one deep   
   ///   @tparam T - the type of deep container to use                        
   ///   @tparam TRANSFER_OR - whether to send the current orness deeper      
   ///   @return a reference to this container                                
   template<CT::Deep T, bool TRANSFER_OR, CT::Block THIS> LANGULUS(INLINED)
   T& Block::Deepen() {
      if constexpr (CT::Typed<THIS> and not CT::Similar<T, TypeOf<THIS>>)
         LANGULUS_ERROR("Can't deepen with incompatible type");
      else {
         auto& me = reinterpret_cast<THIS&>(*this);
         LANGULUS_ASSERT(not me.IsTypeConstrained()
                          or me.template IsSimilar<T>(),
            Mutate, "Can't deepen with incompatible type");
      }

      // Back up the state so that we can restore it if not moved over  
      UNUSED() const DataState state = mState.mState & DataState::Or;
      if constexpr (not TRANSFER_OR)
         mState -= state;

      // Allocate a new T and move this inside it                       
      Block wrapper;
      wrapper.template SetType<T, false>();
      wrapper.template AllocateMore<true>(1);
      wrapper.template Get<Block>() = *this;
      *this = wrapper;
      
      // Restore the state if not moved over                            
      if constexpr (not TRANSFER_OR)
         mState += state;

      return Get<T>();
   }

   /// Semantic-insert that uses the best approach to push anything inside    
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @tparam THIS - the type of the block, used for absorbtion            
   ///   @param index - the index at which to insert (if needed)              
   ///   @param value - the value to smart-push                               
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<bool ALLOW_CONCAT, class FORCE, CT::Block THIS>
   Count Block::SmartPush(
      CT::Index auto index, auto&& value, DataState state
   ) {
      using S = SemanticOf<decltype(value)>;
      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         // We're inserting a deep item, so we can do various smart     
         // things before inserting, like absorbing and concatenating   
         if (not DesemCast(value).IsValid())
            return 0;

         const bool stateCompliant = CanFitState(DesemCast(value));
         if (IsEmpty() and not DesemCast(value).IsStatic() and stateCompliant) {
            Free();
            BlockTransfer<THIS>(S::Nest(value));
            return 1;
         }

         if constexpr (ALLOW_CONCAT) {
            const auto done = SmartConcat<THIS, FORCE>(
               index, stateCompliant, S::Nest(value), state);

            if (done)
               return done;
         }
      }

      return SmartPushInner<THIS, FORCE>(index, S::Nest(value), state);
   }

   
   /// Smart concatenation inner call, used by smart push                     
   /// Attempts to either concatenate elements, or deepen and push block      
   ///   @tparam THIS - the type of the block, used for absorbtion            
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the place to insert at                                
   ///   @param sc - is this block state-compliant for insertion              
   ///   @param value - the value to concatenate                              
   ///   @param state - the state to apply after concatenation                
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, template<class> class S, CT::Deep T> LANGULUS(INLINED)
   Count Block::SmartConcat(CT::Index auto index, bool sc, S<T>&& value, DataState state)
   requires CT::Semantic<S<T>> {
      auto& me = reinterpret_cast<THIS&>(*this);

      // If this container is compatible and concatenation is           
      // enabled, try concatenating the two containers                  
      const bool typeCompliant = me.IsUntyped()
              or (not CT::Void<FORCE> and value->IsDeep())
              or me.IsSimilar(value->GetType());

      if (not me.IsConstant() and not me.IsStatic() and typeCompliant and sc
         // Make sure container is or-compliant after the change        
         and not (me.GetCount() > 1 and not me.IsOr() and state.IsOr())) {
         if (me.IsUntyped()) {
            // Block insert never mutates, so make sure type            
            // is valid before insertion                                
            SetType<false>(value->GetType());
         }
         else if constexpr (not CT::Void<FORCE>) {
            if (not me.IsDeep() and value->IsDeep())
               Deepen<FORCE, false, THIS>();
         }

         const auto cat = InsertBlock<THIS, void>(index, value.Forward());
         mState += state;
         return cat;
      }

      return 0;
   }
   
   /// Inner smart-push function                                              
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///                   deep with provided type - use void to disable        
   ///   @param index - the place to insert at                                
   ///   @param value - the value to concatenate                              
   ///   @param state - the state to apply after concatenation                
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, template<class> class S, class T> LANGULUS(INLINED)
   Count Block::SmartPushInner(CT::Index auto index, S<T>&& value, DataState state)
   requires CT::Semantic<S<T>> {
      if (IsUntyped() and IsInvalid()) {
         // Mutate-insert inside untyped container                      
         SetState(mState + state);
         return Insert<THIS, void>(index, value.Forward());
      }
      else if (IsExact<T>()) {
         // Insert to a same-typed container                            
         SetState(mState + state);
         return Insert<THIS, void>(index, value.Forward());
      }
      else if (IsEmpty() and mType and not IsTypeConstrained()) {
         // If incompatibly typed but empty and not constrained, we     
         // can still reset the container and reuse it                  
         Reset();
         SetState(mState + state);
         return Insert<THIS, void>(index, value.Forward());
      }
      else if (IsDeep()) {
         // If this is deep, then push value wrapped in a container     
         if (mCount > 1 and not IsOr() and state.IsOr()) {
            // If container is not or-compliant after insertion, we     
            // need	to add another layer                               
            Deepen<THIS, true, THIS>();
            SetState(mState + state);
         }
         else SetState(mState + state);

         return Insert<THIS, void>(index, THIS {value.Forward()});
      }

      if constexpr (not CT::Void<FORCE>) {
         // If this is reached, all else failed, but we are allowed to  
         // deepen, so do it                                            
         Deepen<FORCE, false, THIS>();
         SetState(mState + state);
         return Insert<THIS, void>(index, FORCE {value.Forward()});
      }
      else return 0;
   }
   
   /// Concatenate this, and another block into a new block, semantically     
   ///   @tparam THIS - the type of the resulting container                   
   ///   @param rhs - block and semantic to concatenate with (right side)     
   ///   @return the concatenated container                                   
   template<CT::Block THIS, template<class> class S, CT::Block T>
   THIS Block::ConcatBlock(S<T>&& rhs) const requires CT::Semantic<S<T>> {
      auto& lhs = reinterpret_cast<const THIS&>(*this);
      if (IsEmpty())
         return {rhs.Forward()};
      else if (rhs->IsEmpty())
         return lhs;

      THIS result;
      if constexpr (not CT::Typed<THIS>)
         result.SetType(mType);
      result.AllocateFresh(result.RequestSize(mCount + rhs->GetCount()));
      result.template InsertBlock<THIS>(0, Copy(lhs));
      result.template InsertBlock<THIS>(mCount, rhs.Forward());
      return Abandon(result);
   }

   /// Call default constructors in a region and initialize memory            
   ///   @attention never modifies any block state                            
   ///   @attention assumes block has at least 'count' elements reserved      
   ///   @attention assumes memory is not initialized                         
   ///   @param count - the number of elements to initialize                  
   inline void Block::CallUnknownDefaultConstructors(Count count) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved, "Count outside limits");

      if (mType->mIsSparse) {
         // Zero pointers and entries                                   
         ZeroMemory(mRawSparse, count);
         ZeroMemory(const_cast<Block*>(this)->GetEntries(), count);
      } 
      else if (mType->mIsNullifiable) {
         // Zero the dense memory (optimization)                        
         ZeroMemory(mRaw, count * mType->mSize);
      }
      else {
         LANGULUS_ASSERT(
            mType->mDefaultConstructor, Construct,
            "Can't default-construct elements"
            " - no default constructor reflected"
         );
         
         // Construct requested elements one by one                     
         auto to = mRaw;
         const auto stride = mType->mSize;
         const auto toEnd = to + count * stride;
         while (to != toEnd) {
            mType->mDefaultConstructor(to);
            to += stride;
         }
      }
   }
   
   /// Call default constructors in a region and initialize memory            
   ///   @attention never modifies any block state                            
   ///   @attention assumes block has at least 'count' elements reserved      
   ///   @attention assumes memory is not initialized                         
   ///   @attention assumes T is the type of the container                    
   ///   @param count - the number of elements to initialize                  
   template<CT::Data T>
   void Block::CallKnownDefaultConstructors(const Count count) const {
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(), "Type mismatch");
      LANGULUS_ASSUME(DevAssumes, count <= mReserved, "Count outside limits");

      auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Sparse<T>) {
         // Zero pointers and entries                                   
         ZeroMemory(mthis->mRawSparse, count);
         ZeroMemory(mthis->GetEntries(), count);
      }
      else if constexpr (CT::Nullifiable<T>) {
         // Zero the dense memory (optimization)                        
         ZeroMemory(mthis->GetRawAs<T>(), count);
      }
      else if constexpr (CT::Defaultable<T>) {
         // Construct requested elements one by one                     
         auto to = mthis->GetRawAs<T>();
         const auto toEnd = to + count;
         while (to != toEnd)
            new (to++) T {};
      }
      else LANGULUS_ERROR(
         "Trying to default-construct elements that are "
         "incapable of default-construction");
   }
   
   /// Call descriptor constructors in a region, initializing memory          
   ///   @attention never modifies any block state                            
   ///   @attention assumes this has at least 'count' items reserved          
   ///   @attention assumes that none of the elements is initialized          
   ///   @param count - the number of elements to construct                   
   ///   @param descriptor - the descriptor to pass on to constructors        
   inline void Block::CallUnknownDescriptorConstructors(
      Count count, const Neat& descriptor
   ) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved,
         "Count outside limits", '(', count, " > ", mReserved);
      LANGULUS_ASSERT(
         mType->mDescriptorConstructor, Construct,
         "Can't descriptor-construct ", '`', mType,
         "` - no descriptor-constructor reflected"
      );

      if (mType->mDeptr) {
         if (not mType->mDeptr->mIsSparse) {
            // Bulk-allocate the required count, construct each instance
            // and set the pointers                                     
            auto lhsPtr = const_cast<Block*>(this)->GetRawSparse();
            auto lhsEnt = const_cast<Block*>(this)->GetEntries();
            const auto lhsEnd = lhsPtr + count;
            const auto allocation = Allocator::Allocate(
               mType->mOrigin,
               mType->mOrigin->mSize * count
            );
            allocation->Keep(count - 1);

            auto rhs = allocation->GetBlockStart();
            while (lhsPtr != lhsEnd) {
               mType->mOrigin->mDescriptorConstructor(rhs, descriptor);
               *(lhsPtr++) = rhs;
               const_cast<const Allocation*&>(*(lhsEnt++)) = allocation;
               rhs += mType->mOrigin->mSize;
            }
         }
         else {
            // We need to allocate another indirection layer            
            TODO();
         }
      }
      else {
         // Construct all dense elements in place                       
         auto lhs = mRaw;
         const auto lhsEnd = lhs + count * mType->mSize;
         while (lhs != lhsEnd) {
            mType->mDescriptorConstructor(lhs, descriptor);
            lhs += mType->mSize;
         }
      }
   }
   
   /// Call descriptor constructors in a region, initializing memory          
   ///   @attention never modifies any block state                            
   ///   @attention assumes T is the type of the block                        
   ///   @attention assumes this has at least 'count' items reserved          
   ///   @tparam T - type of the data to descriptor-construct                 
   ///   @param count - the number of elements to construct                   
   ///   @param descriptor - the descriptor to pass on to constructors        
   template<CT::Data T>
   void Block::CallKnownDescriptorConstructors(
      const Count count, const Neat& descriptor
   ) const {
      static_assert(CT::DescriptorMakable<T>,
         "T is not descriptor-constructible");

      LANGULUS_ASSUME(DevAssumes, count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "T doesn't match LHS type");

      if constexpr (CT::Sparse<T>) {
         // Bulk-allocate the required count, construct each instance   
         // and push the pointers                                       
         auto lhsPtr = const_cast<Block*>(this)->GetRawSparse();
         auto lhsEnt = const_cast<Block*>(this)->GetEntries();
         const auto lhsEnd = lhsPtr + count;
         const auto allocation = Allocator::Allocate(
            MetaDataOf<Decay<T>>(),
            sizeof(Decay<T>) * count
         );
         allocation->Keep(count - 1);

         auto rhs = allocation->template As<Decay<T>*>();
         while (lhsPtr != lhsEnd) {
            new (rhs) Decay<T> {descriptor};
            *(lhsPtr++) = rhs;
            *(lhsEnt++) = allocation;
            ++rhs;
         }
      }
      else {
         // Construct all dense elements in place                       
         auto lhs = const_cast<Block&>(*this).template GetRawAs<T>();
         const auto lhsEnd = lhs + count;
         while (lhs != lhsEnd) {
            new (lhs++) Decay<T> {descriptor};
         }
      }
   }
   
   /// Call a specific constructors in a region, initializing memory          
   ///   @attention never modifies any block state                            
   ///   @attention assumes T is the type of the block                        
   ///   @attention assumes this has at least 'count' items reserved          
   ///   @tparam T - type of the data to construct                            
   ///   @tparam A... - arguments to pass to constructor (deducible)          
   ///   @param count - the number of elements to construct                   
   ///   @param arguments... - the arguments to forward to constructors       
   template<CT::Data T, class... A>
   void Block::CallKnownConstructors(const Count count, A&&... arguments) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "Type mismatch");

      if constexpr (sizeof...(A) == 0) {
         // No arguments, just fallback to default construction         
         CallKnownDefaultConstructors<T>(count);
      }
      else if constexpr (CT::Sparse<T>) {
         static_assert(sizeof...(A) == 1, "Bad argument");
         using AA = FirstOf<A...>;

         // Construct pointers                                          
         auto lhs = const_cast<Block&>(*this).template GetRawAs<T>();
         const auto lhsEnd = lhs + count;
         auto lhsEntry = const_cast<Block&>(*this).GetEntries();

         while (lhs != lhsEnd) {
            if constexpr (CT::Handle<AA> and CT::Same<T, AA>) {
               // Set pointer and entry from handle                     
               (*lhs = ... = arguments.mPointer);
               (*lhsEntry = ... = arguments.mEntry);
            }
            else if constexpr (CT::Inner::MakableFrom<T, AA>) {
               // Set pointer and find entry                            
               (*lhs = ... = arguments);
               *lhsEntry = Allocator::Find(mType, *lhs);
            }
            else LANGULUS_ERROR("T is not constructible with these arguments");

            ++lhs;
            ++lhsEntry;
         }
      }
      else {
         // Construct dense stuff                                       
         auto lhs = const_cast<Block&>(*this).template GetRawAs<T>();
         const auto lhsEnd = lhs + count;
         while (lhs != lhsEnd) {
            if constexpr (::std::constructible_from<T, A...>)
               new (lhs++) T (arguments...);
            else
               LANGULUS_ERROR("T is not constructible with these arguments");
         }
      }
   }

   /// Call move constructors in a region and initialize memory               
   ///   @attention never modifies any block state                            
   ///   @attention assumes this is not initialized                           
   ///   @attention assumes blocks are binary-compatible                      
   ///   @attention assumes source has at least 'count' items                 
   ///   @attention assumes this has at least 'count' items reserved          
   ///   @tparam REVERSE - calls move constructors in reverse, to let you     
   ///                     account for potential memory overlap               
   ///   @param count - number of elements to move-construct                  
   ///   @param source - the source of the elements to move                   
   template<bool REVERSE>
   void Block::CallUnknownSemanticConstructors(
      const Count count, CT::Semantic auto&& source
   ) const {
      using S = Deref<decltype(source)>;
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");
      LANGULUS_ASSUME(DevAssumes, count <= source->mCount and count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, mType->IsExact(source->mType),
         "LHS and RHS are different types");

      // First make sure that reflected constructors are available      
      // There's no point in iterating anything otherwise               
      if constexpr (S::Move) {
         if constexpr (S::Keep) {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mMoveConstructor, Construct,
               "Can't move-construct elements "
               "- no move-constructor was reflected"
            );
         }
         else {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mAbandonConstructor, Construct,
               "Can't abandon-construct elements "
               "- no abandon-constructor was reflected"
            );
         }
      }
      else if constexpr (S::Shallow) {
         if constexpr (S::Keep) {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mCopyConstructor, Construct,
               "Can't copy-construct elements"
               " - no copy-constructor was reflected");
         }
         else {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mDisownConstructor, Construct,
               "Can't disown-construct elements"
               " - no disown-constructor was reflected");
         }
      }
      else {
         LANGULUS_ASSERT(
            mType->mIsSparse or mType->mCloneConstructor, Construct,
            "Can't clone-construct elements"
            " - no clone-constructor was reflected");
      }

      auto mthis = const_cast<Block*>(this);
      if (mType->mIsSparse) {
         // Both LHS and RHS are sparse                                 
         if constexpr (S::Shallow) {
            // Shallow pointer transfer                                 
            ShallowBatchPointerConstruction(count, source.Forward());
         }
         else if (not mType->mDeptr->mIsSparse
                  and (mType->mIsUnallocatable or not mType->mCloneConstructor)
         ) {
            // We early-return with an enforced shallow pointer         
            // transfer, because its requesting to clone                
            // unallocatable/unclonable/abstract data, such as metas    
            ShallowBatchPointerConstruction(count, Copy(*source));
         }
         else if (mType->mDeptr->mIsSparse or not mType->mResolver) {
            // If contained type is not resolvable (or is just          
            // another level of indirection), we can coalesce all       
            // clones into a single allocation                          
            Block clonedCoalescedSrc {mType->mDeptr};
            clonedCoalescedSrc.AllocateFresh(
               clonedCoalescedSrc.RequestSize(count));
            clonedCoalescedSrc.mCount = count;

            // Clone each inner element by nesting this call            
            auto lhs = mthis->template GetHandle<Byte*>(0);
            const auto lhsEnd = lhs.mValue + count;
            auto dst = clonedCoalescedSrc.GetElement();
            auto src = source->GetElement();
            while (lhs != lhsEnd) {
               dst.CallUnknownSemanticConstructors(
                  1, Clone(src.template GetDense<1>())
               );

               lhs.New(dst.mRaw, clonedCoalescedSrc.mEntry);
               dst.Next();
               src.Next();
               ++lhs;
            }

            const_cast<Allocation*>(clonedCoalescedSrc.mEntry)->Keep(count - 1);
         }
         else {
            // Type is resolved to dense elements of varying size,      
            // so we are forced to make a separate allocation for       
            // each of them                                             
            TODO();
         }
      }
      else if (mType->mIsPOD) {
         // Both are POD - Copy/Disown/Move/Abandon/Clone by memcpy     
         // all at once (batch optimization)                            
         const auto bytesize = mType->mSize * count;
         if constexpr (REVERSE)
            MoveMemory(mRaw, source->mRaw, bytesize);
         else
            CopyMemory(mRaw, source->mRaw, bytesize);
      }
      else {
         // Both RHS and LHS are dense and non-POD                      
         // We invoke reflected constructors for each element           
         const auto stride = mType->mSize;
         auto lhs = mRaw + (REVERSE ? (count - 1) * stride : 0);
         auto rhs = source->mRaw + (REVERSE ? (count - 1) * stride : 0);
         const auto rhsEnd = REVERSE ? rhs - count * stride : rhs + count * stride;

         while (rhs != rhsEnd) {
            if constexpr (S::Move) {
               if constexpr (S::Keep)
                  mType->mMoveConstructor(rhs, lhs);
               else
                  mType->mAbandonConstructor(rhs, lhs);
            }
            else if constexpr (S::Shallow) {
               if constexpr (S::Keep)
                  mType->mCopyConstructor(rhs, lhs);
               else
                  mType->mDisownConstructor(rhs, lhs);
            }
            else mType->mCloneConstructor(rhs, lhs);

            if constexpr (REVERSE) {
               lhs -= stride;
               rhs -= stride;
            }
            else {
               lhs += stride;
               rhs += stride;
            }
         }
      }
   }
   
   /// Call move constructors in a region and initialize memory               
   ///   @attention never modifies any block state                            
   ///   @attention assumes T is the exact type of both blocks                
   ///   @attention assumes count <= reserved elements                        
   ///   @attention assumes source contains at least 'count' items            
   ///   @tparam T - the type to move-construct                               
   ///   @tparam REVERSE - calls move constructors in reverse, to let you     
   ///                     account for potential memory overlap               
   ///   @param count - number of elements to move                            
   ///   @param source - the block of elements to move                        
   template<CT::Data T, bool REVERSE>
   void Block::CallKnownSemanticConstructors(
      const Count count, CT::Semantic auto&& source
   ) const {
      using S = Deref<decltype(source)>;
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      LANGULUS_ASSUME(DevAssumes, count <= source->mCount and count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "T doesn't match LHS type");
      LANGULUS_ASSUME(DevAssumes, source->template IsExact<T>(),
         "T doesn't match RHS type",
         ": ", source->GetType(), " != ", MetaDataOf<T>());

      const auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Sparse<T>) {
         using DT = Deptr<T>;

         if constexpr (S::Shallow) {
            // Shallow pointer transfer                                 
            ShallowBatchPointerConstruction(count, source.Forward());
         }
         else if constexpr (CT::Unallocatable<T> or not CT::CloneMakable<T>) {
            // We early-return with an enforced shallow pointer         
            // transfer, because its requesting to clone                
            // unallocatable/unclonable/abstract data, such as metas    
            ShallowBatchPointerConstruction(count, Copy(*source));
         }
         else if constexpr (CT::Sparse<DT> or not CT::Resolvable<T>) {
            // If contained type is not resolvable, or its deptr        
            // version is still a pointer, we can coalesce all          
            // clones into a single allocation (optimization)           
            Block clonedCoalescedSrc {mType->mDeptr};
            clonedCoalescedSrc.AllocateFresh(clonedCoalescedSrc.RequestSize(count));
            clonedCoalescedSrc.mCount = count;

            // Clone each inner element                                 
            auto handle = GetHandle<T>(0);
            auto dst = clonedCoalescedSrc.template GetRawAs<DT>();
            auto src = source->template GetRawAs<T>();
            const auto srcEnd = src + count;
            while (src != srcEnd) {
               SemanticNew(dst, Clone(**src));
               handle.New(dst, clonedCoalescedSrc.mEntry);

               ++dst;
               ++src;
               ++handle;
            }

            const_cast<Allocation*>(clonedCoalescedSrc.mEntry)->Keep(count - 1);
         }
         else {
            // Type can be resolved to objects of varying size, so      
            // we are forced to make a separate allocation for each     
            // element                                                  
            TODO();
         }
      }
      else if constexpr (CT::POD<T>) {
         // Both RHS and LHS are dense and POD                          
         auto lhs = mthis->template GetRawAs<T>();
         auto rhs = source->template GetRawAs<T>();
         if constexpr (REVERSE)
            MoveMemory(lhs, rhs, count);
         else
            CopyMemory(lhs, rhs, count);
      }
      else {
         // Both RHS and LHS are dense and non POD                      
         // Call constructor for each element (optionally in reverse)   
         auto lhs = mthis->template GetRawAs<T>();
         auto rhs = source->template GetRawAs<T>();
         if constexpr (REVERSE) {
            lhs += count - 1;
            rhs += count - 1;
         }
         const auto lhsEnd = REVERSE ? lhs - count : lhs + count;
         while (lhs != lhsEnd) {
            if constexpr (CT::Abandoned<S> and not CT::AbandonMakable<T>) {
               if constexpr (CT::MoveMakable<T>) {
                  // We can fallback to move-construction, but report   
                  // a performance warning                              
                  IF_SAFE(Logger::Warning(
                     "Move used, instead of abandon - implement an "
                     "abandon-constructor for type ", NameOf<T>(),
                     " to fix this warning"
                  ));
                  SemanticNew(lhs, Move(*rhs));
               }
               else LANGULUS_ERROR("T is not movable/abandon-constructible");
            }
            else SemanticNew(lhs, S::Nest(*rhs));

            if constexpr (REVERSE) {
               --lhs;
               --rhs;
            }
            else {
               ++lhs;
               ++rhs;
            }
         }
      }
   }
   
   /// Batch-optimized semantic pointer constructions                         
   ///   @attention overwrites pointers without dereferencing their memory    
   ///   @param count - number of elements to construct                       
   ///   @param source - source                                               
   inline void Block::ShallowBatchPointerConstruction(
      const Count count, CT::Semantic auto&& source
   ) const {
      using S = Deref<decltype(source)>;
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      const auto mthis = const_cast<Block*>(this);
      const auto pointersDst = mthis->GetRawSparse();
      const auto pointersSrc = source->GetRawSparse();
      const auto entriesDst = mthis->GetEntries();
      const auto entriesSrc = source->GetEntries();

      if constexpr (S::Move) {
         // Move/Abandon                                                
         MoveMemory(pointersDst, pointersSrc, count);
         MoveMemory(entriesDst, entriesSrc, count);

         // Reset source ownership                                      
         ZeroMemory(entriesSrc, count);

         // Reset source pointers, too, if not abandoned                
         if constexpr (S::Keep)
            ZeroMemory(pointersSrc, count);
      }
      else {
         // Copy/Disown                                                 
         CopyMemory(pointersDst, pointersSrc, count);
         CopyMemory(entriesDst, entriesSrc, count);

         if constexpr (S::Keep) {
            // Reference each entry, if not disowned                    
            auto entry = entriesDst;
            const auto entryEnd = entry + count;
            while (entry != entryEnd) {
               if (*entry)
                  const_cast<Allocation*>(*entry)->Keep();
               ++entry;
            }
         }
         else {
            // Otherwise make sure all entries are zero                 
            ZeroMemory(entriesDst, count);
         }
      }
   }
   
   /// Call semantic assignment in a region                                   
   ///   @attention never modifies any block state                            
   ///   @attention assumes blocks don't overlap (sparse elements may still   
   ///      overlap, but this is handled in the assignment operators)         
   ///   @attention assumes blocks are binary compatible                      
   ///   @attention assumes both blocks have at least 'count' items           
   ///   @param count - the number of elements to move-assign                 
   ///   @param source - the elements to assign                               
   void Block::CallUnknownSemanticAssignment(
      const Count count, CT::Semantic auto&& source
   ) const {
      using S = Deref<decltype(source)>;
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      LANGULUS_ASSUME(DevAssumes, mCount >= count and source->mCount >= count,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, mType->IsExact(source->mType),
         "LHS and RHS are different types");

      // First make sure that reflected assigners are available         
      // There's no point in iterating anything otherwise               
      if constexpr (S::Move) {
         if constexpr (S::Keep) {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mMoveAssigner, Construct,
               "Can't move-assign elements "
               "- no move-assigner was reflected"
            );
         }
         else {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mAbandonAssigner, Construct,
               "Can't abandon-assign elements "
               "- no abandon-assigner was reflected"
            );
         }
      }
      else if constexpr (S::Shallow) {
         if constexpr (S::Keep) {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mCopyAssigner, Construct,
               "Can't copy-assign elements"
               " - no copy-assigner was reflected");
         }
         else {
            LANGULUS_ASSERT(
               mType->mIsSparse or mType->mDisownAssigner, Construct,
               "Can't disown-assign elements"
               " - no disown-assigner was reflected");
         }
      }
      else {
         LANGULUS_ASSERT(
            mType->mIsSparse or mType->mCloneAssigner, Construct,
            "Can't clone-assign elements"
            " - no clone-assigner was reflected");
      }

      const auto mthis = const_cast<Block*>(this);
      if (mType->mIsSparse) {
         // Since we're overwriting pointers, we have to dereference    
         // the old ones, but conditionally reference the new ones      
         auto lhs = mthis->GetRawSparse();
         const auto lhsEnd = lhs + count;
         auto rhs = source->GetRawSparse();
         auto lhsEntry = mthis->GetEntries();
         auto rhsEntry = source->GetEntries();

         while (lhs != lhsEnd) {
            if (*lhsEntry) {
               // Free old LHS                                          
               if ((*lhsEntry)->GetUses() == 1) {
                  mType->mOrigin->mDestructor(*lhs);
                  Allocator::Deallocate(const_cast<Allocation*>(*lhsEntry));
               }
               else const_cast<Allocation*>(*lhsEntry)->Free();
            }

            if constexpr (S::Move) {
               // Move/Abandon RHS in LHS                               
               *lhs = const_cast<Byte*>(*rhs);
               *lhsEntry = *rhsEntry;
               *rhsEntry = nullptr;

               if constexpr (S::Keep) {
                  // We're not abandoning RHS, make sure it's cleared   
                  *rhs = nullptr;
               }
            }
            else if constexpr (S::Shallow) {
               // Copy/Disown RHS in LHS                                
               *lhs = const_cast<Byte*>(*rhs);

               if constexpr (S::Keep) {
                  *lhsEntry = *rhsEntry;
                  if (*lhsEntry)
                     const_cast<Allocation*>(*lhsEntry)->Keep();
               }
               else *lhsEntry = nullptr;
            }
            else {
               // Clone RHS in LHS                                      
               TODO();
            }

            ++lhs;
            ++rhs;
            ++lhsEntry;
            ++rhsEntry;
         }
      }
      else if (mType->mIsPOD) {
         // Both RHS and LHS are dense and POD                          
         // So we batch-overwrite them at once                          
         const auto bytesize = mType->mSize * count;
         CopyMemory(mRaw, source->mRaw, bytesize);
      }
      else {
         // Both RHS and LHS are dense and non-POD                      
         // We invoke reflected assignments for each element            
         const auto stride = mType->mSize;
         auto lhs = mRaw;
         auto rhs = source->mRaw;
         const auto rhsEnd = rhs + count * stride;

         while (rhs != rhsEnd) {
            if constexpr (S::Move) {
               if constexpr (S::Keep)
                  mType->mMoveAssigner(rhs, lhs);
               else
                  mType->mAbandonAssigner(rhs, lhs);
            }
            else if constexpr (S::Shallow) {
               if constexpr (S::Keep)
                  mType->mCopyAssigner(rhs, lhs);
               else
                  mType->mDisownAssigner(rhs, lhs);
            }
            else mType->mCloneAssigner(rhs, lhs);

            lhs += stride;
            rhs += stride;
         }
      }
   }

   /// Call semantic assignment in a region                                   
   ///   @attention don't assign to overlapping memory regions!               
   ///   @attention never modifies any block state                            
   ///   @attention assumes blocks are binary compatible                      
   ///   @attention assumes both blocks have at least 'count' items           
   ///   @tparam T - the type to use for the assignment                       
   ///   @param count - the number of elements to move-assign                 
   ///   @param source - the elements to assign                               
   template<CT::Data T>
   void Block::CallKnownSemanticAssignment(Count count, CT::Semantic auto&& source) const {
      using S = Deref<decltype(source)>;
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");
      static_assert(CT::Mutable<T>,
         "Can't assign to container filled with constant items");

      LANGULUS_ASSUME(DevAssumes, count <= source->mCount and count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "T doesn't match LHS type");
      LANGULUS_ASSUME(DevAssumes, source->template IsExact<T>(),
         "T doesn't match RHS type",
         ": ", source->GetType(), " != ", MetaDataOf<T>());

      const auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Sparse<T>) {
         // We're reassigning pointers                                  
         using DT = Deptr<T>;

         if constexpr (S::Shallow) {
            // Shallow pointer transfer                                 
            CallKnownDestructors<T>();
            ShallowBatchPointerConstruction(count, source.Forward());
         }
         else if constexpr (CT::Unallocatable<T> or not CT::CloneAssignable<T>) {
            // We early-return with an enforced shallow pointer         
            // transfer, because its requesting to clone                
            // unallocatable/unclonable/abstract data, such as metas    
            CallKnownDestructors<T>();
            ShallowBatchPointerConstruction(count, Copy(*source));
         }
         else if constexpr (CT::Sparse<DT> or not CT::Resolvable<T>) {
            // If contained type is not resolvable, or its deptr        
            // version is still a pointer, we can coalesce all          
            // clones into a single allocation (optimization)           
            Block clonedCoalescedSrc {mType->mDeptr};
            clonedCoalescedSrc.AllocateFresh(clonedCoalescedSrc.RequestSize(count));
            clonedCoalescedSrc.mCount = count;

            // Clone each inner element                                 
            auto handle = GetHandle<T>(0);
            auto dst = clonedCoalescedSrc.template GetRawAs<DT>();
            auto src = source->template GetRawAs<T>();
            const auto srcEnd = src + count;
            while (src != srcEnd) {
               SemanticNew(dst, Clone(**src));
               handle.Assign(dst, clonedCoalescedSrc.mEntry);
               ++dst;
               ++src;
               ++handle;
            }

            const_cast<Allocation*>(clonedCoalescedSrc.mEntry)
               ->Keep(count - 1);
         }
         else {
            // Type can be resolved to objects of varying size, so      
            // we are forced to make a separate allocation for each     
            // element                                                  
            TODO();
         }
      }
      else if constexpr (CT::POD<T>) {
         // Both RHS and LHS are dense and POD                          
         // So we batch-overwrite them at once                          
         const auto bytesize = mType->mSize * count;
         CopyMemory(mRaw, source->mRaw, bytesize);
      }
      else {
         // Both RHS and LHS are dense and non POD                      
         // Assign to each element                                      
         auto lhs = mthis->template GetRawAs<T>();
         auto rhs = source->template GetRawAs<T>();
         const auto lhsEnd = lhs + count;
         while (lhs != lhsEnd) {
            SemanticAssign(lhs, S::Nest(*rhs));
            ++lhs;
            ++rhs;
         }
      }
   }

} // namespace Langulus::Anyness