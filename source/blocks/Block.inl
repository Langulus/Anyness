///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Block.hpp"
#include "../inner/Handle.hpp"

namespace Langulus::Anyness
{

   /// Clear the block, only zeroing its size                                 
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::ClearInner() noexcept {
      mCount = 0;
   }

   /// Reset the memory inside the block                                      
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::ResetMemory() noexcept {
      mRaw = nullptr;
      mEntry = nullptr;
      mCount = mReserved = 0;
   }
   
   /// Reset the block's state                                                
   LANGULUS(ALWAYSINLINE)
   constexpr void Block::ResetState() noexcept {
      mState = mState.mState & DataState::Typed;
      ResetType();
   }


   /// Copy-insert anything compatible at an index                            
   ///   @attention assumes offset is in the block's limits, if simple        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
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

   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::InsertAt(const T& item, INDEX idx) {
      return InsertAt<MUTABLE, WRAPPER>(Langulus::Copy(item), idx);
   }

   /// Move-insert anything compatible at an index                            
   ///   @attention assumes offset is in the block's limits when simple       
   ///   @tparam MUTABLE - is it allowed the block to deepen to incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - the item to move in                                    
   ///   @param index - the index to insert at                                
   ///   @return number of inserted elements                                  
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::InsertAt(T&& item, INDEX idx) {
      return InsertAt<MUTABLE, WRAPPER>(Langulus::Move(item), idx);
   }
   
   /// Move-insert anything compatible at an index                            
   ///   @attention assumes offset is in the block's limits when simple       
   ///   @tparam MUTABLE - is it allowed the block to deepen to incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - the item to move in                                    
   ///   @param index - the index to insert at                                
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

   /// Copy-insert anything compatible either at the start or the end         
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
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

   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::Insert(const T& item) {
      return Insert<INDEX, MUTABLE, WRAPPER>(Langulus::Copy(item));
   }

   /// Move-insert anything compatible either at the start or the end         
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param item - item to move int                                       
   ///   @return number of inserted elements                                  
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::Insert(T&& item) {
      return Insert<INDEX, MUTABLE, WRAPPER>(Langulus::Move(item));
   }
   
