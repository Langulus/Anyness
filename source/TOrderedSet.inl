///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOrderedSet.hpp"

#define TEMPLATE() template<CT::Data T>
#define SET() TOrderedSet<T>

namespace Langulus::Anyness
{

   /// Copy-constructor                                                       
   ///   @param other - the map to copy                                       
   TEMPLATE()
   SET()::TOrderedSet(const TOrderedSet& other)
      : Self {Copy(other)} { }

   /// Move-constructor                                                       
   ///   @param other - the map to move                                       
   TEMPLATE()
   SET()::TOrderedSet(TOrderedSet&& other) noexcept
      : Self {Move(other)} { }

   /// Copy assignment                                                        
   ///   @param rhs - the map to copy                                         
   TEMPLATE()
   SET()& SET()::operator = (const TOrderedSet& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the map to move                                         
   TEMPLATE()
   SET()& SET()::operator = (TOrderedSet&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   /// Assign a single element by copy                                        
   ///   @param rhs - the element to assign                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   SET()& SET()::operator = (const T& rhs) {
      return operator = (Copy(rhs));
   }
   
   /// Assign a single element by move                                        
   ///   @param rhs - the element to assign                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   SET()& SET()::operator = (T&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantic assignment for an ordered set                                 
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the ordered set to use for construction                 
   TEMPLATE()
   SET()& SET()::operator = (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Set<ST>) {
         if (&static_cast<const BlockSet&>(rhs.mValue) == this)
            return *this;

         Base::Reset();
         new (this) Self {rhs.Forward()};
      }
      else if constexpr (CT::Exact<T, ST>) {
         Base::Clear();
         Insert(S::Nest(rhs.mValue));
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");

      return *this;
   }
   
   /// Merge an element inside the set by copy                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   LANGULUS(INLINED)
   Count SET()::Insert(const T& key) {
      return Insert(Copy(key));
   }

   /// Merge an element inside the set by move                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   LANGULUS(INLINED)
   Count SET()::Insert(T&& key) {
      return Insert(Move(key));
   }

   /// Merge an element inside the set by a semantic                          
   ///   @param key - the element to insert                                   
   ///   @return 1 if key was inserted, 0 otherwise                           
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Count SET()::Insert(S&& key) requires (CT::Exact<TypeOf<S>, T>) {
      Base::Reserve(Base::GetCount() + 1);
      Base::template InsertInner<true>(
         Base::GetBucket(Base::GetReserved() - 1, key.mValue),
         key.Forward()
      );
      return 1;
   }

   /// Merge an element inside the set by copy                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   SET()& SET()::operator << (const T& rhs) {
      return operator << (Copy(rhs));
   }

   /// Merge an element inside the set by move                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   SET()& SET()::operator << (T&& rhs) {
      return operator << (Move(rhs));
   }

   /// Merge an element inside the set by a semantic                          
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   template<CT::Semantic S>
   SET()& SET()::operator << (S&& rhs) requires (CT::Exact<TypeOf<S>, T>) {
      Insert(S::Nest(rhs.mValue));
      return *this;
   }

} // namespace Langulus::Anyness

#undef SET
#undef TEMPLATE

   
