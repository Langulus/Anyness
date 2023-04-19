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

   /// Check if key type matches another, ignoring qualifiers                 
   ///   @tparam ALT_T the type to compare against                            
   ///   @return true if types loosely match                                  
   template<class ALT_T>
   LANGULUS(INLINED)
   constexpr bool BlockSet::Is() const noexcept {
      return mKeys.template Is<ALT_T>();
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @tparam T - the type                                                 
   template<CT::Data T>
   LANGULUS(INLINED)
   void BlockSet::Mutate() {
      Mutate(MetaData::Of<T>());
   }

   /// Checks type compatibility and sets type for the type-erased map        
   ///   @param key - the key type                                            
   LANGULUS(INLINED)
   void BlockSet::Mutate(DMeta key) {
      if (!mKeys.mType) {
         // Set a fresh key type                                        
         mKeys.mType = key;
      }
      else {
         // Key type already set, so check compatibility                
         LANGULUS_ASSERT(mKeys.IsExact(key), Mutate,
            "Attempting to mutate type-erased unordered map's key type"
         );
      }
   }

} // namespace Langulus::Anyness
