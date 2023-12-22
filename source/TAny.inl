///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TAny.hpp"
#include "Any.inl"
#include <cctype>

#define TEMPLATE() template<CT::Data T>


namespace Langulus::Anyness
{

   /// Default construction                                                   
   /// TAny is always type-constrained, but its type is set on demand to      
   /// avoid requesting meta definitions before meta database initialization, 
   /// and to significanty improve TAny initialization time (also to allow    
   /// for constexpr default construction)                                    
   TEMPLATE() LANGULUS(INLINED)
   constexpr TAny<T>::TAny() {
      if constexpr (CT::Constant<T>)
         mState = DataState::Typed | DataState::Constant;
      else
         mState = DataState::Typed;
   }

   /// Shallow-copy construction (const)                                      
   ///   @param other - the TAny to reference                                 
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>::TAny(const TAny& other)
      : TAny {Copy(other)} {}
    
   /// Move construction                                                      
   ///   @param other - the TAny to move                                      
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>::TAny(TAny&& other) noexcept
      : TAny {Move(other)} {}

   /// Semantic-construction                                                  
   ///   @param other - the TAny and semantic to construct with               
   TEMPLATE() template<template<class> class S> LANGULUS(INLINED)
   TAny<T>::TAny(S<TAny>&& other)
   requires (CT::Inner::SemanticMakable<S,T>) {
      BlockTransfer<TAny>(other.Forward());
   }
   
   /// Create from a list of elements, each of them can be semantic or not,   
   /// an array, as well as any other kinds of anies                          
   ///   @param t1 - first element                                            
   ///   @param tail - tail of elements (optional)                            
   TEMPLATE() template<class T1, class... TAIL> LANGULUS(INLINED)
   TAny<T>::TAny(T1&& t1, TAIL&&... tail)
   requires (CT::Inner::UnfoldMakableFrom<T, T1, TAIL...>) {
      Insert(IndexFront, Forward<T1>(t1), Forward<TAIL>(tail)...);
   }
   
   /// Semantic construction from any container/element                       
   ///   @param other - the container/element and semantic                    
   /*TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::ConstructFrom(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using ST = TypeOf<S>;
      mType = MetaDataOf<T>();

      if constexpr (CT::Array<ST> and CT::Similar<T, Deext<ST>>) {
         // Integration with bounded arrays                             
         constexpr auto count = ExtentOf<ST>;
         AllocateFresh(RequestSize(count));
         InsertInner<Copied>(*other, *other + count, 0);
      }
      else if constexpr (CT::Deep<ST>) {
         if constexpr (not CT::Typed<ST>) {
            // RHS is type-erased deep, do runtime type checks          
            if (mType->IsExact(other->GetType())) {
               // If types are exactly the same, it is safe to directly 
               // absorb the block, essentially converting a type-      
               // erased Any, to this TAny representation               
               BlockTransfer<TAny>(other.Forward());
               return;
            }
            else if constexpr (CT::Deep<T> and not CT::Typed<T>) {
               // All deep containers are binary-compatible with each   
               // other (unless typed), so RHS can be contained in this 
               // TAny, just reinterpret it as the type-erased T, and   
               // push it                                               
               AllocateFresh(RequestSize(1));
               InsertInner(Abandon(T {other.Forward()}), 0);
               return;
            }
            else {
               // Other corner cases?                                   
               TODO();
            }
         }
         else {
            // RHS is not type-erased deep, do compile-time checks      
            using ContainedType = TypeOf<ST>;

            if constexpr (CT::Similar<T, ContainedType>) {
               // If types are exactly the same, it is safe to directly 
               // transfer the block                                    
               BlockTransfer<TAny<T>>(other.Forward());
            }
            else if constexpr (CT::DerivedFrom<T, ContainedType>) {
               // The statically typed RHS contains items that are      
               // base of this container's type. Each element should be 
               // dynamically cast to this type                         
               if constexpr (CT::Sparse<T, ContainedType>) {
                  for (auto pointer : *other) {
                     auto dcast = dynamic_cast<T>(&(*pointer));
                     if (dcast)
                        (*this) << dcast;
                  }
               }
               else TODO();
            }
            else {
               // Other corner cases?                                   
               TODO();
            }
         }
      }
      else if constexpr (CT::DerivedFrom<ST, TAny<T>>
                    and sizeof(ST) == sizeof(TAny<T>)) {
         // Some containers, like Bytes and Text aren't CT::Deep, so    
         // we have this special case for them, only if they're binary- 
         // compatible                                                  
         BlockTransfer<TAny>(other.Forward());
      }
      else if constexpr (requires { T(other.Forward()); }) {
         // Element is semantically-constructible from argument         
         AllocateFresh(RequestSize(1));
         if constexpr (CT::Sparse<T> or CT::Similar<T, ST>)
            InsertInner(other.Forward(), 0);
         else if constexpr (CT::AbandonMakable<T>)
            InsertInner(Abandon(T (other.Forward())), 0);
         else if constexpr (CT::MoveMakable<T>)
            InsertInner(Move(T (other.Forward())), 0);
         else LANGULUS_ERROR("Bad semantic construction");
      }
      else LANGULUS_ERROR("Bad semantic construction");
   }*/

   /// Destructor                                                             
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>::~TAny() {
      Free();
   }

