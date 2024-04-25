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
                  Base::BlockTransfer(S::Nest(t1));
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
                  else Base::Insert(IndexBack, Forward<T1>(t1));
               }
               else Base::Insert(IndexBack, Forward<T1>(t1));
            }
            else if constexpr (CT::Deep<ST>) {
               // Type-erased block, do run-time type checks            
               if (IsSimilar(DesemCast(t1).GetType())) {
                  // If types are similar, it is safe to                
                  // absorb the block, essentially converting a type-   
                  // erased Many back to its TMany equivalent           
                  Base::BlockTransfer(S::Nest(t1));
               }
               else if constexpr (CT::Deep<T>) {
                  // This TMany accepts any kind of deep element        
                  Base::Insert(IndexBack, Forward<T1>(t1));
               }
               else if constexpr (CT::Typed<ST> and CT::MakableFrom<T, typename S::template As<TypeOf<ST>>>) {
                  // Attempt converting all elements to T               
                  Base::InsertBlock(IndexBack, Forward<T1>(t1));
               }
               else LANGULUS_OOPS(Meta, "Unable to absorb block");
            }
            else LANGULUS_ERROR("Can't construct this TMany from this kind of Block");
         }
         else Base::Insert(IndexBack, Forward<T1>(t1));
      }
      else Base::Insert(IndexBack, Forward<T1>(t1), Forward<TN>(tn)...);
   }

   /// Destructor                                                             
   TEMPLATE() LANGULUS(INLINED)
   TMany<T>::~TMany() {
      Base::Free();
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
      return Base::From<TMany>(Forward(what), count);
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
      /*using S  = SemanticOf<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Block<ST>) {
         // Potentially absorb a container                              
         if (static_cast<const A::Block*>(this)
          == static_cast<const A::Block*>(&DesemCast(rhs)))
            return *this;

         Base::Free();
         new (this) TMany {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Base::Clear();
         Base::template UnfoldInsert<void, true>(IndexBack, S::Nest(rhs));
      }*/
      Base::BlockAssign(Forward<T1>(rhs));
      return *this;
   }

   /// Insert an element at the back of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator << (T1&& rhs) {
      Base::Insert(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Insert an element at the front of the container                        
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator >> (T1&& rhs) {
      Base::Insert(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the back of the container                          
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator <<= (T1&& rhs) {
      Base::Merge(IndexBack, Forward<T1>(rhs));
      return *this;
   }

   /// Merge an element at the front of the container                         
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this container for chaining                   
   TEMPLATE() template<class T1>
   requires CT::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator >>= (T1&& rhs) {
      Base::Merge(IndexFront, Forward<T1>(rhs));
      return *this;
   }

   /// Pick a constant region and reference it from another container         
   ///   @param start - starting element index                                
   ///   @param count - number of elements                                    
   ///   @return the container                                                
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Crop(Offset start, Count count) const {
      return Base::Crop<TMany>(start, count);
   }
   
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Crop(Offset start, Count count) {
      return Base::Crop<TMany>(start, count);
   }
   
   /// Extend the container via default construction, and return the new part 
   ///   @param count - the number of elements to extend by                   
   ///   @return a container that represents only the extended part           
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Extend(const Count count) {
      return Base::Extend<TMany>(count);
   }
  
   /// Concatenate anything, semantically or not                              
   ///   @param rhs - the element/block/array to copy-concatenate             
   ///   @return a new container, containing both blocks                      
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TMany<T> TMany<T>::operator + (T1&& rhs) const {
      using S = SemanticOf<decltype(rhs)>;
      return Base::ConcatBlock<TMany>(S::Nest(rhs));
   }

   /// Concatenate destructively, semantically or not                         
   ///   @param rhs - the element/block/array to semantically concatenate     
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepMakable<T, T1> LANGULUS(INLINED)
   TMany<T>& TMany<T>::operator += (T1&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      Base::template InsertBlock<void>(IndexBack, S::Nest(rhs));
      return *this;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE