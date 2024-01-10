///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TSet.hpp"
#include "Set.inl"

#define TEMPLATE() template<CT::Data T, bool ORDERED>
#define TABLE() TSet<T, ORDERED>


namespace Langulus::Anyness
{

   /// Default construction                                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr TABLE()::TSet() {
      mKeys.mState = DataState::Typed;
      if constexpr (CT::Constant<T>)
         mKeys.MakeConst();
   }

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::TSet(const TSet& other)
      : TSet {Copy(other)} {}

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::TSet(TSet&& other)
      : TSet {Move(other)} {}
   
   /// Create from a list of elements, each of them can be semantic or not,   
   /// an array, as well as any other kinds of sets                           
   ///   @param t1 - first element                                            
   ///   @param tail - tail of elements (optional)                            
   TEMPLATE() template<class T1, class...TAIL>
   requires CT::DeepSetMakable<T, T1, TAIL...> LANGULUS(INLINED)
   TABLE()::TSet(T1&& t1, TAIL&&... tail) {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using ST = TypeOf<S>;

         if constexpr (CT::Set<ST>) {
            if constexpr (CT::Typed<ST>) {
               // Not type-erased set, do compile-time type checks      
               using STT = TypeOf<ST>;
               if constexpr (CT::Similar<T, STT>) {
                  // Type is binary compatible, just transfer set       
                  BlockTransfer<TSet>(S::Nest(t1));
               }
               else if constexpr (CT::Sparse<T, STT>) {
                  if constexpr (CT::DerivedFrom<T, STT>) {
                     // The statically typed set contains items that    
                     // are base of this container's type. Each element 
                     // should be dynamically cast to this type         
                     for (auto pointer : DesemCast(t1)) {
                        auto dcast = dynamic_cast<T>(&(*pointer));
                        if (dcast)
                           (*this) << dcast;
                     }
                  }
                  else if constexpr (CT::DerivedFrom<STT, T>) {
                     // The statically typed set contains items that    
                     // are derived from this container's type. Each    
                     // element should be statically sliced to this type
                     for (auto pointer : DesemCast(t1))
                        (*this) << static_cast<T>(&(*pointer));
                  }
                  else Insert(Forward<T1>(t1));
               }
               else Insert(Forward<T1>(t1));
            }
            else {
               // Type-erased set, do run-time type checks              
               if (mKeys.mType == DesemCast(t1).GetType()) {
                  // If types are exactly the same, it is safe to       
                  // absorb the set, essentially converting a type-     
                  // erased Set back to its TSet equivalent             
                  BlockTransfer<TSet>(S::Nest(t1));
               }
               else Insert(Forward<T1>(t1));
            }
         }
         else Insert(Forward<T1>(t1));
      }
      else Insert(Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Destroys the set and all it's contents                                 
   TEMPLATE()
   TABLE()::~TSet() {
      Free<TSet>();
   }

   /// Shallow-copy assignment                                                
   ///   @param rhs - the container to shallow-copy                           
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const TSet& rhs) {
      static_assert(CT::DeepSetAssignable<T, Copied<TSet<T>>>);
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the container to move                                   
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (TSet&& rhs) {
      static_assert(CT::DeepSetAssignable<T, Moved<TSet<T>>>);
      return operator = (Move(rhs));
   }

   /// Generic assignment                                                     
   ///   @param rhs - the element/array/container to assign                   
   ///   @return a reference to this container                                
   TEMPLATE() template<class T1>
   requires CT::DeepSetAssignable<T, T1> LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (T1&& rhs) {
      using S = SemanticOf<T1>;
      using ST = TypeOf<S>;
       
      if constexpr (CT::Set<ST>) {
         // Potentially absorb a set                                    
         if (static_cast<const BlockSet*>(this)
          == static_cast<const BlockSet*>(&DesemCast(rhs)))
            return *this;

         Free<TSet>();
         new (this) TSet {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Clear();
         UnfoldInsert(S::Nest(rhs));
      }

      return *this;
   }

   /// Templated tables are always typed                                      
   ///   @return false                                                        
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsUntyped() const noexcept {
      return false;
   }
   
   /// Templated tables are always type-constrained                           
   ///   @return true                                                         
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsTypeConstrained() const noexcept {
      return true;
   }
   
   /// Check if key type is deep                                              
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsDeep() const noexcept {
      return CT::Deep<T>;
   }

   /// Check if the key type is a pointer                                     
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsSparse() const noexcept {
      return CT::Sparse<T>;
   }

   /// Check if the key type is not a pointer                                 
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsDense() const noexcept {
      return CT::Dense<T>;
   }

   /// Get the size of a single key, in bytes                                 
   ///   @return the number of bytes a single key contains                    
   TEMPLATE()
   constexpr Size TABLE()::GetStride() const noexcept {
      return sizeof(T); 
   }

   /// Get a raw key entry (const)                                            
   ///   @param index - the key index                                         
   ///   @return a constant reference to the element                          
   TEMPLATE() LANGULUS(INLINED)
   constexpr const T& TABLE()::GetRaw(Offset index) const noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a raw key entry                                                    
   ///   @param index - the key index                                         
   ///   @return a mutable reference to the element                           
   TEMPLATE() LANGULUS(INLINED)
   constexpr T& TABLE()::GetRaw(Offset index) noexcept {
      return GetValues().GetRaw()[index];
   }

   /// Get a handle to a key                                                  
   ///   @param index - the key index                                         
   ///   @return the handle                                                   
   TEMPLATE() LANGULUS(INLINED)
   constexpr Handle<T> TABLE()::GetHandle(Offset index) noexcept {
      return GetValues().GetHandle(index);
   }

   /// Get the key meta data                                                  
   /// Also implicitly initializes the internal key type                      
   ///   @attention this shouldn't be called on static initialization time    
   ///   @return the meta definition of the key type                          
   TEMPLATE() LANGULUS(INLINED)
   DMeta TABLE()::GetType() const {
      return BlockSet::GetType<TSet>();
   }

   /// Check if value origin type matches any of the list                     
   ///   @tparam T1, TN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TABLE()::Is() const noexcept {
      return CT::SameAsOneOf<T, T1, TN...>;
   }

   /// Check if value origin type matches another                             
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::Is(DMeta value) const noexcept {
      return GetType()->Is(value);
   }

   /// Check if cv-unqualified value type matches any of the list             
   ///   @tparam T1, TN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsSimilar() const noexcept {
      return CT::SimilarAsOneOf<T, T1, TN...>;
   }

   /// Check if cv-unqualified value type matches another                     
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsSimilar(DMeta value) const noexcept {
      return GetType()->IsSimilar(value);
   }

   /// Check if value type exactly matches any of the list                    
   ///   @tparam T1, TN... - the list of types to compare against             
   ///   @return true if value type matches at least one of the others        
   TEMPLATE() template<CT::Data T1, CT::Data... TN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsExact() const noexcept {
      return CT::ExactAsOneOf<T, T1, TN...>;
   }

   /// Check if value type exactly matches another                            
   ///   @param value - the value type to compare against                     
   ///   @return true if value matches the contained key origin type          
   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TABLE()::IsExact(DMeta value) const noexcept {
      return GetType()->IsExact(value);
   }

   /// Reserves space for the specified number of pairs                       
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of pairs to allocate                           
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Reserve(Count count) {
      BlockSet::Reserve<TSet>(count);
   }

   /// Unfold-insert elements, semantically or not                            
   ///   @param t1, tail... - elements, or arrays of elements, to insert      
   ///   @return the number of inserted elements                              
   TEMPLATE() template<class T1, class... TAIL>
   requires CT::Inner::UnfoldMakableFrom<T, T1, TAIL...> LANGULUS(INLINED)
   Count TABLE()::Insert(T1&& t1, TAIL&&...tail){
      Count inserted = 0;
        inserted += UnfoldInsert<TSet>(Forward<T1>(t1));
      ((inserted += UnfoldInsert<TSet>(Forward<TAIL>(tail))), ...);
      return inserted;
   }
   
   /// Move-insert a pair inside the map                                      
   ///   @param rhs - the pair to insert                                      
   ///   @return a reference to this table for chaining                       
   TEMPLATE() template<class T1>
   requires CT::Inner::UnfoldMakableFrom<T, T1> LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (T1&& rhs){
      Insert(Forward<T1>(rhs));
      return *this;
   }

   /// Clears all data, but doesn't deallocate, and retains state             
   TEMPLATE()
   void TABLE()::Clear() {
      BlockSet::Clear<TSet>();
   }

   /// Clears all data, state, and deallocates                                
   TEMPLATE()
   void TABLE()::Reset() {
      BlockSet::Reset<TSet>();
   }
   
   /// Safely erases element at a specific iterator                           
   ///   @attention assumes iterator is produced by this map instance         
   ///   @param index - the index to remove                                   
   ///   @return the iterator of the previous element, unless index is the    
   ///           first, or at the end already                                 
   TEMPLATE()
   typename TABLE()::Iterator TABLE()::RemoveIt(const Iterator& index) {
      const auto sentinel = GetReserved();
      auto offset = static_cast<Offset>(index.mInfo - mInfo);
      if (offset >= sentinel)
         return end();

      RemoveInner<T>(offset--); //TODO what if map shrinks, offset might become invalid? Doesn't shrink for now
      
      while (offset < sentinel and 0 == mInfo[offset])
         --offset;

      if (offset >= sentinel)
         offset = 0;

      return {mInfo + offset, index.mSentinel, &GetRaw(offset)};
   }

   /// Erase a pair via key                                                   
   ///   @param match - the key to search for                                 
   ///   @return the number of removed pairs                                  
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Remove(const T& match) {
      return BlockSet::template Remove<TABLE(), T>(match);
   }

   /// If possible reallocates the map to a smaller one                       
   TEMPLATE()
   void TABLE()::Compact() {
      TODO();
   }


   ///                                                                        
   ///   SEARCH                                                               
   ///                                                                        
   
   /// Checks if both tables contain the same entries                         
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (CT::Set auto const& other) const {
      return BlockSet::operator == <TSet> (other);
   }

   /// Search for a key inside the table                                      
   ///   @param key - the key to search for                                   
   ///   @return true if key is found, false otherwise                        
   TEMPLATE() template<CT::NotSemantic T1>
   requires ::std::equality_comparable_with<T, T1> LANGULUS(INLINED)
   bool TABLE()::Contains(T1 const& key) const {
      return BlockSet::Contains<TSet>(key);
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TEMPLATE() template<CT::NotSemantic T1>
   requires ::std::equality_comparable_with<T, T1> LANGULUS(INLINED)
   Index TABLE()::Find(T1 const& key) const {
      return BlockSet::Find<TSet>(key);
   }
   
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() template<CT::NotSemantic T1>
   requires ::std::equality_comparable_with<T, T1> LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::FindIt(T1 const& key) {
      const auto found = FindInner<TABLE()>(key);
      if (found == InvalidOffset)
         return end();

      return {
         GetInfo() + found, GetInfoEnd(),
         &GetRaw(found)
      };
   }
      
   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() template<CT::NotSemantic T1>
   requires ::std::equality_comparable_with<T, T1> LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::FindIt(T1 const& key) const {
      return const_cast<TABLE()*>(this)->FindIt(key);
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   const TAny<T>& TABLE()::GetValues() const noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get the templated values container                                     
   ///   @attention for internal use only, elements might not be initialized  
   TEMPLATE() LANGULUS(INLINED)
   TAny<T>& TABLE()::GetValues() noexcept {
      return BlockSet::GetValues<T>();
   }

   /// Get element at any index, safely                                       
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TEMPLATE()
   T& TABLE()::Get(CT::Index auto i) {
      auto offset = mKeys.SimplifyIndex<T>(i);
      auto info = GetInfo();
      const auto infoEnd = GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            if (offset == 0)
               return GetRaw(info - GetInfo());
            --offset;
         }
         ++info;
      }

      LANGULUS_THROW(Access, "This shouldn't be reached, ever");
   }
   
   /// Get element at any index, safely (const)                               
   ///   @param i - the index                                                 
   ///   @return the immutable key reference                                  
   TEMPLATE() LANGULUS(INLINED)
   const T& TABLE()::Get(CT::Index auto i) const {
      return const_cast<TABLE()*>(this)->Get(i);
   }

   /// Get element at an index, safely                                        
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TEMPLATE() LANGULUS(INLINED)
   T& TABLE()::operator[] (CT::Index auto i) {
      return Get(i);
   }

   /// Get element at an index, safely (const)                                
   ///   @param i - the index                                                 
   ///   @return the mutable key reference                                    
   TEMPLATE() LANGULUS(INLINED)
   const T& TABLE()::operator[] (CT::Index auto i) const {
      return Get(i);
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::begin() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::last() noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::begin() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info, or hit sentinel at the end              
      auto info = GetInfo();
      while (not *info) ++info;
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::last() const noexcept {
      if (IsEmpty())
         return end();

      // Seek first valid info in reverse, until one past first is met  
      auto info = GetInfoEnd();
      while (info >= GetInfo() and not *--info);
      return {info, GetInfoEnd(), &GetRaw(info - GetInfo())};
   }

   /// Iterate all keys inside the map, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   /*TEMPLATE() template<class F>
   Count TABLE()::ForEachElement(F&& f) const {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      static_assert(CT::Block<A>, "Function argument must be a block type");

      Offset i {};
      if constexpr (not CT::Void<R>) {
         return GetValues().ForEachElement([&](const Block& element) {
            return mInfo[i++] ? f(element) : Flow::Continue;
         });
      }
      else {
         return GetValues().ForEachElement([&](const Block& element) {
            if (mInfo[i++])
               f(element);
         });
      }
   }

   /// Iterate all keys inside the map, and perform f() on them (mutable)     
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TEMPLATE() template<class F>
   Count TABLE()::ForEachElement(F&& f) {
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      static_assert(CT::Block<A>, "Function argument must be a block type");

      Offset i {};
      if constexpr (not CT::Void<R>) {
         return GetValues().ForEachElement([&](const Block& element) {
            return mInfo[i++] ? f(element) : Flow::Continue;
         });
      }
      else {
         return GetValues().ForEachElement([&](const Block& element) {
            if (mInfo[i++])
               f(element);
         });
      }
   }*/

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TABLE
