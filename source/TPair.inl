///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TPair.hpp"

#define TEMPLATE() template<CT::NotSemantic K, CT::NotSemantic V>
#define PAIR() TPair<K, V>


namespace Langulus::Anyness
{

   /// Comparison                                                             
   ///   @param rhs - pair to compare against                                 
   ///   @return true if pairs match                                          
   TEMPLATE() LANGULUS(INLINED)
   bool PAIR()::operator == (CT::Pair auto const& rhs) const {
      return mKey == rhs.mKey and mValue == rhs.mValue;
   }

   /// Get the pair's hash                                                    
   ///   @attention hash is not cached, so this function is slow              
   ///   @return the hash                                                     
   TEMPLATE() LANGULUS(INLINED)
   Hash PAIR()::GetHash() const {
      return HashOf(mKey, mValue);
   }

   /// Get the type of the contained key                                      
   ///   @return the key type                                                 
   TEMPLATE() LANGULUS(INLINED)
   DMeta PAIR()::GetKeyType() const noexcept {
      return MetaDataOf<K>();
   }

   /// Get the type of the contained value                                    
   ///   @return the value type                                               
   TEMPLATE() LANGULUS(INLINED)
   DMeta PAIR()::GetValueType() const noexcept {
      return MetaDataOf<V>();
   }

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef PAIR

