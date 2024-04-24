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

   /// Never allocate new elements, instead assign all currently initialized  
   /// elements a single value                                                
   ///   @param what - the value to assign                                    
   TEMPLATE() template<class A> requires CT::AssignableFrom<T, A>
   LANGULUS(INLINED)
   void TMany<T>::Fill(A&& what){
      return Block::Fill<TMany>(Forward<Deref<decltype(what)>>(what));
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
      return Base::Crop<TMany>(start, count);
   }
   
   TEMPLATE() LANGULUS(INLINED)
   TMany<T> TMany<T>::Crop(Offset start, Count count) {
      return Base::Crop<TMany>(start, count);
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