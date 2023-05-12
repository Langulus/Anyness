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

      if constexpr (CT::Set<T>) {
         // Construct from any kind of set                              
         if constexpr (!T::Ordered) {
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
                     GetBucket(hashmask, key),
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
      else if constexpr (CT::Array<T>) {
         // Construct from array of elements                            
         for (auto& key : *other)
            Insert(S::Nest(key));
      }
      else {
         // Construct from any kind of pair                             
         mKeys.mType = MetaData::Of<T>();

         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         // Insert a statically typed element                           
         InsertInner<false>(
            GetBucket(MinimalAllocation - 1, *other),
            other.Forward()
         );
      }
   }
   
   /// Create from a list of elements                                         
   ///   @param head - first element                                          
   ///   @param tail - tail of elements                                       
   template<CT::Data HEAD, CT::Data... TAIL>
   OrderedSet::OrderedSet(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1) {
      if constexpr (CT::Semantic<HEAD>)
         mKeys.mType = MetaData::Of<TypeOf<HEAD>>();
      else
         mKeys.mType = MetaData::Of<HEAD>();

      constexpr auto capacity = Roof2(
         sizeof...(TAIL) + 1 < MinimalAllocation
            ? MinimalAllocation
            : sizeof...(TAIL) + 1
      );

      AllocateFresh(capacity);
      ZeroMemory(mInfo, capacity);
      mInfo[capacity] = 1;
      Inner::NestedSemanticInsertion(
         *this, Forward<HEAD>(head), Forward<TAIL>(tail)...
      );
   }

   /// Set destructor                                                         
   LANGULUS(INLINED)
   OrderedSet::~OrderedSet() {
      Free();
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
               GetBucket(GetReserved() - 1, *other),
               other.Forward()
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
   
   /// Insert a single element inside table via copy                          
   ///   @param key - the key to add                                          
   ///   @return 1 if element was inserted, zero otherwise                    
   LANGULUS(INLINED)
   Count OrderedSet::Insert(CT::NotSemantic auto& key) {
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
   Count OrderedSet::Insert(CT::Semantic auto&& key) {
      using S = Decay<decltype(key)>;
      using T = TypeOf<S>;

      Mutate<T>();
      Reserve(GetCount() + 1);
      InsertInner<true>(
         GetBucket(GetReserved() - 1, *key),
         key.Forward()
      );
      return 1;
   }
   
   /// Copy-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator << (const CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }
   
   /// Copy-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   OrderedSet& OrderedSet::operator << (CT::NotSemantic auto& item) {
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
   LANGULUS(INLINED)
   Count OrderedSet::InsertUnknown(CT::Semantic auto&& key) {
      using S = Decay<decltype(key)>;
      static_assert(CT::Block<TypeOf<S>>, "S's type must be a block type");

      Mutate(key->mType);
      Reserve(GetCount() + 1);
      InsertInnerUnknown<true>(
         GetBucketUnknown(GetReserved() - 1, *key),
         key.Forward()
      );
      return 1;
   }
   
} // namespace Langulus::Anyness
