///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "OrderedSet.hpp"
#include "blocks/BlockSet/BlockSet-Construct.inl"
#include "blocks/BlockSet/BlockSet-Capsulation.inl"
#include "blocks/BlockSet/BlockSet-Indexing.inl"
#include "blocks/BlockSet/BlockSet-RTTI.inl"
#include "blocks/BlockSet/BlockSet-Compare.inl"
#include "blocks/BlockSet/BlockSet-Memory.inl"
#include "blocks/BlockSet/BlockSet-Insert.inl"
#include "blocks/BlockSet/BlockSet-Remove.inl"
#include "blocks/BlockSet/BlockSet-Iteration.inl"


namespace Langulus::Anyness
{

   /// Copy constructor                                                       
   ///   @param other - set to shallow-copy                                   
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(const OrderedSet& other)
      : OrderedSet {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - set to move                                           
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(OrderedSet&& other)
      : OrderedSet {Move(other)} {}

   /// Constructor from any set/element by copy                               
   ///   @param other - the set/element                                       
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(const CT::NotSemantic auto& other)
      : OrderedSet {Copy(other)} {}
   
   /// Constructor from any set/element by copy                               
   ///   @param other - the set/element                                       
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(CT::NotSemantic auto& other)
      : OrderedSet {Copy(other)} {}
   
   /// Constructor from any set/element by move                               
   ///   @param other - the set/element                                       
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(CT::NotSemantic auto&& other)
      : OrderedSet {Move(other)} {}

   /// Semantic constructor from any set/element                              
   ///   @param other - the semantic type                                     
   LANGULUS(INLINED)
   OrderedSet::OrderedSet(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Array<T>) {
         // Construct from array of elements                            
         for (auto& key : *other)
            Insert(S::Nest(key));
      }
      else if constexpr (CT::Set<T>) {
         // Construct from any kind of set                              
         if constexpr (not T::Ordered) {
            // We have to reinsert everything, because source is        
            // unordered and uses a different bucketing approach        
            mKeys.mType = other->GetType();

            AllocateFresh(other->GetReserved());
            ZeroMemory(mInfo, GetReserved());
            mInfo[GetReserved()] = 1;

            const auto hashmask = GetReserved() - 1;
            if constexpr (CT::TypedSet<T>) {
               for (auto& key : *other) {
                  InsertInner<false>(
                     GetBucket(hashmask, key),
                     S::Nest(key)
                  );
               }
            }
            else {
               for (auto key : *other) {
                  InsertUnkownInner<false>(
                     GetBucketUnknown(hashmask, key),
                     S::Nest(key)
                  );
               }
            }
         }
         else {
            // We can directly interface set, because it is ordered     
            // and uses the same bucketing approach                     
            BlockTransfer<OrderedSet>(other.Forward());
         }
      }
      else {
         // Construct from any kind of pair                             
         mKeys.mType = MetaDataOf<T>();

         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         // Insert a statically typed element                           
         InsertInner<false, true>(
            GetBucket(MinimalAllocation - 1, *other),
            other.Forward()
         );
      }
   }
   
   /// Create from a list of elements                                         
   ///   @param head - first element                                          
   ///   @param tail - tail of elements                                       
   template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
   OrderedSet::OrderedSet(T1&& t1, T2&& t2, TAIL&&... tail) {
      if constexpr (CT::Semantic<T1>)
         mKeys.mType = MetaDataOf<TypeOf<T1>>();
      else
         mKeys.mType = MetaDataOf<T1>();

      constexpr auto capacity = Roof2(
         sizeof...(TAIL) + 2 < MinimalAllocation
            ? MinimalAllocation
            : sizeof...(TAIL) + 2
      );

      AllocateFresh(capacity);
      ZeroMemory(mInfo, capacity);
      mInfo[capacity] = 1;

      Insert(Forward<T1>(t1));
      Insert(Forward<T2>(t2));
      (Insert(Forward<TAIL>(tail)), ...);
   }

   /// Set destructor                                                         
   LANGULUS(INLINED)
   OrderedSet::~OrderedSet() {
      Free<OrderedSet>();
   }

   /// Copy assignment                                                        
   ///   @param rhs - ordered set to copy-insert                              
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (const OrderedSet& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - ordered set to move-insert                              
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (OrderedSet&& rhs) {
      return operator = (Move(rhs));
   }

   /// Assign any set/element by copy                                         
   ///   @param other - the semantic and element to merge                     
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (const CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }
   
   /// Assign any set/element by copy                                         
   ///   @param other - the semantic and element to merge                     
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }

   /// Assign any set/element by move                                         
   ///   @param other - the semantic and element to merge                     
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (CT::NotSemantic auto&& other) {
      return operator = (Move(other));
   }

   /// Assignment any set/element by a semantic                               
   ///   @param other - the semantic and element to merge                     
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Set<T>) {
         if (static_cast<const BlockSet*>(this)
          == static_cast<const BlockSet*>(&*other))
            return *this;

         Free<OrderedSet>();
         new (this) OrderedSet {other.Forward()};
      }
      else {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Free<OrderedSet>();
            new (this) OrderedSet {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Clear<OrderedSet>();

            // Insert a statically typed pair                           
            InsertInner<false, true>(
               GetBucket(GetReserved() - 1, *other),
               other.Forward()
            );
         }
      }

      return *this;
   }
   
   /// Insert a single element inside table via semantic or not               
   ///   @param key - the key (optionally in a semantic) to add               
   ///   @return 1 if element was inserted, zero otherwise                    
   LANGULUS(INLINED)
   Count OrderedSet::Insert(auto&& key) {
      return BlockSet::Insert<true>(
         Forward<Deref<decltype(key)>>(key)
      );
   }

   LANGULUS(INLINED)
   Count OrderedSet::InsertBlock(auto&& key) {
      return BlockSet::InsertBlock<true>(
         Forward<Deref<decltype(key)>>(key)
      );
   }

   /// Insert any kind of key, semantic or not                                
   ///   @param key - the key to insert                                       
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator << (auto&& key) {
      BlockSet::Insert<true>(Forward<decltype(key)>(key));
      return *this;
   }

} // namespace Langulus::Anyness
