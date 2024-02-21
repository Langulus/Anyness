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
#include "../../one/Handle.hpp"
#include "../../Index.inl"


namespace Langulus::Anyness
{
   
   /// Get the internal byte array with a given offset                        
   /// This is lowest level access and checks nothing                         
   ///   @attention assumes block is allocated                                
   ///   @param byteOffset - number of bytes to add                           
   ///   @return pointer to the selected raw data offset                      
   LANGULUS(INLINED) IF_UNSAFE(constexpr)
   Byte* Block::At(const Offset byteOffset) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Invalid memory");
      return mRaw + byteOffset;
   }

   LANGULUS(INLINED) IF_UNSAFE(constexpr)
   const Byte* Block::At(Offset byte_offset) const IF_UNSAFE(noexcept) {
      return const_cast<Block*>(this)->At(byte_offset);
   }

   /// Access element at a specific index                                     
   ///   @param idx - the index                                               
   ///   @return the element (or block, if THIS is type-erased)               
   template<CT::Block THIS> LANGULUS(INLINED)
   decltype(auto) Block::operator[] (CT::Index auto idx) {
      const auto index = SimplifyIndex<THIS>(idx);
      LANGULUS_ASSERT(index < mCount, Access, "Index out of range");

      if constexpr (CT::Typed<THIS>)
         return GetRaw<THIS>()[index];
      else
         return GetElement(index);
   }

   template<CT::Block THIS> LANGULUS(INLINED)
   decltype(auto) Block::operator[] (CT::Index auto idx) const {
      const auto index = SimplifyIndex<THIS>(idx);
      LANGULUS_ASSERT(index < mCount, Access, "Index out of range");

      if constexpr (CT::Typed<THIS>)
         return GetRaw<THIS>()[index];
      else
         return GetElement(index);
   }
   
   /// Get an element pointer or reference with a given index                 
   /// This is a lower-level routine that does only sparseness checking       
   /// No conversion or copying occurs, only pointer arithmetic               
   ///   @attention assumes the container is typed                            
   ///   @tparam T - the type of data we're accessing                         
   ///   @param idx - simple index for accessing                              
   ///   @param baseOffset - byte offset from the element to apply            
   ///   @return either pointer or reference to the element (depends on T)    
   template<CT::Data T> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   decltype(auto) Block::Get(Offset idx, Offset baseOffset) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mType, "Block is not typed");
      Byte* pointer;
      if (mType->mIsSparse)
         pointer = GetRawSparseAs<Byte, Block>()[idx] + baseOffset;
      else
         pointer = At(mType->mSize * idx) + baseOffset;

      if constexpr (CT::Dense<T>)
         return *reinterpret_cast<Deref<T>*>(pointer);
      else
         return reinterpret_cast<Deref<T>>(pointer);
   }

   template<CT::Data T> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   decltype(auto) Block::Get(Offset idx, Offset baseOffset) const IF_UNSAFE(noexcept) {
      return const_cast<Block*>(this)->Get<T>(idx, baseOffset);
   }
   
   /// Get an element at an index, trying to interpret it as T                
   /// No conversion or copying shall occur in this routine, only pointer     
   /// arithmetic based on CTTI or RTTI                                       
   ///   @tparam T - the type to interpret to                                 
   ///   @param index - the index                                             
   ///   @return either pointer or reference to the element (depends on T)    
   template<CT::Data T>
   decltype(auto) Block::As(CT::Index auto index) {
      // First quick type stage for fast access                         
      LANGULUS_ASSUME(DevAssumes, mType, "Block is not typed");
      if (mType->Is<T>()) {
         const auto idx = SimplifyIndex<TAny<T>>(index);
         LANGULUS_ASSERT(idx < mCount, Access, "Index out of range");
         return Get<T>(idx);
      }

      // Second fallback stage for compatible bases and mappings        
      const auto idx = SimplifyIndex<Any>(index);
      LANGULUS_ASSERT(idx < mCount, Access, "Index out of range");
      RTTI::Base base;
      if (not mType->template GetBase<T>(0, base)) {
         // There's still a chance if this container is resolvable      
         // This is the third and final stage                           
         auto resolved = GetElementResolved(idx);
         if (resolved.mType->template IsExact<T>()) {
            // Element resolved to a compatible type, so get it         
            return resolved.template Get<T>();
         }
         else if (resolved.mType->template GetBase<T>(0, base)) {
            // Get base memory of the resolved element and access       
            return resolved.GetBaseMemory(base)
               .template Get<T>(idx % base.mCount);
         }

         // All stages of interpretation failed                         
         // Don't log this, because it will spam the crap out of us     
         // That throw is used by ForEach to handle irrelevant types    
         LANGULUS_THROW(Access, "Type mismatch");
      }

      // Get base memory of the required element and access             
      return 
         GetElementDense(idx / base.mCount)
            .GetBaseMemory(base)
               .template Get<T>(idx % base.mCount);
   }

   template<CT::Data T> LANGULUS(INLINED)
   decltype(auto) Block::As(CT::Index auto index) const {
      return const_cast<Block&>(*this).As<T>(index);
   }
   
   /// Select an initialized region from the memory block                     
   ///   @param start - starting element index                                
   ///   @param count - number of elements to remain after 'start'            
   ///   @return the block representing the region                            
   template<CT::Block THIS> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   THIS Block::Crop(Offset start, Count count) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, start + count <= mCount, "Out of limits");

      auto& me = reinterpret_cast<THIS&>(*this);
      if (count == 0) {
         THIS result {Disown(me)};
         result.ResetMemory();
         return Abandon(result);
      }

      THIS result {Disown(me)};
      result.mCount = result.mReserved = count;
      result.mRaw += start * GetStride<THIS>();
      return Abandon(result);
   }

   /// Select an initialized region from the memory block (const)             
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the block representing the region                            
   template<CT::Block THIS> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   THIS Block::Crop(Offset start, Count count)
   const IF_UNSAFE(noexcept) {
      auto result = const_cast<Block*>(this)->Crop<THIS>(start, count);
      result.MakeConst();
      return result;
   }

   /// Get an element in container, and wrap it in a mutable dense block      
   ///   @attention the result will be empty if a sparse nullptr              
   ///   @tparam COUNT - number of indirections to remove                     
   ///   @param index - index of the element inside the block                 
   ///   @return the dense mutable memory block for the element               
   template<Count COUNT, CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetElementDense(Offset index) {
      return GetElement(index).template GetDense<COUNT, THIS>();
   }

   template<Count COUNT, CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetElementDense(Offset index) const {
      auto result = GetElement(index).template GetDense<COUNT, THIS>();
      result.MakeConst();
      return result;
   }
   
   /// Get the dense and most concrete block of an element inside the block   
   ///   @attention the element might be empty if resolved a sparse nullptr   
   ///   @param index - index of the element inside the block                 
   ///   @return the dense resolved memory block for the element              
   template<CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetElementResolved(Offset index) {
      return GetElement(index).template GetResolved<THIS>();
   }

   template<CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetElementResolved(Offset index) const {
      auto result = GetElement(index).template GetResolved<THIS>();
      result.MakeConst();
      return result;
   }

   /// Public function, to get a specific element block                       
   /// The resulting container will be a static view                          
   ///   @param index - the element's index                                   
   ///   @return the element's block                                          
   LANGULUS(INLINED)
   Block Block::GetElement(Offset index) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, index < mReserved,
         "Index out of range");

      Block result = GetElementInner(index);
      result.mState += DataState::Static;
      result.mState -= DataState::Or;
      return result;
   }

   LANGULUS(INLINED)
   Block Block::GetElement(Offset index) const IF_UNSAFE(noexcept) {
      auto result = const_cast<Block*>(this)->GetElement(index);
      result.MakeConst();
      return result;
   }
   
   /// Get a specific element block (inner, unsafe)                           
   ///   @attention will not make the resulting block static or const         
   ///   @param index - the element's index                                   
   ///   @return the element's block                                          
   LANGULUS(INLINED)
   Block Block::GetElementInner(Offset index) IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Invalid memory");

      Block result {*this};
      result.mCount = 1;
      result.mRaw += index * mType->mSize;
      return result;
   }

   LANGULUS(INLINED)
   Block Block::GetElementInner(Offset index) const IF_UNSAFE(noexcept) {
      return const_cast<Block*>(this)->GetElementInner(index);
   }

   /// Get a deep memory sub-block                                            
   ///   @param index - the index to get, indices are mapped as the following:
   ///      0 always refers to this block                                     
   ///      [1; mCount] always refer to subblocks in this block               
   ///      [mCount + 1; mCount + N] refer to subblocks in the first subblock 
   ///                               N being the size of that subblock        
   ///      ... and so on ...                                                 
   ///   @return a pointer to the block or nullptr if index is invalid        
   template<CT::Block THIS>
   Block* Block::GetBlockDeep(Count index) noexcept {
      // Zero index always returns this                                 
      if (index == 0)
         return this;
      if (not IsDeep<THIS>())
         return nullptr;

      --index;

      // [1; mCount] always refer to subblocks in this block            
      if (index < mCount)
         return GetRawAs<Block, THIS>() + index;

      index -= mCount;

      // [mCount + 1; mCount + N] refer to subblocks in local blocks    
      auto data = GetRawAs<Block, THIS>();
      const auto dataEnd = data + mCount;
      while (data != dataEnd) {
         const auto subpack = data->template GetBlockDeep<Block>(index + 1); //TODO can be optimized further with typed THIS
         if (subpack)
            return subpack;

         index -= data->template GetCountDeep<Block>() - 1; //TODO excess loops here, should be retrieved from GetElementDeep above as an optimization
         ++data;
      }

      return nullptr;
   }

   template<CT::Block THIS> LANGULUS(INLINED)
   const Block* Block::GetBlockDeep(Count index) const noexcept {
      return const_cast<Block*>(this)->GetBlockDeep<THIS>(index);
   }

   /// Get a deep element block                                               
   ///   @param index - the index to get                                      
   ///   @return the element block                                            
   template<CT::Block THIS>
   Block Block::GetElementDeep(Count index) noexcept {
      if (not IsDeep<THIS>())
         return index < mCount ? GetElement(index) : Block {};

      auto data = GetRawAs<Block, THIS>();
      const auto dataEnd = data + mCount;
      while (data != dataEnd) {
         const auto subpack = data->template GetElementDeep<Block>(index); //TODO can be optimized further with typed THIS
         if (subpack)
            return subpack;

         index -= data->template GetCountElementsDeep<Block>(); //TODO excess loops here, should be retrieved from GetElementDeep above as an optimization
         ++data;
      }

      return {};
   }

   template<CT::Block THIS> LANGULUS(INLINED)
   Block Block::GetElementDeep(Count index) const noexcept {
      auto result = const_cast<Block*>(this)->GetElementDeep<THIS>(index);
      result.MakeConst();
      return result;
   }
   
   /// Get the resolved first mutable element of this block                   
   ///   @attention assumes this block is valid and has at least one element  
   ///   @return the mutable resolved first element                           
   template<CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetResolved() {
      LANGULUS_ASSUME(DevAssumes, IsTyped<THIS>(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Block is empty");

      if (mType->mResolver)
         return mType->mResolver(GetDense<CountMax, THIS>().mRaw);
      else
         return GetDense<CountMax, THIS>();
   }

   template<CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetResolved() const {
      auto result = const_cast<Block*>(this)->template GetResolved<THIS>();
      result.MakeConst();
      return result;
   }

   /// Dereference first contained pointer                                    
   ///   @attention throws if type is incomplete and origin was reached       
   ///   @attention assumes this block is valid and has exactly one element   
   ///   @tparam COUNT - how many levels of indirection to remove?            
   ///   @return the mutable denser first element                             
   template<Count COUNT, CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetDense() {
      static_assert(COUNT > 0, "COUNT must be greater than 0");

      LANGULUS_ASSUME(DevAssumes, IsTyped<THIS>(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Block is empty");

      Block copy {*this};
      copy.mCount = 1;

      if constexpr (CT::Typed<THIS> and CT::Dense<TypeOf<THIS>>)
         return copy;
      else if constexpr (CT::Typed<THIS> and COUNT == 1) {
         // Statically dereference once                                 
         static_assert(CT::Complete<Deptr<TypeOf<THIS>>>,
            "Trying to interface incomplete data as dense");

         copy.mEntry = *GetEntries<THIS>();
         copy.mRaw = *mRawSparse;
         copy.mType = copy.mType->mDeptr;
      }
      else {
         // Dereference as much as needed at runtime                    
         Count counter = COUNT;
         while (counter and copy.mType->mIsSparse) {
            LANGULUS_ASSERT(copy.mType->mDeptr, Access,
               "Trying to interface incomplete data as dense");

            copy.mEntry = *GetEntries<THIS>();
            copy.mRaw = *mRawSparse;
            copy.mType = copy.mType->mDeptr;
            --counter;
         }
      }

      return copy;
   }

   template<Count COUNT, CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::GetDense() const {
      auto result = const_cast<Block*>(this)->template GetDense<COUNT, THIS>();
      result.MakeConst();
      return result;
   }

   /// Dereference first contained pointer once                               
   template<CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::operator * () {
      return GetDense<1, THIS>();
   }

   template<CT::BlockBased THIS> LANGULUS(INLINED)
   Block Block::operator * () const {
      return GetDense<1, THIS>();
   }
   
   /// Swap two elements inside this container                                
   ///   @param from_ - first index                                           
   ///   @param to_ - second index                                            
   template<CT::Block THIS> LANGULUS(INLINED)
   void Block::SwapIndices(CT::Index auto from_, CT::Index auto to_) {
      const auto from = SimplifyIndex<THIS>(from_);
      const auto to = SimplifyIndex<THIS>(to_);
      if (from >= mCount or to >= mCount or from == to)
         return;

      if constexpr (CT::Typed<THIS>) {
         auto data = GetRaw<THIS>();
         TypeOf<THIS> temp {::std::move(data[to])};
         data[to] = ::std::move(data[from]);
         SemanticAssign(data[from], Abandon(temp));
      }
      else {
         auto fblock = GetElementInner(from);
         auto tblock = GetElementInner(to);
         fblock.template Swap<THIS>(Abandon(tblock));
      }
   }
   
   /// Swap contents of this block, with the contents of another, using       
   /// a temporary block                                                      
   ///   @param rhs - the block to swap with                                  
   template<CT::Block THIS, class T> requires CT::Block<Desem<T>>
   void Block::Swap(T&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      LANGULUS_ASSUME(DevAssumes, mCount and DesemCast(rhs).mCount == mCount,
         "Invalid count");
      // Type-erased pointers (void*) are acceptable                    
      LANGULUS_ASSUME(DevAssumes, (DesemCast(rhs).GetType()->IsSimilar(GetType<THIS>())
         or (DesemCast(rhs).GetType()->template IsSimilar<void*>() and IsSparse<THIS>())),
         "Type mismatch on swap", ": ", DesemCast(rhs).GetType(), " != ", GetType<THIS>());

      using B = Conditional<CT::Typed<THIS>, THIS, TypeOf<S>>;
      Block temporary {mState, mType};
      temporary.AllocateFresh<B>(temporary.RequestSize<B>(mCount));
      temporary.mCount = mCount;

      // Move this to temporary                                         
      temporary.CreateSemantic<B>(Move(*this));
      // Destroy elements in this                                       
      /*CallDestructors<THIS>();
      // Abandon rhs to this                                            
      CallSemanticConstructors<THIS>(rhs->mCount, rhs.Forward());*/
      // Assign all elements from rhs to this                           
      AssignSemantic<B>(S::Nest(rhs));
      // Destroy elements in rhs                                        
      /*rhs->template CallDestructors<THIS>();
      // Abandon temporary to rhs                                       
      rhs->template CallSemanticConstructors<THIS>(temporary.mCount, Abandon(temporary));*/
      // Assign all elements from temporary to rhs                      
      DesemCast(rhs).template AssignSemantic<B>(Abandon(temporary));
      // Cleanup temporary                                              
      temporary.Destroy<B>();

      Allocator::Deallocate(const_cast<Allocation*>(temporary.mEntry));
   }

   /// Get the index of the biggest/smallest element                          
   ///   @tparam INDEX - either IndexBiggest or IndexSmallest                 
   ///   @return the index of the biggest element T inside this block         
   template<Index INDEX, CT::Block THIS> LANGULUS(INLINED)
   Index Block::GetIndex() const IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS> and CT::Inner::Sortable<TypeOf<THIS>>) {
         if (IsEmpty())
            return IndexNone;

         auto data = GetRaw<THIS>();
         const auto dataEnd = data + mCount;
         auto selection = data++;
         while (data != dataEnd) {
            if constexpr (INDEX == IndexBiggest) {
               if (*data > *selection)
                  selection = data;
            }
            else if constexpr (INDEX == IndexSmallest) {
               if (*data < *selection)
                  selection = data;
            }
            else LANGULUS_ERROR("Unsupported index");

            ++data;
         }

         return selection - GetRaw<THIS>();
      }
      else return IndexNone;
   }

   /// Get the index of element that repeats the most times                   
   ///   @param count - [out] count the number of repeats for the mode        
   ///   @return the index of the first found mode                            
   template<CT::Block THIS>
   Index Block::GetIndexMode(Count& count) const IF_UNSAFE(noexcept) {
      if constexpr (CT::Typed<THIS> and CT::Inner::Comparable<TypeOf<THIS>>) {
         if (IsEmpty()) {
            count = 0;
            return IndexNone;
         }

         auto data = GetRaw<THIS>();
         const auto dataEnd = data + mCount;
         decltype(data) best = nullptr;
         Count best_count {};
         while (data != dataEnd) {
            Count counter {};
            auto tail = data;
            while (tail != dataEnd) {
               if (*data == *tail)
                  ++counter;

               if (counter + (dataEnd - tail) <= best_count)
                  break;

               ++tail;
            }

            if (counter > best_count or not best) {
               best_count = counter;
               best = data;
            }

            ++data;
         }

         count = best_count;
         return best - data;
      }
      else return IndexNone;
   }

   /// Sort the contents of this container using a static type                
   ///   @attention assumes T is the type of the container                    
   ///   @tparam T - the type to use for comparison                           
   ///   @tparam ASCEND - whether to sort in ascending order (123)            
   template<CT::Data T, bool ASCEND>
   void Block::Sort() noexcept {
      auto lhs = GetRawAs<T>();
      const auto lhsEnd = lhs + mCount;
      while (lhs != lhsEnd) {
         auto rhs = GetRawAs<T>();
         while (rhs != lhs) {
            if constexpr (ASCEND) {
               if (*lhs < *rhs)
                  ::std::swap(*lhs, *rhs);
            }
            else {
               if (*lhs > *rhs)
                  ::std::swap(*lhs, *rhs);
            }

            ++rhs;
         }

         ++rhs;

         while (rhs != lhsEnd) {
            if constexpr (ASCEND) {
               if (*lhs < *rhs)
                  ::std::swap(*lhs, *rhs);
            }
            else {
               if (*lhs > *rhs)
                  ::std::swap(*lhs, *rhs);
            }

            ++rhs;
         }

         ++lhs;
      }
   }
   
   /// Return a handle to an element                                          
   ///   @tparam T - the contained type, or alternatively a Byte* for sparse  
   ///               unknowns; Byte for dense unknowns                        
   ///   @param index - the element index                                     
   ///   @return the handle                                                   
   template<CT::Data T, CT::Block THIS> LANGULUS(INLINED)
   Handle<T> Block::GetHandle(const Offset index) const IF_UNSAFE(noexcept) {
      const auto mthis = const_cast<Block*>(this);
      if constexpr (CT::Sparse<T>) {
         return {
            mthis->template GetRawAs<T, THIS>()[index],
            mthis->template GetEntries<THIS>()[index]
         };
      }
      else return {
         mthis->template GetRawAs<T, THIS>()[index], 
         mthis->mEntry
      };
   }

   /// Select region from the memory block - unsafe and may return memory     
   /// that has not been initialized yet (for internal use only)              
   ///   @attention assumes block is typed and allocated                      
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the block representing the region                            
   LANGULUS(INLINED)
   Block Block::CropInner(const Offset start, const Count count) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");

      Block result {*this};
      result.mCount = count;
      result.mRaw += start * mType->mSize;
      return result;
   }
   
   /// Constrain an index to the limits of the current block                  
   ///   @param idx - the index to constrain                                  
   ///   @return the constrained index or a special one of constrain fails    
   template<CT::Block THIS> LANGULUS(INLINED)
   Index Block::Constrain(const Index idx) const IF_UNSAFE(noexcept) {
      const auto result = idx.Constrained(mCount);
      if (result == IndexBiggest)
         return GetIndex<IndexBiggest, THIS>();
      else if (result == IndexSmallest)
         return GetIndex<IndexSmallest, THIS>();
      else if (result == IndexMode) {
         UNUSED() Count unused;
         return GetIndexMode<THIS>(unused);
      }

      return result;
   }

   /// Convert an index to an offset                                          
   /// Complex indices will be fully constrained                              
   /// Unsigned/signed integers are directly forwarded without any overhead   
   ///   @attention assumes T is correct for type-erased containers           
   ///   @tparam SAFE - whether to throw if index is beyond initialized count 
   ///   @param index - the index to simplify                                 
   ///   @return the simplified index, as a simple offset                     
   template<CT::Block THIS, bool SAFE, CT::Index INDEX> LANGULUS(INLINED)
   Offset Block::SimplifyIndex(const INDEX index) const
   noexcept(not LANGULUS_SAFE() and CT::BuiltinInteger<INDEX>) {
      if constexpr (CT::Same<INDEX, Index>) {
         // This is the most safe path, throws on errors                
         if constexpr (SAFE)
            return Constrain<THIS>(index).GetOffset();
         else
            return Constrain<THIS>(index).GetOffsetUnsafe();
      }
      else {
         // Unsafe, works only on assumptions                           
         // Using an integer index explicitly makes a statement, that   
         // you know what you're doing                                  
         LANGULUS_ASSUME(UserAssumes, 
            not SAFE or index < static_cast<INDEX>(mCount),
            "Integer index out of range"
         );

         if constexpr (CT::Signed<INDEX>) {
            LANGULUS_ASSUME(UserAssumes, index >= 0, 
               "Integer index is below zero, "
               "use Index for reverse indices instead"
            );
         }

         return index;
      }
   }
   
   /// Access last element                                                    
   ///   @return a mutable reference to the last element                      
   template<CT::Block THIS> LANGULUS(INLINED)
   decltype(auto) Block::Last() {
      if (IsEmpty())
         LANGULUS_OOPS(Access, "Unable to access last element of empty block");

      if constexpr (CT::Typed<THIS>)
         return GetRaw<THIS>()[mCount - 1];
      else
         return GetElement(mCount - 1);
   }

   template<CT::Block THIS> LANGULUS(INLINED)
   decltype(auto) Block::Last() const {
      if (IsEmpty())
         LANGULUS_OOPS(Access, "Unable to access last element of empty block");

      if constexpr (CT::Typed<THIS>)
         return GetRaw<THIS>()[mCount - 1];
      else
         return GetElement(mCount - 1);
   }
} // namespace Langulus::Anyness