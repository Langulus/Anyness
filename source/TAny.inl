///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TAny.hpp"
#include <cctype>

#define TEMPLATE() template<CT::Data T>

namespace Langulus::Anyness
{

   /// Default construction                                                   
   /// TAny is type-constrained and always has a type                         
   TEMPLATE()
   TAny<T>::TAny()
      : Any {Block {DataState::Typed, MetaData::Of<Decay<T>>()}} {
      if constexpr (CT::Sparse<T>)
         MakeSparse();
      if constexpr (CT::Constant<T>)
         MakeConst();
   }

   /// Shallow-copy construction (const)                                      
   ///   @param other - the TAny to reference                                 
   TEMPLATE()
   TAny<T>::TAny(const TAny& other)
      : Any {other} { }

   /// Move construction                                                      
   ///   @param other - the TAny to move                                      
   TEMPLATE()
   TAny<T>::TAny(TAny&& other) noexcept
      : Any {Forward<TAny>(other)} { }

   /// Copy-construct from another container by performing runtime type check 
   ///   @tparam KEEP - whether or not to reference contents                  
   ///   @tparam ALT_T - the container type (deducible)                       
   ///   @param other - the container to incorporate                          
   TEMPLATE()
   template<bool KEEP, CT::Deep ALT_T>
   void TAny<T>::ConstructFromContainer(const ALT_T& other) {
      if constexpr (CT::Same<ALT_T, TAny>) {
         CopyProperties<false, false>(other);

         if constexpr (KEEP)
            Keep();
      }
      else {
         if (CastsToMeta(other.GetType())) {
            // Always attempt to copy containers directly first,        
            // instead of doing allocations                             
            CopyProperties<false, false>(other);

            if constexpr (KEEP)
               Keep();
            return;
         }

         LANGULUS_THROW(Copy, "Bad copy-construction");
      }
   }

   /// Move-construct from another container by performing runtime type check 
   ///   @tparam KEEP - whether or not to reference contents                  
   ///   @tparam ALT_T - the container type (deducible)                       
   ///   @param other - the container to incorporate                          
   TEMPLATE()
   template<bool KEEP, CT::Deep ALT_T>
   void TAny<T>::ConstructFromContainer(ALT_T&& other) {
      if constexpr (CT::Same<ALT_T, TAny>) {
         CopyProperties<false, true>(other);

         if constexpr (!KEEP)
            other.mEntry = nullptr;
         else {
            other.ResetMemory();
            other.ResetState();
         }
      }
      else {
         if (CastsToMeta(other.GetType())) {
            // Always attempt to copy containers directly first,        
            // instead of doing allocations                             
            CopyProperties<false, true>(other);

            if constexpr (!KEEP)
               other.mEntry = nullptr;
            else {
               other.ResetMemory();
               other.ResetState();
            }
            return;
         }

         LANGULUS_THROW(Copy, "Bad move-construction");
      }
   }

   /// Copy-construction from any deep container, with a bit of               
   /// runtime type-checking overhead                                         
   ///   @param other - the anyness to reference                              
   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>::TAny(const ALT_T& other)
      : TAny {} {
      ConstructFromContainer<true>(other);
   }

   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>::TAny(ALT_T& other)
      : TAny {const_cast<const ALT_T&>(other)} {}

   /// Move-construction from any deep container, with a bit of               
   /// runtime type-checking overhead                                         
   ///   @param other - the anyness to reference                              
   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>::TAny(ALT_T&& other)
      : TAny {} {
      ConstructFromContainer<true>(Forward<Any>(other));
   }

   /// Disown-construction from any deep container, with a bit of             
   /// runtime type-checking overhead                                         
   ///   @param other - the anyness to copy                                   
   TEMPLATE()
   template<CT::Deep ALT_T>
   constexpr TAny<T>::TAny(Disowned<ALT_T>&& other)
      : TAny {} {
      ConstructFromContainer<false>(other.mValue);
   }

   /// Abandon-construction from any deep container, with a bit of            
   /// runtime type-checking overhead                                         
   ///   @param other - the anyness to copy                                   
   TEMPLATE()
   template<CT::Deep ALT_T>
   constexpr TAny<T>::TAny(Abandoned<ALT_T>&& other)
      : TAny {} {
      ConstructFromContainer<false>(Move(other.mValue));
   }

   /// Construct by copying/referencing an array of non-block type            
   ///   @param start - start of the array                                    
   ///   @param end - end of the array                                        
   TEMPLATE()
   TAny<T>::TAny(const T* start, const T* end) requires CT::Data<T>
      : Any {start, end} { }

   /// Construct by copying/referencing value of non-block type               
   ///   @param other - the value to shallow-copy                             
   TEMPLATE()
   TAny<T>::TAny(const T& other) requires CT::CustomData<T>
      : Any {other} { }

   /// Construct by moving a dense value of non-block type                    
   ///   @param initial - the value to forward and emplace                    
   TEMPLATE()
   TAny<T>::TAny(T&& initial) requires CT::CustomData<T>
      : Any {Forward<T>(initial)} { }

   /// Construct by inserting a disowned non-block element                    
   ///   @param other - the value to insert                                   
   TEMPLATE()
   TAny<T>::TAny(Disowned<T>&& other) requires CT::CustomData<T>
      : Any {other.Forward()} { }

   /// Construct by inserting an abandoned non-block element                  
   ///   @param other - the value to insert                                   
   TEMPLATE()
   TAny<T>::TAny(Abandoned<T>&& other) requires CT::CustomData<T>
      : Any {other.Forward()} { }

   /// Construct manually by interfacing memory directly                      
   /// Data will be copied, if not in jurisdiction, which involves a slow     
   /// authority check. If you want to avoid checking and copying, use the    
   /// Disowned override of this function                                     
   ///   @param raw - raw memory to reference, or clone if not owned          
   ///   @param count - number of items inside 'raw'                          
   TEMPLATE()
   TAny<T>::TAny(const T* raw, const Count& count)
      : Any {Block {DataState::Constrained, MetaData::Of<Decay<T>>(), count, raw}} {
      TakeAuthority();
   }

   /// Construct manually by interfacing memory directly                      
   ///   @attention unsafe, make sure that lifetime of memory is sane         
   ///   @param raw - raw memory to interface without referencing or copying  
   ///   @param count - number of items inside 'raw'                          
   TEMPLATE()
   TAny<T>::TAny(Disowned<const T*>&& raw, const Count& count) noexcept
      : Any {Block {
         DataState::Constrained, MetaData::Of<Decay<T>>(), count, raw.mValue, nullptr
      }} {}

   /// Destructor                                                             
   TEMPLATE()
   TAny<T>::~TAny() {
      Free();
   }

   /// Dereference and eventually destroy all elements                        
   TEMPLATE()
   void TAny<T>::Free() {
      if (!mEntry)
         return;

      if (mEntry->GetUses() == 1) {
         if constexpr (CT::Sparse<T> || !CT::POD<T>)
            CallKnownDestructors<T>();
         Inner::Allocator::Deallocate(mEntry);
         mEntry = nullptr;
         return;
      }

      mEntry->Free();
      mEntry = nullptr;
   }

   /// Shallow-copy assignment                                                
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator = (const TAny& other) {
      if (this == &other)
         return *this;

      Free();
      other.Keep();
      CopyProperties<true, true>(other);
      return *this;
   }

   /// Move assignment                                                        
   ///   @param other - the container to move                                 
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator = (TAny&& other) noexcept {
      if (this == &other)
         return *this;

      Free();
      CopyProperties<true, true>(other);
      other.ResetMemory();
      other.ResetState();
      return *this;
   }

   /// Copy-construct from another container by performing runtime type check 
   ///   @tparam KEEP - whether or not to reference contents                  
   ///   @tparam ALT_T - the container type (deducible)                       
   ///   @param other - the container to incorporate                          
   TEMPLATE()
   template<bool KEEP, CT::Deep ALT_T>
   void TAny<T>::AssignFromContainer(const ALT_T& other) {
      if constexpr (CT::Same<TAny, ALT_T>) {
         Free();
         CopyProperties<true, false>(other);
         return;
      }
      else if (CastsToMeta(other.GetType())) {
         // Always attempt to copy containers directly first, instead   
         // of doing allocations                                        
         Free();
         if constexpr (KEEP)
            other.Keep();
         ResetState();
         CopyProperties<false, true>(other);
         return;
      }

      LANGULUS_THROW(Copy, "Bad copy-assignment");
   }

