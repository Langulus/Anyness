///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "blocks/BlockMap.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased map                                                      
   ///                                                                        
   template<bool ORDERED = false>
   struct Map : BlockMap {
      static constexpr bool Ownership = true;
      static constexpr bool Ordered = ORDERED;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Map() noexcept = default;
      Map(const Map&);
      Map(Map&&);

      template<class T1, class...TAIL>
      requires CT::Inner::UnfoldInsertable<T1, TAIL...>
      Map(T1&&, TAIL&&...);

      ~Map();

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Map& operator = (const Map&);
      Map& operator = (Map&&);
      Map& operator = (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Map& operator << (CT::Inner::UnfoldInsertable auto&&);
      Map& operator >> (CT::Inner::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      NOD() TIterator<Map> begin() noexcept;
      NOD() TIterator<Map> end() noexcept;
      NOD() TIterator<Map> last() noexcept;
      NOD() TIterator<const Map> begin() const noexcept;
      NOD() TIterator<const Map> end() const noexcept;
      NOD() TIterator<const Map> last() const noexcept;
   };

} // namespace Langulus::Anyness