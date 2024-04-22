///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TMany.hpp"
#include "Many.inl"
#include <cctype>

#define TEMPLATE() template<CT::Data T>


namespace Langulus::Anyness
{

   /// Default construction                                                   
   /// TMany is always type-constrained, but its type is set on demand to     
   /// avoid requesting meta definitions before meta database initialization, 
   /// and to significanty improve TMany initialization time (also to allow    
   /// for constexpr default construction)                                    
   TEMPLATE() LANGULUS(INLINED)
   constexpr TMany<T>::TMany() {
      if constexpr (CT::Constant<T>)
         mState = DataState::Typed | DataState::Constant;
      else
         mState = DataState::Typed;
   }

   /// Refer constructor                                                      
   ///   @param other - the TMany to reference                                 
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>::TMany(const TMany& other)
      : TMany {Refer(other)} {}
    
   /// Move constructor                                                       
   ///   @param other - the TMany to move                                      
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>::TMany(TMany&& other) noexcept
      : TMany {Move(other)} {}
   
   /// Create from a list of elements, each of them can be semantic or not,   
   /// an array, as well as any other kinds of anies                          
   ///   @param t1 - first element                                            
   ///   @param tn - tail of elements (optional)                              
   TEMPLATE() template<class T1, class...TN>
   requires CT::DeepMakable<T, T1, TN...> LANGULUS(INLINED)
   TMany<T>::TMany(T1&& t1, TN&&...tn) {
      mType = MetaDataOf<T>();

      if constexpr (sizeof...(TN) == 0) {
         using S = SemanticOf<decltype(t1)>;
         using ST = TypeOf<S>;

         if constexpr (CT::Block<ST>) {
            if constexpr (CT::Typed<ST>) {
               // Not type-erased block, do compile-time type checks    
               using STT = TypeOf<ST>;

               if constexpr (CT::Similar<T, STT>) {
                  // Type is binary compatible, just transfer block     
                  Block::BlockTransfer<TMany>(S::Nest(t1));
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
            else if constexpr (CT::Deep<ST>) {
               // Type-erased block, do run-time type checks            
               if (IsSimilar(DesemCast(t1).GetType())) {
                  // If types are similar, it is safe to                
                  // absorb the block, essentially converting a type-   
                  // erased Many back to its TMany equivalent           
                  Block::BlockTransfer<TMany>(S::Nest(t1));
               }
               else if constexpr (CT::Deep<T>) {
                  // This TMany accepts any kind of deep element        
                  Insert(IndexBack, Forward<T1>(t1));
               }
               else if constexpr (CT::Typed<ST> and CT::MakableFrom<T, typename S::template As<TypeOf<ST>>>) {
                  // Attempt converting all elements to T               
                  InsertBlock(IndexBack, Forward<T1>(t1));
               }
               else LANGULUS_OOPS(Meta, "Unable to absorb block");
            }
            else LANGULUS_ERROR("Can't construct this TMany from this kind of Block");
         }
         else Insert(IndexBack, Forward<T1>(t1));
      }
      else Insert(IndexBack, Forward<T1>(t1), Forward<TN>(tn)...);
   }

   /// Destructor                                                             
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>::~TMany() {
      Block::Free<TMany>();
   }

   /// Construct manually by interfacing memory directly                      
   /// Data will be copied, if not in jurisdiction, which involves a slow     
   /// authority check. If you want to avoid checking and copying, use the    
   /// Disowned semantic                                                      
   ///   @param what - data to semantically interface                         
   ///   @param count - number of items, in case 'what' is sparse             
   ///   @return the provided data, wrapped inside a TMany<T>                 
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::From(auto&& what, Count count) {
      using S = SemanticOf<decltype(what)>;
      using ST = TypeOf<S>;

      TMany<T> result;
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
               new (&result) Block {
                  DataState::Constrained,
                  result.GetType(), count,
                  DesemCast(what), nullptr
               };
            }
            else {
               LANGULUS_ERROR(
                  "Can't wrap a bounded array inside incompatible TMany<T>:"
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
               new (&result) Block {
                  DataState::Constrained,
                  result.GetType(), count,
                  DesemCast(what), nullptr
               };
            }
            else {
               LANGULUS_ERROR(
                  "Can't wrap a unbounded array inside incompatible TMany<T>:"
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
               new (&result) Block {
                  DataState::Constrained,
                  result.GetType(), count,
                  &DesemCast(what), nullptr
               };
            }
            else {
               LANGULUS_ERROR(
                  "Can't wrap a dense element inside incompatible TMany<T>:"
                  " types are not binary compatible"
               );
            }
         }
      }
      else LANGULUS_ERROR("Can't manually interface a sparse block");

      if constexpr (not S::Move and S::Keep)
         result.TakeAuthority<TMany>();
      return result;
   }

   /// Get the static type of the container                                   
   /// Also initializes the type of this container                            
   ///   @attention this should not be called at static initialization time   
   ///   @return the meta definition of the type                              
   TEMPLATE() LANGULUS(INLINED)
   DMeta TMany<T>::GetType() const noexcept {
      return Block::GetType<TMany>();
   }

   /// Get the name of the contained type                                     
   ///   @return the name of the contained type                               
   TEMPLATE() LANGULUS(INLINED)
   constexpr Token TMany<T>::GetToken() const noexcept {
      return Block::GetToken<TMany>();
   }

   /// Refer assignment                                                       
   ///   @param rhs - the container to refer to                               
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator = (const TMany& rhs) {
      static_assert(CT::DeepAssignable<T, Referred<TMany<T>>>);
      return operator = (Refer(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the container to move                                   
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator = (TMany&& rhs) {
      static_assert(CT::DeepAssignable<T, Moved<TMany<T>>>);
      return operator = (Move(rhs));
   }

   /// Generic assignment                                                     
   ///   @param rhs - the element/array/container to assign                   
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepAssignable<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator = (T1&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Block<ST>) {
         // Potentially absorb a container                              
         if (static_cast<const Block*>(this)
          == static_cast<const Block*>(&DesemCast(rhs)))
            return *this;

         Block::Free<TMany>();
         new (this) TMany {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Clear();
         Block::UnfoldInsert<TMany, void, true>(IndexBack, S::Nest(rhs));
      }

      return *this;
   }

   /// Reset container state (inner function)                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr void TMany<T>::ResetState() noexcept {
      Block::ResetState<TMany>();
   }
   
   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TMany<T>::Is() const noexcept {
      return Block::Is<TMany, T1, TN...>();
   }

   /// Check if type origin is the same as one of the provided types          
   ///   @attention ignores sparsity and cv-qualifiers                        
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data similar to 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TMany<T>::Is(DMeta type) const noexcept {
      return Block::Is<TMany>(type);
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TMany<T>::IsSimilar() const noexcept {
      return Block::IsSimilar<TMany, T1, TN...>();
   }

   /// Check if unqualified type is the same as one of the provided types     
   ///   @attention ignores only cv-qualifiers                                
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data similar to 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TMany<T>::IsSimilar(DMeta type) const noexcept {
      return Block::IsSimilar<TMany>(type);
   }

   /// Check if this type is exactly one of the provided types                
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if data type matches at least one type                  
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TMany<T>::IsExact() const noexcept {
      return Block::IsExact<TMany, T1, TN...>();
   }

   /// Check if this type is exactly one of the provided types                
   ///   @param type - the type to check for                                  
   ///   @return if this block contains data of exactly 'type'                
   TEMPLATE() LANGULUS(INLINED)
   bool TMany<T>::IsExact(DMeta type) const noexcept {
      return Block::IsExact<TMany>(type);
   }

   /// Check if contained data can be interpreted as a given type             
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE() template<bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool TMany<T>::CastsToMeta(DMeta type) const {
      return Block::CastsToMeta<BINARY_COMPATIBLE>(type);
   }

   /// Check if contained data can be interpreted as a given count of type    
   /// For example: a Vec4 can interpret as float[4]                          
   /// Beware, direction matters (this is the inverse of CanFit)              
   ///   @param type - the type check if current type interprets to           
   ///   @param count - the number of elements to interpret as                
   ///   @return true if able to interpret current type to 'type'             
   TEMPLATE() template<bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool TMany<T>::CastsToMeta(DMeta type, const Count count) const {
      return Block::CastsToMeta<BINARY_COMPATIBLE>(type, count);
   }
   
   /// Check if this container's data can be represented as type T            
   /// with nothing more than pointer arithmetic                              
   ///   @tparam ALT_T - the type to compare against                          
   ///   @tparam BINARY_COMPATIBLE - do we require for the type to be         
   ///      binary compatible with this container's type                      
   ///   @return true if contained data is reinterpretable as T               
   TEMPLATE() template<CT::Data ALT_T, bool BINARY_COMPATIBLE> LANGULUS(INLINED)
   bool TMany<T>::CastsTo() const {
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
   bool TMany<T>::CastsTo(const Count count) const {
      return Block::CastsTo<ALT_T, BINARY_COMPATIBLE>(count);
   }

   /// Reinterpret contents of this Block as the type and state of another    
   /// You can interpret Vec4 as float[4] for example, or any other such      
   /// reinterpretation, as long as data remains tightly packed and aligned   
   /// No real conversion is performed, only pointer arithmetic               
   ///   @param pattern - the type of data to try interpreting as             
   ///   @return a block representing this block, interpreted as the pattern  
   TEMPLATE() template<CT::Block B> LANGULUS(INLINED)
   B TMany<T>::ReinterpretAs(const B& rhs) const {
      return Block::ReinterpretAs<TMany, B>(rhs);
   }

   TEMPLATE() template<CT::Data T1> LANGULUS(INLINED)
   TMany<T1> TMany<T>::ReinterpretAs() const {
      return Block::ReinterpretAs<T1, TMany>();
   }

   /// Get the memory block corresponding to a local member variable          
   ///   @attention assumes block is not empty                                
   ///   @param member - the member to get                                    
   ///   @param idx - the element to get member from                          
   ///   @return a static memory block                                        
   TEMPLATE() LANGULUS(INLINED)
   Block TMany<T>::GetMember(const RTTI::Member& member, CT::Index auto idx) {
      return Block::GetMember<TMany>(member, idx);
   }

   TEMPLATE() LANGULUS(INLINED)
   Block TMany<T>::GetMember(const RTTI::Member& member, CT::Index auto idx) const {
      return Block::GetMember<TMany>(member, idx);
   }

   /// Allocate 'count' elements and fill the container with zeroes           
   /// If T is not CT::Nullifiable, this function does default construction,  
   /// which would be slower, than batch zeroing                              
   TEMPLATE() LANGULUS(INLINED)
   void TMany<T>::Null(Count count) {
      return Block::Null<TMany>(count);
   }
   
   /// Never allocate new elements, instead assign all currently initialized  
   /// elements a single value                                                
   ///   @param what - the value to assign                                    
   TEMPLATE() template<class A> requires CT::AssignableFrom<T, A>
   LANGULUS(INLINED)
   void TMany<T>::Fill(A&& what){
      return Block::Fill<TMany>(Forward<Deref<decltype(what)>>(what));
   }

   /// Clear the container, destroying all elements,                          
   /// but retaining allocation if possible                                   
   TEMPLATE() LANGULUS(INLINED)
   void TMany<T>::Clear() {
      Block::Clear<TMany>();
   }

   /// Reset the container, destroying all elements, and deallocating         
   TEMPLATE() LANGULUS(INLINED)
   void TMany<T>::Reset() {
      Block::Reset<TMany>();
   }
   
   /// Get the raw data inside the container                                  
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @return a pointer to the first allocated element                     
   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   constexpr auto TMany<T>::GetRaw() noexcept {
      return Block::GetRaw<THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   constexpr auto TMany<T>::GetRaw() const noexcept {
      return Block::GetRaw<THIS>();
   }

   /// Get the end raw data pointer inside the container (const)              
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @attention the resulting pointer never points to a valid element     
   ///   @return a pointer to the last+1 element (never initialized)          
   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   constexpr auto TMany<T>::GetRawEnd() const noexcept {
      return Block::GetRawEnd<THIS>();
   }

   /// Get a pointer array - useful only for sparse containers                
   ///   @return the raw data as an array of pointers                         
   TEMPLATE() template<class THIS> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   auto TMany<T>::GetRawSparse() IF_UNSAFE(noexcept) {
      return Block::GetRawSparse<THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED) IF_UNSAFE(constexpr)
   auto TMany<T>::GetRawSparse() const IF_UNSAFE(noexcept) {
      return Block::GetRawSparse<THIS>();
   }
   
   /// Get a pointer array - useful only for sparse containers                
   ///   @tparam T - the type (dense) to interpret pointers as                
   ///   @return the pointer to the first pointer of T                        
   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   T1** TMany<T>::GetRawSparseAs() IF_UNSAFE(noexcept) {
      return Block::GetRawSparseAs<T1, THIS>();
   }

   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   const T1* const* TMany<T>::GetRawSparseAs() const IF_UNSAFE(noexcept) {
      return Block::GetRawSparseAs<T1, THIS>();
   }
   
   /// Get the raw data inside the container, reinterpreted as some type      
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @tparam T - the type we're interpreting as                           
   ///   @return a pointer to the first element of type T                     
   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   T1* TMany<T>::GetRawAs() noexcept {
      return Block::GetRawAs<T1, THIS>();
   }

   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   const T1* TMany<T>::GetRawAs() const noexcept {
      return Block::GetRawAs<T1, THIS>();
   }

   /// Get the end raw data pointer inside the container                      
   ///   @attention never points to a valid element                           
   ///   @attention as unsafe as it gets, but as fast as it gets              
   ///   @tparam T - the type we're interpreting as                           
   ///   @return a pointer to the last+1 element of type T                    
   TEMPLATE() template<CT::Data T1, class THIS> LANGULUS(INLINED)
   const T1* TMany<T>::GetRawEndAs() const noexcept {
      return Block::GetRawEndAs<T1, THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   const Allocation* const* TMany<T>::GetEntries() const IF_UNSAFE(noexcept) {
      return Block::GetEntries<THIS>();
   }

   TEMPLATE() template<class THIS> LANGULUS(INLINED)
   const Allocation** TMany<T>::GetEntries() IF_UNSAFE(noexcept) {
      return Block::GetEntries<THIS>();
   }

   /// Get an element in the way you want (unsafe)                            
   /// This is a statically optimized variant of Block::Get                   
   ///   @tparam ALT_T - how to interpret data                                
   ///   @param index - the element index to retrieve                         
   ///   @return a reference/pointer to the element, depending on ALT_T       
   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   decltype(auto) TMany<T>::Get(const Offset index) const noexcept {
      return Block::Get<ALT_T>(index);
   }

   TEMPLATE() template<CT::Data ALT_T> LANGULUS(INLINED)
   decltype(auto) TMany<T>::Get(const Offset index) noexcept {
      return Block::Get<ALT_T>(index);
   }

   /// Access typed dense elements by index                                   
   ///   @param idx - the index to get                                        
   ///   @return a reference to the element                                   
   TEMPLATE() LANGULUS(INLINED)
   const T& TMany<T>::operator [] (const CT::Index auto index) const {
      return Block::operator [] <TMany> (index);
   }

   TEMPLATE() LANGULUS(INLINED)
   T& TMany<T>::operator [] (const CT::Index auto index) {
      return Block::operator [] <TMany> (index);
   }
   
   /// Get an element at an index, trying to interpret it as T                
   /// No conversion or copying shall occur in this routine, only pointer     
   /// arithmetic based on CTTI or RTTI                                       
   ///   @tparam T1 - the type to interpret to                                
   ///   @param index - the index                                             
   ///   @return either pointer or reference to the element (depends on T)    
   TEMPLATE() template<CT::Data T1>
   decltype(auto) TMany<T>::As(CT::Index auto index) {
      if constexpr (CT::Deep<T1>) {
         // Optimize if we're interpreting as a container               
         static_assert(CT::Deep<Decay<T>>, "Type mismatch");
         const auto idx = SimplifyIndex<TMany<T>>(index);
         LANGULUS_ASSERT(idx < mCount, Access, "Index out of range");
         Block& result = Get<Block>(idx);

         if constexpr (CT::Typed<T1>) {
            // Additional check, if T is a typed block                  
            if (not result.template IsSimilar<Many, TypeOf<T1>>())
               LANGULUS_THROW(Access, "Deep type mismatch");
         }

         if constexpr (CT::Sparse<T1>)
            return reinterpret_cast<T1>(&result);
         else
            return reinterpret_cast<T1&>(result);
      }
      else if constexpr (CT::Sparse<T1>
      and requires (Decay<T>* e) { dynamic_cast<T1>(e); }) {
         // Do a dynamic_cast whenever possible                         
         const auto idx = SimplifyIndex<TMany<T>>(index);
         LANGULUS_ASSERT(idx < mCount, Access, "Index out of range");

         Decvq<T1> ptr;
         if constexpr (CT::Sparse<T>)
            ptr = dynamic_cast<T1>(Get<T>(idx));
         else
            ptr = dynamic_cast<T1>(Get<T*>(idx));
         LANGULUS_ASSERT(ptr, Access, "Failed dynamic_cast");
         return ptr;
      }
      else if constexpr (requires (Decay<T>* e) { static_cast<Decay<T1>*>(e); }) {
         // Do a quick static_cast whenever possible                    
         const auto idx = SimplifyIndex<TMany<T>>(index);
         LANGULUS_ASSERT(idx < mCount, Access, "Index out of range");

         if constexpr (CT::Sparse<T1>) {
            if constexpr (CT::Sparse<T>)
               return static_cast<T1>(Get<T>(idx));
            else
               return static_cast<T1>(Get<T*>(idx));
         }
         else {
            if constexpr (CT::Sparse<T>)
               return static_cast<T1&>(*Get<T>(idx));
            else
               return static_cast<T1&>(Get<T>(idx));
         }
      }
      else LANGULUS_ERROR("Type mismatch");
   }

   TEMPLATE() template<CT::Data T1> LANGULUS(INLINED)
   decltype(auto) TMany<T>::As(CT::Index auto index) const {
      return const_cast<TMany&>(*this).As<T1>(index);
   }

   /// Get the handle of an element                                           
   TEMPLATE() LANGULUS(INLINED)
   Handle<T> TMany<T>::GetHandle(Offset i) IF_UNSAFE(noexcept) {
      return Block::GetHandle<T, TMany<T>>(i);
   }

   TEMPLATE() LANGULUS(INLINED)
   Handle<const T> TMany<T>::GetHandle(Offset i) const IF_UNSAFE(noexcept) {
      return Block::GetHandle<T, TMany<T>>(i);
   }

   /// Access last element                                                    
   ///   @attention assumes container has at least one item                   
   ///   @return a mutable reference to the last element                      
   TEMPLATE() LANGULUS(INLINED)
   T& TMany<T>::Last() {
      return Block::Last<TMany>();
   }

   TEMPLATE() LANGULUS(INLINED)
   const T& TMany<T>::Last() const {
      return Block::Last<TMany>();
   }
   
   /// Templated Many containers are always typed                             
   ///   @return true                                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsTyped() const noexcept {
      return true;
   }
   
   /// Templated Many containers are never untyped                            
   ///   @return false                                                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsUntyped() const noexcept {
      return false;
   }
   
   /// Templated Many containers are always type-constrained                  
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsTypeConstrained() const noexcept {
      return true;
   }
      
   /// Check if contained type is deep                                        
   /// This is a statically optimized alternative to Block::IsDeep            
   ///   @return true if this container contains deep items                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsDeep() const noexcept {
      return Block::IsDeep<TMany>();
   }

   /// Check if the contained type is a pointer                               
   /// This is a statically optimized alternative to Block::IsSparse          
   ///   @return true if container contains pointers                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsSparse() const noexcept {
      return Block::IsSparse<TMany>();
   }

   /// Check if the contained type is not a pointer                           
   /// This is a statically optimized alternative to Block::IsDense           
   ///   @return true if container contains sequential data                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsDense() const noexcept {
      return Block::IsDense<TMany>();
   }

   /// Check if block contains POD items - if so, it's safe to directly copy  
   /// raw memory from container. Note, that this doesn't only consider the   
   /// standard c++ type traits, like trivially_constructible. You also need  
   /// to explicitly reflect your type with LANGULUS(POD) true;               
   /// This gives a lot more control over your code                           
   /// This is a statically optimized alternative to Block::IsPOD             
   ///   @return true if contained data is plain old data                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsPOD() const noexcept {
      return Block::IsPOD<TMany>();
   }

   /// Check if block contains resolvable items, that is, items that have a   
   /// GetBlock() function, that can be used to represent themselves as their 
   /// most concretely typed block                                            
   /// This is a statically optimized alternative to Block::IsResolvable      
   ///   @return true if contained data can be resolved on element basis      
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsResolvable() const noexcept {
      return Block::IsResolvable<TMany>();
   }

   /// Get the size of a single contained element, in bytes                   
   /// This is a statically optimized alternative to Block::GetStride         
   ///   @return the number of bytes a single element contains                
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TMany<T>::GetStride() const noexcept {
      return Block::GetStride<TMany>();
   }
   
   /// Get the size of all elements, in bytes                                 
   ///   @return the total amount of initialized bytes                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr Size TMany<T>::GetBytesize() const noexcept {
      return Block::GetBytesize<TMany>();
   }

