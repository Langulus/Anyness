///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block-Construct.inl"
#include "Block-Indexing.inl"
#include "Block-Capsulation.inl"
#include "../../Neat.inl"


namespace Langulus::Anyness
{
   
   /// Insert a range of elements by shallow-copy                             
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param start - pointer to the first item                             
   ///   @param end - pointer to the end of items                             
   ///   @param idx - the index to insert at                                  
   ///   @return number of inserted elements                                  
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   Count Block::InsertAt(const T* start, const T* end, INDEX idx) {
      static_assert(CT::Deep<WRAPPER>,
         "WRAPPER must be deep");
      static_assert(CT::Sparse<T> || CT::Mutable<T>,
         "Can't copy-insert into container of constant elements");

      const auto index = SimplifyIndex<T>(idx);

      if constexpr (MUTABLE) {
         // Type may mutate                                             
         if (Mutate<T, true, WRAPPER>()) {
            WRAPPER temp;
            temp.template SetType<T, false>();
            temp.template Insert<IndexBack, false>(start, end);
            return InsertAt<false>(Abandon(temp), index);
         }
      }

      // Allocate                                                       
      const auto count = end - start;
      AllocateMore<false>(mCount + count);

      if (index < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any potential overlap                              
         const auto moved = mCount - index;
         CropInner(index + count, moved)
            .template CallKnownSemanticConstructors<T, true>(
               moved, Abandon(CropInner(index, moved))
            );
      }

      InsertInner<Copied<T>>(start, end, index);
      return count;
   }

   /// Insert a single element by shallow-copy                                
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - item to insert                                         
   ///   @param idx - the index to insert at                                  
   ///   @return number of inserted elements                                  
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::InsertAt(const T& item, INDEX idx) {
      return InsertAt<MUTABLE, WRAPPER>(Copy(item), idx);
   }

   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::InsertAt(T& item, INDEX idx) {
      return InsertAt<MUTABLE, WRAPPER>(Copy(item), idx);
   }

   /// Insert a single element by move                                        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - the item to move in                                    
   ///   @param idx - the index to insert at                                  
   ///   @return number of inserted elements                                  
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::InsertAt(T&& item, INDEX idx) {
      return InsertAt<MUTABLE, WRAPPER>(Move(item), idx);
   }
   
   /// Insert a single element by a semantic                                  
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam S - the type and semantic to use (deducible)                 
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - the item to move in                                    
   ///   @param idx - the index to insert at                                  
   ///   @return number of inserted elements                                  
   template<bool MUTABLE, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   Count Block::InsertAt(S&& item, INDEX idx) {
      using T = TypeOf<S>;

      static_assert(CT::Deep<WRAPPER>,
         "WRAPPER must be deep");
      static_assert(CT::Sparse<T> || CT::Mutable<T>,
         "Can't move-insert into container of constant elements");

      const auto index = SimplifyIndex<T>(idx);

      if constexpr (MUTABLE) {
         // Type may mutate                                             
         if (Mutate<T, true, WRAPPER>()) {
            return InsertAt<false>(
               Abandon(WRAPPER {item.Forward()}), index);
         }
      }

      // Allocate                                                       
      AllocateMore<false>(mCount + 1);

      if (index < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any potential overlap                              
         const auto moved = mCount - index;
         CropInner(index + 1, moved)
            .template CallKnownSemanticConstructors<T, true>(
               moved, Abandon(CropInner(index, moved))
            );
      }

      InsertInner(item.Forward(), index);
      return 1;
   }
   
   /// Insert a range of elements by shallow-copy at a static index           
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param start - pointer to the first item                             
   ///   @param end - pointer to the end of items                             
   ///   @return number of inserted elements                                  
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   Count Block::Insert(const T* start, const T* end) {
      static_assert(CT::Deep<WRAPPER>,
         "WRAPPER must be deep");
      static_assert(INDEX == IndexFront || INDEX == IndexBack,
         "INDEX can be either IndexBack or IndexFront; "
         "use Block::InsertAt to insert at specific offset");

      if constexpr (MUTABLE) {
         // Type may mutate                                             
         if (Mutate<T, true, WRAPPER>()) {
            WRAPPER temp;
            temp.template SetType<T, false>();
            temp.template Insert<IndexBack, false>(start, end);
            return Insert<INDEX, false>(Abandon(temp));
         }
      }

      // Allocate                                                       
      const auto count = end - start;
      AllocateMore<false>(mCount + count);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any overlap                                        
         CropInner(count, 0)
            .template CallKnownSemanticConstructors<T, true>(
               mCount, Abandon(CropInner(0, mCount))
            );

         InsertInner<Copied<T>>(start, end, 0);
      }
      else InsertInner<Copied<T>>(start, end, mCount);

