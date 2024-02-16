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

   /// Allocate 'count' elements and fill the container with zeroes           
   /// If T is not CT::Nullifiable, this function does default construction,  
   /// which would be slower, than batch zeroing                              
   ///   @param count - number of elements to zero-construct                  
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::Null(const Count count) {
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;

         if constexpr (CT::Nullifiable<T>) {
            if (count < mReserved)
               AllocateLess<THIS>(count);
            else
               AllocateMore<THIS, false, true>(count);

            ZeroMemory(GetRawAs<T>(), count);
         }
         else New(count);
      }
      else TODO();
   }

   /// Extend the container by default construction, and return the new part  
   ///   @attention if extending memory without jurisdiction, the container   
   ///      will take authority and diverge                                   
   ///   @param count - the number of elements to extend by                   
   ///   @return a container that represents only the extended part           
   template<CT::Block THIS> LANGULUS(INLINED)
   THIS Block::Extend(const Count count) {
      const auto previousCount = mCount;
      AllocateMore<THIS, true>(mCount + count);
      const auto newRegion = CropInner(previousCount, count);
      return Copy(reinterpret_cast<const THIS&>(newRegion));
   }

   /// Create N new elements, using default construction                      
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @return the number of new elements                                   
   template<CT::Block THIS> LANGULUS(INLINED)
   Count Block::New(const Count count) {
      AllocateMore<THIS>(mCount + count);
      CropInner(mCount, count).CreateDefault<THIS>();
      mCount += count;
      return count;
   }
   
   /// Create N new elements, using the provided arguments for construction   
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @param arguments... - constructor arguments, all forwarded together  
   ///      for each instance of T                                            
   ///   @return the number of new elements                                   
   template<CT::Block THIS, class...A> LANGULUS(INLINED)
   Count Block::New(const Count count, A&&...arguments) {
      LANGULUS_ASSUME(UserAssumes, count, "Zero count not allowed");
      AllocateMore<THIS>(mCount + count);
      CropInner(mCount, count).Create<THIS>(Forward<A>(arguments)...);
      mCount += count;
      return count;
   }

   /// Inner semantic function for a contiguous range-insertion               
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @tparam T1 - the type that S<Block> contains (void for type-erasure) 
   ///   @param index - the offset at which to start inserting                
   ///   @param data - data and semantic to use                               
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE, class T1, template<class> class S>
   requires CT::Semantic<S<Block>>
   void Block::InsertBlockInner(CT::Index auto index, S<Block>&& data) {
      // Infer inserted type first from THIS, then from T1              
      // If both are void, then we have a type-erased insertion         
      using T = Conditional<CT::Typed<THIS>, TypeOf<THIS>, Decvq<T1>>;
      auto& me = reinterpret_cast<THIS&>(*this);

      if constexpr (CT::CanBeDeepened<FORCE, THIS> and MOVE_ASIDE) {
         // Type may mutate                                             
         bool depened;
         if constexpr (CT::TypeErased<T>)
            depened = Mutate<THIS, FORCE>(data->GetType());
         else
            depened = Mutate<THIS, T, FORCE>();

         // If reached, then type mutated to a deep type                
         if (depened) {
            FORCE temp;
            temp.template InsertBlockInner<FORCE, void, true, T1>(
               IndexBack, Forward<S<Block>>(data));
            Insert<THIS, void, true>(index, Abandon(temp));
            return;
         }
      }
      else {
         // Type can't mutate, but we still have to check if compatible 
         if constexpr (CT::TypeErased<T1>) {
            // This branch will always do a slower run-time type check  
            LANGULUS_ASSERT(me.IsSimilar(data->GetType()), Meta,
               "Inserting incompatible type `", data->GetType(),
               "` to container of type `", me.GetType(), '`'
            );
         }
         else {
            // This branch can potentially happen at compile-time       
            // It's the happy path                                      
            LANGULUS_ASSERT(me.template IsSimilar<T1>(), Meta,
               "Inserting incompatible type `", MetaDataOf<T1>(),
               "` to container of type `", me.GetType(), '`'
            );
         }
      }

      // If reached, then we have binary compatible type, so allocate   
      const auto count = data->GetCount();
      const auto idx = SimplifyIndex<THIS, false>(index);

      if constexpr (MOVE_ASIDE) {
         AllocateMore<THIS>(mCount + count);

         if (idx < mCount) {
            // Move memory if required                                  
            LANGULUS_ASSERT(GetUses() == 1, Move,
               "Moving elements that are used from multiple places");

            // We're moving to the right, so make sure we do it in      
            // reverse to avoid any potential overlap                   
            const auto moved = mCount - idx;
            CropInner(idx + count, moved)
               .template CreateSemantic<THIS, true>(
                  Abandon(CropInner(idx, moved)));
         }
      }

      // Construct data in place                                        
      CropInner(idx, count).template CreateSemantic<THIS>(data.Forward());
      mCount += count;
   }

   /// Inner semantic insertion function                                      
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the offset at which to insert                         
   ///   @param item - item (and semantic) to insert                          
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE>
   void Block::InsertInner(CT::Index auto index, auto&& item) {
      using S = SemanticOf<decltype(item)>;
      auto& me = reinterpret_cast<THIS&>(*this);

      if constexpr (CT::Similar<S, Describe>) {
         // We're using descriptor constructors                         
         // For this to work, contained type must be known              
         DMeta type = me.GetType();
         LANGULUS_ASSERT(type, Meta,
            "Unknown type, can't insert via descriptor");
         LANGULUS_ASSERT(type->mDescriptorConstructor, Meta,
            "Type is not descriptor-constructible");

         const auto idx = SimplifyIndex<THIS, false>(index);

         if constexpr (MOVE_ASIDE) {
            AllocateMore<THIS>(mCount + 1);

            if (idx < mCount) {
               // Move memory if required                               
               LANGULUS_ASSERT(GetUses() == 1, Move,
                  "Moving elements that are used from multiple places");

               // We're moving to the right, so make sure we do it in   
               // reverse to avoid any potential overlap                
               const auto moved = mCount - idx;
               CropInner(idx + 1, moved)
                  .template CreateSemantic<THIS, true>(
                     Abandon(CropInner(idx, moved)));
            }
         }

         CropInner(idx, 1).template CreateDescribe<THIS>(*item);
      }
      else {
         using T = Conditional<CT::Typed<THIS>, TypeOf<THIS>, TypeOf<S>>;
         static_assert(CT::Insertable<T>, "T is not insertable");

         if constexpr (CT::CanBeDeepened<FORCE, THIS> and MOVE_ASIDE) {
            // Type may mutate                                          
            if (Mutate<THIS, T, FORCE>()) {
               // If reached, then type mutated to a deep type          
               FORCE temp {S::Nest(item)};
               Insert<THIS, void, true>(index, Abandon(temp));
               return;
            }
         }
         else {
            // We still have to mutate if untyped - this also acts      
            // as a runtime type-check                                  
            Mutate<THIS, T, void>();
         }

         const auto idx = SimplifyIndex<THIS, false>(index);

         // If reached, we have compatible type, so allocate            
         if constexpr (MOVE_ASIDE) {
            AllocateMore<THIS>(mCount + 1);

            if (idx < mCount) {
               // Move memory if required                               
               LANGULUS_ASSERT(GetUses() == 1, Move,
                  "Moving elements that are used from multiple places");

               // We're moving to the right, so make sure we do it in   
               // reverse to avoid any potential overlap                
               const auto moved = mCount - idx;
               CropInner(idx + 1, moved)
                  .template CreateSemantic<THIS, true>(
                     Abandon(CropInner(idx, moved)));
            }
         }

         GetHandle<T, THIS>(idx).CreateSemantic(S::Nest(item));
      }

      ++mCount;
   }

   /// Insert an element, or an array of elements                             
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the index at which to insert                          
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE>
   Count Block::UnfoldInsert(CT::Index auto index, auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;
      
      if constexpr (CT::Array<T>) {
         if constexpr (CT::StringLiteral<T>) {
            // Implicitly convert string literals to Text containers    
            InsertInner<THIS, FORCE, MOVE_ASIDE>(
               index, Text {S::Nest(item)});
            return 1;
         }
         else {
            // Insert the array                                         
            InsertBlockInner<THIS, FORCE, MOVE_ASIDE, Deext<T>>(
               index, S::Nest(Block::From(item)));
            return ExtentOf<T>;
         }
      }
      else {
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         InsertInner<THIS, FORCE, MOVE_ASIDE>(index, S::Nest(item));
         return 1;
      }
   }

   /// Insert an element, or an array of elements, if not found               
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the index at which to insert                          
   ///   @param item - the argument to unfold and insert, can be semantic     
   ///   @return the number of inserted elements after unfolding              
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE>
   Count Block::UnfoldMerge(CT::Index auto index, auto&& item) {
      using S = SemanticOf<decltype(item)>;
      using T = TypeOf<S>;
      
      if constexpr (CT::Array<T>) {
         if constexpr (CT::StringLiteral<T>) {
            // Implicitly convert string literals to Text containers    
            if (CT::Void<FORCE> and not IsSimilar<THIS, Text>())
               return 0;

            Text text {S::Nest(item)};
            if (not IsSimilar<THIS, Text>() or not Find<false, THIS>(text)) {
               InsertInner<THIS, FORCE, MOVE_ASIDE>(
                  index, Abandon(text));
               return 1;
            }
         }
         else {
            // Insert the array                                         
            using DT = Deext<T>;
            if (CT::Void<FORCE> and not IsSimilar<THIS, DT>())
               return 0;

            const auto data = Block::From(item);
            if (not IsSimilar<THIS, DT>() or not FindBlock<false, THIS>(data, IndexFront)) {
               InsertBlockInner<THIS, FORCE, MOVE_ASIDE, DT>(
                  index, S::Nest(data));
               return ExtentOf<T>;
            }
         }
      }
      else {
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         if (CT::Void<FORCE> and not IsSimilar<THIS, T>())
            return 0;

         if (not IsSimilar<THIS, T>() or not Find<false, THIS>(DesemCast(item))) {
            InsertInner<THIS, FORCE, MOVE_ASIDE>(
               index, S::Nest(item));
            return 1;
         }
      }

      return 0;
   }

   /// Insert elements at a given index, semantically or not                  
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param idx - the index at which to insert                            
   ///   @param t1 - the first item to insert                                 
   ///   @param tail... - the rest of items to insert (optional)              
   ///   @return number of inserted elements                                  
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE, class T1, class...TAIL>
   LANGULUS(INLINED)
   Count Block::Insert(CT::Index auto idx, T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
        inserted += UnfoldInsert<THIS, FORCE, MOVE_ASIDE>(
         idx, Forward<T1>(t1));
      ((inserted += UnfoldInsert<THIS, FORCE, MOVE_ASIDE>(
         idx, Forward<TAIL>(tail))), ...);
      return inserted;
   }
   
   /// Insert all elements of a block at an index, semantically or not        
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - index to insert thems at                              
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE, class T>
   requires CT::Block<Desem<T>> LANGULUS(INLINED)
   Count Block::InsertBlock(CT::Index auto index, T&& other) {
      using S = SemanticOf<decltype(other)>;
      using ST = TypeOf<S>;
      auto& rhs = DesemCast(other);
      const auto count = rhs.GetCount();
      if (not count)
         return 0;

      // Insert all elements                                            
      if constexpr (CT::Typed<ST>) {
         InsertBlockInner<THIS, FORCE, MOVE_ASIDE, TypeOf<ST>>(
            index, S::Nest(rhs).template Forward<Block>());
      }
      else {
         InsertBlockInner<THIS, FORCE, MOVE_ASIDE>(
            index, S::Nest(rhs).template Forward<Block>());
      }

      if constexpr (S::Move and S::Keep and ST::Ownership) {
         // All elements were moved, only empty husks remain            
         // so destroy them, and discard ownership of 'other'           
         rhs.template Free<Desem<T>>();
      }

      return count;
   }
   
   /// Merge a single element by a semantic                                   
   /// Element will be pushed only if not found in block                      
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the index at which to insert                          
   ///   @param t1 - the first item to insert                                 
   ///   @param tail... - the rest of items to insert (optional)              
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE, class T1, class...TAIL>
   LANGULUS(INLINED)
   Count Block::Merge(CT::Index auto index, T1&& t1, TAIL&&...tail) {
      Count inserted = 0;
        inserted += UnfoldMerge<THIS, FORCE, MOVE_ASIDE>(
         index, Forward<T1>(t1));
      ((inserted += UnfoldMerge<THIS, FORCE, MOVE_ASIDE>(
         index, Forward<TAIL>(tail))), ...);
      return inserted;
   }

   /// Search for a sequence of elements, and if not found, semantically      
   /// insert it                                                              
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - index to insert at                                    
   ///   @param other - the block to search for, and eventually insert        
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE, class T>
   requires CT::Block<Desem<T>> LANGULUS(INLINED)
   Count Block::MergeBlock(CT::Index auto index, T&& other) {
      using S = SemanticOf<decltype(other)>;
      Count inserted = 0;
      if (not FindBlock(DesemCast(other), IndexFront)) {
         inserted += InsertBlock<THIS, FORCE, MOVE_ASIDE>(
            index, S::Nest(other));
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
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param idx - the index to emplace at                                 
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return 1 if the element was emplaced successfully                   
   template<CT::Block THIS, bool MOVE_ASIDE, class...A> LANGULUS(INLINED)
   Count Block::Emplace(CT::Index auto idx, A&&...arguments) {
      const auto offset = SimplifyIndex<THIS, false>(idx);

      if constexpr (MOVE_ASIDE) {
         AllocateMore<THIS>(mCount + 1);

         if (offset < mCount) {
            // Move memory if required                                  
            LANGULUS_ASSERT(GetUses() == 1, Move,
               "Emplacing elements to memory block, used from multiple places, "
               "requires memory to move");

            // We're moving to the right, so make sure we do it in      
            // reverse to avoid any overlap                             
            const auto tail = mCount - offset;
            CropInner(offset + 1, tail)
               .template CreateSemantic<THIS, true>(
                  Abandon(CropInner(offset, tail)));
         }
      }

      CropInner(offset, 1).template Create<THIS>(Forward<A>(arguments)...);
      ++mCount;
      return 1;
   }
   
   /// Wrap all contained elements inside a sub-block, making this one deep   
   ///   @tparam T - the type of deep container to use                        
   ///   @tparam TRANSFER_OR - whether to send the current orness deeper      
   ///   @return a reference to this container                                
   template<CT::Deep T, bool TRANSFER_OR, CT::Block THIS>
   requires CT::CanBeDeepened<T, THIS>
   LANGULUS(INLINED) T& Block::Deepen() {
      auto& me = reinterpret_cast<THIS&>(*this);
      LANGULUS_ASSERT(not me.IsTypeConstrained()
                       or me.template IsSimilar<T>(),
         Mutate, "Can't deepen with incompatible type");

      // Back up the state so that we can restore it if not moved over  
      UNUSED() const DataState state = mState.mState & DataState::Or;
      if constexpr (not TRANSFER_OR)
         mState -= state;

      // Allocate a new T and move this inside it                       
      Block wrapper;
      wrapper.template SetType<T, false, THIS>();
      wrapper.template AllocateMore<TAny<T>, true>(1);
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

         const bool stateCompliant = CanFitState<THIS>(DesemCast(value));
         if (IsEmpty() and not DesemCast(value).IsStatic() and stateCompliant) {
            Free<THIS>();
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
   template<CT::Block THIS, class FORCE, template<class> class S, CT::Deep T>
   requires CT::Semantic<S<T>> LANGULUS(INLINED)
   Count Block::SmartConcat(
      CT::Index auto index, bool sc, S<T>&& value, DataState state
   ) {
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
            SetType<false, THIS>(value->GetType());
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
   template<CT::Block THIS, class FORCE, template<class> class S, class T>
   requires CT::Semantic<S<T>> LANGULUS(INLINED)
   Count Block::SmartPushInner(
      CT::Index auto index, S<T>&& value, DataState state
   ) {
      if (IsUntyped<THIS>() and IsInvalid()) {
         // Mutate-insert inside untyped container                      
         SetState(mState + state);
         return Insert<THIS, void>(index, value.Forward());
      }
      else if (IsExact<THIS, T>()) {
         // Insert to a same-typed container                            
         SetState(mState + state);
         return Insert<THIS, void>(index, value.Forward());
      }
      else if (IsEmpty() and IsTyped<THIS>() and not IsTypeConstrained<THIS>()) {
         // If incompatibly typed but empty and not constrained, we     
         // can still reset the container and reuse it                  
         Reset<THIS>();
         SetState(mState + state);
         return Insert<THIS, void>(index, value.Forward());
      }
      else if (IsDeep<THIS>()) {
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
   ///   @param rhs - block and semantic to concatenate with (right side)     
   ///   @return the concatenated container                                   
   template<CT::Block THIS, template<class> class S, CT::Block T>
   requires CT::Semantic<S<T>> LANGULUS(INLINED)
   THIS Block::ConcatBlock(S<T>&& rhs) const {
      auto& lhs = reinterpret_cast<const THIS&>(*this);
      if (IsEmpty())
         return {rhs.Forward()};
      else if (rhs->IsEmpty())
         return lhs;

      if (GetUses() == 1) {
         // Silently append rhs to this block's memory, to save on a    
         // reallocation. This block will remain the same, but it will  
         // diverge, if changed in the future                           
         const auto countBackup = mCount;
         const_cast<Block*>(this)->template
            InsertBlock<THIS, void>(IndexBack, rhs.Forward());
         Block result = *this;
         const_cast<Block*>(this)->mCount = countBackup;
         return reinterpret_cast<THIS&>(result);
      }

      // Allocate a new concatenated container, and push inside         
      THIS result;
      if constexpr (CT::Untyped<THIS>)
         result.template SetType<false>(rhs->GetType());

      result.Block::template AllocateFresh<THIS>(
         result.Block::template RequestSize<THIS>(mCount + rhs->GetCount()));
      result.Block::template InsertBlock<THIS, void, false>(
         IndexBack, Copy(lhs));
      result.Block::template InsertBlock<THIS, void, false>(
         IndexBack, rhs.Forward());
      return Abandon(result);
   }

   /// Call default constructors in a region and initialize memory            
   ///   @attention never modifies any block state                            
   ///   @attention assumes block elements are not initialized, despite having
   ///      mCount set                                                        
   template<CT::Block THIS>
   void Block::CreateDefault() const {
      LANGULUS_ASSUME(DevAssumes, mCount and mCount <= mReserved,
         "Count outside limits", '(', mCount, " > ", mReserved);

      auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;

         if constexpr (CT::Sparse<T>) {
            // Zero pointers and entries                                
            ZeroMemory(mthis->mRawSparse, mCount);
            ZeroMemory(mthis->GetEntries<THIS>(), mCount);
         }
         else if constexpr (CT::Nullifiable<T>) {
            // Zero the dense memory (optimization)                     
            ZeroMemory(mthis->GetRaw<THIS>(), mCount);
         }
         else if constexpr (CT::Defaultable<T>) {
            // Construct requested elements one by one                  
            auto to = mthis->GetRaw<THIS>();
            const auto toEnd = to + mCount;
            while (to != toEnd)
               new (to++) T {};
         }
         else LANGULUS_ERROR(
            "Trying to default-construct elements that are "
            "incapable of default-construction");
      }
      else {
         if (mType->mIsSparse) {
            // Zero pointers and entries                                
            ZeroMemory(mRawSparse, mCount);
            ZeroMemory(mthis->GetEntries<THIS>(), mCount);
         }
         else if (mType->mIsNullifiable) {
            // Zero the dense memory (optimization)                     
            ZeroMemory(mRaw, mCount * mType->mSize);
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
            const auto toEnd = to + mCount * stride;
            while (to != toEnd) {
               mType->mDefaultConstructor(to);
               to += stride;
            }
         }
      }
   }
   
   /// Call descriptor constructors in a region, initializing memory          
   ///   @attention never modifies any block state                            
   ///   @attention assumes block elements are not initialized, despite having
   ///      mCount set                                                        
   ///   @param desc - the descriptor to pass on to constructors              
   template<CT::Block THIS, class...A>
   void Block::CreateDescribe(A&&...arguments) const {
      static_assert(sizeof...(A) > 0, "Bad number of arguments");
      LANGULUS_ASSUME(DevAssumes, mCount and mCount <= mReserved,
         "Count outside limits", '(', mCount, " > ", mReserved);

      const auto getNeat = [&] {
         if constexpr (sizeof...(A) == 1) {
            using A1 = FirstOf<A...>;
            if constexpr (CT::Similar<A1, Describe>) {
               const Neat* t;
               (t = ... = &*arguments);
               return *t;
            }
            else if constexpr (CT::Similar<A1, Neat>) {
               const Neat* t;
               (t = ... = &arguments);
               return *t;
            }
            else return Neat {Forward<A>(arguments)...};
         }
         else return Neat {Forward<A>(arguments)...};
      };

      auto mthis = reinterpret_cast<THIS*>(const_cast<Block*>(this));
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         static_assert(CT::DescriptorMakable<T>,
            "T is not descriptor-constructible");

         if constexpr (CT::Sparse<T>) {
            // Bulk-allocate the required count, construct each         
            // instance and push the pointers                           
            auto lhsPtr = mthis->GetRawSparse();
            auto lhsEnt = mthis->GetEntries();
            const auto lhsEnd = lhsPtr + mCount;
            const auto allocation = Allocator::Allocate(
               MetaDataOf<Decay<T>>(),
               sizeof(Decay<T>) * mCount
            );
            allocation->Keep(mCount - 1);

            auto rhs = allocation->template As<Decay<T>*>();
            while (lhsPtr != lhsEnd) {
               new (rhs) Decay<T> {Describe(getNeat())};
               *(lhsPtr++) = rhs;
               *(lhsEnt++) = allocation;
               ++rhs;
            }
         }
         else {
            // Construct all dense elements in place                    
            auto lhs = mthis->GetRaw();
            const auto lhsEnd = lhs + mCount;
            while (lhs != lhsEnd) {
               new (lhs++) Decay<T> {Describe(getNeat())};
            }
         }
      }
      else {
         LANGULUS_ASSUME(DevAssumes, IsTyped(),
            "Block is expected to be typed");
         LANGULUS_ASSERT(
            mType->mDescriptorConstructor, Construct,
            "Can't descriptor-construct ", '`', mType,
            "` - no descriptor-constructor reflected"
         );

         if (mType->mDeptr) {
            if (not mType->mDeptr->mIsSparse) {
               // Bulk-allocate the required count, construct each      
               // instance and set the pointers                         
               auto lhsPtr = mthis->GetRawSparse();
               auto lhsEnt = mthis->GetEntries();
               const auto lhsEnd = lhsPtr + mCount;
               const auto allocation = Allocator::Allocate(
                  mType->mOrigin,
                  mType->mOrigin->mSize * mCount
               );
               allocation->Keep(mCount - 1);

               auto rhs = allocation->GetBlockStart();
               while (lhsPtr != lhsEnd) {
                  mType->mOrigin->mDescriptorConstructor(rhs, getNeat());
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
            const auto lhsEnd = lhs + mCount * mType->mSize;
            while (lhs != lhsEnd) {
               mType->mDescriptorConstructor(lhs, getNeat());
               lhs += mType->mSize;
            }
         }
      }
   }
   
   /// Construct region.mCount items of THIS container's type in the specified
   /// region by forwarding A... as constructor arguments                     
   /// If this container is type-erased, exact constructor signatures aren't  
   /// reflected, and the following stock constructors will be attempted:     
   ///   1. If A is a single argument of exactly the same type, the reflected 
   ///      move constructor will be used, if available                       
   ///   2. If A is empty, the reflected default constructor is used          
   ///   3. If A is not empty, not exactly same as the contained type, or     
   ///      is more than a single argument, then all arguments will be        
   ///      wrapped in a Neat, and then forwarded to the descriptor-          
   ///      constructor, if such is reflected for the type                    
   ///   If none of these constructors are available, this function throws    
   ///   Except::Construct                                                    
   ///   @attention this is assumed to have no initialized elements, despite  
   ///      having its mCount set                                             
   ///   @attention be mindful when initializing multiple elements with       
   ///      move/abandon semantics, since those might move data away          
   ///      from arguments, thus ruining initialization of all elements,      
   ///      except the first one                                              
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return the number of emplaced elements                              
   template<CT::Block THIS, class...A>
   void Block::Create(A&&...arguments) const {
      LANGULUS_ASSUME(DevAssumes, mCount and mCount <= mReserved,
         "Count outside limits", '(', mCount, " > ", mReserved);
      auto mthis = reinterpret_cast<THIS*>(const_cast<Block*>(this));

      if constexpr (sizeof...(A) == 0) {
         // Attempt default construction                                
         CreateDefault<THIS>();
      }
      else if constexpr (CT::Typed<THIS>) {
         // Construct by directly checking if arguments satisfy a       
         // constructor signature, knowing what the contained type is   
         // at compile time                                             
         using T = TypeOf<THIS>;
         if constexpr (::std::constructible_from<T, A...>) {
            auto lhs = mthis->template GetRaw<THIS>();
            const auto lhsEnd = lhs + mCount;
            while (lhs != lhsEnd)
               new (lhs++) T (Forward<A>(arguments)...);

            if constexpr (sizeof...(A) == 1 and CT::Sparse<T>) {
               // We just copied a pointer multiple times, make sure    
               // we reference the memory behind it, if we own it       
               const Allocation* allocation = nullptr;
               auto ent = mthis->template GetEntries<THIS>();
               const auto entEnd = ent + mCount;

               allocation = Allocator::Find(MetaDataOf<Deptr<T>>(), *(--lhs));
               if (allocation)
                  const_cast<Allocation*>(allocation)->Keep(mCount);

               while (ent != entEnd)
                  *(ent++) = allocation;
            }
         }
         else CreateDescribe<THIS>(Forward<A>(arguments)...);
      }
      else {
         // Constructing type-erased items                              
         // We expect, that type has been previously set                
         LANGULUS_ASSUME(DevAssumes, IsTyped(),
            "Block was expected to be typed");

         if constexpr (sizeof...(A) == 1) {
            using F = Decvq<Deref<FirstOf<A...>>>;

            if constexpr (CT::Similar<F, Describe>) {
               // We have a descriptor for argument, forward it to the  
               // reflected descriptor constructor, if any              
               CreateDescribe<THIS>(Forward<A>(arguments)...);
            }
            else if constexpr (CT::Semantic<F>) {
               // We have a semantic for argument - extract inner type, 
               // check if compatible, and if so - forward it to the    
               // appropriate reflected semantic constructor, if any    
               using FT = TypeOf<F>;
               if (IsSimilar<THIS, FT>())
                  Create<TAny<FT>>(Forward<A>(arguments)...);
               else
                  CreateDescribe<THIS>(Forward<A>(arguments)...);
            }
            else if (IsSimilar<THIS, F>())
               Create<TAny<F>>(Forward<A>(arguments)...);
            else
               CreateDescribe<THIS>(Forward<A>(arguments)...);
         }
         else CreateDescribe<THIS>(Forward<A>(arguments)...);
      }
   }

   /// Call semantic constructors in a region and initialize memory, by       
   /// using a Block as a source                                              
   ///   @attention never modifies any block state                            
   ///   @attention assumes none of the elements are constructed, despite     
   ///      having a valid mCount                                             
   ///   @attention assumes blocks types are similar                          
   ///   @attention assumes source has at least mCount items                  
   ///   @tparam REVERSE - calls constructors in reverse, to let you          
   ///                     account for potential memory overlap               
   ///   @param source - the source of the elements to initialize with        
   template<CT::Block THIS, bool REVERSE, template<class> class S, CT::Block OTHER>
   requires CT::Semantic<S<OTHER>>
   void Block::CreateSemantic(S<OTHER>&& source) const {
      LANGULUS_ASSUME(DevAssumes, mCount and mCount <= mReserved,
         "Count outside limits", '(', mCount, " > ", mReserved);
      LANGULUS_ASSUME(DevAssumes, mCount <= source->mCount,
         "Count mismatch");
      // Type-erased pointers (void*) are acceptable                    
      LANGULUS_ASSUME(DevAssumes, 
            (source->GetType()->IsSimilar(GetType<THIS>())
         or (source->GetType()->template IsSimilar<void*>() and IsSparse<THIS>())
         or (source->GetType()->mIsSparse and GetType<THIS>()->template IsSimilar<void*>())),
         "Type mismatch on creation", ": ", source->GetType(), " != ", GetType<THIS>());

      using B = Conditional<CT::Typed<THIS>, THIS, OTHER>;
      using T = Conditional<CT::Typed<B>, TypeOf<B>, void>;
      using SS = S<B>;
      const auto mthis = reinterpret_cast<B*>(const_cast<Block*>(this));
      const auto other = reinterpret_cast<B*>(const_cast<OTHER*>(&(*source)));

      if constexpr (not CT::TypeErased<T>) {
         // Leverage the fact, that containers are statically typed     
         if constexpr (CT::Sparse<T>) {
            using DT = Deptr<T>;
            if constexpr (SS::Shallow) {
               // Shallow pointer transfer                              
               ShallowBatchPointerConstruction(source.Forward());
            }
            else if constexpr (CT::Unallocatable<Decay<T>> or not CT::CloneMakable<T>) {
               // We early-return with an enforced shallow pointer      
               // transfer, because its requesting to clone             
               // unallocatable/unclonable/abstract data, such as metas 
               ShallowBatchPointerConstruction(Copy(*source));
            }
            else if constexpr (CT::Sparse<DT> or not CT::Resolvable<T>) {
               // If contained type is not resolvable, or its deptr     
               // version is still a pointer, we can coalesce all       
               // clones into a single allocation (optimization)        
               Block clonedCoalescedSrc {mType->mDeptr};
               clonedCoalescedSrc.AllocateFresh<Any>(
                  clonedCoalescedSrc.RequestSize<Any>(mCount));
               clonedCoalescedSrc.mCount = mCount;

               // Clone each inner element                              
               auto handle = GetHandle<T, THIS>(0);
               auto dst = clonedCoalescedSrc.template GetRawAs<DT, Any>();
               auto src = source->GetRaw();
               const auto srcEnd = src + mCount;
               while (src != srcEnd) {
                  SemanticNew(dst, Clone(**src));
                  handle.Create(dst, clonedCoalescedSrc.mEntry);

                  ++dst;
                  ++src;
                  ++handle;
               }

               const_cast<Allocation*>(clonedCoalescedSrc.mEntry)
                  ->Keep(mCount - 1);
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
            auto lhs = mthis->GetRaw();
            auto rhs = other->GetRaw();
            if constexpr (REVERSE)
               MoveMemory(lhs, rhs, mCount);
            else
               CopyMemory(lhs, rhs, mCount);
         }
         else {
            // Both RHS and LHS are dense and non POD                   
            // Call constructor for each element (optionally in reverse)
            auto lhs = mthis->GetRaw();
            auto rhs = other->GetRaw();
            if constexpr (REVERSE) {
               lhs += mCount - 1;
               rhs += mCount - 1;
            }
            const auto lhsEnd = REVERSE ? lhs - mCount : lhs + mCount;
            while (lhs != lhsEnd) {
               if constexpr (CT::Abandoned<SS> and not CT::AbandonMakable<T>) {
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
                  else LANGULUS_ERROR("T is not movable, nor abandon-constructible");
               }
               else SemanticNew(lhs, SS::Nest(*rhs));

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
      else {
         // Containers are type-erased                                  
         // First make sure that reflected constructors are available   
         // There's no point in iterating anything otherwise            
         if constexpr (SS::Move) {
            if constexpr (SS::Keep) {
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
         else if constexpr (SS::Shallow) {
            if constexpr (SS::Keep) {
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
            LANGULUS_ASSERT(mType->mCloneConstructor, Construct,
               "Can't clone-construct elements"
               " - no clone-constructor was reflected");
         }

         if (mType->mIsSparse) {
            // Both LHS and RHS are sparse                              
            if constexpr (SS::Shallow) {
               // Shallow pointer transfer                              
               ShallowBatchPointerConstruction(source.Forward());
            }
            else if (not mType->mDeptr->mIsSparse
            and (mType->mIsUnallocatable or not mType->mCloneConstructor)) {
               // We early-return with an enforced shallow pointer      
               // transfer, because its requesting to clone             
               // unallocatable/unclonable/abstract data, such as metas 
               ShallowBatchPointerConstruction(Copy(source));
            }
            else if (mType->mDeptr->mIsSparse or not mType->mResolver) {
               // If contained type is not resolvable (or is just       
               // another level of indirection), we can coalesce all    
               // clones into a single allocation                       
               Block clonedCoalescedSrc {mType->mDeptr};
               clonedCoalescedSrc.AllocateFresh<Any>(
                  clonedCoalescedSrc.RequestSize<Any>(mCount));
               clonedCoalescedSrc.mCount = mCount;

               // Clone each inner element by nesting this call         
               auto lhs = mthis->template GetHandle<Byte*, THIS>(0);
               const auto lhsEnd = lhs + mCount;
               auto dst = clonedCoalescedSrc.GetElementInner();
               auto src = source->GetElementInner();
               while (lhs.mValue != lhsEnd.mValue) {
                  dst.CreateSemantic(Clone(src.template GetDense<1>()));
                  lhs.Create(dst.mRaw, clonedCoalescedSrc.mEntry);
                  ++dst;
                  ++src;
                  ++lhs;
               }

               const_cast<Allocation*>(clonedCoalescedSrc.mEntry)
                  ->Keep(mCount - 1);
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
            const auto bytesize = mType->mSize * mCount;
            if constexpr (REVERSE)
               MoveMemory(mRaw, source->mRaw, bytesize);
            else
               CopyMemory(mRaw, source->mRaw, bytesize);
         }
         else {
            // Both RHS and LHS are dense and non-POD                   
            // We invoke reflected constructors for each element        
            const auto stride = mType->mSize;
            auto lhs = mRaw + (REVERSE ? (mCount - 1) * stride : 0);
            auto rhs = source->mRaw + (REVERSE ? (mCount - 1) * stride : 0);
            const auto rhsEnd = REVERSE ? rhs - mCount * stride : rhs + mCount * stride;

            while (rhs != rhsEnd) {
               if constexpr (SS::Move) {
                  if constexpr (SS::Keep)
                     mType->mMoveConstructor(rhs, lhs);
                  else
                     mType->mAbandonConstructor(rhs, lhs);
               }
               else if constexpr (SS::Shallow) {
                  if constexpr (SS::Keep)
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
   }
   
   /// Call a single semantic constructor by using a Handle as a source       
   ///   @attention never modifies any block state                            
   ///   @attention assumes none of the elements are constructed, despite     
   ///      having a valid mCount                                             
   ///   @attention assumes blocks types are similar                          
   ///   @param source - the handle to initialize with                        
   template<CT::Block THIS, template<class> class S, CT::Handle OTHER>
   requires CT::Semantic<S<OTHER>>
   void Block::CreateSemantic(S<OTHER>&& source) const {
      static_assert(CT::Sparse<TypeOf<OTHER>>,
         "Handle isn't sparse");
      LANGULUS_ASSUME(DevAssumes, mCount and mCount <= mReserved,
         "Count outside limits", '(', mCount, " > ", mReserved);
      LANGULUS_ASSUME(DevAssumes, mCount == 1,
         "Count mismatch");
      LANGULUS_ASSUME(DevAssumes, IsSparse(),
         "Container is not sparse");

      // Type-erased pointers (void*) are acceptable, because they're   
      // required for some internal stuff, although not recommended     
      LANGULUS_ASSUME(DevAssumes, (
         CT::Similar<TypeOf<OTHER>, void*>
      or GetType<THIS>()->template IsSimilar<TypeOf<OTHER>>()),
         "Type mismatch on creation from handle", ": ", GetType<THIS>(),
         " instead of ", MetaDataOf<TypeOf<OTHER>>());

      using T = Conditional<CT::Typed<THIS>, TypeOf<THIS>, TypeOf<OTHER>>;
      auto handle = GetHandle<T, THIS>(0);
      handle.CreateSemantic(source.Forward());
   }
   
   /// Batch-optimized semantic pointer constructions                         
   ///   @attention overwrites pointers without dereferencing their memory    
   ///   @attention doesn't modify any container state                        
   ///   @param source - the source of pointers                               
   template<template<class> class S, CT::Block T> requires CT::Semantic<S<T>>
   void Block::ShallowBatchPointerConstruction(S<T>&& source) const {
      static_assert(S<T>::Shallow,
         "This function works only for shallow semantics");

      const auto mthis       = const_cast<Block*>(this);
      const auto pointersDst = mthis->template  GetRawSparse<T>();
      const auto pointersSrc = source->template GetRawSparse<T>();
      const auto entriesDst  = mthis->template  GetEntries<T>();
      const auto entriesSrc = source->mEntry
         ? source->template GetEntries<T>()
         : nullptr;

      if constexpr (S<T>::Move) {
         // Move/Abandon                                                
         MoveMemory(pointersDst, pointersSrc, mCount);

         if (entriesSrc) {
            // Transfer entries, if available                           
            MoveMemory(entriesDst, entriesSrc, mCount);
            ZeroMemory(entriesSrc, mCount);
         }
         else {
            // Otherwise make sure all entries are zero                 
            ZeroMemory(entriesDst, mCount);
         }

         // Reset source pointers, too, if not abandoned                
         if constexpr (S<T>::Keep)
            ZeroMemory(pointersSrc, mCount);
      }
      else {
         // Copy/Disown                                                 
         CopyMemory(pointersDst, pointersSrc, mCount);

         if constexpr (S<T>::Keep) {
            // Reference each entry, if not disowned                    
            if (entriesSrc) {
               CopyMemory(entriesDst, entriesSrc, mCount);
               auto entry = entriesDst;
               const auto entryEnd = entry + mCount;
               while (entry != entryEnd) {
                  if (*entry)
                     const_cast<Allocation*>(*entry)->Keep();
                  ++entry;
               }
            }
            else {
               // Otherwise make sure all entries are zero              
               ZeroMemory(entriesDst, mCount);
            }
         }
         else {
            // Otherwise make sure all entries are zero                 
            ZeroMemory(entriesDst, mCount);
         }
      }
   }
   
   /// Call semantic assignment in a region                                   
   ///   @attention never modifies any block state                            
   ///   @attention assumes blocks don't overlap (sparse elements may still   
   ///      overlap, but this is handled in the assignment operators)         
   ///   @attention assumes blocks are binary compatible                      
   ///   @attention assumes source has at least mCount items                  
   ///   @param source - the elements to assign                               
   template<CT::Block THIS, template<class> class S, CT::Block OTHER>
   requires CT::Semantic<S<OTHER>>
   void Block::AssignSemantic(S<OTHER>&& source) const {
      LANGULUS_ASSUME(DevAssumes, mCount and mCount <= mReserved,
         "Count outside limits", '(', mCount, " > ", mReserved);
      LANGULUS_ASSUME(DevAssumes, mCount <= source->mCount,
         "Count mismatch");
      // Type-erased pointers (void*) are acceptable                    
      LANGULUS_ASSUME(DevAssumes, (source->GetType()->IsSimilar(GetType<THIS>())
         or (source->GetType()->template IsSimilar<void*>() and IsSparse<THIS>())
         or (source->GetType()->mIsSparse and GetType<THIS>()->template IsSimilar<void*>())),
         "Type mismatch on assignment", ": ", source->GetType(), " != ", GetType<THIS>());

      using B = Conditional<CT::Typed<THIS>, THIS, OTHER>;
      using T = Conditional<CT::Typed<B>, TypeOf<B>, void>;
      using SS = S<B>;
      const auto mthis = reinterpret_cast<B*>(const_cast<Block*>(this));
      const auto other = reinterpret_cast<B*>(const_cast<OTHER*>(&(*source)));

      if constexpr (not CT::TypeErased<T>) {
         if constexpr (CT::Sparse<T>) {
            // We're reassigning pointers                               
            using DT = Deptr<T>;
            if constexpr (SS::Shallow) {
               // Shallow pointer transfer                              
               Destroy<THIS>();
               ShallowBatchPointerConstruction(source.Forward());
            }
            else if constexpr (CT::Unallocatable<Decay<T>> or not CT::CloneAssignable<T>) {
               // We early-return with an enforced shallow pointer      
               // transfer, because its requesting to clone             
               // unallocatable/unclonable/abstract data, such as metas 
               Destroy<THIS>();
               ShallowBatchPointerConstruction(Copy(*source));
            }
            else if constexpr (CT::Sparse<DT> or not CT::Resolvable<T>) {
               // If contained type is not resolvable, or its deptr     
               // version is still a pointer, we can coalesce all       
               // clones into a single allocation (optimization)        
               Block clonedCoalescedSrc {mType->mDeptr};
               clonedCoalescedSrc.AllocateFresh<Any>(
                  clonedCoalescedSrc.RequestSize<Any>(mCount));
               clonedCoalescedSrc.mCount = mCount;

               // Clone each inner element                              
               auto handle = GetHandle<T, THIS>(0);
               auto dst = clonedCoalescedSrc.template GetRawAs<DT, Any>();
               auto src = source->GetRaw();
               const auto srcEnd = src + mCount;
               while (src != srcEnd) {
                  SemanticNew(dst, Clone(**src));
                  handle.Assign(dst, clonedCoalescedSrc.mEntry);

                  ++dst;
                  ++src;
                  ++handle;
               }

               const_cast<Allocation*>(clonedCoalescedSrc.mEntry)
                  ->Keep(mCount - 1);
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
            auto lhs = mthis->GetRaw();
            auto rhs = other->GetRaw();
            CopyMemory(lhs, rhs, mCount);
         }
         else {
            // Both RHS and LHS are dense and non POD                   
            // Assign to each element                                   
            auto lhs = mthis->GetRaw();
            auto rhs = other->GetRaw();
            const auto lhsEnd = lhs + mCount;
            while (lhs != lhsEnd) {
               if constexpr (CT::Abandoned<SS> and not CT::AbandonAssignable<T>) {
                  if constexpr (CT::MoveAssignable<T>) {
                     // We can fallback to move-assignment, but report  
                     // a performance warning                           
                     IF_SAFE(Logger::Warning(
                        "Move used, instead of abandon - implement an "
                        "abandon-assignment for type ", NameOf<T>(),
                        " to fix this warning"
                     ));
                     SemanticAssign(*lhs, Move(*rhs));
                  }
                  else LANGULUS_ERROR("T is not movable, nor abandon-assignable");
               }
               else SemanticAssign(*lhs, SS::Nest(*rhs));

               //SemanticAssign(*lhs, S<Block>::Nest(*rhs));
               ++lhs;
               ++rhs;
            }
         }
      }
      else {
         // Containers are type-erased                                  
         // First make sure that reflected assigners are available      
         // There's no point in iterating anything otherwise            
         if constexpr (SS::Move) {
            if constexpr (SS::Keep) {
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
         else if constexpr (SS::Shallow) {
            if constexpr (SS::Keep) {
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
            LANGULUS_ASSERT(mType->mCloneAssigner, Construct,
               "Can't clone-assign elements"
               " - no clone-assigner was reflected");
         }

         if (mType->mIsSparse) {
            // Since we're overwriting pointers, we have to dereference 
            // the old ones, but conditionally reference the new ones   
            auto lhs = mthis->mRawSparse;
            const auto lhsEnd = lhs + mCount;
            auto rhs = source->mRawSparse;
            auto lhsEntry = mthis->template GetEntries<THIS>();
            auto rhsEntry = source->template GetEntries<THIS>();

            while (lhs != lhsEnd) {
               if (*lhsEntry) {
                  // Free old LHS                                       
                  if ((*lhsEntry)->GetUses() == 1) {
                     mType->mOrigin->mDestructor(*lhs);
                     Allocator::Deallocate(const_cast<Allocation*>(*lhsEntry));
                  }
                  else const_cast<Allocation*>(*lhsEntry)->Free();
               }

               if constexpr (SS::Move) {
                  // Move/Abandon RHS in LHS                            
                  *lhs = const_cast<Byte*>(*rhs);
                  *lhsEntry = *rhsEntry;
                  *rhsEntry = nullptr;

                  if constexpr (SS::Keep) {
                     // We're not abandoning RHS, make sure it's cleared
                     *rhs = nullptr;
                  }
               }
               else if constexpr (SS::Shallow) {
                  // Copy/Disown RHS in LHS                             
                  *lhs = const_cast<Byte*>(*rhs);

                  if constexpr (SS::Keep) {
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
            CopyMemory(mRaw, source->mRaw, GetBytesize<THIS>());
         }
         else {
            // Both RHS and LHS are dense and non-POD                   
            // We invoke reflected assignments for each element         
            const auto stride = mType->mSize;
            auto lhs = mRaw;
            auto rhs = source->mRaw;
            const auto rhsEnd = rhs + mCount * stride;

            while (rhs != rhsEnd) {
               if constexpr (SS::Move) {
                  if constexpr (SS::Keep)
                     mType->mMoveAssigner(rhs, lhs);
                  else
                     mType->mAbandonAssigner(rhs, lhs);
               }
               else if constexpr (SS::Shallow) {
                  if constexpr (SS::Keep)
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
   }

} // namespace Langulus::Anyness