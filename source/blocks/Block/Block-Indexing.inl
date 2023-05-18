///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Block.hpp"
#include "../../inner/Handle.hpp"

namespace Langulus::Anyness
{
   
   /// Constrain an index to the limits of the current block                  
   ///   @tparam COUNT_CONSTRAINED - whether to use mCount or mReserved       
   ///   @param idx - the index to constrain                                  
   ///   @return the constrained index or a special one of constrain fails    
   template<bool COUNT_CONSTRAINED>
   LANGULUS(INLINED)
   constexpr Index Block::Constrain(const Index& idx) const noexcept {
      return idx.Constrained(COUNT_CONSTRAINED ? mCount : mReserved);
   }
   
   /// Constrain an index to the limits of the current block                  
   ///   @attention assumes T is the type of the container                    
   ///   @tparam T - the type to use for comparisons                          
   ///   @tparam COUNT_CONSTRAINED - whether to use mCount or mReserved       
   ///   @param idx - the index to constrain                                  
   ///   @return the constrained index or a special one of constrain fails    
   template<CT::Data T, bool COUNT_CONSTRAINED>
   LANGULUS(INLINED)
   Index Block::ConstrainMore(const Index& idx) const SAFETY_NOEXCEPT() {
      const auto result = Constrain<COUNT_CONSTRAINED>(idx);

      if (result == IndexBiggest) {
         if constexpr (CT::Sortable<T, T>)
            return GetIndex<T, IndexBiggest>();
         else
            return IndexNone;
      }
      else if (result == IndexSmallest) {
         if constexpr (CT::Sortable<T, T>)
            return GetIndex<T, IndexSmallest>();
         else
            return IndexNone;
      }
      else if (result == IndexMode) {
         if constexpr (CT::Sortable<T, T>) {
            UNUSED() Count unused;
            return GetIndexMode<T>(unused);
         }
         else return IndexNone;
      }

      return result;
   }

