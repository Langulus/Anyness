///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "OrderedSet.hpp"

namespace Langulus::Anyness
{
   
   /// Default unordered set constructor                                      
   LANGULUS(INLINED)
   constexpr OrderedSet::OrderedSet()
      : BlockSet {} {}

   /// Create from a list of elements                                         
   ///   @tparam T - the element type                                         
   ///   @param list - list of elements                                       
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(::std::initializer_list<T> list) {
      mKeys.mType = MetaData::Of<T>();

      AllocateFresh(
         Roof2(
            list.size() < MinimalAllocation
               ? MinimalAllocation
               : list.size()
         )
      );

      ZeroMemory(mInfo, GetReserved());
      mInfo[GetReserved()] = 1;

      for (auto& it : list) {
         // Insert a dynamically typed pair                             
         InsertInner<true>(GetBucket(it), Copy(it));
      }
   }

   /// Copy constructor                                                       
   ///   @param other - set to shallow-copy                                   
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(const OrderedSet& other)
      : OrderedSet {Langulus::Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - set to move                                           
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(OrderedSet&& other) noexcept
      : OrderedSet {Langulus::Move(other)} {}

   /// Semantic constructor from any set/element                              
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   template<CT::Semantic S>
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(S&& other) noexcept {
      using T = TypeOf<S>;

      if constexpr (CT::Set<T>) {
         // Construct from any kind of set                              
         if constexpr (!T::Ordered) {
            // We have to reinsert everything, because source is        
            // unordered and uses a different bucketing approach        
            mKeys.mType = other.mValue.GetType();

            AllocateFresh(other.mValue.GetReserved());
            ZeroMemory(mInfo, GetReserved());
            mInfo[GetReserved()] = 1;

            const auto hashmask = GetReserved() - 1;
            other.mValue.ForEach([this, hashmask](Block& element) {
               // Insert a dynamically typed element                    
               InsertInnerUnknown<false>(
                  element.GetHash().mHash & hashmask,
                  S::Nest(element)
               );
            });
         }
         else {
            // We can directly interface set, because it is ordered     
            // and uses the same bucketing approach                     
            BlockTransfer<OrderedSet>(other.Forward());
         }
      }
      else {
         // Construct from any kind of pair                             
         mKeys.mType = MetaData::Of<T>();

         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, GetReserved());
         mInfo[GetReserved()] = 1;

         // Insert a statically typed element                           
         InsertInner<false>(
            GetBucket(other.mValue),
            S::Nest(other.mValue)
         );
      }
   }

   /// Copy assignment                                                        
   ///   @param rhs - ordered set to copy-insert                              
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (const OrderedSet& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - ordered set to move-insert                              
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (OrderedSet&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Semantic assignment from any set/element                               
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   template<CT::Semantic S>
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (S&& other) noexcept {
      using T = TypeOf<S>;

      if constexpr (CT::Set<T>) {
         if (static_cast<const BlockSet*>(this)
          == static_cast<const BlockSet*>(&other.mValue))
            return *this;

         Free();
         new (this) OrderedSet {other.Forward()};
      }
      else {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Free();
            new (this) OrderedSet {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Clear();

            // Insert a statically typed pair                           
            InsertInner<false>(
               GetBucket(other.mValue),
               S::Nest(other.mValue)
            );
         }
      }

      return *this;
   }
   
   /// Insert a single element inside table via copy                          
   ///   @param key - the key to add                                          
   ///   @return 1 if element was inserted, zero otherwise                    
   LANGULUS(INLINED)
   Count OrderedSet::Insert(const CT::NotSemantic auto& key) {
      return Insert(Copy(key));
   }

   /// Insert a single element inside table via move                          
   ///   @param key - the key to add                                          
   ///   @return 1 if element was inserted, zero otherwise                    
   LANGULUS(INLINED)
   Count OrderedSet::Insert(CT::NotSemantic auto&& key) {
      return Insert(Move(key));
   }
      
   /// Semantically insert key                                                
   ///   @param key - the key to insert                                       
   ///   @return 1 if element was inserted, zero otherwise                    
   template<CT::Semantic S>
   Count OrderedSet::Insert(S&& key) {
      using T = TypeOf<S>;

      Mutate<T>();
      Allocate(GetCount() + 1);
      InsertInner<true>(GetBucket(key.mValue), key.Forward());
      return 1;
   }
   
   /// Copy-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator << (const CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }

   /// Move-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator << (CT::NotSemantic auto&& item) {
      return operator << (Move(item));
   }

   /// Semantic insertion of any element inside the set                       
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator << (CT::Semantic auto&& item) {
      Insert(item.Forward());
      return *this;
   }
   
   /// Semantically insert a type-erased element                              
   ///   @param key - the key to insert                                       
   ///   @return 1 if element was inserted                                    
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Count OrderedSet::InsertUnknown(S&& key) {
      static_assert(CT::Block<TypeOf<S>>,
         "S's type must be a block type");

      Mutate(key.mValue.mType);
      Allocate(GetCount() + 1);
      const auto index = key.mValue.GetHash().mHash & (GetReserved() - 1);
      InsertInnerUnknown<true>(index, key.Forward());
      return 1;
   }
   
} // namespace Langulus::Anyness