      return count;
   }

   /// Insert a single element by shallow-copy at a static index              
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - item to insert                                         
   ///   @return number of inserted elements                                  
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::Insert(const T& item) {
      return Insert<INDEX, MUTABLE, WRAPPER>(Copy(item));
   }

   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::Insert(T& item) {
      return Insert<INDEX, MUTABLE, WRAPPER>(Copy(item));
   }

   /// Insert a single element by move at a static index                      
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - item to insert                                         
   ///   @return number of inserted elements                                  
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::Insert(T&& item) {
      return Insert<INDEX, MUTABLE, WRAPPER>(Move(item));
   }
   
   /// Insert a single element by semantic at a static index                  
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam S - the type and semantic to use (deducible)                 
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - item to insert                                         
   ///   @return number of inserted elements                                  
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::Semantic S>
   Count Block::Insert(S&& item) {
      using T = TypeOf<S>;

      static_assert(CT::Deep<WRAPPER>,
         "WRAPPER must be deep");
      static_assert(INDEX == IndexFront || INDEX == IndexBack,
         "INDEX can be either IndexBack or IndexFront; "
         "use Block::InsertAt to insert at specific offset");

      if constexpr (MUTABLE) {
         // Type may mutate                                             
         if (Mutate<T, true, WRAPPER>()) {
            return Insert<INDEX, false>(Abandon(WRAPPER {item.Forward()}));
         }
      }

      // Allocate                                                       
      AllocateMore<false>(mCount + 1);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any potential overlap                              
         CropInner(1, 0)
            .template CallKnownSemanticConstructors<T, true>(
               mCount, Abandon(CropInner(0, mCount))
            );

         InsertInner(item.Forward(), 0);
      }
      else InsertInner(item.Forward(), mCount);

      return 1;
   }
   
   /// Merge a range of elements by shallow-copy                              
   /// Each element will be pushed only if not found in block                 
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param start - pointer to the first item                             
   ///   @param end - pointer to the end of items                             
   ///   @param index - the special index to insert at                        
   ///   @return the number of inserted elements                              
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   Count Block::MergeAt(const T* start, const T* end, INDEX index) {
      auto offset = SimplifyIndex(index);
      Count added {};
      while (start != end) {
         if (!FindKnown(*start)) {
            added += InsertAt<MUTABLE, WRAPPER>(Copy(*start), offset);
            ++offset;
         }

         ++start;
      }

      return added;
   }

   /// Merge a single element by shallow-copy                                 
   /// Element will be pushed only if not found in block                      
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - item to insert                                         
   ///   @param index - the special index to insert at                        
   ///   @return the number of inserted elements                              
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::MergeAt(const T& item, INDEX index) {
      return MergeAt<MUTABLE, WRAPPER>(Copy(item), index);
   }

   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::MergeAt(T& item, INDEX index) {
      return MergeAt<MUTABLE, WRAPPER>(Copy(item), index);
   }

   /// Merge a single element by move                                         
   /// Element will be pushed only if not found in block                      
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - item to insert                                         
   ///   @param index - the special index to insert at                        
   ///   @return the number of inserted elements                              
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::MergeAt(T&& item, INDEX index) {
      return MergeAt<MUTABLE, WRAPPER>(Move(item), index);
   }
   
   /// Merge a single element by a semantic                                   
   /// Element will be pushed only if not found in block                      
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam S - the type and semantic to use (deducible)                 
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - item to insert                                         
   ///   @param index - the special index to insert at                        
   ///   @return the number of inserted elements                              
   template<bool MUTABLE, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::MergeAt(S&& item, INDEX index) {
      if (!FindKnown(item.mValue))
         return InsertAt<MUTABLE, WRAPPER>(item.Forward(), index);
      return 0;
   }

   /// Merge a range of elements by shallow-copy at a static index            
   /// Each element will be pushed only if not found in block                 
   ///   @tparam INDEX - static index (either IndexFront or IndexBack)        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param start - pointer to the first item                             
   ///   @param end - pointer to the end of items                             
   ///   @return the number of inserted elements                              
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::Merge(const T* start, const T* end) {
      Count added {};
      while (start != end) {
         if (!FindKnown(*start))
            added += Insert<INDEX, MUTABLE, WRAPPER, T>(Copy(*start));
         ++start;
      }

      return added;
   }

   /// Merge an elements by shallow-copy at a static index                    
   /// Each element will be pushed only if not found in block                 
   ///   @tparam INDEX - static index (either IndexFront or IndexBack)        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param item - item to insert                                         
   ///   @return the number of inserted elements                              
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::Merge(const T& item) {
      return Merge<INDEX, MUTABLE, WRAPPER>(Copy(item));
   }

   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::Merge(T& item) {
      return Merge<INDEX, MUTABLE, WRAPPER>(Copy(item));
   }

   /// Merge an elements by move at a static index                            
   /// Each element will be pushed only if not found in block                 
   ///   @tparam INDEX - static index (either IndexFront or IndexBack)        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param item - item to insert                                         
   ///   @return the number of inserted elements                              
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::Merge(T&& item) {
      return Merge<INDEX, MUTABLE, WRAPPER>(Move(item));
   }
   
   /// Merge an elements by semantic at a static index                        
   /// Each element will be pushed only if not found in block                 
   ///   @tparam INDEX - static index (either IndexFront or IndexBack)        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible?              
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam S - the type and semantic to use (deducible)                 
   ///   @param item - item to insert                                         
   ///   @return the number of inserted elements                              
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::Semantic S>
   LANGULUS(INLINED)
   Count Block::Merge(S&& item) {
      if (!FindKnown(*item))
         return Insert<INDEX, MUTABLE, WRAPPER>(item.Forward());
      return 0;
   }
   
   /// Copy-insert all elements of a block at an index                        
   ///   @tparam T - type of block (deducible)                                
   ///   @tparam INDEX - type of indexing (deducible)                         
   ///   @param other - the block to insert                                   
   ///   @param idx - index to insert them at                                 
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::InsertBlockAt(const T& other, INDEX idx) {
      return InsertBlockAt(Copy(other), idx);
   }

   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::InsertBlockAt(T& other, INDEX idx) {
      return InsertBlockAt(Copy(other), idx);
   }

   /// Move-insert all elements of a block at an index                        
   ///   @tparam T - type of block (deducible)                                
   ///   @tparam INDEX - type of indexing (deducible)                         
   ///   @param other - the block to insert                                   
   ///   @param idx - index to insert them at                                 
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::InsertBlockAt(T&& other, INDEX idx) {
      return InsertBlockAt(Move(other), idx);
   }

   /// Semantically insert all elements of a block at an index                
   ///   @tparam S - type and semantic to use (deducible)                     
   ///   @tparam INDEX - type of indexing (deducible)                         
   ///   @param other - the block to insert                                   
   ///   @param idx - index to insert them at                                 
   ///   @return the number of inserted elements                              
   template<CT::Semantic S, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::InsertBlockAt(S&& other, INDEX idx) {
      using T = TypeOf<S>;
      static_assert(CT::Block<T>, "T must be a block type");
      if (other->IsEmpty())
         return 0;

      Block region;
      AllocateRegion(*other, SimplifyIndex<T>(idx), region);
      if (region.IsAllocated()) {
         if constexpr (CT::Typed<T>) {
            region.CallKnownSemanticConstructors<TypeOf<T>>(
               other->mCount, other.template Forward<Block>()
            );
         }
         else {
            region.CallUnknownSemanticConstructors(
               other->mCount, other.template Forward<Block>()
            );
         }

         mCount += other->mCount;
         return other->mCount;
      }

      return 0;
   }
   
   /// Copy-insert all elements of a block either at the start or at end      
   ///   @tparam INDEX - either IndexBack or IndexFront                       
   ///   @tparam T - type of the block to traverse (deducible)                
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::InsertBlock(const T& other) {
      return InsertBlock<INDEX>(Copy(other));
   }

   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::InsertBlock(T& other) {
      return InsertBlock<INDEX>(Copy(other));
   }

   /// Move-insert all elements of a block either at the start or at end      
   ///   @tparam INDEX - either IndexBack or IndexFront                       
   ///   @tparam T - type of the block to traverse (deducible)                
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::InsertBlock(T&& other) {
      return InsertBlock<INDEX>(Move(other));
   }

   /// Semantic-insert all elements of a block either at start or end         
   ///   @tparam INDEX - either IndexBack or IndexFront                       
   ///   @tparam S - semantic to use for the copy (deducible)                 
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::Semantic S>
   Count Block::InsertBlock(S&& other) {
      using T = TypeOf<S>;

      static_assert(CT::Block<T>,
         "S::Type must be a block type");
      static_assert(INDEX == IndexFront || INDEX == IndexBack,
         "INDEX must be either IndexFront or IndexEnd;"
         " use InsertBlockAt for specific indices");

      if (other->IsEmpty())
         return 0;

      // Type may mutate, but never deepen                              
      Mutate<false>(other->mType);

      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + other->mCount);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Inserting requires moving elements, "
            "that are used from multiple location");

         // We're moving to the right to form the gap, so we have to    
         // call abandon-constructors in reverse to avoid overlap       
         CropInner(other->mCount, 0)
            .template CallUnknownSemanticConstructors<true>(
               mCount, Abandon(CropInner(0, mCount))
            );

         CropInner(0, 0)
            .CallUnknownSemanticConstructors(
               other->mCount, other.template Forward<Block>());
      }
      else {
         CropInner(mCount, 0)
            .CallUnknownSemanticConstructors(
               other->mCount, other.template Forward<Block>());
      }

      mCount += other->mCount;

      if constexpr (S::Move && S::Keep && T::Ownership) {
         // All elements were moved, only empty husks remain            
         // so destroy them, and discard ownership of 'other'           
         const auto pushed = other->mCount;
         other->Free();
         other->mEntry = nullptr;
         return pushed;
      }
      else return other->mCount;
   }
   
   /// Copy-insert each block element that is not found in this container     
   ///   @tparam T - the block type to merge (deducible)                      
   ///   @tparam INDEX - the type of indexing (deducible)                     
   ///   @param other - the block to insert                                   
   ///   @param index - special/simple index to insert at                     
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::MergeBlockAt(const T& other, INDEX index) {
      return MergeBlockAt(Copy(other), index);
   }

   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::MergeBlockAt(T& other, INDEX index) {
      return MergeBlockAt(Copy(other), index);
   }

   /// Move-insert each block element that is not found in this container     
   ///   @tparam T - the block type to merge (deducible)                      
   ///   @tparam INDEX - the type of indexing (deducible)                     
   ///   @param other - the block to insert                                   
   ///   @param index - special/simple index to insert at                     
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::MergeBlockAt(T&& other, INDEX index) {
      return MergeBlockAt(Move(other), index);
   }

   /// Semantically insert each element that is not found in this container   
   ///   @tparam S - the block type and semantic to use (deducible)           
   ///   @tparam INDEX - the type of indexing (deducible)                     
   ///   @param other - the block to insert                                   
   ///   @param index - special/simple index to insert at                     
   ///   @return the number of inserted elements                              
   template<CT::Semantic S, CT::Index INDEX>
   Count Block::MergeBlockAt(S&& other, INDEX index) {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");
      static_assert(CT::SameAsOneOf<INDEX, Index, Offset>,
         "INDEX must be an index type");

      Count inserted {};
      for (Count i = 0; i < other->GetCount(); ++i) {
         auto right = other->GetElement(i);
         if (!FindUnknown(right))
            inserted += InsertBlockAt(S::Nest(right), index);
      }

      return inserted;
   }

   /// Copy-insert each block element that is not found in this container     
   /// at a static index                                                      
   ///   @tparam INDEX - the index to insert at (IndexFront or IndexBack)     
   ///   @tparam T - the type of block to merge with (deducible)              
   ///   @param other - the block to merge                                    
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::MergeBlock(const T& other) {
      return MergeBlock<INDEX>(Copy(other));
   }

   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::MergeBlock(T& other) {
      return MergeBlock<INDEX>(Copy(other));
   }

   /// Move-insert each block element that is not found in this container     
   /// at a static index                                                      
   ///   @tparam INDEX - the index to insert at (IndexFront or IndexBack)     
   ///   @tparam T - the type of block to merge with (deducible)              
   ///   @param other - the block to merge                                    
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::MergeBlock(T&& other) {
      return MergeBlock<INDEX>(Move(other));
   }

   /// Semantically insert each block element that is not found in this       
   /// container at a static index                                            
   ///   @tparam INDEX - the index to insert at (IndexFront or IndexBack)     
   ///   @tparam S - the type and semantic to use (deducible)                 
   ///   @param other - the block to merge                                    
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::Semantic S>
   Count Block::MergeBlock(S&& other) {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");
      static_assert(INDEX == IndexFront || INDEX == IndexBack,
         "INDEX must be either IndexFront or IndexBack");

      //TODO do a pass first and allocate & move once instead of each time?
      Count inserted {};
      for (Count i = 0; i < other->GetCount(); ++i) {
         auto right = other->GetElementResolved(i);
         if (!FindUnknown(right))
            inserted += InsertBlock<INDEX>(S::Nest(right));
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
   template<CT::Index IDX, class... A>
   LANGULUS(INLINED)
   Count Block::EmplaceAt(const IDX& idx, A&&... arguments) {
      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + 1);

      const auto index = SimplifyIndex<void, false>(idx);
      if (index < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We need to shift elements right from the insertion point    
         // Therefore, we call move constructors in reverse, to avoid   
         // memory overlap                                              
         const auto moved = mCount - index;
         CropInner(index + 1, 0)
            .template CallUnknownSemanticConstructors<true>(
               moved, Abandon(CropInner(index, moved))
            );
      }

      // Pick the region that should be overwritten with new stuff      
      const auto region = CropInner(index, 0);
      EmplaceInner(region, 1, Forward<A>(arguments)...);
      return 1;
   }

   /// Construct an item of this container's type at front/back               
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
   ///   @tparam INDEX - the index to emplace at, IndexFront or IndexBack     
   ///   @tparam A... - argument types (deducible)                            
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return 1 if the element was emplaced successfully                   
   template<Index INDEX, class... A>
   LANGULUS(INLINED)
   Count Block::Emplace(A&&... arguments) {
      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + 1);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We need to shift elements right from the insertion point    
         // Therefore, we call move constructors in reverse, to avoid   
         // potential memory overlap                                    
         CropInner(1, 0)
            .template CallUnknownSemanticConstructors<true>(
               mCount, Abandon(CropInner(0, mCount))
            );
      }

      // Pick the region that should be overwritten with new stuff      
      const auto region = CropInner(INDEX == IndexFront ? 0 : mCount, 0);
      EmplaceInner(region, 1, Forward<A>(arguments)...);
      return 1;
   }
   
   /// Create N new elements, using the provided arguments for construction   
   /// Elements will be added to the back of the container                    
   /// If none of the constructors are available, or block is not typed,      
   /// this function throws Except::Allocate                                  
   ///   @tparam ...A - arguments for construction (deducible)                
   ///   @param count - number of elements to construct                       
   ///   @param ...arguments - constructor arguments                          
   ///   @return the number of new elements                                   
   template<class... A>
   LANGULUS(INLINED)
   Count Block::New(Count count, A&&... arguments) {
      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + count);

      // Pick the region that should be overwritten with new stuff      
      const auto region = CropInner(mCount, 0);
      EmplaceInner(region, count, Forward<A>(arguments)...);
      return count;
   }
   
   /// Wrap all contained elements inside a sub-block, making this one deep   
   ///   @tparam T - the type of deep container to use                        
   ///   @tparam MOVE_STATE - whether or not to send the current orness over  
   ///   @return a reference to this container                                
   template<CT::Data T, bool MOVE_STATE>
   LANGULUS(INLINED)
   T& Block::Deepen() {
      static_assert(CT::Deep<T>, "T must be deep");

      LANGULUS_ASSERT(!IsTypeConstrained() || Is<T>(),
         Mutate, "Incompatible type");

      // Back up the state so that we can restore it if not moved over  
      UNUSED() const DataState state {mState.mState & DataState::Or};
      if constexpr (!MOVE_STATE)
         mState -= state;

      // Allocate a new T and move this inside it                       
      Block wrapper;
      wrapper.template SetType<T, false>();
      wrapper.template AllocateMore<true>(1);
      wrapper.template Get<Block>() = ::std::move(*this);
      *this = wrapper;
      
      // Restore the state if not moved over                            
      if constexpr (!MOVE_STATE)
         mState += state;

      return Get<T>();
   }
   
   /// A copy-insert that uses the best approach to push anything inside      
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam WRAPPER - type of container used for deepening or transfer   
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam INDEX - type of index to use                                 
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::SmartPushAt(const T& value, INDEX index, DataState state) {
      return SmartPushAt<ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Copy(value), index, state);
   }

   /// This is required to disambiguate calls correctly                       
   /// It's the same as the above                                             
   template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::SmartPushAt(T& value, INDEX index, DataState state) {
      return SmartPushAt<ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Copy(value), index, state);
   }

   /// A move-insert that uses the best approach to push anything inside      
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam WRAPPER - type of container used for deepening or transfer   
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam INDEX - type of index to use                                 
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::SmartPushAt(T&& value, INDEX index, DataState state) {
      return SmartPushAt<ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Move(value), index, state);
   }

   /// Semantic-insert that uses the best approach to push anything inside    
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam WRAPPER - type of container used for deepening or transfer   
   ///   @tparam S - type of data and semantic to push (deducible)            
   ///   @tparam INDEX - type of index to use                                 
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   Count Block::SmartPushAt(S&& value, INDEX index, DataState state) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");

      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         // We're inserting a deep item, so we can do various smart     
         // things before inserting, like absorbing and concatenating   
         if (!value->IsValid())
            return 0;

         const bool stateCompliant = CanFitState(*value);
         if (IsEmpty() && !value->IsStatic() && stateCompliant) {
            Free();
            BlockTransfer<WRAPPER>(value.Forward());
            return 1;
         }

         if constexpr (ALLOW_CONCAT) {
            const auto done = SmartConcatAt<ALLOW_DEEPEN, WRAPPER>(
               stateCompliant, value.Forward(), state, index);

            if (done)
               return done;
         }
      }

      return SmartPushAtInner<ALLOW_DEEPEN, WRAPPER>(
         value.Forward(), state, index);
   }
   
   /// A smart copy-insert uses the best approach to push anything inside     
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam INDEX - either IndexFront or IndexBack to insert there       
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam WRAPPER - type of container used for deepening or transfer   
   ///   @tparam T - type of data to push (deducible)                         
   ///   @param value - the value to smart-push                               
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::SmartPush(const T& value, DataState state) {
      return SmartPush<INDEX, ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Copy(value), state);
   }

   /// Required to disambiguate calls correctly                               
   /// It's the same as the above                                             
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::SmartPush(T& value, DataState state) {
      return SmartPush<INDEX, ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Copy(value), state);
   }
   
   /// A smart move-insert uses the best approach to push anything inside     
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam INDEX - either IndexFront or IndexBack to insert there       
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam WRAPPER - type of container used for deepening or transfer   
   ///   @tparam T - type of data to push (deducible)                         
   ///   @param value - the value to smart-push                               
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(INLINED)
   Count Block::SmartPush(T&& value, DataState state) {
      return SmartPush<INDEX, ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Move(value), state);
   }

   /// Semantic-insert that uses the best approach to push anything inside    
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam INDEX - either IndexFront or IndexBack to insert there       
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam WRAPPER - type of container used for deepening or transfer   
   ///   @tparam S - type of data and semantic to push (deducible)            
   ///   @param value - the value to smart-push                               
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::Semantic S>
   Count Block::SmartPush(S&& value, DataState state) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");
      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         // We're inserting a deep item, so we can do various smart     
         // things before inserting, like absorbing and concatenating   
         if (!value->IsValid())
            return 0;

         const bool stateCompliant = CanFitState(*value);
         if (IsEmpty() && !value->IsStatic() && stateCompliant) {
            Free();
            BlockTransfer<WRAPPER>(value.Forward());
            return 1;
         }

         if constexpr (ALLOW_CONCAT) {
            const auto done = SmartConcat<ALLOW_DEEPEN, INDEX, WRAPPER>(
               stateCompliant, value.Forward(), state);

            if (done)
               return done;
         }
      }

      return SmartPushInner<ALLOW_DEEPEN, INDEX, WRAPPER>(
         value.Forward(), state);
   }
   
   /// Inner semantic insertion function for a range                          
   ///   @attention this is an inner function and should be used with caution 
   ///   @attention assumes required free space has been prepared at offset   
   ///   @attention assumes that TypeOf<S> is this container's type           
   ///   @tparam S - the semantic to insert                                   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param start - start of range                                        
   ///   @param end - end of range                                            
   ///   @param at - the offset at which to start inserting                   
   template<CT::Semantic S, CT::NotSemantic T>
   void Block::InsertInner(const T* start, const T* end, Offset at) {
      static_assert(CT::Exact<TypeOf<S>, T>,
         "S type must be exactly T (build-time optimization)");
      static_assert(CT::Sparse<T> || CT::Insertable<T>,
         "Dense type is not insertable");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "Inserting incompatible type");

      const auto count = end - start;
      if constexpr (CT::Sparse<T>) {
         if constexpr (S::Shallow) {
            // Pointer copy/move/abandon/disown                         
            CopyMemory(GetRawAs<T>() + at, start, count);

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               // If we're using managed memory, we can search if each  
               // pointer is owned by us, and get its allocation entry  
               if constexpr (CT::Allocatable<Deptr<T>> && S::Keep) {
                  auto it = start;
                  auto entry = GetEntries() + at;
                  while (it != end) {
                     *entry = Allocator::Find(MetaData::Of<Deptr<T>>(), it);
                     if (*entry)
                        (*entry)->Keep();

                     ++it;
                     ++entry;
                  }
               }
               else
            #endif
               ZeroMemory(GetEntries() + at, count);
         }
         else {
            // Pointer clone                                            
            TODO();
         }
      }
      else {
         // Handle dense data copy/move/abandon/disown/clone            
         static_assert(!CT::Abstract<T>,
            "Can't insert abstract item in dense container");

         auto data = GetRawAs<T>() + at;
         if constexpr (CT::POD<T>) {
            // Optimized POD range insertion                            
            CopyMemory(data, start, count);
         }
         else {
            // Call semantic construction for each element in range     
            while (start != end)
               SemanticNew<T>(data++, S::Nest(*(start++)));
         }
      }

      mCount += count;
   }

   /// Inner semantic insertion function                                      
   ///   @attention this is an inner function and should be used with caution 
   ///   @attention assumes required free space has been prepared at offset   
   ///   @attention assumes that TypeOf<S> is this container's type           
   ///   @tparam S - the semantic & type to insert (deducible)                
   ///   @param item - item to insert                                         
   ///   @param at - the offset at which to insert                            
   template<CT::Semantic S>
   void Block::InsertInner(S&& item, Offset at) {
      using T = TypeOf<S>;
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "Inserting incompatible type");

      GetHandle<T>(at).New(item.Forward());
      ++mCount;
   }
   
   /// Smart concatenation inner call, used by smart push                     
   /// Attempts to either concatenate elements, or deepen and push block      
   ///   @tparam ALLOW_DEEPEN - is the block allowed to change to deep type   
   ///   @tparam WRAPPER - type of block used when deepening                  
   ///   @tparam S - the semantic and type we're concatenating (deducible)    
   ///   @tparam INDEX - the type of indexing used (deducible)                
   ///   @param sc - is this block state-compliant for insertion              
   ///   @param value - the value to concatenate                              
   ///   @param state - the state to apply after concatenation                
   ///   @param index - the place to insert at                                
   ///   @return the number of inserted elements                              
   template<bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::SmartConcatAt(const bool& sc, S&& value, const DataState& state, const INDEX& index) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");
      static_assert(CT::Deep<TypeOf<S>>, "S::Type must be deep");

      // If this container is compatible and concatenation is           
      // enabled, try concatenating the two containers                  
      const bool typeCompliant = IsUntyped()
         || (ALLOW_DEEPEN && value->IsDeep())
         || CanFit(value->GetType());

      if (!IsConstant() && !IsStatic() && typeCompliant && sc
         // Make sure container is or-compliant after the change        
         && !(mCount > 1 && !IsOr() && state.IsOr())) {
         if (IsUntyped()) {
            // Block insert never mutates, so make sure type            
            // is valid before insertion                                
            SetType<false>(value->GetType());
         }
         else {
            if constexpr (ALLOW_DEEPEN) {
               if (!IsDeep() && value->IsDeep())
                  Deepen<WRAPPER, false>();
            }
         }

         const auto cat = InsertBlockAt(value.Forward(), index);
         mState += state;
         return cat;
      }

      return 0;
   }

   /// Smart concatenation inner call, used by smart push                     
   /// Attempts to either concatenate elements, or deepen and push block      
   ///   @tparam ALLOW_DEEPEN - is the block allowed to change to deep type   
   ///   @tparam INDEX - index to insert at (either IndexBack or IndexFront)  
   ///   @tparam WRAPPER - type of block used when deepening                  
   ///   @tparam S - the semantic and type we're concatenating (deducible)    
   ///   @param sc - is this block state-compliant for insertion              
   ///   @param value - the value to concatenate                              
   ///   @param state - the state to apply after concatenation                
   ///   @return the number of inserted elements                              
   template<bool ALLOW_DEEPEN, Index INDEX, CT::Data WRAPPER, CT::Semantic S>
   LANGULUS(INLINED)
   Count Block::SmartConcat(const bool& sc, S&& value, const DataState& state) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");
      static_assert(CT::Deep<TypeOf<S>>, "S::Type must be deep");

      // If this container is compatible and concatenation is           
      // enabled, try concatenating the two containers                  
      const bool typeCompliant = IsUntyped()
         || (ALLOW_DEEPEN && value->IsDeep())
         || Is(value->GetType());

      if (!IsConstant() && !IsStatic() && typeCompliant && sc
         // Make sure container is or-compliant after the change        
         && !(mCount > 1 && !IsOr() && state.IsOr())) {
         if (IsUntyped()) {
            // Block insert never mutates, so make sure type            
            // is valid before insertion                                
            SetType<false>(value->GetType());
         }
         else {
            if constexpr (ALLOW_DEEPEN) {
               if (!IsDeep() && value->IsDeep())
                  Deepen<WRAPPER, false>();
            }
         }

         const auto cat = InsertBlock<INDEX>(value.Forward());
         mState += state;
         return cat;
      }

      return 0;
   }
   
   /// Inner smart-push function                                              
   ///   @tparam ALLOW_DEEPEN - is the block allowed to change to deep type   
   ///   @tparam WRAPPER - type of block used when deepening                  
   ///   @tparam S - the semantic and type we're concatenating (deducible)    
   ///   @tparam INDEX - the type of indexing used (deducible)                
   ///   @param value - the value to concatenate                              
   ///   @param state - the state to apply after concatenation                
   ///   @param index - the place to insert at                                
   ///   @return the number of inserted elements                              
   template<bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   LANGULUS(INLINED)
   Count Block::SmartPushAtInner(S&& value, const DataState& state, const INDEX& index) {
      if (IsUntyped() && IsInvalid()) {
         // Mutate-insert inside untyped container                      
         SetState(mState + state);
         return InsertAt<true>(value.Forward(), index);
      }
      else if (IsExact<TypeOf<S>>()) {
         // Insert to a same-typed container                            
         SetState(mState + state);
         return InsertAt<false>(value.Forward(), index);
      }
      else if (IsEmpty() && mType && !IsTypeConstrained()) {
         // If incompatibly typed but empty and not constrained, we     
         // can still reset the container and reuse it                  
         Reset();
         SetState(mState + state);
         return InsertAt<true>(value.Forward(), index);
      }
      else if (IsDeep()) {
         // If this is deep, then push value wrapped in a container     
         if (mCount > 1 && !IsOr() && state.IsOr()) {
            // If container is not or-compliant after insertion, we     
            // need	to add another layer                               
            Deepen<WRAPPER, false>();
            SetState(mState + state);
         }
         else SetState(mState + state);

         return InsertAt<false>(Abandon(WRAPPER {value.Forward()}), index);
      }

      if constexpr (ALLOW_DEEPEN) {
         // If this is reached, all else failed, but we are allowed to  
         // deepen, so do it                                            
         Deepen<WRAPPER, false>();
         SetState(mState + state);
         return InsertAt<false>(Abandon(WRAPPER {value.Forward()}), index);
      }
      else return 0;
   }

   /// Inner smart-push function at a static index                            
   ///   @tparam ALLOW_DEEPEN - is the block allowed to change to deep type   
   ///   @tparam INDEX - index to insert at (either IndexBack or IndexFront)  
   ///   @tparam WRAPPER - type of block used when deepening                  
   ///   @tparam S - the semantic and type we're concatenating (deducible)    
   ///   @param value - the value to concatenate                              
   ///   @param state - the state to apply after concatenation                
   ///   @return the number of inserted elements                              
   template<bool ALLOW_DEEPEN, Index INDEX, CT::Data WRAPPER, CT::Semantic S>
   LANGULUS(INLINED)
   Count Block::SmartPushInner(S&& value, const DataState& state) {
      if (IsUntyped() && IsInvalid()) {
         // Mutate-insert inside untyped container                      
         SetState(mState + state);
         return Insert<INDEX, true>(value.Forward());
      }
      else if (IsExact<TypeOf<S>>()) {
         // Insert to a same-typed container                            
         SetState(mState + state);
         return Insert<INDEX, false>(value.Forward());
      }
      else if (IsEmpty() && mType && !IsTypeConstrained()) {
         // If incompatibly typed but empty and not constrained, we     
         // can still reset the container and reuse it                  
         Reset();
         SetState(mState + state);
         return Insert<INDEX, true>(value.Forward());
      }
      else if (IsDeep()) {
         // If this is deep, then push value wrapped in a container     
         if (mCount > 1 && !IsOr() && state.IsOr()) {
            // If container is not or-compliant after insertion, we     
            // need to add another layer                                
            Deepen<WRAPPER, false>();
         }

         SetState(mState + state);
         return Insert<INDEX, false>(Abandon(WRAPPER {value.Forward()}));
      }

      if constexpr (ALLOW_DEEPEN) {
         // If this is reached, all else failed, but we are allowed to  
         // deepen, so do it                                            
         Deepen<WRAPPER, false>();
         SetState(mState + state);
         return Insert<INDEX, false>(Abandon(WRAPPER {value.Forward()}));
      }
      else return 0;
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
   inline void Block::CallUnknownDescriptorConstructors(Count count, const Neat& descriptor) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSERT(
         mType->mDescriptorConstructor, Construct,
         "Can't descriptor-construct elements"
         " - no descriptor-constructor reflected"
      );

      if (mType->mDeptr) {
         if (!mType->mDeptr->mIsSparse) {
            // Bulk-allocate the required count, construct each instance
            // and set the pointers                                     
            auto lhsPtr = const_cast<Block*>(this)->GetRawSparse();
            auto lhsEnt = const_cast<Block*>(this)->GetEntries();
            const auto lhsEnd = lhsPtr + count;
            const auto allocation = Allocator::Allocate(mType->mOrigin, mType->mOrigin->mSize * count);
            allocation->Keep(count - 1);

            auto rhs = allocation->GetBlockStart();
            while (lhsPtr != lhsEnd) {
               mType->mOrigin->mDescriptorConstructor(rhs, descriptor);
               *(lhsPtr++) = rhs;
               *(lhsEnt++) = allocation;
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
   void Block::CallKnownDescriptorConstructors(const Count count, const Neat& descriptor) const {
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
         const auto allocation = Allocator::Allocate(MetaData::Of<Decay<T>>(), sizeof(Decay<T>) * count);
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

         // Construct pointers                                          
         auto lhs = const_cast<Block&>(*this).template GetRawAs<T>();
         const auto lhsEnd = lhs + count;
         auto lhsEntry = const_cast<Block&>(*this).GetEntries();

         while (lhs != lhsEnd) {
            if constexpr (CT::Handle<A...> && CT::Same<T, TypeOf<A>...>) {
               // Set pointer and entry from handle                     
               (*lhs = ... = arguments.mPointer);
               (*lhsEntry = ... = arguments.mEntry);
            }
            else if constexpr (::std::constructible_from<T, A...>) {
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
   template<bool REVERSE, CT::Semantic S>
   void Block::CallUnknownSemanticConstructors(const Count count, S&& source) const {
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");
      LANGULUS_ASSUME(DevAssumes, count <= source->mCount && count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, mType->IsExact(source->mType),
         "LHS and RHS are different types");

      auto mthis = const_cast<Block*>(this);
      if (mType->mIsSparse && source->mType->mIsSparse) {
         if constexpr (S::Shallow) {
            // Shallow pointer transfer                                 
            ShallowBatchPointerConstruction(count, source.Forward());
         }
         else if (mType->mIsUnallocatable || !mType->mCloneConstructor) {
            // Shallow pointer transfer, because its requesting to      
            // clone unallocatable/unclonable data, such as meta        
            // definitions, or factory elements                         
            ShallowBatchPointerConstruction(count, Copy(*source));
         }
         else {
            // Clone                                                    
            if (mType->mDeptr->mIsSparse || mType->mResolver == nullptr) {
               // If contained type is not resolvable, or its deptr     
               // version is still a pointer, we can coalesce all       
               // clones into a single allocation (optimization)        
               Block clonedCoalescedSrc {mType->mDeptr};
               clonedCoalescedSrc.AllocateFresh(clonedCoalescedSrc.RequestSize(count));
               clonedCoalescedSrc.mCount = count;

               // Clone each inner element                              
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

               clonedCoalescedSrc.mEntry->Keep(count - 1);
            }
            else {
               // Type can be resolved to objects of varying size, so   
               // we are forced to make a separate allocation for each  
               // element                                               
               TODO();
            }
         }

         return;
      }
      else if (mType->mIsPOD && mType->mIsSparse == source->mType->mIsSparse) {
         // Both dense and POD                                          
         // Copy/Disown/Move/Abandon/Clone                              
         const auto bytesize = mType->mSize * count;
         if constexpr (S::Move)
            MoveMemory(mRaw, source->mRaw, bytesize);
         else
            CopyMemory(mRaw, source->mRaw, bytesize);
         return;
      }

      if (mType->mIsSparse) {
         // LHS is pointer, RHS must be dense                           
         // Copy each pointer from RHS (can't move them)                
         auto lhs = mthis->template GetHandle<Byte*>(0);
         const auto lhsEnd = lhs.mValue + count;
         auto rhs = source->template GetHandle<Byte>(0);
         const auto rhsStride = source->mType->mSize;
         while (lhs != lhsEnd) {
            lhs.NewUnknown(mType, S::Nest(rhs));
            ++lhs;
            rhs += rhsStride;
         }

         if constexpr (S::Shallow) {
            // We have to reference RHS by the number of pointers we    
            // made. Since we're converting dense to sparse, the        
            // referencing is MANDATORY!                                
            source->mEntry->Keep(count);
         }
      }
      else {
         // LHS is dense                                                
         if constexpr (S::Move) {
            if constexpr (S::Keep) {
               LANGULUS_ASSERT(
                  mType->mMoveConstructor != nullptr, Construct,
                  "Can't move-construct elements "
                  "- no move-constructor was reflected"
               );
            }
            else {
               LANGULUS_ASSERT(
                  mType->mAbandonConstructor != nullptr ||
                  mType->mMoveConstructor != nullptr, Construct,
                  "Can't abandon-construct elements "
                  "- no abandon-constructor was reflected"
               );
            }
         }
         else {
            if constexpr (S::Keep) {
               if constexpr (S::Shallow) {
                  LANGULUS_ASSERT(
                     mType->mCopyConstructor != nullptr, Construct,
                     "Can't copy-construct elements"
                     " - no copy-constructor was reflected");
               }
               else {
                  LANGULUS_ASSERT(
                     mType->mCloneConstructor != nullptr ||
                     mType->mCopyConstructor != nullptr, Construct,
                     "Can't clone-construct elements"
                     " - no copy/clone-constructor was reflected");
               }
            }
            else {
               LANGULUS_ASSERT(
                  mType->mDisownConstructor != nullptr ||
                  mType->mCopyConstructor != nullptr, Construct,
                  "Can't disown-construct elements"
                  " - no disown-constructor was reflected");
            }
         }

         if constexpr (S::Move) {
            // Moving construction                                      
            if constexpr (REVERSE) {
               const auto lhsStride = mType->mSize;
               auto lhs = mRaw + (count - 1) * lhsStride;

               if (source->mType->mIsSparse) {
                  // RHS is pointer, LHS is dense                       
                  // Move each dense element from RHS                   
                  auto rhs = source->mRawSparse + count - 1;
                  const auto rhsEnd = rhs - count;
                  if constexpr (S::Keep) {
                     // Move required                                   
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(*(rhs--), lhs);
                        lhs -= lhsStride;
                     }
                  }
                  else if (mType->mAbandonConstructor) {
                     // Attempt abandon                                 
                     while (rhs != rhsEnd) {
                        mType->mAbandonConstructor(*(rhs--), lhs);
                        lhs -= lhsStride;
                     }
                  }
                  else {
                     // Fallback to move if abandon not available       
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(*(rhs--), lhs);
                        lhs -= lhsStride;
                     }
                  }
               }
               else {
                  // Both RHS and LHS are dense                         
                  auto rhs = source->mRaw + (count - 1) * lhsStride;
                  const auto rhsEnd = rhs - count * lhsStride;
                  if constexpr (S::Keep) {
                     // Move required                                   
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(rhs, lhs);
                        lhs -= lhsStride;
                        rhs -= lhsStride;
                     }
                  }
                  else if (mType->mAbandonConstructor) {
                     // Attempt abandon                                 
                     while (rhs != rhsEnd) {
                        mType->mAbandonConstructor(rhs, lhs);
                        lhs -= lhsStride;
                        rhs -= lhsStride;
                     }
                  }
                  else {
                     // Fallback to move if abandon not available       
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(rhs, lhs);
                        lhs -= lhsStride;
                        rhs -= lhsStride;
                     }
                  }
               }
            }
            else {
               auto lhs = mRaw;
               const auto lhsStride = mType->mSize;

               if (source->mType->mIsSparse) {
                  // RHS is pointer, LHS is dense                       
                  // Move each dense element from RHS                   
                  auto rhs = source->mRawSparse;
                  const auto rhsEnd = rhs + count;
                  if constexpr (S::Keep) {
                     // Move required                                   
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(*(rhs++), lhs);
                        lhs += lhsStride;
                     }
                  }
                  else if (mType->mAbandonConstructor) {
                     // Attempt abandon                                 
                     while (rhs != rhsEnd) {
                        mType->mAbandonConstructor(*(rhs++), lhs);
                        lhs += lhsStride;
                     }
                  }
                  else {
                     // Fallback to move if abandon not available       
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(*(rhs++), lhs);
                        lhs += lhsStride;
                     }
                  }
               }
               else {
                  // Both RHS and LHS are dense                         
                  auto rhs = source->mRaw;
                  const auto rhsEnd = rhs + count * lhsStride;
                  if constexpr (S::Keep) {
                     // Move required                                   
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(rhs, lhs);
                        lhs += lhsStride;
                        rhs += lhsStride;
                     }
                  }
                  else if (mType->mAbandonConstructor) {
                     // Attempt abandon                                 
                     while (rhs != rhsEnd) {
                        mType->mAbandonConstructor(rhs, lhs);
                        lhs += lhsStride;
                        rhs += lhsStride;
                     }
                  }
                  else {
                     // Fallback to move if abandon not available       
                     while (rhs != rhsEnd) {
                        mType->mMoveConstructor(rhs, lhs);
                        lhs += lhsStride;
                        rhs += lhsStride;
                     }
                  }
               }
            }
         }
         else {
            // Copy construction                                        
            auto lhs = mRaw;
            const auto lhsStride = mType->mSize;

            if (source->mType->mIsSparse) {
               // RHS is pointer, LHS is dense                          
               // Shallow-copy or clone each dense element from RHS     
               auto rhs = source->mRawSparse;
               const auto rhsEnd = rhs + count;
               if constexpr (S::Keep) {
                  if constexpr (S::Shallow) {
                     // Copy required                                   
                     while (rhs != rhsEnd) {
                        mType->mCopyConstructor(*(rhs++), lhs);
                        lhs += lhsStride;
                     }
                  }
                  else if (mType->mCloneConstructor) {
                     // Attempt clone                                   
                     while (rhs != rhsEnd) {
                        mType->mCloneConstructor(*(rhs++), lhs);
                        lhs += lhsStride;
                     }
                  }
                  else {
                     // Fallback to copy if clone not available         
                     while (rhs != rhsEnd) {
                        mType->mCopyConstructor(*(rhs++), lhs);
                        lhs += lhsStride;
                     }
                  }
               }
               else if (mType->mDisownConstructor) {
                  // Attempt disown                                     
                  while (rhs != rhsEnd) {
                     mType->mDisownConstructor(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
               else {
                  // Fallback to copy if disown not available           
                  while (rhs != rhsEnd) {
                     mType->mCopyConstructor(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
            }
            else {
               // Both RHS and LHS are dense                            
               // Call the reflected copy-constructor for each element  
               auto rhs = source->mRaw;
               const auto rhsEnd = rhs + count * lhsStride;
               if constexpr (S::Keep) {
                  if constexpr (S::Shallow) {
                     // Copy required                                   
                     while (rhs != rhsEnd) {
                        mType->mCopyConstructor(rhs, lhs);
                        lhs += lhsStride;
                        rhs += lhsStride;
                     }
                  }
                  else if (mType->mCloneConstructor) {
                     // Attempt clone                                   
                     while (rhs != rhsEnd) {
                        mType->mCloneConstructor(rhs, lhs);
                        lhs += lhsStride;
                        rhs += lhsStride;
                     }
                  }
                  else {
                     // Fallback to copy if clone not available         
                     while (rhs != rhsEnd) {
                        mType->mCopyConstructor(rhs, lhs);
                        lhs += lhsStride;
                        rhs += lhsStride;
                     }
                  }
               }
               else if (mType->mDisownConstructor) {
                  // Attempt disown                                     
                  while (rhs != rhsEnd) {
                     mType->mDisownConstructor(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
               else {
                  // Fallback to copy if disown not available           
                  while (rhs != rhsEnd) {
                     mType->mCopyConstructor(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
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
   template<CT::Data T, bool REVERSE, CT::Semantic S>
   void Block::CallKnownSemanticConstructors(const Count count, S&& source) const {
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");
      static_assert(CT::Sparse<T> || CT::Mutable<T>,
         "Can't move-construct in container of constant elements");

      LANGULUS_ASSUME(DevAssumes, count <= source->mCount && count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "T doesn't match LHS type");
      LANGULUS_ASSUME(DevAssumes, source->template IsExact<T>(),
         "T doesn't match RHS type");
      LANGULUS_ASSUME(DevAssumes, IsSparse() == source->IsSparse(),
         "Blocks are not of same sparsity");

      const auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Sparse<T>) {
         using DT = Deptr<T>;

         if constexpr (S::Shallow) {
            // Shallow pointer transfer                                 
            ShallowBatchPointerConstruction(count, source.Forward());
         }
         else if constexpr (CT::Unallocatable<DT> || !CT::CloneMakable<DT>) {
            // Shallow pointer transfer, because its requesting to      
            // clone unallocatable/unclonable data, such as meta        
            // definitions, or factory elements                         
            ShallowBatchPointerConstruction(count, Copy(*source));
         }
         else {
            // Clone                                                    
            if constexpr (CT::Sparse<DT> || !CT::Resolvable<T>) {
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
                  SemanticNew<DT>(dst, Clone(**src));
                  handle.New(dst, clonedCoalescedSrc.mEntry);

                  ++dst;
                  ++src;
                  ++handle;
               }

               clonedCoalescedSrc.mEntry->Keep(count - 1);
            }
            else {
               // Type can be resolved to objects of varying size, so   
               // we are forced to make a separate allocation for each  
               // element                                               
               TODO();
            }
         }

         return;
      }
      else if constexpr (CT::POD<T>) {
         // We're constructing dense POD data                           
         auto lhs = mthis->template GetRawAs<T>();
         auto rhs = source->template GetRawAs<T>();
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
            SemanticNew<T>(lhs, S::Nest(*rhs));

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
   ///   @tparam S - the semantic and type to use                             
   ///   @param count - number of elements to construct                       
   ///   @param source - source                                               
   template<CT::Semantic S>
   void Block::ShallowBatchPointerConstruction(const Count count, S&& source) const {
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      const auto mthis = const_cast<Block*>(this);
      const auto pointersDst = mthis->GetRawSparse();
      const auto pointersSrc = source->GetRawSparse();
      const auto entriesDst = mthis->GetEntries();
      const auto entriesSrc = source->GetEntries();

      // Copy/Disown/Move/Abandon                                       
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
                  (*entry)->Keep();
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
   ///   @attention don't assign to overlapping memory regions!               
   ///   @attention never modifies any block state                            
   ///   @attention assumes blocks are binary compatible                      
   ///   @attention assumes both blocks have at least 'count' items           
   ///   @tparam S - semantic and block to use for the assignment (deducible) 
   ///   @param count - the number of elements to move-assign                 
   ///   @param source - the elements to assign                               
   template<CT::Semantic S>
   void Block::CallUnknownSemanticAssignment(const Count count, S&& source) const {
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      LANGULUS_ASSUME(DevAssumes, mCount >= count && source->mCount >= count,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, mType->IsExact(source->mType),
         "LHS and RHS are different types");

      const auto mthis = const_cast<Block*>(this);
      if (mType->mIsSparse && source->mType->mIsSparse) {
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
                  Allocator::Deallocate(*lhsEntry);
               }
               else (*lhsEntry)->Free();
            }

            if constexpr (S::Move) {
               // Move/Abandon RHS in LHS                               
               *lhs = const_cast<Byte*>(*rhs);
               *lhsEntry = const_cast<Allocation*>(*rhsEntry);
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
                  *lhsEntry = const_cast<Allocation*>(*rhsEntry);
                  if (*lhsEntry)
                     (*lhsEntry)->Keep();
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

         return;
      }
      else if (mType->mIsPOD && mType->mIsSparse == source->mType->mIsSparse) {
         const auto bytesize = mType->mSize * count;
         if constexpr (S::Move)
            MoveMemory(mRaw, source->mRaw, bytesize);
         else
            CopyMemory(mRaw, source->mRaw, bytesize);
         return;
      }

      if (mType->mIsSparse) {
         // LHS is pointer, RHS must be dense                           
         // Move each pointer from RHS                                  
         auto lhs = mRawSparse;
         auto lhsEntry = mthis->GetEntries();
         const auto lhsEnd = lhs + count;
         auto rhs = source->mRaw;
         const auto rhsStride = source->mType->mSize;
         while (lhs != lhsEnd) {
            if (*lhsEntry) {
               // Free old LHS                                       
               if ((*lhsEntry)->GetUses() == 1) {
                  mType->mOrigin->mDestructor(*lhs);
                  Allocator::Deallocate(*lhsEntry);
               }
               else (*lhsEntry)->Free();
            }

            if constexpr (S::Move || S::Shallow) {
               // Set LHS to point to dense RHS element                 
               *lhs = const_cast<Byte*>(rhs);
               *lhsEntry = source->mEntry;

               // We're converting dense to sparse, so reference     
               if (*lhsEntry)
                  (*lhsEntry)->Keep();
            }
            else {
               // Clone RHS and set a pointer to it in LHS              
               TODO();
            }
         
            ++lhs;
            ++lhsEntry;
            rhs += rhsStride;
         }
      }
      else {
         // LHS is dense                                                
         if constexpr (S::Move) {
            if constexpr (S::Keep) {
               LANGULUS_ASSERT(
                  mType->mMover != nullptr, Construct,
                  "Can't move-assign elements"
                  " - no move-assignment was reflected");
            }
            else {
               LANGULUS_ASSERT(
                  mType->mMover != nullptr ||
                  mType->mAbandonMover != nullptr, Construct,
                  "Can't abandon-assign elements"
                  " - no abandon-assignment was reflected");
            }
         }
         else {
            if constexpr (!S::Shallow) {
               LANGULUS_ASSERT(
                  mType->mCloneCopier != nullptr ||
                  mType->mCopier != nullptr, Construct,
                  "Can't clone/copy-assign elements"
                  " - no clone/copy-assignment was reflected");
            }
            else if constexpr (S::Keep) {
               LANGULUS_ASSERT(
                  mType->mCopier != nullptr, Construct,
                  "Can't copy-assign elements"
                  " - no copy-assignment was reflected");
            }
            else {
               LANGULUS_ASSERT(
                  mType->mCopier != nullptr ||
                  mType->mDisownCopier != nullptr, Construct,
                  "Can't disown-assign elements"
                  " - no disown-assignment was reflected");
            }
         }

         auto lhs = mRaw;
         const auto lhsStride = mType->mSize;

         if constexpr (S::Move) {
            // Moving/Abandoning                                        
            if (source->mType->mIsSparse) {
               // RHS is pointer, LHS is dense                          
               // Copy each dense element from RHS                      
               auto rhs = source->mRawSparse;
               const auto rhsEnd = rhs + count;
               if constexpr (S::Keep) {
                  // Move required                                      
                  while (rhs != rhsEnd) {
                     mType->mMover(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
               else if (mType->mAbandonMover) {
                  // Attempt abandon                                    
                  while (rhs != rhsEnd) {
                     mType->mAbandonMover(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
               else {
                  // Fallback to move if abandon not available          
                  while (rhs != rhsEnd) {
                     mType->mMover(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
            }
            else {
               // Both RHS and LHS are dense                            
               auto rhs = source->mRaw;
               const auto rhsEnd = rhs + count * lhsStride;
               if constexpr (S::Keep) {
                  // Move required                                      
                  while (rhs != rhsEnd) {
                     mType->mMover(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
               else if (mType->mAbandonMover) {
                  // Attempt abandon                                    
                  while (rhs != rhsEnd) {
                     mType->mAbandonMover(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
               else {
                  // Fallback to move if abandon not available          
                  while (rhs != rhsEnd) {
                     mType->mMover(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
            }
         }
         else {
            // Copying/Disowning/Cloning                                
            if (source->mType->mIsSparse) {
               // RHS is pointer, LHS is dense                          
               // Shallow-copy each dense element from RHS              
               auto rhs = source->mRawSparse;
               const auto rhsEnd = rhs + count;
               if constexpr (S::Keep) {
                  // Move required                                      
                  while (rhs != rhsEnd) {
                     mType->mCopier(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
               else if (mType->mDisownCopier) {
                  // Attempt abandon                                    
                  while (rhs != rhsEnd) {
                     mType->mDisownCopier(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
               else {
                  // Fallback to move if abandon not available          
                  while (rhs != rhsEnd) {
                     mType->mCopier(*(rhs++), lhs);
                     lhs += lhsStride;
                  }
               }
            }
            else {
               // Both RHS and LHS are dense                            
               // Call the reflected copy-constructor for each element  
               auto rhs = source->mRaw;
               const auto rhsEnd = rhs + count * lhsStride;
               if constexpr (S::Keep) {
                  // Move required                                      
                  while (rhs != rhsEnd) {
                     mType->mCopier(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
               else if (mType->mDisownCopier) {
                  // Attempt abandon                                    
                  while (rhs != rhsEnd) {
                     mType->mDisownCopier(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
               else {
                  // Fallback to move if abandon not available          
                  while (rhs != rhsEnd) {
                     mType->mCopier(rhs, lhs);
                     lhs += lhsStride;
                     rhs += lhsStride;
                  }
               }
            }
         }
      }
   }

   /// Call semantic assignment in a region                                   
   ///   @attention don't assign to overlapping memory regions!               
   ///   @attention never modifies any block state                            
   ///   @attention assumes blocks are binary compatible                      
   ///   @attention assumes both blocks have at least 'count' items           
   ///   @tparam T - the type to use for the assignment                       
   ///   @tparam S - semantic and block to use for the assignment (deducible) 
   ///   @param count - the number of elements to move-assign                 
   ///   @param source - the elements to assign                               
   template<CT::Data T, CT::Semantic S>
   void Block::CallKnownSemanticAssignment(Count, S&&) const {
      static_assert(CT::Exact<TypeOf<S>, Block>,
         "S type must be exactly Block (build-time optimization)");

      TODO();
   }

} // namespace Langulus::Anyness