   /// Move-insert anything compatible either at the start or the end         
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param item - item to move int                                       
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
   ///   If none of these constructors are available, this function throws    
   ///   Except::Construct                                                    
   ///   @tparam IDX - type of indexing to use (deducible)                    
   ///   @tparam A... - argument types (deducible)                            
   ///   @param idx - the index to emplace at                                 
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return 1 if the element was emplace successfully                    
   template<CT::Index IDX, class... A>
   LANGULUS(ALWAYSINLINE)
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
      EmplaceInner(region, Forward<A>(arguments)...);
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
   ///   If none of these constructors are available, this function throws    
   ///   Except::Construct                                                    
   ///   @tparam INDEX - the index to emplace at, IndexFront or IndexBack     
   ///   @tparam A... - argument types (deducible)                            
   ///   @param arguments... - the arguments to forward to constructor        
   ///   @return 1 if the element was emplace successfully                    
   template<Index INDEX, class... A>
   LANGULUS(ALWAYSINLINE)
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
      EmplaceInner(region, Forward<A>(arguments)...);
      return 1;
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
      static_assert(CT::Sparse<T> || CT::Insertable<T>,
         "Dense type is not insertable");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "Inserting incompatible type");

      const auto count = end - start;
      if constexpr (CT::Sparse<T>) {
         if constexpr (S::Shallow) {
            // Pointer copy/move/abandon/disown                         
            ::std::memcpy(GetRawSparse() + at, start, sizeof(Pointer) * count);

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               // If we're using managed memory, we can search if each  
               // pointer is owned by us, and get its allocation entry  
               if constexpr (CT::Allocatable<Deptr<T>> && S::Keep) {
                  auto it = start;
                  auto entry = GetEntries() + at;
                  while (it != end) {
                     *entry = Inner::Allocator::Find(
                        MetaData::Of<Deptr<T>>(), it
                     );

                     if (*entry)
                        (*entry)->Keep();

                     ++it;
                     ++entry;
                  }
               }
               else ::std::memset(GetEntries() + at, 0, sizeof(Pointer) * count);
            #endif
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
            ::std::memcpy(data, start, sizeof(T) * count);
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
   LANGULUS(ALWAYSINLINE)
   void Block::InsertInner(S&& item, Offset at) {
      using T = TypeOf<S>;
      static_assert(CT::Sparse<T> || CT::Insertable<T>,
         "Dense type is not insertable");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "Inserting incompatible type");

      if constexpr (CT::Sparse<T>) {
         if constexpr (S::Shallow) {
            // Pointer copy/move/abandon/disown                         
            GetRawSparse()[at] = const_cast<Byte*>(
               reinterpret_cast<const Byte*>(item.mValue)
            );

            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               // If we're using managed memory, we can search if the   
               // pointer is owned by us, and get its allocation entry  
               if constexpr (CT::Allocatable<Deptr<T>> && S::Keep) {
                  const auto entry = Inner::Allocator::Find(
                     MetaData::Of<Deptr<T>>(), item.mValue
                  );

                  GetEntries()[at] = entry;
                  if (entry)
                     entry->Keep();
               }
               else GetEntries()[at] = nullptr;
            #endif
         }
         else {
            // Pointer clone                                            
            TODO();
         }
      }
      else {
         // Dense data insertion (moving/abandoning value)              
         static_assert(!CT::Abstract<T>,
            "Can't insert abstract item in dense block");

         using DT = Decvq<Deref<T>>;
         const auto data = GetRawAs<DT>() + at;
         SemanticNew<DT>(data, item.Forward());
      }

      ++mCount;
   }

   /// Statically optimized InsertInner, used in fold expressions             
   ///   @tparam INDEX - offset to start inserting at                         
   ///   @tparam head - first element, semantic or not (deducible)            
   ///   @tparam tail... - the rest, semantic or not (deducible)              
   template<Offset INDEX, CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(ALWAYSINLINE)
   void Block::InsertStatic(HEAD&& head, TAIL&&... tail) {
      if constexpr (CT::Semantic<HEAD>)
         InsertInner(head.Forward(), INDEX);
      else if constexpr (::std::is_rvalue_reference_v<HEAD>)
         InsertInner(Langulus::Move(head), INDEX);
      else 
         InsertInner(Langulus::Copy(head), INDEX);

      if constexpr (sizeof...(TAIL) > 0)
         InsertStatic<INDEX + 1>(Forward<TAIL>(tail)...);
   }

   /// Remove non-sequential element(s)                                       
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param items - the items to search for and remove                    
   ///   @param count - number of items inside array                          
   ///   @param index - the index to start searching from                     
   ///   @return the number of removed items                                  
   template<bool REVERSE, CT::Data T>
   LANGULUS(ALWAYSINLINE)
   Count Block::Remove(const T& item) {
      const auto found = FindKnown<REVERSE>(item);
      if (found)
         return RemoveIndex(found.GetOffset(), 1);
      return 0;
   }

   /// Merge-copy-insert array elements at index                              
   /// Each element will be pushed only if not found in block                 
   /// A bit of runtime overhead due to resolving index, if special           
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
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
            added += InsertAt<MUTABLE, WRAPPER>(Langulus::Copy(*start), offset);
            ++offset;
         }

         ++start;
      }

      return added;
   }

   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::MergeAt(const T& item, INDEX index) {
      return MergeAt<MUTABLE, WRAPPER>(Langulus::Copy(item), index);
   }

   /// Merge-move-insert array elements at index                              
   /// Element will be pushed only if not found in block                      
   /// A bit of runtime overhead due to resolving index, when special         
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - the item to move in                                    
   ///   @param index - the special index to insert at                        
   ///   @return the number of inserted elements                              
   template<bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::MergeAt(T&& item, INDEX index) {
      return MergeAt<MUTABLE, WRAPPER>(Langulus::Move(item), index);
   }
   
   /// Merge-move-insert array elements at index                              
   /// Element will be pushed only if not found in block                      
   /// A bit of runtime overhead due to resolving index, when special         
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @tparam INDEX - the type of the index (deducible)                    
   ///   @param item - the item to move in                                    
   ///   @param index - the special index to insert at                        
   ///   @return the number of inserted elements                              
   template<bool MUTABLE, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::MergeAt(S&& item, INDEX index) {
      if (!FindKnown(item.mValue))
         return InsertAt<MUTABLE, WRAPPER>(item.Forward(), index);
      return 0;
   }
   
   /// Merge-copy-insert array elements at a static index                     
   /// Each element will be pushed only if not found in block                 
   ///   @tparam INDEX - static index (either IndexFront or IndexBack)        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param start - pointer to the first item                             
   ///   @param end - pointer to the end of items                             
   ///   @return the number of inserted elements                              
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::Merge(const T* start, const T* end) {
      Count added {};
      while (start != end) {
         if (!FindKnown(*start))
            added += Insert<INDEX, MUTABLE, WRAPPER, T>(Langulus::Copy(*start));
         ++start;
      }

      return added;
   }

   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::Merge(const T& item) {
      return Merge<INDEX, MUTABLE, WRAPPER>(Langulus::Copy(item));
   }

   /// Merge-move-insert array elements at index                              
   /// Element will be pushed only if not found in block                      
   ///   @tparam INDEX - static index (either IndexFront or IndexBack)        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param item - the item to move in                                    
   ///   @return the number of inserted elements                              
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::Merge(T&& item) {
      return Merge<INDEX, MUTABLE, WRAPPER>(Langulus::Move(item));
   }
   
   /// Merge-move-insert array elements at index                              
   /// Element will be pushed only if not found in block                      
   ///   @tparam INDEX - static index (either IndexFront or IndexBack)        
   ///   @tparam MUTABLE - is it allowed the block to deepen or incorporate   
   ///                     the new insertion, if not compatible               
   ///   @tparam WRAPPER - the type to use to deepen, if MUTABLE is enabled   
   ///   @tparam T - the type to insert (deducible)                           
   ///   @param item - the item to move in                                    
   ///   @return the number of inserted elements                              
   template<Index INDEX, bool MUTABLE, CT::Data WRAPPER, CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Count Block::Merge(S&& item) {
      if (!FindKnown(item.mValue))
         return Insert<INDEX, MUTABLE, WRAPPER>(item.Forward());
      return 0;
   }

   /// Turn into another container (inner function)                           
   ///   @tparam S - semantic value (deducible)                               
   ///   @param value - semantically provided value to absorb                 
   ///   @param state - the state to absorb                                   
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   void Block::Absorb(S&& value, const DataState& state) {
      static_assert(CT::Deep<TypeOf<S>>, "S::Type must be deep");

      const auto previousType = !mType ? value.mValue.GetType() : mType;
      const auto previousState = mState;

      operator = (value.mValue);

      if constexpr (S::Keep)
         Keep();

      mState = mState + previousState + state;

      if (previousState.IsTyped()) {
         // Retain type if original package was constrained             
         SetType<true>(previousType);
      }
      else if (IsSparse()) {
         // Retain type if current package is sparse                    
         SetType<false>(previousType);
      }

      if constexpr (S::Move) {
         if constexpr (S::Keep) {
            value.mValue.ResetMemory();
            value.mValue.ResetState();
         }
         else value.mValue.mEntry = nullptr;
      }
   }

   ///                                                                        
   template<bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPushAtInner(S&& value, const DataState& state, const INDEX& index) {
      if (IsUntyped() && IsInvalid()) {
         // Mutate-insert inside untyped container                      
         SetState(mState + state);
         return InsertAt<true>(value.Forward(), index);
      }
      else if (Is<typename S::Type>()) {
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

   ///                                                                        
   template<bool ALLOW_DEEPEN, Index INDEX, CT::Data WRAPPER, CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPushInner(S&& value, const DataState& state) {
      if (IsUntyped() && IsInvalid()) {
         // Mutate-insert inside untyped container                      
         SetState(mState + state);
         return Insert<INDEX, true>(value.Forward());
      }
      else if (Is<TypeOf<S>>()) {
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

   ///                                                                        
   template<bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::Semantic S, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartConcatAt(const bool& sc, S&& value, const DataState& state, const INDEX& index) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");
      static_assert(CT::Deep<TypeOf<S>>, "S::Type must be deep");

      // If this container is compatible and concatenation is           
      // enabled, try concatenating the two containers                  
      const bool typeCompliant = IsUntyped()
         || (ALLOW_DEEPEN && value.mValue.IsDeep())
         || CanFit(value.mValue.GetType());

      if (!IsConstant() && !IsStatic() && typeCompliant && sc
         // Make sure container is or-compliant after the change        
         && !(mCount > 1 && !IsOr() && state.IsOr())) {
         if (IsUntyped()) {
            // Block insert never mutates, so make sure type            
            // is valid before insertion                                
            SetType<false>(value.mValue.GetType());
         }
         else {
            if constexpr (ALLOW_DEEPEN) {
               if (!IsDeep() && value.mValue.IsDeep())
                  Deepen<WRAPPER, false>();
            }
         }

         const auto cat = InsertBlockAt(value.Forward(), index);
         mState += state;
         return cat;
      }

      return 0;
   }

   ///                                                                        
   template<bool ALLOW_DEEPEN, Index INDEX, CT::Data WRAPPER, CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartConcat(const bool& sc, S&& value, const DataState& state) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");
      static_assert(CT::Deep<TypeOf<S>>, "S::Type must be deep");

      // If this container is compatible and concatenation is           
      // enabled, try concatenating the two containers                  
      const bool typeCompliant = IsUntyped()
         || (ALLOW_DEEPEN && value.mValue.IsDeep())
         || Is(value.mValue.GetType());

      if (!IsConstant() && !IsStatic() && typeCompliant && sc
         // Make sure container is or-compliant after the change        
         && !(mCount > 1 && !IsOr() && state.IsOr())) {
         if (IsUntyped()) {
            // Block insert never mutates, so make sure type            
            // is valid before insertion                                
            SetType<false>(value.mValue.GetType());
         }
         else {
            if constexpr (ALLOW_DEEPEN) {
               if (!IsDeep() && value.mValue.IsDeep())
                  Deepen<WRAPPER, false>();
            }
         }

         const auto cat = InsertBlock<INDEX>(value.Forward());
         mState += state;
         return cat;
      }

      return 0;
   }

   /// A copy-insert that uses the best approach to push anything inside      
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam INDEX - type of index to use                                 
   ///   @tparam WRAPPER - type of container used for deepening if enabled    
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPushAt(const T& value, INDEX index, DataState state) {
      return SmartPushAt<ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Langulus::Copy(value), index, state);
   }

   /// This is required to disambiguate calls correctly                       
   template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPushAt(T& value, INDEX index, DataState state) {
      return SmartPushAt<ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Langulus::Copy(value), index, state);
   }

   /// A move-insert that uses the best approach to push anything inside      
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam INDEX - type of index to use                                 
   ///   @tparam WRAPPER - type of container used for deepening if enabled    
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPushAt(T&& value, INDEX index, DataState state) {
      return SmartPushAt<ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Langulus::Move(value), index, state);
   }

   /// A disown-insert that uses the best approach to push anything inside    
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam INDEX - type of index to use                                 
   ///   @tparam WRAPPER - type of container used for deepening if enabled    
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
         if (!value.mValue.IsValid())
            return 0;

         const bool stateCompliant = CanFitState(value.mValue);
         if (IsEmpty() && !value.mValue.IsStatic() && stateCompliant) {
            Absorb(value.Forward(), state);
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
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam WRAPPER - type of container used for deepening if enabled    
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPush(const T& value, DataState state) {
      return SmartPush<INDEX, ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Langulus::Copy(value), state);
   }

   /// Required to disambiguate calls correctly                               
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPush(T& value, DataState state) {
      return SmartPush<INDEX, ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Langulus::Copy(value), state);
   }
   
   /// A smart move-insert uses the best approach to push anything inside     
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam INDEX - either IndexFront or IndexBack to insert there       
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam WRAPPER - type of container used for deepening if enabled    
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::SmartPush(T&& value, DataState state) {
      return SmartPush<INDEX, ALLOW_CONCAT, ALLOW_DEEPEN, WRAPPER>(
         Langulus::Move(value), state);
   }

   /// A smart disown-insert uses the best approach to push anything inside   
   /// container in order to keep hierarchy and states, but also reuse memory 
   ///   @tparam INDEX - either IndexFront or IndexBack to insert there       
   ///   @tparam ALLOW_CONCAT - whether or not concatenation is allowed       
   ///   @tparam ALLOW_DEEPEN - whether or not deepening is allowed           
   ///   @tparam T - type of data to push (deducible)                         
   ///   @tparam WRAPPER - type of container used for deepening if enabled    
   ///   @param value - the value to smart-push                               
   ///   @param index - the index at which to insert (if needed)              
   ///   @param state - a state to apply after pushing is done                
   ///   @return the number of pushed items (zero if unsuccessful)            
   template<Index INDEX, bool ALLOW_CONCAT, bool ALLOW_DEEPEN, CT::Data WRAPPER, CT::Semantic S>
   Count Block::SmartPush(S&& value, DataState state) {
      static_assert(CT::Deep<WRAPPER>, "WRAPPER must be deep");

      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         // We're inserting a deep item, so we can do various smart     
         // things before inserting, like absorbing and concatenating   
         if (!value.mValue.IsValid())
            return 0;

         const bool stateCompliant = CanFitState(value.mValue);
         if (IsEmpty() && !value.mValue.IsStatic() && stateCompliant) {
            Absorb(value.Forward(), state);
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

   /// Wrap all contained elements inside a sub-block, making this one deep   
   ///   @tparam T - the type of deep container to use                        
   ///   @tparam MOVE_STATE - whether or not to send the current orness over  
   ///   @return a reference to this container                                
   template<CT::Data T, bool MOVE_STATE>
   LANGULUS(ALWAYSINLINE)
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
      
      // Restore the state of not moved over                            
      if constexpr (!MOVE_STATE)
         mState += state;

      return Get<T>();
   }
   
   /// Remove sequential indices                                              
   ///   @param index - index                                                 
   ///   @param count - number of items to remove                             
   ///   @return the number of removed elements                               
   template<CT::Index INDEX>
   Count Block::RemoveIndex(const INDEX index, const Count count) {
      if constexpr (CT::Same<INDEX, Index>) {
         // By special indices                                          
         if (index == IndexAll) {
            const auto oldCount = mCount;
            Free();
            ResetMemory();
            ResetState();
            return oldCount;
         }

         const auto idx = Constrain(index);
         if (idx.IsSpecial())
            return 0;

         return RemoveIndex(idx.GetOffset(), count);
      }
      else {
         Offset idx;
         if constexpr (CT::Signed<INDEX>) {
            if (index < 0)
               idx = mCount - static_cast<Offset>(-index);
            else
               idx = static_cast<Offset>(index);
         }
         else idx = index;

         // By simple index (signed or not)                             
         LANGULUS_ASSUME(DevAssumes, idx + count <= mCount, "Out of range");

         if (IsConstant() || IsStatic()) {
            if (mType->mIsPOD && idx + count >= mCount) {
               // If data is POD and elements are on the back, we can   
               // get around constantness and staticness, by simply     
               // truncating the count without any reprecussions        
               const auto removed = mCount - idx;
               mCount = idx;
               return removed;
            }
            else {
               LANGULUS_ASSERT(!IsConstant(), Access,
                  "Removing from constant container");
               LANGULUS_ASSERT(!IsStatic(), Access,
                  "Removing from static container");
               return 0;
            }
         }

         // First call the destructors on the correct region            
         const auto ender = idx + count;
         const auto removed = ender - idx;
         CropInner(idx, removed).CallUnknownDestructors();

         if (ender < mCount) {
            // Fill gap by invoking abandon-constructors                
            // We're moving to the left, so no reverse is required      
            LANGULUS_ASSERT(GetUses() == 1, Move, "Moving elements in use");
            const auto tail = mCount - ender;
            CropInner(idx, 0)
               .CallUnknownSemanticConstructors(
                  tail, Abandon(CropInner(ender, tail))
               );
         }

         // Change count                                                
         mCount -= removed;
         return removed;
      }
   }

   /// Remove a raw deep index corresponding to a whole block inside          
   ///   @param index - index to remove                                       
   ///   @return 1 if removed                                                 
   template<CT::Index INDEX>
   Count Block::RemoveIndexDeep(INDEX index) {
      if constexpr (!CT::Same<INDEX, Index>) {
         if (!IsDeep())
            return 0;

         --index;

         for (Count i = 0; i != mCount; i += 1) {
            if (index == 0)
               return RemoveIndex(i);

            auto ith = As<Block*>(i);
            const auto count = ith->GetCountDeep();
            if (index <= count && ith->RemoveIndexDeep(index))
               return 1;

            index -= count;
         }

         return 0;
      }
      else TODO();
   }

   /// Wrapper for memcpy                                                     
   ///   @param from - source of data to copy                                 
   ///   @param to - [out] destination memory                                 
   ///   @param size - number of bytes to copy                                
   LANGULUS(ALWAYSINLINE)
   void Block::CopyMemory(const void* from, void* to, const Size& size) noexcept {
      ::std::memcpy(to, from, size);
   }
   
   /// Wrapper for memmove                                                    
   ///   @param from - source of data to move                                 
   ///   @param to - [out] destination memory                                 
   ///   @param size - number of bytes to move                                
   LANGULUS(ALWAYSINLINE)
   void Block::MoveMemory(const void* from, void* to, const Size& size) noexcept {
      ::std::memmove(to, from, size);
      #if LANGULUS(PARANOID)
         TODO() // zero old memory, but beware - `from` and `to` might overlap
      #endif
   }
   
   /// Wrapper for memset                                                     
   ///   @param to - [out] destination memory                                 
   ///   @param filler - the byte to fill with                                
   ///   @param size - number of bytes to move                                
   LANGULUS(ALWAYSINLINE)
   void Block::FillMemory(void* to, Byte filler, const Size& size) noexcept {
      ::std::memset(to, static_cast<int>(filler), size);
   }

   /// A helper function, that allocates and moves inner memory               
   ///   @param other - the memory we'll be inserting                         
   ///   @param index - the place we'll be inserting at                       
   ///   @param region - the newly allocated region (!mCount, only mReserved) 
   ///   @return number if inserted items in case of mutation                 
   inline void Block::AllocateRegion(const Block& other, Offset index, Block& region) {
      // Type may mutate, but never deepen                              
      Mutate<false>(other.mType);

      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + other.mCount);

      if (index < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Moving elements that are used from multiple places");

         // We need to shift elements right from the insertion point    
         // Therefore, we call move constructors in reverse, to avoid   
         // memory overlap                                              
         const auto moved = mCount - index;
         CropInner(index + other.mCount, 0)
            .template CallUnknownSemanticConstructors<true>(
               moved, Abandon(CropInner(index, moved))
            );
      }

      // Pick the region that should be overwritten with new stuff      
      region = CropInner(index, 0);
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

      if constexpr (CT::Sparse<T>) {
         // Zero pointers                                               
         ::std::memset(mRaw, 0, count * sizeof(Pointer));

         // Zero entries if managed memory is enabled                   
         IF_LANGULUS_MANAGED_MEMORY(::std::memset(
            const_cast<Block*>(this)->GetEntries(),
            0, count * sizeof(Pointer)
         ));
      }
      else if constexpr (CT::Nullifiable<T>) {
         // Zero the dense memory (optimization)                        
         ::std::memset(mRaw, 0, count * sizeof(T));
      }
      else if constexpr (CT::Defaultable<T>) {
         // Construct requested elements in place                       
         new (mRaw) T [count];
      }
      else LANGULUS_ERROR(
         "Trying to default-construct elements that are "
         "incapable of default-construction");
   }
   
   /// Call default constructors in a region and initialize memory            
   ///   @attention never modifies any block state                            
   ///   @attention assumes block has at least 'count' elements reserved      
   ///   @attention assumes memory is not initialized                         
   ///   @param count - the number of elements to initialize                  
   inline void Block::CallUnknownDefaultConstructors(Count count) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved, "Count outside limits");

      if (mType->mIsSparse) {
         // Zero pointers                                               
         ::std::memset(mRaw, 0, count * sizeof(Pointer));

         // Zero entries if managed memory is enabled                   
         IF_LANGULUS_MANAGED_MEMORY(::std::memset(
            const_cast<Block*>(this)->GetEntries(),
            0, count * sizeof(Pointer)
         ));
      } 
      else if (mType->mIsNullifiable) {
         // Zero the dense memory (optimization)                        
         ::std::memset(mRaw, 0, count * mType->mSize);
      }
      else {
         LANGULUS_ASSERT(
            mType->mDefaultConstructor != nullptr, Construct,
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

   /// Call move constructors in a region and initialize memory               
   ///   @attention never modifies any block state                            
   ///   @attention assumes T is the type of both blocks                      
   ///   @attention assumes both blocks are of same sparsity                  
   ///   @attention assumes count <= reserved elements                        
   ///   @attention assumes source contains at least 'count' items            
   ///   @tparam T - the type to move-construct                               
   ///   @tparam REVERSE - calls move constructors in reverse, to let you     
   ///                     account for potential memory overlap               
   ///   @param count - number of elements to move                            
   ///   @param source - the block of elements to move                        
   template<CT::Data T, bool REVERSE, CT::Semantic S>
   void Block::CallKnownSemanticConstructors(const Count count, S&& source) const {
      static_assert(CT::Block<TypeOf<S>>,
         "Semantic should apply to a Block");
      static_assert(CT::Sparse<T> || CT::Mutable<T>,
         "Can't move-construct in container of constant elements");

      LANGULUS_ASSUME(DevAssumes, count <= source.mValue.mCount && count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(),
         "T doesn't match LHS type");
      LANGULUS_ASSUME(DevAssumes, source.mValue.template IsExact<T>(),
         "T doesn't match RHS type");
      LANGULUS_ASSUME(DevAssumes, IsSparse() == source.mValue.IsSparse(),
         "Blocks are not of same sparsity");

      const auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Sparse<T>) {
         // Batch-optimized semantic pointer constructions              
         const auto byteSize = sizeof(Pointer) * count;
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            const auto entriesDst = mthis->GetEntries();
            const auto entriesSrc = source.mValue.GetEntries();
         #endif

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon                                 
            if constexpr (S::Move) {
               // Move/Abandon                                          
               ::std::memmove(mthis->GetRaw(), source.mValue.GetRaw(), byteSize);
               #if LANGULUS_FEATURE(MANAGED_MEMORY)
                  ::std::memmove(entriesDst, entriesSrc, byteSize);

                  // Reset source ownership                             
                  ::std::memset(entriesSrc, 0, byteSize);
               #endif

               // Reset source pointers, too, if not abandoned          
               if constexpr (S::Keep)
                  ::std::memset(source.mValue.GetRaw(), 0, byteSize);
            }
            else {
               // Copy/Disown                                           
               ::std::memcpy(mthis->GetRaw(), source.mValue.GetRaw(), byteSize);

               #if LANGULUS_FEATURE(MANAGED_MEMORY)
                  ::std::memcpy(entriesDst, entriesSrc, byteSize);

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
                     ::std::memset(entriesDst, 0, byteSize);
                  }
               #endif
            }
         }
         else {
            // Clone                                                    
            if constexpr (CT::Sparse<Deptr<T>> || !CT::Resolvable<T>) {
               using DT = Deptr<T>;

               // If contained type is not resolvable, or its deptr     
               // version is still a pointer, we can coalesce all       
               // clones into a single allocation (optimization)        
               Block clonedCoalescedSrc {mType->mDeptr};
               clonedCoalescedSrc.AllocateFresh(clonedCoalescedSrc.RequestSize(count));
               clonedCoalescedSrc.mCount = count;

               // Clone each inner element                              
               auto handle = GetHandle<T>(0);
               DT* dst = clonedCoalescedSrc.template GetRawAs<DT>();
               auto src = source.mValue.GetRaw();
               const auto srcEnd = src + count;
               while (src != srcEnd) {
                  SemanticNew<DT>(dst, Langulus::Clone(**src));

                  handle.Set(dst);
                  handle.SetEntry(clonedCoalescedSrc.mEntry);

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
         const auto bytecount = sizeof(T) * count;
         auto lhs = mthis->GetRaw();
         auto rhs = source.mValue.GetRaw();
         ::std::memcpy(lhs, rhs, bytecount);
      }
      else {
         // Both RHS and LHS are dense and non POD                      
         // Call constructor for each element (optionally in reverse)   
         auto lhs = mthis->template GetRawAs<T>();
         auto rhs = source.mValue.template GetRawAs<T>();
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
      static_assert(CT::Block<TypeOf<S>>,
         "S type must be a block type");

      LANGULUS_ASSUME(DevAssumes, count <= source.mValue.mCount && count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, mType->IsExact(source.mValue.mType),
         "LHS and RHS are different types");

      auto mthis = const_cast<Block*>(this);
      if (mType->mIsSparse && source.mValue.mType->mIsSparse) {
         // Batch-optimized semantic pointer constructions              
         const auto byteSize = sizeof(Pointer) * count;
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            const auto entriesDst = mthis->GetEntries();
            const auto entriesSrc = source.mValue.GetEntries();
         #endif

         if constexpr (S::Shallow) {
            // Copy/Disown/Move/Abandon                                 
            if constexpr (S::Move) {
               // Move/Abandon                                          
               ::std::memmove(mthis->GetRaw(), source.mValue.GetRaw(), byteSize);
               #if LANGULUS_FEATURE(MANAGED_MEMORY)
                  ::std::memmove(entriesDst, entriesSrc, byteSize);

                  // Reset source ownership                             
                  ::std::memset(entriesSrc, 0, byteSize);
               #endif

               // Reset source pointers, too, if not abandoned          
               if constexpr (S::Keep)
                  ::std::memset(source.mValue.GetRaw(), 0, byteSize);
            }
            else {
               // Copy/Disown                                           
               ::std::memcpy(mthis->GetRaw(), source.mValue.GetRaw(), byteSize);

               #if LANGULUS_FEATURE(MANAGED_MEMORY)
                  ::std::memcpy(entriesDst, entriesSrc, byteSize);

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
                     ::std::memset(entriesDst, 0, byteSize);
                  }
               #endif
            }
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
               auto handle = GetHandle<void*>(0);
               auto dst = clonedCoalescedSrc.GetElement();
               auto src = source.mValue.GetElement();
               for (Count i = 0; i < count; ++i) {
                  dst.CallUnknownSemanticConstructors(
                     1, Langulus::Clone(src.template GetDense<1>())
                  );

                  #if LANGULUS_FEATURE(MANAGED_MEMORY)
                     handle.Set(dst.mRaw);
                     handle.SetEntry(clonedCoalescedSrc.mEntry);
                  #else
                     *handle = dst.mRaw;
                  #endif

                  dst.Next();
                  src.Next();
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
      else if (mType->mIsPOD && mType->mIsSparse == source.mValue.mType->mIsSparse) {
         // Both dense and POD                                          
         // Copy/Disown/Move/Abandon/Clone                              
         if constexpr (S::Move)
            ::std::memmove(mRaw, source.mValue.mRaw, mType->mSize * count);
         else
            ::std::memcpy(mRaw, source.mValue.mRaw, mType->mSize * count);
         return;
      }

      if (mType->mIsSparse) {
         // LHS is pointer, RHS must be dense                           
         // Copy each pointer from RHS (can't move them)                
         auto lhs = mthis->GetRawSparse();
         IF_LANGULUS_MANAGED_MEMORY(auto lhsEntry = mthis->GetEntries());
         const auto lhsEnd = lhs + count;
         auto rhs = source.mValue.mRaw;
         const auto rhsStride = source.mValue.mType->mSize;
         while (lhs != lhsEnd) {
            if constexpr (S::Shallow) {
               // Shallow-copy a pointer to the dense element           
               (*lhs) = rhs;
               IF_LANGULUS_MANAGED_MEMORY((*lhsEntry) = source.mValue.mEntry);
            }
            else {
               // Deep-copy dense element and set pointer to it         
               TODO();
            }

            ++lhs;
            IF_LANGULUS_MANAGED_MEMORY(++lhsEntry);
            rhs += rhsStride;
         }

         if constexpr (S::Shallow) {
            // We have to reference RHS by the number of pointers we    
            // made. Since we're converting dense to sparse, the        
            // referencing is MANDATORY!                                
            source.mValue.mEntry->Keep(count);
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

               if (source.mValue.mType->mIsSparse) {
                  // RHS is pointer, LHS is dense                       
                  // Move each dense element from RHS                   
                  auto rhs = source.mValue.mRawSparse + count - 1;
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
                  auto rhs = source.mValue.mRaw + (count - 1) * lhsStride;
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

               if (source.mValue.mType->mIsSparse) {
                  // RHS is pointer, LHS is dense                       
                  // Move each dense element from RHS                   
                  auto rhs = source.mValue.mRawSparse;
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
                  auto rhs = source.mValue.mRaw;
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

            if (source.mValue.mType->mIsSparse) {
               // RHS is pointer, LHS is dense                          
               // Shallow-copy or clone each dense element from RHS     
               auto rhs = source.mValue.mRawSparse;
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
               auto rhs = source.mValue.mRaw;
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
   
   /// Call descriptor constructors in a region, initializing memory          
   ///   @attention never modifies any block state                            
   ///   @attention assumes T is the type of the block                        
   ///   @attention assumes this has at least 'count' items reserved          
   ///   @tparam T - type of the data to descriptor-construct                 
   ///   @param count - the number of elements to construct                   
   ///   @param descriptor - the descriptor to pass on to constructors        
   template<CT::Data T>
   void Block::CallKnownDescriptorConstructors(const Count count, const Any& descriptor) const {
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
         IF_LANGULUS_MANAGED_MEMORY(auto lhsEnt = const_cast<Block*>(this)->GetEntries());
         const auto lhsEnd = lhsPtr + count;
         const auto allocation = Inner::Allocator::Allocate(sizeof(Decay<T>) * count);
         allocation->Keep(count - 1);

         auto rhs = allocation->As<Decay<T>*>();
         while (lhsPtr != lhsEnd) {
            new (rhs) Decay<T> {descriptor};
            *(lhsPtr++) = rhs;
            IF_LANGULUS_MANAGED_MEMORY(*(lhsEnt++) = allocation);
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
   
   /// Call descriptor constructors in a region, initializing memory          
   ///   @attention never modifies any block state                            
   ///   @attention assumes this has at least 'count' items reserved          
   ///   @attention assumes that none of the elements is initialized          
   ///   @param count - the number of elements to construct                   
   ///   @param descriptor - the descriptor to pass on to constructors        
   inline void Block::CallUnknownDescriptorConstructors(Count count, const Any& descriptor) const {
      LANGULUS_ASSUME(DevAssumes, count <= mReserved,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, mType->mDescriptorConstructor != nullptr,
         "Type is not descriptor-constructible");

      if (mType->mDeptr) {
         if (!mType->mDeptr->mIsSparse) {
            // Bulk-allocate the required count, construct each instance
            // and set the pointers                                     
            auto lhsPtr = const_cast<Block*>(this)->GetRawSparse();
            IF_LANGULUS_MANAGED_MEMORY(auto lhsEnt = const_cast<Block*>(this)->GetEntries());
            const auto lhsEnd = lhsPtr + count;
            const auto allocation = Inner::Allocator::Allocate(mType->mOrigin->mSize * count);
            allocation->Keep(count - 1);

            auto rhs = allocation->GetBlockStart();
            while (lhsPtr != lhsEnd) {
               mType->mOrigin->mDescriptorConstructor(rhs, descriptor);
               *(lhsPtr++) = rhs;
               IF_LANGULUS_MANAGED_MEMORY(*(lhsEnt++) = allocation);
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
         IF_LANGULUS_MANAGED_MEMORY(
            auto lhsEntry = const_cast<Block&>(*this).GetEntries()
         );

         while (lhs != lhsEnd) {
            if constexpr (CT::Handle<A...> && CT::Same<T, TypeOf<A>...>) {
               // Set pointer and entry from handle                     
               (*lhs = ... = arguments.mPointer);

               IF_LANGULUS_MANAGED_MEMORY(
                  (*lhsEntry = ... = arguments.mEntry)
               );
            }
            else if constexpr (::std::constructible_from<T, A...>) {
               // Set pointer and find entry                            
               (*lhs = ... = arguments);

               IF_LANGULUS_MANAGED_MEMORY(
                  *lhsEntry = Inner::Allocator::Find(mType, *lhs)
               );
            }
            else LANGULUS_ERROR("T is not constructible with these arguments");

            ++lhs;
            IF_LANGULUS_MANAGED_MEMORY(++lhsEntry);
         }
      }
      else {
         // Construct dense stuff                                       
         auto lhs = const_cast<Block&>(*this).template GetRawAs<T>();
         const auto lhsEnd = lhs + count;
         while (lhs != lhsEnd) {
            if constexpr (::std::constructible_from<T, A...>)
               new (lhs++) T {arguments...};
            else
               LANGULUS_ERROR("T is not constructible with these arguments");
         }
      }
   }
   
   /// Call move-assignment in a region, initializing memory                  
   ///   @attention don't assign to overlapping memory regions!               
   ///   @attention never modifies any block state                            
   ///   @attention assumes blocks are binary compatible                      
   ///   @attention assumes both blocks have at least 'count' items           
   ///   @tparam KEEP - false to minimally reset source elements and block    
   ///   @param count - the number of elements to move-assign                 
   ///   @param source - the elements to move                                 
   template<CT::Semantic S>
   void Block::CallUnknownSemanticAssignment(const Count count, S&& source) const {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");

      LANGULUS_ASSUME(DevAssumes, mCount >= count && source.mValue.mCount >= count,
         "Count outside limits");
      LANGULUS_ASSUME(DevAssumes, mType->IsExact(source.mValue.mType),
         "LHS and RHS are different types");

      const auto mthis = const_cast<Block*>(this);
      if (mType->mIsSparse && source.mValue.mType->mIsSparse) {
         // Since we're overwriting pointers, we have to dereference    
         // the old ones, but conditionally reference the new ones      
         auto lhs = mthis->GetRawSparse();
         const auto lhsEnd = lhs + count;
         auto rhs = source.mValue.GetRawSparse();

         IF_LANGULUS_MANAGED_MEMORY(
            auto lhsEntry = mthis->GetEntries();
            auto rhsEntry = source.mValue.GetEntries();
         );

         while (lhs != lhsEnd) {
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               if (*lhsEntry) {
                  // Free old LHS                                       
                  if ((*lhsEntry)->GetUses() == 1) {
                     mType->mOrigin->mDestructor(*lhs);
                     Inner::Allocator::Deallocate(*lhsEntry);
                  }
                  else (*lhsEntry)->Free();
               }
            #endif

            if constexpr (S::Move) {
               // Move/Abandon RHS in LHS                               
               *lhs = const_cast<Byte*>(*rhs);

               IF_LANGULUS_MANAGED_MEMORY(
                  *lhsEntry = const_cast<Inner::Allocation*>(*rhsEntry);
                  *rhsEntry = nullptr;
               );

               if constexpr (S::Keep) {
                  // We're not abandoning RHS, make sure it's cleared   
                  *rhs = nullptr;
               }
            }
            else if constexpr (S::Shallow) {
               // Copy/Disown RHS in LHS                                
               *lhs = const_cast<Byte*>(*rhs);

               #if LANGULUS_FEATURE(MANAGED_MEMORY)
                  if constexpr (S::Keep) {
                     *lhsEntry = const_cast<Inner::Allocation*>(*rhsEntry);
                     if (*lhsEntry)
                        (*lhsEntry)->Keep();
                  }
                  else *lhsEntry = nullptr;
               #endif
            }
            else {
               // Clone RHS in LHS                                      
               TODO();
            }

            ++lhs;
            ++rhs;

            IF_LANGULUS_MANAGED_MEMORY(
               ++lhsEntry;
               ++rhsEntry;
            );
         }

         return;
      }
      else if (mType->mIsPOD && mType->mIsSparse == source.mValue.mType->mIsSparse) {
         if constexpr (S::Move)
            MoveMemory(source.mValue.mRaw, mRaw, mType->mSize * count);
         else
            CopyMemory(source.mValue.mRaw, mRaw, mType->mSize * count);
         return;
      }

      if (mType->mIsSparse) {
         // LHS is pointer, RHS must be dense                           
         // Move each pointer from RHS                                  
         auto lhs = mRawSparse;
         IF_LANGULUS_MANAGED_MEMORY(auto lhsEntry = mthis->GetEntries());
         const auto lhsEnd = lhs + count;
         auto rhs = source.mValue.mRaw;
         const auto rhsStride = source.mValue.mType->mSize;
         while (lhs != lhsEnd) {
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               if (*lhsEntry) {
                  // Free old LHS                                       
                  if ((*lhsEntry)->GetUses() == 1) {
                     mType->mOrigin->mDestructor(*lhs);
                     Inner::Allocator::Deallocate(*lhsEntry);
                  }
                  else (*lhsEntry)->Free();
               }
            #endif

            if constexpr (S::Move || S::Shallow) {
               // Set LHS to point to dense RHS element                 
               *lhs = const_cast<Byte*>(rhs);

               #if LANGULUS_FEATURE(MANAGED_MEMORY)
                  *lhsEntry = source.mValue.mEntry;

                  // We're converting dense to sparse, so reference     
                  if (*lhsEntry)
                     (*lhsEntry)->Keep();
               #endif
            }
            else {
               // Clone RHS and set a pointer to it in LHS              
               TODO();
            }
         
            ++lhs;
            IF_LANGULUS_MANAGED_MEMORY(++lhsEntry);
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
            if (source.mValue.mType->mIsSparse) {
               // RHS is pointer, LHS is dense                          
               // Copy each dense element from RHS                      
               auto rhs = source.mValue.mRawSparse;
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
               auto rhs = source.mValue.mRaw;
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
            if (source.mValue.mType->mIsSparse) {
               // RHS is pointer, LHS is dense                          
               // Shallow-copy each dense element from RHS              
               auto rhs = source.mValue.mRawSparse;
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
               auto rhs = source.mValue.mRaw;
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

   /// Call destructors of all initialized items                              
   ///   @attention never modifies any block state                            
   ///   @attention assumes block is of type T, or is at least virtual base   
   ///   @tparam T - the type to destroy                                      
   template<CT::Data T>
   void Block::CallKnownDestructors() const {
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Container is empty");
      LANGULUS_ASSUME(DevAssumes, 
         IsExact<T>() || mType->template HasDerivation<T>(),
         "T isn't related to contained type");

      using DT = Decay<T>;
      const auto mthis = const_cast<Block*>(this);
      constexpr bool destroy = !CT::POD<T> && CT::Destroyable<T>;
      if constexpr (CT::Sparse<T> && CT::Dense<Deptr<T>>) {
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            // We dereference each pointer - destructors will be called 
            // if data behind these pointers is fully dereferenced, too 
            auto data = mthis->GetRawSparse();
            auto dataEntry = mthis->GetEntries();
            const auto dataEnd = data + mCount;
            while (data != dataEnd) {
               auto entry = *dataEntry;
               if (entry) {
                  if (entry->GetUses() == 1) {
                     if (destroy)
                        reinterpret_cast<T>(*data)->~DT();
                     Inner::Allocator::Deallocate(entry);
                  }
                  else entry->Free();
               }

               ++data;
               ++dataEntry;
            }
         #endif
      }
      else if constexpr (CT::Sparse<T>) {
         // Destroy each indirection layer                              
         TODO();
      }
      else if constexpr (destroy) {
         // Destroy every dense element                                 
         auto data = mthis->template GetRawAs<T>();
         const auto dataEnd = data + mCount;
         while (data != dataEnd)
            (data++)->~DT();
      }

      // Always nullify upon destruction only if we're paranoid         
      PARANOIA(::std::memset(GetRaw(), 0, GetByteSize()));
   }
   
   /// Call destructors of all initialized items                              
   ///   @attention never modifies any block state                            
   ///   @attention assumes there's at least one valid element                
   inline void Block::CallUnknownDestructors() const {
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Container is empty");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Container has no type");

      const bool destroy = !mType->mIsPOD && mType->mDestructor;
      if (mType->mIsSparse && !mType->mDeptr->mIsSparse) {
         // We dereference each pointer - destructors will be called    
         // if data behind these pointers is fully dereferenced, too    
         #if LANGULUS_FEATURE(MANAGED_MEMORY)
            const auto mthis = const_cast<Block*>(this);
            auto data = mthis->GetRawSparse();
            auto dataEntry = mthis->GetEntries();
            const auto dataEnd = data + mCount;
            if (destroy) {
               while (data != dataEnd) {
                  auto entry = *dataEntry;
                  if (entry) {
                     if (entry->GetUses() == 1) {
                        mType->mDeptr->mDestructor(*data);
                        Inner::Allocator::Deallocate(entry);
                     }
                     else entry->Free();
                  }

                  ++data;
                  ++dataEntry;
               }
            }
            else {
               while (data != dataEnd) {
                  auto entry = *dataEntry;
                  if (entry) {
                     if (entry->GetUses() == 1)
                        Inner::Allocator::Deallocate(entry);
                     else
                        entry->Free();
                  }

                  ++data;
                  ++dataEntry;
               }
            }
         #endif
      }
      else if (mType->mIsSparse) {
         // Destroy each indirection layer                              
         TODO();
      }
      else if (destroy) {
         // Destroy every dense element, one by one, using the          
         // reflected destructors (if any)                              
         auto data = mRaw;
         const auto dataStride = mType->mSize;
         const auto dataEnd = data + mCount * dataStride;
         while (data != dataEnd) {
            mType->mDestructor(data);
            data += dataStride;
         }
      }

      // Always nullify upon destruction only if we're paranoid         
      PARANOIA(::std::memset(GetRaw(), 0, GetByteSize()));
   }

   /// Copy-insert all elements of a block at an index                        
   ///   @attention assumes that index is inside block's limits               
   ///   @param other - the block to insert                                   
   ///   @param index - index to insert them at                               
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::InsertBlockAt(const T& other, INDEX idx) {
      return InsertBlockAt(Langulus::Copy(other), idx);
   }

   /// Move-insert all elements of a block at an index                        
   ///   @param other - the block to move in                                  
   ///   @param index - index to insert them at                               
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::InsertBlockAt(T&& other, INDEX idx) {
      return InsertBlockAt(Langulus::Move(other), idx);
   }

   /// Move-insert all elements of an abandoned block at an index             
   ///   @param other - the block to move in                                  
   ///   @param idx - index to insert them at                                 
   ///   @return the number of inserted elements                              
   template<CT::Semantic S, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::InsertBlockAt(S&& other, INDEX idx) {
      using T = TypeOf<S>;
      static_assert(CT::Block<T>, "T must be a block type");
      if (other.mValue.IsEmpty())
         return 0;

      Block region;
      AllocateRegion(other.mValue, SimplifyIndex<T>(idx), region);
      if (region.IsAllocated()) {
         region.CallUnknownSemanticConstructors(
            other.mValue.mCount, other.Forward());
         mCount += other.mValue.mCount;
         return other.mValue.mCount;
      }

      return 0;
   }

   /// Copy-insert all elements of a block either at the start or at end      
   ///   @tparam INDEX - either IndexBack or IndexFront                       
   ///   @tparam T - type of the block to traverse (deducible)                
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::InsertBlock(const T& other) {
      return InsertBlock<INDEX>(Langulus::Copy(other));
   }

   /// Move-insert all elements of a block either at the start or at end      
   ///   @tparam INDEX - either IndexBack or IndexFront                       
   ///   @tparam T - type of the block to traverse (deducible)                
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::InsertBlock(T&& other) {
      return InsertBlock<INDEX>(Langulus::Move(other));
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

      if (other.mValue.IsEmpty())
         return 0;

      // Type may mutate, but never deepen                              
      Mutate<false>(other.mValue.mType);

      // Allocate the required memory - this will not initialize it     
      AllocateMore<false>(mCount + other.mValue.mCount);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Move,
            "Inserting requires moving elements, "
            "that are used from multiple location");

         // We're moving to the right to form the gap, so we have to    
         // call abandon-constructors in reverse to avoid overlap       
         CropInner(other.mValue.mCount, 0)
            .template CallUnknownSemanticConstructors<true>(
               mCount, Abandon(CropInner(0, mCount))
            );

         CropInner(0, 0)
            .CallUnknownSemanticConstructors(
               other.mValue.mCount, other.template Forward<Block>());
      }
      else {
         CropInner(mCount, 0)
            .CallUnknownSemanticConstructors(
               other.mValue.mCount, other.template Forward<Block>());
      }

      mCount += other.mValue.mCount;

      if constexpr (S::Move && S::Keep && T::Ownership) {
         // All elements were moved, only empty husks remain            
         // so destroy them, and discard ownership of 'other'           
         const auto pushed = other.mValue.mCount;
         other.mValue.Free();
         other.mValue.mEntry = nullptr;
         return pushed;
      }
      else return other.mValue.mCount;
   }
   
   /// Copy-insert each block element that is not found in this container     
   /// One by one, by using a slow and tedious RTTI copies and compares       
   ///   @attention assumes simple index is in container's limits             
   ///   @param other - the block to insert                                   
   ///   @param index - special/simple index to insert at                     
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::MergeBlockAt(const T& other, INDEX index) {
      return MergeBlockAt(Langulus::Copy(other), index);
   }

   /// Move-insert each block element that is not found in this container     
   /// One by one, by using a slow and tedious RTTI copies and compares       
   /// The moved elements will be removed from the source container           
   ///   @attention assumes simple index is in container's limits             
   ///   @param other - the block to insert                                   
   ///   @param index - special/simple index to insert at                     
   ///   @return the number of inserted elements                              
   template<CT::NotSemantic T, CT::Index INDEX>
   LANGULUS(ALWAYSINLINE)
   Count Block::MergeBlockAt(T&& other, INDEX index) {
      return MergeBlockAt(Langulus::Move(other), index);
   }

   /// Copy-insert each block element that is not found in this container     
   /// One by one, by using a slow and tedious RTTI copies and compares       
   ///   @attention assumes simple index is in container's limits             
   ///   @param other - the block to insert                                   
   ///   @param index - special/simple index to insert at                     
   ///   @return the number of inserted elements                              
   template<CT::Semantic S, CT::Index INDEX>
   Count Block::MergeBlockAt(S&& other, INDEX index) {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");
      static_assert(CT::SameAsOneOf<INDEX, Index, Offset>,
         "INDEX must be an index type");

      //TODO do a pass first and allocate & move once instead of each time?
      Count inserted {};
      for (Count i = 0; i < other.mValue.GetCount(); ++i) {
         auto right = other.mValue.GetElementResolved(i);
         if (!FindUnknown(right))
            inserted += InsertBlockAt(S::Nest(right), index);
      }

      return inserted;
   }
   
   /// Copy-insert each block element that is not found in this container     
   /// One by one, by using a slow and tedious RTTI copies and compares       
   /// Insertions will be appended either at the front, or at the back        
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::MergeBlock(const T& other) {
      return MergeBlock<INDEX>(Langulus::Copy(other));
   }

   /// Move-insert each block element that is not found in this container     
   /// One by one, by using a slow and tedious RTTI copies and compares       
   /// The moved elements will be removed from the source container           
   /// Insertions will be appended either at the front, or at the back        
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Count Block::MergeBlock(T&& other) {
      return MergeBlock<INDEX>(Langulus::Move(other));
   }

   /// Copy-insert each block element that is not found in this container     
   /// One by one, by using a slow and tedious RTTI copies and compares       
   /// Insertions will be appended either at the front, or at the back        
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   template<Index INDEX, CT::Semantic S>
   Count Block::MergeBlock(S&& other) {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");
      static_assert(INDEX == IndexFront || INDEX == IndexBack,
         "INDEX must be either IndexFront or IndexBack");

      //TODO do a pass first and allocate & move once instead of each time?
      Count inserted {};
      for (Count i = 0; i < other.mValue.GetCount(); ++i) {
         auto right = other.mValue.GetElementResolved(i);
         if (!FindUnknown(right))
            inserted += InsertBlock<INDEX>(S::Nest(right));
      }

      return inserted;
   }

} // namespace Langulus::Anyness

