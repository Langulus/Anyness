///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "BlockSet.hpp"

namespace Langulus::Anyness
{

   /// Shallow-copy construction                                              
   ///   @param other - the table to copy                                     
   LANGULUS(INLINED)
   BlockSet::BlockSet(const BlockSet& other)
      : mInfo {other.mInfo}
      , mKeys {other.mKeys} {
      mKeys.Keep();
   }

   /// Move construction                                                      
   ///   @param other - the table to move                                     
   LANGULUS(INLINED)
   BlockSet::BlockSet(BlockSet&& other) noexcept
      : mInfo {other.mInfo}
      , mKeys {other.mKeys} {
      other.mKeys.ResetMemory();
      other.mKeys.ResetState();
   }
   
   /// Semantic copy (block has no ownership, so always just shallow copy)    
   ///   @tparam S - the semantic to use (irrelevant)                         
   ///   @param other - the block to shallow-copy                             
   template<CT::Semantic S>
   LANGULUS(INLINED)
   constexpr BlockSet::BlockSet(S&& other) noexcept
      : mInfo {other.mValue.mInfo}
      , mKeys {other.mValue.mKeys} {
      static_assert(CT::Set<TypeOf<S>>, "S type should be a set type");
      if constexpr (S::Move && !S::Keep)
         other.mValue.mKeys.mEntry = nullptr;
      else if constexpr (!S::Move && !S::Keep)
         mKeys.mEntry = nullptr;
   }
   
   /// Manual construction via an initializer list                            
   ///   @param initlist - the initializer list to forward                    
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   BlockSet::BlockSet(::std::initializer_list<T> initlist)
      : BlockSet {} {
      Mutate<T>();
      Allocate(initlist.size());
      for (auto& it : initlist)
         Insert(it);
   }

   /// Destroys the map and all it's contents                                 
   LANGULUS(INLINED)
   BlockSet::~BlockSet() {
      Free();
   }

   /// Move a table                                                           
   ///   @param rhs - the table to move                                       
   ///   @return a reference to this table                                    
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator = (BlockSet&& rhs) noexcept {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) BlockSet {Forward<BlockSet>(rhs)};
      return *this;
   }

   /// Creates a shallow copy of the given table                              
   ///   @param rhs - the table to reference                                  
   ///   @return a reference to this table                                    
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator = (const BlockSet& rhs) {
      if (&rhs == this)
         return *this;

      Reset();
      new (this) BlockSet {rhs};
      return *this;
   }
   
   /// Insert a single pair into a cleared map                                
   ///   @param pair - the pair to copy                                       
   ///   @return a reference to this table                                    
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator = (const CT::Data auto& element) {
      Clear();
      Insert(element);
      return *this;
   }

   /// Emplace a single pair into a cleared map                               
   ///   @param pair - the pair to emplace                                    
   ///   @return a reference to this table                                    
   LANGULUS(INLINED)
   BlockSet& BlockSet::operator = (CT::Data auto&& element) noexcept {
      Clear();
      Insert(::std::move(element));
      return *this;
   }
   
   /// Semantically transfer the members of one set onto another              
   ///   @tparam TO - the type of set we're transferring to                   
   ///   @tparam S - the semantic to use for the transfer (deducible)         
   ///   @param from - the set and semantic to transfer from                  
   template<class TO, CT::Semantic S>
   LANGULUS(INLINED)
   void BlockSet::BlockTransfer(S&& other) {
      using FROM = TypeOf<S>;
      static_assert(CT::Set<TO>, "TO must be a set type");
      static_assert(CT::Set<FROM>, "FROM must be a set type");

      mKeys.mCount = other.mValue.mKeys.mCount;

      if constexpr (!CT::TypedSet<TO>) {
         // TO is not statically typed                                  
         mKeys.mType = other.mValue.GetType();
         mKeys.mState = other.mValue.mKeys.mState;
      }
      else {
         // TO is statically typed                                      
         mKeys.mType = MetaData::Of<TypeOf<TO>>();
         mKeys.mState = other.mValue.mKeys.mState + DataState::Typed;
      }

      if constexpr (S::Shallow) {
         mKeys.mRaw = other.mValue.mKeys.mRaw;
         mKeys.mReserved = other.mValue.mKeys.mReserved;
         mInfo = other.mValue.mInfo;

         if constexpr (S::Keep) {
            // Move/Copy other                                          
            mKeys.mEntry = other.mValue.mKeys.mEntry;

            if constexpr (S::Move) {
               if constexpr (!FROM::Ownership) {
                  // Since we are not aware if that block is referenced 
                  // or not we reference it just in case, and we also   
                  // do not reset 'other' to avoid leaks When using raw 
                  // BlockSets, it's your responsibility to take care   
                  // of ownership.                                      
                  Keep();
               }
               else {
                  other.mValue.mKeys.ResetMemory();
                  other.mValue.mKeys.ResetState();
               }
            }
            else Keep();
         }
         else if constexpr (S::Move) {
            // Abandon other                                            
            mKeys.mEntry = other.mValue.mKeys.mEntry;
            other.mValue.mKeys.mEntry = nullptr;
         }
      }
      else {
         // We're cloning, so we guarantee, that data is no longer      
         // static                                                      
         mKeys.mState -= DataState::Static;

         if constexpr (CT::TypedSet<TO>)
            BlockClone<TO>(other.mValue);
         else if constexpr (CT::TypedSet<FROM>)
            BlockClone<FROM>(other.mValue);
         else {
            // Use type-erased cloning                                  
            auto asTo = reinterpret_cast<TO*>(this);
            asTo->AllocateFresh(other.mValue.GetReserved());

            // Clone info array                                         
            CopyMemory(asTo->mInfo, other.mValue.mInfo, GetReserved() + 1);

            auto info = asTo->GetInfo();
            const auto infoEnd = asTo->GetInfoEnd();
            auto dstKey = asTo->GetValue(0);
            auto srcKey = other.mValue.GetValue(0);
            while (info != infoEnd) {
               if (*info) {
                  dstKey.CallUnknownSemanticConstructors(
                     1, Langulus::Clone(srcKey));
               }

               ++info;
               dstKey.Next();
               srcKey.Next();
            }
         }
      }
   }
   
   /// Clone info and keys from a statically typed set                        
   ///   @attention assumes this is not allocated                             
   ///   @tparam T - the statically optimized type of set we're using         
   ///   @param other - the set we'll be cloning                              
   template<class T>
   void BlockSet::BlockClone(const BlockSet& other) {
      static_assert(CT::TypedSet<T>, "T must be statically typed set");
      LANGULUS_ASSUME(DevAssumes, !mKeys.mRaw, "Set is already allocated");

      // Use statically optimized cloning                               
      auto asFrom = reinterpret_cast<T*>(&const_cast<BlockSet&>(other));
      auto asTo = reinterpret_cast<T*>(this);
      asTo->AllocateFresh(other.GetReserved());

      // Clone info array                                               
      CopyMemory(asTo->mInfo, asFrom->mInfo, GetReserved() + 1);

      // Clone keys and values                                          
      auto info = asTo->GetInfo();
      const auto infoEnd = asTo->GetInfoEnd();
      auto dstKey = asTo->GetHandle(0);
      auto srcKey = asFrom->GetHandle(0);
      while (info != infoEnd) {
         if (*info)
            dstKey.New(Langulus::Clone(srcKey));

         ++info;
         ++dstKey;
         ++srcKey;
      }
   }

} // namespace Langulus::Anyness
