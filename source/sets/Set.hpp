///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../blocks/BlockSet.hpp"


namespace Langulus::Anyness
{

   ///                                                                        
   ///   Type-erased set                                                      
   ///                                                                        
   template<bool ORDERED = false>
   struct Set : BlockSet {
      using BlockType = Many;

      LANGULUS(POD) false;
      LANGULUS_BASES(BlockSet);

      static constexpr bool Ownership = true;
      static constexpr bool Ordered = ORDERED;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Set() noexcept = default;
      Set(const Set&);
      Set(Set&&);

      template<class T1, class...TAIL>
      requires CT::UnfoldInsertable<T1, TAIL...>
      Set(T1&&, TAIL&&...);

      ~Set();

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      Set& operator = (const Set&);
      Set& operator = (Set&&);
      Set& operator = (CT::UnfoldInsertable auto&&);

      ///                                                                     
      ///   Indexing                                                          
      ///                                                                     
      NOD() decltype(auto) Get(CT::Index auto) const;

      NOD() decltype(auto) operator[] (CT::Index auto) const;

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      using Iterator = BlockSet::Iterator<Set>;
      using ConstIterator = BlockSet::Iterator<const Set>;

      NOD() auto begin() noexcept -> Iterator;
      NOD() auto last()  noexcept -> Iterator;
      NOD() auto begin() const noexcept -> ConstIterator;
      NOD() auto last()  const noexcept -> ConstIterator;

      template<bool REVERSE = false>
      Count ForEach(auto&&...);
      template<bool REVERSE = false>
      Count ForEach(auto&&...) const;

      template<bool REVERSE = false>
      Count ForEachElement(auto&&);
      template<bool REVERSE = false>
      Count ForEachElement(auto&&) const;
      
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...);
      template<bool REVERSE = false, bool SKIP = true>
      Count ForEachDeep(auto&&...) const;

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Data, CT::Data...>
      NOD() constexpr bool Is() const noexcept;
      NOD() bool Is(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsSimilar() const noexcept;
      NOD() bool IsSimilar(DMeta) const noexcept;

      template<CT::Data, CT::Data...>
      NOD() constexpr bool IsExact() const noexcept;
      NOD() bool IsExact(DMeta) const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      using BlockSet::operator ==;

      NOD() auto Find  (const CT::NoIntent auto&) const -> Index;
      NOD() auto FindIt(const CT::NoIntent auto&)       -> Iterator;
      NOD() auto FindIt(const CT::NoIntent auto&) const -> ConstIterator;

      ///                                                                     
      ///   Memory management                                                 
      ///                                                                     
      void Reserve(Count);

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Set& operator << (CT::UnfoldInsertable auto&&);
      Set& operator >> (CT::UnfoldInsertable auto&&);
   };

} // namespace Langulus::Anyness