   /// Construct manually by interfacing memory directly                      
   /// Data will be copied, if not in jurisdiction, which involves a slow     
   /// authority check                                                        
   ///   @param what - data to shallow (or deep) copy                         
   ///   @param count - number of items behind the pointer                    
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::From(const T* what, const Count& count) {
      return From(Copy(what), count);
   }

   /// Construct manually by interfacing memory directly                      
   /// Data will be copied, if not in jurisdiction, which involves a slow     
   /// authority check. If you want to avoid checking and copying, use the    
   /// Disowned semantic                                                      
   ///   @param what - data to semantically interface                         
   ///   @param count - number of items, in case 'what' is a pointer          
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::From(CT::Semantic auto&& what, const Count& count) {
      using S = Decay<decltype(what)>;

      TAny<T> result;
      if constexpr (CT::Sparse<TypeOf<S>>)
         result.SetMemory(DataState::Constrained, result.GetType(), count, *what, nullptr);
      else
         result.SetMemory(DataState::Constrained, result.GetType(), 1, &(*what), nullptr);

      if constexpr (not S::Move and S::Keep)
         result.TakeAuthority();
      return result;
   }

   /// Get the static type of the container                                   
   /// Also initializes the type of this container                            
   ///   @attention this should not be called at static initialization time   
   ///   @return the meta definition of the type                              
   TEMPLATE() LANGULUS(INLINED)
   DMeta TAny<T>::GetType() const noexcept {
      mType = MetaDataOf<T>();
      return mType;
   }

   /// Dereference and eventually destroy all elements                        
   ///   @attention this never modifies any state, except mEntry              
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Free() {
      if (not mEntry)
         return;

      if (mEntry->GetUses() == 1) {
         if constexpr (CT::Sparse<T> or not CT::POD<T>) {
            if (mCount)
               CallKnownDestructors<T>();
         }

         Allocator::Deallocate(const_cast<Allocation*>(mEntry));
         mEntry = nullptr;
         return;
      }

      const_cast<Allocation*>(mEntry)->Free();
      mEntry = nullptr;
   }

   /// Shallow-copy assignment                                                
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator = (const TAny& other) {
      return operator = (Copy(other));
   }

   /// Move assignment                                                        
   ///   @param other - the container to move                                 
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator = (TAny&& other) {
      return operator = (Move(other));
   }
   
   /// Generic semantic assignment                                            
   ///   @param other - the container to shallow-copy                         
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>& TAny<T>::AssignFrom(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;

      if constexpr (CT::Deep<TypeOf<S>>) {
         if (static_cast<const Block*>(this)
          == static_cast<const Block*>(&*other))
            return *this;

         Free();
         new (this) TAny {other.Forward()};
      }
      else {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Reset();
            operator << (other.Forward());
         }
         else {
            // Just destroy and reuse memory                            
            CallKnownDestructors<T>();
            mCount = 1;

            if constexpr (CT::Sparse<T>) {
               *mRawSparse = const_cast<Byte*>(
                  reinterpret_cast<const Byte*>(*other)
               );
               *GetEntries() = nullptr;
            }
            else SemanticNew(mRaw, other.Forward());
         }
      }

