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
   
   /// Create N new elements, using default construction                      
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @return the number of new elements                                   
   template<CT::Block THIS> LANGULUS(INLINED)
   Count Block::New(const Count count) {
      AllocateMore<THIS, false>(mCount + count);
      CropInner(mCount, 0).CallDefaultConstructors<THIS>(count);
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
      Offset idx;

      if constexpr (MOVE_ASIDE) {
         AllocateMore<THIS>(mCount + count);
         idx = SimplifyIndex<T>(index);

         if (idx < mCount) {
            // Move memory if required                                  
            LANGULUS_ASSERT(GetUses() == 1, Move,
               "Moving elements that are used from multiple places");

            // We're moving to the right, so make sure we do it in      
            // reverse to avoid any potential overlap                   
            const auto moved = mCount - idx;
            CropInner(idx + count, moved)
               .template CallSemanticConstructors<THIS, true>(
                  moved, Abandon(CropInner(idx, moved))
               );
         }
      }
      else idx = SimplifyIndex<T>(index);

      // Construct data in place                                        
      CropInner(idx, count).template CallSemanticConstructors<THIS>(
         count, Forward<S<Block>>(data));
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

         using T = Conditional<CT::Typed<THIS>, TypeOf<THIS>, void>;
         Offset idx;

         if constexpr (MOVE_ASIDE) {
            AllocateMore<THIS>(mCount + 1);
            idx = SimplifyIndex<T>(index);

            if (idx < mCount) {
               // Move memory if required                               
               LANGULUS_ASSERT(GetUses() == 1, Move,
                  "Moving elements that are used from multiple places");

               // We're moving to the right, so make sure we do it in   
               // reverse to avoid any potential overlap                
               const auto moved = mCount - idx;
               CropInner(idx + 1, moved)
                  .CallSemanticConstructors<THIS, true>(
                     moved, Abandon(CropInner(idx, moved))
                  );
            }
         }
         else idx = SimplifyIndex<T>(index);

         CropInner(idx, 1).CallDescriptorConstructors<THIS>(1, *item);
      }
      else {
         using T = Conditional<CT::Typed<THIS>, TypeOf<THIS>, TypeOf<S>>;
         static_assert(CT::Inner::Insertable<T>, "T is not insertable");

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
            // Type can't mutate, but we still have to check it         
            LANGULUS_ASSERT(me.template IsSimilar<T>(), Meta,
               "Inserting incompatible type `", MetaDataOf<T>(),
               "` to container of type `", me.GetType(), '`'
            );
         }

         Offset idx;

         // If reached, we have compatible type, so allocate            
         if constexpr (MOVE_ASIDE) {
            AllocateMore<THIS>(mCount + 1);
            idx = SimplifyIndex<T>(index);

            if (idx < mCount) {
               // Move memory if required                               
               LANGULUS_ASSERT(GetUses() == 1, Move,
                  "Moving elements that are used from multiple places");

               // We're moving to the right, so make sure we do it in   
               // reverse to avoid any potential overlap                
               const auto moved = mCount - idx;
               CropInner(idx + 1, moved)
                  .CallSemanticConstructors<THIS, true>(
                     moved, Abandon(CropInner(idx, moved))
                  );
            }
         }
         else idx = SimplifyIndex<T>(index);

         GetHandle<T>(idx).New(S::Nest(item));
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
            InsertContiguousInner<THIS, FORCE, MOVE_ASIDE, Deext<T>>(
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
            if (CT::Void<FORCE> and not IsSimilar<Text>())
               return 0;

            Text text {S::Nest(item)};
            if (not IsSimilar<Text>() or not Find<THIS>(text)) {
               InsertInner<THIS, FORCE, MOVE_ASIDE>(
                  index, Abandon(text));
               return 1;
            }
         }
         else {
            // Insert the array                                         
            using DT = Deext<T>;
            if (CT::Void<FORCE> and not IsSimilar<DT>())
               return 0;

            const auto data = Block::From(item);
            if (not IsSimilar<DT>() or not FindBlock<THIS>(data)) {
               InsertContiguousInner<THIS, FORCE, MOVE_ASIDE, DT>(
                  index, S::Nest(data));
               return ExtentOf<T>;
            }
         }
      }
      else {
         // Some of the arguments might still be used directly to       
         // make an element, forward these to standard insertion here   
         if (CT::Void<FORCE> and not IsSimilar<T>())
            return 0;

         if (not IsSimilar<T>() or not Find<THIS>(DesemCast(item))) {
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
   ///   @param index - the index at which to insert                          
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

   /// Semantically insert each element that is not found in this container   
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - special/simple index to insert at                     
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<CT::Block THIS, class FORCE, bool MOVE_ASIDE, class T>
   requires CT::Block<Desem<T>> LANGULUS(INLINED)
   Count Block::MergeBlock(CT::Index auto index, T&& other) {
      using S = SemanticOf<decltype(other)>;
      decltype(auto) rhs = DesemCast(other);

      Count inserted = 0;
      for (Count i = 0; i < rhs.GetCount(); ++i) {
         auto element = rhs.GetElement(i);
         if (not FindUnknown(element)) {
            inserted += InsertBlock<THIS, FORCE, MOVE_ASIDE>(
               index, S::Nest(element));
         }
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
   template<CT::Block THIS, bool MOVE_ASIDE, class... A> LANGULUS(INLINED)
   Count Block::Emplace(CT::Index auto idx, A&&... arguments) {
      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         if constexpr (not ::std::constructible_from<T, A...>)
            LANGULUS_ERROR("T is not constructible with the given arguments");

         const auto offset = SimplifyIndex<T>(idx);

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
               CropInner(offset + 1, 0)
                  .template CallKnownSemanticConstructors<T, true>(
                     tail, Abandon(CropInner(offset, tail))
                  );
            }
         }

         CropInner(offset, 0)
            .template CallKnownConstructors<T, A...>(
               1, Forward<A>(arguments)...);

         ++mCount;
      }
      else {
         const auto offset = SimplifyIndex<void>(idx);

         if constexpr (MOVE_ASIDE) {
            AllocateMore<THIS>(mCount + 1);

            if (offset < mCount) {
               // Move memory if required                               
               LANGULUS_ASSERT(GetUses() == 1, Move,
                  "Moving elements that are used from multiple places");

               // We're moving to the right, so make sure we do it in   
               // reverse to avoid any overlap                          
               const auto moved = mCount - offset;
               CropInner(offset + 1, 0)
                  .template CallUnknownSemanticConstructors<true>(
                     moved, Abandon(CropInner(offset, moved))
                  );
            }
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
      AllocateMore<THIS>(mCount + count);

      // Pick the region that should be overwritten with new stuff      
      const auto region = CropInner(mCount, 0);
      EmplaceInner<THIS>(region, count, Forward<A>(arguments)...);
      return count;
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
      else if (IsExact<T>()) {
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
   ///   @tparam THIS - the type of the resulting container                   
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

      THIS result;
      result.template AllocateFresh<THIS>(
         result.template RequestSize<THIS>(mCount + rhs->GetCount()));
      result.template InsertBlock<THIS, void, false>(
         0, Copy(lhs));
      result.template InsertBlock<THIS, void, false>(
         mCount, rhs.Forward());
      return Abandon(result);
   }

   /// Call default constructors in a region and initialize memory            
   ///   @attention never modifies any block state                            
   ///   @attention assumes block has at least 'count' elements reserved      
   ///   @attention assumes memory is not initialized                         
   ///   @param count - the number of elements to initialize                  
   template<CT::Block THIS>
   void Block::CallDefaultConstructors(const Count count) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved, "Count outside limits");
      auto mthis = const_cast<Block*>(this);

      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;

         if constexpr (CT::Sparse<T>) {
            // Zero pointers and entries                                
            ZeroMemory(mthis->mRawSparse, count);
            ZeroMemory(mthis->GetEntries<THIS>(), count);
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
      else {
         if (mType->mIsSparse) {
            // Zero pointers and entries                                
            ZeroMemory(mRawSparse, count);
            ZeroMemory(mthis->GetEntries<THIS>(), count);
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
   }
   
   /// Call descriptor constructors in a region, initializing memory          
   ///   @attention never modifies any block state                            
   ///   @attention assumes this has at least 'count' items reserved          
   ///   @attention assumes that none of the elements is initialized          
   ///   @param count - the number of elements to construct                   
   ///   @param desc - the descriptor to pass on to constructors              
   template<CT::Block THIS>
   void Block::CallDescriptorConstructors(const Count count, Describe&& desc) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved,
         "Count outside limits", '(', count, " > ", mReserved);
      auto mthis = const_cast<Block*>(this);

      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         static_assert(CT::DescriptorMakable<T>,
            "T is not descriptor-constructible");

         if constexpr (CT::Sparse<T>) {
            // Bulk-allocate the required count, construct each instance
            // and push the pointers                                    
            auto lhsPtr = mthis->GetRawSparse<THIS>();
            auto lhsEnt = mthis->GetEntries<THIS>();
            const auto lhsEnd = lhsPtr + count;
            const auto allocation = Allocator::Allocate(
               MetaDataOf<Decay<T>>(),
               sizeof(Decay<T>) * count
            );
            allocation->Keep(count - 1);

            auto rhs = allocation->template As<Decay<T>*>();
            while (lhsPtr != lhsEnd) {
               new (rhs) Decay<T> {Forward<Describe>(desc)};
               *(lhsPtr++) = rhs;
               *(lhsEnt++) = allocation;
               ++rhs;
            }
         }
         else {
            // Construct all dense elements in place                    
            auto lhs = mthis.template GetRawAs<T>();
            const auto lhsEnd = lhs + count;
            while (lhs != lhsEnd) {
               new (lhs++) Decay<T> {Forward<Describe>(desc)};
            }
         }
      }
      else {
         LANGULUS_ASSERT(
            mType->mDescriptorConstructor, Construct,
            "Can't descriptor-construct ", '`', mType,
            "` - no descriptor-constructor reflected"
         );

         if (mType->mDeptr) {
            if (not mType->mDeptr->mIsSparse) {
               // Bulk-allocate the required count, construct each      
               // instance and set the pointers                         
               auto lhsPtr = mthis->GetRawSparse<THIS>();
               auto lhsEnt = mthis->GetEntries<THIS>();
               const auto lhsEnd = lhsPtr + count;
               const auto allocation = Allocator::Allocate(
                  mType->mOrigin,
                  mType->mOrigin->mSize * count
               );
               allocation->Keep(count - 1);

               auto rhs = allocation->GetBlockStart();
               while (lhsPtr != lhsEnd) {
                  mType->mOrigin->mDescriptorConstructor(rhs, *desc);
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
               mType->mDescriptorConstructor(lhs, *desc);
               lhs += mType->mSize;
            }
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
   template<CT::Block THIS, class...A> requires CT::Typed<THIS>
   void Block::CallConstructors(const Count count, A&&...arguments) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved, "Count outside limits");
      using T = TypeOf<THIS>;
      auto mthis = const_cast<Block*>(this);

      if constexpr (sizeof...(A) == 0) {
         // No arguments, just fallback to default construction         
         CallDefaultConstructors<THIS>(count);
      }
      else if constexpr (CT::Sparse<T>) {
         static_assert(sizeof...(A) == 1, "Bad argument");
         using AA = FirstOf<A...>;

         // Construct pointers                                          
         auto lhs = mthis->GetRawAs<T>();
         const auto lhsEnd = lhs + count;
         auto lhsEntry = mthis->GetEntries<THIS>();

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
         auto lhs = mthis->GetRawAs<T>();
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
   template<CT::Block THIS, bool REVERSE, template<class> class S>
   requires CT::Semantic<S<Block>>
   void Block::CallSemanticConstructors(
      const Count count, S<Block>&& source
   ) const {
      LANGULUS_ASSUME(DevAssumes, count <= source->mCount and count <= mReserved,
         "Count outside limits");

      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         LANGULUS_ASSUME(DevAssumes, source->template IsSimilar<T>(),
            "T doesn't match RHS type",
            ": ", source->GetType(), " != ", MetaDataOf<T>());

         const auto mthis = const_cast<Block*>(this);
         if constexpr (CT::Sparse<T>) {
            using DT = Deptr<T>;

            if constexpr (S<Block>::Shallow) {
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
               clonedCoalescedSrc.AllocateFresh<Any>(
                  clonedCoalescedSrc.RequestSize<Any>(count));
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
               if constexpr (CT::Abandoned<S<Block>> and not CT::AbandonMakable<T>) {
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
               else SemanticNew(lhs, S<Block>::Nest(*rhs));

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
         LANGULUS_ASSUME(DevAssumes, mType->IsSimilar(source->mType),
            "LHS and RHS are different types");

         // First make sure that reflected constructors are available   
         // There's no point in iterating anything otherwise            
         if constexpr (S<Block>::Move) {
            if constexpr (S<Block>::Keep) {
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
         else if constexpr (S<Block>::Shallow) {
            if constexpr (S<Block>::Keep) {
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
            if constexpr (S<Block>::Shallow) {
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
               clonedCoalescedSrc.AllocateFresh<Any>(
                  clonedCoalescedSrc.RequestSize<Any>(count));
               clonedCoalescedSrc.mCount = count;

               // Clone each inner element by nesting this call         
               auto lhs = mthis->template GetHandle<Byte*>(0);
               const auto lhsEnd = lhs.mValue + count;
               auto dst = clonedCoalescedSrc.GetElement();
               auto src = source->GetElement();
               while (lhs != lhsEnd) {
                  dst.CallSemanticConstructors<THIS>(
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
               if constexpr (S<Block>::Move) {
                  if constexpr (S<Block>::Keep)
                     mType->mMoveConstructor(rhs, lhs);
                  else
                     mType->mAbandonConstructor(rhs, lhs);
               }
               else if constexpr (S<Block>::Shallow) {
                  if constexpr (S<Block>::Keep)
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
   
   /// Batch-optimized semantic pointer constructions                         
   ///   @attention overwrites pointers without dereferencing their memory    
   ///   @param count - number of elements to construct                       
   ///   @param source - source                                               
   template<template<class> class S> requires CT::Semantic<S<Block>>
   void Block::ShallowBatchPointerConstruction(
      const Count count, S<Block>&& source
   ) const {
      const auto mthis = const_cast<Block*>(this);
      const auto pointersDst = mthis->GetRawSparse<Block>();
      const auto pointersSrc = source->template GetRawSparse<Block>();
      const auto entriesDst = mthis->GetEntries<Block>();
      const auto entriesSrc = source->template GetEntries<Block>();

      if constexpr (S<Block>::Move) {
         // Move/Abandon                                                
         MoveMemory(pointersDst, pointersSrc, count);
         MoveMemory(entriesDst, entriesSrc, count);

         // Reset source ownership                                      
         ZeroMemory(entriesSrc, count);

         // Reset source pointers, too, if not abandoned                
         if constexpr (S<Block>::Keep)
            ZeroMemory(pointersSrc, count);
      }
      else {
         // Copy/Disown                                                 
         CopyMemory(pointersDst, pointersSrc, count);
         CopyMemory(entriesDst, entriesSrc, count);

         if constexpr (S<Block>::Keep) {
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
   template<CT::Block THIS, template<class> class S> requires CT::Semantic<S<Block>>
   void Block::CallSemanticAssigners(
      const Count count, S<Block>&& source
   ) const {
      LANGULUS_ASSUME(DevAssumes, count <= source->mCount and count <= mReserved,
         "Count outside limits");
      const auto mthis = const_cast<Block*>(this);

      if constexpr (CT::Typed<THIS>) {
         using T = TypeOf<THIS>;
         static_assert(CT::Mutable<T>,
            "Can't assign to container filled with constant items");

         LANGULUS_ASSUME(DevAssumes, source->template IsExact<T>(),
            "T doesn't match RHS type",
            ": ", source->GetType(), " != ", MetaDataOf<T>());

         if constexpr (CT::Sparse<T>) {
            // We're reassigning pointers                               
            using DT = Deptr<T>;

            if constexpr (S<Block>::Shallow) {
               // Shallow pointer transfer                              
               CallDestructors<THIS>();
               ShallowBatchPointerConstruction(count, source.Forward());
            }
            else if constexpr (CT::Unallocatable<T> or not CT::CloneAssignable<T>) {
               // We early-return with an enforced shallow pointer      
               // transfer, because its requesting to clone             
               // unallocatable/unclonable/abstract data, such as metas 
               CallDestructors<THIS>();
               ShallowBatchPointerConstruction(count, Copy(*source));
            }
            else if constexpr (CT::Sparse<DT> or not CT::Resolvable<T>) {
               // If contained type is not resolvable, or its deptr     
               // version is still a pointer, we can coalesce all       
               // clones into a single allocation (optimization)        
               Block clonedCoalescedSrc {mType->mDeptr};
               clonedCoalescedSrc.AllocateFresh<Any>(
                  clonedCoalescedSrc.RequestSize<Any>(count));
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
               SemanticAssign(lhs, S<Block>::Nest(*rhs));
               ++lhs;
               ++rhs;
            }
         }
      }
      else {
         LANGULUS_ASSUME(DevAssumes, mType->IsExact(source->mType),
            "LHS and RHS are different types");

         // First make sure that reflected assigners are available      
         // There's no point in iterating anything otherwise            
         if constexpr (S<Block>::Move) {
            if constexpr (S<Block>::Keep) {
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
         else if constexpr (S<Block>::Shallow) {
            if constexpr (S<Block>::Keep) {
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

         if (mType->mIsSparse) {
            // Since we're overwriting pointers, we have to dereference 
            // the old ones, but conditionally reference the new ones   
            auto lhs = mthis->mRawSparse;
            const auto lhsEnd = lhs + count;
            auto rhs = source->mRawSparse;
            auto lhsEntry = mthis->GetEntries<THIS>();
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

               if constexpr (S<Block>::Move) {
                  // Move/Abandon RHS in LHS                            
                  *lhs = const_cast<Byte*>(*rhs);
                  *lhsEntry = *rhsEntry;
                  *rhsEntry = nullptr;

                  if constexpr (S<Block>::Keep) {
                     // We're not abandoning RHS, make sure it's cleared
                     *rhs = nullptr;
                  }
               }
               else if constexpr (S<Block>::Shallow) {
                  // Copy/Disown RHS in LHS                             
                  *lhs = const_cast<Byte*>(*rhs);

                  if constexpr (S<Block>::Keep) {
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
               if constexpr (S<Block>::Move) {
                  if constexpr (S<Block>::Keep)
                     mType->mMoveAssigner(rhs, lhs);
                  else
                     mType->mAbandonAssigner(rhs, lhs);
               }
               else if constexpr (S<Block>::Shallow) {
                  if constexpr (S<Block>::Keep)
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