///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedSet.hpp"

namespace Langulus::Anyness
{
   
   /// Default unordered set constructor                                      
   LANGULUS(INLINED)
   constexpr UnorderedSet::UnorderedSet()
      : BlockSet {} {}

   /// Copy constructor                                                       
   ///   @param other - set to shallow-copy                                   
   LANGULUS(INLINED)
      UnorderedSet::UnorderedSet(const UnorderedSet& other)
      : UnorderedSet {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - set to move                                           
   LANGULUS(INLINED)
   UnorderedSet::UnorderedSet(UnorderedSet&& other) 
      : UnorderedSet {Move(other)} {}

   /// Constructor from any set/element by copy                               
   ///   @param other - the set/element                                       
   LANGULUS(INLINED)
   UnorderedSet::UnorderedSet(const CT::NotSemantic auto& other)
      : UnorderedSet {Copy(other)} {}
   
   /// Constructor from any set/element by copy                               
   ///   @param other - the set/element                                       
   LANGULUS(INLINED)
   UnorderedSet::UnorderedSet(CT::NotSemantic auto& other)
      : UnorderedSet {Copy(other)} {}
   
   /// Constructor from any set/element by move                               
   ///   @param other - the set/element                                       
   LANGULUS(INLINED)
   UnorderedSet::UnorderedSet(CT::NotSemantic auto&& other)
      : UnorderedSet {Move(other)} {}
   
   /// Semantic constructor from any set/element                              
   ///   @param other - the semantic type                                     
   LANGULUS(INLINED)
   UnorderedSet::UnorderedSet(CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Set<T>) {
         // Construct from any kind of set                              
         if constexpr (T::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
            mKeys.mType = other.mValue.GetType();

            AllocateFresh(other.mValue.GetReserved());
            ZeroMemory(mInfo, GetReserved());
            mInfo[GetReserved()] = 1;

            const auto hashmask = GetReserved() - 1;
            other.mValue.ForEach(
               [this, hashmask](Block& element) {
                  // Insert a dynamically typed element                 
                  InsertInnerUnknown<false>(
                     GetBucketUnknown(hashmask, element),
                     S::Nest(element)
                  );
               }
            );
         }
         else {
            // We can directly interface set, because it is unordered   
            // and uses the same bucketing approach                     
            BlockTransfer<UnorderedSet>(other.Forward());
         }
      }
      else {
         // Construct from any kind of pair                             
         mKeys.mType = MetaData::Of<T>();

         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         // Insert a statically typed element                           
         InsertInner<false>(
            GetBucket(MinimalAllocation - 1, other.mValue),
            S::Nest(other.mValue)
         );
      }
   }
   
   /// Create from a list of elements                                         
   ///   @param head - first element                                          
   ///   @param tail - tail of elements                                       
   template<CT::Data HEAD, CT::Data... TAIL>
   UnorderedSet::UnorderedSet(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1) {
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

   /// Copy assignment                                                        
   ///   @param rhs - unordered set to copy-insert                            
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator = (const UnorderedSet& rhs) {
      return operator = (Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - unordered set to move-insert                            
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator = (UnorderedSet&& rhs) {
      return operator = (Move(rhs));
   }

   /// Assign any set/element by copy                                         
   ///   @param other - the set/element to assign                             
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator = (const CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }

   /// Assign any set/element by copy                                         
   ///   @param other - the set/element to assign                             
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator = (CT::NotSemantic auto& other) {
      return operator = (Copy(other));
   }
   
   /// Assign any set/element by move                                         
   ///   @param other - the set/element to assign                             
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator = (CT::NotSemantic auto&& other) {
      return operator = (Move(other));
   }

   /// Assign from any set/element by a semantic                              
   ///   @param other - the semantic type                                     
   ///   @return a reference to this set                                      
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator = (CT::Semantic auto&& other) {
      using S = Decay<decltype(other)>;
      using T = TypeOf<S>;

      if constexpr (CT::Set<T>) {
         if (static_cast<const BlockSet*>(this)
          == static_cast<const BlockSet*>(&other.mValue))
            return *this;

         Free();
         new (this) UnorderedSet {other.Forward()};
      }
      else {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Free();
            new (this) UnorderedSet {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Clear();

            // Insert a statically typed pair                           
            InsertInner<false>(
               GetBucket(GetReserved() - 1, other.mValue),
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
   Count UnorderedSet::Insert(const CT::NotSemantic auto& key) {
      return Insert(Copy(key));
   }
   
   /// Insert a single element inside table via copy                          
   ///   @param key - the key to add                                          
   ///   @return 1 if element was inserted, zero otherwise                    
   LANGULUS(INLINED)
   Count UnorderedSet::Insert(CT::NotSemantic auto& key) {
      return Insert(Copy(key));
   }

   /// Insert a single element inside table via move                          
   ///   @param key - the key to add                                          
   ///   @return 1 if element was inserted, zero otherwise                    
   LANGULUS(INLINED)
   Count UnorderedSet::Insert(CT::NotSemantic auto&& key) {
      return Insert(Move(key));
   }
      
   /// Semantically insert key                                                
   ///   @param key - the key to insert                                       
   ///   @return 1 if element was inserted, zero otherwise                    
   LANGULUS(INLINED)
   Count UnorderedSet::Insert(CT::Semantic auto&& key) {
      using S = Decay<decltype(key)>;
      using T = TypeOf<S>;

      Mutate<T>();
      Reserve(GetCount() + 1);
      InsertInner<true>(
         GetBucket(GetReserved() - 1, key.mValue),
         key.Forward()
      );
      return 1;
   }
   
   /// Copy-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator << (const CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }
   
   /// Copy-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator << (CT::NotSemantic auto& item) {
      return operator << (Copy(item));
   }

   /// Move-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator << (CT::NotSemantic auto&& item) {
      return operator << (Move(item));
   }

   /// Semantic insertion of any element inside the set                       
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator << (CT::Semantic auto&& item) {
      Insert(item.Forward());
      return *this;
   }
   
   /// Semantically insert a type-erased element                              
   ///   @param key - the key to insert                                       
   ///   @return 1 if element was inserted                                    
   LANGULUS(INLINED)
   Count UnorderedSet::InsertUnknown(CT::Semantic auto&& key) {
      using S = Decay<decltype(key)>;
      static_assert(CT::Block<TypeOf<S>>, "S's type must be a block type");

      Mutate(key.mValue.mType);
      Reserve(GetCount() + 1);
      InsertInnerUnknown<true>(
         GetBucketUnknown(GetReserved() - 1, key.mValue),
         key.Forward()
      );
      return 1;
   }
   
} // namespace Langulus::Anyness
