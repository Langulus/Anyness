///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Set.hpp"
#include "../blocks/BlockSet/BlockSet-Construct.inl"
#include "../blocks/BlockSet/BlockSet-Capsulation.inl"
#include "../blocks/BlockSet/BlockSet-Indexing.inl"
#include "../blocks/BlockSet/BlockSet-RTTI.inl"
#include "../blocks/BlockSet/BlockSet-Compare.inl"
#include "../blocks/BlockSet/BlockSet-Memory.inl"
#include "../blocks/BlockSet/BlockSet-Insert.inl"
#include "../blocks/BlockSet/BlockSet-Remove.inl"
#include "../blocks/BlockSet/BlockSet-Iteration.inl"

#define TEMPLATE() template<bool ORDERED>
#define TABLE() Set<ORDERED>


namespace Langulus::Anyness
{

   /// Shallow-copy constructor                                               
   ///   @param other - the container to shallow-copy                         
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::Set(const Set& other)
      : Set {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - the container to move                                 
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::Set(Set&& other)
      : Set {Move(other)} {}
   
   /// Unfold constructor                                                     
   /// If there's one set argument, it will be absorbed                       
   ///   @param t1 - first element (can be semantic)                          
   ///   @param tail... - the rest of the elements (optional, can be semantic)
   TEMPLATE() template<class T1, class...TAIL>
   requires CT::Inner::UnfoldInsertable<T1, TAIL...>
   LANGULUS(INLINED) TABLE()::Set(T1&& t1, TAIL&&...tail) {
      if constexpr (sizeof...(TAIL) == 0) {
         using S = SemanticOf<T1>;
         using T = TypeOf<S>;

         if constexpr (CT::Set<T>)
            BlockTransfer<Set>(S::Nest(t1));
         else
            Insert<Set>(Forward<T1>(t1));
      }
      else Insert<Set>(Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Set destructor                                                         
   TEMPLATE() LANGULUS(INLINED)
   TABLE()::~Set() {
      Free<Set>();
   }

   /// Copy assignment                                                        
   ///   @param rhs - unordered set to copy-insert                            
   ///   @return a reference to this set                                      
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (const Set& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - unordered set to move-insert                            
   ///   @return a reference to this set                                      
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (Set&& rhs) {
      return operator = (Move(rhs));
   }

   /// Pair/map assignment, semantic or not                                   
   ///   @param rhs - the pair, or map to assign                              
   ///   @return a reference to this container                                
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator = (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::Set<T>) {
         // Potentially absorb a container                              
         if (static_cast<const BlockSet*>(this)
          == static_cast<const BlockSet*>(&DesemCast(rhs)))
            return *this;

         Free<Set>();
         new (this) Set {S::Nest(rhs)};
      }
      else {
         // Unfold-insert                                               
         Clear();
         BlockSet::UnfoldInsert<Set>(S::Nest(rhs));
      }

      return *this;
   }
   
   /// Get a valid key by any index, safely                                   
   ///   @param index - the index to use                                      
   ///   @return the element, wrapped in a Block                              
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::Get(const CT::Index auto index) const {
      return BlockSet::Get<Set>(index);
   }
   
   /// Accesses set elements based on a safe index, that accounts for         
   /// empty table slots                                                      
   ///   @param idx - the index to access                                     
   ///   @return a reference to the value, or a block if set is type-erased   
   TEMPLATE() LANGULUS(INLINED)
   decltype(auto) TABLE()::operator[] (const CT::Index auto index) const {
      return BlockSet::operator[]<Set>(index);
   }
   
   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::begin() noexcept {
      return BlockSet::begin<Set>();
   }
   
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::begin() const noexcept {
      return BlockSet::begin<Set>();
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::last() noexcept {
      return BlockSet::last<Set>();
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::last() const noexcept {
      return BlockSet::last<Set>();
   }

   /// Iterate keys inside the map, and perform a set of functions on them    
   /// depending on the contained type                                        
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEach(auto&&...f) const {
      return BlockSet::ForEach<REVERSE, const Set>(
         Forward<Deref<decltype(f)>>(f)...);
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEach(auto&&...f) {
      return BlockSet::ForEach<REVERSE, Set>(
         Forward<Deref<decltype(f)>>(f)...);
   }

   /// Iterate all keys inside the set, and perform f() on them               
   /// You can break the loop, by returning false inside f()                  
   ///   @param f - the function to call for each key block                   
   ///   @return the number of successful f() executions                      
   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachElement(auto&& f) const {
      return BlockSet::ForEachElement<REVERSE, const Set>(
         Forward<Deref<decltype(f)>>(f));
   }

   TEMPLATE() template<bool REVERSE> LANGULUS(INLINED)
   Count TABLE()::ForEachElement(auto&& f) {
      return BlockSet::ForEachElement<REVERSE, Set>(
         Forward<Deref<decltype(f)>>(f));
   }

   /// Iterate each subblock of keys inside the set, and perform a set of     
   /// functions on them                                                      
   ///   @param f - the functions to call for each key block                  
   ///   @return the number of successful f() executions                      
   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TABLE()::ForEachDeep(auto&&...f) const {
      return BlockSet::ForEachDeep<REVERSE, const Set>(
         Forward<Deref<decltype(f)>>(f)...);
   }

   TEMPLATE() template<bool REVERSE, bool SKIP> LANGULUS(INLINED)
   Count TABLE()::ForEachDeep(auto&&...f) {
      return BlockSet::ForEachDeep<REVERSE, Set>(
         Forward<Deref<decltype(f)>>(f)...);
   }
   
   /// Check if this value type is similar to one of the listed types,        
   /// ignoring density and cv-qualifiers                                     
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if value type is similar to at least one of the types   
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TABLE()::Is() const noexcept {
      return BlockSet::Is<Set, T1, TN...>();
   }
   
   /// Check if value type loosely matches a given type, ignoring             
   /// density and cv-qualifiers                                              
   ///   @param meta - the type to check for                                  
   ///   @return true if this set contains similar value data                 
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::Is(DMeta meta) const noexcept {
      return BlockSet::Is<Set>(meta);
   }

   /// Check if value type is similar to one of the listed types,             
   /// ignoring cv-qualifiers only                                            
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if value type is similar to at least one of the types   
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsSimilar() const noexcept {
      return BlockSet::IsSimilar<Set, T1, TN...>();
   }
   
   /// Check if value type loosely matches a given type, ignoring             
   /// cv-qualifiers only                                                     
   ///   @param meta - the type to check for                                  
   ///   @return true if this set contains similar value data                 
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsSimilar(DMeta meta) const noexcept {
      return BlockSet::IsSimilar<Set>(meta);
   }

   /// Check if value type is exactly as one of the listed types,             
   /// including by density and cv-qualifiers                                 
   ///   @tparam T1, TN... - the types to compare against                     
   ///   @return true if value type exactly matches at least one type         
   TEMPLATE() template<CT::Data T1, CT::Data...TN> LANGULUS(INLINED)
   constexpr bool TABLE()::IsExact() const noexcept {
      return BlockSet::IsExact<Set, T1, TN...>();
   }
   
   /// Check if value type is exactly the provided type,                      
   /// including the density and cv-qualifiers                                
   ///   @param type - the type to match                                      
   ///   @return true if this set contains this exact value data              
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::IsExact(DMeta meta) const noexcept {
      return BlockSet::IsExact<Set>(meta);
   }

   /// Search for a key inside the table, and return it if found              
   ///   @param key - the key to search for                                   
   ///   @return the index if key was found, or IndexNone if not              
   TEMPLATE() LANGULUS(INLINED)
   Index TABLE()::Find(const CT::NotSemantic auto& key) const {
      return BlockSet::Find<Set>(key);
   }

   /// Search for a key inside the table, and return an iterator to it        
   ///   @param key - the key to search for                                   
   ///   @return the iterator                                                 
   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::Iterator TABLE()::FindIt(const CT::NotSemantic auto& key) {
      return BlockSet::FindIt<Set>(key);
   }

   TEMPLATE() LANGULUS(INLINED)
   typename TABLE()::ConstIterator TABLE()::FindIt(const CT::NotSemantic auto& key) const {
      return BlockSet::FindIt<Set>(key);
   }

   /// Insert an element/set/array in the set                                 
   ///   @attention << and >> do the same thing, as sets aren't sequential    
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (CT::Inner::UnfoldInsertable auto&& other) {
      BlockSet::UnfoldInsert<Set>(Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Insert an element/set/array in the set                                 
   ///   @attention << and >> do the same thing, as sets aren't sequential    
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator >> (CT::Inner::UnfoldInsertable auto&& other) {
      BlockSet::UnfoldInsert<Set>(Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Reserves space for the specified number of elements                    
   ///   @attention does nothing if reserving less than current reserve       
   ///   @param count - number of elements to allocate                        
   TEMPLATE() LANGULUS(INLINED)
   void TABLE()::Reserve(Count count) {
      BlockSet::Reserve<Set>(count);
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TABLE