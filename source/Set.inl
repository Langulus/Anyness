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
#include "blocks/BlockSet/BlockSet-Construct.inl"
#include "blocks/BlockSet/BlockSet-Capsulation.inl"
#include "blocks/BlockSet/BlockSet-Indexing.inl"
#include "blocks/BlockSet/BlockSet-RTTI.inl"
#include "blocks/BlockSet/BlockSet-Compare.inl"
#include "blocks/BlockSet/BlockSet-Memory.inl"
#include "blocks/BlockSet/BlockSet-Insert.inl"
#include "blocks/BlockSet/BlockSet-Remove.inl"
#include "blocks/BlockSet/BlockSet-Iteration.inl"

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

   /// Insert an element                                                      
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator << (CT::Inner::UnfoldInsertable auto&& other) {
      BlockSet::UnfoldInsert<Set>(Forward<decltype(other)>(other));
      return *this;
   }
   
   /// Insert an element                                                      
   ///   @param other - the data to insert                                    
   ///   @return a reference to this container for chaining                   
   TEMPLATE() LANGULUS(INLINED)
   TABLE()& TABLE()::operator >> (CT::Inner::UnfoldInsertable auto&& other) {
      BlockSet::UnfoldInsert<Set>(Forward<decltype(other)>(other));
      return *this;
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef TABLE