   /// Move-construct from another container by performing runtime type check 
   ///   @tparam KEEP - whether or not to reference contents                  
   ///   @tparam ALT_T - the container type (deducible)                       
   ///   @param other - the container to incorporate                          
   TEMPLATE()
   template<bool KEEP, CT::Deep ALT_T>
   void TAny<T>::AssignFromContainer(ALT_T&& other) {
      if constexpr (CT::Same<TAny, ALT_T>) {
         Free();
         CopyProperties<true, true>(other);
         other.mEntry = nullptr;
         return;
      }
      else if (CastsToMeta(other.GetType())) {
         // Always attempt to copy containers directly first, instead   
         // of doing allocations                                        
         Free();
         ResetState();
         CopyProperties<false, true>(other);

         if constexpr (KEEP)
            other.mEntry = nullptr;
         else {
            other.ResetMemory();
            other.ResetState();
         }
         return;
      }

      LANGULUS_THROW(Copy, "Bad move-assignment");
   }

   /// Copy-assign an unknown container                                       
   /// This is a bit slower, because it checks type compatibility at runtime  
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>& TAny<T>::operator = (const ALT_T& other) {
      if (static_cast<Block*>(this) == &other)
         return *this;

      AssignFromContainer<true>(other);
      return *this;
   }
   
   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>& TAny<T>::operator = (ALT_T& other) {
      return operator = (const_cast<const ALT_T&>(other));
   }

   /// Move-assign an unknown container                                       
   /// This is a bit slower, because it checks type compatibility at runtime  
   ///   @param other - the container to move                                 
   ///   @return a reference to this container                                
   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>& TAny<T>::operator = (ALT_T&& other) {
      if (static_cast<Block*>(this) == &other)
         return *this;

      AssignFromContainer<true>(Forward<ALT_T>(other));
      return *this;
   }

   /// Shallow-copy disowned runtime container without referencing contents   
   /// This is a bit slower, because checks type compatibility at runtime     
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>& TAny<T>::operator = (Disowned<ALT_T>&& other) {
      if (static_cast<Block*>(this) == &other.mValue)
         return *this;

      AssignFromContainer<false>(other.mValue);
      return *this;
   }

   /// Move abandoned runtime container without fully resetting it            
   /// This is a bit slower, because checks type compatibility at runtime     
   ///   @param other - the container to move                                 
   ///   @return a reference to this container                                
   TEMPLATE()
   template<CT::Deep ALT_T>
   TAny<T>& TAny<T>::operator = (Abandoned<ALT_T>&& other) {
      if (static_cast<Block*>(this) == &other.mValue)
         return *this;

      AssignFromContainer<false>(Forward<ALT_T>(other.mValue));
      return *this;
   }

   /// Assign by shallow-copying an element                                   
   ///   @param other - the value to copy                                     
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator = (const T& other) requires CT::CustomData<T> {
      if (GetUses() == 1) {
         // Just destroy and reuse memory											
         CallKnownDestructors<T>();
         mCount = 0;
         InsertInner<true>(&other, &other + 1, 0);
      }
      else {
         // Reset and allocate new memory											
         Reset();
         operator << (other);
      }

      return *this;
   }
   
   TEMPLATE()
   TAny<T>& TAny<T>::operator = (T& other) requires CT::CustomData<T> {
      return operator = (const_cast<const T&>(other));
   }

   /// Assign by moving an element                                            
   ///   @param other - the value to move                                     
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator = (T&& other) requires CT::CustomData<T> {
      if (GetUses() == 1) {
         // Just destroy and reuse memory											
         CallKnownDestructors<T>();
         mCount = 0;
         InsertInner<true>(Forward<T>(other), 0);
      }
      else {
         // Reset and allocate new memory											
         Reset();
         operator << (Forward<T>(other));
      }

      return *this;
   }

   /// Assign by interfacing a disowned element                               
   ///   @param other - the element to interface                              
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator = (Disowned<T>&& other) noexcept requires CT::CustomData<T> {
      if (GetUses() != 1) {
         // Reset and allocate new memory                               
         // Disowned-construction will be used if possible              
         Reset();
         operator << (other.Forward());
      }
      else {
         // Just destroy and reuse memory                               
         if constexpr (CT::Sparse<T>) {
            CallKnownDestructors<T>();
            mCount = 1;
            GetRawSparse()->mPointer = other.mValue;
            GetRawSparse()->mEntry = nullptr;
         }
         else {
            CallKnownDestructors<T>();
            mCount = 1;
            if constexpr (CT::DisownMakable<T>)
               new (mRaw) T {other.Forward()};
            else
               new (mRaw) T {other.mValue};
         }
      }

      return *this;
   }

   /// Assign by interfacing an abandoned element                             
   ///   @param other - the element to interface                              
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator = (Abandoned<T>&& other) noexcept requires CT::CustomData<T> {
      if (GetUses() != 1) {
         // Reset and allocate new memory                               
         // Abandoned-construction will be used if possible             
         Reset();
         operator << (other.Forward());
      }
      else {
         // Just destroy and reuse memory                               
         if constexpr (CT::Sparse<T>) {
            CallKnownDestructors<T>();
            mCount = 1;
            GetRawSparse()->mPointer = other.mValue;
            GetRawSparse()->mEntry = nullptr;
         }
         else {
            CallKnownDestructors<T>();
            mCount = 1;
            if constexpr (CT::AbandonMakable<T>)
               new (mRaw) T {other.Forward()};
            else
               new (mRaw) T {Forward<T>(other.mValue)};
         }
      }

      return *this;
   }

   /// An internal function used to copy members, without copying type and    
   /// without overwriting states, if required                                
   ///   @param other - the block to copy from                                
   TEMPLATE()
   template<bool OVERWRITE_STATE, bool OVERWRITE_ENTRY>
   void TAny<T>::CopyProperties(const Block& other) noexcept {
      mRaw = other.mRaw;
      mCount = other.mCount;
      mReserved = other.mReserved;
      if constexpr (OVERWRITE_STATE)
         mState = other.mState;
      else
         mState += other.mState;

      if constexpr (OVERWRITE_ENTRY)
         mEntry = other.mEntry;
   }

   /// Reset container state (inner function)                                 
   TEMPLATE()
   constexpr void TAny<T>::ResetState() noexcept {
      mState = mState.mState & (DataState::Typed | DataState::Sparse);
   }

   /// Reset container type (does nothing for typed container)                
   TEMPLATE()
   constexpr void TAny<T>::ResetType() noexcept {}

   /// Check if contained data can be interpreted as a given type             
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE()
   bool TAny<T>::CastsToMeta(DMeta type) const {
      return mType->CastsTo<CT::Sparse<T>>(type);
   }

   /// Check if contained data can be interpreted as a given count of type    
   /// For example: a vec4 can interpret as float[4]                          
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @param count - the number of elements to interpret as                
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE()
   bool TAny<T>::CastsToMeta(DMeta type, Count count) const {
      return mType->CastsTo(type, count);
   }

   /// Wrap stuff in a container                                              
   ///   @param anything - pack it inside a dense container                   
   ///   @returns the pack containing the data                                
   TEMPLATE()
   template<CT::Data... LIST_T>
   TAny<T> TAny<T>::Wrap(LIST_T&&... list) {
      TAny<T> temp;
      temp.Allocate(sizeof...(list));
      (temp << ... << Forward<LIST_T>(list));
      return Abandon(temp);
   }

   /// Allocate 'count' elements and fill the container with zeroes           
   TEMPLATE()
   void TAny<T>::Null(const Count& count) {
      Allocate(count, false, true);
      FillMemory(mRaw, {}, GetByteSize());
   }

   /// Clear the container, destroying all elements,                          
   /// but retaining allocation if possible                                   
   TEMPLATE()
   void TAny<T>::Clear() {
      if (!mCount)
         return;

      if (GetUses() == 1) {
         // Only one use - just destroy elements and reset count,       
         // reusing the allocation for later                            
         CallKnownDestructors<T>();
         ClearInner();
      }
      else {
         // We're forced to reset the memory, because it's in use       
         // Keep the type and state, though                             
         const auto state = GetUnconstrainedState();
         Reset();
         mState += state;
      }
   }

   /// Reset the container, destroying all elements, and deallocating         
   TEMPLATE()
   void TAny<T>::Reset() {
      Free();
      ResetMemory();
      ResetState();
   }

   /// Clone the templated container                                          
   ///   @returns either a deep clone of the container data, or a shallow     
   ///            copy, if contained type is not clonable                     
   TEMPLATE()
   TAny<T> TAny<T>::Clone() const {
      using DT = Decay<T>;

      if constexpr (CT::Clonable<T> || CT::POD<T>) {
         // Always clone the state, but make it unconstrained           
         TAny<T> result {Disown(*this)};
         result.mState -= DataState::Static | DataState::Constant;

         if (!IsAllocated())
            return Abandon(result);

         result.ResetMemory();
         result.Allocate<false>(mCount);
         result.mCount = mCount;
         auto from = GetRaw();
         auto to = result.GetRaw();

         if constexpr (CT::Sparse<T>) {
            // Clone all data in the same block                         
            TAny<DT> coalesced;
            coalesced.Allocate(mCount);
            auto co = coalesced.GetRaw();

            // Clone data behind each valid pointer                     
            Count counter {};
            while (from < GetRawEnd()) {
               if (!from->mPointer) {
                  to->mPointer = nullptr;
                  to->mEntry = nullptr;
                  ++from; ++to;
                  continue;
               }

               if constexpr (CT::Clonable<T>)
                  new (co + counter) DT {from->mPointer->Clone()};
               else if constexpr (CT::CopyMakable<T>)
                  new (co + counter) DT {*from->mPointer};
               else if constexpr (CT::POD<T>)
                  CopyMemory(from->mPointer, co + counter, sizeof(DT));
               else
                  LANGULUS_ERROR("Can't clone a container made of non-clonable/non-POD type");

               to->mPointer = co + counter;
               to->mEntry = coalesced.mEntry;
               ++from; ++to;
               ++counter;
            }

            coalesced.Reference(counter);
         }
         else if constexpr (CT::Clonable<T>) {
            // Clone dense elements by calling their Clone()            
            while (from < GetRawEnd()) {
               new (to) DT {from->Clone()};
               ++from; ++to;
            }
         }
         else if constexpr (CT::CopyMakable<T>) {
            // Copy elements by calling their copy-constructors         
            while (from < GetRawEnd()) {
               new (to) DT {*from};
               ++from; ++to;
            }
         }
         else if constexpr (CT::POD<T>) {
            // Batch copy everything at once                            
            CopyMemory(from, to, sizeof(DT) * mCount);
         }
         else LANGULUS_ERROR("Can't clone a container made of non-clonable/non-POD type");

         return Abandon(result);
      }
      else {
         // Can't clone the data, just return a shallow-copy            
         return *this;
      }
   }

   /// Return the typed raw data (const)                                      
   ///   @return a constant pointer to the first element in the array         
   TEMPLATE()
   auto TAny<T>::GetRaw() const noexcept {
      if constexpr (CT::Dense<T>)
         return Any::GetRawAs<T>();
      else
         return Any::GetRawAs<KnownPointer>();
   }

   /// Return the typed raw data                                              
   ///   @return a mutable pointer to the first element in the array          
   TEMPLATE()
   auto TAny<T>::GetRaw() noexcept {
      if constexpr (CT::Dense<T>)
         return Any::GetRawAs<T>();
      else
         return Any::GetRawAs<KnownPointer>();
   }

   /// Return the typed raw data end pointer (const)                          
   ///   @return a constant pointer to one past the last element in the array 
   TEMPLATE()
   auto TAny<T>::GetRawEnd() const noexcept {
      return GetRaw() + mCount;
   }

   /// Return the typed raw data	end pointer                                  
   ///   @return a mutable pointer to one past the last element in the array  
   TEMPLATE()
   auto TAny<T>::GetRawEnd() noexcept {
      return GetRaw() + mCount;
   }

   /// Return the typed raw sparse data (const)                               
   ///   @return a constant pointer to the first element in the array         
   TEMPLATE()
   auto TAny<T>::GetRawSparse() const noexcept {
      return reinterpret_cast<const KnownPointer*>(mRawSparse);
   }

   /// Return the typed raw sparse data                                       
   ///   @return a mutable pointer to the first element in the array          
   TEMPLATE()
   auto TAny<T>::GetRawSparse() noexcept {
      return reinterpret_cast<KnownPointer*>(mRawSparse);
   }

   /// Get an element in the way you want (const, unsafe)                     
   /// This is a statically optimized variant of Block::Get                   
   TEMPLATE()
   template<CT::Data ALT_T>
   decltype(auto) TAny<T>::Get(const Offset& index) const noexcept {
      const auto& element = GetRaw()[index];
      if constexpr (CT::Dense<T> && CT::Dense<ALT_T>)
         // Dense -> Dense (returning a reference)                      
         return static_cast<const Decay<ALT_T>&>(element);
      else if constexpr (CT::Dense<T> && CT::Sparse<ALT_T>)
         // Dense -> Sparse (returning a pointer)                       
         return static_cast<const Decay<ALT_T>*>(&element);
      else if constexpr (CT::Sparse<T> && CT::Dense<ALT_T>)
         // Sparse -> Dense (returning a reference)                     
         return static_cast<const Decay<ALT_T>&>(*element.mPointer);
      else
         // Sparse -> Sparse (returning a reference to pointer)         
         return static_cast<const Decay<ALT_T>* const&>(element.mPointer);
   }

   /// Get an element in the way you want (unsafe)                            
   /// This is a statically optimized variant of Block::Get                   
   TEMPLATE()
   template<CT::Data ALT_T>
   decltype(auto) TAny<T>::Get(const Offset& index) noexcept {
      auto& element = GetRaw()[index];
      if constexpr (CT::Dense<T> && CT::Dense<ALT_T>)
         // Dense -> Dense (returning a reference)                      
         return static_cast<Decay<ALT_T>&>(element);
      else if constexpr (CT::Dense<T> && CT::Sparse<ALT_T>)
         // Dense -> Sparse (returning a pointer)                       
         return static_cast<Decay<ALT_T>*>(&element);
      else if constexpr (CT::Sparse<T> && CT::Dense<ALT_T>)
         // Sparse -> Dense (returning a reference)                     
         return static_cast<Decay<ALT_T>&>(*element.mPointer);
      else
         // Sparse -> Sparse (returning a reference to pointer)         
         return static_cast<Decay<ALT_T>*&>(element.mPointer);
   }

   /// Access typed dense elements by index                                   
   ///   @param idx - the index to get                                        
   ///   @return a reference to the element                                   
   TEMPLATE()
   template<CT::Index IDX>
   decltype(auto) TAny<T>::operator [] (const IDX& index) const {
      const auto offset = SimplifyIndex<T>(index);
      return TAny<T>::GetRaw()[offset];
   }