      return *this;
   }

   /// An internal function used to copy members, without copying type and    
   /// without overwriting states, if required                                
   ///   @param other - the block to copy from                                
   TEMPLATE() template<bool OVERWRITE_STATE, bool OVERWRITE_ENTRY>
   LANGULUS(INLINED)
   void TAny<T>::CopyProperties(const Block& other) noexcept {
      mRaw = other.mRaw;
      mCount = other.mCount;
      mReserved = other.mReserved;
      mType = other.mType;
      if constexpr (OVERWRITE_STATE)
         mState = other.mState;
      else
         mState += other.mState;

      if constexpr (OVERWRITE_ENTRY)
         mEntry = other.mEntry;
   }

   /// Reset container state (inner function)                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr void TAny<T>::ResetState() noexcept {
      mState &= DataState::Typed;
   }

   /// Reset container type (does nothing for typed container)                
   TEMPLATE() LANGULUS(INLINED)
   constexpr void TAny<T>::ResetType() noexcept {}

   /// Check if contained data can be interpreted as a given type             
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::CastsToMeta(DMeta type) const {
      return GetType()->template CastsTo<CT::Sparse<T>>(type);
   }

   /// Check if contained data can be interpreted as a given count of type    
   /// For example: a Vec4 can interpret as float[4]                          
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @param count - the number of elements to interpret as                
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::CastsToMeta(DMeta type, Count count) const {
      return GetType()->CastsTo(type, count);
   }
   
   /// Check if this container's data can be represented as type T            
   /// with nothing more than pointer arithmetic                              
   ///   @tparam T - the type to compare against                              
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @return true if contained data is reinterpretable as T               
   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   bool TAny<T>::CastsTo() const {
      return CastsToMeta(MetaDataOf<Decay<ALT_T>>());
   }

   /// Check if this container's data can be represented as a specific number 
   /// of elements of type T, with nothing more than pointer arithmetic       
   ///   @tparam T - the type to compare against                              
   ///   @param count - the number of elements of T                           
   ///   @return true if contained data is reinterpretable as T               
   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   bool TAny<T>::CastsTo(Count count) const {
      return CastsToMeta(MetaDataOf<Decay<ALT_T>>(), count);
   }

   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TAny<T>::Is() const noexcept {
      return CT::SameAsOneOf<T, T1, TN...>;
   }

   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data similar to 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::Is(DMeta type) const noexcept {
      return GetType()->Is(type);
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TAny<T>::IsSimilar() const noexcept {
      return CT::SimilarAsOneOf<T, T1, TN...>;
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data similar to 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::IsSimilar(DMeta type) const noexcept {
      return GetType()->IsSimilar(type);
   }

   /// Check if this type is exactly one of the provided types                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TAny<T>::IsExact() const noexcept {
      return CT::ExactAsOneOf<T, T1, TN...>;
   }

   /// Check if this type is exactly one of the provided types                
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data of exactly 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::IsExact(DMeta type) const noexcept {
      return GetType()->IsExact(type);
   }

   /// Allocate 'count' elements and fill the container with zeroes           
   /// If T is not CT::Nullifiable, this function does default construction,  
   /// which would be slower, than batch zeroing                              
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Null(const Count& count) {
      if constexpr (CT::Nullifiable<T>) {
         if (count < mReserved)
            AllocateLess(count);
         else
            AllocateMore<false, true>(count);

         ZeroMemory(GetRaw(), count);
      }
      else New(count);
   }

   /// Clear the container, destroying all elements,                          
   /// but retaining allocation if possible                                   
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Clear() {
      if (not mCount)
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
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Reset() {
      Free();
      ResetMemory();
      ResetState();
   }

   /// Return the typed raw data (const)                                      
   ///   @return a constant pointer to the first element in the array         
   TEMPLATE() LANGULUS(INLINED)
   const T* TAny<T>::GetRaw() const noexcept {
      return GetRawAs<T>();
   }

   /// Return the typed raw data                                              
   ///   @return a mutable pointer to the first element in the array          
   TEMPLATE() LANGULUS(INLINED)
   T* TAny<T>::GetRaw() noexcept {
      return GetRawAs<T>();
   }

   /// Return the typed raw data end pointer (const)                          
   ///   @return a constant pointer to one past the last element in the array 
   TEMPLATE() LANGULUS(INLINED)
   const T* TAny<T>::GetRawEnd() const noexcept {
      return GetRaw() + mCount;
   }
   
   /// Return a handle to a sparse element, or a pointer to dense one         
   ///   @param index - the element index                                     
   ///   @return the handle/pointer                                           
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TAny<T>::GetHandle(Offset index) IF_UNSAFE(noexcept) {
      return Block::GetHandle<T>(index);
   }

   /// GetHandle ignores constness                                            
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TAny<T>::GetHandle(Offset index) const IF_UNSAFE(noexcept) {
      return Block::GetHandle<T>(index);
   }

   /// Get a size based on reflected allocation page and count                
   /// This is an optimization for predictable fundamental types              
   ///   @param count - the number of elements to request                     
   ///   @returns both the provided byte size and reserved count              
   TEMPLATE() LANGULUS(INLINED)
   AllocationRequest TAny<T>::RequestSize(const Count& count) const noexcept {
      if constexpr (CT::Fundamental<T> or CT::Exact<T, Byte>) {
         AllocationRequest result;
         result.mByteSize = ::std::max(Roof2(count * sizeof(T)), Alignment);
         result.mElementCount = result.mByteSize / sizeof(T);
         return result;
      }
      else return GetType()->RequestSize(count);
   }
   
   /// Get an element in the way you want (const, unsafe)                     
   /// This is a statically optimized variant of Block::Get                   
   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   decltype(auto) TAny<T>::Get(const Offset& index) const noexcept {
      const auto& element = GetRaw()[index];
      if constexpr (CT::Dense<T> and CT::Dense<ALT_T>)
         // Dense -> Dense (returning a reference)                      
         return static_cast<const Decay<ALT_T>&>(element);
      else if constexpr (CT::Dense<T> and CT::Sparse<ALT_T>)
         // Dense -> Sparse (returning a pointer)                       
         return static_cast<const Decay<ALT_T>*>(&element);
      else if constexpr (CT::Sparse<T> and CT::Dense<ALT_T>)
         // Sparse -> Dense (returning a reference)                     
         return static_cast<const Decay<ALT_T>&>(*element.mPointer);
      else
         // Sparse -> Sparse (returning a reference to pointer)         
         return static_cast<const Decay<ALT_T>* const&>(element.mPointer);
   }

   /// Get an element in the way you want (unsafe)                            
   /// This is a statically optimized variant of Block::Get                   
   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   decltype(auto) TAny<T>::Get(const Offset& index) noexcept {
      auto& element = GetRaw()[index];
      if constexpr (CT::Dense<T> and CT::Dense<ALT_T>)
         // Dense -> Dense (returning a reference)                      
         return static_cast<Decay<ALT_T>&>(element);
      else if constexpr (CT::Dense<T> and CT::Sparse<ALT_T>)
         // Dense -> Sparse (returning a pointer)                       
         return static_cast<Decay<ALT_T>*>(&element);
      else if constexpr (CT::Sparse<T> and CT::Dense<ALT_T>)
         // Sparse -> Dense (returning a reference)                     
         return static_cast<Decay<ALT_T>&>(*element.mPointer);
      else
         // Sparse -> Sparse (returning a reference to pointer)         
         return static_cast<Decay<ALT_T>*&>(element.mPointer);
   }

   /// Access typed dense elements by index                                   
   ///   @param idx - the index to get                                        
   ///   @return a reference to the element                                   
   TEMPLATE() LANGULUS(INLINED)
   const T& TAny<T>::operator [] (const CT::Index auto& index) const {
      const auto offset = SimplifyIndex<T>(index);
      return GetRaw()[offset];
   }

   TEMPLATE() LANGULUS(INLINED)
   T& TAny<T>::operator [] (const CT::Index auto& index) {
      const auto offset = SimplifyIndex<T>(index);
      return GetRaw()[offset];
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TEMPLATE() LANGULUS(INLINED)
   T& TAny<T>::Last() {
      LANGULUS_ASSUME(UserAssumes, mCount, "Can't get last index");
      return GetRaw()[mCount - 1];
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a constant reference to the last element                     
   TEMPLATE() LANGULUS(INLINED)
   const T& TAny<T>::Last() const {
      LANGULUS_ASSUME(UserAssumes, mCount, "Can't get last index");
      return GetRaw()[mCount - 1];
   }
   
   /// Templated Any containers are always typed                              
   ///   @return true                                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsTyped() const noexcept {
      return true;
   }
   
   /// Templated Any containers are never untyped                             
   ///   @return false                                                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsUntyped() const noexcept {
      return false;
   }
   
   /// Templated Any containers are always type-constrained                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if contained type is abstract                                    
   /// This is a statically optimized alternative to Block::IsAbstract        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsAbstract() const noexcept {
      return CT::Abstract<T>;
   }
   
   /// Check if contained type is deep                                        
   /// This is a statically optimized alternative to Block::IsDeep            
   ///   @return true if this container contains deep items                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsDeep() const noexcept {
      // Sparse types are never considered deep, except when contained  
      return CT::Deep<Decay<T>>;
   }

   /// Check if the contained type is a pointer                               
   /// This is a statically optimized alternative to Block::IsSparse          
   ///   @return true if container contains pointers                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsSparse() const noexcept {
      return CT::Sparse<T>;
   }

   /// Check if the contained type is not a pointer                           
   /// This is a statically optimized alternative to Block::IsDense           
   ///   @return true if container contains sequential data                   
   TEMPLATE() LANGULUS(INLINED)
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
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsPOD() const noexcept {
      return CT::POD<T>;
   }

   /// Check if block contains resolvable items, that is, items that have a   
   /// GetBlock() function, that can be used to represent themselves as their 
   /// most concretely typed block                                            
   /// This is a statically optimized alternative to Block::IsResolvable      
   ///   @return true if contained data can be resolved on element basis      
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsResolvable() const noexcept {
      return CT::Resolvable<T>;
   }

   /// Get the size of a single contained element, in bytes                   
   /// This is a statically optimized alternative to Block::GetStride         
   ///   @return the number of bytes a single element contains                
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TAny<T>::GetStride() const noexcept {
      return sizeof(T);
   }
   
   /// Get the size of all elements, in bytes                                 
   ///   @return the total amount of initialized bytes                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TAny<T>::GetBytesize() const noexcept {
      return GetStride() * mCount;
   }

   /// Unfold-insert item(s) at an index, semantically or not                 
   ///   @param index - the index to insert at                                
   ///   @param t1 - the first element                                        
   ///   @param tail - the rest of the elements (optional)                    
   ///   @return number of inserted items                                     
   TEMPLATE() template<class T1, class...TAIL> LANGULUS(INLINED)
   Count TAny<T>::Insert(CT::Index auto index, T1&& t1, TAIL&&...tail)
   requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...> {
      Count inserted = 0;
      inserted   += UnfoldInsert<TAny, void>(index, Forward<T1>(t1));
      ((inserted += UnfoldInsert<TAny, void>(index + inserted, Forward<TAIL>(tail))), ...);
      return inserted;
   }

   /// Emplace a single item at the given index, forwarding all arguments     
   /// to its constructor                                                     
   ///   @param at - the index to emplace at                                  
   ///   @param arguments... - the arguments for the element's constructor    
   ///   @return 1 if the item was emplaced, 0 if not                         
   TEMPLATE() template<class...A>
   Count TAny<T>::Emplace(CT::Index auto at, A&&...arguments)
   requires CT::Inner::MakableFrom<T, A...> {
      return Block::Emplace<TAny>(at, Forward<A>(arguments)...);
   }

   /// Create N new elements, using default construction                      
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @return the number of new elements                                   
   TEMPLATE() LANGULUS(INLINED)
   Count TAny<T>::New(Count count) {
      // Allocate                                                       
      AllocateMore<false>(mCount + count);

      // Call constructors                                              
      CropInner(mCount, 0)
         .template CallKnownDefaultConstructors<T>(count);

      mCount += count;
      return count;
   }

   /// Create N new elements, using the provided arguments for construction   
   /// Elements will be added to the back of the container                    
   ///   @tparam ...A - arguments for construction (deducible)                
   ///   @param count - number of elements to construct                       
   ///   @param ...arguments - constructor arguments                          
   ///   @return the number of new elements                                   
   TEMPLATE() template<class... A> LANGULUS(INLINED)
   Count TAny<T>::New(Count count, A&&... arguments)
   requires CT::Inner::MakableFrom<T, A...> {
      // Allocate                                                       
      AllocateMore<false>(mCount + count);

      // Call constructors                                              
      CropInner(mCount, 0)
         .template CallKnownConstructors<T>(
            count, Forward<A>(arguments)...);

      mCount += count;
      return count;
   }

   /// Insert an element at the back of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator << (T1&& rhs)
   requires CT::Inner::UnfoldMakableFrom<T, T1> {
      Insert(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Insert an element at the front of the container                        
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator >> (T1&& rhs)
   requires CT::Inner::UnfoldMakableFrom<T, T1> {
      Insert(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Merge a single element by a semantic                                   
   /// Element will be pushed only if not found in block                      
   ///   @param index - the index at which to insert                          
   ///   @param t1 - the first item to insert                                 
   ///   @param tail... - the rest of items to insert (optional)              
   ///   @return the number of inserted elements                              
   TEMPLATE() template<class T1, class...TAIL> LANGULUS(INLINED)
   Count TAny<T>::Merge(CT::Index auto index, T1&& t1, TAIL&&...tail)
   requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...> {
      Count inserted = 0;
      inserted += UnfoldMerge(index, Forward<T1>(t1));
      ((inserted += UnfoldMerge(index + inserted, Forward<TAIL>(tail))), ...);
      return inserted;
   }

   /// Merge an element at the back of the container                          
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator <<= (T1&& other)
   requires CT::Inner::UnfoldMakableFrom<T, T1> {
      Merge(IndexBack, Forward<T1>(other));
      return *this;
   }

   /// Merge an element at the front of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator >>= (T1&& other)
   requires CT::Inner::UnfoldMakableFrom<T, T1> {
      Merge(IndexFront, Forward<T1>(other));
      return *this;
   }

   /// Find element(s) index inside container                                 
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @param cookie - resume search from a given index                     
   ///   @return the index of the found item, or IndexNone if none found      
   TEMPLATE() template<bool REVERSE, CT::Data ALT_T>
   Index TAny<T>::Find(const ALT_T& item, const Offset& cookie) const noexcept {
      static_assert(CT::Inner::Comparable<T, ALT_T>, 
         "Can't find non-comparable element");

      const T* start = REVERSE
         ? GetRawEnd() - 1 - cookie
         : GetRaw() + cookie;
      const T* const end = REVERSE
         ? start - mCount
         : start + mCount;

      while (start != end) {
         if (*start == item)
            return start - GetRaw();

         if constexpr (REVERSE)
            --start;
         else
            ++start;
      }

      // If this is reached, then no match was found                    
      return IndexNone;
   }

   /// Remove matching items by value                                         
   ///   @tparam REVERSE - whether to search in reverse order                 
   ///   @param item - the item to search for to remove                       
   ///   @return the number of removed items                                  
   TEMPLATE() template<bool REVERSE, CT::Data ALT_T> LANGULUS(INLINED)
   Count TAny<T>::Remove(const ALT_T& item) {
      const auto found = Find<REVERSE>(item);
      if (found)
         return RemoveIndex(found.GetOffsetUnsafe(), 1);
      return 0;
   }

   /// Remove sequential raw indices in a given range                         
   ///   @attention assumes starter + count <= mCount                         
   ///   @param index - index to start removing from                          
   ///   @param count - number of elements to remove                          
   ///   @return the number of removed elements                               
   TEMPLATE()
   Count TAny<T>::RemoveIndex(const CT::Index auto& index, Count count) {
      using INDEX = Deref<decltype(index)>;

      if constexpr (CT::Same<INDEX, Index>) {
         // By special indices                                          
         if (index == IndexAll) {
            const auto oldCount = mCount;
            Free();
            ResetMemory();
            ResetState();
            return oldCount;
         }

         const auto idx = ConstrainMore<T>(index);
         if (idx.IsSpecial())
            return 0;

         return RemoveIndex(idx.GetOffsetUnsafe(), count);
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
         const auto ender = idx + count;
         LANGULUS_ASSUME(DevAssumes, ender <= mCount, "Out of range");

         if constexpr (CT::Sparse<T> or CT::POD<T>) {
            if (ender == mCount) {
               // If data is POD and elements are on the back, we can   
               // get around constantness and staticness, by simply     
               // truncating the count without any reprecussions        
               // We can completely skip destroying POD things          
               mCount = idx;
               return count;
            }

            LANGULUS_ASSERT(GetUses() == 1, Move,
               "Removing elements from memory block, used from multiple places, "
               "requires memory to move");
            LANGULUS_ASSERT(IsMutable(), Access,
               "Attempting to remove from constant container");
            LANGULUS_ASSERT(not IsStatic(), Access,
               "Attempting to remove from static container");

            MoveMemory(GetRaw() + idx, GetRaw() + ender, mCount - ender);
         }
         else {
            if (IsStatic() and ender == mCount) {
               // If data is static and elements are on the back, we    
               // can get around constantness and staticness, by simply 
               // truncating the count without any reprecussions        
               // We can't destroy static element anyways               
               mCount = idx;
               return count;
            }

            LANGULUS_ASSERT(GetUses() == 1, Move,
               "Removing elements from memory block, used from multiple places, "
               "requires memory to move");
            LANGULUS_ASSERT(IsMutable(), Access,
               "Attempting to remove from constant container");
            LANGULUS_ASSERT(not IsStatic(), Access,
               "Attempting to remove from static container");

            // Call the destructors on the correct region               
            CropInner(idx, count)
               .template CallKnownDestructors<T>();

            if (ender < mCount) {
               // Fill gap	if any by invoking move constructions        
               // Moving to the left, so no overlap possible if forward 
               const auto tail = mCount - ender;
               CropInner(idx, 0)
                  .template CallKnownSemanticConstructors<T>(
                     tail, Abandon(CropInner(ender, tail))
                  );
            }
         }

         mCount -= count;
         return count;
      }
   }
   
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this TAny instance        
   ///   @param index - the index to start removing at                        
   ///   @param count - number of elements to remove                          
   ///   @return the iterator of the previous element, unless index is first  
   TEMPLATE()
   typename TAny<T>::Iterator TAny<T>::RemoveIt(const Iterator& index, Count count) {
      const auto rawend = GetRawEnd();
      if (index.mElement >= rawend)
         return {rawend};

      const auto rawstart = GetRaw();
      RemoveIndex(static_cast<Offset>(index.mElement - rawstart), count); //TODO what if map shrinks, offset might become invalid? Doesn't shrink for now
      
      if (index.mElement == rawstart)
         return {rawstart};
      else
         return {index.mElement - 1};
   }

   /// Sort the pack                                                          
   TEMPLATE() template<bool ASCEND> LANGULUS(INLINED)
   void TAny<T>::Sort() {
      if constexpr (CT::Sortable<T>)
         Any::Sort<T, ASCEND>();
      else
         LANGULUS_ERROR("Can't sort container - T is not sortable");
   }

   /// Remove elements on the back                                            
   ///   @param count - the new count                                         
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Trim(const Count& count) {
      if (count >= mCount)
         return;

      if (IsConstant() or IsStatic()) {
         if (mType->mIsPOD) {
            // If data is POD and elements are on the back, we can      
            // get around constantness and staticness, by simply        
            // truncating the count without any reprecussions           
            mCount = count;
         }
         else {
            LANGULUS_ASSERT(not IsConstant(), Access,
               "Removing from constant container");
            LANGULUS_ASSERT(not IsStatic(), Access,
               "Removing from static container");
         }

         return;
      }

      // Call destructors and change count                              
      CropInner(count, mCount - count).template CallKnownDestructors<T>();
      mCount = count;
   }

   /// Swap two elements                                                      
   ///   @param from - the first element                                      
   ///   @param to - the second element                                       
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Swap(const Offset& from, const Offset& to) {
      Any::Swap<T>(from, to);
   }

   /// Swap two elements using special indices                                
   ///   @param from - the first element                                      
   ///   @param to - the second element                                       
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Swap(const Index& from, const Index& to) {
      Any::Swap<T>(from, to);
   }

   /// Gather items from source container, and fill this one                  
   ///   @tparam REVERSE - iterate in reverse?                                
   ///   @param source - container to gather from, type acts as filter        
   ///   @return the number of gathered elements                              
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TAny<T>::GatherFrom(const Block& source) {
      return GatherInner<REVERSE>(source, *this);
   }

   /// Gather items of specific state from source container, and fill this one
   ///   @tparam REVERSE - iterate in reverse?                                
   ///   @param source - container to gather from, type acts as filter        
   ///   @param state - state filter                                          
   ///   @return the number of gathered elements                              
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TAny<T>::GatherFrom(const Block& source, DataState state) {
      return GatherPolarInner<REVERSE>(GetType(), source, *this, state);
   }

   /// Clone container array into a new owned memory block                    
   /// If we have jurisdiction, the memory won't move at all                  
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::TakeAuthority() {
      TAny<T> clone = Clone(*this);
      Free();
      CopyMemory<void, void>(this, &clone, sizeof(TAny<T>));
      clone.mEntry = nullptr;
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::Crop(const Offset& start, const Count& count) const {
      return Block::Crop<TAny>(start, count);
   }
   
   /// Pick a region and reference it from another container                  
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::Crop(const Offset& start, const Count& count) {
      return Block::Crop<TAny>(start, count);
   }
   
   /// Allocate a number of elements, relying on the type of the container    
   ///   @tparam CREATE - true to call constructors and initialize count      
   ///   @tparam SETSIZE - true to set size without calling any constructors  
   ///   @param elements - number of elements to allocate                     
   TEMPLATE() template<bool CREATE, bool SETSIZE>
   void TAny<T>::AllocateMore(Count elements) {
      LANGULUS_ASSUME(DevAssumes, elements > mCount, "Bad element count");

      // Allocate/reallocate                                            
      const auto request = RequestSize(elements);
      if (mEntry) {
         if (mReserved >= elements) {
            // Required memory is already available                     
            if constexpr (CREATE) {
               // But is not yet initialized, so initialize it          
               if (mCount < elements) {
                  const auto count = elements - mCount;
                  CropInner(mCount, count)
                     .template CallKnownDefaultConstructors<T>(count);
               }
            }

            if constexpr (CREATE or SETSIZE)
               mCount = elements;
            return;
         }

         // Reallocate                                                  
         Block previousBlock {*this};
         mEntry = Allocator::Reallocate(
            request.mByteSize * (CT::Sparse<T> ? 2 : 1),
            const_cast<Allocation*>(mEntry)
         );
         LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
         mReserved = request.mElementCount;

         if (mEntry != previousBlock.mEntry) {
            // Memory moved, and we should move all elements in it      
            // We're moving to new memory, so no reverse required       
            if (mEntry->GetUses() == 1) {
               // Memory is used only once and it is safe to move it    
               // Make note, that Allocator::Reallocate doesn't copy    
               // anything, it doesn't use realloc for various          
               // reasons, so we still have to call move construction   
               // for all elements if entry moved (enabling             
               // MANAGED_MEMORY feature significantly reduces the      
               // chance for a move). Sparse containers have            
               // additional memory allocated for each pointer's        
               // entry, if managed memory is enabled                   
               if constexpr (CT::AbandonMakable<T> or CT::MoveMakable<T> or CT::CopyMakable<T>) {
                  mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
                  CallKnownSemanticConstructors<T>(
                     previousBlock.mCount, Abandon(previousBlock)
                  );

                  // Also, we should free the previous allocation       
                  previousBlock.Free();
               }
               else LANGULUS_THROW(Construct, "Memory moved, but T is not move-constructible");
            }
            else {
               // Memory is used from multiple locations, and we must   
               // copy the memory for this block - we can't move it!    
               // This will throw, if data is not copy-constructible    
               if constexpr (CT::DisownMakable<T> or CT::CopyMakable<T>) {
                  AllocateFresh(request);
                  CallKnownSemanticConstructors<T>(
                     previousBlock.mCount, Copy(previousBlock)
                  );
               }
               else LANGULUS_THROW(Construct, "Memory moved, but T is not copy-constructible");
            }
         }
         else {
            // Memory didn't move, but reserved count changed           
            if constexpr (CT::Sparse<T>) {
               // Move entry data to its new place                      
               MoveMemory(
                  GetEntries(), previousBlock.GetEntries(), mCount
               );
            }
         }

         if constexpr (CREATE) {
            // Default-construct the rest                               
            const auto count = elements - mCount;
            CropInner(mCount, count)
               .template CallKnownDefaultConstructors<T>(count);
         }
      }
      else {
         // Allocate a fresh set of elements                            
         mType = MetaDataOf<T>();
         AllocateFresh(request);

         if constexpr (CREATE) {
            // Default-construct everything                             
            CropInner(mCount, elements)
               .template CallKnownDefaultConstructors<T>(elements);
         }
      }
      
      if constexpr (CREATE or SETSIZE)
         mCount = elements;
   }
   
   /// Shrink the block, depending on currently reserved	elements             
   /// Initialized elements on the back will be destroyed                     
   ///   @attention assumes 'elements' is smaller than the current reserve    
   ///   @param elements - number of elements to allocate                     
   TEMPLATE()
   void TAny<T>::AllocateLess(Count elements) {
      LANGULUS_ASSUME(DevAssumes, elements < mReserved, "Bad element count");
      LANGULUS_ASSUME(DevAssumes, mType, "Invalid type");

      if (mCount > elements) {
         // Destroy back entries on smaller allocation                  
         // Allowed even when container is static and out of            
         // jurisdiction, as in that case this acts as a simple count   
         // decrease, and no destructors shall be called                
         Trim(elements);
         return;
      }

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         // Shrink the memory block                                     
         // Guaranteed that entry doesn't move                          
         const auto request = RequestSize(elements);
         if (request.mElementCount != mReserved) {
            (void)Allocator::Reallocate(
               request.mByteSize * (CT::Sparse<T> ? 2 : 1),
               const_cast<Allocation*>(mEntry)
            );
            mReserved = request.mElementCount;
         }
      #endif
   }

   /// Allocate a fresh allocation (inner function)                           
   ///   @attention changes entry, memory and reserve count                   
   ///   @param request - request to fulfill                                  
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::AllocateFresh(const AllocationRequest& request) {
      // Sparse containers have additional memory allocated             
      // for each pointer's entry                                       
      mEntry = Allocator::Allocate(
         GetType(), request.mByteSize * (CT::Sparse<T> ? 2 : 1)
      );
      LANGULUS_ASSERT(mEntry, Allocate, "Out of memory");
      mRaw = const_cast<Byte*>(mEntry->GetBlockStart());
      mReserved = request.mElementCount;
   }

   /// Reserve a number of elements without initializing them                 
   ///   @param count - number of elements to reserve                         
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Reserve(Count count) {
      // Notice, it calls the locally defined function equivalents      
      if (count < mCount)
         AllocateLess(count);
      else
         AllocateMore(count);
   }
   
   /// Extend the container and return the new part                           
   ///   @tparam WRAPPER - the container to use for the extended part         
   ///   @param count - the number of elements to extend by                   
   ///   @return a container that represents the extended part                
   TEMPLATE() template<CT::Block WRAPPER> LANGULUS(INLINED)
   WRAPPER TAny<T>::Extend(const Count& count) {
      const auto previousCount = mCount;
      AllocateMore<true>(mCount + count);
      const auto wrapped = Crop(previousCount, count);
      return reinterpret_cast<const WRAPPER&>(wrapped);
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

      if constexpr (CT::Sparse<T> or CT::POD<T>) {
         // Batch compare POD/pointers                                  
         return 0 == ::std::memcmp(GetRaw(), other.GetRaw(), GetBytesize());
      }
      else if constexpr (CT::Inner::Comparable<T>) {
         // Use comparison operator between all elements                
         auto t1 = GetRaw();
         auto t2 = other.GetRaw();
         const auto t1end = t1 + mCount;
         while (t1 < t1end and *t1 == *t2) {
            ++t1;
            ++t2;
         }
         return t1 == t1end;
      }
      else LANGULUS_ERROR("Elements not comparable");
   }

   /// Compare with another container of the same type                        
   ///   @param other - the container to compare with                         
   ///   @return true if both containers are identical                        
   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   bool TAny<T>::operator == (const TAny<ALT_T>& other) const noexcept
   requires (CT::Inner::Comparable<T, ALT_T>) {
      if constexpr (CT::Same<T, ALT_T>)
         return Compare(other);
      else
         return false;
   }

   /// Compare with block of unknown type                                     
   ///   @param other - the block to compare with                             
   ///   @return true if both containers are identical                        
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::operator == (const Any& other) const noexcept
   requires (CT::Inner::Comparable<T>) {
      if (IsExact(other.GetType())) {
         // Use statically optimized routine if possible                
         return Compare(reinterpret_cast<const TAny<T>&>(other));
      }

      // Fallback to RTTI routine                                       
      return Any::Compare(other);
   }

   /// Compare loosely with another TAny, ignoring case                       
   /// This function applies only if T is character                           
   ///   @param other - text to compare with                                  
   ///   @return true if both containers match loosely                        
   TEMPLATE()
   bool TAny<T>::CompareLoose(const TAny& other) const noexcept {
      if constexpr (CT::Character<T>) {
         if (mRaw == other.mRaw)
            return mCount == other.mCount;
         else if (mCount != other.mCount)
            return false;

         auto t1 = GetRaw();
         auto t2 = other.GetRaw();
         while (t1 < GetRawEnd() and ::std::tolower(*t1)
                                  == ::std::tolower(*(t2++)))
            ++t1;
         return (t1 - GetRaw()) == mCount;
      }
      else return Compare(other);
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
      while (t1 != t1end and t2 != t2end and *t1 == *(t2++))
         ++t1;
      return t1 - GetRaw();
   }

   /// Compare loosely with another, ignoring upper-case                      
   /// Count how many consecutive letters match in two strings                
   ///   @param other - text to compare with                                  
   ///   @return the number of matching symbols                               
   TEMPLATE()
   Count TAny<T>::MatchesLoose(const TAny& other) const noexcept {
      if constexpr (CT::Character<T>) {
         if (mRaw == other.mRaw)
            return ::std::min(mCount, other.mCount);

         auto t1 = GetRaw();
         auto t2 = other.GetRaw();
         const auto t1end = GetRawEnd();
         const auto t2end = other.GetRawEnd();
         while (t1 != t1end and t2 != t2end and ::std::tolower(*t1)
                                             == ::std::tolower(*(t2++)))
            ++t1;
         return t1 - GetRaw();
      }
      else return Matches(other);
   }
  
   /// Hash data inside memory block                                          
   ///   @attention order matters, so you might want to Neat data first       
   ///   @return the hash                                                     
   TEMPLATE() LANGULUS(INLINED)
   Hash TAny<T>::GetHash() const requires CT::Hashable<T> {
      if (not mCount)
         return {};

      if (mCount == 1) {
         // Exactly one element means exactly one hash                  
         return HashOf(*GetRaw());
      }

      // Hashing multiple elements                                      
      if constexpr (CT::Sparse<T>) {
         if constexpr (CT::Meta<T>) {
            // Always dereference the metas and get their hash          
            TAny<Hash> h;
            h.Reserve(mCount);
            for (auto& element : *this)
               h << element ? element->GetHash() : Hash {};
            return h.GetHash();
         }
         else return HashBytes<DefaultHashSeed, false>(
            mRaw, static_cast<int>(GetBytesize()));
      }
      else if constexpr (CT::POD<T> and not CT::Inner::HasGetHashMethod<T>) {
         // Hash all PODs at once                                       
         return HashBytes<DefaultHashSeed, alignof(T) < Bitness / 8>(
            mRaw, static_cast<int>(GetBytesize()));
      }
      else {
         // Hash each element, and then combine hashes in a final one   
         TAny<Hash> h;
         h.Reserve(mCount);
         for (auto& element : *this)
            h << HashOf(element);
         return h.GetHash();
      }
   }

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::Iterator TAny<T>::begin() noexcept {
      return {GetRaw()};
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::IteratorEnd TAny<T>::end() noexcept {
      return {GetRawEnd()};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::Iterator TAny<T>::last() noexcept {
      return {IsEmpty() ? GetRawEnd() : GetRawEnd() - 1};
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::ConstIterator TAny<T>::begin() const noexcept {
      return {GetRaw()};
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::ConstIteratorEnd TAny<T>::end() const noexcept {
      return {GetRawEnd()};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::ConstIterator TAny<T>::last() const noexcept {
      return {IsEmpty() ? GetRawEnd(): GetRawEnd() - 1};
   }


   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// Concatenate anything, semantically or not                              
   ///   @param rhs - the element/block/array to copy-concatenate             
   ///   @return a new container, containing both blocks                      
   TEMPLATE() template<class T1> LANGULUS(INLINED)
   TAny<T> TAny<T>::operator + (T1&& rhs) const
   requires CT::Inner::UnfoldMakableFrom<T, T1> {
      using S = SemanticOf<decltype(rhs)>;
      return ConcatBlock<TAny>(S::Nest(rhs));
   }

   /// Concatenate destructively, semantically or not                         
   ///   @param rhs - the element/block/array to semantically concatenate     
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator += (T1&& rhs)
   requires CT::Inner::UnfoldMakableFrom<T, T1> {
      using S = SemanticOf<decltype(rhs)>;
      InsertBlock<TAny, void>(IndexBack, S::Nest(rhs));
      return *this;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef KNOWNPOINTER