   /// Get the internal byte array with a given offset                        
   /// This is lowest level access and checks nothing                         
   ///   @attention assumes block is allocated                                
   ///   @param byteOffset - number of bytes to add                           
   ///   @return pointer to the selected raw data offset                      
   LANGULUS(INLINED)
   SAFETY_CONSTEXPR()
   Byte* Block::At(const Offset& byteOffset) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Invalid memory");
      LANGULUS_ASSUME(DevAssumes, byteOffset < GetReservedSize(),
         "Byte offset out of range");
      return mRaw + byteOffset;
   }

   /// Get the internal byte array with a given offset (const)                
   /// This is lowest level access and checks nothing                         
   ///   @attention assumes block is allocated                                
   ///   @param byteOffset - number of bytes to add                           
   ///   @return pointer to the selected raw data offset                      
   LANGULUS(INLINED)
   SAFETY_CONSTEXPR()
   const Byte* Block::At(const Offset& byte_offset) const SAFETY_NOEXCEPT() {
      return const_cast<Block*>(this)->At(byte_offset);
   }

   /// Access element at a specific index, and wrap it in a mutable Block     
   ///   @tparam IDX - the type of index we're using (deducible)              
   ///   @param idx - the index                                               
   ///   @return mutable type-erased element, wrapped in a Block              
   template<CT::Index IDX>
   LANGULUS(INLINED)
   Block Block::operator[] (const IDX& idx) {
      const auto index = SimplifyIndex<void>(idx);
      return GetElement(index);
   }

   /// Access element at a specific index, and wrap it in a constant Block    
   ///   @tparam IDX - the type of index we're using (deducible)              
   ///   @param idx - the index                                               
   ///   @return immutable type-erased element, wrapped in a Block            
   template<CT::Index IDX>
   LANGULUS(INLINED)
   Block Block::operator[] (const IDX& idx) const {
      const auto index = SimplifyIndex<void>(idx);
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
   template<CT::Data T>
   LANGULUS(INLINED)
   SAFETY_CONSTEXPR()
   decltype(auto) Block::Get(const Offset& idx, const Offset& baseOffset) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, IsTyped(), "Block is not typed");

      Byte* pointer;
      if (mType->mIsSparse)
         pointer = GetRawSparse()[idx] + baseOffset;
      else
         pointer = At(mType->mSize * idx) + baseOffset;

      if constexpr (CT::Dense<T>)
         return *reinterpret_cast<Deref<T>*>(pointer);
      else
         return reinterpret_cast<Deref<T>>(pointer);
   }

   /// Get a constant element pointer or reference with a given index         
   /// This is a lower-level routine that does only sparseness checking       
   /// No conversion or copying occurs, only pointer arithmetic               
   ///   @attention assumes the container is typed                            
   ///   @tparam T - the type of data we're accessing                         
   ///   @param idx - simple index for accessing                              
   ///   @param baseOffset - byte offset from the element to apply            
   ///   @return either pointer or reference to the element (depends on T)    
   template<CT::Data T>
   LANGULUS(INLINED)
   SAFETY_CONSTEXPR()
   decltype(auto) Block::Get(const Offset& idx, const Offset& baseOffset) const SAFETY_NOEXCEPT() {
      return const_cast<Block*>(this)->template Get<T>(idx, baseOffset);
   }
   
   /// Get an element at an index, trying to interpret it as T                
   /// No conversion or copying shall occur in this routine, only pointer     
   /// arithmetic based on CTTI or RTTI                                       
   ///   @attention assumes the container is typed                            
   ///   @tparam T - the type to interpret to                                 
   ///   @tparam IDX - the type used for indexing (deducible)                 
   ///   @param index - the index                                             
   ///   @return either pointer or reference to the element (depends on T)    
   template<CT::Data T, CT::Index IDX>
   decltype(auto) Block::As(const IDX& index) {
      if (!mType)
         LANGULUS_THROW(Access, "Untyped block");

      // First quick type stage for fast access                         
      if (mType->Is<T>()) {
         const auto idx = SimplifyIndex<T>(index);
         return Get<T>(idx);
      }

      // Second fallback stage for compatible bases and mappings        
      const auto idx = SimplifyIndex<void>(index);
      RTTI::Base base;
      if (!mType->template GetBase<T>(0, base)) {
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

   /// Get a constant element at an index, trying to interpret it as T        
   /// No conversion or copying shall occur in this routine, only pointer     
   /// arithmetic based on CTTI or RTTI                                       
   ///   @attention assumes the container is typed                            
   ///   @tparam T - the type to interpret to                                 
   ///   @tparam IDX - the type used for indexing (deducible)                 
   ///   @param index - the index                                             
   ///   @return either pointer or reference to the element (depends on T)    
   template<CT::Data T, CT::Index IDX>
   LANGULUS(INLINED)
   decltype(auto) Block::As(const IDX& index) const {
      return const_cast<Block&>(*this).template As<T, IDX>(index);
   }
   
   /// Select an initialized region from the memory block                     
   ///   @param start - starting element index                                
   ///   @param count - number of elements to remain after 'start'            
   ///   @return the block representing the region                            
   LANGULUS(INLINED)
   SAFETY_CONSTEXPR()
   Block Block::Crop(const Offset& start, const Count& count) SAFETY_NOEXCEPT() {
      if (count == 0)
         return {mState, mType};

      LANGULUS_ASSUME(DevAssumes, start + count <= mCount, "Out of limits");
      Block result {*this};
      result.mCount = count;
      result.mRaw += start * mType->mSize;
      result.mState += DataState::Member;
      return result;
   }

   /// Select an initialized region from the memory block (const)             
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the block representing the region                            
   LANGULUS(INLINED)
   SAFETY_CONSTEXPR()
   Block Block::Crop(const Offset& start, const Count& count) const SAFETY_NOEXCEPT() {
      auto result = const_cast<Block*>(this)->Crop(start, count);
      result.MakeConst();
      return result;
   }

   /// Get an element in container, and wrap it in a mutable dense block      
   ///   @attention the result will be empty if a sparse nullptr              
   ///   @param index - index of the element inside the block                 
   ///   @return the dense mutable memory block for the element               
   LANGULUS(INLINED)
   Block Block::GetElementDense(Offset index) {
      return GetElement(index).GetDense();
   }

   /// Get an element in container, and wrap it in a constant dense block     
   ///   @attention the result will be empty if a sparse nullptr              
   ///   @param index - index of the element inside the block                 
   ///   @return the dense immutable memory block for the element             
   LANGULUS(INLINED)
   const Block Block::GetElementDense(Offset index) const {
      return const_cast<Block*>(this)->GetElementDense(index);
   }
   
   /// Get the dense and most concrete block of an element inside the block   
   ///   @attention the element might be empty if resolved a sparse nullptr   
   ///   @param index - index of the element inside the block                 
   ///   @return the dense resolved memory block for the element              
   LANGULUS(INLINED)
   Block Block::GetElementResolved(Offset index) {
      return GetElement(index).GetResolved();
   }

   /// Get the dense const block of an element inside the block               
   ///   @param index - index of the element inside the block                 
   ///   @return the dense resolved memory block for the element              
   LANGULUS(INLINED)
   const Block Block::GetElementResolved(Count index) const {
      return const_cast<Block*>(this)->GetElementResolved(index);
   }

   /// Get a specific element block (unsafe)                                  
   ///   @param index - the element's index                                   
   ///   @return the element's block                                          
   LANGULUS(INLINED)
   Block Block::GetElement(Offset index) SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Invalid memory");
      LANGULUS_ASSUME(DevAssumes, index < mReserved,
         "Index out of range");

      Block result {*this};
      result.mState += DataState::Static;
      result.mState -= DataState::Or;
      result.mCount = 1;
      result.mRaw += index * mType->mSize;
      return result;
   }

   /// Get a specific element block (const, unsafe)                           
   ///   @param index - the element's index                                   
   ///   @return the element's block                                          
   LANGULUS(INLINED)
   const Block Block::GetElement(Offset index) const SAFETY_NOEXCEPT() {
      Block result {const_cast<Block*>(this)->GetElement(index)};
      result.MakeConst();
      return result;
   }

   /// Get first element block (unsafe)                                       
   ///   @return the first element's block                                    
   LANGULUS(INLINED)
   Block Block::GetElement() SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw,
         "Invalid memory");
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Block is empty");

      Block result {*this};
      result.mState += DataState::Static;
      result.mState -= DataState::Or;
      result.mCount = 1;
      return result;
   }

   /// Get first element block (const, unsafe)                                
   ///   @return the first element's block                                    
   LANGULUS(INLINED)
   const Block Block::GetElement() const SAFETY_NOEXCEPT() {
      Block result {const_cast<Block*>(this)->GetElement()};
      result.MakeConst();
      return result;
   }

   /// Get a deep memory sub-block                                            
   ///   @param index - the index to get, where 0 corresponds to this block   
   ///   @return a pointer to the block or nullptr if index is invalid        
   inline Block* Block::GetBlockDeep(Count index) noexcept {
      if (index == 0)
         return this;
      if (!IsDeep())
         return nullptr;

      --index;

      auto data = GetRawAs<Block>();
      const auto dataEnd = data + mCount;
      while (data != dataEnd) {
         const auto count = data->GetCountDeep();
         if (index <= count) {
            const auto subpack = data->GetBlockDeep(index);
            if (subpack)
               return subpack;
         }

         index -= count;
         ++data;
      }

      return nullptr;
   }

   /// Get a deep memory sub-block (const)                                    
   ///   @param index - the index to get                                      
   ///   @return a pointer to the block or nullptr if index is invalid        
   LANGULUS(INLINED)
   const Block* Block::GetBlockDeep(Count index) const noexcept {
      return const_cast<Block*>(this)->GetBlockDeep(index);
   }

   /// Get a deep element block                                               
   ///   @param index - the index to get                                      
   ///   @return the element block                                            
   LANGULUS(INLINED)
   Block Block::GetElementDeep(Count index) noexcept {
      if (!IsDeep())
         return index < mCount ? GetElement(index) : Block {};

      auto data = GetRawAs<Block>();
      const auto dataEnd = data + mCount;
      while (data != dataEnd) {
         const auto count = data->GetCountElementsDeep();
         if (index < count) 
            return data->GetElementDeep(index);

         index -= count;
         ++data;
      }

      return {};
   }

   /// Get a deep element block (const)                                       
   ///   @param index - the index to get                                      
   ///   @return the element block                                            
   LANGULUS(INLINED)
   const Block Block::GetElementDeep(Count index) const noexcept {
      Block result {const_cast<Block*>(this)->GetElementDeep(index)};
      result.MakeConst();
      return result;
   }
   
   /// Get the resolved first mutable element of this block                   
   ///   @attention assumes this block is valid and has at least one element  
   ///   @return the mutable resolved first element                           
   LANGULUS(INLINED)
   Block Block::GetResolved() {
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Block is empty");

      if (mType->mResolver)
         return mType->mResolver(GetDense().mRaw);
      else
         return GetDense();
   }

   /// Get the resolved first constant element of this block                  
   ///   @attention assumes this block is valid and has at least one element  
   ///   @return the immutable resolved first element                         
   LANGULUS(INLINED)
   const Block Block::GetResolved() const {
      Block result {const_cast<Block*>(this)->GetResolved()};
      result.MakeConst();
      return result;
   }

   /// Get the mutable first element of this block, with pointers removed     
   ///   @attention throws if type is incomplete and origin was reached       
   ///   @attention assumes this block is valid and has exactly one element   
   ///   @tparam COUNT - how many levels of indirection to remove?            
   ///   @return the mutable denser first element                             
   template<Count COUNT>
   LANGULUS(INLINED)
   Block Block::GetDense() {
      static_assert(COUNT > 0, "COUNT must be greater than 0");

      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");
      LANGULUS_ASSUME(DevAssumes, mCount > 0,
         "Block is empty");

      Count counter {COUNT};
      Block copy {*this};
      while (counter && copy.mType->mIsSparse) {
         LANGULUS_ASSERT(copy.mType->mDeptr, Access,
            "Trying to interface incomplete data as dense");

         copy.mEntry = *GetEntries();
         copy.mRaw = *GetRawSparse();
         copy.mType = copy.mType->mDeptr;
         --counter;
      }

      return copy;
   }

   /// Get the immutable first element of this block, with pointers removed   
   ///   @attention throws if type is incomplete and origin was reached       
   ///   @attention assumes this block is valid and has exactly one element   
   ///   @tparam COUNT - how many levels of indirection to remove?            
   ///   @return the immutable denser first element                           
   template<Count COUNT>
   LANGULUS(INLINED)
   const Block Block::GetDense() const {
      Block result {const_cast<Block*>(this)->template GetDense<COUNT>()};
      result.MakeConst();
      return result;
   }
   
   /// Swap two elements                                                      
   ///   @attention assumes T is exactly the contained type                   
   ///   @tparam T - the contained type                                       
   ///   @tparam INDEX1 - type of the first index (deducible)                 
   ///   @tparam INDEX2 - type of the second index (deducible)                
   ///   @param from_ - first index                                           
   ///   @param to_ - second index                                            
   template<CT::Data T, CT::Index INDEX1, CT::Index INDEX2>
   LANGULUS(INLINED)
   void Block::Swap(INDEX1 from_, INDEX2 to_) {
      LANGULUS_ASSUME(DevAssumes, IsExact<T>(), "Type mismatch");

      const auto from = SimplifyIndex(from_);
      const auto to = SimplifyIndex(to_);
      if (from >= mCount || to >= mCount || from == to)
         return;

      auto data = GetRawAs<T>();
      T temp {::std::move(data[to])};
      data[to] = ::std::move(data[from]);
      SemanticAssign(data[from], Abandon(temp));
   }
   
   /// Get the index of the biggest/smallest element                          
   ///   @attention assumes T is the type of the container                    
   ///   @tparam T - the type to use for comparison                           
   ///   @tparam INDEX - either IndexBiggest or IndexSmallest                 
   ///   @return the index of the biggest element T inside this block         
   template<CT::Data T, Index INDEX>
   LANGULUS(INLINED)
   Index Block::GetIndex() const SAFETY_NOEXCEPT() requires (CT::Sortable<T, T>) {
      if (IsEmpty())
         return IndexNone;

      auto data = GetRawAs<T>();
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

      return selection - GetRawAs<T>();
   }

   /// Get the index of element that repeats the most times                   
   ///   @attention assumes T is the type of the container                    
   ///   @tparam T - the type to use for comparison                           
   ///   @param count - [out] count the number of repeats for the mode        
   ///   @return the index of the first found mode                            
   template<CT::Data T>
   Index Block::GetIndexMode(Count& count) const SAFETY_NOEXCEPT() {
      if (IsEmpty()) {
         count = 0;
         return IndexNone;
      }

      auto data = GetRawAs<T>();
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

         if (counter > best_count || !best) {
            best_count = counter;
            best = data;
         }

         ++data;
      }

      count = best_count;
      return best - data;
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
   ///   @attention you must iterate handle differently if handling uknowns   
   ///   @param index - the element index                                     
   ///   @return the handle                                                   
   template<CT::Data T>
   LANGULUS(INLINED)
   Handle<T> Block::GetHandle(Offset index) const SAFETY_NOEXCEPT() {
      const auto mthis = const_cast<Block*>(this);
      return {
         mthis->template GetRawAs<T>()[index], 
         CT::Sparse<T> ? mthis->GetEntries()[index] : mthis->mEntry
      };
   }

   /// Select region from the memory block - unsafe and may return memory     
   /// that has not been initialized yet (for internal use only)              
   ///   @attention assumes block is typed and allocated                      
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the block representing the region                            
   LANGULUS(INLINED)
   Block Block::CropInner(const Offset& start, const Count& count) const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw != nullptr,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");

      Block result {*this};
      result.mCount = count;
      result.mRaw += start * mType->mSize;
      return result;
   }
   
   /// Get next element by incrementing data pointer (for inner use)          
   LANGULUS(INLINED)
   void Block::Next() SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw != nullptr,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");

      mRaw += mType->mSize;
   }

   /// Get previous element by decrementing data pointer (for inner use)      
   LANGULUS(INLINED)
   void Block::Prev() SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw != nullptr,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");

      mRaw -= mType->mSize;
   }

   /// Get next element by incrementing data pointer (for inner use)          
   ///   @return a new block with the incremented pointer                     
   LANGULUS(INLINED)
   Block Block::Next() const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw != nullptr,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");

      Block copy {*this};
      copy.mRaw += mType->mSize;
      return copy;
   }

   /// Get previous element by decrementing data pointer (for inner use)      
   ///   @return a new block with the decremented pointer                     
   LANGULUS(INLINED)
   Block Block::Prev() const SAFETY_NOEXCEPT() {
      LANGULUS_ASSUME(DevAssumes, mRaw != nullptr,
         "Block is not allocated");
      LANGULUS_ASSUME(DevAssumes, IsTyped(),
         "Block is not typed");

      Block copy {*this};
      copy.mRaw -= mType->mSize;
      return copy;
   }

   /// Convert an index to an offset                                          
   /// Complex indices will be fully constrained                              
   /// Signed index types will be checked for negative indices (for reverses) 
   /// Unsigned indices are directly forwarded without any overhead           
   ///   @attention assumes T is correct for type-erased containers           
   ///   @attention assumes index is in container count limit, if unsigned,   
   ///              and COUNT_CONSTRAINED is true                             
   ///   @attention assumes index is in container reserve limit, if unsigned, 
   ///              and COUNT_CONSTRAINED is false                            
   ///   @tparam T - the type we're indexing, used for additional special     
   ///               index handling, like Min and Max, that require type info 
   ///               use void to skip these indices at no cost                
   ///   @tparam COUNT_CONSTRAINED - will check count limits if true or       
   ///                               reserve limit if false, when DevAssumes  
   ///                               is enabled                               
   ///   @tparam INDEX - type of the index to simplify                        
   ///   @param index - the index to simplify                                 
   ///   @return the offset                                                   
   template<class T, bool COUNT_CONSTRAINED, CT::Index INDEX>
   LANGULUS(INLINED)
   Offset Block::SimplifyIndex(const INDEX& index) const noexcept(!LANGULUS_SAFE() && CT::Unsigned<INDEX>) {
      if constexpr (CT::Same<INDEX, Index>) {
         // This is the most safe path, throws on errors                
         if constexpr (CT::Void<T>)
            return Constrain<COUNT_CONSTRAINED>(index).GetOffset();
         else {
            if constexpr (!CT::Void<T>)
               LANGULUS_ASSUME(DevAssumes, (CastsTo<T, true>()), "Type mismatch");

            return ConstrainMore<T, COUNT_CONSTRAINED>(index).GetOffset();
         }
      }
      else if constexpr (CT::Signed<INDEX>) {
         // Somewhat safe, default literal type is signed               
         if (index < 0) {
            const auto unsign = static_cast<Offset>(-index);
            LANGULUS_ASSERT(
               unsign <= (COUNT_CONSTRAINED ? mCount : mReserved), Access,
               "Reverse index out of count range"
            );
            return mCount - unsign;
         }
         else {
            const auto unsign = static_cast<Offset>(index);
            LANGULUS_ASSERT(
               unsign < (COUNT_CONSTRAINED ? mCount : mReserved), Access,
               "Signed index out of count range"
            );
            return unsign;
         }
      }
      else {
         // Unsafe, works only on assumptions                           
         // Using an unsigned index explicitly makes a statement, that  
         // you know what you're doing                                  
         LANGULUS_ASSUME(UserAssumes, 
            index < (COUNT_CONSTRAINED ? mCount : mReserved),
            "Unsigned index out of range"
         );
         return index;
      }
   }

} // namespace Langulus::Anyness