   TEMPLATE()
   template<CT::Index IDX>
   decltype(auto) TAny<T>::operator [] (const IDX& index) {
      const auto offset = SimplifyIndex<T>(index);
      return TAny<T>::GetRaw()[offset];
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TEMPLATE()
   decltype(auto) TAny<T>::Last() {
      LANGULUS_ASSUME(UserAssumes, mCount, "Can't get last index");
      return Get<T>(mCount - 1);
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TEMPLATE()
   decltype(auto) TAny<T>::Last() const {
      LANGULUS_ASSUME(UserAssumes, mCount, "Can't get last index");
      return Get<T>(mCount - 1);
   }
   
   /// Templated Any containers are always typed                              
   TEMPLATE()
   constexpr bool TAny<T>::IsUntyped() const noexcept {
      return false;
   }
   
   /// Templated Any containers are always type-constrained                   
   TEMPLATE()
   constexpr bool TAny<T>::IsTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if contained type is abstract                                    
   /// This is a statically optimized alternative to Block::IsAbstract        
   TEMPLATE()
   constexpr bool TAny<T>::IsAbstract() const noexcept {
      return CT::Abstract<T>;
   }
   
   /// Check if contained type is default-constructible                       
   /// This is a statically optimized alternative to Block::IsDefaultable     
   TEMPLATE()
   constexpr bool TAny<T>::IsDefaultable() const noexcept {
      return CT::Defaultable<T>;
   }
   
   /// Check if contained type is deep                                        
   /// This is a statically optimized alternative to Block::IsDeep            
   ///   @return true if this container contains deep items                   
   TEMPLATE()
   constexpr bool TAny<T>::IsDeep() const noexcept {
      // Sparse types are never considered deep, but when contained,    
      // it's safe to erase that aspect                                 
      return CT::Deep<Decay<T>>;
   }

   /// Check if the contained type is a pointer                               
   /// This is a statically optimized alternative to Block::IsSparse          
   ///   @return true if container contains pointers                          
   TEMPLATE()
   constexpr bool TAny<T>::IsSparse() const noexcept {
      return CT::Sparse<T>;
   }

   /// Check if the contained type is not a pointer                           
   /// This is a statically optimized alternative to Block::IsDense           
   ///   @return true if container contains sequential data                   
   TEMPLATE()
   constexpr bool TAny<T>::IsDense() const noexcept {
      return CT::Dense<T>;
   }

   /// Check if block contains POD items - if so, it's safe to directly copy  
   /// raw memory from container. Note, that this doesn't only consider the   
   /// standard c++ type traits, like trivially_constructible. You also need  
   /// to explicitly reflect your type with LANGULUS(POD) true;               
   /// This gives a lot more control over your code                           
   /// This is a statically optimized alternative to Block::IsPOD             
   ///   @return true if contained data is plain old data                     
   TEMPLATE()
   constexpr bool TAny<T>::IsPOD() const noexcept {
      return CT::POD<T>;
   }

   /// Check if block contains resolvable items, that is, items that have a   
   /// GetBlock() function, that can be used to represent themselves as their 
   /// most concretely typed block                                            
   /// This is a statically optimized alternative to Block::IsResolvable      
   ///   @return true if contained data can be resolved on element basis      
   TEMPLATE()
   constexpr bool TAny<T>::IsResolvable() const noexcept {
      return CT::Resolvable<T>;
   }

   /// Check if block data can be safely set to zero bytes                    
   /// This is tied to LANGULUS(NULLIFIABLE) reflection parameter             
   /// This is a statically optimized alternative to Block::IsNullifiable     
   ///   @return true if contained data can be memset(0) safely               
   TEMPLATE()
   constexpr bool TAny<T>::IsNullifiable() const noexcept {
      return CT::Nullifiable<T>;
   }

   /// Get the size of a single contained element, in bytes                   
   /// This is a statically optimized alternative to Block::GetStride         
   ///   @return the number of bytes a single element contains                
   TEMPLATE()
   constexpr Size TAny<T>::GetStride() const noexcept {
      if constexpr (CT::Dense<T>)
         return sizeof(T);
      else
         return sizeof(KnownPointer);
   }
   
   /// Get the size of all elements, in bytes                                 
   ///   @return the total amount of initialized bytes                        
   TEMPLATE()
   constexpr Size TAny<T>::GetByteSize() const noexcept {
      return GetStride() * mCount;
   }

   /// Copy-insert item(s) at an index                                        
   ///   @attention assumes index is in the container's limits, if simple     
   ///   @tparam KEEP - whether or not to reference inserted data             
   ///   @tparam IDX - type of indexing to use (deducible)                    
   ///   @param start - pointer to the first element to insert                
   ///   @param end - pointer to the end of elements to insert                
   ///   @param index - the index to insert at                                
   ///   @return number of inserted items                                     
   TEMPLATE()
   template<bool KEEP, CT::Index IDX>
   Count TAny<T>::InsertAt(const T* start, const T* end, const IDX& index) {
      const auto offset = SimplifyIndex<T>(index);
      const auto count = end - start;
      Allocate<false>(mCount + count);

      if (offset < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
            "Inserting elements to memory block, used from multiple places, "
            "requires memory to move");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any overlap                                        
         CropInner(offset + count, 0, mCount - offset)
            .template CallKnownMoveConstructors<T, false, true>(
               mCount - offset,
               CropInner(offset, mCount - offset, mCount - offset)
            );
      }

      InsertInner<KEEP, T>(start, end, offset);
      return count;
   }

   /// Move-insert an item at an index                                        
   ///   @attention assumes index is in the container's limits, if simple     
   ///   @tparam KEEP - whether or not to reference inserted data             
   ///   @tparam IDX - type of indexing to use (deducible)                    
   ///   @param item - the item to move in                                    
   ///   @param index - the index to insert at                                
   ///   @return number of inserted items                                     
   TEMPLATE()
   template<bool KEEP, CT::Index IDX>
   Count TAny<T>::InsertAt(T&& item, const IDX& index) {
      const auto offset = SimplifyIndex<T>(index);
      Allocate<false>(mCount + 1);

      if (offset < mCount) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
            "Inserting elements to memory block, used from multiple places, "
            "requires memory to move");
         
         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any overlap                                        
         CropInner(offset + 1, 0, mCount - offset)
            .template CallKnownMoveConstructors<T, false, true>(
               mCount - offset,
               CropInner(offset, mCount - offset, mCount - offset)
            );
      }