   /// Get the number of sub-blocks (this one included)                       
   ///   @return the number of contained blocks, including this one           
   TEMPLATE() LANGULUS(INLINED)
   constexpr Count TMany<T>::GetCountDeep() const noexcept {
      return Block::GetCountDeep<TMany>();
   }

   /// Get the sum of initialized non-deep elements in all sub-blocks         
   ///   @return the number of contained non-deep elements                    
   TEMPLATE() LANGULUS(INLINED)
   constexpr Count TMany<T>::GetCountElementsDeep() const noexcept {
      return Block::GetCountElementsDeep<TMany>();
   }

   /// Deep (slower) check if there's anything missing inside nested blocks   
   ///   @return true if any deep or flat memory block contains missing data  
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsMissingDeep() const {
      return Block::IsMissingDeep<TMany>();
   }

   /// Check if a memory block can be concatenated to this one                
   ///   @param b - the block to concatenate                                  
   ///   @return true if able to concatenate to this one                      
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsConcatable(const CT::Block auto& b) const noexcept {
      return Block::IsConcatable<TMany>(b);
   }

   /// Check if a type can be inserted to this block                          
   ///   @param other - check if a given type is insertable to this block     
   ///   @return true if able to insert an instance of the type to this block 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TMany<T>::IsInsertable(DMeta type) const noexcept {
      return Block::IsInsertable<TMany>(type);
   }

   /// Check if a static type can be inserted                                 
   ///   @return true if able to insert an instance of the type to this block 
   TEMPLATE() template<CT::Data T1> LANGULUS(INLINED)
   constexpr bool TMany<T>::IsInsertable() const noexcept {
      return Block::IsInsertable<T1, TMany>();
   }

   /// Unfold-insert item(s) at an index, semantically or not                 
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the index to insert at                                
   ///   @param t1 - the first element                                        
   ///   @param tn - the rest of the elements (optional)                      
   ///   @return number of inserted items                                     
   TEMPLATE() template<bool MOVE_ASIDE, class T1, class...TN>
   requires CT::UnfoldMakableFrom<T, T1, TN...> LANGULUS(INLINED)
   Count TMany<T>::Insert(CT::Index auto index, T1&& t1, TN&&...tn) {
      return Block::Insert<TMany, Many, MOVE_ASIDE>(
         index, Forward<T1>(t1), Forward<TN>(tn)...);
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
   Count TMany<T>::InsertBlock(CT::Index auto index, T1&& other) {
      return Block::InsertBlock<TMany, FORCE, MOVE_ASIDE>(
         index, Forward<T1>(other));
   }

   /// Merge elements                                                         
   /// Element will be pushed only if not found in block                      
   ///   @tparam MOVE_ASIDE - true to allocate more elements, and move any    
   ///      elements at index to the right, in order to fit the insertion     
   ///   @param index - the index at which to insert                          
   ///   @param t1 - the first item to insert                                 
   ///   @param tn... - the rest of items to insert (optional)                
   ///   @return the number of inserted items                                 
   TEMPLATE() template<bool MOVE_ASIDE, class T1, class...TN>
   requires CT::UnfoldMakableFrom<T, T1, TN...> LANGULUS(INLINED)
   Count TMany<T>::Merge(CT::Index auto index, T1&& t1, TN&&...tn) {
      return Block::Merge<TMany, Many, MOVE_ASIDE>(
         index, Forward<T1>(t1), Forward<TN>(tn)...);
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
   Count TMany<T>::MergeBlock(CT::Index auto index, T1&& other) {
      return Block::MergeBlock<TMany, FORCE, MOVE_ASIDE>(
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
   TMany<T>::Emplace(CT::Index auto at, A&&...arguments) {
      Block::Emplace<TMany, MOVE_ASIDE>(at, Forward<A>(arguments)...);
      return Get(mCount - 1);
   }

   /// Create N new elements, using default construction                      
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @return the number of new elements                                   
   TEMPLATE() LANGULUS(INLINED)
   Count TMany<T>::New(const Count count) requires CT::Defaultable<T> {
      return Block::New<TMany>(count);
   }

   /// Create N new elements, using the provided arguments for construction   
   /// Elements will be added to the back of the container                    
   ///   @param count - number of elements to construct                       
   ///   @param arguments... - constructor arguments, all forwarded together  
   ///      for each instance of T                                            
   ///   @return the number of new elements                                   
   TEMPLATE() template<class...A>
   requires ::std::constructible_from<T, A...> LANGULUS(INLINED)
   Count TMany<T>::New(const Count count, A&&...arguments) {
      return Block::New<TMany>(count, Forward<A>(arguments)...);
   }

   /// Insert an element at the back of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator << (T1&& rhs) {
      Insert(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Insert an element at the front of the container                        
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator >> (T1&& rhs) {
      Insert(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the back of the container                          
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator <<= (T1&& rhs) {
      Merge(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the front of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator >>= (T1&& rhs) {
      Merge(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Find element index inside container                                    
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @param cookie - resume search from a given index                     
   ///   @return the index of the found item, or IndexNone if none found      
   TEMPLATE() template<bool REVERSE, CT::NotSemantic T1>
   requires CT::Comparable<T, T1>
   Index TMany<T>::Find(const T1& item, Offset cookie) const noexcept {
      return Block::Find<REVERSE, TMany>(item, cookie);
   }

   /// Find a sequence of one or more elements inside container               
   ///   @tparam REVERSE - true to perform search in reverse                  
   ///   @param item - the item to search for                                 
   ///   @param cookie - resume search from a given index                     
   ///   @return the index of the found item, or IndexNone if none found      
   TEMPLATE() template<bool REVERSE>
   Index TMany<T>::FindBlock(const CT::Block auto& item, CT::Index auto cookie) const noexcept {
      return Block::FindBlock<REVERSE, TMany>(item, cookie);
   }

   /// Remove matching items by value                                         
   ///   @tparam REVERSE - whether to search in reverse order                 
   ///   @param item - the item to search for to remove                       
   ///   @return the number of removed items                                  
   TEMPLATE() template<bool REVERSE, CT::Data ALT_T> LANGULUS(INLINED)
   Count TMany<T>::Remove(const ALT_T& item) {
      const auto found = Find<REVERSE>(item);
      return found ? RemoveIndex(found.GetOffsetUnsafe(), 1) : 0;
   }

   /// Remove sequential raw indices in a given range                         
   ///   @attention assumes starter + count <= mCount                         
   ///   @param index - index to start removing from                          
   ///   @param count - number of elements to remove                          
   ///   @return the number of removed elements                               
   TEMPLATE()
   Count TMany<T>::RemoveIndex(CT::Index auto index, Count count) {
      return Block::RemoveIndex<TMany>(index, count);
   }
   
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this TMany instance        
   ///   @param index - the index to start removing at                        
   ///   @param count - number of elements to remove                          
   ///   @return the iterator of the previous element, unless index is first  
   TEMPLATE()
   typename TMany<T>::Iterator TMany<T>::RemoveIt(const Iterator& index, Count count) {
      return Block::RemoveIt<TMany>(index, count);
   }

   /// Sort the pack                                                          
   TEMPLATE() template<bool ASCEND> requires CT::Sortable<T>
   LANGULUS(INLINED) void TMany<T>::Sort() {
      Block::Sort<ASCEND, TMany>();
   }

   /// Remove elements on the back                                            
   ///   @param count - the new count                                         
   TEMPLATE() LANGULUS(INLINED)
   void TMany<T>::Trim(Count count) {
      Block::Trim<TMany>(count);
   }

   /// Swap two elements                                                      
   ///   @param from - the first element                                      
   ///   @param to - the second element                                       
   TEMPLATE() LANGULUS(INLINED)
   void TMany<T>::Swap(CT::Index auto from, CT::Index auto to) {
      Many::Swap<T>(from, to);
   }

   /// Gather items from source container, and fill this one                  
   ///   @tparam REVERSE - iterate in reverse?                                
   ///   @param source - container to gather from, type acts as filter        
   ///   @return the number of gathered elements                              
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TMany<T>::GatherFrom(const CT::Block auto& source) {
      return source.template GatherInner<REVERSE>(*this);
   }

   /// Gather items of specific state from source container, and fill this one
   ///   @tparam REVERSE - iterate in reverse?                                
   ///   @param source - container to gather from, type acts as filter        
   ///   @param state - state filter                                          
   ///   @return the number of gathered elements                              
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TMany<T>::GatherFrom(const CT::Block auto& source, DataState state) {
      return source.template GatherPolarInner<REVERSE>(GetType(), *this, state);
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Crop(Offset start, Count count) const {
      return Block::Crop<TMany>(start, count);
   }
   
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Crop(Offset start, Count count) {
      return Block::Crop<TMany>(start, count);
   }
     
   /// Iterate each element block and execute F for it                        
   ///   @tparam REVERSE - whether to iterate in reverse                      
   ///   @param call - function to execute for each element block             
   ///   @return the number of executions                                     
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TMany<T>::ForEachElement(auto&& call) {
      return Block::ForEachElement<REVERSE, TMany>(
         Forward<Deref<decltype(call)>>(call));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TMany<T>::ForEachElement(auto&& call) const {
      return Block::ForEachElement<REVERSE, const TMany>(
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
   Count TMany<T>::ForEach(auto&&...call) {
      return Block::ForEach<REVERSE, TMany>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TMany<T>::ForEach(auto&&...call) const {
      return Block::ForEach<REVERSE, const TMany>(
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
   Count TMany<T>::ForEachDeep(auto&&...call) {
      return Block::ForEachDeep<REVERSE, SKIP, TMany>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TMany<T>::ForEachDeep(auto&&...call) const {
      return Block::ForEachDeep<REVERSE, SKIP, const TMany>(
         Forward<Deref<decltype(call)>>(call)...);
   }

   /// Reserve a number of elements without initializing them                 
   ///   @tparam SETSIZE - whether or not to set size, too                    
   ///   @attention using SETSIZE will NOT construct any elements, use only   
   ///      if you know what you're doing                                     
   ///   @param count - number of elements to reserve                         
   TEMPLATE() template<bool SETSIZE> LANGULUS(INLINED)
   void TMany<T>::Reserve(const Count count) {
      Block::Reserve<SETSIZE, TMany>(count);
   }
   
   /// Extend the container via default construction, and return the new part 
   ///   @param count - the number of elements to extend by                   
   ///   @return a container that represents only the extended part           
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Extend(const Count count) {
      return Block::Extend<TMany>(count);
   }
   
   /// Compare with another container, order matters                          
   ///   @param other - container to compare with                             
   ///   @return true if both containers match completely                     
   TEMPLATE() template<bool RESOLVE>
   bool TMany<T>::Compare(const CT::Block auto& other) const {
      return Block::Compare<RESOLVE, TMany>(other);
   }

   /// Compare with anything that isn't a Block type                          
   ///   @param other - the block to compare with                             
   ///   @return true if both containers are identical                        
   TEMPLATE() template<CT::NotSemantic T1>
   requires (not CT::Block<T1> and CT::Comparable<T, T1> and CT::NotOwned<T1>)
   LANGULUS(INLINED) bool TMany<T>::operator == (const T1& other) const {
      return Block::operator == <TMany> (other);
   }

   /// Compare with any other kind of block                                   
   ///   @param other - the block to compare with                             
   ///   @return true if both containers are identical                        
   TEMPLATE() template<CT::NotSemantic T1>
   requires (CT::UntypedBlock<T1> or (CT::TypedBlock<T1> and CT::Comparable<T, TypeOf<T1>>))
   LANGULUS(INLINED)
   bool TMany<T>::operator == (const T1& other) const {
      return Block::operator == <TMany> (other);
   }

   /// Compare loosely with another TMany, ignoring case                       
   /// This function applies only if T is character                           
   ///   @param other - text to compare with                                  
   ///   @return true if both containers match loosely                        
   TEMPLATE()
   bool TMany<T>::CompareLoose(const CT::Block auto& other) const noexcept {
      return Block::CompareLoose<TMany>(other);
   }

   /// Count how many consecutive elements match in two containers            
   ///   @param other - container to compare with                             
   ///   @return the number of matching items                                 
   TEMPLATE()
   Count TMany<T>::Matches(const CT::Block auto& other) const noexcept {
      return Block::Matches<TMany>(other);
   }

   /// Compare loosely with another, ignoring upper-case                      
   /// Count how many consecutive letters match in two strings                
   ///   @param other - text to compare with                                  
   ///   @return the number of matching symbols                               
   TEMPLATE()
   Count TMany<T>::MatchesLoose(const CT::Block auto& other) const noexcept {
      return Block::MatchesLoose<TMany>(other);
   }
  
   /// Hash data inside memory block                                          
   ///   @attention order matters, so you might want to Neat data first       
   ///   @return the hash                                                     
   TEMPLATE() LANGULUS(INLINED)
   Hash TMany<T>::GetHash() const requires CT::Hashable<T> {
      return Block::GetHash<TMany>();
   }

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename TMany<T>::Iterator TMany<T>::begin() noexcept {
      return Block::begin<TMany>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TMany<T>::ConstIterator TMany<T>::begin() const noexcept {
      return Block::begin<TMany>();
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename TMany<T>::Iterator TMany<T>::last() noexcept {
      return Block::last<TMany>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TMany<T>::ConstIterator TMany<T>::last() const noexcept {
      return Block::last<TMany>();
   }

   /// Concatenate anything, semantically or not                              
   ///   @param rhs - the element/block/array to copy-concatenate             
   ///   @return a new container, containing both blocks                      
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TMany<T> TMany<T>::operator + (T1&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      return Block::ConcatBlock<TMany>(S::Nest(rhs));
   }

   /// Concatenate destructively, semantically or not                         
   ///   @param rhs - the element/block/array to semantically concatenate     
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator += (T1&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      Block::InsertBlock<TMany, void>(IndexBack, S::Nest(rhs));
      return *this;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE