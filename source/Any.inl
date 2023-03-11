///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Any.hpp"

namespace Langulus::Anyness
{

   /// Copy constructor - does only a shallow copy                            
   ///   @param other - the container to shallow-copy                         
   LANGULUS(ALWAYSINLINE)
   Any::Any(const Any& other)
      : Any {Langulus::Copy(other)} {}

   /// Move constructor - transfers ownership                                 
   ///   @param other - the container to move                                 
   LANGULUS(ALWAYSINLINE)
   Any::Any(Any&& other) noexcept
      : Any {Langulus::Move(other)} {}

   /// Construct by shallow-copying element/container                         
   ///   @param other - the element/container to shallow-copy                 
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any::Any(const T& other)
      : Any {Langulus::Copy(other)} {}

   /// Construct by shallow-copying element/container                         
   ///   @param other - the element/container to shallow-copy                 
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any::Any(T& other)
      : Any {Langulus::Copy(other)} {}

   /// Construct by moving element/container                                  
   ///   @param other - the element/container to move                         
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any::Any(T&& other) requires CT::Mutable<T>
      : Any {Langulus::Move(other)} {}

   /// Semantic constructor from deep container or custom data element        
   ///   @tparam S - type of insertion and semantic to use (deducible)        
   ///   @param other - the element/container to initialize with              
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Any::Any(S&& other) noexcept {
      using T = TypeOf<S>;

      if constexpr (CT::Deep<T>) {
         // Copy/Disown/Move/Abandon/Clone a deep container             
         BlockTransfer<Any>(other.Forward());
      }
      else if constexpr (CT::CustomData<T>) {
         // Copy/Disown/Move/Abandon/Clone an element                   
         SetType<T, false>();
         AllocateFresh(RequestSize(1));
         InsertInner(other.Forward(), 0);
      }
      else LANGULUS_ERROR("Bad semantic constructor argument");
   }

   /// Pack any number of elements sequentially                               
   /// If any of the types doesn't match exactly, the container becomes deep  
   /// to incorporate all elements                                            
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   template<CT::Data HEAD, CT::Data... TAIL>
   Any::Any(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1) {
      if constexpr (CT::Semantic<HEAD>) {
         // Types differ, so wrap each of them in a separate Any        
         SetType<Any, false>();
         AllocateFresh(RequestSize(sizeof...(TAIL) + 1));

         InsertInner(Abandon(Any {head.Forward()}), 0);
         InsertStatic<1>(Abandon(Any {Forward<TAIL>(tail)})...);
      }
      else if constexpr (CT::Exact<HEAD, TAIL...>) {
         // All types are the same, so pack them tightly                
         SetType<Decvq<Deref<HEAD>>, false>();
         AllocateFresh(RequestSize(sizeof...(TAIL) + 1));

         if constexpr (::std::is_rvalue_reference_v<HEAD>)
            InsertInner(Langulus::Move(head), 0);
         else
            InsertInner(Langulus::Copy(head), 0);

         InsertStatic<1>(Forward<TAIL>(tail)...);
      }
      else {
         // Types differ, so wrap each of them in a separate Any        
         SetType<Any, false>();
         AllocateFresh(RequestSize(sizeof...(TAIL) + 1));

         InsertInner(Abandon(Any {Forward<HEAD>(head)}), 0);
         InsertStatic<1>(Abandon(Any {Forward<TAIL>(tail)})...);
      }
   }

   /// Destruction                                                            
   LANGULUS(ALWAYSINLINE)
   Any::~Any() {
      Free();
   }

   /// Create an empty Any from a dynamic type and state                      
   ///   @param type - type of the container                                  
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   LANGULUS(ALWAYSINLINE)
   Any Any::FromMeta(DMeta type, const DataState& state) noexcept {
      return Any {Block {state, type}};
   }

   /// Create an empty Any by copying type and state of a block               
   ///   @param block - the source of type and state                          
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(ALWAYSINLINE)
   Any Any::FromBlock(const Block& block, const DataState& state) noexcept {
      return Any::FromMeta(block.GetType(), block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any by copying only state of a block                   
   ///   @param block - the source of the state                               
   ///   @param state - additional state of the container                     
   ///   @return the new container instance                                   
   LANGULUS(ALWAYSINLINE)
   Any Any::FromState(const Block& block, const DataState& state) noexcept {
      return Any::FromMeta(nullptr, block.GetUnconstrainedState() + state);
   }

   /// Create an empty Any from a static type and state                       
   ///   @tparam T - the contained type                                       
   ///   @param state - optional state of the container                       
   ///   @return the new container instance                                   
   template<CT::Data T>
   LANGULUS(ALWAYSINLINE)
   Any Any::From(const DataState& state) noexcept {
      return Block {state, MetaData::Of<T>()};
   }

   /// Pack any number of elements sequentially                               
   /// If any of the elements doesn't match the rest, the container becomes   
   /// deep to incorporate all elements                                       
   ///   @tparam LIST... - the list of element types (deducible)              
   ///   @param elements - sequential elements                                
   ///   @returns the pack containing the data                                
   template<CT::Data... LIST>
   LANGULUS(ALWAYSINLINE)
   Any Any::Wrap(LIST&&... elements) {
      if constexpr (sizeof...(LIST) == 0)
         return {};
      else
         return {Forward<LIST>(elements)...};
   }

   /// Pack any number of similarly typed elements sequentially               
   ///   @tparam AS - the type to wrap elements as                            
   ///                use 'void' to deduce AS from the HEAD                   
   ///                (void by default)                                       
   ///   @tparam HEAD - the first element type (deducible)                    
   ///   @tparam TAIL... - the rest of the element types (deducible)          
   ///   @param head - first element                                          
   ///   @param tail... - the rest of the elements                            
   ///   @returns the new container containing the data                       
   template<class AS, CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(ALWAYSINLINE)
   Any Any::WrapAs(HEAD&& head, TAIL&&... tail) {
      if constexpr (sizeof...(TAIL) == 0)
         return {};
      else if constexpr (CT::Void<AS>) {
         static_assert(CT::Exact<HEAD, TAIL...>, "Type mismatch");
         return {Forward<HEAD>(head), Forward<HEAD>(tail)...};
      }
      else {
         static_assert(CT::DerivedFrom<HEAD, AS>, "Head not related");
         static_assert((CT::DerivedFrom<TAIL, AS> && ...), "Tail not related");
         return {Forward<AS>(head), Forward<AS>(tail)...};
      }
   }
   
   /// Shallow-copy assignment                                                
   ///   @param other - the container to copy                                 
   ///   @return a reference to this container                                
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator = (const Any& other) {
      return operator = (Langulus::Copy(other));
   }

   /// Move assignment                                                        
   ///   @param other - the container to move and reset                       
   ///   @return a reference to this container                                
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator = (Any&& other) noexcept {
      return operator = (Langulus::Move(other));
   }

   /// Shallow copy assignment of anything                                    
   ///   @tparam T - the type to copy (deducible)                             
   ///   @param other - the value to copy                                     
   ///   @return a reference to this container                                
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator = (const T& other) {
      return operator = (Langulus::Copy(other));
   }
   
   /// Shallow copy assignment of anything                                    
   ///   @tparam T - the type to copy (deducible)                             
   ///   @param other - the value to copy                                     
   ///   @return a reference to this container                                
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator = (T& other) {
      return operator = (Langulus::Copy(other));
   }

   /// Move assignment of anything                                            
   ///   @tparam T - the type to move in (deducible)                          
   ///   @param other - the value to move in                                  
   ///   @return a reference to this container                                
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator = (T&& other) requires CT::Mutable<T> {
      return operator = (Langulus::Move(other));
   }

   /// Semantic assignment                                                    
   ///   @tparam S - the semantic and type to assign (deducible)              
   ///   @param other - the container to semantically assign                  
   ///   @return a reference to this container                                
   template<CT::Semantic S>
   Any& Any::operator = (S&& other) {
      using T = TypeOf<S>;
      static_assert(CT::Insertable<T>, "T must be an insertable type");

      if constexpr (CT::Deep<T>) {
         // Assign a container                                          
         if (this == &other.mValue)
            return *this;

         // Since Any is type-erased, we make a runtime type check      
         LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(other.mValue.GetType()),
            Assign, "Incompatible types");

         Free();
         new (this) Any {other.Forward()};
      }
      else {
         // Assign a non-deep value                                     
         const auto meta = MetaData::Of<T>();

         LANGULUS_ASSERT(!IsTypeConstrained() || CastsToMeta(meta),
            Assign, "Incompatible types");

         if (GetUses() != 1 || mType->mIsSparse != CT::Sparse<T>) {
            // Reset and allocate fresh memory                          
            Reset();
            operator << (other.Forward());
         }
         else{
            // Just destroy and reuse memory                            
            CallKnownDestructors<T>();
            mCount = 1;
            SemanticNew<T>(mRaw, other.Forward());
         }
      }

      return *this;
   }

   /// Destroy all elements, but retain allocated memory if possible          
   LANGULUS(ALWAYSINLINE)
   void Any::Clear() {
      if (IsEmpty())
         return;

      if (GetUses() == 1) {
         // Only one use - just destroy elements and reset count,       
         // reusing the allocation for later                            
         CallUnknownDestructors();
         ClearInner();
      }
      else {
         // We're forced to reset the memory, because it's in use       
         // Keep the type and state, though                             
         const auto state = GetUnconstrainedState();
         const auto meta = mType;
         Reset();
         mType = meta;
         mState += state;
      }
   }

   /// Copy-insert an element (including arrays) at the back                  
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator << (const T& other) {
      Insert<IndexBack>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator << (T& other) {
      Insert<IndexBack>(Langulus::Copy(other));
      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator << (T&& other) {
      Insert<IndexBack>(Langulus::Move(other));
      return *this;
   }

   /// Move-insert an element at the back                                     
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator << (S&& other) {
      Insert<IndexBack>(other.Forward());
      return *this;
   }

   /// Copy-insert an element (including arrays) at the front                 
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >> (const T& other) {
      Insert<IndexFront>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >> (T& other) {
      Insert<IndexFront>(Langulus::Copy(other));
      return *this;
   }

   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >> (T&& other) {
      Insert<IndexFront>(Langulus::Move(other));
      return *this;
   }
   
   /// Move-insert element at the front                                       
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >> (S&& other) {
      Insert<IndexFront>(other.Forward());
      return *this;
   }

   /// Merge data (including arrays) at the back                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator <<= (const T& other) {
      Merge<IndexBack, true>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator <<= (T& other) {
      Merge<IndexBack, true>(Langulus::Copy(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator <<= (T&& other) {
      Merge<IndexBack, true>(Langulus::Move(other));
      return *this;
   }

   /// Merge data at the back by move-insertion                               
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator <<= (S&& other) {
      Merge<IndexBack, true>(other.Forward());
      return *this;
   }

   /// Merge data at the front                                                
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >>= (const T& other) {
      Merge<IndexFront, true>(Langulus::Copy(other));
      return *this;
   }

   /// Used to disambiguate from the && variant                               
   /// Dimo, I know you want to remove this, but don't, said Dimo to himself  
   /// after actually deleting this function numerous times                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >>= (T& other) {
      Merge<IndexFront, true>(Langulus::Copy(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::NotSemantic T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >>= (T&& other) {
      Merge<IndexFront, true>(Langulus::Move(other));
      return *this;
   }

   /// Merge data at the front by move-insertion                              
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator >>= (S&& other) {
      Merge<IndexFront, true>(other.Forward());
      return *this;
   }

   /// Reset the container                                                    
   LANGULUS(ALWAYSINLINE)
   void Any::Reset() {
      Free();
      mRaw = nullptr;
      mCount = mReserved = 0;
      ResetState();
   }

   /// Swap two container's contents                                          
   ///   @param other - [in/out] the container to swap contents with          
   LANGULUS(ALWAYSINLINE)
   void Any::Swap(Any& other) noexcept {
      other = ::std::exchange(*this, ::std::move(other));
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(ALWAYSINLINE)
   Any Any::Crop(const Offset& start, const Count& count) const {
      return Any {Block::Crop(start, count)};
   }

   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   LANGULUS(ALWAYSINLINE)
   Any Any::Crop(const Offset& start, const Count& count) {
      return Any {Block::Crop(start, count)};
   }




   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// An inner concatenation routine using move/abandon                      
   ///   @tparam WRAPPER - the type of the concatenated container             
   ///   @tparam T - block type to concatenate with (deducible)               
   ///   @param rhs - block to concatenate                                    
   ///   @return the concatenated container                                   
   template<CT::Block WRAPPER, CT::Semantic S>
   WRAPPER Any::Concatenate(S&& rhs) const {
      static_assert(CT::Block<TypeOf<S>>,
         "S::Type must be a block type");

      if (IsEmpty())
         return {rhs.Forward()};
      else if (rhs.mValue.IsEmpty())
         return reinterpret_cast<const WRAPPER&>(*this);

      WRAPPER result;
      if constexpr (!CT::Typed<WRAPPER>)
         result.template SetType<false>(mType);

      result.AllocateFresh(result.RequestSize(mCount + rhs.mValue.mCount));
      result.InsertBlock(reinterpret_cast<const WRAPPER&>(*this));
      result.InsertBlock(rhs.Forward());
      return Abandon(result);
   }

   /// Copy-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   LANGULUS(ALWAYSINLINE)
   Any Any::operator + (const T& rhs) const requires CT::Dense<T> {
      return Concatenate<Any>(Langulus::Copy(rhs));
   }

   template<CT::Deep T>
   LANGULUS(ALWAYSINLINE)
   Any Any::operator + (T& rhs) const requires CT::Dense<T> {
      return Concatenate<Any>(Langulus::Copy(rhs));
   }

   /// Move-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Deep T>
   LANGULUS(ALWAYSINLINE)
   Any Any::operator + (T&& rhs) const requires CT::Dense<T> {
      return Concatenate<Any>(Langulus::Move(rhs));
   }

   /// Move-concatenate with any deep type                                    
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Any Any::operator + (S&& rhs) const requires (CT::Deep<TypeOf<S>>&& CT::Dense<TypeOf<S>>) {
      return Concatenate<Any>(rhs.Forward());
   }

   /// Destructive copy-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator += (const T& rhs) requires CT::Dense<T> {
      InsertBlock(Langulus::Copy(rhs));
      return *this;
   }

   template<CT::Deep T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator += (T& rhs) requires CT::Dense<T> {
      InsertBlock(Langulus::Copy(rhs));
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Deep T>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator += (T&& rhs) requires CT::Dense<T> {
      InsertBlock(Langulus::Move(rhs));
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @tparam T - type of the container to concatenate (deducible)         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   Any& Any::operator += (S&& rhs) requires (CT::Deep<TypeOf<S>>&& CT::Dense<TypeOf<S>>) {
      InsertBlock(rhs.Forward());
      return *this;
   }
   
   /// Find element(s) index inside container                                 
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @tparam BY_ADDRESS_ONLY - true to compare addresses only             
   ///   @param item - the item to search for                                 
   ///   @return the index of the found item, or IndexNone if none found      
   template<bool REVERSE, CT::Data T>
   LANGULUS(ALWAYSINLINE)
   Index Any::Find(const T& item, const Offset& cookie) const {
      return Block::template FindKnown<REVERSE>(item, cookie);
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   LANGULUS(ALWAYSINLINE)
   typename Any::Iterator Any::begin() noexcept {
      return IsEmpty() ? end() : GetElement();
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   LANGULUS(ALWAYSINLINE)
   typename Any::Iterator Any::end() noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * mCount;
      result.mCount = 0;
      return result;
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   LANGULUS(ALWAYSINLINE)
   typename Any::Iterator Any::last() noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * (mCount - 1);
      result.mCount = 1;
      return result;
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   LANGULUS(ALWAYSINLINE)
   typename Any::ConstIterator Any::begin() const noexcept {
      return IsEmpty() ? end() : GetElement();
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   LANGULUS(ALWAYSINLINE)
   typename Any::ConstIterator Any::end() const noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * mCount;
      result.mCount = 0;
      return result;
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   LANGULUS(ALWAYSINLINE)
   typename Any::ConstIterator Any::last() const noexcept {
      Block result {*this};
      if (IsEmpty())
         return result;

      result.MakeStatic();
      result.mRaw = mRaw + mType->mSize * (mCount - 1);
      result.mCount = 1;
      return result;
   }


   ///                                                                        
   ///   Block iterator                                                       
   ///                                                                        

   /// Construct an iterator                                                  
   ///   @param value - pointer to the value element                          
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   Any::TIterator<MUTABLE>::TIterator(const Block& value) noexcept
      : mValue {value} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename Any::TIterator<MUTABLE>& Any::TIterator<MUTABLE>::operator ++ () noexcept {
      mValue.mRaw += mValue.GetStride();
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename Any::TIterator<MUTABLE> Any::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare block entries                                                  
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   bool Any::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mValue.mRaw == rhs.mValue.mRaw;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const Block& Any::TIterator<MUTABLE>::operator * () const noexcept {
      return mValue;
   }

   


   
} // namespace Langulus::Anyness