      InsertInner<KEEP, T>(Move(item), offset);
      return 1;
   }

   /// Copy-insert elements either at the start or the end                    
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam KEEP - whether to reference data on copy                     
   ///   @param start - pointer to the first item                             
   ///   @param end - pointer to the end of items                             
   ///   @return number of inserted elements                                  
   TEMPLATE()
   template<Index INDEX, bool KEEP>
   Count TAny<T>::Insert(const T* start, const T* end) {
      static_assert(CT::Sparse<T> || CT::Mutable<T>,
         "Can't copy-insert into container of constant elements");
      static_assert(INDEX == IndexFront || INDEX == IndexBack,
         "Invalid index provided; use either IndexBack "
         "or IndexFront, or Block::InsertAt to insert at an offset");

      // Allocate                                                       
      const auto count = end - start;
      Allocate<false>(mCount + count);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
            "Inserting elements to memory block, used from multiple places, "
            "requires memory to move");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any overlap                                        
         CropInner(count, 0, mCount)
            .template CallKnownMoveConstructors<T, false, true>(
               mCount, CropInner(0, mCount, mCount)
            );

         InsertInner<KEEP>(start, end, 0);
      }
      else InsertInner<KEEP>(start, end, mCount);

      return count;
   }

   /// Move-insert an element at the start or the end                         
   ///   @tparam INDEX - use IndexBack or IndexFront to append accordingly    
   ///   @tparam KEEP - whether to completely reset source after move         
   ///   @param item - item to move int                                       
   ///   @return 1 if element was pushed                                      
   TEMPLATE()
   template<Index INDEX, bool KEEP>
   Count TAny<T>::Insert(T&& item) {
      static_assert(CT::Sparse<T> || CT::Mutable<T>,
         "Can't copy-insert into container of constant elements");
      static_assert(INDEX == IndexFront || INDEX == IndexBack,
         "Invalid index provided; use either IndexBack "
         "or IndexFront, or Block::InsertAt to insert at an offset");

      // Allocate                                                       
      Allocate<false>(mCount + 1);

      if constexpr (INDEX == IndexFront) {
         // Move memory if required                                     
         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
            "Inserting elements to memory block, used from multiple places, "
            "requires memory to move");

         // We're moving to the right, so make sure we do it in reverse 
         // to avoid any overlap                                        
         CropInner(1, 0, mCount)
            .template CallKnownMoveConstructors<T, false, true>(
               mCount, CropInner(0, mCount, mCount)
            );

         InsertInner<KEEP>(Move(item), 0);
      }
      else InsertInner<KEEP>(Move(item), mCount);

      return 1;
   }

   /// Emplace a single item at the given index                               
   ///   @tparam IDX - the type of indexing used                              
   ///   @tparam A... - arguments for the element's construction              
   ///   @param at - the index to emplace at                                  
   ///   @param arguments... - the arguments for the element's constructor    
   ///   @return 1 if the item was emplaced, 0 if not                         
   TEMPLATE()
   template<CT::Index IDX, class... A>
   Count TAny<T>::EmplaceAt(const IDX& at, A&&... arguments) {
      if constexpr (CT::Dense<T>)
         return InsertAt<false, IDX>(T {arguments...}, at);
      else {
         // Allocate a new element, and push a pointer to it            
         using DT = Decay<T>;
         auto Tallocation = Inner::Allocator::Allocate(sizeof(DT));
         T Tptr = reinterpret_cast<T>(Tallocation->GetBlockStart());
         new (Tptr) DT {arguments...};
         return InsertAt<false, IDX>(Move(Tptr), at);
      }
   }

   /// Emplace a single item at front or back                                 
   ///   @tparam INDEX - the index to emplace at, either front or back        
   ///   @tparam A... - arguments for the element's construction              
   ///   @param arguments... - the arguments for the element's constructor    
   ///   @return 1 if the item was emplaced, 0 if not                         
   TEMPLATE()
   template<Index INDEX, class... A>
   Count TAny<T>::Emplace(A&&...arguments) {
      if constexpr (CT::Dense<T>)
         return Insert<INDEX, false>(T {arguments...});
      else {
         // Allocate a new element, and push a pointer to it            
         using DT = Decay<T>;
         auto Tallocation = Inner::Allocator::Allocate(sizeof(DT));
         T Tptr = reinterpret_cast<T>(Tallocation->GetBlockStart());
         new (Tptr) DT {arguments...};
         return Insert<INDEX, false>(Move(Tptr));
      }
   }

   /// Push data at the back by copy-construction                             
   ///   @param other - the item to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator << (const T& other) {
      Insert<IndexBack>(&other, &other + 1);
      return *this;
   }

   /// Push data at the back by move-construction                             
   ///   @param other - the item to move                                      
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator << (T&& other) {
      Insert<IndexBack>(Forward<T>(other));
      return *this;
   }

   /// Push data at the back by copy-construction, but don't reference the    
   /// new element, because it's disowned                                     
   ///   @param other - the item to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator << (Disowned<T>&& other) {
      Insert<IndexBack, false>(&other.mValue, &other.mValue + 1);
      return *this;
   }

   /// Push data at the back by move-construction, but don't fully reset the  
   /// source, because it's abandoned                                         
   ///   @param other - the item to move                                      
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator << (Abandoned<T>&& other) {
      Insert<IndexBack, false>(Move(other.mValue));
      return *this;
   }

   /// Push data at the front by copy-construction                            
   ///   @param other - the item to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator >> (const T& other) {
      Insert<IndexFront>(&other, &other + 1);
      return *this;
   }

   /// Push data at the front by move-construction                            
   ///   @param other - the item to move                                      
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator >> (T&& other) {
      Insert<IndexFront>(Forward<T>(other));
      return *this;
   }

   /// Push data at the front by copy-construction, but don't reference the   
   /// new element, because it's disowned                                     
   ///   @param other - the item to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator >> (Disowned<T>&& other) {
      Insert<IndexFront, false>(&other.mValue, &other.mValue + 1);
      return *this;
   }

   /// Push data at the front by move-construction, but don't fully reset the 
   /// source, because it's abandoned                                         
   ///   @param other - the item to move                                      
   ///   @return a reference to this container for chaining                   
   TEMPLATE()
   TAny<T>& TAny<T>::operator >> (Abandoned<T>&& other) {
      Insert<IndexFront, false>(Move(other.mValue));
      return *this;
   }

   /// Copy-insert elements that are not found, at an index                   
   ///   @attention assumes index is in container's limits, if simple         
   ///   @tparam KEEP - whether or not to reference inserted data             
   ///   @tparam IDX - type for indexing (deducible)                          
   ///   @param start - pointer to the first element to insert                
   ///   @param end - pointer to the end of elements to insert                
   ///   @param index - the index to insert at                                
   ///   @return the number of inserted items                                 
   TEMPLATE()
   template<bool KEEP, CT::Index IDX>
   Count TAny<T>::MergeAt(const T* start, const T* end, const IDX& index) {
      return Block::MergeAt<TAny, KEEP, true, T>(start, end, index);
   }

   /// Move-insert element, if not found, at an index                         
   ///   @attention assumes index is in container's limits, if simple         
   ///   @tparam KEEP - whether or not to reference inserted data             
   ///   @tparam IDX - type for indexing (deducible)                          
   ///   @param item - the item to find and push                              
   ///   @param index - the index to insert at                                
   ///   @return the number of inserted items                                 
   TEMPLATE()
   template<bool KEEP, CT::Index IDX>
   Count TAny<T>::MergeAt(T&& item, const IDX& index) {
      return Block::MergeAt<TAny, KEEP, true, T>(Forward<T>(item), index);
   }

   /// Copy-construct element at the back, if element is not found            
   ///   @param other - the element to shallow-copy                           
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator <<= (const T& other) {
      Merge<IndexBack>(&other, &other + 1);
      return *this;
   }

   /// Move-construct element at the back, if element is not found            
   ///   @param other - the element to move                                   
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator <<= (T&& other) {
      Merge<IndexBack>(Forward<T>(other));
      return *this;
   }

   /// Copy-construct element at the back, if element is not found            
   /// The element's contents won't be referenced, because it is disowned     
   ///   @param other - the element to shallow-copy                           
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator <<= (Disowned<T>&& other) {
      Merge<IndexBack, false>(&other.mValue, &other.mValue + 1);
      return *this;
   }

   /// Move-construct element at the back, if element is not found            
   /// The element won't be fully reset, because it's abandoned               
   ///   @param other - the element to move                                   
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator <<= (Abandoned<T>&& other) {
      Merge<IndexBack, false>(Move(other.mValue));
      return *this;
   }

   /// Copy-construct element at the front, if element is not found           
   ///   @param other - the element to shallow-copy                           
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator >>= (const T& other) {
      Merge<IndexFront>(&other, &other + 1);
      return *this;
   }

   /// Move-construct element at the front, if element is not found           
   ///   @param other - the element to move                                   
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator >>= (T&& other) {
      Merge<IndexFront>(Forward<T>(other));
      return *this;
   }

   /// Copy-construct element at the front, if element is not found           
   /// The element's contents won't be referenced, because it is disowned     
   ///   @param other - the element to shallow-copy                           
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator >>= (Disowned<T>&& other) {
      Merge<IndexFront, false>(&other.mValue, &other.mValue + 1);
      return *this;
   }

   /// Move-construct element at the front, if element is not found           
   /// The element won't be fully reset, because it's abandoned               
   ///   @param other - the element to move                                   
   ///   @return a reference to this container                                
   TEMPLATE()
   TAny<T>& TAny<T>::operator >>= (Abandoned<T>&& other) {
      Merge<IndexFront, false>(Move(other.mValue));
      return *this;
   }

   /// Find element(s) index inside container                                 
   ///   @tparam REVERSE - whether to search in reverse order                 
   ///   @param item - the item to search for                                 
   ///   @return the index of the found item, or IndexNone if none found      
   TEMPLATE()
   template<bool REVERSE, bool BY_ADDRESS_ONLY, CT::Data ALT_T>
   Index TAny<T>::Find(const ALT_T& item) const {
      if constexpr (CT::Same<T, ALT_T>) {
         const TypeInner* start;
         const TypeInner* end;

         if constexpr (REVERSE) {
            start = GetRawEnd() - 1;
            end = start - mCount;
         }
         else {
            start = GetRaw();
            end = start + mCount;
         }

         while (start != end) {
            if constexpr (BY_ADDRESS_ONLY || !CT::Comparable<T, T>) {
               if constexpr (CT::Sparse<T>) {
                  if (DenseCast(start) == SparseCast(item))
                     return start - GetRaw();
               }
               else if (start == SparseCast(item))
                  return start - GetRaw();
            }
            else {
               if constexpr (CT::Sparse<T>) {
                  if (DenseCast(start) == SparseCast(item) || *DenseCast(start) == DenseCast(item))
                     return start - GetRaw();
               }
               else if (start == SparseCast(item) || *start == DenseCast(item))
                  return start - GetRaw();
            }

            if constexpr (REVERSE)
               --start;
            else
               ++start;
         }
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }

   /// Remove matching items by value                                         
   ///   @tparam REVERSE - whether to search in reverse order                 
   ///   @param item - the item to search for to remove                       
   ///   @return the number of removed items                                  
   TEMPLATE()
   template<bool REVERSE, bool BY_ADDRESS_ONLY, CT::Data ALT_T>
   Count TAny<T>::RemoveValue(const ALT_T& item) {
      const auto found = Find<REVERSE, BY_ADDRESS_ONLY>(item);
      if (found)
         return RemoveIndex(found.GetOffset(), 1);
      return 0;
   }

   /// Remove sequential raw indices in a given range                         
   ///   @attention assumes starter + count <= mCount                         
   ///   @param starter - simple index to start removing from                 
   ///   @param count - number of elements to remove                          
   ///   @return the number of removed elements                               
   TEMPLATE()
   Count TAny<T>::RemoveIndex(const Offset& starter, const Count& count) {
      LANGULUS_ASSUME(UserAssumes, starter + count <= mCount,
         "Index out of range");

      const auto ender = starter + count;
      if constexpr (CT::Sparse<T> || CT::POD<T>) {
         if (ender == mCount) {
            // If data is POD and elements are on the back, we can      
            // get around constantness and staticness, by simply        
            // truncating the count without any reprecussions           
            // We can completely skip destroying POD things             
            mCount = starter;
            return count;
         }

         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
            "Removing elements from memory block, used from multiple places, "
            "requires memory to move");
         LANGULUS_ASSERT(IsMutable(), Except::Access,
            "Attempting to remove from constant container");
         LANGULUS_ASSERT(!IsStatic(), Except::Access,
            "Attempting to remove from static container");

         MoveMemory(GetRaw() + ender, GetRaw() + starter, sizeof(TypeInner) * (mCount - ender));
         mCount -= count;
         return count;
      }
      else {
         if (IsStatic() && ender == mCount) {
            // If data is static and elements are on the back, we can   
            // get around constantness and staticness, by simply        
            // truncating the count without any reprecussions           
            // We can't destroy static element anyways                  
            mCount = starter;
            return count;
         }

         LANGULUS_ASSERT(GetUses() == 1, Except::Move,
            "Removing elements from memory block, used from multiple places, "
            "requires memory to move");
         LANGULUS_ASSERT(IsMutable(), Except::Access,
            "Attempting to remove from constant container");
         LANGULUS_ASSERT(!IsStatic(), Except::Access,
            "Attempting to remove from static container");

         // Call the destructors on the correct region                  
         CropInner(starter, count, count)
            .template CallKnownDestructors<T>();

         if (ender < mCount) {
            // Fill gap	if any by invoking move constructions           
            // Moving to the left, so no overlap possible if forward    
            const auto remains = mCount - ender;
            CropInner(starter, 0, remains)
               .template CallKnownMoveConstructors<T, false>(
                  remains, CropInner(ender, remains, remains)
               );
         }

         mCount -= count;
         return count;
      }
   }

   /// Sort the pack                                                          
   TEMPLATE()
   template<bool ASCEND>
   void TAny<T>::Sort() {
      if constexpr (CT::Sortable<T>)
         Any::Sort<T, ASCEND>();
      else LANGULUS_ERROR("Can't sort container - T is not sortable");
   }

   /// Remove elements on the back                                            
   TEMPLATE()
   TAny<T>& TAny<T>::Trim(const Count& count) {
      Any::Trim(count);
      return *this;
   }

   /// Swap two elements                                                      
   ///   @param from - the first element                                      
   ///   @param to - the second element                                       
   TEMPLATE()
   void TAny<T>::Swap(const Offset& from, const Offset& to) {
      Any::Swap<T>(from, to);
   }

   /// Swap two elements using special indices                                
   ///   @param from - the first element                                      
   ///   @param to - the second element                                       
   TEMPLATE()
   void TAny<T>::Swap(const Index& from, const Index& to) {
      Any::Swap<T>(from, to);
   }

   /// Clone container array into a new owned memory block                    
   /// If we have jurisdiction, the memory won't move at all                  
   TEMPLATE()
   void TAny<T>::TakeAuthority() {
      if (mEntry)
         return;

      operator = (Clone());
   }

   /// Get a constant part of this container                                  
   ///   @tparam WRAPPER - the container to use for the part                  
   ///                     use Block for unreferenced container               
   ///   @return a container that represents the cropped part                 
   TEMPLATE()
   template<CT::Block WRAPPER>
   WRAPPER TAny<T>::Crop(const Offset& start, const Count& count) const {
      auto result = const_cast<TAny*>(this)->Crop<WRAPPER>(start, count);
      result.MakeConst();
      return Abandon(result);
   }
   
   /// Get a part of this container                                           
   ///   @tparam WRAPPER - the container to use for the part                  
   ///                     use Block for unreferenced container               
   ///   @return a container that represents the cropped part                 
   TEMPLATE()
   template<CT::Block WRAPPER>
   WRAPPER TAny<T>::Crop(const Offset& start, const Count& count) {
      LANGULUS_ASSUME(DevAssumes, start + count <= mCount, "Out of limits");

      if (count == 0) {
         WRAPPER result {Disown(*this)};
         result.ResetMemory();
         return Abandon(result);
      }

      WRAPPER result {*this};
      result.MakeStatic();
      result.mCount = result.mReserved = count;
      result.mRaw += start * GetStride();
      return Abandon(result);
   }
   
   /// Get a size based on reflected allocation page and count (unsafe)       
   ///   @param count - the number of elements to request                     
   ///   @returns both the provided byte size and reserved count              
   TEMPLATE()
   auto TAny<T>::RequestSize(const Count& count) const noexcept {
      if constexpr (CT::Sparse<T>) {
         RTTI::AllocationRequest result;
         const auto requested = sizeof(KnownPointer) * count;
         result.mByteSize = requested > Alignment ? Roof2(requested) : Alignment;
         result.mElementCount = result.mByteSize / sizeof(KnownPointer);
         return result;
      }
      else return mType->RequestSize(sizeof(T) * count);
   }

   /// Allocate a number of elements, relying on the type of the container    
   ///   @tparam CREATE - true to call constructors and initialize count      
   ///   @tparam SETSIZE - true to set size without calling any constructors  
   ///   @param elements - number of elements to allocate                     
   TEMPLATE()
   template<bool CREATE, bool SETSIZE>
   void TAny<T>::Allocate(Count elements) {
      static_assert(!CREATE || CT::Sparse<T> || !CT::Abstract<T>,
         "Can't allocate and default-construct abstract items in dense TAny");

      // Allocate/reallocate                                            
      const auto request = RequestSize(elements);
      if (mEntry) {
         if (mReserved >= elements) {
            if (mCount > elements) {
               // Destroy back entries on smaller allocation            
               RemoveIndex(elements, mCount - elements);
               return;
            }

            // Required memory is already available                     
            if constexpr (CREATE) {
               // But is not yet initialized, so initialize it          
               if (mCount < elements) {
                  const auto count = elements - mCount;
                  CropInner(mCount, count, count)
                     .template CallKnownDefaultConstructors<T>(count);
               }
            }

            if constexpr (CREATE || SETSIZE)
               mCount = elements;
            return;
         }

         // Reallocate                                                  
         Block previousBlock = *this;
         if (mEntry->GetUses() == 1) {
            // Memory is used only once and it is safe to move it       
            // Make note, that Allocator::Reallocate doesn't copy       
            // anything, it doesn't use realloc for various reasons, so 
            // we still have to call move construction for all elements 
            // if entry moved (enabling MANAGED_MEMORY feature          
            // significantly reduces the possiblity for a move)         
            // Also, make sure to free the previous mEntry if moved     
            mEntry = Inner::Allocator::Reallocate(request.mByteSize, mEntry);
            LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");

            if (mEntry != previousBlock.mEntry) {
               // Memory moved, and we should call move-construction    
               // We're moving to new memory, so no reverse required    
               mRaw = mEntry->GetBlockStart();
               CallKnownMoveConstructors<T, false>(previousBlock.mCount, previousBlock);
            }
         }
         else {
            // Memory is used from multiple locations, and we must      
            // copy the memory for this block - we can't move it!       
            mEntry = Inner::Allocator::Allocate(request.mByteSize);
            LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");
            mRaw = mEntry->GetBlockStart();
            CallKnownCopyConstructors<T>(previousBlock.mCount, previousBlock);
         }

         if constexpr (CREATE) {
            // Default-construct the rest                               
            const auto count = elements - mCount;
            CropInner(mCount, count, count)
               .template CallKnownDefaultConstructors<T>(count);
         }
      }
      else {
         // Allocate a fresh set of elements                            
         mEntry = Inner::Allocator::Allocate(request.mByteSize);
         LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");
         mRaw = mEntry->GetBlockStart();

         if constexpr (CREATE) {
            // Default-construct everything                             
            CropInner(mCount, elements, elements)
               .template CallKnownDefaultConstructors<T>(elements);
         }
      }
      
      if constexpr (CREATE || SETSIZE)
         mCount = elements;
      mReserved = request.mElementCount;
   }
   
   /// Extend the container and return the new part                           
   ///   @tparam WRAPPER - the container to use for the extended part         
   ///   @return a container that represents the extended part                
   TEMPLATE()
   template<CT::Block WRAPPER>
   WRAPPER TAny<T>::Extend(const Count& count) {
      if (IsStatic())
         return {};

      const auto newCount = mCount + count;
      if (mEntry && newCount > mReserved) {
         // Allocate more space                                         
         mEntry = Inner::Allocator::Reallocate(GetStride() * newCount, mEntry);
         LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");
         mRaw = mEntry->GetBlockStart();
         mReserved = newCount;
      }

      // Initialize new elements                                        
      auto extension = CropInner(mCount, count, count);
      extension.template CallKnownDefaultConstructors<T>(count);
      extension.MakeStatic();

      // Return the extension                                           
      mCount += count;
      WRAPPER result;
      static_cast<Block&>(result) = extension;
      if constexpr (!CT::Same<WRAPPER, Block>)
         mEntry->Keep();
      return Abandon(result);
   }
   
   /// Compare with another TAny, order matters                               
   ///   @param other - container to compare with                             
   ///   @return true if both containers match completely                     
   TEMPLATE()
   bool TAny<T>::Compare(const TAny& other) const noexcept {
      if (mRaw == other.mRaw)
         return mCount == other.mCount;
      else if (mCount != other.mCount)
         return false;

      auto t1 = GetRaw();
      auto t2 = other.GetRaw();
      const auto t1end = t1 + mCount;
      if constexpr (CT::Dense<T> && CT::Comparable<T, T>) {
         while (t1 < t1end && *t1 == *t2) {
            ++t1; ++t2;
         }
      }
      else if constexpr (CT::Comparable<T, T>) {
         while (t1 < t1end && (t1 == t2 || **t1 == **t2)) {
            ++t1; ++t2;
         }
      }
      else {
         while (t1 < t1end && t1 == t2) {
            ++t1; ++t2;
         }
      }

      return static_cast<Count>(t1 - GetRaw()) == mCount;
   }

   /// Compare with another container of the same type                        
   ///   @param other - the container to compare with                         
   ///   @return true if both containers are identical                        
   TEMPLATE()
   template<CT::Data ALT_T>
   bool TAny<T>::operator == (const TAny<ALT_T>& other) const noexcept {
      if constexpr (CT::Same<T, ALT_T>)
         return Compare(other);
      else
         return false;
   }

   /// Compare with block of unknown type                                     
   ///   @param other - the block to compare with                             
   ///   @return true if both containers are identical                        
   TEMPLATE()
   bool TAny<T>::operator == (const Any& other) const noexcept {
      static_assert(sizeof(Block) == sizeof(TAny), "Binary incompatiblity");
      if (!Is(other.GetType()))
         return false;
      return Compare(reinterpret_cast<const TAny&>(other));
   }

   /// Compare loosely with another TAny, ignoring case                       
   /// This function applies only if T is character                           
   ///   @param other - text to compare with                                  
   ///   @return true if both containers match loosely                        
   TEMPLATE()
   bool TAny<T>::CompareLoose(const TAny& other) const noexcept requires CT::Character<T> {
      if (mRaw == other.mRaw)
         return mCount == other.mCount;
      else if (mCount != other.mCount)
         return false;

      auto t1 = GetRaw();
      auto t2 = other.GetRaw();
      while (t1 < GetRawEnd() && ::std::tolower(*t1) == ::std::tolower(*(t2++)))
         ++t1;
      return (t1 - GetRaw()) == mCount;
   }

   /// Count how many consecutive elements match in two containers            
   ///   @param other - container to compare with                             
   ///   @return the number of matching items                                 
   TEMPLATE()
   Count TAny<T>::Matches(const TAny& other) const noexcept {
      if (mRaw == other.mRaw)
         return ::std::min(mCount, other.mCount);

      auto t1 = GetRaw();
      auto t2 = other.GetRaw();
      const auto t1end = GetRawEnd();
      const auto t2end = other.GetRawEnd();
      while (t1 != t1end && t2 != t2end && *t1 == *(t2++))
         ++t1;

      /*
      __m128i first = _mm_loadu_si128( reinterpret_cast<__m128i*>( &arr1 ) );
      __m128i second = _mm_loadu_si128( reinterpret_cast<__m128i*>( &arr2 ) );
      return std::popcount(_mm_movemask_epi8( _mm_cmpeq_epi8( first, second ) ));
      */

      return t1 - GetRaw();
   }

   /// Compare loosely with another, ignoring upper-case                      
   /// Count how many consecutive letters match in two strings                
   ///   @param other - text to compare with                                  
   ///   @return the number of matching symbols                               
   TEMPLATE()
   Count TAny<T>::MatchesLoose(const TAny& other) const noexcept requires CT::Character<T> {
      if (mRaw == other.mRaw)
         return ::std::min(mCount, other.mCount);

      auto t1 = GetRaw();
      auto t2 = other.GetRaw();
      const auto t1end = GetRawEnd();
      const auto t2end = other.GetRawEnd();
      while (t1 != t1end && t2 != t2end && ::std::tolower(*t1) == ::std::tolower(*(t2++)))
         ++t1;
      return t1 - GetRaw();
   }



   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// Copy-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   TEMPLATE()
   TAny<T> TAny<T>::operator + (const TAny& rhs) const {
      return Concatenate<TAny, true>(rhs);
   }

   /// Move-concatenate with another TAny                                     
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   TEMPLATE()
   TAny<T> TAny<T>::operator + (TAny&& rhs) const {
      return Concatenate<TAny, true>(Forward<T>(rhs));
   }

   /// Disown-concatenate with another TAny                                   
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   TEMPLATE()
   TAny<T> TAny<T>::operator + (Disowned<TAny>&& rhs) const {
      return Concatenate<TAny, false>(rhs.mValue);
   }

   /// Abandon-concatenate with another TAny                                  
   ///   @param rhs - the right operand                                       
   ///   @return the combined container                                       
   TEMPLATE()
   TAny<T> TAny<T>::operator + (Abandoned<TAny>&& rhs) const {
      return Concatenate<TAny, false>(Abandoned(Forward<T>(rhs)));
   }

   /// Destructive copy-concatenate with another TAny                         
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   TEMPLATE()
   TAny<T>& TAny<T>::operator += (const TAny& rhs) {
      InsertBlock(rhs);
      return *this;
   }

   /// Destructive move-concatenate with any deep type                        
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   TEMPLATE()
   TAny<T>& TAny<T>::operator += (TAny&& rhs) {
      InsertBlock(Forward<TAny>(rhs));
      return *this;
   }

   /// Destructive disown-concatenate with any deep type                      
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   TEMPLATE()
   TAny<T>& TAny<T>::operator += (Disowned<TAny>&& rhs) {
      InsertBlock(rhs.Forward());
      return *this;
   }

   /// Destructive abandon-concatenate with any deep type                     
   ///   @param rhs - the right operand                                       
   ///   @return a reference to this modified container                       
   TEMPLATE()
   TAny<T>& TAny<T>::operator += (Abandoned<TAny>&& rhs) {
      InsertBlock(rhs.Forward());
      return *this;
   }





   ///                                                                        
   ///   Known pointer implementation                                         
   ///                                                                        
   #define KNOWNPOINTER() TAny<T>::KnownPointer
   
   /// Copy-construct a pointer - references the block                        
   ///   @param other - the pointer to reference                              
   TEMPLATE()
   KNOWNPOINTER()::KnownPointer(const KnownPointer& other) noexcept
      : mPointer {other.mPointer}
      , mEntry {other.mEntry} {
      if (mEntry)
         mEntry->Keep();
   }

   /// Move-construct a pointer                                               
   ///   @param other - the pointer to move                                   
   TEMPLATE()
   KNOWNPOINTER()::KnownPointer(KnownPointer&& other) noexcept
      : mPointer {other.mPointer}
      , mEntry {other.mEntry} {
      other.mPointer = nullptr;
      other.mEntry = nullptr;
   }

   /// Copy-construct a pointer, without referencing it                       
   ///   @param other - the pointer to copy                                   
   TEMPLATE()
   KNOWNPOINTER()::KnownPointer(Disowned<KnownPointer>&& other) noexcept
      : mPointer {other.mValue.mPointer}
      , mEntry {nullptr} {}

   /// Move-construct a pointer, minimally resetting the source               
   ///   @param other - the pointer to move                                   
   TEMPLATE()
   KNOWNPOINTER()::KnownPointer(Abandoned<KnownPointer>&& other) noexcept
      : mPointer {other.mValue.mPointer}
      , mEntry {other.mValue.mEntry} {
      other.mValue.mEntry = nullptr;
   }

   /// Find and reference a pointer                                           
   ///   @param pointer - the pointer to reference                            
   TEMPLATE()
   KNOWNPOINTER()::KnownPointer(const T& pointer)
      : mPointer {pointer} {
      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         // If we're using managed memory, we can search if the pointer 
         // is owned by us, and get its block                           
         // This has no point when the pointer is a meta (optimization) 
         if constexpr (!CT::Meta<T>) {
            mEntry = Inner::Allocator::Find(MetaData::Of<Decay<T>>(), pointer);
            if (mEntry)
               mEntry->Keep();
         }
      #endif
   }

   /// Copy a disowned pointer, no search for block will be performed         
   ///   @param pointer - the pointer to copy                                 
   TEMPLATE()
   KNOWNPOINTER()::KnownPointer(Disowned<T>&& pointer) noexcept
      : mPointer {pointer.mValue} {}

   /// Dereference (and eventually destroy)                                   
   TEMPLATE()
   KNOWNPOINTER()::~KnownPointer() {
      Free();
   }

   /// Free the contents of the known pointer (inner function)                
   ///   @attention doesn't reset pointers                                    
   TEMPLATE()
   void KNOWNPOINTER()::Free() {
      if (!mEntry)
         return;

      if (mEntry->GetUses() == 1) {
         if constexpr (!CT::POD<T> && CT::Destroyable<T>) {
            using DT = Decay<T>;
            mPointer->~DT();
         }
         Inner::Allocator::Deallocate(mEntry);
      }
      else mEntry->Free();
   }

   /// Copy-assign a known pointer, dereferencing old and referencing new     
   ///   @param rhs - the new pointer                                         
   TEMPLATE()
   typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (const KnownPointer& rhs) noexcept {
      Free();
      new (this) KnownPointer {rhs};
      return *this;
   }

   /// Move-assign a known pointer, dereferencing old and moving new          
   ///   @param rhs - the new pointer                                         
   TEMPLATE()
   typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (KnownPointer&& rhs) noexcept {
      Free();
      new (this) KnownPointer {Forward<KnownPointer>(rhs)};
      return *this;
   }

   /// Copy-assign a known pointer, dereferencing old but not referencing new 
   ///   @param rhs - the new pointer                                         
   TEMPLATE()
   typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (Disowned<KnownPointer>&& rhs) noexcept {
      Free();
      new (this) KnownPointer {rhs.Forward()};
      return *this;
   }

   /// Move-assign a known pointer, dereferencing old and moving new          
   ///   @param rhs - the new pointer                                         
   TEMPLATE()
   typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (Abandoned<KnownPointer>&& rhs) noexcept {
      Free();
      new (this) KnownPointer {rhs.Forward()};
      return *this;
   }

   /// Copy-assign a dangling pointer, finding its block and referencing      
   ///   @param rhs - pointer to copy and reference                           
   TEMPLATE()
   typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (const T& rhs) {
      Free();
      new (this) KnownPointer {rhs};
      return *this;
   }

   /// Copy-assign a dangling pointer, but don't reference it                 
   ///   @param rhs - pointer to copy                                         
   TEMPLATE()
   typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (Disowned<T>&& rhs) {
      Free();
      new (this) KnownPointer {rhs.Forward()};
      return *this;
   }

   /// Reset the known pointer                                                
   TEMPLATE()
   typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (::std::nullptr_t) {
      Free();
      mPointer = nullptr;
      mEntry = nullptr;
      return *this;
   }

   /// Compare two known pointers                                             
   ///   @param rhs - the pointer to compare against                          
   ///   @return true if pointers/values match                                
   TEMPLATE()
   bool KNOWNPOINTER()::operator == (const KNOWNPOINTER()& rhs) const noexcept {
      return mPointer == rhs.mPointer;
   }

   /// Get hash of the pointer inside                                         
   ///   @return the hash                                                     
   TEMPLATE()
   Hash KNOWNPOINTER()::GetHash() const {
      if (!mPointer)
         return {};
      return HashData(*mPointer);
   }

   /// Pointer dereferencing (const)                                          
   ///   @attention assumes contained pointer is valid                        
   TEMPLATE()
   auto KNOWNPOINTER()::operator -> () const {
      LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
      return mPointer;
   }

   /// Pointer dereferencing                                                  
   ///   @attention assumes contained pointer is valid                        
   TEMPLATE()
   auto KNOWNPOINTER()::operator -> () {
      LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
      return mPointer;
   }

   /// Pointer dereferencing (const)                                          
   ///   @attention assumes contained pointer is valid                        
   TEMPLATE()
   decltype(auto) KNOWNPOINTER()::operator * () const {
      LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
      return *mPointer;
   }

   /// Pointer dereferencing                                                  
   ///   @attention assumes contained pointer is valid                        
   TEMPLATE()
   decltype(auto) KNOWNPOINTER()::operator * () {
      LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
      return *mPointer;
   }

   
   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE()
   typename TAny<T>::Iterator TAny<T>::begin() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const TAny<T>*>(this)->begin();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TEMPLATE()
   typename TAny<T>::Iterator TAny<T>::end() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const TAny<T>*>(this)->end();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE()
   typename TAny<T>::Iterator TAny<T>::last() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const TAny<T>*>(this)->last();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TEMPLATE()
   typename TAny<T>::ConstIterator TAny<T>::begin() const noexcept {
      if (IsEmpty())
         return end();

      return {GetRaw(), GetRawEnd()};
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TEMPLATE()
   typename TAny<T>::ConstIterator TAny<T>::end() const noexcept {
      const auto endptr = GetRawEnd();
      return {endptr, endptr};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TEMPLATE()
   typename TAny<T>::ConstIterator TAny<T>::last() const noexcept {
      if (IsEmpty())
         return end();

      const auto endptr = GetRawEnd();
      return {endptr - 1, endptr};
   }



   
   ///                                                                        
   ///   TAny iterator                                                        
   ///                                                                        
   #define ITERATOR() TAny<T>::template TIterator<MUTABLE>

   /// Construct an iterator                                                  
   ///   @param e - element                                                   
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   TAny<T>::TIterator<MUTABLE>::TIterator(const TypeInner* e, const TypeInner* s) noexcept
      : mElement {e}
      , mSentinel {s} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename ITERATOR()& TAny<T>::TIterator<MUTABLE>::operator ++ () noexcept {
      if (mElement == mSentinel)
         return *this;

      // Seek next valid info, or hit sentinel at the end               
      ++mElement;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename ITERATOR() TAny<T>::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare unordered map entries                                          
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   bool TAny<T>::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mElement == rhs.mElement;
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   TAny<T>::TIterator<MUTABLE>::operator TypeInner& () const noexcept requires (MUTABLE) {
      return const_cast<TypeInner&>(*mElement);
   }

   /// Iterator access operator                                               
   ///   @return a pair at the current iterator position                      
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   TAny<T>::TIterator<MUTABLE>::operator const MemberType& () const noexcept requires (!MUTABLE) {
      if constexpr (CT::Dense<T>)
         return *mElement;
      else
         return mElement->mPointer;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef KNOWNPOINTER
#undef ITERATOR