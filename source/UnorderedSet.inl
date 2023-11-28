///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedSet.hpp"
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

      if constexpr (CT::Array<T>) {
         // Construct from array of elements                            
         for (auto& key : *other)
            Insert(S::Nest(key));
      }
      else if constexpr (CT::Set<T>) {
         // Construct from any kind of set                              
         if constexpr (T::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
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
            // We can directly interface set, because it is unordered   
            // and uses the same bucketing approach                     
            BlockTransfer<UnorderedSet>(other.Forward());
         }
      }
      else {
         // Construct from any kind of element                          
         mKeys.mType = MetaData::Of<T>();

         AllocateFresh(MinimalAllocation);
         ZeroMemory(mInfo, MinimalAllocation);
         mInfo[MinimalAllocation] = 1;

         // Insert a statically typed element                           
         InsertInner<false, false>(
            GetBucket(MinimalAllocation - 1, *other),
            other.Forward()
         );
      }
   }
   
   /// Create from a list of elements                                         
   ///   @param head - first element                                          
   ///   @param tail - tail of elements                                       
   template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
   UnorderedSet::UnorderedSet(T1&& t1, T2&& t2, TAIL&&... tail) {
      if constexpr (CT::Semantic<T1>)
         mKeys.mType = MetaData::Of<TypeOf<T1>>();
      else
         mKeys.mType = MetaData::Of<T1>();

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
   UnorderedSet::~UnorderedSet() {
      Free<UnorderedSet>();
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
          == static_cast<const BlockSet*>(&*other))
            return *this;

         Free<UnorderedSet>();
         new (this) UnorderedSet {other.Forward()};
      }
      else {
         if (GetUses() != 1) {
            // Reset and allocate fresh memory                          
            Free<UnorderedSet>();
            new (this) UnorderedSet {other.Forward()};
         }
         else {
            // Just destroy and reuse memory                            
            Clear<UnorderedSet>();

            // Insert an element                                        
            InsertInner<false, false>(
               GetBucket(GetReserved() - 1, *other),
               other.Forward()
            );
         }
      }

      return *this;
   }
   
   /// Copy-insert any element inside the set                                 
   ///   @param item - the element to insert                                  
   ///   @return a reference to this set for chaining                         
   LANGULUS(INLINED)
   UnorderedSet& UnorderedSet::operator << (auto&& item) {
      Insert(Forward<decltype(item)>(item));
      return *this;
   }
   
} // namespace Langulus::Anyness
