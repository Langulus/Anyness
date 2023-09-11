///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../BlockMap.hpp"

namespace Langulus::Anyness
{
   
   /// Checks type compatibility and sets type for the type-erased map        
   ///   @tparam K - the key type                                             
   ///   @tparam V - the value type                                           
   template<CT::NotSemantic K, CT::NotSemantic V>
   LANGULUS(INLINED)
   void BlockMap::Mutate() {
      Mutate(MetaData::Of<K>(), MetaData::Of<V>());
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @param key - the key type                                            
   ///   @param value - the value type                                        
   LANGULUS(INLINED)
   void BlockMap::Mutate(DMeta key, DMeta value) {
      if (not mKeys.mType) {
         // Set a fresh key type                                        
         mKeys.mType = key;
      }
      else {
         // Key type already set, so check compatibility                
         LANGULUS_ASSERT(mKeys.IsExact(key), Mutate,
            "Attempting to mutate type-erased unordered map's key type"
         );
      }

      if (not mValues.mType) {
         // Set a fresh value type                                      
         mValues.mType = value;
      }
      else {
         // Value type already set, so check compatibility              
         LANGULUS_ASSERT(mValues.IsExact(value), Mutate,
            "Attempting to mutate type-erased unordered map's value type"
         );
      }
   }

   /// Check if key type exactly matches another                              
   template<class ALT_K>
   LANGULUS(INLINED)
   constexpr bool BlockMap::KeyIs() const noexcept {
      return mKeys.Is<ALT_K>();
   }

   /// Check if value type exactly matches another                            
   template<class ALT_V>
   LANGULUS(INLINED)
   constexpr bool BlockMap::ValueIs() const noexcept {
      return mValues.Is<ALT_V>();
   }

   /// Check if types of two maps are compatible for writing                  
   ///   @param other - map to test with                                      
   ///   @return true if both maps are type-compatible                        
   LANGULUS(INLINED)
   bool BlockMap::IsTypeCompatibleWith(const BlockMap& other) const noexcept {
      return mKeys.IsExact(other.mKeys.mType)
         and mValues.IsExact(other.mValues.mType);
   }

} // namespace Langulus::Anyness
