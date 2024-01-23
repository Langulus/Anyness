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
   
   /// Create from a list of elements, each of them can be semantic or not,   
   /// an array, as well as any other kinds of anies                          
   ///   @param t1 - first element                                            
   ///   @param tail - tail of elements (optional)                            
   TEMPLATE() template<class T1, class...TAIL>
   requires CT::DeepMakable<T, T1, TAIL...> LANGULUS(INLINED)
   TAny<T>::TAny(T1&& t1, TAIL&&...tail) {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using ST = TypeOf<S>;

         if constexpr (CT::Block<ST>) {
            if constexpr (CT::Typed<ST>) {
               // Not type-erased block, do compile-time type checks    
               using STT = TypeOf<ST>;
               if constexpr (CT::Similar<T, STT>) {
                  // Type is binary compatible, just transfer block     
                  BlockTransfer<TAny>(S::Nest(t1));
               }
               else if constexpr (CT::Sparse<T, STT>) {
                  if constexpr (CT::DerivedFrom<T, STT>) {
                     // The statically typed block contains items that  
                     // are base of this container's type. Each element 
                     // should be dynamically cast to this type         
                     for (auto pointer : DesemCast(t1)) {
                        auto dcast = dynamic_cast<T>(&(*pointer));
                        if (dcast)
                           (*this) << dcast;
                     }
                  }
                  else if constexpr (CT::DerivedFrom<STT, T>) {
                     // The statically typed block contains items that  
                     // are derived from this container's type. Each    
                     // element should be statically sliced to this type
                     for (auto pointer : DesemCast(t1))
                        (*this) << static_cast<T>(&(*pointer));
                  }
                  else Insert(IndexBack, Forward<T1>(t1));
               }
               else Insert(IndexBack, Forward<T1>(t1));
            }
            else {
               // Type-erased block, do run-time type checks            
               if (mType == DesemCast(t1).GetType()) {
                  // If types are exactly the same, it is safe to       
                  // absorb the block, essentially converting a type-   
                  // erased Any back to its TAny equivalent             
                  BlockTransfer<TAny>(S::Nest(t1));
               }
               else InsertBlock(IndexBack, Forward<T1>(t1));
            }
         }
         else Insert(IndexBack, Forward<T1>(t1));
      }
      else Insert(IndexBack, Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Destructor                                                             
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>::~TAny() {
      Free<TAny>();
   }

   /// Construct manually by interfacing memory directly                      
   /// Data will be copied, if not in jurisdiction, which involves a slow     
   /// authority check. If you want to avoid checking and copying, use the    
   /// Disowned semantic                                                      
   ///   @param what - data to semantically interface                         
   ///   @param count - number of items, in case 'what' is sparse             
   ///   @return the provided data, wrapped inside a TAny<T>                  
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::From(auto&& what, Count count) {
      using S = SemanticOf<decltype(what)>;
      using ST = TypeOf<S>;
      TAny<T> result;

      if constexpr (CT::Dense<T>) {
         // We're creating a dense block...                             
         if constexpr (CT::Array<ST>) {
            // ... from a bounded array                                 
            using DST = Deext<ST>;
            const auto count2 = count * ExtentOf<ST> * sizeof(DST);
            LANGULUS_ASSERT(0 == (count2 % sizeof(T)),
               Meta, "Provided array type is not a multiple of sizeof(T)");
            count = count2 / sizeof(T);

            if constexpr (CT::Similar<T, DST> or CT::POD<T, DST>) {
               result.SetMemory(
                  DataState::Constrained,
                  result.GetType(), count,
                  DesemCast(what), nullptr
               );
            }
            else {
               LANGULUS_ERROR(
                  "Can't wrap a bounded array inside incompatible TAny<T>:"
                  " types are not binary compatible"
               );
            }
         }
         else if constexpr (CT::Sparse<ST>) {
            // ... from a pointer                                       
            using DST = Deptr<ST>;
            const auto count2 = count * sizeof(DST);
            LANGULUS_ASSERT(0 == (count2 % sizeof(T)),
               Meta, "Provided pointer type is not a multiple of sizeof(T)");
            count = count2 / sizeof(T);

            if constexpr (CT::Similar<T, DST> or CT::POD<T, DST>) {
               result.SetMemory(
                  DataState::Constrained,
                  result.GetType(), count,
                  DesemCast(what), nullptr
               );
            }
            else {
               LANGULUS_ERROR(
                  "Can't wrap a unbounded array inside incompatible TAny<T>:"
                  " types are not binary compatible"
               );
            }
         }
         else {
            // ... from a value                                         
            static_assert(0 == (sizeof(ST) % sizeof(T)), 
               "Provided type is not a multiple of sizeof(T)");
            count = sizeof(ST) / sizeof(T);

            if constexpr (CT::Similar<T, ST> or CT::POD<T, ST>) {
               result.SetMemory(
                  DataState::Constrained,
                  result.GetType(), count,
                  &DesemCast(what), nullptr
               );
            }
            else {
               LANGULUS_ERROR(
                  "Can't wrap a dense element inside incompatible TAny<T>:"
                  " types are not binary compatible"
               );
            }
         }
      }
      else LANGULUS_ERROR("Can't manually interface a sparse block");

      if constexpr (not S::Move and S::Keep)
         result.TakeAuthority<TAny>();
      return result;
   }

   /// Get the static type of the container                                   
   /// Also initializes the type of this container                            
   ///   @attention this should not be called at static initialization time   
   ///   @return the meta definition of the type                              
   TEMPLATE() LANGULUS(INLINED)
   DMeta TAny<T>::GetType() const noexcept {
      return Block::GetType<TAny>();
   }

   /// Shallow-copy assignment                                                
   ///   @param rhs - the container to shallow-copy                           
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator = (const TAny& rhs) {
      static_assert(CT::DeepAssignable<T, Copied<TAny<T>>>);
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the container to move                                   
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator = (TAny&& rhs) {
      static_assert(CT::DeepAssignable<T, Moved<TAny<T>>>);
      return operator = (Move(rhs));
   }

   /// Generic assignment                                                     
   ///   @param rhs - the element/array/container to assign                   
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepAssignable<T, T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator = (T1&& rhs) {
      using S = SemanticOf<T1>;
      using ST = TypeOf<S>;

      if constexpr (CT::Block<ST>) {
         // Potentially absorb a container                              
         if (static_cast<const Block*>(this)
          == static_cast<const Block*>(&DesemCast(rhs)))
            return *this;

         Free<TAny>();
         new (this) TAny {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Clear();
         Block::UnfoldInsert<TAny, void, true>(0, S::Nest(rhs));
      }

      return *this;
   }

   /// Reset container state (inner function)                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr void TAny<T>::ResetState() noexcept {
      Block::ResetState<TAny>();
   }
   
   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TAny<T>::Is() const noexcept {
      return Block::Is<TAny, T1, TN...>();
   }

   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data similar to 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::Is(DMeta type) const noexcept {
      return Block::Is<TAny>(type);
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TAny<T>::IsSimilar() const noexcept {
      return Block::IsSimilar<TAny, T1, TN...>();
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data similar to 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::IsSimilar(DMeta type) const noexcept {
      return Block::IsSimilar<TAny>(type);
   }

   /// Check if this type is exactly one of the provided types                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TAny<T>::IsExact() const noexcept {
      return Block::IsExact<TAny, T1, TN...>();
   }

   /// Check if this type is exactly one of the provided types                
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data of exactly 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TAny<T>::IsExact(DMeta type) const noexcept {
      return Block::IsExact<TAny>(type);
   }

   /// Check if contained data can be interpreted as a given type             
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE() template<bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool TAny<T>::CastsToMeta(DMeta type) const {
      return Block::CastsToMeta<BINARY_COMPATIBLE>(type);
   }

   /// Check if contained data can be interpreted as a given count of type    
   /// For example: a Vec4 can interpret as float[4]                          
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @param count - the number of elements to interpret as                
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE() template<bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool TAny<T>::CastsToMeta(DMeta type, const Count count) const {
      return Block::CastsToMeta<BINARY_COMPATIBLE>(type, count);
   }
   
   /// Check if this container's data can be represented as type T            
   /// with nothing more than pointer arithmetic                              
   ///   @tparam ALT_T - the type to compare against                          
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @return true if contained data is reinterpretable as T               
   TEMPLATE() template<CT::Data ALT_T, bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool TAny<T>::CastsTo() const {
      return Block::CastsTo<ALT_T, BINARY_COMPATIBLE>();
   }

   /// Check if this container's data can be represented as a specific number 
   /// of elements of type T, with nothing more than pointer arithmetic       
   ///   @tparam ALT_T - the type to compare against                          
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @param count - the number of elements of T                           
   ///   @return true if contained data is reinterpretable as T               
   TEMPLATE() template<CT::Data ALT_T, bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool TAny<T>::CastsTo(const Count count) const {
      return Block::CastsTo<ALT_T, BINARY_COMPATIBLE>(count);
   }

   /// Reinterpret contents of this Block as the type and state of another    
   /// You can interpret Vec4 as float[4] for example, or any other such      
   /// reinterpretation, as long as data remains tightly packed and aligned   
   /// No real conversion is performed, only pointer arithmetic               
   ///   @param pattern - the type of data to try interpreting as             
   ///   @return a block representing this block, interpreted as the pattern  
   TEMPLATE() template<CT::Block B> LANGULUS(INLINED)
   B TAny<T>::ReinterpretAs(const B& rhs) const {
      return Block::ReinterpretAs<TAny, B>(rhs);
   }

   TEMPLATE() template<CT::Data T1> LANGULUS(INLINED)
   TAny<T1> TAny<T>::ReinterpretAs() const {
      return Block::ReinterpretAs<T1, TAny>();
   }

   /// Get the memory block corresponding to a local member variable          
   ///   @attention assumes block is not empty                                
   ///   @param member - the member to get                                    
   ///   @param idx - the element to get member from                          
   ///   @return a static memory block                                        
   TEMPLATE() LANGULUS(INLINED)
   Block TAny<T>::GetMember(const RTTI::Member& member, CT::Index auto idx) {
      return Block::GetMember<TAny>(member, idx);
   }

   TEMPLATE() LANGULUS(INLINED)
   Block TAny<T>::GetMember(const RTTI::Member& member, CT::Index auto idx) const {
      return Block::GetMember<TAny>(member, idx);
   }

   /// Allocate 'count' elements and fill the container with zeroes           
   /// If T is not CT::Nullifiable, this function does default construction,  
   /// which would be slower, than batch zeroing                              
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Null(Count count) {
      return Block::Null<TAny>(count);
   }

   /// Clear the container, destroying all elements,                          
   /// but retaining allocation if possible                                   
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Clear() {
      Block::Clear<TAny>();
   }

   /// Reset the container, destroying all elements, and deallocating         
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Reset() {
      Block::Reset<TAny>();
   }
   
   /// Get the raw data inside the container                                  
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @return a pointer to the first allocated element                     
   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   constexpr auto TAny<T>::GetRaw() noexcept {
      return Block::GetRaw<THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   constexpr auto TAny<T>::GetRaw() const noexcept {
      return Block::GetRaw<THIS>();
   }

   /// Get the end raw data pointer inside the container (const)              
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @attention the resulting pointer never points to a valid element     
   ///   @return a pointer to the last+1 element (never initialized)          
   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   constexpr auto TAny<T>::GetRawEnd() const noexcept {
      return Block::GetRawEnd<THIS>();
   }

   /// Get a pointer array - useful only for sparse containers                
   ///   @return the raw data as an array of pointers                         
   TEMPLATE() template<class THIS> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   auto TAny<T>::GetRawSparse() IF_UNSAFE(noexcept) {
      return Block::GetRawSparse<THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   auto TAny<T>::GetRawSparse() const IF_UNSAFE(noexcept) {
      return Block::GetRawSparse<THIS>();
   }
   
   /// Get a pointer array - useful only for sparse containers                
   ///   @tparam T - the type (dense) to interpret pointers as                
   ///   @return the pointer to the first pointer of T                        
   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   T1** TAny<T>::GetRawSparseAs() IF_UNSAFE(noexcept) {
      return Block::GetRawSparseAs<T1, THIS>();
   }

   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   const T1* const* TAny<T>::GetRawSparseAs() const IF_UNSAFE(noexcept) {
      return Block::GetRawSparseAs<T1, THIS>();
   }
   
   /// Get the raw data inside the container, reinterpreted as some type      
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @tparam T - the type we're interpreting as                           
   ///   @return a pointer to the first element of type T                     
   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   T1* TAny<T>::GetRawAs() noexcept {
      return Block::GetRawAs<T1, THIS>();
   }

   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   const T1* TAny<T>::GetRawAs() const noexcept {
      return Block::GetRawAs<T1, THIS>();
   }

   /// Get the end raw data pointer inside the container                      
   ///   @attention never points to a valid element                           
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @tparam T - the type we're interpreting as                           
   ///   @return a pointer to the last+1 element of type T                    
   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   const T1* TAny<T>::GetRawEndAs() const noexcept {
      return Block::GetRawEndAs<T1, THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   const Allocation* const* TAny<T>::GetEntries() const IF_UNSAFE(noexcept) {
      return Block::GetEntries<THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   const Allocation** TAny<T>::GetEntries() IF_UNSAFE(noexcept) {
      return Block::GetEntries<THIS>();
   }

   /// Return a handle to a sparse element, or a pointer to dense one         
   ///   @param index - the element index                                     
   ///   @return the handle/pointer                                           
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TAny<T>::GetHandle(const Offset index) IF_UNSAFE(noexcept) {
      return Block::GetHandle<T, TAny>(index);
   }

   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TAny<T>::GetHandle(const Offset index) const IF_UNSAFE(noexcept) {
      return Block::GetHandle<T, TAny>(index);
   }

   /// Get a deep memory sub-block                                            
   ///   @param index - the index to get, indices are mapped as the following:
   ///      0 always refers to this block                                     
   ///      [1; mCount] always refer to subblocks in this block               
   ///      [mCount + 1; mCount + N] refer to subblocks in the first subblock 
   ///                               N being the size of that subblock        
   ///      ... and so on ...                                                 
   ///   @return a pointer to the block or nullptr if index is invalid        
   /*TEMPLATE() LANGULUS(INLINED)
   Block* TAny<T>::GetBlockDeep(const Offset index) noexcept {
      return Block::GetBlockDeep<TAny>(index);
   }

   TEMPLATE() LANGULUS(INLINED)
   Block const* TAny<T>::GetBlockDeep(const Offset index) const noexcept {
      return Block::GetBlockDeep<TAny>(index);
   }

   /// Get a deep element block                                               
   ///   @param index - the index to get                                      
   ///   @return the element block                                            
   TEMPLATE() LANGULUS(INLINED)
   Block TAny<T>::GetElementDeep(const Offset index) noexcept {
      return Block::GetElementDeep<TAny>(index);
   }

   TEMPLATE() LANGULUS(INLINED)
   Block TAny<T>::GetElementDeep(const Offset index) const noexcept {
      return Block::GetElementDeep<TAny>(index);
   }*/

   /// Get an element in the way you want (unsafe)                            
   /// This is a statically optimized variant of Block::Get                   
   ///   @tparam ALT_T - how to interpret data                                
   ///   @param index - the element index to retrieve                         
   ///   @return a reference/pointer to the element, depending on ALT_T       
   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   decltype(auto) TAny<T>::Get(const Offset index) const noexcept {
      return Block::Get<ALT_T>(index);
   }

   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   decltype(auto) TAny<T>::Get(const Offset index) noexcept {
      return Block::Get<ALT_T>(index);
   }

   /// Access typed dense elements by index                                   
   ///   @param idx - the index to get                                        
   ///   @return a reference to the element                                   
   TEMPLATE() LANGULUS(INLINED)
   const T& TAny<T>::operator [] (const CT::Index auto index) const {
      return Block::operator [] <TAny> (index);
   }

   TEMPLATE() LANGULUS(INLINED)
   T& TAny<T>::operator [] (const CT::Index auto index) {
      return Block::operator [] <TAny> (index);
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TEMPLATE() LANGULUS(INLINED)
   T& TAny<T>::Last() {
      return Block::Last<TAny>();
   }

   TEMPLATE() LANGULUS(INLINED)
   const T& TAny<T>::Last() const {
      return Block::Last<TAny>();
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
      
   /// Check if contained type is deep                                        
   /// This is a statically optimized alternative to Block::IsDeep            
   ///   @return true if this container contains deep items                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsDeep() const noexcept {
      return Block::IsDeep<TAny>();
   }

   /// Check if the contained type is a pointer                               
   /// This is a statically optimized alternative to Block::IsSparse          
   ///   @return true if container contains pointers                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsSparse() const noexcept {
      return Block::IsSparse<TAny>();
   }

   /// Check if the contained type is not a pointer                           
   /// This is a statically optimized alternative to Block::IsDense           
   ///   @return true if container contains sequential data                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsDense() const noexcept {
      return Block::IsDense<TAny>();
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
      return Block::IsPOD<TAny>();
   }

   /// Check if block contains resolvable items, that is, items that have a   
   /// GetBlock() function, that can be used to represent themselves as their 
   /// most concretely typed block                                            
   /// This is a statically optimized alternative to Block::IsResolvable      
   ///   @return true if contained data can be resolved on element basis      
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsResolvable() const noexcept {
      return Block::IsResolvable<TAny>();
   }

   /// Get the size of a single contained element, in bytes                   
   /// This is a statically optimized alternative to Block::GetStride         
   ///   @return the number of bytes a single element contains                
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TAny<T>::GetStride() const noexcept {
      return Block::GetStride<TAny>();
   }
   
   /// Get the size of all elements, in bytes                                 
   ///   @return the total amount of initialized bytes                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TAny<T>::GetBytesize() const noexcept {
      return Block::GetBytesize<TAny>();
   }

   /// Get the number of sub-blocks (this one included)                       
   ///   @return the number of contained blocks, including this one           
   TEMPLATE() LANGULUS(INLINED)
   constexpr Count TAny<T>::GetCountDeep() const noexcept {
      return Block::GetCountDeep<TAny>();
   }

   /// Get the sum of initialized non-deep elements in all sub-blocks         
   ///   @return the number of contained non-deep elements                    
   TEMPLATE() LANGULUS(INLINED)
   constexpr Count TAny<T>::GetCountElementsDeep() const noexcept {
      return Block::GetCountElementsDeep<TAny>();
   }

   /// Deep (slower) check if there's anything missing inside nested blocks   
   ///   @return true if any deep or flat memory block contains missing data  
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsMissingDeep() const {
      return Block::IsMissingDeep<TAny>();
   }

   /// Check if a memory block can be concatenated to this one                
   ///   @param b - the block to concatenate                                  
   ///   @return true if able to concatenate to this one                      
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsConcatable(const CT::Block auto& b) const noexcept {
      return Block::IsConcatable<TAny>(b);
   }

   /// Check if a type can be inserted to this block                          
   ///   @param other - check if a given type is insertable to this block     
   ///   @return true if able to insert an instance of the type to this block 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsInsertable(DMeta type) const noexcept {
      return Block::IsInsertable<TAny>(type);
   }

   /// Check if a static type can be inserted                                 
   ///   @return true if able to insert an instance of the type to this block 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TAny<T>::IsInsertable() const noexcept {
      return Block::IsInsertable<TAny>();
   }

   /// Unfold-insert item(s) at an index, semantically or not                 
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the index to insert at                                
   ///   @param t1 - the first element                                        
   ///   @param tail - the rest of the elements (optional)                    
   ///   @return number of inserted items                                     
   TEMPLATE() template<bool MOVE_ASIDE, class T1, class...TAIL>
   requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...> LANGULUS(INLINED)
   Count TAny<T>::Insert(CT::Index auto index, T1&& t1, TAIL&&...tail) {
      return Block::Insert<TAny, Any, MOVE_ASIDE>(
         index, Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Insert all elements of a block at an index, semantically or not        
   ///   @tparam FORCE - insert even if types mismatch, by making this block  
   ///      deep with provided type - use void to disable                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - index to insert thems at                              
   ///   @param other - the block to insert                                   
   ///   @return the number of inserted elements                              
   TEMPLATE() template<class FORCE, bool MOVE_ASIDE, class T1>
   requires CT::Block<Desem<T1>> LANGULUS(INLINED)
   Count TAny<T>::InsertBlock(CT::Index auto index, T1&& other) {
      return Block::InsertBlock<TAny, FORCE, MOVE_ASIDE>(
         index, Forward<T1>(other));
   }

   /// Merge elements                                                         
   /// Element will be pushed only if not found in block                      
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the index at which to insert                          
   ///   @param t1 - the first item to insert                                 
   ///   @param tail... - the rest of items to insert (optional)              
   ///   @return the number of inserted items                                 
   TEMPLATE() template<bool MOVE_ASIDE, class T1, class...TAIL>
   requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...> LANGULUS(INLINED)
   Count TAny<T>::Merge(CT::Index auto index, T1&& t1, TAIL&&...tail) {
      return Block::Merge<TAny, Any, MOVE_ASIDE>(
         index, Forward<T1>(t1), Forward<TAIL>(tail)...);
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
   TEMPLATE() template<class FORCE, bool MOVE_ASIDE, class T1>
   requires CT::Block<Desem<T1>> LANGULUS(INLINED)
   Count TAny<T>::MergeBlock(CT::Index auto index, T1&& other) {
      return Block::MergeBlock<TAny, FORCE, MOVE_ASIDE>(
         index, Forward<T1>(other));
   }

   /// Emplace a single item at the given index, forwarding all arguments     
   /// to its constructor                                                     
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param at - the index to emplace at                                  
   ///   @param arguments... - the arguments for the element's constructor    
   ///   @return 1 if the item was emplaced, 0 if not                         
   TEMPLATE() template<bool MOVE_ASIDE, class...A>
   requires ::std::constructible_from<T, A...> LANGULUS(INLINED)
   Conditional<CT::Sparse<T>, T, T&>
   TAny<T>::Emplace(CT::Index auto at, A&&...arguments) {
      Block::Emplace<TAny, MOVE_ASIDE>(at, Forward<A>(arguments)...);
      return Get(mCount - 1);
   }

   /// Create N new elements, using default construction                      
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @return the number of new elements                                   
   TEMPLATE() LANGULUS(INLINED)
   Count TAny<T>::New(const Count count) requires CT::Inner::Defaultable<T> {
      return Block::New<TAny>(count);
   }

   /// Create N new elements, using the provided arguments for construction   
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @param arguments... - constructor arguments, all forwarded together  
   ///      for each instance of T                                            
   ///   @return the number of new elements                                   
   TEMPLATE() template<class...A>
   requires ::std::constructible_from<T, A...> LANGULUS(INLINED)
   Count TAny<T>::New(const Count count, A&&...arguments) {
      return Block::New<TAny>(count, Forward<A>(arguments)...);
   }

   /// Insert an element at the back of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator << (T1&& rhs) {
      Insert(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Insert an element at the front of the container                        
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator >> (T1&& rhs) {
      Insert(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the back of the container                          
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator <<= (T1&& rhs) {
      Merge(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the front of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator >>= (T1&& rhs) {
      Merge(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Find element index inside container                                    
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @param cookie - resume search from a given index                     
   ///   @return the index of the found item, or IndexNone if none found      
   TEMPLATE() template<bool REVERSE, CT::NotSemantic T1>
   requires CT::Inner::Comparable<T, T1>
   Index TAny<T>::Find(const T1& item, Offset cookie) const noexcept {
      return Block::Find<REVERSE, TAny>(item, cookie);
   }

   /// Find a sequence of one or more elements inside container               
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @param cookie - resume search from a given index                     
   ///   @return the index of the found item, or IndexNone if none found      
   TEMPLATE() template<bool REVERSE>
   Index TAny<T>::FindBlock(const CT::Block auto& item, CT::Index auto cookie) const noexcept {
      return Block::FindBlock<REVERSE, TAny>(item, cookie);
   }

   /// Remove matching items by value                                         
   ///   @tparam REVERSE - whether to search in reverse order                 
   ///   @param item - the item to search for to remove                       
   ///   @return the number of removed items                                  
   TEMPLATE() template<bool REVERSE, CT::Data ALT_T> LANGULUS(INLINED)
   Count TAny<T>::Remove(const ALT_T& item) {
      const auto found = Find<REVERSE>(item);
      return found ? RemoveIndex(found.GetOffsetUnsafe(), 1) : 0;
   }

   /// Remove sequential raw indices in a given range                         
   ///   @attention assumes starter + count <= mCount                         
   ///   @param index - index to start removing from                          
   ///   @param count - number of elements to remove                          
   ///   @return the number of removed elements                               
   TEMPLATE()
   Count TAny<T>::RemoveIndex(CT::Index auto index, Count count) {
      return Block::RemoveIndex<TAny>(index, count);
   }
   
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this TAny instance        
   ///   @param index - the index to start removing at                        
   ///   @param count - number of elements to remove                          
   ///   @return the iterator of the previous element, unless index is first  
   TEMPLATE()
   typename TAny<T>::Iterator TAny<T>::RemoveIt(const Iterator& index, Count count) {
      return Block::RemoveIt<TAny>(index, count);
   }

   /// Sort the pack                                                          
   TEMPLATE() template<bool ASCEND> requires CT::Inner::Sortable<T>
   LANGULUS(INLINED) void TAny<T>::Sort() {
      Block::Sort<ASCEND, TAny>();
   }

   /// Remove elements on the back                                            
   ///   @param count - the new count                                         
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Trim(Count count) {
      Block::Trim<TAny>(count);
   }

   /// Swap two elements                                                      
   ///   @param from - the first element                                      
   ///   @param to - the second element                                       
   TEMPLATE() LANGULUS(INLINED)
   void TAny<T>::Swap(CT::Index auto from, CT::Index auto to) {
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

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::Crop(Offset start, Count count) const {
      return Block::Crop<TAny>(start, count);
   }
   
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::Crop(Offset start, Count count) {
      return Block::Crop<TAny>(start, count);
   }
     
   /// Iterate each element block and execute F for it                        
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - function to execute for each element block             
   ///   @return the number of executions                                     
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TAny<T>::ForEachElement(auto&& call) {
      return Block::ForEachElement<REVERSE, TAny>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TAny<T>::ForEachElement(auto&& call) const {
      return Block::ForEachElement<REVERSE, const TAny>(
         Forward<Deref<decltype(call)>>(call));
   }

   /// Execute functions for each element inside container                    
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. The rest of the provided 
   /// functions are ignored, after the first function with viable argument.  
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TAny<T>::ForEach(auto&&...call) {
      return Block::ForEach<REVERSE, TAny>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TAny<T>::ForEach(auto&&...call) const {
      return Block::ForEach<REVERSE, const TAny>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   /// Execute functions in each sub-block, inclusively                       
   /// Unlike the flat variants above, this one reaches into sub-blocks.      
   /// Each function has a distinct argument type, that is tested against the 
   /// contained type. If argument is compatible with the type, the block is  
   /// iterated, and F is executed for all elements. None of the provided     
   /// functions are ignored.                                                 
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @tparam SKIP - set to false, to execute F for intermediate blocks,   
   ///                  too; otherwise will execute only for non-blocks       
   ///   @param calls - all potential functions to iterate with               
   ///   @return the number of executions                                     
   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TAny<T>::ForEachDeep(auto&&...call) {
      return Block::ForEachDeep<REVERSE, SKIP, TAny>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TAny<T>::ForEachDeep(auto&&...call) const {
      return Block::ForEachDeep<REVERSE, SKIP, const TAny>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   /// Reserve a number of elements without initializing them                 
   ///   @tparam SETSIZE - whether or not to set size, too                    
   ///   @attention using SETSIZE will NOT construct any elements, use only   
   ///      if you know what you're doing                                     
   ///   @param count - number of elements to reserve                         
   TEMPLATE() template<bool SETSIZE> LANGULUS(INLINED)
   void TAny<T>::Reserve(const Count count) {
      Block::Reserve<SETSIZE, TAny>(count);
   }
   
   /// Extend the container via default construction, and return the new part 
   ///   @param count - the number of elements to extend by                   
   ///   @return a container that represents only the extended part           
   TEMPLATE() LANGULUS(INLINED)
   TAny<T> TAny<T>::Extend(const Count count) {
      return Block::Extend<TAny>(count);
   }
   
   /// Compare with another container, order matters                          
   ///   @param other - container to compare with                             
   ///   @return true if both containers match completely                     
   TEMPLATE() template<bool RESOLVE>
   bool TAny<T>::Compare(const CT::Block auto& other) const {
      return Block::Compare<RESOLVE, TAny>(other);
   }

   /// Compare with any other kind of block                                   
   ///   @param other - the block to compare with                             
   ///   @return true if both containers are identical                        
   TEMPLATE() template<CT::NotSemantic T1>
   requires (CT::UntypedBlock<T1>
      or (CT::TypedBlock<T1> and CT::Inner::Comparable<T, TypeOf<T1>>)
      or CT::Inner::Comparable<T, T1>) LANGULUS(INLINED)
   bool TAny<T>::operator == (const T1& other) const {
      return Block::operator == <TAny> (other);
   }

   /// Compare loosely with another TAny, ignoring case                       
   /// This function applies only if T is character                           
   ///   @param other - text to compare with                                  
   ///   @return true if both containers match loosely                        
   TEMPLATE()
   bool TAny<T>::CompareLoose(const CT::Block auto& other) const noexcept {
      return Block::CompareLoose<TAny>(other);
   }

   /// Count how many consecutive elements match in two containers            
   ///   @param other - container to compare with                             
   ///   @return the number of matching items                                 
   TEMPLATE()
   Count TAny<T>::Matches(const CT::Block auto& other) const noexcept {
      return Block::Matches<TAny>(other);
   }

   /// Compare loosely with another, ignoring upper-case                      
   /// Count how many consecutive letters match in two strings                
   ///   @param other - text to compare with                                  
   ///   @return the number of matching symbols                               
   TEMPLATE()
   Count TAny<T>::MatchesLoose(const CT::Block auto& other) const noexcept {
      return Block::MatchesLoose<TAny>(other);
   }
  
   /// Hash data inside memory block                                          
   ///   @attention order matters, so you might want to Neat data first       
   ///   @return the hash                                                     
   TEMPLATE() LANGULUS(INLINED)
   Hash TAny<T>::GetHash() const requires CT::Hashable<T> {
      return Block::GetHash<TAny>();
   }

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::Iterator TAny<T>::begin() noexcept {
      return Block::begin<TAny>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::ConstIterator TAny<T>::begin() const noexcept {
      return Block::begin<TAny>();
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::Iterator TAny<T>::last() noexcept {
      return Block::last<TAny>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TAny<T>::ConstIterator TAny<T>::last() const noexcept {
      return Block::last<TAny>();
   }


   ///                                                                        
   ///   Concatenation                                                        
   ///                                                                        

   /// Concatenate anything, semantically or not                              
   ///   @param rhs - the element/block/array to copy-concatenate             
   ///   @return a new container, containing both blocks                      
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TAny<T> TAny<T>::operator + (T1&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      return Block::ConcatBlock<TAny>(S::Nest(rhs));
   }

   /// Concatenate destructively, semantically or not                         
   ///   @param rhs - the element/block/array to semantically concatenate     
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TAny<T>& TAny<T>::operator += (T1&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      Block::InsertBlock<TAny, void>(IndexBack, S::Nest(rhs));
      return *this;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE