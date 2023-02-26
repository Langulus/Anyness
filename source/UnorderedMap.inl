///                                                                           
/// Langulus::Anyness                                                         
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "UnorderedMap.hpp"

namespace Langulus::Anyness
{

   /// Default unordered map constructor                                      
   LANGULUS(ALWAYSINLINE)
   constexpr UnorderedMap::UnorderedMap()
      : BlockMap {} {}

   /// Create from a list of pairs                                            
   ///   @tparam P - the pair type                                            
   ///   @param list - list of pairs                                          
   template<CT::Pair P>
   LANGULUS(ALWAYSINLINE)
   UnorderedMap::UnorderedMap(::std::initializer_list<P> list) {
      mKeys.mType = list.begin()->GetKeyType();
      mValues.mType = list.begin()->GetValueType();

      AllocateData<false>(
         Roof2(
            list.size() < MinimalAllocation
               ? MinimalAllocation
               : list.size()
         )
      );

      for (auto& it : list) {
         InsertUnknown(
            Langulus::Copy(it.mKey), 
            Langulus::Copy(it.mValue)
         );
      }
   }

   /// Copy constructor                                                       
   ///   @param other - map to shallow-copy                                   
   LANGULUS(ALWAYSINLINE)
   UnorderedMap::UnorderedMap(const UnorderedMap& other)
      : UnorderedMap {Langulus::Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - map to move                                           
   LANGULUS(ALWAYSINLINE)
   UnorderedMap::UnorderedMap(UnorderedMap&& other) noexcept
      : UnorderedMap {Langulus::Move(other)} {}

   /// Semantic constructor from any map/pair                                 
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   template<CT::Semantic S>
   UnorderedMap::UnorderedMap(S&& other) noexcept {
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         mKeys.mType = other.mValue.GetKeyType();
         mValues.mType = other.mValue.GetValueType();

         // Construct from any kind of map                              
         if constexpr (T::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
            AllocateData<false>(other.mValue.GetReserved());
            other.mValue.ForEach([this](const T::Pair& pair) {
               InsertUnknown(S::Nest(pair));
            });
         }
         else if constexpr (S::Shallow) {
            // We can directly interface map, because it is unordered   
            // and uses the same bucketing approach                     
            mKeys.BlockTransfer<Any>(S::Nest(other.mValue.mKeys));
            mValues.BlockTransfer<Any>(S::Nest(other.mValue.mValues));
         }
         else {
            // We have to clone all valid entries                       
            TODO();
         }
      }
      else if constexpr (CT::Pair<T>) {
         // Construct from any kind of pair                             
         mKeys.mType = other.mValue.GetKeyType();
         mValues.mType = other.mValue.GetValueType();

         AllocateData<false>(MinimalAllocation);
      }
      else LANGULUS_ERROR("Unsupported semantic constructor");
   }

   /// Copy assignment of a pair                                              
   ///   @param rhs - pair to copy-insert                                     
   ///   @return a reference to this map                                      
   LANGULUS(ALWAYSINLINE)
   UnorderedMap& UnorderedMap::operator = (const CT::Pair auto& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assignment of a pair                                              
   ///   @param rhs - pair to move-insert                                     
   ///   @return a reference to this map                                      
   LANGULUS(ALWAYSINLINE)
   UnorderedMap& UnorderedMap::operator = (CT::Pair auto&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Copy assignment                                                        
   ///   @param rhs - unordered map to copy-insert                            
   ///   @return a reference to this map                                      
   LANGULUS(ALWAYSINLINE)
   UnorderedMap& UnorderedMap::operator = (const UnorderedMap& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   /// Move assignment                                                        
   ///   @param rhs - unordered map to move-insert                            
   ///   @return a reference to this map                                      
   LANGULUS(ALWAYSINLINE)
   UnorderedMap& UnorderedMap::operator = (UnorderedMap&& rhs) noexcept {
      return operator = (Langulus::Move(rhs));
   }

   /// Semantic assignment from any map/pair                                  
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the semantic type                                     
   template<CT::Semantic S>
   LANGULUS(ALWAYSINLINE)
   UnorderedMap& UnorderedMap::operator = (S&&) noexcept {
      using T = TypeOf<S>;

      if constexpr (CT::Map<T>) {
         // Construct from any kind of map                              
         if constexpr (T::Ordered) {
            // We have to reinsert everything, because source is        
            // ordered and uses a different bucketing approach          
            TODO();
         }
         else {
            // We can directly interface map, because it is unordered   
            // and uses the same bucketing approach                     
            TODO();
         }
      }
      else if constexpr (CT::Pair<T>) {
         // Construct from any kind of pair                             
      }
      else LANGULUS_ERROR("Unsupported semantic assignment");
      return *this;
   }

} // namespace Langulus::Anyness
