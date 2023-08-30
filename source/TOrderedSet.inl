///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TOrderedSet.hpp"
#include "OrderedSet.inl"

#define TEMPLATE() template<CT::Data T>
#define TABLE() TOrderedSet<T>


namespace Langulus::Anyness
{

   /// Copy-constructor                                                       
   ///   @param other - the map to copy                                       
   TEMPLATE()
   TABLE()::TOrderedSet(const TOrderedSet& other)
      : Self {Copy(other)} { }

   /// Move-constructor                                                       
   ///   @param other - the map to move                                       
   TEMPLATE()
   TABLE()::TOrderedSet(TOrderedSet&& other) noexcept
      : Self {Move(other)} { }

   /// Copy assignment                                                        
   ///   @param rhs - the map to copy                                         
   TEMPLATE()
   TABLE()& TABLE()::operator = (const TOrderedSet& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - the map to move                                         
   TEMPLATE()
   TABLE()& TABLE()::operator = (TOrderedSet&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   /// Assign a single element by copy                                        
   ///   @param rhs - the element to assign                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   TABLE()& TABLE()::operator = (const T& rhs) {
      return operator = (Copy(rhs));
   }
   
   /// Assign a single element by move                                        
   ///   @param rhs - the element to assign                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   TABLE()& TABLE()::operator = (T&& rhs) {
      return operator = (Move(rhs));
   }

   /// Semantic assignment for an ordered set                                 
   ///   @tparam S - the semantic (deducible)                                 
   ///   @param rhs - the ordered set to use for construction                 
   TEMPLATE()
   TABLE()& TABLE()::operator = (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using ST = TypeOf<S>;

      if constexpr (CT::Set<ST>) {
         if (&static_cast<const BlockSet&>(*rhs) == this)
            return *this;

         Base::Reset();
         new (this) Self {rhs.Forward()};
      }
      else if constexpr (CT::Exact<T, ST>) {
         Base::Clear();
         Insert(rhs.Forward());
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");

      return *this;
   }
   
   /// Checks if both tables contain the same entries                         
   ///   @param other - the table to compare against                          
   ///   @return true if tables match                                         
   TEMPLATE() LANGULUS(INLINED)
   bool TABLE()::operator == (const TOrderedSet& other) const {
      if (other.GetCount() != Base::GetCount())
         return false;

      auto info = Base::GetInfo();
      const auto infoEnd = Base::GetInfoEnd();
      while (info != infoEnd) {
         if (*info) {
            const auto lhs = info - Base::GetInfo();
            const auto rhs = other.FindIndex(Base::GetRaw(lhs));
            if (rhs == other.GetReserved())
               return false;
         }

         ++info;
      }

      return true;
   }

   /// Merge an element inside the set by copy                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(const T& key) {
      return Insert(Copy(key));
   }

   /// Merge an element inside the set by move                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE() LANGULUS(INLINED)
   Count TABLE()::Insert(T&& key) {
      return Insert(Move(key));
   }

   /// Merge an element inside the set by a semantic                          
   ///   @param key - the element to insert                                   
   ///   @return 1 if key was inserted, 0 otherwise                           
   TEMPLATE()
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Count TABLE()::Insert(S&& key) requires (CT::Exact<TypeOf<S>, T>) {
      Base::Reserve(Base::GetCount() + 1);
      Base::template InsertInner<true>(
         Base::GetBucket(Base::GetReserved() - 1, *key),
         key.Forward()
      );
      return 1;
   }

   /// Merge an element inside the set by copy                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   TABLE()& TABLE()::operator << (const T& rhs) {
      return operator << (Copy(rhs));
   }

   /// Merge an element inside the set by move                                
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   TABLE()& TABLE()::operator << (T&& rhs) {
      return operator << (Move(rhs));
   }

   /// Merge an element inside the set by a semantic                          
   ///   @param rhs - the element to insert                                   
   ///   @return a reference to this set for chaining                         
   TEMPLATE()
   template<CT::Semantic S>
   TABLE()& TABLE()::operator << (S&& rhs) requires (CT::Exact<TypeOf<S>, T>) {
      Insert(rhs.Forward());
      return *this;
   }

} // namespace Langulus::Anyness

#undef TABLE
#undef TEMPLATE

   
